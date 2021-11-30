/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

// the original server example was modified, a lot, for RedClock. It has this license...
/* HTTP File Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/* DIRECTORY
redirect_to_setup_html_handler -        redirect to the setup page, used by save/cancel/... commands
bufferedhttpd_resp_sendstr_chunk -      buffer send responses - maybe not really needed - moved to clockHtml.c
status_json_handler -                   AJAX JSON response
status_html_handler                     HTML for above
recent_wifi_json_handler                AJAX JSON response
recent_wifi_html_handler                HTML for above
setup_html_handler_real                 main setup HTML page
directory_html_handler -                show the spiffs dir links for downloading, include a script for uploading
set_content_type_from_file -            downloads with content type "do the right thing"
get_path_from_uri -                     URL splitter
download_get_handler -                  choose what page to deliver based on URL, update parms in NVS, or down load a file
upload_post_handler -                   upload file...specialize this to handle the timezone update file outside of spiffs
delete_post_handler -                   delete file
start_file_server -                     ADD MORE HANDLERS for splitting up pages
urldecode2 -                            params with < > etc encoded by browser
loadNVSparms -                          loop through the form data for NVS parm addresses

struct FormData -  define the setup page contents
struct KEYVAL - ditto
struct FormPanel - ditto

struct CharStarParm...Uint8Parm - etc,etc - lists of same data-type parms for a for-loop

define emitjson(x) - helpers to emit html and json via *httpd_resp_send*
define emit(x) - ditto
define emitpairnocomma(key,val) - ditto
define emitpair(key,val) - ditto
define emittime - ditto

static const char cBlueTable - css for the tables on the html pages

*/



#include "clockMain.h"


/* Max length a file path can have on storage */
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN) // 15 + 32

/* Max size of an individual file. Make sure this
 * value is same as that set in upload_script.html */
#define MAX_FILE_SIZE   (1024*1024) // 200 KB. no, 1M
#define MAX_FILE_SIZE_STR "1MB"

/* Scratch buffer size */
#define SCRATCH_BUFSIZE  8192

//////// used by the json writers
#define emitjson(x) do {bufferedhttpd_resp_sendjson_chunk(req,(x));} while(0)
#define emit(x) do {bufferedhttpd_resp_sendstr_chunk(req,(x));} while(0)
#define emitpairnocomma(key,val) do {emit("\n\"");emit((key));emit("\":\"");emitjson((val));emit("\"");} while(0)
#define emitpair(key,val) do {emit(",\n\"");emit((key));emit("\":\"");emitjson((val));emit("\"");} while(0)
#define emittime(unit,code) do {strftime(strftime_buf, sizeof(strftime_buf), (code), &sTimeinfo);emitpair((unit),strftime_buf);} while(0)
////////

/*
I could not figure out how to get the number of items in a static initializer list for an array in a structure.
There are two examples of that here, FormData.vals and FormPanel.keyVals . FormData uses nvals, in the structure, 
just before the array. It is hand counted in the initializer. FormPanel is similar. look for the [] in the struct.

how to add new config parms to the HTML screen: 
clockmain.c and .h define globals like sClockState.cfgWiFiName . Add to the asprintf/mallocs in main.
Use FormData to associate a key name with the global; the keynames need to be unique and
become parameters after the url/?  For text box formdata with zero vals specified, the 
textbox value is the value. For radio buttons, with >0 vals, one of those vals (radio button names) 
should match the sClockState value. Next, make a KEYVAL to represent a line in a form that holds the 
name/value you just made. if it is a radio, the radio will be on multiple lines and needs multiple KEYVALs.
Next, make a FormPanel to combine related stuff; the FormPanel will create the ordering of the KEYVALs.
Finally, add the FormPanel to formPanels. Done!

how to add json dict entries to the HTML status screen:
look for emitpair in status_json_handler. use one of the existing lists of typed variables.

how to add a file to the list of files in the HTML directory screen:
any file in the spiffsdata is copied in by "clear;idf.py -p /dev/ttyUSB0 clean flash monitor" from the shell
(don't upload via the html screen, too easy to forget!)
*/

struct FormData { // represents either a name/value input field (router password) or a named radio group
    char const key[8]; // 7 character key, unique across all forms, inserted into form and looked up when form recvd back; a bit human readable
    char *(* const currentValue)[2]; // this is malloc memory; the sClockState holds the ptr to the malloc block
    short nvals; // this is needed for walking the structure for lookups
    char const vals[][8]; // only exist for the radio values; nvals=0 is for a text box
};

static struct FormData const textWifiName = {"wfiname",&sClockState.cfgWiFiName,0,{}}; 
static struct FormData const textWifiPassword = {"wfipass",&sClockState.cfgWiFiPass,0,{}};

static struct FormData const textMyFiName = {"myname",&sClockState.cfgMyFiName,0,{}}; 
static struct FormData const textMyFiPassword = {"mypass",&sClockState.cfgMyFiPass,0,{}};

static struct FormData const textBrightHigh = {"brthigh",&sClockState.cfgBrthigh,0,{}}; // goes with bright radio
static struct FormData const textBrightLow = {"brtlow",&sClockState.cfgBrtlow,0,{}};
static struct FormData const textBrightDawn = {"brtdawn",&sClockState.cfgBrtdawn,0,{}};
static struct FormData const textBrightDusk = {"brtdusk",&sClockState.cfgBrtdusk,0,{}};

static struct FormData const textTimeSourceURL= {"sntpurl",&sClockState.cfgSntpUrl,0,{}}; // goes with tzone radio
//static struct FormData const textTimezoneOffset = {"tzoffse",&sClockState.cfgTzoffset,0,{}}; // goes with tzone radio
static struct FormData const textTimezoneRule = {"tzrule",&sClockState.cfgTzrule,0,{}};

static struct FormData const radioWifi = {"wifi",&sClockState.cfgWiFi, 2,{"on","off"} }; // radio to disable wifi
static struct FormData const radioBright = {"bright",&sClockState.cfgBright, 4,{"sense","time","max","min"} };
static struct FormData const radioClock = {"clock",&sClockState.cfgClock, 3,{"gps","sntp","open"} }; // could add fields for sntp server and neighbor wifi
static struct FormData const radioTZone = {"tzone",&sClockState.cfgTzone, 2,{"lookup","rule"} };
static struct FormData const radioDisplay = {"display",&sClockState.cfgDisplay, 2,{"civ","mil"} };

static struct FormData const textLocationNote = {"locnote",&sClockState.cfgLocationNote,0,{}};

static struct FormData const radioNeighborHoodWatch = {"nwatch",&sClockState.cfgNeighborHoodWatch, 8,{"0","1","2","4","8","16","24","32"} };

// this list of FormData is used for save/resore/POST(save,reset,cancel) which all need to
// work through the pairs, once. I started out using the formPanels[], but it struggles
// because the radios are in there multiple times (and has an extra loop over panels.)
static struct FormData const * const formDatas[] = {&textWifiName,&textWifiPassword,&textMyFiName,&textMyFiPassword,
        &textBrightHigh,&textBrightLow,&textBrightDawn,&textBrightDusk,
        &textTimeSourceURL,/*&textTimezoneOffset,*/&textTimezoneRule,&textLocationNote,
        &radioWifi,&radioBright,&radioClock,&radioTZone,&radioNeighborHoodWatch,&radioDisplay
        };


enum INPUTTYPE {inputText,inputPassword,inputRadio,inputNothing};// nothing is informative text without input capability

struct KEYVAL { // this is a line in a form, either a single text box, or a single radio button
    enum INPUTTYPE const inputType;
    short const nthRadioLine; // zero for text, zero for 1st radio line
    short const textSize; // width of text input field in characters
    struct FormData const * const formData; // a text box or a radio button. nthRadioLine chooses the button.
    char const * const title; // tool tip
    char const * const label; // visible instructions 
};

static struct KEYVAL const kvWifiName = { inputText,0,15,&textWifiName,"Using a wifi is needed if SNTP (or HTML date header) needs to fetch the time. 31 chars max.","Router" };
static struct KEYVAL const kvWifiPass = { inputPassword,0,15,&textWifiPassword,"WPA passwords are 8-63 chars long. The single dot represents the current pw. Leave empty for open WIFI with no WPA password. That is typical of insecure public WIFI routers with a sign-on screen. They might still give the time if HTML date header is selected for Time Source.","Password" };

static struct KEYVAL const kvMyFiName = { inputText,0,15,&textMyFiName,"My internal access point (router) name, 31 chars max. Used for configuration or downloads; it is not bridged to your router.", "AccessPoint" };
static struct KEYVAL const kvMyFiPass = { inputPassword,0,15,&textMyFiPassword,"WPA passwords are 8-63 chars long. The single dot represents the current pw.","Password" };

static struct KEYVAL const kvBrightMax = { inputText,0,5,&textBrightHigh,"Maximum brightness when sensor maxed out or during dawn to dusk.",
"High (prefer 7000)" };
static struct KEYVAL const kvBrightMin = { inputText,0,5,&textBrightLow,"Minimum brightness when sensor dark or during dusk to dawn.",
"Low (prefer 3000)" };
static struct KEYVAL const kvBrightDawn = { inputText,0,5,&textBrightDawn,"Pick Dawn/Dusk above, set the High and Low values. Go bright (high) at this time.",
"Dawn (typically 0700)" };
static struct KEYVAL const kvBrightDusk = { inputText,0,5,&textBrightDusk,"Go dim (low) at this time",
"Dusk (typically 1900)" };
//static struct KEYVAL const kvTzOffset = { inputText,0,5,&textTimezoneOffset,"Offset in minutes: EST is 300 for 5 hours.",
//"Offset minutes (0 = UTC)" };
static struct KEYVAL const kvTzRule = { inputText,0,30,&textTimezoneRule,"Best bet is to look this up. RedClock is using rules from 2020 era. This value is used before the GPS gets a position, so good idea to specify it correctly anyway.","Rule" };
static struct KEYVAL const kvUrlSNTP = { inputText,0,15,&textTimeSourceURL,"pool.ntp.org is a good choice in 2020.",
"Server" };

static struct KEYVAL const kvLocationNote = { inputText,0,30,&textLocationNote,"A small note, typically like \"Room 720\" or \"Kitchen Clock\"","Location Note" };


// the radios, defined here, but not positioned yet
static struct KEYVAL const kvRadioWifi[] = {
    { inputRadio,0,0,&radioWifi,"The Simple Network Time Protocol can get the time from the internet. Turning wifi on also allows configuration via your router, below.","on - allow SNTP or HTML" },
    { inputRadio,1,0,&radioWifi,"Turning off isolates RedClock from internet. The GPS will try to get the time.","off - only GPS" }
};

static struct KEYVAL const kvRadioBright[] = {
    { inputRadio,0,0,&radioBright,"There is a CDS photo sensor that RedClock can use to automatically dim the clock when the lights are dim, etc. The numbers below determine how dim/bright RedClock is when the sensor is at its limits. 3000 to 7000 is linear and covers almost all of the range. 0 to 10000 is the total range and adds a little more on the ends. The clock may flicker in the extended range.","High to Low using sensor" },
    { inputRadio,1,0,&radioBright,"RedClock can dim the clock at night (dusk to dawn) and brighten in the day (dawn to dusk). Set the time, below, and the brightness, above.","Use dawn and dusk..." },
    { inputRadio,2,0,&radioBright,"Clock intensity will always be the high value, above. >5000 would be pretty intense in the dark.","Always use High" },
    { inputRadio,3,0,&radioBright,"Clock intensity will always be the low value, above. <3000 won&apos;t be visible in the day time.","Always use Low" }
};


static struct KEYVAL const kvRadioNeighborHoodWatch[] = {
    { inputRadio,0,0,&radioNeighborHoodWatch,"Turn off the scan for SSIDs -- the WIFIs in your neighborhood.","0" },
    { inputRadio,1,0,&radioNeighborHoodWatch,"Scan 1/32. Allows more time for sending status and recent web page updates.","1" },
    { inputRadio,2,0,&radioNeighborHoodWatch,"Scan 2/32. Default. Eventually catches most SSIDs.","2" },
    { inputRadio,3,0,&radioNeighborHoodWatch,"Scan 4/32. Spend a little more time watching for WIFIs.","4" },
    { inputRadio,4,0,&radioNeighborHoodWatch,"Scan 8/32. The more time looking for WIFIs, the less time for updating recent/status.","8" },
    { inputRadio,5,0,&radioNeighborHoodWatch,"Scan 16/32. Catches most SSIDs sooner. Rare finds may dribble in for days.","16" },
    { inputRadio,6,0,&radioNeighborHoodWatch,"Scan 24/32. May impact clock performance; Status/etc page may lag behind.","24" },
    { inputRadio,7,0,&radioNeighborHoodWatch,"scan 32/32. May impact clock performance; Recent/etc page may lag behind.","32" }
};


static struct KEYVAL const kvRadioClock[] = {
    { inputRadio,0,0,&radioClock,"RedClock always uses GPS if there is a signal. Updates once a second. Set this to prevent SNTP and HTML date header.",
    "GPS only" },
    { inputRadio,1,0,&radioClock,"RedClock uses SNTP if signed in to a router. Updates every two hours. GPS will override immediately if available. Set this to allow GPS and SNTP and prevent HTML date header. Recommended/default.",
    "GPS or SNTP" },
    { inputRadio,2,0,&radioClock,"RedClock might be able to get the time from the sign-in page on a non-WPA WIFI router; it might not be accurate and RedClock only re-asks once an hour. Set this to allow using HTML date header; SNTP or GPS will set a more accurate time if available.",
    "GPS, SNTP or HTML date header" }
};

//
// https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// remove the offset and just use the rule.
//
static struct KEYVAL const kvRadioTZone[] = {
    { inputRadio,0,0,&radioTZone,"RedClock has an internal table to turn GPS locations into posix timezone strings. The table may be wrong within a few miles of a timezone boundary; you might need to specify below. Data from 2019-2020 era.",
    "GPS lookup (best if it works)" },
//    { inputRadio,1,0,&radioTZone,"If you want UTC time, or a fixed offset from UTC, but NO daylight time change, use this.",
//    "Offset from UTC, no daylight" },
    { inputRadio,1,0,&radioTZone,"The rule string is complicated because it specifies when daylight time starts and ends (nth sunday at 2am, etc). Clock firmware is from 2020 era. Select this and specify the rule string below if the built in GPS table gets the wrong answer.",
    "<a target='_blank' href='https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv'>Posix Timezone Rule</a>" }
};


static struct KEYVAL const kvRadioDisplay[] = {
    { inputRadio,0,0,&radioDisplay,"There is no AM/PM indicator; look out a window.","12 hour civilian" },
    { inputRadio,1,0,&radioDisplay,"00:01 is just after midnight; 23:59 is just before midnight.","24 hour military" }
};



// the FormPanels are assembled here, with the text and radio lines in desired interleaved order...

