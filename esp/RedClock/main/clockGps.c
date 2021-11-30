/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

#include "clockMain.h"
/*
https://www.u-blox.com/sites/default/files/products/documents/u-blox7-V14_ReceiverDescriptionProtocolSpec_%28GPS.G7-SW-12001%29_Public.pdf
says 
"6.2 Navigation Epochs
Each navigation solution is triggered by the tick of the 1kHz clock nearest to the desired navigation solution
time. This tick is referred to as a navigation epoch. If the navigation solution attempt is successful, one of the
results is an accurate measurement of time in the time-base of the chosen GNSS system, called GNSS system
time. The difference between the calculated GNSS system time and receiver local time is called the clock bias
(and the clock drift is the rate at which this bias is changing).
In practice the receiver’s local oscillator will not be as stable as the atomic clocks to which GNSS systems are
referenced and consequently clock bias will tend to accumulate. However, when selecting the next navigation
epoch, the receiver will always try to use the 1kHz clock tick which it estimates to be closest to the desired fix
period as measured in GNSS system time. Consequently the number of 1kHz clock ticks between fixes will
occasionally vary (so when producing one fix per second, there will normally be 1000 clock ticks between fixes,
but sometimes, to correct drift away from GNSS system time, there will be 999 or 1001).
The GNSS system time calculated in the navigation solution is always converted to a time in both the GPS and
UTC time-bases for output.
Clearly when the receiver has chosen to use the GPS time-base for its GNSS system time, conversion to GPS
time requires no work at all, but conversion to UTC requires knowledge of the number of leap seconds since
GPS time started (and other minor correction terms). The relevant GPS to UTC conversion parameters are
transmitted periodically (every 12.5 minutes) by GPS satellites, but can also be supplied to the receiver via the
UBX-AID-HUI aiding message. By contrast when the receiver has chosen to use the GLONASS time-base as its
GNSS system time, conversion to GPS time is more difficult as it requires knowledge of the difference between
the two time-bases, but conversion to UTC is easier (as GLONASS time is closely linked to UTC).
Where insufficient information is available for the receiver to perform any of these time-base conversions
precisely, pre-defined default offsets are used. Consequently plausible times are nearly always generated, but
they may be wrong by a few seconds (especially shortly after receiver start). Depending on the configuration of
the receiver, such "invalid" times may well be output, but with flags indicating their state (e.g. the "valid" flags
in UBX-NAV-PVT)"

which explains why a cold start results in the GPS time being 2 seconds ahead of real time for several minutes.

*/
struct tm gGPStimeinfo;
time_t gGPStime_t;
uint64_t gGPStimeMessageReceived;

// GPS serial support


static const int RX_BUF_SIZE = 256;

void uart_init(void) {
// 128: printf("UART_FIFO_LEN=%d\n",UART_FIFO_LEN);
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_2, RX_BUF_SIZE/*rx*/, 0/*tx*/, 0/*evt queue size*/, NULL/* return evt queue handle*/, 0/*interrupt flags*/);
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE/*rts*/, UART_PIN_NO_CHANGE/*cts*/);
}

