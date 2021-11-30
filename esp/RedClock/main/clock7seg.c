/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/


#include "clockMain.h"

const DRAM_ATTR unsigned char segs[NSEGMENTS] = {Aseg,Bseg,Cseg,Dseg,Eseg,Fseg,Gseg,Pseg};
const DRAM_ATTR unsigned char digs[NDIGITS] = {Hten,Hone,Mten,Mone};

const DRAM_ATTR unsigned char ascii[256] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,134,34,118,73,83,117,32,57,15,54,73,136,64,DECIMAL,82,
63,6,91,79,102,109,125,7,127,111,9,137,24,72,12,75,
220,119,124,88,94,121,113,103,116,4,12,112,56,85,84,92,
115,231,80,237,120,28,42,62,54,42,75,49,100,14,33,8,
0,119,124,88,94,121,113,103,116,4,12,112,56,85,84,92,
115,231,80,237,120,28,42,62,54,42,75,57,48,15,82,127,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

DRAM_ATTR unsigned char vals[NDIGITS] = {
ascii['8']+DECIMAL,
ascii['8']+DECIMAL,
ascii['8']+DECIMAL,
ascii['8']+DECIMAL
};



/*
 * Timer group0 ISR handler
 *
 * Note:
 * We don't call the timer API here because they are not declared with IRAM_ATTR.
 * If we're okay with the timer irq not being serviced while SPI flash cache is disabled,
 * we can allocate this interrupt without the ESP_INTR_FLAG_IRAM flag and use the normal API.
 */

//   /home/c/esp/esp-idf/components/soc/soc/esp32/include/soc/soc.h has #define #define REG_SET_BIT(_r, _b) which does not understand 0..31 vs 32..??
//    has gpio_set_level which understands 0..40?? but is not IRAM (either case below)
//    THERE ARE TWO VERSIONS of gpio.c ! Written very differently ! 
//    /home/c/esp/esp-iot-solution/submodule/esp-idf/components/driver/gpio.c
//    /home/c/esp/esp-idf/components/driver/gpio.c
//    the iot bersion uses the registers, the components version uses hal

/* the iot version */
//#define GPIO_CHECK(a, str, ret_val) 
//    if (!(a)) { 
//        ESP_LOGE("MYgpio_set_level","%s(%d): %s", __FUNCTION__, __LINE__, str); 
//        return (ret_val); 
//    }

esp_err_t IRAM_ATTR MYgpio_set_level(gpio_num_t gpio_num, uint32_t level)
{
   // GPIO_CHECK(GPIO_IS_VALID_OUTPUT_GPIO(gpio_num), "GPIO output gpio_num error", ESP_ERR_INVALID_ARG);
    if (level) {
        if (gpio_num < 32) {
            GPIO.out_w1ts = (1 << gpio_num);
        } else {
            GPIO.out1_w1ts.data = (1 << (gpio_num - 32));
        }
    } else {
        if (gpio_num < 32) {
            GPIO.out_w1tc = (1 << gpio_num);
        } else {
            GPIO.out1_w1tc.data = (1 << (gpio_num - 32));
        }
    }
    return ESP_OK;
}
// pre-package the sets and clears for out and out1 ... for speed
static uint32_t DRAM_ATTR sLoSet[NDIGITS]={0};
static uint32_t DRAM_ATTR sHiSet[NDIGITS]={0};
static uint32_t DRAM_ATTR sLoClear[NDIGITS]={0};
static uint32_t DRAM_ATTR sHiClear[NDIGITS]={0};
static uint32_t  sLoSet_=0;
static uint32_t  sHiSet_=0;
static uint32_t  sLoClear_=0;
static uint32_t  sHiClear_=0;
void makegpio(int idig,gpio_num_t gpio_num, uint32_t level) {
    if (level) {
        if (gpio_num < 32) {
            sLoSet_ |= (1 << gpio_num);
        } else {
            sHiSet_ |= (1 << (gpio_num - 32));
        }
    } else {
        if (gpio_num < 32) {
            sLoClear_ |= (1 << gpio_num);
        } else {
            sHiClear_ |= (1 << (gpio_num - 32));
        }
    }
}

/* end MYgpio_set_level */

static int DRAM_ATTR sError = 0;


/*
uint64_t timer_group_get_counter_value_in_isr(timer_group_t group_num, timer_idx_t timer_num);
void timer_group_set_alarm_value_in_isr(timer_group_t group_num, timer_idx_t timer_num, uint64_t alarm_val);
timer_group_set_counter_enable_in_isr(timer_group_tgroup_num, timer_idx_ttimer_num, timer_start_tcounter_en)
instead of hard loop, set a future time for the alarm to turn off the display...and future to advance...

on and off delays must be at least 256.
on+off should be small. around 25600. 256000 max.
ratio from dark=256/256000 to lite=256000/256
256/25600 . 257/25599 . 258/25598 . . . 25599/257 . 25600/256 . 25601/256 . 25602/256 . 256000/256

*/