struct FormPanel { // the panel box around a form
    char const * const legend;
    int const nKeyVals;
    struct KEYVAL const * const keyVals[];
};

static struct FormPanel const fpWIFI = {"WIFI",4, {&kvRadioWifi[1],&kvRadioWifi[0],&kvWifiName,&kvWifiPass}};
static struct FormPanel const fpMYFI = {"MYFI",2, {&kvMyFiName,&kvMyFiPass}};

static struct FormPanel const fpBright = {"Brightness",8, {
            &kvRadioBright[0],// radio
            &kvBrightMax, // text
            &kvBrightMin, 
            &kvRadioBright[1], 
            &kvBrightDawn,
            &kvBrightDusk, 
            &kvRadioBright[2], 
            &kvRadioBright[3]
            }};

static struct FormPanel const fpNeighborHoodWatch = {"Recent WIFI/SSID watcher",8, {
            &kvRadioNeighborHoodWatch[0],
            &kvRadioNeighborHoodWatch[1], 
            &kvRadioNeighborHoodWatch[2], 
            &kvRadioNeighborHoodWatch[3], 
            &kvRadioNeighborHoodWatch[4], 
            &kvRadioNeighborHoodWatch[5], 
            &kvRadioNeighborHoodWatch[6], 
            &kvRadioNeighborHoodWatch[7]
            }};

static struct FormPanel const fpTimeSource = {"Time source",4, {&kvRadioClock[0],&kvRadioClock[1],&kvUrlSNTP,&kvRadioClock[2]}};

static struct FormPanel const fpTimeZone = {"Timezone",3,{&kvRadioTZone[0],&kvRadioTZone[1],&kvTzRule}};
static struct FormPanel const fpDisplay = {"Display",2,{&kvRadioDisplay[0],&kvRadioDisplay[1]}};

static struct FormPanel const fpLocationNote = {"Location",1,{&kvLocationNote}};

// put the form panels in an array for processing

static struct FormPanel const * const formPanels[] = {&fpMYFI,&fpWIFI,&fpBright,&fpDisplay,&fpTimeSource,&fpTimeZone,&fpNeighborHoodWatch,&fpLocationNote};


////////

struct file_server_data {
    /* Base path of file storage */
    char base_path[ESP_VFS_PATH_MAX + 1];

    /* Scratch buffer for temporary storage during file transfer */
    char scratch[SCRATCH_BUFSIZE];
};

static const char *TAG = "file_server";

// redirect back to setup page
static esp_err_t redirect_to_setup_html_handler(httpd_req_t *req)
{
    httpd_resp_set_status(req, "307 Temporary Redirect");
    httpd_resp_set_hdr(req, "Location", "/setup.html");
    httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}

//static char sEpochTime[16];
static char sUpTime[16];

struct CharStarParm {
    char const *name;
    char **variable;
};
static struct CharStarParm sCharStarParm[] = {
    {"router_name",&(sClockState.cfgWiFiName[0])},
    {"access_point_name",&(sClockState.cfgMyFiName[0])},
    {"location_note",&(sClockState.cfgLocationNote[0])},
    {"timezone_build_date",(char **)&sClockState.timezoneBuildDate},
    {"chip_model",&sClockState.chip_infoModel}
};

struct CharParm {
    char const *name;
    char *variable;
};
static struct CharParm sCharParm[] = {
    {"clock_up",sUpTime},
    {"station_ip",sClockState.ip_addr_txt},
    {"base_mac_addr",sClockState.baseMacAddress},
    {"access_point_gateway", sClockState.accesspoint_gateway_txt},
    {"gps_last_reported_time",sClockState.gpstime},
    {"gps_last_reported_date",sClockState.gpsdate},
    {"serial_number",sClockState.serialnumber},
    {"app_desc_project_name",sClockState.app_desc.project_name},
    {"app_desc_version",sClockState.app_desc.version},
    {"app_desc_time",sClockState.app_desc.time},
    {"app_desc_date",sClockState.app_desc.date},
    {"app_desc_idf_ver",sClockState.app_desc.idf_ver},
    {"gps_firmware",sClockState.gpsGPTXT},
    {"GPGLL",sClockState.gpsGPGLL},
    {"GPRMC",sClockState.gpsGPRMC},
    {"GPVTG",sClockState.gpsGPVTG},
    {"GPGGA",sClockState.gpsGPGGA},
    {"GPGSA",sClockState.gpsGPGSA}
};

struct DoubleParm {
    char const *name;
    char const format[8];
    double *variable;
};
static struct DoubleParm sDoubleParm[] = {
    {"lat","%1.6f",&sClockState.lat},
    {"lon","%1.6f",&sClockState.lon},
    {"ambient","%1.1f",&sClockState.smoothTrueAmbient},
    {"temperature","%1.1f",&sClockState.temperature},
    {"pressure","%1.1f",&sClockState.pressure},
    {"humidity","%1.1f",&sClockState.humidity}

};

struct UintParm {
    char const *name;
    char const format[4];
    unsigned int *variable;
};
static struct UintParm sUintParm[] = {
    {"router_disconnects","%u",&sClockState.nDisconnects},
    {"chip_features","%u",&sClockState.chip_info.features},
    {"cpu_frequency","%u",&sClockState.rcf.freq_mhz} 
};

struct IntParm {
    char const *name;
    char const format[4];
    int *variable;
};
static struct IntParm sIntParm[] = {
    {"ondelay","%d",&sClockState.onDelay},
    {"offdelay","%d",&sClockState.offDelay},
    {"crystal_frequency","%d",&sClockState.xtalfreq}
};

struct Uint8Parm {
    char const *name;
    char const format[4];
    uint8_t *variable;
};
static struct Uint8Parm sUint8Parm[] = {
    {"chip_revision","%u",&sClockState.chip_info.revision},
    {"chip_cores","%u",&sClockState.chip_info.cores}
};

static unsigned int getChannel(){
    uint8_t primary;
    wifi_second_chan_t second;
    esp_wifi_get_channel(&primary, &second);
    return primary;
}

static struct timeval sTime_now;
static unsigned int epochtime() {
    gettimeofday(&sTime_now, NULL);
// https://www.delftstack.com/howto/cpp/how-to-get-time-in-milliseconds-cpp/
//    time_t msecs_time = (sTime_now.tv_sec * 1000) + (sTime_now.tv_usec / 1000);
//    cout << "seconds since epoch: " << sTime_now.tv_sec << endl;
//    cout << "milliseconds since epoch: "  << msecs_time << endl << endl;

   // printf("gettimeofday=%ld+%ld/1e6 time=%ld\n",sTime_now.tv_sec,sTime_now.tv_usec,time(NULL));
    
    return sTime_now.tv_sec;

// was:    return (unsigned int)time(NULL); // the 32-bit *signed* time, unsigned for the CallParm data structure below
}
static unsigned int epochmicro() {
    return sTime_now.tv_usec;
}