int sendData(/*const char* logName,*/int len, /*const*/ char* data)
{
    // calculate and overwrite the checksum https://github.com/PowerBroker2/NEO-6M_GPS/blob/master/src/neo6mGPS.cpp
    // also: https://stackoverflow.com/questions/23038037/ublox-neo-6m-ubx-command-sanity-check-with-fletcher-checksum
    // and https://en.wikipedia.org/wiki/Fletcher%27s_checksum#Straightforward
    uint8_t ck_a = 0;
	uint8_t ck_b = 0;
	// exclude the first and last two bytes in data
	for (int i = 2; i < (len - 2); i++)
	{
		ck_a += data[i];
		ck_b += ck_a;
	}
	data[len - 2] = ck_a;
	data[len - 1] = ck_b;    

    const int txBytes = uart_write_bytes(UART_NUM_2, data, len);
    ESP_LOGI("gps send", "Wrote %d bytes", txBytes);
    return txBytes;
}
void gpsHotReset(){

/*
 Notes about commands: the ublox device can be set to modes that optimize different things; 
 if it is known to be stationary, apparently time-keeping can be optimized.
 not doing that for now. I just wanted to get the firmware, and I want to force the GPS reset
 as well since I might not be able to cycle the power on some devices.
*/

// page 140 of https://www.u-blox.com/sites/default/files/products/documents/u-blox6_ReceiverDescrProtSpec_(GPS.G6-SW-10018)_Public.pdf
    // hotstart might be enough to get version info again? (YES!)
    // (8-bit math) CK_A CK_B cka = 6+4+0+4+0+0+0+0 ckb=6+(6+4)+(6+4+0)+(6+4+0+4)+(6+4+0+4+0)+(6+4+0+4+0+0)+(6+4+0+4+0+0+0)+(6+4+0+4+0+0+0+0)
/*
0x0000 Hotstart
0x0001 Warmstart (this is an "X2" field (page 85). I guess it would be '\x00','\x01', but not sure.)
0xFFFF Coldstart
*/        
    // the header sync and checksum could be kept in the sendData function...
    
    static char hotstartCommand[] = {'\xB5','\x62',/*header sync bytes*/
    '\x06','\x04',/*id==CFG-RST*/
    '\x04','\x00',/*length==4 bytes follow...*/
    '\x00','\x00',/*navBbrMask==hotstart*/
    '\x04'/*graceful watchdog*/,
    '\x00',/*reserved*/
    '\x00','\x00'}; /*checksum filled in by send*/

    sendData(sizeof hotstartCommand,hotstartCommand);
}

void gpsColdReset(){

    static char coldstartCommand[] = {'\xB5','\x62',/*header sync bytes*/
    '\x06','\x04',/*id==CFG-RST*/
    '\x04','\x00',/*length==4 bytes follow...*/
    '\xFF','\xFF',/*navBbrMask==coldstart*/
    '\x04'/*graceful watchdog*/,
    '\x00',/*reserved*/
    '\x00','\x00'}; /*checksum filled in by send*/

    sendData(sizeof coldstartCommand,coldstartCommand);
}

