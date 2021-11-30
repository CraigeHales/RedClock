/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

extern void my_i2c_bus_init();
extern bme280_handle_t my_bme280_init();
extern void my_bme280_test_task(void* pvParameters);
extern void my_bme280_test(); 
