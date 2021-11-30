/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

#define USE_HTTPS 0 // see note in clockServer.c, https uses a lot of memory and time

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#if USE_HTTPS
#include "esp_https_server.h" // testing
#else
#include "esp_http_server.h"
#endif
#include "esp_http_client.h"
#include "esp_sntp.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include <driver/adc.h>
//#include "rom/ets_sys.h" deprecated
#include "esp32/rom/ets_sys.h"
#include "driver/i2c.h"
#include "bme280.h"
#include "i2c_bus.h"
#include "esp_ota_ops.h"
#include <dirent.h>
#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "soc/rtc.h"
#include "mdns.h"
#include "mbedtls/base64.h" // from /home/c/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/base64.h for mbedtls_base64_decode
#include "soc/rtc_wdt.h"

#include "sha/sha_parallel_engine.h"

#include "esp_heap_caps.h"

#include "clockTZ.h"
#include "clock7seg.h"
#include "clockSntp.h"
#include "clockGps.h"
#include "clockBme280.h"
#include "clockspiffs.h"
#include "clockConnect.h"
#include "clockServer.h"
#include "clockHtml.h"

/* Common functions for protocol examples, to establish Wi-Fi or Ethernet connection.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   modified (wch) to provide both STA and AP.
   The AP name and password are on the back of the clock. That's
   how the clock can be configured when there is no valid wifi
   for the clock to connect as STA. Possibly the AP password
   should be a 4-character random string, displayed on power up.
   Or longer, with a scroll... 0123456789AbCdEFHLnoPrUY-_.

   4 words from this 450-word list is 38 billion  https://www.thefreedictionary.com/4-letter-words.htm 2K bytes, not zero terminated...
   "Note that some letters such as K (K), M (M), V (V), W (W), X (X), and Z (Z) are completely unrecognizable by most people and as such been left out of the table below." https://en.wikichip.org/wiki/seven-segment_display/representing_letters
 */



#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m" // https://stackoverflow.com/users/4274163/kritpal-singh
//printf("%sred\n", KRED); https://stackoverflow.com/questions/5412761/using-colors-with-printf
    

extern int sPPS;
extern const int cPPSSET;
extern struct tm gGPStimeinfo;
extern time_t gGPStime_t;
extern uint64_t gGPStimeMessageReceived;
extern time_t gHTTPtime_t;

// a timer task should check periodically; the GPS might fail in a basement,
// SNTP might fail after a few days if *our* router fails, openAP might not be available
// if becomes none, the clock should ... blink?
enum TimeAcquired {
    taNone,  // initially
    taForced, // someone logged in and forced it
    taOpenAP, // fallback - use any open AP to borrow the time from an HTML header
    taSntp, // use a specific AP (with PW) to start SNTP
    taGps // use the GPS
};

// the timezone can be hardcoded in a config file or
// discovered by GPS lookup. otherwise display UTC.
enum ZoneAcquired {
    zaNone,
    zaForced, // someone logged in and forced it, config and gps do not override a forced value
    zaConfigFile, // ignore CLOCKSTATE.timezone. Once set from the config file, do not reset?
    zaGpsLookup // the CLOCKSTATE.timezone is valid
};

enum Brightness {
    brightUseAmbient,
    brightUseConstant
};

struct SCANRECORD {
    wifi_ap_record_t r;
    uint64_t time;
};
struct SATRECORD {
    char prn[6];
    char el[6];
    char az[6];
    char snr[6];
};
struct CLOCKSTATE {
    tcpip_adapter_ip_info_t s_ip_addrs;
    char ip_addr_txt[16]; // "255.255.255.255\0"
    char serialnumber[10]; // "RCacozsw\0" for RedClock and 6 mac-based letters
    char baseMacAddress[18];// xx:xx:xx:xx:xx:xx\0
    char accesspoint_gateway_txt[16]; // 192.168.5.1
    unsigned int nDisconnects;
    uint64_t tickSecondOfDefinitiveTime; // GPS/SNTP set this when definitive time received as: esp_timer_get_time()/1000000;
    uint64_t tickSecondOfDefinitiveTimeSNTP; // GPS/SNTP set this when definitive time received as: esp_timer_get_time()/1000000;
    uint64_t tickSecondOfDefinitiveTimeGPS; // GPS/SNTP set this when definitive time received as: esp_timer_get_time()/1000000;
    double lat;
    double lon;
    char gpstime[16];
    char gpsdate[8];

    char gpsGPTXT[1024]; // really only needs about 300 for ublox...but how to be sure?
    
    
    char gpsGPGLL[101];
    char gpsGPRMC[101];
    char gpsGPVTG[101];
    char gpsGPGGA[101];
    char gpsGPGSA[101];
    
    
    
    /*wifi_ap_record_t*/ struct SCANRECORD scanRecords[256];
#define SatsPerGSV 4
#define MaxGSVRecs 5
    struct SATRECORD satRecords[SatsPerGSV*MaxGSVRecs];// 20 should be enough; I've seen 16
   // xSemaphoreHandle satSemaphore;
    
    esp_chip_info_t chip_info;
    char *chip_infoModel;
    esp_app_desc_t app_desc;
    rtc_cpu_freq_config_t rcf;
    int xtalfreq;
    
    // bme
    double temperature;
    double pressure;
    double humidity;
    
    
//    enum TimeAcquired timeAcquired;
//    enum ZoneAcquired zoneAcquired;
//    char *timezone; // this should point within the lookup table or be null?
    char const *timezoneBuildDate;
//    enum Brightness brightness;
    double smoothAmbient; 
    double smoothTrueAmbient;
    int onDelay;
    int offDelay;
//    int brightConstant;
    // web config stores here...these are the current values, as strings
    char *cfgWiFi[2]; // "on","off"
    char *cfgWiFiName[2]; // for connecting to the user's router "OneThirty"
    char *cfgWiFiPass[2];
    char *cfgMyFiName[2]; // my internal router is for configuration "RCxxyxyxy"
    char *cfgMyFiPass[2]; // "88888888"
    char *cfgBrthigh[2]; // "1000", a string value
    char *cfgBrtlow[2];
    char *cfgBrtdawn[2]; // "0700"
    char *cfgBrtdusk[2];
//    char *cfgTzoffset[2]; // "300"
    char *cfgSntpUrl[2];
    char *cfgTzrule[2];
    char *cfgBright[2]; // "sense","time","max","min"
    char *cfgClock[2]; // "gps","sntp","open"
    char *cfgTzone[2]; // "lookup","rule"
    char *cfgNeighborHoodWatch[2]; //time listening for nearby SSIDs "0","1","2","4","8","16","24","32"
    char *cfgDisplay[2]; // "civilian","military"
    char *cfgLocationNote[2]; // a small user-defined note: "In the Kitchen" or "Room 720"
};

extern struct CLOCKSTATE sClockState;