//static void tx_task(void *arg)
//{
//    static const char *TX_TASK_TAG = "TX_TASK";
//    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
//    while (1) {
//        sendData(TX_TASK_TAG, "Hello world");
//        vTaskDelay(2000 / portTICK_PERIOD_MS);
//    }
//}

    
void gpsparse(char *c){
//printf("%c",*c);
    // $GPRMC,160442.00,A,35xx.xxxxx,N,078xx.xxxxx,W,0.016,,250920,,,A*64
    // GMT time is 16:04:42 lat is 35 deg xx.xxxxx min North lon is 078 deg xx.xxxxx min West, speed is .016Knot, angle missing, date is 25sep2020
    // state machine is mostly waiting for "$", then "GPRMC,", time, AV, lat, NS, lon, EW, speed, angle, date

    // OK, I also want the GPGSV message to get the sat positions and snr.
    // now it is uglier. and uglier: add GPTXT
    
    static char name[8]; // important! this is common to RMC and GSV! and TXT!

    // GPTXT support
    static char txtNsentences[3];
    static char txtSentenceNo[3];
    static char txtTextIdentifier[3];
    static char txtMessage[72]; // maybe only 61?
    static char * const wordsGPTXT[] = {name,txtNsentences,txtSentenceNo,txtTextIdentifier,txtMessage};
    static const int lengthsGPTXT[] = { sizeof name-1,sizeof txtNsentences-1,sizeof txtSentenceNo-1,sizeof txtTextIdentifier-1,sizeof txtMessage-1};
    
    // GPRMC support
    static char gmtime[16];
    static char av[4];
    static char lat[16];
    static char ns[4];
    static char lon[16];
    static char ew[4];
    static char speed[16];
    static char angle[16];
    static char date[8];
    // for the GPRMC we dont collect the last few items
    static char * const wordsGPRMC[]={  name, gmtime, av, lat, ns, 
                                        lon, ew, speed, angle, date };
    static const int lengthsGPRMC[]={   sizeof name-1, sizeof gmtime-1, sizeof av-1, sizeof lat-1, sizeof ns-1, 
                                        sizeof lon-1, sizeof ew-1, sizeof speed-1, sizeof angle-1, sizeof date-1};

// $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
/*
    1    = Total number of messages of this type in this cycle
    2    = Message number
    3    = Total number of SVs in view
    4    = SV PRN number
    5    = Elevation in degrees, 90 maximum
    6    = Azimuth, degrees from true north, 000 to 359
    7    = SNR, 00-99 dB (null when not tracking)
    8-11 = Information about second SV, same as field 4-7
    12-15= Information about third SV, same as field 4-7
    16-19= Information about fourth SV, same as field 4-7
*/
    static char nMessages[2];
    static char iMessage[2];
    static char nSatInView[4];
//    static char satPRN[SatsPerGSV][6];
//    static char satElevation[SatsPerGSV][6];
//    static char satAzimuth[SatsPerGSV][6];
//    static char satSNR[SatsPerGSV][6];
    static struct SATRECORD sats[SatsPerGSV];
    // for the GPGSV all the items are needed, parse to the * in the checksum
    static char * const wordsGPGSV[]={  name,nMessages,iMessage,nSatInView,
                                        sats[0].prn,sats[0].el,sats[0].az,sats[0].snr,
                                        sats[1].prn,sats[1].el,sats[1].az,sats[1].snr,
                                        sats[2].prn,sats[2].el,sats[2].az,sats[2].snr,
                                        sats[3].prn,sats[3].el,sats[3].az,sats[3].snr};
    static const int lengthsGPGSV[]={   sizeof name-1, sizeof nMessages-1, sizeof iMessage-1, sizeof nSatInView-1, 
                                        sizeof sats[0].prn-1, sizeof sats[0].el-1, sizeof sats[0].az-1, sizeof sats[0].snr-1, 
                                        sizeof sats[0].prn-1, sizeof sats[0].el-1, sizeof sats[0].az-1, sizeof sats[0].snr-1, 
                                        sizeof sats[0].prn-1, sizeof sats[0].el-1, sizeof sats[0].az-1, sizeof sats[0].snr-1, 
                                        sizeof sats[0].prn-1, sizeof sats[0].el-1, sizeof sats[0].az-1, sizeof sats[0].snr-1
    };
    
    static char * const (*words) = wordsGPRMC; // this is for name to be parsed, which is
    static const int (*lengths) = lengthsGPRMC; // common to both nmea sentences. Once the
    static int dimwords = DIM(wordsGPRMC); // name is parsed, (state==0) choose RMC vs GSV
    
    static int state = -1;

    static uint64_t messageStart = -1;//esp_timer_get_time()

// debugging the stream of characters...
    static char rollingbuffer[sizeof(sClockState.gpsGPGLL)];
    static int iroll=0;
    if(iroll<sizeof(rollingbuffer)-1) {
        rollingbuffer[iroll++] = *c;
    }
    if ( *c == '$' ) { // force reset
        state=-1;
        //printf("                %s",rollingbuffer);
        if(iroll > 5)
            iroll -= 2; // newline and $ replaced with nul
        rollingbuffer[iroll-1]=0;
        rollingbuffer[iroll]=0;
        rollingbuffer[iroll+1]=0;
        rollingbuffer[iroll+2]=0;
        if(memcmp(rollingbuffer,"GPGLL",5)==0) {
            memcpy(sClockState.gpsGPGLL,rollingbuffer,iroll+1);//dst,src,len
           // DumpHex(sClockState.gpsGPGLL, 0, iroll+1);
        }
        else if(memcmp(rollingbuffer,"GPRMC",5)==0)
            memcpy(sClockState.gpsGPRMC,rollingbuffer,iroll+1);
        else if(memcmp(rollingbuffer,"GPVTG",5)==0)
            memcpy(sClockState.gpsGPVTG,rollingbuffer,iroll+1);
        else if(memcmp(rollingbuffer,"GPGGA",5)==0)
            memcpy(sClockState.gpsGPGGA,rollingbuffer,iroll+1);
        else if(memcmp(rollingbuffer,"GPGSA",5)==0)
            memcpy(sClockState.gpsGPGSA,rollingbuffer,iroll+1);
        iroll = 0;   
    }
    if (state == -1 ) {
        if ( *c == '$' ) {
            state += 1; // go to state 0, looking for name
            name[0] = 0; // common
            messageStart = esp_timer_get_time();
        }
    } else {
        if ( *c==',' || *c=='*') { // end of a field in a record (now need the * for GPGSV last item)
            // state==0, 'name', is common to both RMC and GSV and TXT
            if(state==0) { // end of name field, what name did we get?
                if(strcmp(name,"GPRMC")==0){ // the first field is the name. Do we want it?
                    words = wordsGPRMC;
                    lengths = lengthsGPRMC;
                    dimwords = DIM(wordsGPRMC);
                }
                else if (strcmp(name,"GPGSV")==0) {
                    words = wordsGPGSV;
                    lengths = lengthsGPGSV;
                    dimwords = DIM(wordsGPGSV);
                } 
                else if (strcmp(name,"GPTXT")==0) {
                    words = wordsGPTXT;
                    lengths = lengthsGPTXT;
                    dimwords = DIM(wordsGPTXT);
                } 
                else { // everything did not go ok...
                    state = -1; // go to state looking for $
                    return;
                }
                state = 1; // if everything goes ok, see else below.
                for(int i = state; i<dimwords; i+=1) { // start AFTER name
                    words[i][0]=0;
                }
            } else {
                if( words==wordsGPGSV && (*c=='*' || state == DIM(wordsGPGSV) - 1) ) { // here we are! got the GSV record
//printf("%s n=%s i=%s nview=%s prn=%s el=%s az=%s snr=%s prn=%s el=%s az=%s snr=%s prn=%s el=%s az=%s snr=%s prn=%s el=%s az=%s snr=%s\n", 
//name,nMessages,iMessage,nSatInView,
//sats[0].prn,sats[0].el,sats[0].az,sats[0].snr,
//sats[1].prn,sats[1].el,sats[1].az,sats[1].snr,
//sats[2].prn,sats[2].el,sats[2].az,sats[2].snr,
//sats[3].prn,sats[3].el,sats[3].az,sats[3].snr);
                    int idx = iMessage[0] - '1';// iMessage[0] is '1',...'4' (typ) maybe '5' if more than 16 sats
//#define SatsPerGSV 4
//#define MaxGSVRecs 5
                    if ( 0 <= idx && idx < MaxGSVRecs ) {
                        int idx4 = idx * SatsPerGSV; // 0,4,8,12,16
                        //
                        // this is kind of ugly. the 3 or 4 gps GSV records (might be 0..5?) arrive a character-at-a-time.
                        // the sClockState.satSemaphore should always be consistent, NOT updated as the individual pieces
                        // are received. (the status page is asynchronously using the data.)
                        // here's the local copy
                        //
                        // http://gauss.gge.unb.ca/papers.pdf/hetet.report.pdf is about the SNR values
                        //
                        static struct SATRECORD localSatRecords[DIM(sClockState.satRecords)];
                        //
                   //     bool gotit = xSemaphoreTake(sClockState.satSemaphore, 1000 / portTICK_RATE_MS/*portMAX_DELAY block forever*/ ) == pdTRUE;
                   //     if(!gotit) printf("clockGps.c did not get satSemaphore\n");

                        if(idx==0){ // when n shrinks, wipe trailing entries...brute force...
                            memset(localSatRecords,0,sizeof localSatRecords );
                        }
                        memcpy(&(localSatRecords[idx4]),sats,sizeof sats);
                   //     printf("gps prns ");
                   //     for(int i = 0; i<DIM(localSatRecords); i+=1){
                   //         printf("%s,",localSatRecords[i].prn);
                   //     }
                   //     printf("\n");
                        if(nMessages[0] == iMessage[0]){
                            memcpy(sClockState.satRecords,localSatRecords,sizeof localSatRecords);// copy all at once
                        }
                    //    xSemaphoreGive(sClockState.satSemaphore);
                        
                    }
//                    else {
//                        printf("gps idx=%d\n\n%s\n\n",idx,rollingbuffer);
//                    }
                    state = -1;
                }
                else if( words==wordsGPRMC && (*c=='*' || state == DIM(wordsGPRMC) - 1) ) { // here we are! got the RMC record
                    // store linux time for next PPS, 1 Sec from now
                    long d = atol(date);
                    gGPStimeinfo.tm_year = 100 + d % 100; d=d/100; // years since 1900
                    gGPStimeinfo.tm_mon = -1 + d % 100; d=d/100; // months since january
                    gGPStimeinfo.tm_mday = d % 100; d=d/100;
                    long t = atol(gmtime);
                    gGPStimeinfo.tm_sec = t % 100; t=t/100;
                    gGPStimeinfo.tm_min = t % 100; t=t/100;
                    gGPStimeinfo.tm_hour = t % 100; t=t/100;
                    gGPStimeinfo.tm_isdst = 0;
                    gGPStime_t = mktime(&gGPStimeinfo) - _timezone;
                    gGPStimeMessageReceived = messageStart; // esp_timer_get_time() rough indication of how old this data is
                    strlcpy(sClockState.gpstime,gmtime,sizeof(sClockState.gpstime));
                    strlcpy(sClockState.gpsdate,date,sizeof(sClockState.gpsdate));
                    // https://www.mkssoftware.com/docs/man5/tzname.5.asp - don't use the _daylight like that!
                    // https://stackoverflow.com/questions/9076494/how-to-convert-from-utc-to-local-time-in-c
                    if(  strcmp(sClockState.cfgTzone[0],"lookup") == 0  ){ // lookup based on GPS requested (vs fixed posix timezone rule)
                        sClockState.lat = GpsToDecimalDegrees(lat, ns[0]);
                        sClockState.lon = GpsToDecimalDegrees(lon, ew[0]);
                        char *tz=timezoneGet(sClockState.lat,sClockState.lon);
                        setenv("TZ", tz, 1);
                        tzset();
                    }
                   // printf("time=%ld GPRMC date=%ld seconds=%d\n",atol(gmtime),atol(date),gGPStimeinfo.tm_sec);
                    // success!
                    state = -1; // go to state looking for $
                } 
                else if( words==wordsGPTXT && (*c=='*' || state == DIM(wordsGPTXT) - 1) ) { // here we are! got the TXT record
                    // append txtMessage to sClockState.gpsGPTXT
                    char buffer[32];
                    snprintf(buffer,sizeof buffer,"%lld %s:",esp_timer_get_time(),txtTextIdentifier);
//                    strlcat(sClockState.gpsGPTXT,txtTextIdentifier,sizeof sClockState.gpsGPTXT); 
//                    strlcat(sClockState.gpsGPTXT,": ",sizeof sClockState.gpsGPTXT); 
                    strlcat(sClockState.gpsGPTXT,buffer,sizeof sClockState.gpsGPTXT); // strlcat(char *dst, const char *src, size_t size); 
                    strlcat(sClockState.gpsGPTXT,txtMessage,sizeof sClockState.gpsGPTXT); // strlcat(char *dst, const char *src, size_t size); 
                    strlcat(sClockState.gpsGPTXT,"\n",sizeof sClockState.gpsGPTXT); 
                    // success!
    /* typically... ~50 chars max in a line, 7 lines without distinction...
    $GPTXT,01,01,02,u-blox ag - www.u-blox.com*50
    $GPTXT,01,01,02,HW  UBX-G70xx   00070000 *77
    $GPTXT,01,01,02,ROM CORE 1.00 (59842) Jun 27 2012 17:43:52*59
    $GPTXT,01,01,02,PROTVER 14.00*1E
    $GPTXT,01,01,02,ANTSUPERV=AC SD PDoS SR*20
    $GPTXT,01,01,02,ANTSTATUS=OK*3B  <<< apparently this *could* say OPEN or SHORT, but I only see OK even with open.
    $GPTXT,01,01,02,LLC FFFFFFFF-FFFFFFFD-FFFFFFFF-FFFFFFFF-FFFFFFF9*53
    */
                    printf("%s\n",sClockState.gpsGPTXT);//testing
                    state = -1; // go to state looking for $
                } 
                else {
                    state += 1; // advance to capture next field
                    words[state][0]=0; // make sure next is null if there is no data
                }
            }
        } else { // not , or * end-of-field, add chars to buffer
            int xlen = strlen(words[state]); // append after current text
            if(xlen < lengths[state]){ // check for space. lengthsGPRMC is already -1 for nul
                words[state][xlen]=*c;
                words[state][xlen+1]=0; // always terminated
            }
            else {
//                printf("gps %s parm %d too long %s + %c\n\n'%s'\n\n", words[0], state, words[state], *c, rollingbuffer );
                state = -1; // something gone wrong, wont fit, return to hunting
            }
        }
    }
}

