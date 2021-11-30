/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

esp_err_t start_file_server(const char *base_path);
void urldecode2(char *dst, const char *src);
void loadNVSparms();
void resetNVSparms();