// rework avail memory https://www.esp32.com/viewtopic.php?f=19&t=23609
// total_free_bytes must be called first to init...
static multi_heap_info_t sMHIT;
static unsigned int total_free_bytes() { // Total free bytes in the heap. Equivalent to multi_free_heap_size().
    heap_caps_get_info(&sMHIT, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    return sMHIT.total_free_bytes;
}
static unsigned int total_allocated_bytes() { // Total bytes allocated to data in the heap.
    return sMHIT.total_allocated_bytes;
}
static unsigned int largest_free_block() { // Size of largest free block in the heap. This is the largest malloc-able size.
    return sMHIT.largest_free_block;
}
static unsigned int minimum_free_bytes() { // Lifetime minimum free heap size. Equivalent to multi_minimum_free_heap_size().
    return sMHIT.minimum_free_bytes;
}
static unsigned int allocated_blocks() { // Number of (variable size) blocks allocated in the heap.
    return sMHIT.allocated_blocks;
}
static unsigned int free_blocks() { // Number of (variable size) free blocks in the heap.
    return sMHIT.free_blocks;
}
static unsigned int total_blocks() { // Total number of (variable size) blocks in the heap.
    return sMHIT.total_blocks;
}





struct CallParm {
    char const *name;
    char const format[4];
    unsigned int (*callee)();
};
static struct CallParm sCallParm[] = { // https://www.esp32.com/viewtopic.php?f=19&t=23609&p=84887#p84887 has heap_caps_get_info which includes largest free block; that should replace esp_get_free_heap_size (I only have internal heap?)
    {"available_heap","%u",esp_get_free_heap_size},
    {"available_internal_heap","%u",esp_get_free_internal_heap_size},
    {"minimum_heap","%u",esp_get_minimum_free_heap_size},

    {"heap_caps_total_free_bytes","%u",total_free_bytes}, // must be before the following to init the static struct
    {"heap_caps_total_allocated_bytes","%u",total_allocated_bytes},
    {"heap_caps_largest_free_block","%u",largest_free_block},
    {"heap_caps_minimum_free_bytes","%u",minimum_free_bytes},
    {"heap_caps_allocated_blocks","%u",allocated_blocks},
    {"heap_caps_free_blocks","%u",free_blocks},
    {"heap_caps_total_blocks","%u",total_blocks},



    {"wifi_channel","%u",getChannel},
    {"epoch_time","%u",epochtime}, // this is a time_t, but unsigned *might* go past 2038
    {"epoch_micro","%d",epochmicro} // this is a long and 'could' be negative
};
    
struct FlagParm {
    char const *name;
    char const format[4];
    unsigned int *variable;
    unsigned int flag;
};
static struct FlagParm sFlagParm[] = {
    {"chip_features_embedded_flash","%u",&sClockState.chip_info.features,CHIP_FEATURE_EMB_FLASH},
    {"chip_features_wifi_bgn","%u",&sClockState.chip_info.features,CHIP_FEATURE_WIFI_BGN},
    {"chip_features_bluetooth_le","%u",&sClockState.chip_info.features,CHIP_FEATURE_BLE},
    {"chip_features_bluetooth_classic","%u",&sClockState.chip_info.features,CHIP_FEATURE_BT}
};

struct TimeParm {
    char const *name;
    char const *format;
};
static struct TimeParm sTimeParm[] = {
    {"time","%l:%M:%S %p %Z"},
    {"day_of_week_name","%A"},
    {"day_of_week_number","%w"},
    {"month_name","%B"},
    {"month_number","%m"},
    {"day_of_month","%d"},
    {"year","%Y"},
    {"hour_24","%H"},//leading zero
    {"hour_12","%l"},//leading blank
    {"AM_PM","%p"},
    {"minutes","%M"},
    {"seconds","%S"},
    {"timezone","%Z"},
    {"timezone_offset","%z"},
    {"day_of_year","%j"},
    {"ISO_week","%V"}
};

struct ScaledTimeParm {
    char const *name;
    char const format[8];
    uint64_t *prevGetTime;
};
static struct ScaledTimeParm sScaledTimeParm[] = {
    {"seconds_since_definitive","%llu",&sClockState.tickSecondOfDefinitiveTime},
    {"seconds_since_definitiveGPS","%llu",&sClockState.tickSecondOfDefinitiveTimeGPS},
    {"seconds_since_definitiveSNTP","%llu",&sClockState.tickSecondOfDefinitiveTimeSNTP}
};
static struct tm sTimeinfo;

static char *fixupZero(char *in,bool comma) {
    static char out[16];
    strcpy(out,comma?",":"");// out is initialized, either to ,nul or nul
    if(strlen(in)){// there is some non-nul text
        while(*in == '0')
            in+=1; // json chokes on leading zeros
        if(strlen(in))
            strlcat(out,in,sizeof out);// there is text remaining after removing leading zeros
        else
            strlcat(out,"0",sizeof out);// restore a zero
    }
    else {// the txt was nul, missing value...
        strlcat(out,"0",sizeof out);// I think this is OK; snr is ~20 for low in sky, ~40 for high, "" for very low. change to zero.
    }
    return out;
}

static void emitsats(httpd_req_t *req){
    // the GPS satellite structure in struct SATRECORD satRecords[SatsPerGSV*MaxGSVRecs] sClockState.satRecords
    // prn,el,az,snr
    const int BUFSIZE=16+DIM(sClockState.satRecords)*8;
    char *prnbuffer = (char *)malloc(BUFSIZ);// 8 is 6 and a comma and ?
    strcpy(prnbuffer,",\n\"sat_prn\":[");
    char *elbuffer = (char *)malloc(BUFSIZE);
    strcpy(elbuffer,",\n\"sat_el\":[");
    char *azbuffer = (char *)malloc(BUFSIZE);
    strcpy(azbuffer,",\n\"sat_az\":[");
    char *snrbuffer = (char *)malloc(BUFSIZE);
    strcpy(snrbuffer,",\n\"sat_snr\":[");
    
//    bool gotit = xSemaphoreTake(sClockState.satSemaphore, 1000 / portTICK_RATE_MS/*portMAX_DELAY block forever*/ ) == pdTRUE;
//    if(!gotit) printf("clockServer.c did not get satSemaphore\n"); else printf("clockServer.c running\n");
    
    for(int i=0;i<DIM(sClockState.satRecords);i+=1){
        if(sClockState.satRecords[i].prn[0]==0)
            break;
        strlcat(prnbuffer,fixupZero(sClockState.satRecords[i].prn,i>0),BUFSIZE);
        strlcat(elbuffer,fixupZero(sClockState.satRecords[i].el,i>0),BUFSIZE);
        strlcat(azbuffer,fixupZero(sClockState.satRecords[i].az,i>0),BUFSIZE);
        strlcat(snrbuffer,fixupZero(sClockState.satRecords[i].snr,i>0),BUFSIZE);
    }
//    printf("clockServer.c done\n");
//    xSemaphoreGive(sClockState.satSemaphore);
    
    strlcat(prnbuffer,"]",BUFSIZE);
    strlcat(elbuffer,"]",BUFSIZE);
    strlcat(azbuffer,"]",BUFSIZE);
    strlcat(snrbuffer,"]",BUFSIZE);
    emit(prnbuffer);
    emit(elbuffer);
    emit(azbuffer);
    emit(snrbuffer);
    free(prnbuffer);
    free(elbuffer);
    free(azbuffer);
    free(snrbuffer);
}

static esp_err_t status_json_handler( httpd_req_t *req ) {
    const char *quest = strchr(req->uri, '?'); // status.json?time
    bool onlyTime = quest && strcmp(quest,"?time")==0;
    char strftime_buf[64];
    httpd_resp_set_type(req, "application/json");
    
    emit( "{\n\"version\":\"1\""); // after this, all pairs begin with a comma
    
    time_t now;
    time(&now); // TZ was set in main, once
    localtime_r(&now, &sTimeinfo); // _r is the threadsafe version, now, a time_t, unpacked into sTimeinfo, a tm

    for(int i = 0; i < DIM(sTimeParm); i+=1 ){
        emittime(sTimeParm[i].name, sTimeParm[i].format);
    }
    
    //sprintf(sEpochTime,"%d",(int)time(NULL));
    uint64_t t = esp_timer_get_time()/1000000;
    int d = (int)(t/(24*60*60));
    t = t - d * 24*60*60;
    int h = (int)(t/(60*60));
    t = t - h * 60*60;
    int m = (int)(t/60);
    t = t - m * 60;
    int s = (int)t;
    
    sprintf(sUpTime,"%dd %dh %dm %ds",d,h,m,s);
    if(onlyTime){
        emitpair(sCharParm[0].name,sCharParm[0].variable);//uptime
    }
    else {
        for(int i = 0; i<DIM(sCharStarParm); i+=1){
            emitpair(sCharStarParm[i].name,*sCharStarParm[i].variable);
        }
        for(int i = 0; i<DIM(sCharParm); i+=1){
            //DumpHex(sCharParm[i].variable, 0, sizeof(sClockState.gpsGPGLL));
            emitpair(sCharParm[i].name,sCharParm[i].variable);
        }
        for(int i = 0; i<DIM(sDoubleParm); i+=1){
            sprintf(strftime_buf,sDoubleParm[i].format,*sDoubleParm[i].variable);
            emitpair(sDoubleParm[i].name,strftime_buf);
        }
        for(int i = 0; i<DIM(sUintParm); i+=1){
            sprintf(strftime_buf,sUintParm[i].format,*sUintParm[i].variable);
            emitpair(sUintParm[i].name,strftime_buf);
        }
        for(int i = 0; i<DIM(sIntParm); i+=1){
            sprintf(strftime_buf,sIntParm[i].format,*sIntParm[i].variable);
            emitpair(sIntParm[i].name,strftime_buf);
        }
        for(int i = 0; i<DIM(sUint8Parm); i+=1){
            sprintf(strftime_buf,sUint8Parm[i].format,*sUint8Parm[i].variable);
            emitpair(sUint8Parm[i].name,strftime_buf);
        }
        for(int i = 0; i<DIM(sFlagParm); i+=1){
            sprintf(strftime_buf,sFlagParm[i].format,( *sFlagParm[i].variable & sFlagParm[i].flag ) != 0);
            emitpair(sFlagParm[i].name,strftime_buf);
        }
        for(int i = 0; i<DIM(sCallParm); i+=1){
            sprintf(strftime_buf,sCallParm[i].format,sCallParm[i].callee());
            emitpair(sCallParm[i].name,strftime_buf);
        }
        for(int i = 0; i<DIM(sScaledTimeParm); i+=1){
            sprintf(strftime_buf,sScaledTimeParm[i].format,esp_timer_get_time()/1000000 - *sScaledTimeParm[i].prevGetTime);
            emitpair(sScaledTimeParm[i].name,strftime_buf);
        }
        
        emitsats(req);
        /*
        typedef enum {
            RTC_CPU_FREQ_XTAL = 0,      //!< Main XTAL frequency
            RTC_CPU_FREQ_80M = 1,       //!< 80 MHz
            RTC_CPU_FREQ_160M = 2,      //!< 160 MHz
            RTC_CPU_FREQ_240M = 3,      //!< 240 MHz
            RTC_CPU_FREQ_2M = 4,        //!< 2 MHz
        } rtc_cpu_freq_t;
        */
            //static char frqtab[][4] = {"XTL","80","160","240","2Mh"};
            //rtc_xtal_freq_t freq = rtc_clk_xtal_freq_get();// 40
        //    sprintf(strftime_buf,"%d",sClockState.xtalfreq);
        //    emitpair("crystal_frequency",strftime_buf); // the rtc_xtal_freq_t enum is the value sClockState.freq
        //    
        /*
        typedef struct rtc_cpu_freq_config_s {
            rtc_cpu_freq_src_t source;      //!< The clock from which CPU clock is derived
            uint32_t source_freq_mhz;       //!< Source clock frequency
            uint32_t div;                   //!< Divider, freq_mhz = source_freq_mhz / div
            uint32_t freq_mhz;              //!< CPU clock frequency
        } rtc_cpu_freq_config_t;
        */        
    }
    emit( "\n}");
    emit( NULL);
    return ESP_OK;
}

static const char cBlueTable[] = {
        "<style>"
//            "form {"
//                "vertical-align: top;" // allow the forms to flow across and 
//                "display: inline-block;" // stick to top
//            "}"
//            "input {"
//                "margin-top: 5px;" // space between the text fields
//                "margin-bottom: 1px;"
//            "}"
//            "button {"
//                "margin-top: 15px;" // space above and
//                "margin-left: 5px;" // between the
//                "margin-right: 5px;" // button pair
//            "}"
//            ".tg  {border-collapse:collapse;border-spacing:0;}"
//            ".tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;"
//            "  overflow:hidden;padding:10px 5px;word-break:normal;}"
//            ".tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;"
//            "  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}"
//            ".tg .tg-0lax{text-align:left;vertical-align:top}"
//            ".tg .tg-sjuo{background-color:#C2FFD6;text-align:left;vertical-align:top}"
            
// https://html-css-js.com/html/generator/form/ -- not using for status            
// https://divtable.com/table-styler/           -- the blue table...pretty cool with even/odd support and minimal gradient
"table.blueTable {"
"  font-family: Arial, Helvetica, sans-serif;"
"  border: 1px solid #1C6EA4;"
"  background-color: #FFFFFF;"
//"  width: 100%;"
"  text-align: left;"
"  border-collapse: collapse;"
"}"
"table.blueTable th {"
"  border: 1px solid #AAAAAA;"
"  padding: 3px 3px;" // more
"}"
"table.blueTable td {"
"  border: 1px solid #AAAAAA;"
"  padding: 1px 3px;" // less
"  white-space: pre;" // for the gps_firmware glob with embedded \n  (maybe should be in tbody?)
"}"
"table.blueTable tbody td {"
"  font-size: 13px;"
"  color: #000000;"
"}"
"table.blueTable tr:nth-child(even) {"
"  background: #E0F4FF;"
"}"
"table.blueTable thead {"
"  background: #1C6EA4;"
"  background: -moz-linear-gradient(top, #5592bb 0%, #327cad 66%, #1C6EA4 100%);"
"  background: -webkit-linear-gradient(top, #5592bb 0%, #327cad 66%, #1C6EA4 100%);"
"  background: linear-gradient(to bottom, #5592bb 0%, #327cad 66%, #1C6EA4 100%);"
"  border-bottom: 2px solid #444444;"
"}"
"table.blueTable thead th {"
"  font-size: 15px;"
"  font-weight: bold;"
"  color: #FFFFFF;"
"  text-align: left;"
"  border-left: 2px solid #D0E4F5;"
"}"
"table.blueTable thead th:first-child {"
"  border-left: none;"
"}"
"table.blueTable tfoot td {"
"  font-size: 14px;"
"}"
"table.blueTable tfoot .links {"
"  text-align: right;"
"}"
"table.blueTable tfoot .links a{"
"  display: inline-block;"
"  background: #1C6EA4;"
"  color: #FFFFFF;"
"  padding: 2px 8px;"
"  border-radius: 5px;"
"}"
            
            
        "</style>"
};

static esp_err_t status_html_handler( httpd_req_t *req ) {
//    char strftime_buf[64];

    /* Send HTML file header */
    emit( "<!DOCTYPE html>"
    "<head><meta charset='UTF-8'><title>RedClock Status</title>"

        "<script src='js/jquery-3.5.1.min.js' integrity='sha512-bLT0Qm9VnAYZDflyKcBaQ2gg0hSYNQrJ8RilYldYQ1FxQYoCLtUjuuRuZo+fjqhx/qtq/1itJ0C2ejDxltZVFg==' crossorigin='anonymous'></script>"
    );

emit(cBlueTable);

emit(
    "</head><html>"
    "<body style='background-color: white'>"
    "<span style='font-size:300%;'>RedClock Status <span id='clocktime' style='color: #cc3333;'>clocktime</span> </span>"
    );

    emit("<div>");

//#define ADDROW1(xxx) "<tr><td class='tg-0lax'>" #xxx "</td><td class='tg-0lax'><span id='" #xxx "'>" "000000" "</span> </td></tr>"
//#define ADDROW0(xxx) "<tr><td class='tg-sjuo'>" #xxx "</td><td class='tg-sjuo'><span id='" #xxx "'>" "000000" "</span> </td></tr>"

    emit(
    "<br>"
    "<div>" // extras that also slide about
        "<span style='font-size:75%;'>"
        "Lat(<span id='clocklat'>clocklat</span>) " // labels on link <a> tag
        "Lon(<span id='clocklon'>clocklon</span>) "
        "</span>"
    "<a id='linkToGoogle' target='_blank' href='https://www.google.com/maps/@0,0,5z'>"
//    );
//    sprintf(strftime_buf,"%1.6f,%1.6f,17z'>",sClockState.lat,sClockState.lon); // fill in the parms in the google url, end the open <a> tag
//    emit(strftime_buf);
//    emit(
        " map</a> " // finsh label, close </a> tag
        "<br><span style='font-size:150%;'>up <span id='clock_up'>clock_up</span></span>"
        "<br><span style='font-size:100%;'><a target='_blank' href='https://www.epochconverter.com/'>epoch</a> <span id='epoch_time'>epoch_time</span></span>"

        "<br><span style='font-size:100%;'><span id='router_name'>router</span> "
            "<a target='_blank' href='xx' id='station_ip1'><span id='station_ip2'>station_ip</span></a>"
        "</span>"

        "<br><span style='font-size:100%;'><span id='access_point_name'>my fi name</span>/<span id='serial_number'>serial_number</span> "
            "<a target='_blank' href='xx' id='access_point_gateway1'><span id='access_point_gateway2'>access_point_gateway</span></a>"
        "</span>"

        "<br><br>"
    "</div>" // end of the extras
    "<div>This data is available <a href='/status.json' target='_blank'>here</a></div><br><br>"
    "<table class='blueTable'>"
        "<thead>"
            "<tr><th>Key</th><th>Value</th></tr>"
        "</thead>"
        "<tbody id='status'>"
//            "<tr><td>some</td><td>content</td></tr>"
//            "<tr><td>to be</td><td>removed</td></tr>"
        "</tbody>"
    "</table>"
    );    
    


    


    emit(
    "</div>" // end of the .grid
    );
    



    emit(
        "<script>" 




"var interval = 1000;  // 1000 = 1 second"
"\n"
"function doAjax() { // https://stackoverflow.com/questions/20371695/execute-an-ajax-request-every-second"
    "\n" // https://stackoverflow.com/users/157247/t-j-crowder
    //"console.log('started');"
    "\n"
    "$.ajax({"
        "\n"
        "type: 'GET',"
        "\n"
        "url: 'status.json',"
        "\n"
        "//data: $(this).serialize(),"
        "\n"
        "dataType: 'json',"
        "\n"
        "success: function (data) {"
            "\n"
            "$('#status').empty();"
            "\n"
//            "let even = 0;"
//            "\n"
            "for (let key in data) {"
                "\n"
                "$('#status').append('<tr><td>' + key + '</td><td>' + data[key] + '</td></tr>');"                
//                "if (even==0) {"
//                    "\n"
//                    "even=1;"
//                    "\n"
//                    "$('#status').append('<tr><td class=\"tg-0lax\">' + key + '</td><td class=\"tg-0lax\">' + data[key] + '</td></tr>');"
//                    "\n"
//                "} else {"
//                    "\n"
//                    "even=0;"
//                    "\n"
//                    "$('#status').append('<tr><td class=\"tg-sjuo\">' + key + '</td><td class=\"tg-sjuo\">' + data[key] + '</td></tr>');"                
//                    "\n"
//                "}"    
                "\n"
            "}"
            "\n"
            "$('#clocktime').html(data.time);"
            "\n"
            "$('#clocklat').html(data.lat);"
            "\n"
            "$('#clocklon').html(data.lon);"
            "\n"
            
            "$('#linkToGoogle').attr('href','https://www.google.com/maps/@'+data.lat+','+data.lon+',17z');"
            
            "\n"
            "$('#clock_up').html(data.clock_up);"
            "\n"
            "$('#epoch_time').html(data.epoch_time);"
            "\n"
            "$('#router_name').html(data.router_name);"
            "\n"
            "$('#station_ip1').attr('href', 'http://' + data.station_ip);"
            "\n"
            "$('#station_ip2').html(data.station_ip);"
            "\n"
            "$('#access_point_name').html(data.access_point_name);"
            "$('#serial_number').html(data.serial_number);"
            "\n"
            "$('#access_point_gateway1').attr('href', 'http://' + data.access_point_gateway);"
            "\n"
            "$('#access_point_gateway2').html(data.access_point_gateway);"
            "\n"
        "},"
        "\n"
        "complete: function (data) {"
            "\n"
            "setTimeout(doAjax, interval);"
            "\n"
        "}"
        "\n"
    "});"
    "\n"
"}"
"\n"
"console.log('starting');"
"\n"
"setTimeout(doAjax, interval);"
"\n"

"</script>"
    );
    
// end of a chunk, the commented out bit is the file server code. way below is the close...


    /* Send remaining chunk of HTML file to complete it */
    emit( "</body></html>");

    /* Send empty chunk to signal HTTP response completion */
    emit( NULL);
    return ESP_OK;









}




static esp_err_t recent_wifi_json_handler( httpd_req_t *req ) {
    char strftime_buf[64];
    httpd_resp_set_type(req, "application/json");
    emit( "{\n");
    emitpairnocomma("version","1");
    emitpair("scanrate",sClockState.cfgNeighborHoodWatch[0]);
    emitpair("location_note",sClockState.cfgLocationNote[0]);

    //sCallParm ("available_heap","available_internal_heap","minimum_heap"), sDoubleParm ("ambient","temperature","pressure","humidity")
    
    for(int i = 0; i<DIM(sCallParm); i+=1){
        sprintf(strftime_buf,sCallParm[i].format,sCallParm[i].callee());
        emitpair(sCallParm[i].name,strftime_buf);
    }
    
    for(int i = 0; i<DIM(sDoubleParm); i+=1){
        sprintf(strftime_buf,sDoubleParm[i].format,*sDoubleParm[i].variable);
        emitpair(sDoubleParm[i].name,strftime_buf);
    }

    emitsats(req);
    
    emit(",\n\"wifi\":[\n"); 
    uint64_t now = esp_timer_get_time()/1000000;
    int ndone = 0;
    for(int i = 0; i<DIM(sClockState.scanRecords); i+=1){
        char buf[32];
        static uint8_t emptybssid[6] = {0};
        if(memcmp(emptybssid,sClockState.scanRecords[i].r.bssid,sizeof(emptybssid))!=0 || 
        strlen((char *)sClockState.scanRecords[i].r.ssid)>0 ){
            if(ndone) {
                emit(",");
            }
            ndone+=1;
            emit("{");
            snprintf(buf,sizeof(buf),"%d",ndone);
            emitpairnocomma("n",buf);
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
    WIFI_SECOND_CHAN_NONE = 0,  // *< the channel width is HT20 
    WIFI_SECOND_CHAN_ABOVE,     // *< the channel width is HT40 and the secondary channel is above the primary channel 
    WIFI_SECOND_CHAN_BELOW,     // < the channel width is HT40 and the secondary channel is below the primary channel 
} wifi_second_chan_t;    
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
    
            emitpair("ssid",(char *)sClockState.scanRecords[i].r.ssid);
            snprintf(buf,sizeof(buf),"%02x:%02x:%02x:%02x:%02x:%02x",
                sClockState.scanRecords[i].r.bssid[0],sClockState.scanRecords[i].r.bssid[1],sClockState.scanRecords[i].r.bssid[2],
                sClockState.scanRecords[i].r.bssid[3],sClockState.scanRecords[i].r.bssid[4],sClockState.scanRecords[i].r.bssid[5]);
            emitpair("bssid",buf);
            snprintf(buf,sizeof(buf),"%d",sClockState.scanRecords[i].r.rssi);
            emitpair("sig",buf);
            snprintf(buf,sizeof(buf),"%d",sClockState.scanRecords[i].r.primary);
            emitpair("prime",buf);
            static char c2[][12]={"none (ht20)","above(ht40)","below(ht40)","error 4?!?!"};
            emitpair("second",c2[sClockState.scanRecords[i].r.second]);
            //enum wifi_auth_mode_t
            static char *sAuthmode[] = {
                "📖 OPEN",
                "🔒 WEP",
                "🔒 WPA_PSK",
                "🔒 WPA2_PSK",
                "🔒 WPA_WPA2_PSK",
                "🔒 WPA2_ENTERPRISE",
                "🔒 WPA3_PSK",
                "🔒 WPA2_WPA3_PSK",
                "🔒 WAPI_PSK",
                "invalid"
            };
            emitpair("auth",sAuthmode[sClockState.scanRecords[i].r.authmode]);
            //enum wifi_cipher_type_t
            static char *sCipherType[] = {
                "NONE",
                "WEP40",
                "WEP104",
                "TKIP",
                "CCMP",
                "TKIP_CCMP",
                "AES_CMAC128",
                "SMS4",
                "UNKNOWN"
            };
            emitpair("pairwise",sCipherType[sClockState.scanRecords[i].r.pairwise_cipher]);
            emitpair("group",sCipherType[sClockState.scanRecords[i].r.group_cipher]);
            snprintf(buf,sizeof(buf),"%d",sClockState.scanRecords[i].r.ant);
            emitpair("ant",buf);           
            emitpair("11b",sClockState.scanRecords[i].r.phy_11b ? "1":"0");
            emitpair("11g",sClockState.scanRecords[i].r.phy_11g ? "1":"0");
            emitpair("11n",sClockState.scanRecords[i].r.phy_11n ? "1":"0");
            emitpair("lr",sClockState.scanRecords[i].r.phy_lr ? "1":"0");
            emitpair("wps",sClockState.scanRecords[i].r.wps ? "1":"0");
           // emitpair("ftm_responder",sClockState.scanRecords[i].r.ftm_responder ? "1":"0");
           // emitpair("ftm_initiator",sClockState.scanRecords[i].r.ftm_initiator ? "1":"0");
            snprintf(buf,sizeof(buf),"%.3s",sClockState.scanRecords[i].r.country.cc);
            emitpair("country",buf);
            snprintf(buf,sizeof(buf),"%d",sClockState.scanRecords[i].r.country.schan);
            emitpair("country_schan",buf);
            snprintf(buf,sizeof(buf),"%d",sClockState.scanRecords[i].r.country.nchan);
            emitpair("country_nchan",buf);
            snprintf(buf,sizeof(buf),"%d",sClockState.scanRecords[i].r.country.max_tx_power);
            emitpair("country_xmit_power",buf);
            static char policy[][8]={"auto","manual","error?"};
            emitpair("country_policy",policy[sClockState.scanRecords[i].r.country.policy]);
            snprintf(buf,sizeof(buf),"%lld",now-sClockState.scanRecords[i].time);
            emitpair("age",buf);
            
            emit("}");
        }
    }


    emit( "\n]");

    {    
        char strftime_buf[64];
        time_t now;
        time(&now); // TZ was set in main, once
        localtime_r(&now, &sTimeinfo); // _r is the threadsafe version, now, a time_t, unpacked into sTimeinfo, a tm
        emittime(sTimeParm[0].name, sTimeParm[0].format); // "time
    }
    
    
    emit("\n}");
    emit( NULL);
    return ESP_OK;
}








