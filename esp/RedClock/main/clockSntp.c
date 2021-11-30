/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

#include "clockMain.h"
#include "clockSntp.h"
#include "clock7seg.h"
#include "clockspiffs.h"

static const char *TAG = "clocksntp";

#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_CUSTOM
void sntp_sync_time(struct timeval *tv)
{
garbage...
   settimeofday(tv, NULL);
   ESP_LOGI(TAG, "Time is synchronized from custom code");
   sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}
#endif

void time_sync_notification_cb(struct timeval *tv)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Notification of a time synchronization event %s on core %d", strftime_buf, xPortGetCoreID());
    sClockState.tickSecondOfDefinitiveTime = esp_timer_get_time()/1000000; // GPS/SNTP set this to indicate valid time 
    sClockState.tickSecondOfDefinitiveTimeSNTP = sClockState.tickSecondOfDefinitiveTime;
}

// the pauses were no longer needed when the gpio routine was properly IRAMed
// timer_pause(TIMER_GROUP_0, TIMER_1); // need this pause
// timer_start(TIMER_GROUP_0, TIMER_1);

void obtain_time(void)
{
    if(strcmp(sClockState.cfgClock[0],"gps")==0)
        return; // gps only, dont start sntp at all

    initialize_sntp(); // <<<<<<<<<<<<<<<<<<<<<<
printf("sntp_get_sync_interval %u\n",sntp_get_sync_interval());// 3600000, 1 hour
sntp_set_sync_interval(2*3600*1000); // 2 hours. in 7seg, see secondsSinceDefinitiveTime > 8*3600 for alarm
printf("sntp_get_sync_interval %u\n",sntp_get_sync_interval());// 7200000

    say4("stps",0);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // wait for time to be set
 //   time_t now = 0;
 //   struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d) on core %d", retry, retry_count, xPortGetCoreID());
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
 //   time(&now);
 //   localtime_r(&now, &timeinfo); // not sure the timezone is set yet???
//        say4("    ",100);
//        vTaskDelay(500 / portTICK_PERIOD_MS);

 //   ESP_ERROR_CHECK( example_disconnect() ); // <<<<<<<<<<<<<<<<<<<<<< this stops the connection? wifi_stop, eth_stop in /home/c/esp/esp-idf/examples/common_components/protocol_examples_common/connect.c


}

void initialize_sntp(void)
{ // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html?highlight=time
    ESP_LOGI(TAG, "Initializing SNTP on core %d", xPortGetCoreID());
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, sClockState.cfgSntpUrl[0] ); // "pool.ntp.org"
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
//#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
//    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
//#endif
    sntp_init();
}



