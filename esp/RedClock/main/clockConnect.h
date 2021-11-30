/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

// from /home/c/esp/esp-idf/examples/common_components/protocol_examples_common/include/protocol_examples_common.h
esp_err_t my_connect(void);
esp_err_t my_disconnect(void);
esp_netif_t *myget_example_netif(void);
esp_netif_t *myget_example_netif_from_desc(const char *desc);