static void makegauge(httpd_req_t *req, const char *name, const char *id, const char *unit) {
    emit( // careful, embedded emits follow...
        "<div style='display: inline-block;padding-right: 20px;'>\n"
            "<div style='text-align: center;margin-top:30px;'>"); emit(name); emit("</div>\n"
            "<div id='"); emit(id); emit("' class='gauge' style='display: inline-block; --gauge-value:0; width:90px; height:90px;'>\n"
                "<div class='ticks'>\n"
                    "<div class='tithe' style='--gauge-tithe-tick:1;'></div>"
                    "<div class='tithe' style='--gauge-tithe-tick:2;'></div>"
                    "<div class='tithe' style='--gauge-tithe-tick:3;'></div>"
                    "<div class='tithe' style='--gauge-tithe-tick:4;'></div>"
                    "<div class='tithe' style='--gauge-tithe-tick:6;'></div>"
                    "<div class='tithe' style='--gauge-tithe-tick:7;'></div>"
                    "<div class='tithe' style='--gauge-tithe-tick:8;'></div>"
                    "<div class='tithe' style='--gauge-tithe-tick:9;'></div>\n"
                    "<div class='min'></div>"
                    "<div class='mid'></div>"
                    "<div class='max'></div>\n"
                "</div>\n"
                "<div class='tick-circle'></div>\n"
                "<div class='needle'>"
                    "<div class='needle-head'></div>"
                "</div>\n"
                "<div class='labels'>"
                    "<div class='value-label'>");emit(unit);emit("</div>"
                "</div>\n"
            "</div>\n"
        "</div>\n"
    );
}

// dewpoint (from Centigrade T and RH) 243.04*(LN(RH/100)+((17.625*T)/(243.04+T)))/(17.625-LN(RH/100)-((17.625*T)/(243.04+T)))
// https://bmcnoldy.rsmas.miami.edu/Humidity.html

static esp_err_t recent_wifi_html_handler( httpd_req_t *req ) { 
    /* Send HTML file header */
    emit( "<!DOCTYPE html>"
    "<head><meta charset='UTF-8'><title>RedClock Recent WIFI</title>"

        "<script src='js/jquery-3.5.1.min.js' integrity='sha512-bLT0Qm9VnAYZDflyKcBaQ2gg0hSYNQrJ8RilYldYQ1FxQYoCLtUjuuRuZo+fjqhx/qtq/1itJ0C2ejDxltZVFg==' crossorigin='anonymous'></script>"
    );

emit(cBlueTable);


emit("<link rel='stylesheet' href='css/gauge.min.css' />"
    "<STYLE type='text/css'>"
        "div.centertext {"
            "position:absolute;"
            "left:50%;top:50%;"
            "display:flex;"
            "align-items:center;"
            "justify-content:center;"
            "text-align:center;"
            "width:0;height:0;"
        "}"
    "</STYLE>"
    "</head><html>"
    "<body style='background-color: white'>"
    "<span style='font-size:300%;'>RedClock Recent WIFI <span id='clocktime' style='color: #cc3333;'>clocktime</span> </span>"

    "<div>This data is available <a href='/recent_wifi.json' target='_blank'>here</a></div><br><br>"
    "<div id='location_note'></div><br>"
    // bar-graph left-right
//    "<br><div style='display: inline-block;'>scan rate <div style='display: inline-block;'>Off "
//    "<div style='display: inline-block;background-color:green;' id='scanrateLEFT'></div>"
//    "<div style='display: inline-block;background-color:gray;' id='scanrateRIGHT'></div>"
//    " Agressive</div></div>"
"<div style='display: inline-flex;'>"

"<div style='display: inline-block;width:60vw;'>");
makegauge(req, "Temp", "temperatureGauge","°F");
makegauge(req, "Pressure", "pressureGauge","mb");
makegauge(req, "Humidity", "humidityGauge","%RH");
makegauge(req, "Dewpoint", "dewpointGauge","°F");
makegauge(req, "Amb Light", "ambientGauge","☼");
makegauge(req, "Avail Heap", "available_heapGauge","KB");
makegauge(req, "Min Heap", "minimum_heapGauge","KB");
makegauge(req, "WIFI APs", "scannHeardGauge","🖐");//📶📡🗫🗩
//makegauge(req, "Rate", "scanRateGauge","/32");// this one is pointless
makegauge(req, "GPS Sats", "gpsNviewGauge","🖐");//📶📡🗫🗩🐦
makegauge(req, "GPS SNR", "gpsSnrGauge","C/No");//📶📡🗫🗩🐦   SNR is not correct, but suggests what C/N0 means. https://electronics.stackexchange.com/questions/413624/what-exactly-does-c-no-dbhz-mean-in-u-blox-gps-data
emit("</div>");


emit("<div style='display:inline-block;position:relative;'>"
"<div style='text-align: center;'>GPS Sky View (PRNxSNR)</div>\n"
"<div style='display:block;position:relative;background:rgb(45,90,180);border-radius:50%;width:300px;height:300px;font-size:10px;color: white;'>\n"
);
{
    int i = 0;
    char *buffer;
   // for(int x = 20;x<=80;x+=20){
   //     for(int y = 20;y<=80;y+=20){
   //         i+=1;
    for(i=0;i<DIM(sClockState.satRecords);i+=1){
        double angle = (i)*2.0*3.14159/DIM(sClockState.satRecords) - 3.14159/2.0; // rotate 0 to N. 90 E.
        double x = 45.0*cos(angle)+50.0;
        double y = 45.0*sin(angle)+50.0;
        asprintf(&buffer,"<div class='centertext' style='left:%8.2f%%;top:%8.2f%%;' id='sat%d'>%d</div>",x,y,i,i);
        emit(buffer);
        free(buffer);
    }
   //     }
   // }
}
emit("</div>");
emit("</div>");
emit("</div>"

    "<br><br><table id='wifitable' class='blueTable'>"
        "<thead>"
        // this determines column order, see #wifitable below
            "<tr><th>n</th><th>ssid</th><th>auth</th><th>age</th><th>sig</th><th>prime</th><th>bssid</th><th>second</th><th>pairwise</th><th>group</th><th>ant</th><th>11b</th><th>11g</th><th>11n</th><th>lr</th><th>wps</th><th>country</th></tr>"
        "</thead>"
        // #body is insert point
        "<tbody id='body'>"
//            "<tr><td>some</td><td>content</td></tr>"
//            "<tr><td>to be</td><td>removed</td></tr>"
        "</tbody>"
    "</table>"
    );    
    emit("<br>List capped at ");
    {
        char buffer[8];
        snprintf(buffer,sizeof(buffer),"%5d",DIM(sClockState.scanRecords));
        emit(buffer);
    }
    emit(" rows<br><br>");

    emit(
        "<script>" 




"var interval = 1000;  // 1000 = 1 second"
"\n"
"function doAjax() { // https://stackoverflow.com/questions/20371695/execute-an-ajax-request-every-second"
    "\n" // https://stackoverflow.com/users/157247/t-j-crowder
   // "console.log('started');"
    "\n"
    "$.ajax({"
        "\n"
        "type: 'GET',"
        "\n"
        "url: 'recent_wifi.json',"
        "\n"
        "//data: $(this).serialize(),"
        "\n"
        "dataType: 'json',"
        "\n"
        "success: function (data) {"
            "let rows = data['wifi'];" // an array of one dictionary per row, using those keys
            "\n"
            "let rate = parseInt(data['scanrate'])/32.0;" // 0..1
            "\n"
//            "$('#scanRateGauge').css('--gauge-value',100*rate);\n"
//            "$('#scanRateGauge').css('--gauge-display-value',rate*32);\n"
            
            "let temp = (data['temperature'])/212.0;\n" // 0..1   212 is for 0 deg F to 212 deg F
            "$('#temperatureGauge').css('--gauge-value',100*temp);\n" // 100 is for gauge scale, don't change
            "$('#temperatureGauge').css('--gauge-display-value',Math.round(temp*212));\n" // get the value back
            
            "let press = (data['pressure']-800)/400.0;\n" // 0..1   800 to 1200
            "$('#pressureGauge').css('--gauge-value',100*press);\n" // 100 is for gauge scale, don't change
            "$('#pressureGauge').css('--gauge-display-value',Math.round(data['pressure']));\n" // get the value back
            
            "let humi = (data['humidity'])/100.0;\n" // 0..1   212 is for 0 deg F to 212 deg F
            "$('#humidityGauge').css('--gauge-value',100*humi);\n" // 100 is for gauge scale, don't change
            "$('#humidityGauge').css('--gauge-display-value',Math.round(humi*100));\n" // get the value back
            "temp = (data['temperature'] - 32.0)*5.0/9.0;\n"
            "humi = data['humidity']; //  https://bmcnoldy.rsmas.miami.edu/Humidity.html\n"
            "let dewp = 243.04*(Math.log(humi/100)+((17.625*temp)/(243.04+temp)))/(17.625-Math.log(humi/100)-((17.625*temp)/(243.04+temp)));\n"
            "dewp = dewp * 9.0/5.0 + 32;\n" // F
            "dewp = dewp / 212;\n" // 0..1 for 0..212F
            "$('#dewpointGauge').css('--gauge-value',100*dewp);\n" // 100 is for gauge scale, don't change
            "$('#dewpointGauge').css('--gauge-display-value',Math.round(dewp*212));\n" // get the value back
            
            "let ambi = (data['ambient'])/4096.0;\n" // 0..1   212 is for 0 deg F to 212 deg F
            "$('#ambientGauge').css('--gauge-value',100*ambi);\n" // 100 is for gauge scale, don't change
            "$('#ambientGauge').css('--gauge-display-value',Math.round(ambi*4096));\n" // get the value back
            
            "let heap = (data['minimum_heap'])/(200000.0);\n" // 0..1   212 is for 0 deg F to 212 deg F
            "$('#minimum_heapGauge').css('--gauge-value',100*heap);\n" // 100 is for gauge scale, don't change
            "$('#minimum_heapGauge').css('--gauge-display-value',Math.round(heap*200000/1024));\n" // get the value back, in KB
            
            "let availheap = (data['available_heap'])/(200000.0);\n" // 0..1   212 is for 0 deg F to 212 deg F
            "$('#available_heapGauge').css('--gauge-value',100*availheap);\n" // 100 is for gauge scale, don't change
            "$('#available_heapGauge').css('--gauge-display-value',Math.round(availheap*200000/1024));\n" // get the value back, in KB
            
            
//            "$('#scanrateLEFT').width(rate*200).height(20);"
//            "\n"
//            "$('#scanrateRIGHT').width(200-rate*200).height(20);"
            "\n"
            "$('#body').empty();" // delete old rows
            "\n"
            "let keys = $( '#wifitable thead tr th' );" // column names are also keys
            "\n"
            "let starters = 0;"
            "let starttime = -1;"
//             "console.log('rows:%o', rows);"
  //           "console.log('typeof rows:%o', typeof rows);"
            "\n"
            "for (let irow in rows) {"
                "\n"
                "if(starttime == -1){ starttime = rows[irow]['age']; }"
                "\n"
                "if(starttime == rows[irow]['age']) { starters += 1; }"  // the bunch at the start with the same age are the ones just found
                //"console.log('irow:%o', irow);"
                "\n"
                "let html = '<tr>';"
                "\n"
                "keys.each(function(){ let key=$(this).text(); html += ('<td>' + rows[irow][key] + '</td>')   });"
                "\n"
                "html += '</tr>';"
                "\n"
                "$('#body').append(html);" // add new rows         
                "\n"
            "}"
            "\n"
            
            "$('#location_note').html(data.location_note);"
            "$('#clocktime').html(data.time);"
            "\n"
            "$('#scannHeardGauge').css('--gauge-value',starters*3);"
            "\n"
            "$('#scannHeardGauge').css('--gauge-display-value',starters);"
            "\n"
            //
            // move the satellites, show the snr
            //
            "let prns = data['sat_prn'];\n"
            "let els = data['sat_el'];\n"
            "let azs = data['sat_az'];\n"
            "let snrs = data['sat_snr'];\n"
            "let bigsnr = [0,0,0,0];\n" // remember 4 biggest to make an average
            //"console.log('snrs: %o', snrs);"
            "$('.centertext').html('');\n" // clear all snr
            "for (let irow =0;irow<snrs.length;irow+=1) {\n"
                "let prn=prns[irow];\n"
                "let el=els[irow];\n"
                "let az=azs[irow];\n"
                "let snr=snrs[irow];\n"
                "$('#sat'+irow).html(prn.toString()+'x'+snr.toString());\n"
                "let angle = az * 2.0 * 3.14159 / 360.0 - 3.14159/2.0;\n"
                "let x = (90.0-el)/90.0*45.0*Math.cos(angle)+50.0;\n"
                "let y = (90.0-el)/90.0*45.0*Math.sin(angle)+50.0;\n"                
                "$('#sat'+irow).css('top',y.toString()+'%');\n"
                "$('#sat'+irow).css('left',x.toString()+'%');\n"
                "for(let i = 0; i<bigsnr.length; i+=1) {\n"
                    "if(snr > bigsnr[i]) {\n"
                        "for(let j = i+1;j<bigsnr.length;j+=1){\n"
                            "bigsnr[j]=bigsnr[i];\n"
                        "}\n"
                        "bigsnr[i]=snr;\n"
                        "break;\n"
                    "}\n"
                "}\n"
            "}\n"
            "let totalsnr = 0;\n"
            "for(let i = 0; i<bigsnr.length; i+=1) totalsnr += bigsnr[i];\n"
            "totalsnr /= (100.0*bigsnr.length);\n" // typically ~0.3, SNR values from 0 to 99
            "$('#gpsSnrGauge').css('--gauge-value',100*totalsnr);\n" // 100 is for gauge scale, don't change
            "$('#gpsSnrGauge').css('--gauge-display-value',Math.round(totalsnr*100));\n" // get the value back
            
            "$('#gpsNviewGauge').css('--gauge-value',100*snrs.length/32);\n" // 100 is for gauge scale, don't change
            "$('#gpsNviewGauge').css('--gauge-display-value',Math.round(snrs.length));\n" // get the value back
        "},"
        "\n"
        "complete: function (data) {"
            "\n"
            "setTimeout(doAjax, interval);"
            "\n"
        "}"
        "\n"
    "});"
    "\n"
    "}"
    "\n"
    //"console.log('starting');"
    "\n"
    "setTimeout(doAjax, interval);"
    "\n"

    "</script>"
    );
    
// end of a chunk, the commented out bit is the file server code. way below is the close...


    /* Send remaining chunk of HTML file to complete it */
    emit( "</body></html>");

    /* Send empty chunk to signal HTTP response completion */
    emit( NULL);
    return ESP_OK;




}



