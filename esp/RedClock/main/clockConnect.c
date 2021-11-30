/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

#include "clockMain.h"

time_t gHTTPtime_t; 

#ifdef CONFIG_EXAMPLE_CONNECT_IPV6
#define MAX_IP6_ADDRS_PER_NETIF (5)
#define NR_OF_IP_ADDRESSES_TO_WAIT_FOR (s_active_interfaces*2)

#if defined(CONFIG_EXAMPLE_CONNECT_IPV6_PREF_LOCAL_LINK)
#define EXAMPLE_CONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_LINK_LOCAL
#elif defined(CONFIG_EXAMPLE_CONNECT_IPV6_PREF_GLOBAL)
#define EXAMPLE_CONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_GLOBAL
#elif defined(CONFIG_EXAMPLE_CONNECT_IPV6_PREF_SITE_LOCAL)
#define EXAMPLE_CONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_SITE_LOCAL
#elif defined(CONFIG_EXAMPLE_CONNECT_IPV6_PREF_UNIQUE_LOCAL)
#define EXAMPLE_CONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_UNIQUE_LOCAL
#endif // if-elif CONFIG_EXAMPLE_CONNECT_IPV6_PREF_...

#else
#define NR_OF_IP_ADDRESSES_TO_WAIT_FOR (s_active_interfaces)
#endif

#define EXAMPLE_DO_CONNECT CONFIG_EXAMPLE_CONNECT_WIFI || CONFIG_EXAMPLE_CONNECT_ETHERNET

static int s_active_interfaces = 0;
static xSemaphoreHandle s_semph_get_ip_addrs;
static esp_netif_t *s_example_esp_netif = NULL;

static xSemaphoreHandle sScanSemaphore;

#ifdef CONFIG_EXAMPLE_CONNECT_IPV6
static esp_ip6_addr_t s_ipv6_addr;

/* types of ipv6 addresses to be displayed on ipv6 events */
static const char *s_ipv6_addr_types[] = {
    "ESP_IP6_ADDR_IS_UNKNOWN",
    "ESP_IP6_ADDR_IS_GLOBAL",
    "ESP_IP6_ADDR_IS_LINK_LOCAL",
    "ESP_IP6_ADDR_IS_SITE_LOCAL",
    "ESP_IP6_ADDR_IS_UNIQUE_LOCAL",
    "ESP_IP6_ADDR_IS_IPV4_MAPPED_IPV6"
};
#endif

static const char *TAG = "example_connect";

#if CONFIG_EXAMPLE_CONNECT_WIFI
static esp_netif_t *mywifi_start(void);
static void mywifi_stop(void);
#endif
//#if CONFIG_EXAMPLE_CONNECT_ETHERNET
//+;// compile error test not using this
//static esp_netif_t *eth_start(void);
//static void eth_stop(void);
//#endif

/**
 * @brief Checks the netif description if it contains specified prefix.
 * All netifs created withing common connect component are prefixed with the module TAG,
 * so it returns true if the specified netif is owned by this module
 */
static bool myis_our_netif(const char *prefix, esp_netif_t *netif)
{
    return strncmp(prefix, esp_netif_get_desc(netif), strlen(prefix) - 1) == 0;
}

/* set up connection, Wi-Fi and/or Ethernet */
static void mystart(void)
{

#if CONFIG_EXAMPLE_CONNECT_WIFI
    s_example_esp_netif = mywifi_start();
    s_active_interfaces++;
#endif

//#if CONFIG_EXAMPLE_CONNECT_ETHERNET
//    s_example_esp_netif = eth_start();
//    s_active_interfaces++;
//#endif

//#if CONFIG_EXAMPLE_CONNECT_WIFI && CONFIG_EXAMPLE_CONNECT_ETHERNET
//    /* if both intefaces at once, clear out to indicate that multiple netifs are active */
//    s_example_esp_netif = NULL;
//#endif

#if EXAMPLE_DO_CONNECT
    /* create semaphore if at least one interface is active */
    s_semph_get_ip_addrs = xSemaphoreCreateCounting(NR_OF_IP_ADDRESSES_TO_WAIT_FOR, 0);
#endif

}

/* tear down connection, release resources */
static void mystop(void)
{
#if CONFIG_EXAMPLE_CONNECT_WIFI
    mywifi_stop();
    s_active_interfaces--;
#endif

//#if CONFIG_EXAMPLE_CONNECT_ETHERNET
//    eth_stop();
//    s_active_interfaces--;
//#endif
}

#if EXAMPLE_DO_CONNECT
//static esp_ip4_addr_t s_ip_addr;

static void myon_wifi_any(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
static char *eventname[] = { 
    "WIFI_READY",           /**< ESP32 WiFi ready */
    "SCAN_DONE",                /**< ESP32 finish scanning AP */
    "STA_START",                /**< ESP32 station start */
    "STA_STOP",                 /**< ESP32 station stop */
    "STA_CONNECTED",            /**< ESP32 station connected to AP */
    "STA_DISCONNECTED",         /**< ESP32 station disconnected from AP */
    "STA_AUTHMODE_CHANGE",      /**< the auth mode of AP connected by ESP32 station changed */

    "STA_WPS_ER_SUCCESS",       /**< ESP32 station wps succeeds in enrollee mode */
    "STA_WPS_ER_FAILED",        /**< ESP32 station wps fails in enrollee mode */
    "STA_WPS_ER_TIMEOUT",       /**< ESP32 station wps timeout in enrollee mode */
    "STA_WPS_ER_PIN",           /**< ESP32 station wps pin code in enrollee mode */
    "STA_WPS_ER_PBC_OVERLAP",   /**< ESP32 station wps overlap in enrollee mode */

    "AP_START",                 /**< ESP32 soft-AP start */
    "AP_STOP",                  /**< ESP32 soft-AP stop */
    "AP_STACONNECTED",          /**< a station connected to ESP32 soft-AP */
    "AP_STADISCONNECTED",       /**< a station disconnected from ESP32 soft-AP */
    "AP_PROBEREQRECVED",        /**< Receive probe request packet in soft-AP interface */

    "FTM_REPORT",               /**< Receive report of FTM procedure */

    /* Add next events after this only */
    "STA_BSS_RSSI_LOW"         /**< AP's RSSI crossed configured threshold */
};
    if (event_id != SYSTEM_EVENT_SCAN_DONE){ // too noisy
        ESP_LOGI(TAG,"myon_wifi_any ******* wifi event ********** %s **********",eventname[event_id]);
    }
}

// for qsort, below
static int compare( const void* a, const void* b)
{
   int8_t int_a =  ( (wifi_ap_record_t *) a )->rssi;
   int8_t int_b =  ( (wifi_ap_record_t *) b )->rssi;

   // if <0, a goes before b. -70:-73 produces true - false is >0; the following is a descending sort, appropriate for neg decibel vals to get loudest first
   return (int_a > int_b) - (int_a < int_b);
}