#define MINDELAY 300
static unsigned int sOnDelay = MINDELAY; // 291 (160mHz) or 160 (240mHz) is about as low as these can go
static unsigned int sOffDelay = MINDELAY; // ditto.  256000 is on the flickering edge, acceptable for off.


static void IRAM_ATTR timer_group0_Multiplexer_isr(void *para)
{
    static DRAM_ATTR unsigned int idig = 0;
    static DRAM_ATTR bool displayIsOn = false; // when the timer alerts, this indicates the state of the display

    unsigned int delay;
    int timer_idx = (int) para;
    
    if (displayIsOn) {
        MYgpio_set_level(digs[(idig)&3], DIGOFF); // off as early as possible for a very dark display
        delay = sOffDelay;
    }
    else {
        //MYgpio_set_level(digs[(idig)&3], DIGOFF);
        idig=(idig+1)&(NDIGITS-1);
//        int x=vals[idig];
//        for(int j=0;j<NSEGMENTS;j+=1){
//            MYgpio_set_level(segs[j], x&1 ? SEGON:SEGOFF);
//            x=x>>1;
//        }
        GPIO.out_w1ts = sLoSet[idig];
        GPIO.out1_w1ts.data = sHiSet[idig];
        GPIO.out_w1tc = sLoClear[idig];
        GPIO.out1_w1tc.data = sHiClear[idig];
////        
        delay = sOnDelay;
        MYgpio_set_level(digs[idig], DIGON); // on as late as possible for a very dark display
    }

    displayIsOn = ! displayIsOn;
    

    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, timer_idx);

    //   After the alarm has been triggered
    //    we need enable it again, so it is triggered the next time 
    // but first, move the alarm ahead of the counter
    //
    // the following uses about 256 timer counts
    //
    uint64_t before = timer_group_get_counter_value_in_isr(TIMER_GROUP_0, timer_idx);
    timer_group_set_alarm_value_in_isr(TIMER_GROUP_0, timer_idx, before+delay);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, timer_idx);
#if 1
    uint64_t after = timer_group_get_counter_value_in_isr(TIMER_GROUP_0, timer_idx);
    if ( after-before >= delay ) {
        sError = after-before;
    }
#endif
}


/*
 * Initialize selected timer of the timer group 0
 *
 * timer_idx - the timer number to initialize
 * auto_reload - should the timer auto reload on alarm?
 * timer_interval_sec - the interval of alarm to set
 */
void multiplexer_tg0_timer_init(int timer_idx,/* bool auto_reload,*/ double timer_interval_sec) /*.00008 results in 400 ticks with divider==16*/
{
    printf("timer divider=%d\n",TIMER_DIVIDER);
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = 0,//auto_reload, NO. we'll set the alarm out in front of the counter...
    }; // default clock source is APB
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);// TIMER_BASE_CLK=80e6 / TIMER_DIVIDER==16
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_Multiplexer_isr,
                       (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}

// TASK: format the time, over and over...

