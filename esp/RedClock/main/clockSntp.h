/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

extern void sntp_sync_time(struct timeval *tv);
extern void time_sync_notification_cb(struct timeval *tv);
extern void obtain_time(void);
extern void initialize_sntp(void);