static void my_scan_done_event_handler(void* arg, esp_event_base_t event_base,	int event_id, void* event_data) { // see esp_wifi_scan_start
    uint16_t nrecs = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num( &nrecs ));
    wifi_ap_record_t *ap_records = malloc(nrecs * sizeof(wifi_ap_record_t));
    esp_wifi_scan_get_ap_records( &nrecs, ap_records );
    /*
struct wifi_ap_record_t // Description of a WiFi AP.
// Public Members
uint8_t bssid[6] // MAC address of AP
uint8_t ssid[33] // SSID of AP
uint8_t primary // channel of AP
wifi_second_chan_t second // secondary channel of AP
int8_t rssi // signal strength of AP
wifi_auth_mode_t authmode // authmode of AP
wifi_cipher_type_t pairwise_cipher // pairwise cipher of AP
wifi_cipher_type_t group_cipher // group cipher of AP
wifi_ant_t ant // antenna used to receive beacon from AP
uint32_t phy_11b : 1 // bit: 0 flag to identify if 11b mode is enabled or not
uint32_t phy_11g : 1 // bit: 1 flag to identify if 11g mode is enabled or not
uint32_t phy_11n : 1 // bit: 2 flag to identify if 11n mode is enabled or not
uint32_t phy_lr : 1 // bit: 3 flag to identify if low rate is enabled or not
uint32_t wps : 1 // bit: 4 flag to identify if WPS is supported or not
uint32_t ftm_responder : 1 // bit: 5 flag to identify if FTM is supported in responder mode
uint32_t ftm_initiator : 1 // bit: 6 flag to identify if FTM is supported in initiator mode
uint32_t reserved : 25 //bit: 7..31 reserved
wifi_country_t country //country information of AP    
    */
    /*
typedef enum {
    WIFI_SECOND_CHAN_NONE = 0,  // < the channel width is HT20 
    WIFI_SECOND_CHAN_ABOVE,     // < the channel width is HT40 and the secondary channel is above the primary channel 
    WIFI_SECOND_CHAN_BELOW,     // < the channel width is HT40 and the secondary channel is below the primary channel 
} wifi_second_chan_t;    
    */
    
    

    
    

    
    /*
    enum wifi_ant_t //WiFi antenna.
WIFI_ANT_ANT0
WIFI_ANT_ANT1
WIFI_ANT_MAX
    */

    /*
    struct wifi_country_t // Structure describing WiFi country-based regional restrictions.
char cc[3] // country code string
uint8_t schan // start channel
uint8_t nchan // total channel number
int8_t max_t x_power // This field is used for getting WiFi maximum transmitting power, call esp_wifi_set_max_tx_power to set the maximum transmitting power.
wifi_country_policy_t policy //country policy
    */
    
    /*
    enum wifi_country_policy_t
WIFI_COUNTRY_POLICY_AUTO // Country policy is auto, use the country info of AP to which the station is connected
WIFI_COUNTRY_POLICY_MANUAL // Country policy is manual, always use the configured country info
    */
    
    
    // sort by sig strength

    qsort( ap_records, nrecs, sizeof(ap_records[0]), compare );
    
    
  //  printf("=======%d APs found:=======\n", nrecs);
    uint64_t now = esp_timer_get_time()/1000000;

    for(int i = 0; i<nrecs; i+=1){
    /*
        printf("%d cc='%.3s' start=%d nchan=%d %02x:%02x:%02x:%02x:%02x:%02x %s pairwise=%s group=%s chan=%d strength=%d auth=%s\n",
        i,ap_records[i].country.cc,ap_records[i].country.schan,ap_records[i].country.nchan,
        ap_records[i].bssid[0],ap_records[i].bssid[1],ap_records[i].bssid[2],ap_records[i].bssid[3],ap_records[i].bssid[4],ap_records[i].bssid[5],
        ap_records[i].ssid,sCipherType[ap_records[i].pairwise_cipher],sCipherType[ap_records[i].group_cipher],
        ap_records[i].primary,ap_records[i].rssi, 
        sAuthmode[ap_records[i].authmode]);
      */  
        
        
        // update/insert sClockState.scanRecords using ap_records[i].bssid as key to find a dup
        // 
        // Move existing recs to dup or end to open position for new rec
        int dupPosition;
        for( dupPosition=0; ; dupPosition += 1) {
            if (dupPosition == DIM(sClockState.scanRecords)-1 || // got to the end, the last one is the dup
                memcmp(ap_records[i].bssid,sClockState.scanRecords[dupPosition].r.bssid,sizeof(ap_records[i].bssid))==0) {
        //        printf("dup of %s at %d\n", ap_records[i].ssid, dupPosition);
                break;
            }
        }
        if(dupPosition>0){
          //  printf("collapse %d recs from 0 to 1\n",dupPosition);
            // if (dupPosition == DIM(sClockState.scanRecords)-1) { printf("drop %s in favor of %s\n", sClockState.scanRecords[dupPosition].r.ssid, ap_records[i].ssid); }
            memmove(&sClockState.scanRecords[1]/*dest*/, 
                &sClockState.scanRecords[0]/*src*/, 
                sizeof(sClockState.scanRecords[0])*dupPosition);
        }
//        printf("store %s at 0\n",ap_records[i].ssid);
        memmove(&sClockState.scanRecords[0].r,&ap_records[i],sizeof(sClockState.scanRecords[0].r));
        sClockState.scanRecords[0].time = now;
    }
//    printf(KBLU "------- current list -------\n" KNRM);
//    for(int i = 0; i<DIM(sClockState.scanRecords); i+=1){
//        static uint8_t emptybssid[6] = {0};
//        if(memcmp(emptybssid,sClockState.scanRecords[i].r.bssid,sizeof(emptybssid))!=0 || strlen((char *)sClockState.scanRecords[i].r.ssid)>0 ){
//            printf(KCYN "%02x:%02x:%02x:%02x:%02x:%02x sig %d %s age %lld %s\n" KNRM,
//                sClockState.scanRecords[i].r.bssid[0],
//                sClockState.scanRecords[i].r.bssid[1],
//                sClockState.scanRecords[i].r.bssid[2],
//                sClockState.scanRecords[i].r.bssid[3],
//                sClockState.scanRecords[i].r.bssid[4],
//                sClockState.scanRecords[i].r.bssid[5],
//                sClockState.scanRecords[i].r.rssi,
//                (sClockState.scanRecords[i].r.authmode!=0)?KGRN "🔒" KNRM:KRED "🔓" KNRM,
//                now-sClockState.scanRecords[i].time,
//                sClockState.scanRecords[i].r.ssid
//                );
//        }
//    }
    
    
    free(ap_records);
    
    xSemaphoreGive(sScanSemaphore);
    //printf("scan give sem\n");
}

