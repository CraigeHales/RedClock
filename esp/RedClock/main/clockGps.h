/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

extern void uart_init(void);
extern int sendData(/*const char* logName,*/ int len, /*const*/ char* data);
extern void gpsparse(char *c);
extern void rx_task(void *arg);
extern double GpsToDecimalDegrees(const char *nmeaPos, char quadrant);
extern void gpsHotReset();
extern void gpsColdReset();

