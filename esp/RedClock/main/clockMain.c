/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

#include "clockMain.h"

int sPPS = 0;
const int cPPSSET = 2;

struct CLOCKSTATE sClockState;

//extern int    daylight;//https://stackoverflow.com/questions/9076494/how-to-convert-from-utc-to-local-time-in-c
//extern long   timezone;
//extern char  *tzname[];

//static const char *TAG = "RedClock/clock main";






/*
(Arduino?)In super-brief: WiFiServer is effectively a socket server, not a web server.
WebServer is a full web server but can only handle one call at a time: even a post-back from the same client will cause problems.
AsyncWebServer is a great, full-featured web server that unfortunately suffers from heap corruption under load, so isn't ready for prime time.

(Espressif)
/home/c/esp/esp-idf/examples/protocols/http_server/simple/main          might use Access Point?
/home/c/esp/esp-idf/examples/protocols/http_server/restful_server       not AP
/home/c/esp/esp-idf/examples/protocols/http_server/file_serving         says "uses esp_http_server"
/home/c/esp/esp-idf/examples/protocols/esp_http_client

source for "example_connect()" -- /home/c/esp/esp-idf/examples/common_components/protocol_examples_common/connect.c
o    esp_wifi_init
    esp_netif_create_wifi (for sta not ap)
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL)
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL)
    esp_wifi_set_storage(WIFI_STORAGE_RAM)
o    esp_wifi_set_mode(WIFI_MODE_STA)
o    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config)
o    esp_wifi_start()
    esp_wifi_connect()

WIFI_MODE_AP or WIFI_MODE_APSTA

scanning for AP: /home/c/esp/esp-idf/examples/wifi
BAD actual AP example: /home/c/esp/esp-iot-solution/submodule/esp-idf/examples/wifi/getting_started/softAP/
x    nvs_flash_init()
x    tcpip_adapter_init()<<<<<<<<<<<<<<<<<<<<<<<<<<<don't uses the tcpip stuff -- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/tcpip_adapter_migration.html
x    esp_event_loop_init(event_handler, NULL)
x    esp_wifi_init(&cfg)
x    esp_wifi_set_mode(WIFI_MODE_AP)
x    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config)
x    esp_wifi_start()

GOOD /home/c/esp/esp-idf/examples/wifi/getting_started/softAP/main
...ESP_ERROR_CHECK(esp_netif_init());...etc

https://www.esp32.com/viewtopic.php?t=6851 --

solving ap+sta+server issues: https://www.esp32.com/viewtopic.php?f=2&t=6851&p=29854#p29854
"What is the netmask for the IP addresses in use on your local AP and your ESP32 SoftAP?
192.168.4.106 and 192.168.4.1 are usually private class C networks by default (ie 192.168.4.0/24, aka netmask 255.255.255.0). If either network has a netmask of this kind then it will be impossible for a single machine (the ESP32) to route packets to both of them.
Try changing the AP on the ESP32 to a different network, for example 192.168.5.0/24.
It's also worth checking for error results from calls to the netconn API, in case some operation is outright failing.
Angus
Franck
Posts: 7
Joined: Fri May 25, 2018 3:44 am
Re: HTTP server on AP and STA
Postby Franck » Wed Aug 29, 2018 1:38 pm
Hi Angus,
Thanks for the answer, that was the problem!
The AP netmask is 255.255.255.0.
Changing the AP address to 192.168.5.0/24 fixed the issue.
Also, to get it to work I have to use 2 tasks. One with netconn bound to the STA IP (192.168.4.106) and one to the softAP IP (192.168.5.1).
When I used netconn_bind(conn, IP_ADDR_ANY, 80), the server would work for the softAP as long as I don't query the STA server. Once the STA server has accepted a connection, the softAP server would not work anymore."

a project: https://github.com/lucadentella/esp32-tutorial/tree/master/23_ap_http with tutorial: http://www.lucadentella.it/en/2018/01/29/esp32-30-http-server-in-modalita-softap/

components/esp_netif/include/esp_netif_defaults.h contains #define ESP_NETIF_INHERENT_DEFAULT_WIFI_STA and ESP_NETIF_INHERENT_DEFAULT_WIFI_AP (no APSTA) used in ptoyocol examples connect.c


doc: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/wifi.html

*/