static void myon_ip_any(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
static char *eventname[] = {
    "STA_GOT_IP",               /*!< station got IP from connected AP */
    "STA_LOST_IP",              /*!< station lost IP and the IP is reset to 0 */
    "AP_STAIPASSIGNED",         /*!< soft-AP assign an IP to a connected station */
    "GOT_IP6",                  /*!< station or ap or ethernet interface v6IP addr is preferred */
    "ETH_GOT_IP",               /*!< ethernet got IP from connected AP */
    "PPP_GOT_IP",               /*!< PPP interface got IP */
    "PPP_LOST_IP",              /*!< PPP interface lost IP */
};
    switch(event_id){
    case IP_EVENT_AP_STAIPASSIGNED:{
        ip_event_ap_staipassigned_t *event = (ip_event_ap_staipassigned_t *)event_data;
        ESP_LOGI(TAG,"myon_ip_any ******** IP_EVENT_AP_STAIPASSIGNED " IPSTR " on core %d", IP2STR(&event->ip), xPortGetCoreID());
        }
        break;
    default:
        ESP_LOGI(TAG,"myon_ip_any ip event %s ***********",eventname[event_id]);
    }
}

static void myon_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
/*
typedef ip_event_got_ip_t system_event_sta_got_ip_t;
typedef struct {
    tcpip_adapter_ip_info_t ip_info;
    bool ip_changed;
} system_event_sta_got_ip_t;

typedef struct {
    ip4_addr_t ip;
    ip4_addr_t netmask;
    ip4_addr_t gw;
} tcpip_adapter_ip_info_t;

*/
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (!myis_our_netif(TAG, event->esp_netif)) {
        ESP_LOGW(TAG, " myon_got_ip Got IPv4 from another interface \"%s\": ignored", esp_netif_get_desc(event->esp_netif));
        return;
    }
    ESP_LOGI(TAG, " myon_got_ip IP_EVENT_STA_GOT_IP Got IPv4 event: Interface \"%s\""
    " ip addr: " IPSTR 
    " netmask: " IPSTR 
    " gateway: " IPSTR 
    " on core %d", 
    esp_netif_get_desc(event->esp_netif), 
    IP2STR(&event->ip_info.ip), 
    IP2STR(&event->ip_info.netmask), 
    IP2STR(&event->ip_info.gw), 
    xPortGetCoreID());
    memcpy(&sClockState.s_ip_addrs, &event->ip_info, sizeof(sClockState.s_ip_addrs));
    snprintf(sClockState.ip_addr_txt,sizeof(sClockState.ip_addr_txt),IPSTR,IP2STR(&sClockState.s_ip_addrs.ip));
    xSemaphoreGive(s_semph_get_ip_addrs);
    
    sClockState.nDisconnects = 0; // success!
}
#endif

#ifdef CONFIG_EXAMPLE_CONNECT_IPV6

static void myon_got_ipv6(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
    if (!myis_our_netif(TAG, event->esp_netif)) {
        ESP_LOGW(TAG, "Got IPv6 from another netif: ignored");
        return;
    }
    esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);
    ESP_LOGI(TAG, "Got IPv6 event: Interface \"%s\" address: " IPV6STR ", type: %s", esp_netif_get_desc(event->esp_netif),
             IPV62STR(event->ip6_info.ip), s_ipv6_addr_types[ipv6_type]);
    if (ipv6_type == EXAMPLE_CONNECT_PREFERRED_IPV6_TYPE) {
        memcpy(&s_ipv6_addr, &event->ip6_info.ip, sizeof(s_ipv6_addr));
        xSemaphoreGive(s_semph_get_ip_addrs);
    }
}

#endif // CONFIG_EXAMPLE_CONNECT_IPV6





/*
typedef struct esp_http_client_event {
    esp_http_client_event_id_t event_id;    !< event_id, to know the cause of the event 
    esp_http_client_handle_t client;        !< esp_http_client_handle_t context 
    void *data;                             !< data of the event 
    int data_len;                           !< data length of data 
    void *user_data;                        !< user_data context, from esp_http_client_config_t user_data 
    char *header_key;                       !< For HTTP_EVENT_ON_HEADER event_id, it's store current http header key 
    char *header_value;                     !< For HTTP_EVENT_ON_HEADER event_id, it's store current http header value 
} esp_http_client_event_t;
*/

// the http client code added ~100K ? seems huge. I believe it has https support, and might be useful elsewhere,
// if I wanted to POST a secure upload to a passworded site (like the shop camera did, for example)

esp_err_t _http_event_time_from_headers_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
//            ESP_LOGI(TAG, "HTTP_EVENT_ERROR, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_CONNECTED:
//            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_HEADER_SENT:
//            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_HEADER:
//            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, len=%d", evt->data_len);
//            printf("%.*s", evt->data_len, (char*)evt->data);      // Date: Mon, 01 Mar 2021 00:45:38 GMT
            // post this time, similar to GPS time, for possible use in timesetting if there is no recent GPS or SNTP...
            if(strcmp("Date",evt->header_key)==0) {
                printf("%s: %s\n",evt->header_key,evt->header_value); // Date: Sun, 28 Feb 2021 20:37:30 GMT
                struct tm tm={0};
                strptime( evt->header_value,"%a, %d %b %Y %H:%M:%S GMT", &tm );
                gHTTPtime_t = mktime(&tm) - _timezone;
                printf("epoch from http=%lu\n",gHTTPtime_t);
            }
            break;
        case HTTP_EVENT_ON_DATA:
//            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
           // if (!esp_http_client_is_chunked_response(evt->client)) {
//                printf("%.*s", evt->data_len, (char*)evt->data);
           // }

            break;
        case HTTP_EVENT_ON_FINISH:
//            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_DISCONNECTED:
//            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED, len=%d", evt->data_len);
            break;
    }
    return ESP_OK;
}