// ADMINPW - isLoggedIn check for credentials in req
/* 

Originally I thought I might care about the user's IP, but I don't. Leave the code here, because...interesting.
Now, the last little bit just returns true or false.

from https://pubs.opengroup.org/onlinepubs/009619199/inet_pton.htm (see the [3] below):
A third form that is sometimes more convenient when dealing with a mixed environment of IPv4 and IPv6 nodes is x:x:x:x:x:x:d.d.d.d, where the "x"s are the hexadecimal values of the six high-order 16-bit pieces of the address, and the "d"s are the decimal values of the four low-order 8-bit pieces of the address (standard IPv4 representation). 

also, https://datatracker.ietf.org/doc/html/rfc4291#section-2.5.5.2 :
   IPv4-Mapped IPv6 Address
   A second type of IPv6 address that holds an embedded IPv4 address is
   defined.  This address type is used to represent the addresses of
   IPv4 nodes as IPv6 addresses.  The format of the "IPv4-mapped IPv6
   address" is as follows:
   |                80 bits               | 16 |      32 bits        |
   +--------------------------------------+--------------------------+
   |0000..............................0000|FFFF|    IPv4 address     |
   +--------------------------------------+----+---------------------+
*/
static bool isLoggedIn(httpd_req_t *req) { // print_client_ip from: @ESP_Anurag https://esp32.com/viewtopic.php?t=8680
    int sockfd = httpd_req_to_sockfd(req);
    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in6 addr;   // esp_http_server uses IPv6 addressing
    socklen_t addr_size = sizeof(addr);
    
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addr_size) < 0) {
        ESP_LOGE(TAG, "Error getting client IP");
//        return;
    }
    
    // Convert to IPv6 string
    inet_ntop(AF_INET6, &addr.sin6_addr, ipstr, sizeof(ipstr)); // wch - use AF_INET6?
   // ESP_LOGI(TAG, "Client IP => %s", ipstr);

    // Convert to IPv4 string
    inet_ntop(AF_INET, &addr.sin6_addr.un.u32_addr[3], ipstr, sizeof(ipstr));
   // ESP_LOGI(TAG, "Client IP => %s", ipstr);
    
    
    
    size_t sz=httpd_req_get_hdr_value_len(req,"Authorization"); // does not include space for nul
   // printf("auth length=%d\n",sz);
    bool match = false;
    if(sz>0){
        char *hv = malloc(sz+1); // allow for nul at end, otherwise ...
        httpd_req_get_hdr_value_str(req, "Authorization", hv, sz+1); // this *will* write a nul on the last valid pos, even if it is a data byte!
   //     printf("auth=%s\n",hv);
        
//        if(hv[sz-1]==0) // should not need this now
//            sz -= 1; // dont send nul byte to decode
//        match = strcmp(hv, sClockState.cfgMyFiPass[0] ) == 0;
        
        

/** /home/c/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/base64.h
 * \brief          Decode a base64-formatted buffer
 *
 * \param dst      destination buffer (can be NULL for checking size)
 * \param dlen     size of the destination buffer
 * \param olen     number of bytes written
 * \param src      source buffer
 * \param slen     amount of data to be decoded
 *
 * \return         0 if successful, MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL, or
 *                 MBEDTLS_ERR_BASE64_INVALID_CHARACTER if the input data is
 *                 not correct. *olen is always updated to reflect the amount
 *                 of data that has (or would have) been written.
 *
 * \note           Call this function with *dst = NULL or dlen = 0 to obtain
 *                 the required buffer size in *olen
 */
//int mbedtls_base64_decode( unsigned char *dst, size_t dlen, size_t *olen,
//                   const unsigned char *src, size_t slen );     
        if(sz>6 && (memcmp("Basic ", hv,6)==0||memcmp("basic ", hv,6)==0||memcmp("BASIC ", hv,6)==0)){
            size_t ndecoded=0;
            int rc = mbedtls_base64_decode( NULL, 0, &ndecoded, (unsigned char *)(hv+6), sz-6 ); 
   //         printf("rc=%d ndecoded=%d for <%.*s>\n",rc,ndecoded,sz-6,(unsigned char *)(hv+6));
            if(rc!=MBEDTLS_ERR_BASE64_INVALID_CHARACTER && ndecoded>0) { // -44 is inval char, -42 is buf too short. a nul gives -44.
                ndecoded += 1; // null at end?
                char *decodedText = malloc(ndecoded);
                size_t ndecoded2=0;
                rc = mbedtls_base64_decode( (unsigned char *)(decodedText), ndecoded-1, &ndecoded2, (unsigned char *)(hv+6), sz-6 ); 
   //             printf("rc=%d ndecoded2=%d\n",rc,ndecoded2);
                decodedText[ndecoded2]=0;
   //             printf("%s\n",decodedText);
                if(ndecoded2>3){
                    match = memcmp(decodedText, "rc", 2 ) == 0 && strcmp(decodedText+3, sClockState.cfgMyFiPass[0] ) == 0;
   //                 printf("match test\n");
                }
                if(!match) printf("match=%d %.*s %s %s %s\n",match,2,decodedText,"rc",decodedText+3,sClockState.cfgMyFiPass[0]);
                free(decodedText);
            }    
        }
        free(hv);
    };
    return match; // true: logged in ok
}

// ADMINPW
//
// 3rd try: use basic auth. No cookie, the BA credentials are cached by the browser
// 
// https://en.wikipedia.org/wiki/Basic_access_authentication --
//When the server wants the user agent to authenticate itself towards the server after receiving 
// an unauthenticated request, it must send a response 
// with a HTTP 401 Unauthorized status line
// and a WWW-Authenticate header field.
//The WWW-Authenticate header field for basic authentication is constructed as following:
//WWW-Authenticate: Basic realm="User Visible Realm" 
//---------
//The Authorization header field is constructed as follows:
//    The username and password are combined with a single colon (:). This means that the 
//       username itself cannot contain a colon.
//    The resulting string is encoded into an octet sequence. The character set to use for 
//       this encoding is by default unspecified, as long as it is compatible with US-ASCII, 
//       but the server may suggest use of UTF-8 by sending the charset parameter.
//    The resulting string is encoded using a variant of Base64 (+/ and with padding).
//    The authorization method and a space (e.g. "Basic ") is then prepended to the encoded string.
//
//For example, if the browser uses Aladdin as the username and open sesame as the password, 
//   then the field's value is the Base64 encoding of Aladdin:open sesame, or QWxhZGRpbjpvcGVuIHNlc2FtZQ==. 
//   Then the Authorization header field will appear as:
//
//Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ== 
//
// the browser caches the credentials and sends them every time. I don't need a cookie.
// to check Authorization: Basic <credentials>
// httpd_req_get_hdr_value_len(req,"Authorization")