void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    char* data = (char*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_2, data, RX_BUF_SIZE, 10 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
//            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
//            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
            //printf("%s",data);
            for(int i=0;i<rxBytes;i+=1){
                gpsparse(data+i);
            }
        }
    }
    free(data);
}




/** https://stackoverflow.com/questions/36254363/how-to-convert-latitude-and-longitude-of-nmea-format-data-to-decimal
 * Convert NMEA absolute position to decimal degrees
 * "ddmm.mmmm" or "dddmm.mmmm" really is D+M/60,
 * then negated if quadrant is 'W' or 'S'
 */
double GpsToDecimalDegrees(const char *nmeaPos, char quadrant) // https://stackoverflow.com/users/2638860/mooncactus
{
  double v= 0;
  if(strlen(nmeaPos)>5)
  {
    char integerPart[3+1];
    int digitCount= (nmeaPos[4]=='.' ? 2 : 3);
    memcpy(integerPart, nmeaPos, digitCount);
    integerPart[digitCount]= 0;
    nmeaPos+= digitCount;
    v= atoi(integerPart) + atof(nmeaPos)/60.;
    if(quadrant=='W' || quadrant=='S')
      v= -v;
  }
  return v;
}



                    //printf("call timezoneGet here: %s\n", timezoneGet(sClockState.lat,sClockState.lon));
                    //printf("call timezoneGet vatican 370 (CET-1CEST,M3.5.0,M10.5.0/3): %s\n", timezoneGet(41.91,12.45));
                    //printf("call timezoneGet kansas right: %s\n", timezoneGet(39.9 , -101.9 ));
                    //printf("call timezoneGet kansas right: %s\n", timezoneGet(37.5 , -101.9 ));
                    //printf("call timezoneGet kansas left: %s\n", timezoneGet( 40.1, -101.9 ));
                    //printf("call timezoneGet kansas left: %s\n", timezoneGet( 38, -101.9 ));
                    //printf("call timezoneGet kansas left: %s\n", timezoneGet( 39, -101.9 ));
                    //printf("call timezoneGet kansas left: %s\n", timezoneGet( 39.9, -102.1));
                    //printf("call timezoneGet kansas left: %s\n", timezoneGet( 37.5, -102.1));
                    //printf("prev env var: timezone=%ld daylight=%d\n",_timezone,_daylight);
//tm local;
//if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) { // if reset or power loss
//  struct timeval val;
//  loadStruct(&local); // e.g. load time from eeprom
//  const time_t sec = mktime(&local); // make time_t
//  localtime(&sec); //set time
//} else {
//  getLocalTime(&local);
//}

//settimeofday(&tv, &tz)

//    printf("Setting time: %s", asctime(&tm));
//    struct timeval now = { .tv_sec = t };
//    settimeofday(&now, NULL);

//gettimeofday
//time
//asctime
//clock
//ctime
//difftime
//gmtime
//localtime
//mktime
//strftime
//adjtime*
//settimeofday()
//setenv() to set the TZ
//tzset()