static void time_from_headers_task(void *arg) {
    while(1) {



/*


typedef struct {
    const char                  *url;                //!< HTTP URL, the information on the URL is most important, it overrides the other fields below, if any 
    const char                  *host;               //!< Domain or IP as string 
    int                         port;                //!< Port to connect, default depend on esp_http_client_transport_t (80 or 443) 
    const char                  *username;           //!< Using for Http authentication 
    const char                  *password;           //!< Using for Http authentication 
    esp_http_client_auth_type_t auth_type;           //!< Http authentication type, see `esp_http_client_auth_type_t` NONE=0
    const char                  *path;               //!< HTTP Path, if not set, default is `/` 
    const char                  *query;              //!< HTTP query 
    const char                  *cert_pem;           //!< SSL server certification, PEM format as string, if the client requires to verify server 
    const char                  *client_cert_pem;    //!< SSL client certification, PEM format as string, if the server requires to verify client 
    const char                  *client_key_pem;     //!< SSL client key, PEM format as string, if the server requires to verify client 
    const char                  *user_agent;         //!< The User Agent string to send with HTTP requests 
    esp_http_client_method_t    method;                   //!< HTTP Method GET=0 POST etc
    int                         timeout_ms;               //!< Network timeout in milliseconds 
    bool                        disable_auto_redirect;    //!< Disable HTTP automatic redirects 
    int                         max_redirection_count;    //!< Max number of redirections on receiving HTTP redirect status code, using default value if zero
    int                         max_authorization_retries;    //!< Max connection retries on receiving HTTP unauthorized status code, using default value if zero. Disables authorization retry if -1
    http_event_handle_cb        event_handler;             //!< HTTP Event Handle 
    esp_http_client_transport_t transport_type;           //!< HTTP transport type, see `esp_http_client_transport_t` UNKNOWN=0
    int                         buffer_size;              //!< HTTP receive buffer size 
    int                         buffer_size_tx;           //!< HTTP transmit buffer size 
    void                        *user_data;               //!< HTTP user_data context 
    bool                        is_async;                 //!< Set asynchronous mode, only supported with HTTPS for now 
    bool                        use_global_ca_store;      //!< Use a global ca_store for all the connections in which this bool is set. 
    bool                        skip_cert_common_name_check;    //!< Skip any validation of server certificate CN field 
} esp_http_client_config_t;

*/

        if ( strcmp(sClockState.cfgClock[0],"open") == 0 ) { // don't do this potentially annoying thing unless asked for...
            esp_http_client_config_t config = {
               .url = "http://example.com/nosuchfile.cgi", // years ago, this worked on a number of open routers.
               //  /home/c/linuxFiles/CameraPiWithTimeFromInternet/backup/scanner-py
               // there were a few routers that would not return the time, but asking for a cgi file triggered it to work.
               // generally there is no internet connection and no resource is returned; just the time header on an error page.
               // it *seems* like these open (non-wpa password) routers may be disappearing. I only see one in-the-wild now, far away.
               .event_handler = _http_event_time_from_headers_handler,
            };
            esp_http_client_handle_t client = esp_http_client_init(&config);
            esp_err_t err = esp_http_client_perform(client);

            if (err == ESP_OK) {
               ESP_LOGI(TAG, "******* http client done ********** Status = %d, content_length = %d",
                       esp_http_client_get_status_code(client), // Status = 200, content_length = -1
                       esp_http_client_get_content_length(client));
            }
            esp_http_client_cleanup(client);
            
            vTaskDelay(1*3600*1000 / portTICK_RATE_MS); // once an hour, don't be too rude
        }
        vTaskDelay(2*1000 / portTICK_RATE_MS); // re-test option every 2 seconds
    }
}

static void scan_task(void *arg) {
    sScanSemaphore = xSemaphoreCreateBinary();
    int nextchan = 1;
    while(1) {
        int watch32nds = atoi(sClockState.cfgNeighborHoodWatch[0]); // current config value
        if ( watch32nds < 0 ) watch32nds = 0;
        if ( watch32nds > 32 ) watch32nds = 32;
        if ( watch32nds ) {
           // printf("scan start\n");
            wifi_scan_config_t scan_config = { 0 };
            scan_config.channel = 0;//nextchan; // out of 1..13
            scan_config.show_hidden = 1;
            scan_config.scan_type = WIFI_SCAN_TYPE_PASSIVE; // vs WIFI_SCAN_TYPE_ACTIVE; // vs WIFI_SCAN_TYPE_PASSIVE
            scan_config.scan_time.passive = 50+2*watch32nds; // dwell on each channel, * 11 channels, less than 1500 ms
            
            esp_err_t rc = esp_wifi_scan_start(&scan_config, false);
            if ( rc == ESP_OK ) {
                xSemaphoreTake( sScanSemaphore, portMAX_DELAY/*block forever*/ );
            } 
            else {
                printf("clockConnect: scan_task %d from start scan...reconnecting, skipping scan\n",rc);
            }
        }
        int delay = (int) lerp(watch32nds, 1, 5000, 32, 500); // only wait 1/2 sec during fast 32, 5 sec for slow 1
        vTaskDelay(delay / portTICK_RATE_MS);
        nextchan += 1;
        if (nextchan>11)
            nextchan=1;
    }
    // this is how a task is supposed to exit...
//    vTaskDelete(NULL);
}

esp_err_t my_connect(void)
{
    bool useWIFI = strcmp(sClockState.cfgWiFi[0],"off") != 0; // this really means use STATION mode. always use ACCESS POINT mode.

    snprintf(sClockState.ip_addr_txt,sizeof(sClockState.ip_addr_txt),"(not connected)");
#if EXAMPLE_DO_CONNECT
    if (s_semph_get_ip_addrs != NULL) {
        return ESP_ERR_INVALID_STATE;
    }
#endif
    mystart();
    if(useWIFI){
        ESP_ERROR_CHECK(esp_register_shutdown_handler(&mystop));
        ESP_LOGI(TAG, "Waiting for IP(s) on core %d", xPortGetCoreID());
        for (int i = 0; i < NR_OF_IP_ADDRESSES_TO_WAIT_FOR; ++i) {
            if(xSemaphoreTake(s_semph_get_ip_addrs, 10000 / portTICK_PERIOD_MS)==pdTRUE)// 10 sec timeout
                ESP_LOGI(TAG, "got %d on core %d", i+1, xPortGetCoreID());
            else
                ESP_LOGI(TAG, "FAILED %d on core %d", i+1, xPortGetCoreID());
        }
        // iterate over active interfaces, and print out IPs of "our" netifs
        esp_netif_t *netif = NULL;
        esp_netif_ip_info_t ip;
        for (int i = 0; i < esp_netif_get_nr_of_ifs(); ++i) {
            netif = esp_netif_next(netif);
            if (myis_our_netif(TAG, netif)) {
                ESP_LOGI(TAG, "Connected to %s", esp_netif_get_desc(netif));
                ESP_ERROR_CHECK(esp_netif_get_ip_info(netif, &ip));

                ESP_LOGI(TAG, "- IPv4 address: " IPSTR, IP2STR(&ip.ip));
    #ifdef CONFIG_EXAMPLE_CONNECT_IPV6
                esp_ip6_addr_t ip6[MAX_IP6_ADDRS_PER_NETIF];
                int ip6_addrs = esp_netif_get_all_ip6(netif, ip6);
                for (int j = 0; j < ip6_addrs; ++j) {
                    esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&(ip6[j]));
                    ESP_LOGI(TAG, "- IPv6 address: " IPV6STR ", type: %s", IPV62STR(ip6[j]), s_ipv6_addr_types[ipv6_type]);
                }
    #endif

            }
            else {
                ESP_LOGI(TAG, "Other? %s", esp_netif_get_desc(netif));
                ESP_LOGI(TAG, "other ipv4: " IPSTR, IP2STR(&ip.ip));
            }
        }
    }
    
    // scan? 
    xTaskCreatePinnedToCore(scan_task, "scan_task", 1024*2, NULL, 5/*configMAX_PRIORITIES is 5*/, NULL, 0/*core*/);
    
    // spawn the task that gets the time from headers (this really needs the extra stack size, *2 fails, *3 works)
    xTaskCreatePinnedToCore(time_from_headers_task, "header_task", 1024*3, NULL, 5/*configMAX_PRIORITIES is 5*/, NULL, 0/*core*/);
    
    
    return ESP_OK;
}