static esp_err_t setup_html_handler_real( httpd_req_t *req ) {
//    char strftime_buf[64];

// the name= is used to disable it, not to figure out which one...
#define BUTTONS	"<button type='submit' value='Submit' name='save' formaction='save.cmd' >Save</button>"\
                "<button type='submit' value='Submit' formaction='reset.cmd'>Reset</button>"\
                "<button type='submit' value='Submit' formaction='cancel.cmd'>Cancel</button>"

    /* Send HTML file header */
    emit( "<!DOCTYPE html>"// https://html-css-js.com/html/generator/form/ -- starting point?
    "<head><meta charset='UTF-8'><title>RedClock Setup</title>"
//    "<script src='https://unpkg.com/masonry-layout@4/dist/masonry.pkgd.min.js'></script>" // rectangle arranger
// <script src="https://cdnjs.cloudflare.com/ajax/libs/masonry/4.2.2/masonry.pkgd.min.js" integrity="sha512-JRlcvSZAXT8+5SQQAvklXGJuxXTouyq8oIMaYERZQasB8SBDHZaUbeASsJWpk0UUrf89DP3/aefPPrlMR1h1yQ==" crossorigin="anonymous"></script>
"<script src='js/masonry.pkgd.min.js' integrity='sha512-JRlcvSZAXT8+5SQQAvklXGJuxXTouyq8oIMaYERZQasB8SBDHZaUbeASsJWpk0UUrf89DP3/aefPPrlMR1h1yQ==' crossorigin='anonymous'></script>"
//    "<script src='/masonry.pkgd.min.js'></script>" // rectangle arranger
//    "<script src='https://cdnjs.cloudflare.com/ajax/libs/jquery/3.5.1/jquery.min.js' integrity='sha512-bLT0Qm9VnAYZDflyKcBaQ2gg0hSYNQrJ8RilYldYQ1FxQYoCLtUjuuRuZo+fjqhx/qtq/1itJ0C2ejDxltZVFg==' crossorigin='anonymous'></script>"
    "<script src='js/jquery-3.5.1.min.js' integrity='sha512-bLT0Qm9VnAYZDflyKcBaQ2gg0hSYNQrJ8RilYldYQ1FxQYoCLtUjuuRuZo+fjqhx/qtq/1itJ0C2ejDxltZVFg==' crossorigin='anonymous'></script>"
    "<style>"
    "form {"
        "vertical-align: top;" // allow the forms to flow across and 
        "display: inline-block;" // stick to top
    "}"
    "input {"
        "margin-top: 5px;" // space between the text fields
        "margin-bottom: 1px;"
    "}"
    "button {"
        "margin-top: 15px;" // space above and
        "margin-left: 5px;" // between the
        "margin-right: 5px;" // button pair
    "}"
    
    

".tg  {border-collapse:collapse;border-spacing:0;}"
".tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;"
"  overflow:hidden;padding:10px 5px;word-break:normal;}"
".tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;"
"  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}"
".tg .tg-0lax{text-align:left;vertical-align:top}"
".tg .tg-sjuo{background-color:#C2FFD6;text-align:left;vertical-align:top}"

    
    
    "</style>"
    "</head><html>"
    "<body style='background-color: white'>"
    "<span style='font-size:300%;'>RedClock Setup <span id='clocktime' style='color: #cc3333;'>clocktime</span> </span>"
    );
    
    emit(
    "<br>Save each section as you make changes; unsaved changes in other sections are lost. Reboot is required for some changes to take effect. Help is in the tooltips, hover over the control. Router means your wifi router.<br>This clock uses a 32-bit epoch time that may fail around 03:14:07 UTC on 19 January 2038. The third GPS 1024 week rollover will be November 20-21, 2038; I've no idea what these GPS devices will do.<br><br>"

    "<div class='grid' }>" ); 
    
    for(int iform = 0; iform<DIM(formPanels); iform+=1){
        struct FormPanel const *fpanel = formPanels[iform];

        emit("<form class='slider' accept-charset='ASCII' autocomplete='off' method='GET'>" "<fieldset>"	"<legend>");
        emit(fpanel->legend); // this is the panel-box title
        emit("</legend>");

        for(int k = 0; k < fpanel->nKeyVals; k += 1 ){
            struct KEYVAL const *kval = fpanel->keyVals[k];
            struct FormData const *fdata = kval->formData;
            char const *cv = "nonesuch"; // this is printf, below, for the "inputNothing" paragraph in form 1 
            if(fdata){
                cv = (fdata->currentValue[0] && *fdata->currentValue[0]) ? *fdata->currentValue[0] : "nullptr";
            }
            //printf("iform=%d k=%d cv=%s\n", iform, k, cv);
            switch(kval->inputType){
            case inputPassword:
                cv = "~"; // don't leak out the real password in html. check for this dummy, see below
                // magic comment: https://stackoverflow.com/questions/44511436/how-to-do-an-explicit-fall-through-in-c
                // https://stackoverflow.com/questions/45129741/gcc-7-wimplicit-fallthrough-warnings-and-portable-way-to-clear-them/52707279#52707279
                // https://stackoverflow.com/users/8316315/florian-weimer
                // fall through
            case inputText://"<input name='dawn' value='0700' type='text' size='5' title='Pick Dawn/Dusk above, set the High and Low values. Go bright (high) at this time.'/> Dawn (typically 0700)<br>"
                emit("<input name='");
                emit(fdata->key);
                emit("' id='");
                emit(fdata->key);
                emit("' value='");
                emit(cv);
                if(kval->inputType==inputPassword)
                    emit("' type='password' class='pass8' size='");
                else
                    emit("' type='text' size='");                
                {char b[16];itoa(kval->textSize,b,10);emit(b);}
                emit("' title='");
                emit(kval->title);
                emit("'/> ");
                emit(kval->label);
                emit("<br>");
                break;
            case inputRadio: 
//"<input type='radio' checked='checked' name='brite' value='sensor' title='There is a CDS photo ... its limits.'/> High to Low using sensor<br>"
                {
                    char const *linevalue = fdata->vals[kval->nthRadioLine];
                    if ( strcmp(linevalue, cv ) == 0) { // selected radio
                        emit("<input checked='checked' type='radio' name='");
                    }
                    else { // not selected
                        emit("<input type='radio' name='");
                    }
                    emit(fdata->key);
                    emit("' id='"); // promise the fdata->key+linevalue is unique. linevalue alone probably is.
                    emit(fdata->key);emit(linevalue); // example: "wifi" + "off"
                    emit("' value='");
                    emit(linevalue);
                    emit("' title='");
                    emit(kval->title);
                    emit("'/><label for='");
                    emit(fdata->key);emit(linevalue);
                    emit("'> ");
                    emit(kval->label);/////////// make this be an actual label on the radio so it is clickable
                    emit("</label><br>");
                }
                break;
            case inputNothing: // no longer in use, paragraph replaced with radio
                emit(kval->label);
                break;
            default:
                printf("ERROR****** %d\n%s\n%s\n%s\n\n", formPanels[iform]->nKeyVals, fpWIFI.keyVals[0]->title, fpWIFI.keyVals[1]->title, fpWIFI.keyVals[2]->label);
            }
        }
        emit( BUTTONS "</fieldset>" "</form>" );
    }

    emit("<form class='slider' accept-charset='ASCII' autocomplete='off' method='GET'><fieldset><legend>Reboot</legend>");

    
    emit("Save changes first!<br><button type='submit' value='Submit' formaction='reboot.cmd' >Restart Clock Now</button>");
    emit("</fieldset></form>" );

    emit(
    "</div>" // end of the .grid
    );
 
    emit(
        "<script>" // this does switch masonry on! Similar in the div did not, don't understand what it should have triggered anyway. (maybe had no /div?)
"var msnry = new Masonry('.grid',{columnWidth:50,itemSelector:'.slider'});"


"\n"
//https://stackoverflow.com/questions/18849296/masonry-js-overlapping-items
    "$(document).ready(function(){" // https://stackoverflow.com/users/2701501/jeff-shain
"\n"
        "setTimeout(function() { masonry_go();}, 1000);"
"\n"
    "});"
"\n"
    "$(window).resize(function()"
"\n"
    "{"
"\n"
        "setTimeout(function() { masonry_go();}, 1000);"
"\n"
    "});"
"\n"
    "function masonry_go(){"
"\n"
        "msnry.layout();"
"\n"
        "setTimeout(function() { masonry_go2();}, 1000);"// sometimes another one is needed. yuck.
"\n"
    "}"
"\n"
    "function masonry_go2(){"
"\n"
        "msnry.layout();"
"\n"
    "}"
"\n"





"var interval = 1000;  // 1000 = 1 second"
"\n"
"function doAjax() { // https://stackoverflow.com/questions/20371695/execute-an-ajax-request-every-second"
"\n" // https://stackoverflow.com/users/157247/t-j-crowder
//"console.log('started');"
"\n"
    "$.ajax({"
"\n"
        "type: 'GET',"
"\n"
        "url: 'status.json?time',"
"\n"
        "//data: $(this).serialize(),"
"\n"
        "dataType: 'json',"
"\n"
        "success: function (data) {"
"\n"
//"console.log('tick: %o', data);"
"\n"
            "$('#clocktime').html(data.time);"
"\n"
        "},"
"\n"
        "complete: function (data) {"
"\n"
            "setTimeout(doAjax, interval);"
"\n"
        "}"
"\n"
    "});"
"\n"
"}"
"\n"
"console.log('starting');"
"\n"
"setTimeout(doAjax, interval);"


"\n" // connect the grayed-out behavior
"function enablewifi(){ "
    "console.log('wifioff %o',$('#wifioff').is(':checked'));"
    "$( '#wfiname,#wfipass' ).prop( 'disabled', $('#wifioff').is(':checked') );"
"}"
"\n"
"$('#wifioff').on('change', enablewifi );"
"\n"
"$('#wifion').on('change', enablewifi );"
"\n"
"enablewifi();"
// password length for class pass8



"$('.pass8').on('keyup',function(){"
    "  var my_txt = $(this).val();"
    "  var len = my_txt.length;"
    "  console.log('txt=%o len=%o button=%o',my_txt,len,$(this).closest(\"form\").find(\"button[name='save']\") );"
// allow zero len pw for open wifi, at least 8 for wpa
    "  $(this).closest(\"form\").find(\"button[name='save']\").prop( 'disabled', (1 <= len) && (len <= 7) && (my_txt != '~' ));"
"});"











        "</script>"
    );
    
// end of a chunk, the commented out bit is the file server code. way below is the close...


    /* Send remaining chunk of HTML file to complete it */
    emit( "</body></html>");

    /* Send empty chunk to signal HTTP response completion */
    emit( NULL);
    return ESP_OK;

}


/* Send HTTP response with a run-time generated html consisting of
 * a list of all files and folders under the requested path.
 * In case of SPIFFS this returns empty list when path is any
 * string other than '/', since SPIFFS doesn't support directories */
static esp_err_t directory_html_handler(httpd_req_t *req)
{
//    char strftime_buf[64];

    char *dirpath = "/spiffs/";


    char entrypath[FILE_PATH_MAX]; // 48
    char entrysize[16];
    const char *entrytype;

    struct dirent *entry;
    struct stat entry_stat;

    DIR *dir = opendir(dirpath);
    const size_t dirpath_len = strlen(dirpath);

    /* Retrieve the base path of file storage to construct the full path */
    strlcpy(entrypath, dirpath, sizeof(entrypath));

    if (!dir) {
        ESP_LOGE(TAG, "Failed to stat dir : %s", dirpath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory does not exist");
        return ESP_FAIL;
    }




    /* Send HTML file header */
    emit( "<!DOCTYPE html>"// https://html-css-js.com/html/generator/form/ -- starting point?
    "<head><meta charset='UTF-8'><title>RedClock Directory</title>");
    
    emit(cBlueTable);
    
    emit("</head><html>"
    "<body  style='background-color: white'>"
    );

    /* Get handle to embedded file upload script */
    extern const unsigned char upload_script_start[] asm("_binary_upload_script_html_start");
    extern const unsigned char upload_script_end[]   asm("_binary_upload_script_html_end");
    const size_t upload_script_size = (upload_script_end - upload_script_start);

    /* Add file upload form and script which on execution sends a POST request to /upload */
    bufferedhttpd_resp_send_chunk(req, (const char *)upload_script_start, upload_script_size);

    /* Send file-list table definition and column labels */
    emit(
        "<br><span style='font-size:150%; color: #ff0000'>If you delete something, the clock might stop working! It might be very hard to repair. Knowing how to use cUrl like this may help: curl -v --data-binary @/home/c/Desktop/index.html 192.168.4.147/upload/index.html, but only if you have a copy of the files you deleted!.</span><br>"
        "<table class='blueTable'>"
//        "<table class=\"fixed\" border=\"1\">"
//        "<col width=\"800px\" /><col width=\"300px\" /><col width=\"300px\" /><col width=\"100px\" />"
        "<thead><tr><th>Name</th><th>Type</th><th>Size (Bytes)</th><th>Delete</th></tr></thead>"
        "<tbody>");

    /* Iterate over all files / folders and fetch their names and sizes */
    while ((entry = readdir(dir)) != NULL) {
        entrytype = (entry->d_type == DT_DIR ? "directory" : "file");

        strlcpy(entrypath + dirpath_len, entry->d_name, sizeof(entrypath) - dirpath_len);
        if (stat(entrypath, &entry_stat) == -1) {
            ESP_LOGE(TAG, "Failed to stat %s : %s", entrytype, entry->d_name);
            continue;
        }
        sprintf(entrysize, "%ld", entry_stat.st_size);
   //     ESP_LOGI(TAG, "Found %s : %s (%s bytes)", entrytype, entry->d_name, entrysize); // this filename has no leading slash

        /* Send chunk of HTML file containing table entries with file name and size */
        emit( "<tr><td><a href=\"/"); // <<<< leading slash on file name
//        emit( req->uri); // <<<< this name is the current uri, not what we want
        emit( entry->d_name);
        if (entry->d_type == DT_DIR) { // not sure how to trigger this, css/xxx.css is just another file name in the root dir
            emit( "/");
        }
        emit( "\">");
        emit( entry->d_name);
        emit( "</a></td><td>");
        emit( entrytype);
        emit( "</td><td>");
        emit( entrysize);
        emit( "</td><td>");
        emit( "<form method=\"post\" action=\"/delete/"); // use the trailing slash on delete, not...
        //emit( req->uri); // not the current uri
        emit( entry->d_name);
        emit( "\"><button type=\"submit\" "
        "style='font-size: 12px;"
        "color: white;"
        "background: rgb(180,0,0);" // red button
        "border: 1px solid black;"
        "border-radius: 5px;'"
        ">Delete</button></form>");
        emit( "</td></tr>\n");
    }
    closedir(dir);

    /* Finish the file list table */
    emit( "</tbody></table>");


    size_t total = 0, used = 0;
    esp_err_t ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        //return ESP_FAIL;
    }
    else {
//        ESP_LOGI(TAG, "Partition size: total: %d, used: %d  (on core %d)", total, used, xPortGetCoreID());
        char buf[128];
        snprintf( buf,sizeof(buf),
            "<br><br><pre>%9.3f KBytes total\n-%8.3f KBytes used\n---------\n%9.3f KBytes avail</pre><br>", 
            total/1000.0, used/1000.0, (total-used)/1000.0 );
        emit(buf);
    }
    /* Send remaining chunk of HTML file to complete it */
    emit( "</body></html>");

    /* Send empty chunk to signal HTTP response completion */
    emit( NULL);
    return ESP_OK;
}

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf")) {
        return httpd_resp_set_type(req, "application/pdf");
    } else if (IS_FILE_EXT(filename, ".html")) {
        return httpd_resp_set_type(req, "text/html");
    } else if (IS_FILE_EXT(filename, ".jpeg") || IS_FILE_EXT(filename, ".jpg")) {
        return httpd_resp_set_type(req, "image/jpeg");
    } else if (IS_FILE_EXT(filename, ".png")) {
        return httpd_resp_set_type(req, "image/png");
    } else if (IS_FILE_EXT(filename, ".ico")) {
        return httpd_resp_set_type(req, "image/x-icon");
    } else if (IS_FILE_EXT(filename, "css")) {
        return httpd_resp_set_type(req, "text/css");
    } else if (IS_FILE_EXT(filename, "js")) {
        return httpd_resp_set_type(req, "text/javascript");
    } else if (IS_FILE_EXT(filename, "json")) {
        return httpd_resp_set_type(req, "application/json");
    }    
    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */
static const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash) {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize) {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}

