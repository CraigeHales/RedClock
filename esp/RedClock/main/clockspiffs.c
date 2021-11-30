/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

#include "clockMain.h"
#include "clockspiffs.h"

static const char *TAG="clockspiffs";

/* Function to initialize SPIFFS */
esp_err_t init_spiffs(void) // from: /home/c/esp/esp-idf/examples/protocols/http_server/file_serving/main/main.c
{
    ESP_LOGI(TAG, "Initializing SPIFFS on core %d", xPortGetCoreID());

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs", // no trailing / ? must match clockServer
      .partition_label = NULL,
      // wch: not sure about the following; 10 works even when the package already has ~25
      .max_files = 30,   // This decides the maximum number of files that can be created on the storage
      .format_if_mount_failed = false // reformatting can not help, we need these files!
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

//    size_t total = 0, used = 0;
//    ret = esp_spiffs_info(NULL, &total, &used);
//    if (ret != ESP_OK) {
//        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
//        return ESP_FAIL;
//    }
//
//    ESP_LOGI(TAG, "Partition size: total: %d, used: %d  (on core %d)", total, used, xPortGetCoreID());
    return ESP_OK;
}