esp_err_t my_disconnect(void)
{
    if (s_semph_get_ip_addrs == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    vSemaphoreDelete(s_semph_get_ip_addrs);
    s_semph_get_ip_addrs = NULL;
    mystop();
    ESP_ERROR_CHECK(esp_unregister_shutdown_handler(&mystop));
    return ESP_OK;
}

#ifdef CONFIG_EXAMPLE_CONNECT_WIFI

static void myon_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    sClockState.s_ip_addrs.ip.addr=0;
    sClockState.s_ip_addrs.netmask.addr=0;
    sClockState.s_ip_addrs.gw.addr=0;
    snprintf(sClockState.ip_addr_txt,sizeof(sClockState.ip_addr_txt),IPSTR,IP2STR(&sClockState.s_ip_addrs.ip));
    sClockState.nDisconnects += 1;
    vTaskDelay(2300 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...on code %d", xPortGetCoreID());
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

#ifdef CONFIG_EXAMPLE_CONNECT_IPV6

static void myon_wifi_connect(void *esp_netif, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi connect...on core %d", xPortGetCoreID());
    esp_netif_create_ip6_linklocal(esp_netif);
}

#endif // CONFIG_EXAMPLE_CONNECT_IPV6

static esp_netif_t *mywifi_start(void)
{

    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        printf("MDNS Init failed: %d\n", err);
        //return;
    }
    else {
        //set hostname
        mdns_hostname_set(sClockState.cfgMyFiName[0]); // this is the .local name
        //set default instance
        mdns_instance_name_set(sClockState.cfgMyFiName[0]); // dunno what this is for
        mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
        mdns_service_instance_name_set("_http", "_tcp", sClockState.cfgMyFiName[0]); // this shows up as a confusing label
        // this is a lie, as far as I know, but I'm testing to see if it makes my avahi more reliably hang on to it
        mdns_service_add(NULL, "_workstation", "_tcp", 22, NULL, 0);
        mdns_service_instance_name_set("_workstation", "_tcp", sClockState.cfgMyFiName[0]); // this shows up as a confusing label
    }

    bool useWIFI = strcmp(sClockState.cfgWiFi[0],"off") != 0; // this really means use STATION mode. always use ACCESS POINT mode.

    char *desc;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(esp_wifi_set_mode(0));
    ESP_ERROR_CHECK(esp_wifi_stop());

    esp_netif_t *netifSTA = NULL;
    
    if ( useWIFI ) {
        esp_netif_inherent_config_t esp_netif_configSTA = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA(); // ESP_NETIF_INHERENT_DEFAULT_WIFI_AP();//
        // Prefix the interface description with the module TAG
        // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
        asprintf(&desc, "%s: %s", TAG, esp_netif_configSTA.if_desc);
        esp_netif_configSTA.if_desc = desc;
        esp_netif_configSTA.route_prio = 128;

    // nope    esp_netif_configSTA.flags |= (ESP_NETIF_DHCP_SERVER|ESP_NETIF_DHCP_CLIENT);





        //ESP_LOGI(TAG, "0esp_netif_get_nr_of_ifs = %d", esp_netif_get_nr_of_ifs()); // should be 2, one AP and one STA
        netifSTA = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_configSTA);
        //ESP_LOGI(TAG, "1esp_netif_get_nr_of_ifs = %d", esp_netif_get_nr_of_ifs()); // should be 2, one AP and one STA

        free(desc); // after esp_netif_configSTA is done with esp_netif_create_wifi




        ESP_ERROR_CHECK(esp_wifi_set_default_wifi_sta_handlers());

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &myon_wifi_disconnect, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &myon_got_ip, NULL));
#ifdef CONFIG_EXAMPLE_CONNECT_IPV6
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &myon_wifi_connect, netifSTA));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &myon_got_ipv6, NULL));
#endif
    }
/////////////////////////
// from softap_example_main.c USE THE exampes/wifi/gettingstarted version, NOT the IOT version
// https://esp32.com/viewtopic.php?f=13&t=538&start=10 AP+STA

    esp_netif_inherent_config_t esp_netif_configAP = ESP_NETIF_INHERENT_DEFAULT_WIFI_AP();

/* esp_netif_defaults.c defines this
const esp_netif_ip_info_t _g_esp_netif_soft_ap_ip = {
        .ip = { .addr = ESP_IP4TOADDR( 192, 168, 4, 1) },
        .gw = { .addr = ESP_IP4TOADDR( 192, 168, 4, 1) },
        .netmask = { .addr = ESP_IP4TOADDR( 255, 255, 255, 0) },
};
esp_netif_defaults.h defines ESP_NETIF_INHERENT_DEFAULT_WIFI_AP with
.ip_info = &_g_esp_netif_soft_ap_ip
*/
    // override here
    static const esp_netif_ip_info_t my_esp_netif_soft_ap_ip = {
            .ip = { .addr = ESP_IP4TOADDR( 192, 168, 5, 1) },
            .gw = { .addr = ESP_IP4TOADDR( 192, 168, 5, 1) },
            .netmask = { .addr = ESP_IP4TOADDR( 255, 255, 255, 0) },
    };
    // for the server to grab the address from gw...
    snprintf(sClockState.accesspoint_gateway_txt,sizeof(sClockState.accesspoint_gateway_txt),IPSTR,IP2STR(&my_esp_netif_soft_ap_ip.gw));
    esp_netif_configAP.ip_info = &my_esp_netif_soft_ap_ip;
   // esp_netif_configAP.route_prio = 10; // testing, sta is 100 AP must be < STA for SNTP to work

    //ESP_LOGI(TAG, "1esp_netif_get_nr_of_ifs = %d", esp_netif_get_nr_of_ifs()); // should be 2, one AP and one STA
    esp_netif_t *netifAP = esp_netif_create_wifi(WIFI_IF_AP, &esp_netif_configAP);
    //ESP_LOGI(TAG, "2esp_netif_get_nr_of_ifs = %d", esp_netif_get_nr_of_ifs()); // should be 2, one AP and one STA
    //ESP_ERROR_CHECK(esp_netif_attach_wifi_ap(netifSTA));

//
//    ip4_addr_t ipaddr, netmask, gw;
//	IP4_ADDR(&gw, 192,168,5,1);
//	IP4_ADDR(&ipaddr, 192,168,5,2);
//	IP4_ADDR(&netmask, 255,255,255,0);
//    err_t rc = netifapi_netif_set_addr( netifAP, &ipaddr, &netmask, &gw);
//    ESP_LOGI(TAG, "netifapi_netif_set_addr = %d", rc);
//
//