/* Handler to download a file kept on the server OR a generated file -- any GET request */
static esp_err_t download_get_handler(httpd_req_t *req)
{

// ADMINPW
// httpd_req->sess_ctx is a void* I can use for this session.
// set it to malloc("ok") when the user gives the password.
// if not "ok", redirect to the password screen, which needs
// a special exemption. The server does the free() when the
// session ends (socket closes). Password is the 88888888
// password for myfi, whatever it is changed to.
    // not using this idea ... printf("download ctx=%s\n",req->sess_ctx==NULL ? "--" : (char*)(req->sess_ctx));
    bool loggedIn = isLoggedIn(req);


    // after first time, the pw is set
// not using this idea ...     if(req->sess_ctx==NULL){
// not using this idea ...         req->sess_ctx = strdup("OK");
// not using this idea ...     }

    char filepath[FILE_PATH_MAX]; // 48
    char filename[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;

    /*block*/
    {    
        time_t now;
        time(&now);
        struct tm tmi;
        gmtime_r(&now, &tmi); // _r is the threadsafe version
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%a, %d %b %Y %H:%M:%S GMT", &tmi); // Date: Wed, 24 Feb 2021 15:26:29 GMT
        httpd_resp_set_hdr(req, "Date", strftime_buf); // Date: Tue, 15 Nov 1994 08:12:31 GMT
    }

    char const *tempfile = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path, req->uri, sizeof(filepath));
    if (!tempfile) {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    strlcpy( filename, tempfile, sizeof(filename) );
    
    /*block*/
    {
        int n = strlen(filename);
        if (n<5 || strcmp(&filename[n-5],".json") != 0 ){ // too noisy
            printf("1 filename='%s'\n",filename);
        }
    }
    
    bool issave = strcmp(filename,"/save.cmd")==0;
    bool isreset = strcmp(filename,"/reset.cmd")==0;
    bool iscancel = strcmp(filename,"/cancel.cmd")==0;
    bool isreboot = strcmp(filename,"/reboot.cmd")==0;
    if(isreboot){ // allow reboot without login // ADMINPW
        gpsColdReset(); // 
//        printf("gps cold reset\n");

// there is a JSL program that append ?nnn to the reset uri; I'd like it to 
// go faster than the redirect. check for it...
        if(httpd_req_get_url_query_len(req)){
            httpd_resp_send(req, "resetting", 9);//If no status code and content-type were set, by default this will send 200 OK status code and content type as text/html. 
        }
        else { // not the jsl program...redirect to setup
            redirect_to_setup_html_handler(req);// don't leave the browser trying to refresh the reboot page!
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // not sure this helped or was needed
        printf("Bye, be right back...\n");
        esp_restart(); // a graceful shutdown spews more messages to the console for several seconds
    }
    if(issave || isreset || iscancel) {
        if(!loggedIn){ // ADMINPW required to change config (should cancel be here?)
            httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Red Clock\"");
            httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Password required");
            return ESP_OK; // Must return OK, not FAIL
        }
        int buf_len = httpd_req_get_url_query_len(req) + 1;
        if (buf_len > 1) {
            char *buf = malloc(buf_len); 
            char *param = malloc(buf_len); 
            if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query => %s", buf);
                // walk the formPanels to find possible keys
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html
                    bool madeNVSchanges = false;
                    nvs_handle_t handle;
                    nvs_open("RedClock", NVS_READWRITE, &handle);
                    for(int k = 0; k < DIM(formDatas); k+=1) {
                        struct FormData const *fdata = formDatas[k];
                        //if(fdata){
                            if (httpd_query_key_value(buf, fdata->key, param, buf_len) == ESP_OK) {
                                urldecode2(param,param);
// ESP_LOGI(TAG, "Found URL query parameter => %s=%s old=%s def=%s",fdata->key, param, (*fdata->currentValue)[0], (*fdata->currentValue)[1]);
                                if(issave){ // "~" is the hidden password stored above; the password field will divulge its text in debugger
                                    if ( strcmp( (*fdata->currentValue)[0], param ) != 0 && strcmp( param, "~" ) != 0 ) {
// ESP_LOGI(TAG, "  saved URL query parameter => %s=%s old=%s def=%s",fdata->key, param, (*fdata->currentValue)[0], (*fdata->currentValue)[1]);
                                        free((*fdata->currentValue)[0]);
                                        (*fdata->currentValue)[0] = strdup(param);
                                        ESP_ERROR_CHECK( nvs_set_str( handle, fdata->key, (*fdata->currentValue)[0]) );
                                        madeNVSchanges = true;
                                    }
                                }
                                else if(isreset){
                                    if ( strcmp( (*fdata->currentValue)[0], (*fdata->currentValue)[1] ) != 0){
// ESP_LOGI(TAG, "  reset URL query parameter => %s=%s old=%s def=%s",fdata->key, param, (*fdata->currentValue)[0], (*fdata->currentValue)[1]);
                                        free((*fdata->currentValue)[0]);
                                        (*fdata->currentValue)[0] = strdup((*fdata->currentValue)[1]);
                                        //esp_err_t rc = nvs_set_str( handle, fdata->key, (*fdata->currentValue)[0]);
                                        // perhaps just remove the key from NVS to perform a reset; that
                                        // should work because it has to boot the first time without NVS...
                                        ESP_ERROR_CHECK( nvs_erase_key( handle, fdata->key) );
                                        madeNVSchanges = true;
                                    }
                                }
                                else
                                    ESP_LOGI(TAG, "  iscancel=%d\n",iscancel);
                            }
                        //}
                    }
                    if ( madeNVSchanges ) { // did not test if it was OK to do this needlessly...
                        ESP_ERROR_CHECK( nvs_commit( handle ) );
                    }
                    nvs_close( handle );
                //}
            }
            free(buf);
            free(param);
        }
        return redirect_to_setup_html_handler(req);
    }










// restore this behavior? perhaps no slash is distinct from one slash. No slash->index.html

    //  a single / maps to index.html. there appears to be no empty string; there is at least a /
    if (strcmp(filename, "/") == 0){
        strlcpy(filename, "/index.html", sizeof(filename)); // this is a spiffs page
    }
    strlcpy( filepath, "/spiffs", sizeof(filepath) );
    strlcat( filepath, filename, sizeof(filepath) );

   // printf("2 filename='%s'\n",filename);
   // printf("2 filepath='%s'\n",filepath);

    if (strcmp(filename, "/directory.html") == 0) { // synthetic (not spiffs)
//        if(!loggedIn){ // ADMINPW allow viewing the files and downloading the files
//            httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Red Clock\"");
//            httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Password required");
//            return ESP_OK; // Must return OK, not FAIL
//        }
        return directory_html_handler(req); // trailing / ? must match clockspiffs
    }
    
    // testing ADMINPW ... actually this is the logout mechanism, see index.html
    else if (strcmp(filename, "/auth.html") == 0) { // synthetic (not spiffs)
        if(!loggedIn){
            httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Red Clock\"");
            httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Password required");
        }
//        else {
//            httpd_resp_set_status(req, "307 Temporary Redirect");
//            httpd_resp_set_hdr(req, "Location", "/status.html");
//            httpd_resp_send(req, NULL, 0);  // Response body can be empty
//        }
        return ESP_OK; // Must return OK, not FAIL
    }
//    
    else if (strcmp(filename, "/status.json") == 0) { // synthetic (not spiffs)
        return status_json_handler(req);
    }
    else if (strcmp(filename, "/status.html") == 0) { // synthetic (not spiffs)
        return status_html_handler(req);
    }
    else if (strcmp(filename, "/recent_wifi.json") == 0) { // synthetic (not spiffs)
        return recent_wifi_json_handler(req);
    }
    else if (strcmp(filename, "/recent_wifi.html") == 0) { // synthetic (not spiffs)
        return recent_wifi_html_handler(req);
    }
    else if (strcmp(filename, "/setup.html") == 0) { // synthetic (not spiffs)
//        if(!loggedIn){ // ADMINPW allow viewing the setup
//            httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Red Clock\"");
//            httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Password required");
//            return ESP_OK; // Must return OK, not FAIL
//        }
        return setup_html_handler_real(req); // require the password when save/reset
    }
    else if (stat(filepath, &file_stat) == -1) {
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }
    else {
        fd = fopen(filepath, "r");
        if (!fd) {
            ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...CACHED", filename, file_stat.st_size);
        set_content_type_from_file(req, filename);



        // cache control: an hour of not resending the files seems reasonable. FF and Chrome have different rules on refresh behavior.
        httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=3600, immutable"); //https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control



        /* Retrieve the pointer to scratch buffer for temporary storage */
        char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
        size_t chunksize;
        do {
            /* Read file in chunks into the scratch buffer */
            chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

            if (chunksize > 0) {
                /* Send the buffer contents as HTTP response chunk */
                if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                    fclose(fd);
                    ESP_LOGE(TAG, "File sending failed!");
                    /* Abort sending file */
                    httpd_resp_sendstr_chunk(req, NULL);
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                   return ESP_FAIL;
               }
            }

            /* Keep looping till the whole file is sent */
        } while (chunksize != 0);

        /* Close file after sending complete */
        fclose(fd);
        ESP_LOGI(TAG, "File sending complete");

        /* Respond with an empty chunk to signal HTTP response completion */
        httpd_resp_send_chunk(req, NULL, 0);
    }
    return ESP_OK;
}

/* Handler to upload a file onto the server */
static esp_err_t upload_post_handler(httpd_req_t *req)
{

// ADMINPW
// not using this idea ...     printf("upload ctx=%s\n",req->sess_ctx==NULL ? "--" : (char*)(req->sess_ctx));
    bool loggedIn = isLoggedIn(req);
    if(!loggedIn){ // ADMINPW required for upload
        httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Red Clock\"");
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Password required");
        return ESP_OK; // Must return OK, not FAIL
    }

    esp_err_t rc;
printf("upload post\n");
    char filepath[FILE_PATH_MAX]; // 48
    FILE *fd = NULL;
    struct stat file_stat;
//ESP_LOGI(TAG, "upload");
    /* Skip leading "/upload" from URI to get filename */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri + sizeof("/upload") - 1, sizeof(filepath));
    if (!filename) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_OK;
    }
    
    printf("base path: %s\n", ((struct file_server_data *)req->user_ctx)->base_path);//base path: /spiffs
    printf("filepath: %s\n", filepath);//filepath: /spiffs/index3.html
    printf("filename: %s\n", filename);//filename: /index3.html
    
//ESP_LOGI(TAG, "name : %s", filename);
    /* Filename cannot have a trailing '/' */
    if (filename[strlen(filename) - 1] == '/') {
        ESP_LOGE(TAG, "Invalid filename : %s", filename);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
        return ESP_OK;
    }

    if (stat(filepath, &file_stat) == 0) {
        ESP_LOGE(TAG, "File already exists : %s", filepath);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File already exists");
        return ESP_OK;
    }
/*
to update the running code: similar to the timezone update, overwrite the timezone data with the
new bin file, with a short valid-looking UTC0 - only timezone lookup so it won't be dead.
use the SHA256 to verify the new file appended to the dummy header. 
Finally, use an IRAM function to copy the data from the TZ partition to the Factory App.
May need the SPI_FLASH_DANGEROUS_WRITE in the project config.
Probably need interrupts disabled, maybe wait for next boot and check before starting anything.
Probably need to kill the watch dog timer?

 notes: sha_parallel_engine.h can test the sha
*/
    bool isTIMEZONE = strcmp(filename,"/Timezone_Update.Bin") == 0;
    bool isEXECUTABLE = strcmp(filename,"/Executable_Update.Bin") == 0;
    const esp_partition_t *timezonePartition = NULL;
    const esp_partition_t *otaPartition = NULL;
    esp_ota_handle_t otaHandle;
    bool badUpdate = false;
    if (isTIMEZONE) { // tzmap file
        timezonePartition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "tzmap");
        if (timezonePartition==NULL || req->content_len > timezonePartition->size ) {
            ESP_LOGE(TAG, "update too large: %d bytes for tzmap size: %d", req->content_len, timezonePartition?timezonePartition->size:-1);
            /* Respond with 400 Bad Request */
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "update size must be less than tzmap size (maybe 2031616).");
            /* Return failure to close underlying connection else the
             * incoming file content will keep the socket busy */
             // actually, return OK so FF produces a message. Chrome seems to get it even with Fail
            return ESP_OK;
        
        }
        // point of no return...erase all
        printf("begin erase...\n");
        sClockState.timezoneBuildDate = "bad"; // assume bad, a reboot is needed anyway. this should stop lookup failures.
        rc = esp_partition_erase_range(timezonePartition, 0, timezonePartition->size);
        printf("...end erase\n");
        if(rc!=0){
            printf("tzmap esp_partition_erase_range rc=%d psize=%d\n", rc,timezonePartition->size);
            ESP_ERROR_CHECK(rc);
            badUpdate=true;
        }
    }
    else if(isEXECUTABLE) {
        //
        otaPartition = esp_ota_get_next_update_partition(NULL);
        rc = esp_ota_begin(otaPartition, OTA_SIZE_UNKNOWN, &otaHandle); // we know the size, but unknown erases all
        if(rc!=ESP_OK) {
            badUpdate=true;
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,"esp_ota_begin failed");
            return ESP_OK;            
        }
        printf("esp_ota_begin = %d\n",rc);
        //const esp_partition_t *esp_ota_get_running_partition(void)
    }
    else { // SPIFFS file
        /* File cannot be larger than a limit */ /* wch - afaik this is an arbitrary limitation. the partition is < 1M. */
        if (req->content_len > MAX_FILE_SIZE) {
            ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
            /* Respond with 400 Bad Request */
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                "File size must be less than "
                                MAX_FILE_SIZE_STR "!");
            /* Return failure to close underlying connection else the
             * incoming file content will keep the socket busy */
            return ESP_OK;
        }
        fd = fopen(filepath, "w");
        if (!fd) {
            ESP_LOGE(TAG, "Failed to create file : %s", filepath);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
            return ESP_OK;
        }
        
    }
    
    ESP_LOGI(TAG, "Receiving file : %s...", filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *buf = ((struct file_server_data *)req->user_ctx)->scratch;
    int received;

    /* Content length of the request gives
     * the size of the file being uploaded */
    int remaining = req->content_len;
    size_t completed = 0;


//
//    static struct SMALLTIMEZONE sSmallTimeZone = {
//        .h = {
//            .n = 12,
//            .stringTableIndexOffset = sizeof(struct TZHeader),
//            .stringTableDataOffset = sizeof(struct TZHeader) + sizeof(struct TZStringTableIndex),
//            .RLETableIndexOffset = sizeof(struct TZHeader) + sizeof(struct TZStringTableIndex) + sizeof(struct TZStringTableData),
//            .RLETableDataOffset = sizeof(struct TZHeader) + sizeof(struct TZStringTableIndex) + sizeof(struct TZStringTableData) + sizeof(struct TZRLETableIndex),
//            .buildDateOffset = sizeof(struct TZHeader) + sizeof(struct TZStringTableIndex) + sizeof(struct TZStringTableData) + sizeof(struct TZRLETableIndex) + sizeof(struct TZRLETableData),
//            .nrows = 1,
//            .ncols = 1,
//            .bot = -90,
//            .top = 90,
//            .left = -180,
//            .right = 180,
//            .sig = "END."
//        },
//        .sti = {{0}},
//        .std = {"UTC0\0\0\0\0"},
//        .rti = {{0}},
//        .rtd = {{0,1}},
//        .bd = {"UTCEX\0"},
//        .executableLength = 0
//    };
//    sSmallTimeZone.executableLength = remaining;
//    
//if( sizeof(sSmallTimeZone) != sizeof(struct SMALLTIMEZONE) ) printf("********** bad sizeof TZ ***********\n"); else printf("TZ sizeof good\n");

//    if(isEXECUTABLE) {
//        // use the completed variable to insert the prefix dummy timezone before the data
//        DumpHex(&sSmallTimeZone,0,sizeof(sSmallTimeZone));
//        rc = esp_partition_write(timezonePartition, completed, &sSmallTimeZone, sizeof(sSmallTimeZone));
//        if( rc != 0 ) {
//            printf("error esp_partition_write rc=%d\n", rc);
//            badUpdate = true;
//        }
//        completed += sizeof(sSmallTimeZone); // maybe it will still boot?
//    }
//    
//    
    while (remaining > 0 && !badUpdate) {

        ESP_LOGI(TAG, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */
        if ((received = httpd_req_recv(req, buf, MIN(remaining, SCRATCH_BUFSIZE))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry if timeout occurred */
                continue;
            }

            badUpdate = true;
            if (isTIMEZONE) { // tzmap file
            }
            else if(isEXECUTABLE){
                rc = esp_ota_abort(otaHandle);
            }
            else { // SPIFFS
                /* In case of unrecoverable error,
                 * close and delete the unfinished file*/
                fclose(fd);
                unlink(filepath);
            }
            ESP_LOGE(TAG, "File reception failed!");
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, badUpdate ? "Sorry, bad error, retry, good luck." : "Failed to receive file");
            return ESP_OK;
        }

        if (isTIMEZONE) { // tzmap file
            if (received) {
                // the partition is not encrypted, so no restrictions on sizes...
                rc = esp_partition_write(timezonePartition, completed, buf, received);
                if( rc != 0 ) {
                    printf("error esp_partition_write rc=%d\n", rc);
                    badUpdate = true;
                }
            }
        }
        else if( isEXECUTABLE) {
            rc = esp_ota_write(otaHandle, buf, received);
            if(rc != ESP_OK){
                badUpdate = true;
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "OTA invalid, esp_ota_write did not accept it. You should reboot the clock.");
                return ESP_OK;
            }
            printf("esp_ota_write = %d\n",rc);
        }
        else {
            /* Write buffer content to file on storage */
            if (received && (received != fwrite(buf, 1, received, fd))) {
                /* Couldn't write everything to file!
                 * Storage may be full? */
                fclose(fd);
                unlink(filepath);
                badUpdate = true;
                ESP_LOGE(TAG, "File write failed!");
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write file to storage");
                return ESP_OK;
            }
        }
        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= received;
        completed += received;
    }
    if (isTIMEZONE) { // tzmap file
        if(badUpdate) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "timezone update failed, retry, and good luck.");
            return ESP_OK;
        }
