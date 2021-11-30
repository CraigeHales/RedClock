# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# (Not part of the boilerplate)
# This example uses an extra component for common functions such as Wi-Fi and Ethernet connection.
set(EXTRA_COMPONENT_DIRS 
                         $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
                         $ENV{IDF_PATH}/../esp-iot-solution/components/bus
                         $ENV{IDF_PATH}/../esp-iot-solution/components/sensors/pressure/bme280
                         $ENV{IDF_PATH}/components/mbedtls
                         $ENV{IDF_PATH}/components/heap
#                         $ENV{IDF_PATH}/../esp-iot-solution/components/i2c_devices/i2c_bus
#                         $ENV{IDF_PATH}/../esp-iot-solution/components/i2c_devices/sensor/bme280
                         )
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
#set(SDKCONFIG_DEFAULTS sdkconfig_debug) # or sdkconfig_release
#set(SDKCONFIG_DEFAULTS sdkconfig_release) # or sdkconfig_release
project(RedClock)