//////////////////////
    if( useWIFI ) {
        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) ); // if wifi on, both access point and station

        wifi_config_t wifi_configSTA 
        = {
            .sta = {
    //            .ssid = sClockState.cfgWifiname[0],//CONFIG_EXAMPLE_WIFI_SSID,
    //            .password = sClockState.cfgWifipass[0],//CONFIG_EXAMPLE_WIFI_PASSWORD,
    //        //
    //        //    uint8_t ssid[32];      /**< SSID of target AP. */
    //        //    uint8_t password[64];  /**< Password of target AP. */
    //        //    wifi_scan_method_t scan_method;    /**< do all channel scan or fast scan */
    //        //    bool bssid_set;        /**< whether set MAC address of target AP or not. Generally, station_config.bssid_set needs to be 0; and it needs to be 1 only when users need to check the MAC address of the AP.*/
    //        //    uint8_t bssid[6];     /**< MAC address of target AP*/
    //        //    uint8_t channel;       /**< channel of target AP. Set to 1~13 to scan starting from the specified channel before connecting to AP. If the channel of AP is unknown, set it to 0.*/
    //        //    uint16_t listen_interval;   /**< Listen interval for ESP32 station to receive beacon when WIFI_PS_MAX_MODEM is set. Units: AP beacon intervals. Defaults to 3 if set to 0. */
    //        //    wifi_sort_method_t sort_method;    /**< sort the connect AP in the list by rssi or security mode */
    //        //    wifi_scan_threshold_t  threshold;     /**< When sort_method is set, only APs which have an auth mode that is more secure than the selected auth mode and a signal stronger than the minimum RSSI will be used. */
    //        //    wifi_pmf_config_t pmf_cfg;    /**< Configuration for Protected Management Frame. Will be advertized in RSN Capabilities in RSN IE. */
    //        //    uint32_t rm_enabled:1;        /**< Whether Radio Measurements are enabled for the connection */
    //        //    uint32_t btm_enabled:1;       /**< Whether BSS Transition Management is enabled for the connection */
    //        //    uint32_t reserved:30;         /**< Reserved for future feature set */
    //        //
    //        //
            },
        };
        strlcpy((char *)wifi_configSTA.sta.ssid, sClockState.cfgWiFiName[0], sizeof(wifi_configSTA.sta.ssid));
        strlcpy((char *)wifi_configSTA.sta.password, sClockState.cfgWiFiPass[0], sizeof(wifi_configSTA.sta.password));
        
        ESP_LOGI(TAG, "wifi_configSTA.sta.ssid %s", wifi_configSTA.sta.ssid);
        ESP_LOGI(TAG, "esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_configSTA)");

        err = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configSTA);
        if(err!=0) {
            printf("clockConnect: bad sta config rc=%d\n",err);
        }
    }
    else {
        // actually, using APSTA in anticipation of esp_wifi_scan_start requiring station mode.
        // was working OK in WIFI_MODE_AP, seems to work in WIFI_MODE_APSTA after simple test. lets try scan next.
        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) ); // if wifi is off, only be an access point
    }
    
////////////  ALWAYS config the AP, even when wifi turned off

    wifi_config_t wifi_configAP = {
        .ap = {
            .ssid = "",
            .ssid_len = 0, // uses strlen internally, below adds a null terminated serialnumber for the ssid
            // even though there *seem* to be 11 channels, only 1,6, or 11 should be used in the US
            // you can study the recent_wifi list to get a feeling for how your neighbors behave.
            // read up on it elsewhere. 
            // https://superuser.com/questions/382042/why-use-wifi-channels-other-than-1-6-or-11 argues both sides.
            // a way to think about it: each channel uses 2 channels above and 2 channels below.
            // nominal channel 6 uses 4,5,6,7,8
            // any nominal channel other than 1 or 11 MUST overlap nominal 6.
            // But, if bad neighbors only use 1-6, then 11 will be free, or
            //                                6-11 then 1 will be free.
            // that's a crude approximation; there is not a hard wall between the channels. further away is
            // better; the spill-over from nearby channels makes more noise in the selected channel than
            // the spill-over from further channels. The problem is the current channel does not
            // detect the noise as a signal and transmits anyway, causing potential errors in *both* channels.
            // channels 6-11 can expect more microwave oven interference.
            // channel 1 may be a common default channel.
            // definitly a tragedy of the commons possibility here.
            .channel = 1, // 1 or 11 has a better chance of being clear; 6 is almost always going to have overlaps from uneducated channel selection
            .password = "88888888", // overwritten by cfgMyFiPass, below
            .max_connection = 2,
            .authmode = WIFI_AUTH_WPA2_PSK, //    wifi_configAP.ap.authmode = WIFI_AUTH_OPEN; if password len is 0
            .beacon_interval = 3000,
            .pairwise_cipher = WIFI_CIPHER_TYPE_CCMP
//
//    uint8_t ssid[32];           /**< SSID of ESP32 soft-AP. If ssid_len field is 0, this must be a Null terminated string. Otherwise, length is set according to ssid_len. */
//    uint8_t password[64];       /**< Password of ESP32 soft-AP. */
//    uint8_t ssid_len;           /**< Optional length of SSID field. */
//    uint8_t channel;            /**< Channel of ESP32 soft-AP */
//    wifi_auth_mode_t authmode;  /**< Auth mode of ESP32 soft-AP. Do not support AUTH_WEP in soft-AP mode */
//    uint8_t ssid_hidden;        /**< Broadcast SSID or not, default 0, broadcast the SSID */
//    uint8_t max_connection;     /**< Max number of stations allowed to connect in, default 4, max 10 */
//    uint16_t beacon_interval;   /**< Beacon interval which should be multiples of 100. Unit: TU(time unit, 1 TU = 1024 us). Range: 100 ~ 60000. Default value: 100 */
//
//

        },
    };
//    strlcpy( (char *)wifi_configAP.ap.ssid, sClockState.serialnumber, sizeof(wifi_configAP.ap.ssid) );
    strlcpy( (char *)wifi_configAP.ap.ssid, sClockState.cfgMyFiName[0], sizeof(wifi_configAP.ap.ssid) );
    strlcpy( (char *)wifi_configAP.ap.password, sClockState.cfgMyFiPass[0], sizeof(wifi_configAP.ap.password) );
    
    ESP_LOGI(TAG, "wifi_configAP.ap.ssid %s...", wifi_configAP.ap.ssid);
    ESP_LOGI(TAG, "esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_configAP)");
    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_configAP);
    if(err!=0) {
        printf("clockConnect: bad ap config rc=%d\n",err);
    }
/////////////////
    ESP_ERROR_CHECK(esp_wifi_set_default_wifi_ap_handlers());
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &myon_wifi_any, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &myon_ip_any, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE,&my_scan_done_event_handler, NULL)); // see esp_wifi_scan_start


// end softap_example_main.c
    if(netifSTA) {
        ESP_ERROR_CHECK(esp_netif_set_hostname(netifSTA, sClockState.cfgMyFiName[0])); // must be before start to take effect
    }

    ESP_LOGI(TAG, "esp_wifi_start()");
    ESP_ERROR_CHECK(esp_wifi_start());

    
    if(netifSTA) {
        char const *tttt;// testing temp
        esp_err_t rc=esp_netif_get_hostname(netifSTA, &tttt);
        if(rc!=0)
            printf("failed to get hostname esp_netif_get_hostname in clockConnect.c");
        //ESP_ERROR_CHECK(rc); // must be after start to succeed
        printf("change hostname now %s\n",tttt);
    }