//        if(isEXECUTABLE) { // this worked to get the sha256 checksum, but is done by ota now
//            // checksum? gTimeZoneData
//            unsigned char sha[32];
//            printf("appended begin\n");
//            DumpHex((const unsigned char *)(gTimeZoneData)+sizeof(sSmallTimeZone),0,256);
//            printf("appended sha256\n");
//            // completed includes the dummy header (and the sha256, of course)
//            DumpHex((const unsigned char *)(gTimeZoneData)+sizeof(sSmallTimeZone),completed-sizeof(sSmallTimeZone)-32,32);
//            esp_sha(SHA2_256, 
//                (const unsigned char *)(gTimeZoneData)+sizeof(sSmallTimeZone), // skip over dummy timezone to start of executable
//                completed-sizeof(sSmallTimeZone)-32, // leave out the 32 byte sha256 that should match 
//                sha);
//            printf("calculated sha256\n");
//            DumpHex(sha,0,32);
//
//            // we don't start at the front of the partition, this can't work...
//            //esp_image_verify(ESP_IMAGE_VERIFY, const esp_partition_pos_t *part, esp_image_metadata_t *data);
//            
//        }
    }
    else if(isEXECUTABLE){
        if(!badUpdate){
            rc = esp_ota_end(otaHandle);
            printf("esp_ota_end = %d\n",rc);
            if(rc==ESP_OK){
                rc =  esp_ota_set_boot_partition(otaPartition);
                printf("esp_ota_set_boot_partition = %d\n",rc);
                ESP_LOGI(TAG, "OTA reception complete");
                printf("NOT restarting new executable...\n");
                //esp_restart(); // a graceful shutdown spews more messages to the console for several seconds
                httpd_resp_set_status(req, "303 See Other");
                httpd_resp_set_hdr(req, "Location", "/reboot.html");
                httpd_resp_sendstr(req, "Update installed, please reboot.");
                return ESP_OK;
            }
            else {
                badUpdate=true;
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "OTA invalid, esp_ota_end did not accept it.");
                return ESP_OK;
            }
        }
    }
    else {
        /* Close file upon upload completion */
        fclose(fd);
    }
    ESP_LOGI(TAG, "File reception complete");
    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/directory.html");
    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
}

/* Handler to delete a file from the server */
static esp_err_t delete_post_handler(httpd_req_t *req)
{
// ADMINPW
// not using this idea ...     printf("delete ctx=%s\n",req->sess_ctx==NULL ? "--" : (char*)(req->sess_ctx));
    bool loggedIn = isLoggedIn(req);
    if(!loggedIn){ // ADMINPW required for delete
        httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Red Clock\"");
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Password required");
        return ESP_OK; // Must return OK, not FAIL
    }

    char filepath[FILE_PATH_MAX]; // 48
    struct stat file_stat;

    /* Skip leading "/delete" from URI to get filename */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri  + sizeof("/delete") - 1, sizeof(filepath));
    if (!filename) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (filename[strlen(filename) - 1] == '/') {
        ESP_LOGE(TAG, "Invalid filename : %s", filename);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
        return ESP_FAIL;
    }

    if (stat(filepath, &file_stat) == -1) {
        ESP_LOGE(TAG, "File does not exist : %s", filename);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File does not exist");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Deleting file : %s", filename);
    /* Delete file */
    unlink(filepath);

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/directory.html");
    httpd_resp_sendstr(req, "File deleted successfully");
    return ESP_OK;
}

/* Function to start the file server */
esp_err_t start_file_server(const char *base_path)
{
    static struct file_server_data *server_data = NULL;

    /* Validate file storage base path */
    if (!base_path || strcmp(base_path, "/spiffs") != 0) {
        ESP_LOGE(TAG, "File server presently supports only '/spiffs' as base path");
        return ESP_ERR_INVALID_ARG;
    }

    if (server_data) {
        ESP_LOGE(TAG, "File server already started");
        return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for server data */
    server_data = calloc(1, sizeof(struct file_server_data));
    if (!server_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");
        return ESP_ERR_NO_MEM;
    }
    strlcpy(server_data->base_path, base_path,
            sizeof(server_data->base_path));

    httpd_handle_t server = NULL;
#if USE_HTTPS
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    config.httpd.core_id = 0;
    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.httpd.uri_match_fn = httpd_uri_match_wildcard;
    // THESE MUST BE NUL TERMINATED, use EMBED_TXTFILES not EMBED_FILES
    // see https://github.com/espressif/esp-idf/issues/5177
    extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
    extern const unsigned char cacert_pem_end[]   asm("_binary_cacert_pem_end");
    config.cacert_pem = cacert_pem_start;
    config.cacert_len = cacert_pem_end - cacert_pem_start;
    printf("config.cacert_len=%d\n",config.cacert_len);
    printf("config.cacert_pem=%.*s\n",config.cacert_len,config.cacert_pem);
    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
    config.prvtkey_pem = prvtkey_pem_start;
    config.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;
    printf("config.prvtkey_len=%d\n",config.prvtkey_len);
    printf("config.prvtkey_pem=%.*s\n",config.prvtkey_len,config.prvtkey_pem);
    // httpd_ssl_start *does* work, but...
    // 1) very slow initial access to a page (not too bad on repeated json queries)
    // 2) the min heap goes from 120K to 10K (pretty scary!)
    // 3) does not allow http AND https...everything becomes https on 443
    // probably won't use it.

    ESP_LOGI(TAG, "Starting HTTP Server");
    if (/*httpd_start*/httpd_ssl_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }
#else
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
//    config.recv_wait_timeout = 30; // I'm getting W (3467148) httpd_txrx: httpd_sock_err: error in send : 104
//    config.send_wait_timeout = 30; // https://scientric.com/2019/11/07/esp32-cam-stream-capture/ (this is not the fix)
    config.core_id = 0;
    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;
    ESP_LOGI(TAG, "Starting HTTP Server");
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }
#endif


// https://www.esp32.com/viewtopic.php?t=9219 -- precedence of handlers --
/*
Make sure to register the ****** valid URIs first and then register handler for "/ *" *******
*/

// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_server.html#_CPPv422httpd_uri_match_func_t
/*
bool httpd_uri_match_wildcard(const char *uri_template, const char *uri_to_match, size_t match_upto)
    Test if a URI matches the given wildcard template.
    Template may end with “?” to make the previous character optional (typically a slash), “*” for a wildcard match, and “?*” to make the previous character optional, and if present, allow anything to follow.
    Example:
        * matches everything
        /foo/? matches /foo and /foo/
        /foo/ * (sans the backslash) matches /foo/ and /foo/bar, but not /foo or /fo
        /foo/?* or /foo/ *? (sans the backslash) matches /foo/, /foo/bar, and also /foo, but not /foox or /fo
    The special characters “?” and “*” anywhere else in the template will be taken literally.
*/

    /* URI handler for getting uploaded files */
    httpd_uri_t file_download = {
        .uri       = "*",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = download_get_handler, // most everything comes through here, like /setup.html
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_download);

    /* URI handler for uploading files to server */
    httpd_uri_t file_upload = {
        .uri       = "/upload/*",   // Match all URIs of type /upload/path/to/file
        .method    = HTTP_POST,
        .handler   = upload_post_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_upload);

    /* URI handler for deleting files from server */
    httpd_uri_t file_delete = {
        .uri       = "/delete/*",   // Match all URIs of type /delete/path/to/file
        .method    = HTTP_POST,
        .handler   = delete_post_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_delete);

    return ESP_OK;
}

// https://stackoverflow.com/questions/2673207/c-c-url-decode-library
void urldecode2(char *dst, const char *src) // https://stackoverflow.com/users/2012498/thomash
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A'; // uppercase 
                        if (a >= 'A')
                                a -= ('A' - 10); // 10..15
                        else
                                a -= '0'; // 0..9
                        if (b >= 'a')
                                b -= 'a'-'A'; // ditto
                        if (b >= 'A')
                                b -= ('A' - 10); // ditto
                        else
                                b -= '0'; // ditto
                        *dst++ = 16*a+b;
                        src+=3;
                } else if (*src == '+') {
                        *dst++ = ' ';
                        src++;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}

// called once, from main, to load any saved NVS non-default values on top of the defaults
void loadNVSparms() {
    nvs_handle_t handle;
    nvs_open("RedClock", NVS_READONLY, &handle);
    for(int k = 0; k < DIM(formDatas); k+=1) {
        struct FormData const *fdata = formDatas[k];
        
        size_t required_size;
        esp_err_t rc = nvs_get_str( handle, fdata->key, NULL, &required_size);
        // ESP_ERR_NVS_NOT_FOUND or ESP_OK is expected    
        if ( rc == ESP_OK ) {
            char *param = malloc(required_size);
            ESP_ERROR_CHECK(nvs_get_str( handle, fdata->key, param, &required_size));
            free((*fdata->currentValue)[0]);
            (*fdata->currentValue)[0] = param;
            int startpass = strlen(fdata->key) - 4; // "pass" is last 4 letters in key= "wfipass" and "mypass"
            bool ispass = ((0<startpass)&&(startpass<7)) ? strcmp((fdata->key)+startpass,"pass")==0 : false;
            ESP_LOGI(TAG, "  override parameter for %s=%s (default=%s) on core=%d",fdata->key, ispass ? "***" : param, (*fdata->currentValue)[1], xPortGetCoreID());
        }
        else if (rc == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGI(TAG, "NO override parameter for %s (default: %s==%s)",fdata->key, (*fdata->currentValue)[0], (*fdata->currentValue)[1]);
        }
        else {
            //ESP_ERROR_CHECK(rc);
            if(rc==ESP_ERR_NVS_INVALID_HANDLE)
                printf("ERROR: ESP_ERR_NVS_INVALID_HANDLE in loadNVSparms ******\n");
            else
                printf("ERROR: nvs error %d in loadNVSparms ******\n", rc);
            
        }
    }
    nvs_close( handle );
}

void resetNVSparms() {
    nvs_handle_t handle;
    nvs_open("RedClock", NVS_READWRITE, &handle);
    for(int k = 0; k < DIM(formDatas); k+=1) {
        struct FormData const *fdata = formDatas[k];
        
        size_t required_size;
        esp_err_t rc = nvs_get_str( handle, fdata->key, NULL, &required_size);
        // ESP_ERR_NVS_NOT_FOUND or ESP_OK is expected    
        if ( rc == ESP_OK ) {
            ESP_ERROR_CHECK( nvs_erase_key( handle, fdata->key) );
        }
        else if (rc == ESP_ERR_NVS_NOT_FOUND) {
        }
        else {
            printf("ERROR: nvs error %d in loadNVSparms ******\n", rc);            
        }
    }
    nvs_close( handle );
}


/////////// leftovers


// these are all moved to spiffs now; there is still a small embedded html example elsewhere for the upload script
        /* If file not present on SPIFFS check if URI
         * corresponds to one of the hardcoded paths */
//        if (strcmp(filename, "/index.html") == 0) {
//            return redirect_to_setup_html_handler(req);
//        } 
//        else if (strcmp(filename, "/favicon.ico") == 0) {
//            return favicon_get_handler(req);
//        } else if (strcmp(filename, "/masonry.pkgd.min.js") == 0) {
//            return masonry_get_handler(req);
//        } else if (strcmp(filename, "/jquery-3.5.1.min.js") == 0) {
//            return jquery_get_handler(req);
//        }
// * Handler to respond with an icon file embedded in flash.
// * Browsers expect to GET website icon at URI /favicon.ico.
// * This can be overridden by uploading file with same name 
//static esp_err_t favicon_get_handler(httpd_req_t *req)
//{
//    extern const unsigned char favicon_ico_start[] asm("_binary_favicon_ico_start");
//    extern const unsigned char favicon_ico_end[]   asm("_binary_favicon_ico_end");
//    const size_t favicon_ico_size = (favicon_ico_end - favicon_ico_start);
//    httpd_resp_set_type(req, "image/x-icon");
//    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=3600, immutable"); //https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control
//    httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_size);
//    return ESP_OK;
//}
//
// // the "masonry" javascript slides the form panels about in a fun way... this is a 25K file
//static esp_err_t masonry_get_handler(httpd_req_t *req) { // https://masonry.desandro.com/
//    extern const unsigned char masonry_pkgd_min_js_start[] asm("_binary_masonry_pkgd_min_js_start");
//    extern const unsigned char masonry_pkgd_min_js_end[]   asm("_binary_masonry_pkgd_min_js_end");
//    const size_t masonry_pkgd_min_js_size = (masonry_pkgd_min_js_end - masonry_pkgd_min_js_start);
//    httpd_resp_set_type(req, "text/javascript");
//    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=3600, immutable"); //https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control
//    httpd_resp_send(req, (const char *)masonry_pkgd_min_js_start, masonry_pkgd_min_js_size);
//    return ESP_OK;
//}
//
//static esp_err_t jquery_get_handler(httpd_req_t *req) {
//    extern const unsigned char jquery_3_5_1_min_js_start[] asm("_binary_jquery_3_5_1_min_js_start");
//    extern const unsigned char jquery_3_5_1_min_js_end[]   asm("_binary_jquery_3_5_1_min_js_end");
//    const size_t jquery_3_5_1_min_js_size = (jquery_3_5_1_min_js_end - jquery_3_5_1_min_js_start);
//    httpd_resp_set_type(req, "text/javascript");
//    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=3600, immutable"); //https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control
//    httpd_resp_send(req, (const char *)jquery_3_5_1_min_js_start, jquery_3_5_1_min_js_size);
//    return ESP_OK;
//}

// not using the arcane quotation macro
//#define q(x) qinternal(x) // add quotation marks around x. this layer expands x
//#define qinternal(x) #x // and this layer adds the quotation marks.