static void gpio_pps_handler(void *parm)
{
    // the GPS message *begins* when the pulse goes high
    sPPS = cPPSSET;// duration gpio_get_level(PPS)!=0;
}

static int sTransitionCount = 0;
// the sensor range is 0..4095. 
static void resetRequestDetector_task(void *arg) { // 10 second task at startup to detect waving at ambient sensor as a reset signal

    //HALLSENSOR gpio

    gpio_config_t hallconfig; // another way to do the loops below to init pins https://github.com/espressif/esp-idf/issues/285
    hallconfig.intr_type = GPIO_INTR_DISABLE;// GPIO_INTR_DISABLE GPIO_INTR_POSEDGE GPIO_INTR_NEGEDGE GPIO_INTR_ANYEDGE GPIO_INTR_LOW_LEVEL GPIO_INTR_HIGH_LEVEL
    hallconfig.mode = GPIO_MODE_INPUT;
    hallconfig.pin_bit_mask = (1ULL<<HALLSENSOR);
    hallconfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    hallconfig.pull_up_en = GPIO_PULLUP_DISABLE;
    esp_err_t error = gpio_config(&hallconfig);
    if(error!=ESP_OK){
        printf("error configuring HALLSENSOR input %d\n",error);
    }


    int wentHighTicks = 0;
    int wentLowTicks = 0;
    int ticks;
    double smallest = 4095;
    double biggest = 0;
    int n=0;
    while ( (ticks = esp_timer_get_time()) < 10*1000000 ) { // logic for reset, needed if password is lost...
        
        int hallv = gpio_get_level(HALLSENSOR);
//printf("%d\n",hallv);



    
    
//        int ambient = adc1_get_raw(AMBIENT);
        int ambient = hallv ? 4095 : 0; // quick and dirty...
        if ( ambient < smallest ) {
            smallest = ambient;
        }
        else {
            smallest = lerp(0.001,0.0,smallest,1.0,biggest);
        }
        if (ambient > biggest) {
            biggest = ambient;
        }
        else {
            biggest = lerp(0.999,0.0,smallest,1.0,biggest);
        }
        if ((n++) % 100 == 0) {
            printf("reset ambient time=%d n=%d c=%d s=%f a=%d b=%f\n",ticks,n++,sTransitionCount,smallest,ambient,biggest);
        }
        if (biggest - smallest > 100 && biggest/(smallest+1) > 2) { 
            double lowsense=lerp(0.3,0.0,smallest,1.0,biggest); // tune: .3 and .7 create the border between the
            double highsense=lerp(0.7,0.0,smallest,1.0,biggest); // tune                     low,dead,high bands
            if (ambient >= highsense) { // top quarter
                if (wentLowTicks) { // transition from low to high
                    if (ticks-wentLowTicks > 1000000 / 10) { // low lasted at least 1/10 second
                        sTransitionCount += 1;
                    }
                    wentLowTicks = 0;
                }
                if (wentHighTicks == 0) {
                    wentHighTicks = ticks;                
                }
            }
            else if (ambient < lowsense) { // bottom quarter
                if (wentHighTicks) { // transition from high to low
                    if (ticks-wentHighTicks > 1000000 / 10) { // high lasted at least 1/10 second
                        sTransitionCount += 1;
                    }
                    wentHighTicks = 0;
                }
                if (wentLowTicks == 0) {
                    wentLowTicks = ticks;                
                }
            }
            if (sTransitionCount > 3) {
                // use the server logic to reset saved parms to default and reboot
                // the two critical values are the passwords for BOTH the users router and RedClock's router
                // the user router password is exposed if you know where to look...
                resetNVSparms();
                esp_restart();
            }
        }
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}



static void seg7_task(void *arg) { // this exists just to get timer interrupt on core 1
    multiplexer_tg0_timer_init(TIMER_1, /*TEST_WITH_RELOAD,*/    TIMER_MULTIPLEX_SECONDS); // the clock display begins

//    // but while we are here, keep an eye on cpu busy...    
//    int64_t start;
//    while(1) {
//        start = esp_timer_get_time();
//        int64_t count = 0;
//        int64_t duration;
//        while( (duration = esp_timer_get_time() - start) < 100000) { // .1 seconds
//            count += 1;
//            vTaskDelay(1);// this is a prio 1 (low) task
//        }
//        printf("cpu %d available cycles/sec = %10.1f\n", xPortGetCoreID(), 1000000.0*(double)(count) / (double)(duration) );
//        vTaskDelay(1000 / portTICK_RATE_MS);
//    }
    
    // this is how a task is supposed to exit...
    vTaskDelete(NULL);
}


// the code that updates the factory app in place is in IRAM so it won't get the rug pulled out from under its feet.
// there should not be anything else running...I hope...
/***************** this never worked; the write *might* do something without the erase, but leaves FFFF after the erase.
void IRAM_ATTR upgrade( const unsigned char *from, int length, const esp_partition_t *factoryPartition ) {
return;

    rtc_wdt_protect_off();// not IRAM
    rtc_wdt_disable();// not IRAM  
    
    //esp_partition_erase_range(factoryPartition, 0, factoryPartition->size);
    esp_flash_erase_region(factoryPartition->flash_chip, factoryPartition->address + 0, factoryPartition->size);  // has IRAM_ATTR
    //esp_partition_write(factoryPartition, 0, from, length);
    esp_flash_write(factoryPartition->flash_chip, from, factoryPartition->address + 0, length);  // has IRAM_ATTR
    esp_restart(); // has IRAM_ATTR
}
********************/


void app_main(void)
{
    esp_err_t error;

    printf( KRED "\n\n"  // the UNICODE dots are 2 bytes each. 2 bytes * 5 rows * 8 chars * ~7 dots wide = 560 bytes, if I need some space back...
        "●●●●●   ●●●●●  ●●●●●    ●●●●  ●      ●●●●    ●●●●  ●   ●\n"
        "●    ●  ●      ●    ●  ●      ●     ●    ●  ●      ●  ● \n"
        "●●●●●   ●●●    ●    ●  ●      ●     ●    ●  ●      ●●●  \n"
        "●   ●   ●      ●    ●  ●      ●     ●    ●  ●      ●  ● \n"
        "●    ●  ●●●●●  ●●●●●    ●●●●  ●●●●●  ●●●●    ●●●●  ●   ●\n"
    "\n\n" KWHT);

    timezoneInit(); // memory map the lookup table into gTimeZoneData. this no longer needs to be this early, see OTA.

//    esp_image_header_t *newCandidate = (esp_image_header_t *)((char *)(gTimeZoneData)+sizeof(struct SMALLTIMEZONE));  
//
// this was an attempt to avoid proper OTA by staging the bin update through the timezone map data.
// this code might be useful for sha stuff... and peeking at the running bin...
// there is not room for three app partitions AND the timezone map, so there is no way to revert to factory.
//
/************************** NOTE: the "Factory" partition does NOT exist, just two "app" partitions.
    if(memcmp(gTimeZoneData->bd.text,"UTCEX\0",6)==0 && 256*1024 < gTimeZoneData->executableLength && gTimeZoneData->executableLength < 1024*1024) {
        printf("executable update has sig: bd=%s, exelen=%d\n", gTimeZoneData->bd.text, gTimeZoneData->executableLength);
        unsigned char sha[32];
        printf("appended begin\n");
        DumpHex(gTimeZoneData, sizeof(struct SMALLTIMEZONE), 256);
        printf("appended sha256\n");
        DumpHex((const unsigned char *)(gTimeZoneData),sizeof(struct SMALLTIMEZONE)+gTimeZoneData->executableLength-32,32);
        esp_sha(SHA2_256, 
            (const unsigned char *)(gTimeZoneData)+sizeof(struct SMALLTIMEZONE), // skip over dummy timezone to start of executable
            gTimeZoneData->executableLength-32, // leave out the 32 byte sha256 that should match 
            sha);
        printf("calculated sha256\n");
        DumpHex(sha,0,32);
        if (memcmp( (const unsigned char *)(gTimeZoneData) + sizeof(struct SMALLTIMEZONE) + gTimeZoneData->executableLength - 32,
                sha, 32)==0) { // good data
            printf("sha256 matches\n");
            //const esp_partition_t *running = esp_ota_get_running_partition();
            const esp_partition_t *running = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, "factory");
            if(running==NULL){
                printf("factory app partition not found\n");
            }
            else {
                esp_ota_get_partition_description(running, &sClockState.app_desc);

                // testing: dump the object code beginning from factory app

                const void *data;
                spi_flash_mmap_handle_t handle; // for the unmmap, which will never happen
                error = esp_partition_mmap(running, 0, running->size, SPI_FLASH_MMAP_DATA, (const void**)&data, &handle);
                if(error!=ESP_OK){
                    printf("error esp_partition_mmap %d\n",error);
                }
                printf("(old) factory app data (first 512 of %d) at %d on core %d\n",(int)(running->size),(int)(data), xPortGetCoreID());
                DumpHex(data,0, 512);
                printf("last part\n");
                DumpHex(data,991128,512);

                if(memcmp(data,(const unsigned char *)(gTimeZoneData)+sizeof(struct SMALLTIMEZONE),gTimeZoneData->executableLength)) {
                    printf("loading executable update to factory app\n");
                    upgrade( (const unsigned char *)(gTimeZoneData)+sizeof(struct SMALLTIMEZONE),
                        gTimeZoneData->executableLength, running );
                } // memcmp != 0 means update is not same as factory copy. this is how a second update is prevented...
                else {
                    printf("executable update same as factory app, please load a fresh timezone file to replace the update dummy file.\n");
                }
            }
        } // good data
        else {
            printf("bad sha256 on executable update\n");
        }
    } // h.n==1 means there is an update loaded
    else {
        printf("no executable update found\n");
    }
**********************/





    
    adc1_config_width(ADC_WIDTH_BIT_12); // reset task needs this configured already
    adc1_config_channel_atten(AMBIENT,ADC_ATTEN_DB_11);// up to 3.9V ?
    xTaskCreatePinnedToCore(resetRequestDetector_task, "reset_task", 1024*2, NULL, 5/*configMAX_PRIORITIES is 5*/, NULL, 1/*core*/);


    printf("main on core %d\n",xPortGetCoreID());
    printf ("sizeof time_t is: %d\n", sizeof(time_t));
    printf ("sizeof short is: %d\n", sizeof(short));
    printf ("sizeof int is: %d\n", sizeof(int));
    printf ("sizeof long is: %d\n", sizeof(long));
    printf ("sizeof long int is: %d\n", sizeof(long int));
    printf ("sizeof long long is: %d\n", sizeof(long long));
    printf ("portTICK_RATE_MS is: %d ms/tick\n", portTICK_RATE_MS);
    vTaskDelay(500 / portTICK_PERIOD_MS); // nice welcome message. 1/2 second or it goes by really fast.
    
#ifdef NDEBUG
printf("NDEBUG\n");
#else
printf("not NDEBUG\n");
#endif
/*
typedef struct {
    uint32_t magic_word;        //!< Magic word ESP_APP_DESC_MAGIC_WORD
    uint32_t secure_version;    //!< Secure version
    uint32_t reserv1[2];        //!< reserv1
    char version[32];           //!< Application version
    char project_name[32];      //!< Project name
    char time[16];              //!< Compile time
    char date[16];              //!< Compile date
    char idf_ver[32];           //!< Version IDF
    uint8_t app_elf_sha256[32]; //!< sha256 of elf file
    uint32_t reserv2[20];       //!< reserv2
} esp_app_desc_t;
*/

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_get_partition_description(running, &sClockState.app_desc);


    printf("version %s \nproject_name %s time %s date %s \nidf_ver %s\n",
        sClockState.app_desc.version,sClockState.app_desc.project_name,
        sClockState.app_desc.time,sClockState.app_desc.date,
        sClockState.app_desc.idf_ver);

    esp_ota_img_states_t otaState;
    error = esp_ota_get_state_partition(running, &otaState);
    printf("esp_ota_get_state_partition=%d\n",error);
//typedef enum {
//    ESP_OTA_IMG_NEW             = 0x0U,         /*!< Monitor the first boot. In bootloader this state is changed to ESP_OTA_IMG_PENDING_VERIFY. */
//    ESP_OTA_IMG_PENDING_VERIFY  = 0x1U,         /*!< First boot for this app was. If while the second boot this state is then it will be changed to ABORTED. */
//    ESP_OTA_IMG_VALID           = 0x2U,         /*!< App was confirmed as workable. App can boot and work without limits. */
//    ESP_OTA_IMG_INVALID         = 0x3U,         /*!< App was confirmed as non-workable. This app will not selected to boot at all. */
//    ESP_OTA_IMG_ABORTED         = 0x4U,         /*!< App could not confirm the workable or non-workable. In bootloader IMG_PENDING_VERIFY state will be changed to IMG_ABORTED. This app will not selected to boot at all. */
//    ESP_OTA_IMG_UNDEFINED       = 0xFFFFFFFFU,  /*!< Undefined. App can boot and work without limits. */
//} esp_ota_img_states_t;
    static const char sOta[][8] = {"new","pendver","valid","invalid","aborted"};
    printf("partition state: %s\n", otaState==0xFFFFFFFFU ? "undefined" : sOta[otaState] );
        
    uint8_t temp[8];
    const char *alphabet = "bcdghjknrstvwxyz";
    ESP_ERROR_CHECK( esp_efuse_mac_get_default(temp) );// returns 6 bytes
    snprintf(sClockState.baseMacAddress,sizeof(sClockState.baseMacAddress),
        "%02X:%02X:%02X:%02X:%02X:%02X",
        temp[0],temp[1],temp[2],temp[3],temp[4],temp[5]);
    printf("base mac addr=%s\n",sClockState.baseMacAddress);
//    int nbits = ((temp[0]+3)*(temp[1]+19) + (temp[2]+11)*(temp[3]+13) + (temp[4]+17)*(temp[5]+7)) % 257; // seed from all bytes of MAC
    sClockState.serialnumber[0] = 'R'; // Red
    sClockState.serialnumber[1] = 'C'; // Clock
    
    
    unsigned char sha[32];
    esp_sha(SHA2_256, temp, 6, sha); // replace clumsy homebrew hash with proper hash
    
    
//printf("nbits=%08x\n",nbits);
    for(int i = 0; i < 6; i+=1){
//        nbits += temp[i];
//printf("byte=%02x nbits=%08x\n",temp[i],nbits);
//        sClockState.serialnumber[i+2] = alphabet[nbits&15];
        sClockState.serialnumber[i+2] = alphabet[sha[i]&15];
    }
    sClockState.serialnumber[8] = 0;
    printf ("serial number: %s\n", sClockState.serialnumber);
    
    /*
    chip_info.model is esp_chip_model_t
    CHIP_ESP32  = 1, //!< ESP32
    CHIP_ESP32S2 = 2, //!< ESP32-S2
    CHIP_ESP32S3 = 4, //!< ESP32-S3
    CHIP_ESP32C3 = 5, //!< ESP32-C3    
    */
    static char model[][8]={
    "0?",
    "ESP32",    // = 1, //!< ESP32   <<<<<<<< developed on this one, revision 1
    "ESP32S2",  // = 2, //!< ESP32-S2
    "3?",
    "ESP32S3",  // = 4, //!< ESP32-S3
    "ESP32C3",  // = 5, //!< ESP32-C3    
    "6?",
    "7?",
    "8?",
    "9?",
    "10?"
    };
    esp_chip_info(&sClockState.chip_info);
    sClockState.chip_infoModel =  model[sClockState.chip_info.model]; // mostly because the enum is hard to use later
    
    rtc_clk_cpu_freq_get_config(&sClockState.rcf);
    sClockState.xtalfreq = (int)rtc_clk_xtal_freq_get();// 40
    
    sClockState.tickSecondOfDefinitiveTime = esp_timer_get_time()/1000000 - 24*3600; // last init was yesterday
    sClockState.tickSecondOfDefinitiveTimeGPS = sClockState.tickSecondOfDefinitiveTime;
    sClockState.tickSecondOfDefinitiveTimeSNTP = sClockState.tickSecondOfDefinitiveTime;
    /* Initialize file storage */
    ESP_ERROR_CHECK(init_spiffs());
    ESP_ERROR_CHECK( nvs_flash_init() ); // <<<<<<<<<<<<<<<<<<<<<<

    // defaults are defined right here...then copied to current. Then override with default file.
    
    // these need to be allocated so they can be freed when replaced
    // default lives in [1], a replaceable malloc version is in [0]
    
    sClockState.cfgWiFi[0]=strdup(sClockState.cfgWiFi[1]="off");
    sClockState.cfgWiFiName[0]=strdup(sClockState.cfgWiFiName[1]="WIFI router name");
    sClockState.cfgWiFiPass[0]=strdup(sClockState.cfgWiFiPass[1]="router pw");

    sClockState.cfgMyFiName[0]=strdup(sClockState.cfgMyFiName[1]=sClockState.serialnumber);
    sClockState.cfgMyFiPass[0]=strdup(sClockState.cfgMyFiPass[1]="88888888");

    sClockState.cfgBrthigh[0]=strdup(sClockState.cfgBrthigh[1]="7000"); // these are the defaults to the linear center section
    sClockState.cfgBrtlow[0]=strdup(sClockState.cfgBrtlow[1]="3000"); // of the 0..10,000 range
    sClockState.cfgBrtdawn[0]=strdup(sClockState.cfgBrtdawn[1]="0700");
    sClockState.cfgBrtdusk[0]=strdup(sClockState.cfgBrtdusk[1]="1900");
    sClockState.cfgSntpUrl[0]=strdup(sClockState.cfgSntpUrl[1]="pool.ntp.org");
    sClockState.cfgTzrule[0]=strdup(sClockState.cfgTzrule[1]="EST5EDT,M3.2.0,M11.1.0");
    sClockState.cfgBright[0]=strdup(sClockState.cfgBright[1]="sense");
    sClockState.cfgClock[0]=strdup(sClockState.cfgClock[1]="sntp");
    sClockState.cfgTzone[0]=strdup(sClockState.cfgTzone[1]="lookup");
    sClockState.cfgDisplay[0]=strdup(sClockState.cfgDisplay[1]="mil");// vs "civ"
    sClockState.cfgNeighborHoodWatch[0]=strdup(sClockState.cfgNeighborHoodWatch[1]="2");
    sClockState.cfgLocationNote[0]=strdup(sClockState.cfgLocationNote[1]="location name goes here");
    
    // override the defaults above with values previously saved to non-volatile storage
    loadNVSparms(); // lives in the clockServer because that's where the names and pointers above are connected in a static structure

    bool useWIFI = strcmp(sClockState.cfgWiFi[0],"off") != 0; // this really means use STATION mode. always use ACCESS POINT mode.

//    esp_err_t error;

    setenv("TZ", sClockState.cfgTzrule[0], 1);
    tzset();
    
    //sClockState.satSemaphore = xSemaphoreCreateMutex();
//    timezoneInit(); // memory map the lookup table

    uart_init(); // listen to the GPS
    xTaskCreatePinnedToCore(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL, 0/*core*/);
//    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
//later...    gpsHotReset();

    sClockState.smoothAmbient = 2000;

    gpio_config_t ppsconfig; // another way to do the loops below to init pins https://github.com/espressif/esp-idf/issues/285
    ppsconfig.intr_type = GPIO_INTR_POSEDGE;// GPIO_INTR_DISABLE GPIO_INTR_POSEDGE GPIO_INTR_NEGEDGE GPIO_INTR_ANYEDGE GPIO_INTR_LOW_LEVEL GPIO_INTR_HIGH_LEVEL
    ppsconfig.mode = GPIO_MODE_INPUT;
    ppsconfig.pin_bit_mask = (1ULL<<PPS);
    ppsconfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ppsconfig.pull_up_en = GPIO_PULLUP_DISABLE;
    error = gpio_config(&ppsconfig);
    if(error!=ESP_OK){
        printf("error configuring inputs %d\n",error);
    }
    //to turn on the output 18: gpio_set_level(18, 1);
    //to detect the input 15:int level=gpio_get_level(15);
    error = gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    if(error!=ESP_OK){
        printf("error installing isr service %d\n",error);
    }
    error = gpio_isr_handler_add(PPS,gpio_pps_handler,(void*)0);
    if(error!=ESP_OK){
        printf("error adding pps handler %d\n",error);
    }
    printf("good 1\n");




    // segment and digit gpio initialization
    for(int iseg=0;iseg<DIM(segs);iseg+=1){
        gpio_reset_pin(segs[iseg]);
        gpio_set_direction(segs[iseg], GPIO_MODE_OUTPUT);
        gpio_set_level(segs[iseg], SEGOFF);
    }
    for(int idig=0;idig<DIM(digs);idig+=1){
        gpio_reset_pin(digs[idig]);
        gpio_set_direction(digs[idig], GPIO_MODE_OUTPUT);
        gpio_set_level(digs[idig], DIGOFF);
    }

    // start the 7-seg interrupt. I'm trying to give it core 1, all else on core 0.
    // the interrupt can run a long loop (bright) or a short loop (dim) to switch the
    // displays off early or late to control brightness. If the core is used by
    // other tasks, they may hit the watchdog timer when the display is bright.
    //
    // If I cared about power consumption, I'd figure out how to make a second interrupt
    // after a few nanoseconds instead of a hard loop.
    say4(sClockState.cfgMyFiName[0],0);
//    say4("stas",(sTransitionCount&1)*1000);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    xTaskCreatePinnedToCore(seg7_task, "seg7_task", 1024*2, NULL, 5/*configMAX_PRIORITIES is 5*/, NULL, 1/*core*/);
    
//    say4("ncic",(sTransitionCount&1)*1000);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(esp_netif_init()); // <<<<<<<<<<<<<<<<<<<<<<

//    say4("etlp",(sTransitionCount&1)*1000);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK( esp_event_loop_create_default() ); // <<<<<<<<<<<<<<<<<<<<<<

//    say4("spff",(sTransitionCount&1)*1000);// just 4th decimal
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(start_file_server("/spiffs"));


    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
//    say4("conn",(sTransitionCount&1)*1000);// just 4th decimal
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(my_connect()); // <<<<<<<<<<<<<<<<<<<<<< derived from /home/c/esp/esp-idf/examples/common_components/protocol_examples_common/connect.c


/*

// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/tcpip_adapter_migration.html
// says https://github.com/espressif/esp-idf/blob/526f682/examples/wifi/getting_started/softAP/main/softap_example_main.c is a better example
// wifi_init_softap()
     ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);


*/



//    time_t now;
//    struct tm timeinfo;
//    time(&now);
//    localtime_r(&now, &timeinfo);
//    // Is time set? If not, tm_year will be (1970 - 1900).
//    if (timeinfo.tm_year < (2016 - 1900)) {
//        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
    if(useWIFI) {
//        say4("sttp",(sTransitionCount&1)*1000);// just 1st decimal
        vTaskDelay(100 / portTICK_PERIOD_MS);
        obtain_time();
    }
        // update 'now' variable with current time
//        time(&now); // messed up across the obtain_time function???
//    }


//    char strftime_buf[64];

    // Set timezone to China Standard Time
//    setenv("TZ", "CST-8", 1);
//    tzset();
//    localtime_r(&now, &timeinfo);
//    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
//    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);

    // Set timezone to Eastern Standard Time and print local time
//printf("before timezone=%ld daylight=%d\n",_timezone,_daylight);

//    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
//    tzset();
//printf("after timezone=%ld daylight=%d\n",_timezone,_daylight);

//    localtime_r(&now, &timeinfo);
//    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
//    ESP_LOGI(TAG, "The current date/time in New York is: %s", strftime_buf);

//    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) {
//        struct timeval outdelta;
//        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {
//            adjtime(NULL, &outdelta);
//            ESP_LOGI(TAG, "Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us",
//                        (long)outdelta.tv_sec,
//                        outdelta.tv_usec/1000,
//                        outdelta.tv_usec%1000);
//            vTaskDelay(2000 / portTICK_PERIOD_MS);
//        }
//    }

    if (sTransitionCount>=2){ // three transitions is unlikely if you are not trying...
        // it is not critical to wait here, but it does allow the decimals to show the count. only about 1 second before this, need 9 more
        for(int i = 0; i < 90; i+=1) {
            say4("rrrr",(sTransitionCount&1)*1000);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }

//    say4("ltas",0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    xTaskCreatePinnedToCore(timer_clock_task, // in clock7seg.c, start this after the 10-step startup display. this takes over the display by jamming the time in.
        "MultiplexerTask", // debug name
        1024*2, // testing bigger 2048, // stack size
        NULL, // parms
        5, // prio
        NULL, // return handle
        1 /*core*/
    );

    gpsHotReset();// to regenerate the version data

    my_bme280_test();

    if(otaState==ESP_OTA_IMG_PENDING_VERIFY){ // unclear if this needlessly rewrites spi memory.
        error = esp_ota_mark_app_valid_cancel_rollback(); // a lot of stuff just worked, must be OK?
        printf("esp_ota_mark_app_valid_cancel_rollback=%d\n",error);
    }
// apparently OK to fall off the end; tasks are left running (yes: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/general-notes.html)
}