//vTaskDelay(5000 / portTICK_PERIOD_MS);

    wifi_country_t config_country = {.cc="US", .schan=1, .nchan=11, WIFI_COUNTRY_POLICY_MANUAL};
    ESP_ERROR_CHECK(esp_wifi_set_country(&config_country));
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(80)); // range 8(2dbm) to 80(20dbm) (8 was unreliable, 40 marginal)
    
    if ( useWIFI ) {
        ESP_LOGI(TAG, "esp_netif_dhcpc_start(netifSTA)");
        ESP_ERROR_CHECK(esp_netif_dhcpc_start(netifSTA));
        ESP_LOGI(TAG, "esp_wifi_connect() on core %d", xPortGetCoreID());
        ESP_ERROR_CHECK(esp_wifi_connect());
    }

    ESP_LOGI(TAG, "2esp_netif_get_nr_of_ifs = %d", esp_netif_get_nr_of_ifs()); // should be 2, one AP and one STA


    esp_netif_dns_info_t xxx;
    ESP_ERROR_CHECK(esp_netif_get_dns_info(netifAP,ESP_NETIF_DNS_MAIN,&xxx));
    ESP_LOGI(TAG, "DNS MAIN: " IPSTR, IP2STR(&xxx.ip.u_addr.ip4));
    ESP_ERROR_CHECK(esp_netif_get_dns_info(netifAP,ESP_NETIF_DNS_BACKUP,&xxx));
    ESP_LOGI(TAG, "DNS BACKUP: " IPSTR, IP2STR(&xxx.ip.u_addr.ip4));
    ESP_ERROR_CHECK(esp_netif_get_dns_info(netifAP,ESP_NETIF_DNS_FALLBACK,&xxx));
    ESP_LOGI(TAG, "DNS FALLBACK: " IPSTR, IP2STR(&xxx.ip.u_addr.ip4));
//
    //xxx.ip.u_addr.ip4.addr = ESP_IP4TOADDR(192,168,4,199);
    //ESP_ERROR_CHECK(esp_netif_set_dns_info(netifAP,ESP_NETIF_DNS_MAIN,&xxx));
    //ESP_ERROR_CHECK(esp_netif_get_dns_info(netifAP,ESP_NETIF_DNS_MAIN,&xxx));
    //ESP_LOGI(TAG, "DNS MAIN: " IPSTR, IP2STR(&xxx.ip.u_addr.ip4));


    return netifSTA;
}

static void mywifi_stop(void)
{
    esp_netif_t *wifi_netif = myget_example_netif_from_desc("sta");
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &myon_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &myon_got_ip));
#ifdef CONFIG_EXAMPLE_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &myon_got_ipv6));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &myon_wifi_connect));
#endif
    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return;
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif));
    esp_netif_destroy(wifi_netif);
    s_example_esp_netif = NULL;
}
#endif // CONFIG_EXAMPLE_CONNECT_WIFI

//#ifdef CONFIG_EXAMPLE_CONNECT_ETHERNET
///; compile error not using this code?
//#ifdef CONFIG_EXAMPLE_CONNECT_IPV6
//
// * Event handler for Ethernet events 
//static void on_eth_event(void *esp_netif, esp_event_base_t event_base,
//                         int32_t event_id, void *event_data)
//{
//    switch (event_id) {
//    case ETHERNET_EVENT_CONNECTED:
//        ESP_LOGI(TAG, "Ethernet Link Up");
//        esp_netif_create_ip6_linklocal(esp_netif);
//        break;
//    default:
//        break;
//    }
//}
//
//#endif // CONFIG_EXAMPLE_CONNECT_IPV6
//
//static esp_eth_handle_t s_eth_handle = NULL;
//static esp_eth_mac_t *s_mac = NULL;
//static esp_eth_phy_t *s_phy = NULL;
//static void *s_eth_glue = NULL;
//
//static esp_netif_t *eth_start(void)
//{
//    char *desc;
//    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();
//    // Prefix the interface description with the module TAG
//    // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
//    asprintf(&desc, "%s: %s", TAG, esp_netif_config.if_desc);
//    esp_netif_config.if_desc = desc;
//    esp_netif_config.route_prio = 64;
//    esp_netif_config_t netif_config = {
//        .base = &esp_netif_config,
//        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
//    };
//    esp_netif_t *netif = esp_netif_new(&netif_config);
//    assert(netif);
//    free(desc);
//    // Set default handlers to process TCP/IP stuffs
//    ESP_ERROR_CHECK(esp_eth_set_default_handlers(netif));
//    // Register user defined event handers
//    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &myon_got_ip, NULL));
//#ifdef CONFIG_EXAMPLE_CONNECT_IPV6
//    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_CONNECTED, &on_eth_event, netif));
//    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &myon_got_ipv6, NULL));
//#endif
//    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
//    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
//    phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;
//    phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO;
//#if CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
//    mac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;
//    mac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;
//    s_mac = esp_eth_mac_new_esp32(&mac_config);
//#if CONFIG_EXAMPLE_ETH_PHY_IP101
//    s_phy = esp_eth_phy_new_ip101(&phy_config);
//#elif CONFIG_EXAMPLE_ETH_PHY_RTL8201
//    s_phy = esp_eth_phy_new_rtl8201(&phy_config);
//#elif CONFIG_EXAMPLE_ETH_PHY_LAN8720
//    s_phy = esp_eth_phy_new_lan8720(&phy_config);
//#elif CONFIG_EXAMPLE_ETH_PHY_DP83848
//    s_phy = esp_eth_phy_new_dp83848(&phy_config);
//#endif
//#elif CONFIG_ETH_USE_SPI_ETHERNET
//    gpio_install_isr_service(0);
//    spi_device_handle_t spi_handle = NULL;
//    spi_bus_config_t buscfg = {
//        .miso_io_num = CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO,
//        .mosi_io_num = CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO,
//        .sclk_io_num = CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO,
//        .quadwp_io_num = -1,
//        .quadhd_io_num = -1,
//    };
//    ESP_ERROR_CHECK(spi_bus_initialize(CONFIG_EXAMPLE_ETH_SPI_HOST, &buscfg, 1));
//#if CONFIG_EXAMPLE_USE_DM9051
//    spi_device_interface_config_t devcfg = {
//        .command_bits = 1,
//        .address_bits = 7,
//        .mode = 0,
//        .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
//        .spics_io_num = CONFIG_EXAMPLE_ETH_SPI_CS_GPIO,
//        .queue_size = 20
//    };
//    ESP_ERROR_CHECK(spi_bus_add_device(CONFIG_EXAMPLE_ETH_SPI_HOST, &devcfg, &spi_handle));
//    /* dm9051 ethernet driver is based on spi driver */
//    eth_dm9051_config_t dm9051_config = ETH_DM9051_DEFAULT_CONFIG(spi_handle);
//    dm9051_config.int_gpio_num = CONFIG_EXAMPLE_ETH_SPI_INT_GPIO;
//    s_mac = esp_eth_mac_new_dm9051(&dm9051_config, &mac_config);
//    s_phy = esp_eth_phy_new_dm9051(&phy_config);
//#elif CONFIG_EXAMPLE_USE_W5500
//    spi_device_interface_config_t devcfg = {
//        .command_bits = 16, // Actually it's the address phase in W5500 SPI frame
//        .address_bits = 8,  // Actually it's the control phase in W5500 SPI frame
//        .mode = 0,
//        .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
//        .spics_io_num = CONFIG_EXAMPLE_ETH_SPI_CS_GPIO,
//        .queue_size = 20
//    };
//    ESP_ERROR_CHECK(spi_bus_add_device(CONFIG_EXAMPLE_ETH_SPI_HOST, &devcfg, &spi_handle));
//    /* w5500 ethernet driver is based on spi driver */
//    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(spi_handle);
//    w5500_config.int_gpio_num = CONFIG_EXAMPLE_ETH_SPI_INT_GPIO;
//    s_mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
//    s_phy = esp_eth_phy_new_w5500(&phy_config);
//#endif
//#elif CONFIG_EXAMPLE_USE_OPENETH
//    phy_config.autonego_timeout_ms = 100;
//    s_mac = esp_eth_mac_new_openeth(&mac_config);
//    s_phy = esp_eth_phy_new_dp83848(&phy_config);
//#endif
//
//    // Install Ethernet driver
//    esp_eth_config_t config = ETH_DEFAULT_CONFIG(s_mac, s_phy);
//    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &s_eth_handle));
//#if !CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
//    /* The SPI Ethernet module might doesn't have a burned factory MAC address, we cat to set it manually.
//       02:00:00 is a Locally Administered OUI range so should not be used except when testing on a LAN under your control.
//    */
//    ESP_ERROR_CHECK(esp_eth_ioctl(s_eth_handle, ETH_CMD_S_MAC_ADDR, (uint8_t[]) {
//        0x02, 0x00, 0x00, 0x12, 0x34, 0x56
//    }));
//#endif
//    // combine driver with netif
//    s_eth_glue = esp_eth_new_netif_glue(s_eth_handle);
//    esp_netif_attach(netif, s_eth_glue);
//    esp_eth_start(s_eth_handle);
//    return netif;
//}
//
//static void eth_stop(void)
//{
//    esp_netif_t *eth_netif = myget_example_netif_from_desc("eth");
//    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, &myon_got_ip));
//#ifdef CONFIG_EXAMPLE_CONNECT_IPV6
//    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &myon_got_ipv6));
//    ESP_ERROR_CHECK(esp_event_handler_unregister(ETH_EVENT, ETHERNET_EVENT_CONNECTED, &on_eth_event));
//#endif
//    ESP_ERROR_CHECK(esp_eth_stop(s_eth_handle));
//    ESP_ERROR_CHECK(esp_eth_del_netif_glue(s_eth_glue));
//    ESP_ERROR_CHECK(esp_eth_clear_default_handlers(eth_netif));
//    ESP_ERROR_CHECK(esp_eth_driver_uninstall(s_eth_handle));
//    ESP_ERROR_CHECK(s_phy->del(s_phy));
//    ESP_ERROR_CHECK(s_mac->del(s_mac));
//
//    esp_netif_destroy(eth_netif);
//    s_example_esp_netif = NULL;
//}
//
//#endif // CONFIG_EXAMPLE_CONNECT_ETHERNET