void timer_clock_task(void *arg)
{
    while (1) {

if(sError) {printf("sError %d on=%d off=%d\n",sError,sClockState.onDelay,sClockState.offDelay);sError=0;}

        time_t now;
        time(&now); // TZ was set in main, once
        struct tm timeinfo = { 0 };
        localtime_r(&now, &timeinfo); // _r is the threadsafe version, now, a time_t, unpacked into timeinfo, a tm
        char strftime_buf[8];
        // %k,%l use leading space for hour; %H,%I use leading zero, %k, %H for 24 hour %l, %I for 12 hour
        // After using the leading zero for a while, I really, really don't like it. It is esp bad at 00:10 in the early AM, 0:10 is easier.
        // yes, it could be an option. not for now.
        strftime(strftime_buf, sizeof(strftime_buf), strcmp("mil",sClockState.cfgDisplay[0])==0 ? "%k%M%S" : "%l%M%S", &timeinfo); 

        uint64_t secondsSinceDefinitiveTime = esp_timer_get_time()/1000000 - sClockState.tickSecondOfDefinitiveTime ;
        // sntp is every 2 hours. after 8 hours, show the alarm, 1 second on, one second off
        int alarm = 0;
        if ( secondsSinceDefinitiveTime > 8*3600 ){ // this should be > 3*3600
            if(timeinfo.tm_sec%2){
                alarm += 10; // 2nd decimal from right
            }
            else{
                alarm += 1000; // 4th decimal from right
            }
        }

        say4(strftime_buf,100+(sPPS>0)+alarm); // 100 is the : position, sPPS>0 is the right-most flicker for GPS good

        double hi = atoi(sClockState.cfgBrthigh[0]); // typ 1000
        double lo = atoi(sClockState.cfgBrtlow[0]); // typ 10

        // rules...
        int change = adc1_get_raw(AMBIENT);
        sClockState.smoothTrueAmbient = (sClockState.smoothTrueAmbient*199.0 + change)/200.0;
        if ( strcmp(sClockState.cfgBright[0],"sense")==0) {
            // change already set for the sense case
        } else if ( strcmp(sClockState.cfgBright[0],"time")==0) {
            if (strcmp(sClockState.cfgBrtdawn[0],sClockState.cfgBrtdusk[0]) < 0 ){ // normal: dawn<dusk
                strftime(strftime_buf, sizeof(strftime_buf), "%H%M%S", &timeinfo); // force 24 hour
                if(strcmp(sClockState.cfgBrtdawn[0],strftime_buf)<=0 && strcmp(strftime_buf,sClockState.cfgBrtdusk[0])<0){
                    change=4095;
                }
                else {
                    change=0;
                }
            }
            else { // if dusk <= dawn, say 1AM is dusk, and 6AM is dawn, still needs to work
                if(strcmp(sClockState.cfgBrtdusk[0],strftime_buf)<=0 && strcmp(strftime_buf,sClockState.cfgBrtdawn[0])<0){
                    change=0;
                }
                else {
                    change=4095;
                }
            }
        } else if ( strcmp(sClockState.cfgBright[0],"max")==0) {
            change = 4095; // these switch instantly
        } else if ( strcmp(sClockState.cfgBright[0],"min")==0) {
            change = 0; // these switch instantly
        }
        sClockState.smoothAmbient = (sClockState.smoothAmbient*99.0 + change)/100.0; // these switch smoothly, but faster
//        double delta = fabs(sClockState.smoothTrueAmbient - change);
  //      double ratio = lerp(delta,0.0,1,4095.0,4500 );// ignore big delta, mostly (pps hardware bug pulses ambient on xx cpu?)
    //    sClockState.smoothAmbient = (sClockState.smoothAmbient*ratio + change)/(ratio+1.0); // these switch smoothly, but faster
        int nom = (int)lerp((sClockState.smoothAmbient /* *sClockState.smoothAmbient */), 0, lo, (4095.0 /* *4095.0*/), hi); // see the mux code above
        
        if(nom<3000) { // dark, 0 to 3000
            sOnDelay = MINDELAY;
            sOffDelay = lerp(nom, 0.0, MINDELAY*1000,3000,MINDELAY*100 ); // flicker at *1000 is hard to see because it is dim
        }
        else if (nom<7000) { // midrange, 3000 to 7000
            int on = lerp(nom, 3000.0, MINDELAY, 7000, MINDELAY*100 );
            int off = lerp(nom, 3000.0, MINDELAY*100, 7000, MINDELAY );
            int small = on<off?on:off;
            double scale = (double)(small) / (double)(MINDELAY);
            
            sOnDelay = (int)(.5 + on / scale);
            sOffDelay = (int)(.5 + off / scale);
        }
        else { // light, up to 10000
            sOnDelay = lerp(nom, 7000.0, MINDELAY*100, 10000, MINDELAY*500 ); // stop at *500, bright flicker is easy to see
            sOffDelay = MINDELAY;
        }
        sClockState.onDelay = sOnDelay;
        sClockState.offDelay = sOffDelay;
        //if(sDelay>640) sDelay=640;
        
        //printf("sOnDelay=%d sOffDelay=%d\n",sOnDelay,sOffDelay);

if(xPortGetCoreID()!=1)  printf("7seg:%d\n",xPortGetCoreID());

        if(sPPS>0){
            if (sPPS == cPPSSET && gGPStime_t != 0){
              //  printf("tick %s\n",strftime_buf);
                // if the SNTP is older than 3 hours OR if GPS is primary, use this GPS time
                // (elsewhere, in clockSntp.c, if/when sntp updates, take it always. It might
                // be all we have, or GPS may (here) override it if GPS is primary or sntp is
                // stale.
                if ( 1 || // ALWAYS use the GPS, immediately, if the PPS arrives
                        strcmp(sClockState.cfgClock[0],"gps")==0 || 
                        esp_timer_get_time()/1000000 - sClockState.tickSecondOfDefinitiveTimeSNTP > 3*3600 ) { // this should be < 8*3600 (alarm)
                        
                    uint64_t ageOfMessage = esp_timer_get_time() - gGPStimeMessageReceived; // should be close to 1,000,000 (1 second)
                    int seconds = 1;
                    if ( !((500000 < ageOfMessage) && (ageOfMessage < 1000000)) ){ // typ: 700,000 to 900,000
                        seconds = (ageOfMessage + 500000) / 1000000;// round? probably should floor or ceil?
                        printf("clock7seg.c GPS timing logic error: ageOfMessage=%lld, seconds=%d\n",ageOfMessage,seconds);
                    }
                    else {
                    
                        // does setting the time every tick cause an issue? let's only set if off by > 10/1000 second...


                        struct timeval sTime_now;
                        gettimeofday(&sTime_now, NULL);
                        long long clock_msecs_time = (sTime_now.tv_sec * 1000LL) + (sTime_now.tv_usec / 1000LL);
                    
                        gGPStime_t += seconds; // this was the time of the previous pulse
                        struct timeval t;
                        t.tv_sec = gGPStime_t;
                        t.tv_usec = 0;
                        long long gps_msecs_time =  (t.tv_sec * 1000LL) + (t.tv_usec / 1000LL);
                        if(abs(gps_msecs_time - clock_msecs_time) > 10) { // 10 ms
                            settimeofday(&t,NULL);
                            printf("tick used, gps=%lld clock=%lld  diff=%lld\n",gps_msecs_time,clock_msecs_time,gps_msecs_time - clock_msecs_time);
                        } else {
                          //  printf("tick not used, gps=%lld clock=%lld  diff=%lld\n",gps_msecs_time,clock_msecs_time,gps_msecs_time - clock_msecs_time);
                        }
                     //   printf("%ld\n",gGPStime_t);
                    }
                } 
                gGPStime_t = 0;
                // we've got a definitive pulse AND a time
                sClockState.tickSecondOfDefinitiveTime = esp_timer_get_time()/1000000; // GPS/SNTP set this to indicate valid time
                sClockState.tickSecondOfDefinitiveTimeGPS = sClockState.tickSecondOfDefinitiveTime;
            }
            sPPS-=1; // clear, slowly
        }
        if ( gHTTPtime_t!= 0 ) { 
            int age = esp_timer_get_time()/1000000 - sClockState.tickSecondOfDefinitiveTime;
            if (age > 5*3600 ) {
                // it has been a long time since a GPS or SNTP update.
                // GPS happens above, SNTP is asynchronous notify in Sntp.c.
                // Here we notice a pending Date: header from Connect.c .  Either this is very early
                // in startup or GPS is unavailable and SNTP has missed a couple of 2-hour cycles, so
                // decide to take this update.

                time_t now;
                time(&now); // TZ was set in main, once
                struct tm timeinfo = { 0 };
                localtime_r(&now, &timeinfo); // _r is the threadsafe version, now, a time_t, unpacked into timeinfo, a tm
                char oldtime[64];
                strftime(oldtime, sizeof(oldtime), "%a, %d %b %Y %H:%M:%S", &timeinfo); // requires a tm, not a time_t
                                    
                struct timeval t;
                t.tv_sec = gHTTPtime_t;
                t.tv_usec = 0;
                settimeofday(&t,NULL);                
                
                time(&now); // TZ was set in main, once
                localtime_r(&now, &timeinfo); // _r is the threadsafe version, now, a time_t, unpacked into timeinfo, a tm
                char newtime[64];
                strftime(newtime, sizeof(newtime), "%a, %d %b %Y %H:%M:%S", &timeinfo); // requires a tm, not a time_t
                
                printf("Taking HTTP Date: update, oldtime=%s newtime=%s\n",oldtime,newtime);
            }
            else {
                printf("ignoring HTTP Date: update, definitive time age=%d seconds\n",age);
            }
            gHTTPtime_t = 0; // gone stale now
        }


        vTaskDelay(/*1*/1);// reset watchdog requires at least 1

    }
}

void say4(char *t,int decimals) {
    for(int i = 0;i<DIM(vals);i+=1){
        vals[i] = ascii[(unsigned char)(t[i])]+(decimals>=1000?DECIMAL:0);
        decimals=(decimals%1000)*10;
    }

    for(int idig=0;idig<NDIGITS;idig+=1){
        sLoSet_=0;
        sHiSet_=0;
        sLoClear_=0;
        sHiClear_=0;
        int x=vals[idig];
        for(int j=0;j<NSEGMENTS;j+=1){
            makegpio(idig,segs[j], x&1 ? SEGON:SEGOFF);
            x=x>>1;
        }
        //timer_pause(TIMER_GROUP_0, TIMER_1); // I *think* it flickers ... but this makes display fail after 2-5 seconds
        sLoSet[idig] = sLoSet_;
        sHiSet[idig] = sHiSet_;
        sLoClear[idig] = sLoClear_;
        sHiClear[idig] = sHiClear_;
        //timer_start(TIMER_GROUP_0, TIMER_1);
    }
}

