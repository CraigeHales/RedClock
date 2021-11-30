/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

#include "clockMain.h"

static i2c_bus_handle_t i2c_bus = NULL;
static bme280_handle_t dev = NULL;

/**
 * @brief i2c master initialization
 */
void my_i2c_bus_init()
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL; // wch added, i2c_types.h makes I2C_SCLK_SRC_FLAG_FOR_NOMAL == 0
    i2c_bus = i2c_bus_create(I2C_MASTER_NUM, &conf);
}

bme280_handle_t my_bme280_init()
{
    my_i2c_bus_init();
    if(BME280_I2C_ADDRESS_DEFAULT != 0x76)
        ESP_LOGI("BME280:", "error:%d at %d", 0x76, BME280_I2C_ADDRESS_DEFAULT);

    dev = bme280_create(i2c_bus, BME280_I2C_ADDRESS_DEFAULT);
    ESP_LOGI("BME280:", "bme280_init:%d at %d", bme280_default_init(dev), BME280_I2C_ADDRESS_DEFAULT);
    return dev;
}

void my_bme280_test_task(void* pvParameters)
{
    bme280_handle_t dev = my_bme280_init();
    //int cnt = 3;
    while (1) {
        float temp; bme280_read_temperature(dev,&temp);
        sClockState.temperature = temp*(9.0/5.0)+32.0;
        float pres; bme280_read_pressure(dev,&pres);
        sClockState.pressure = pres;
        float humi; bme280_read_humidity(dev,&humi);
        sClockState.humidity = humi;
      //  ESP_LOGI("BME280:", "temperature:%f on core=%d\n", sClockState.temperature,xPortGetCoreID());
        vTaskDelay(1000 / portTICK_RATE_MS);
      //  ESP_LOGI("BME280:", "humidity:%f on core=%d\n", sClockState.humidity, xPortGetCoreID());
//        vTaskDelay(300 / portTICK_RATE_MS);
      //  ESP_LOGI("BME280:", "pressure:%f on core = %d\n", sClockState.pressure, xPortGetCoreID());
//        vTaskDelay(300 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void my_bme280_test()
{
    
    xTaskCreatePinnedToCore(my_bme280_test_task, "my_bme280_test_task", 1024 * 2, NULL, 10, NULL, 0/*core*/);
}

//TEST_CASE("Device bme280 test", "[bme280][iot][device]")
//{
//    bme280_test();
//}