esp_netif_t *myget_example_netif(void)
{
    return s_example_esp_netif; // likely NULL!
}

esp_netif_t *myget_example_netif_from_desc(const char *desc)
{
    esp_netif_t *netif = NULL;
    char *expected_desc;
    asprintf(&expected_desc, "%s: %s", TAG, desc);
    while ((netif = esp_netif_next(netif)) != NULL) {
        if (strcmp(esp_netif_get_desc(netif), expected_desc) == 0) {
            free(expected_desc);
            return netif;
        }
    }
    free(expected_desc);
    return netif;
}










// from https://esp32.com/viewtopic.php?f=13&t=538&start=10
//
//
//
//
//
// ************************************************************
//@Inputs:
//@Outputs:
//@Comments:
// ************************************************************/
//void Start_wifi(wifi_config_t* wifi_config , wifi_mode_t Mode )
//{
//	wifi_mode_t currentMode;
//	if(esp_wifi_get_mode(&currentMode) == ESP_ERR_WIFI_NOT_INIT) // not currently up
//	{
//		wifiSetup_Init(); // leaves esp_wifi_set_mode=0
//	}
//	else if(currentMode == WIFI_MODE_APSTA) // up, in the ultimate mode
//	{
//		ESP_LOGE(TAG,"wifi APSTA already running\n");
//		return;
//	}
//	else if(currentMode == Mode) // needed mode already good
//	{
//		ESP_LOGE(TAG,"that mode is already running\n");
//		return;
//	}
//	else // need to add the other mode...stop first
//	{
//		esp_wifi_stop();
//	}
//
//   	if(currentMode == WIFI_MODE_AP && Mode == WIFI_MODE_STA)
//   	{
//   		ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) ); // adding sta to existing ap
//   	}
//   	else if(currentMode == WIFI_MODE_STA && Mode == WIFI_MODE_AP)
//   	{
//   		ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) ); // adding ap to existing sta
//   	}
//   	else
//   	{
//   		ESP_ERROR_CHECK( esp_wifi_set_mode(Mode) ); // ap or sta, alone, either can be first
//   	}
//	if(Mode == WIFI_MODE_AP)
//	{
//		ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, wifi_config) );
//	}
//	else if(Mode == WIFI_MODE_STA)
//	{
//		ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
//	}
//	ESP_ERROR_CHECK( esp_wifi_start());
//}
// ************************************************************
//@Inputs:
//@Outputs:
//@Comments:
// ************************************************************/
//void wifiSetup_StopSTA(void)
//{
//	wifi_mode_t currentMode;
//	esp_wifi_get_mode(&currentMode);
//	if(currentMode == WIFI_MODE_STA )
//	{
//		esp_wifi_stop();
//		ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) ); // nothing remains
//	}
//	else if (currentMode == WIFI_MODE_APSTA)
//	{
//		ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) ); // ap remains
//	}
//}
/// ***********************************************************
//@Inputs:
//@Outputs:
//@Comments:
// ************************************************************/
//void wifiSetup_StopAP(void)
//{
//	wifi_mode_t currentMode;
//	esp_wifi_get_mode(&currentMode);
//	if(currentMode == WIFI_MODE_AP )
//	{
//		esp_wifi_stop();
//		ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) ); // nothing remains
//	}
//	else if (currentMode == WIFI_MODE_APSTA)
//	{
//		ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) ); // sta remains
//	}
//}
// ***********************************************************
//@Inputs:
//@Outputs:
//@Comments:
// ************************************************************/
//void Kill_wifi(void)
//{
//
//	esp_wifi_stop();
//	//vEventGroupDelete(wifi_event_group);
//	esp_wifi_deinit();
//}
//
// ************************************************************
//@Inputs:
//@Outputs:
//@Comments:
// ************************************************************/
//void wifiSetup_Init(void)
//{
//	ESP_LOGI(TAG,"Initilising TCP and wifi\n");
//	tcpip_adapter_init();
//	wifi_event_group = xEventGroupCreate();
//	if(esp_event_loop_init(event_handler, NULL) == ESP_FAIL)
//	{
//			ESP_LOGI(TAG,"Event Loop not created , may have been created before \n");
//	}
//	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
//	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
//	esp_wifi_set_mode(0);
//}
//
