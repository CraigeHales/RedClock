/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

// if I do another hardware iteration, I should change Hten and Mten
// with Aseg and Bseg (not F or G or P). During boot the Gseg turns
// on, bright, for the Hten and Mten. The doc suggests F rather than
// G, and the P is also on...thus use A and B (or c,d,e...)

// Hour and Min, tens and ones digit
#define Hten GPIO_NUM_15 // pin 23
#define Hone GPIO_NUM_2  // pin 24
#define Mten GPIO_NUM_0  // pin 25
#define Mone GPIO_NUM_4  // pin 26

// segments A-G, decimal
#define Aseg GPIO_NUM_32 // pin 7
#define Bseg GPIO_NUM_33 // pin 8
#define Cseg GPIO_NUM_25 // pin 9
#define Dseg GPIO_NUM_26 // pin 10
#define Eseg GPIO_NUM_27 // pin 11
#define Fseg GPIO_NUM_14 // pin 12
#define Gseg GPIO_NUM_12 // pin 13
#define Pseg GPIO_NUM_13 // pin15 "point"

// GPS pulse per second on io 39
#define PPS GPIO_NUM_39 // 

#define SEGOFF 1 // brings low-side high: OFF
#define SEGON 0  // low side goes low: could be on
#define DIGOFF 0 // high side goes low: OFF
#define DIGON 1 // brings high-side high: could be on

#define DIM(x) ( (int) (sizeof(x)/sizeof(x[0])) )

#define NDIGITS 4
#define NSEGMENTS 8

#define DECIMAL 128u


#define TXD_PIN (GPIO_NUM_17)//was 4
#define RXD_PIN (GPIO_NUM_16)//was 5

#define AMBIENT ADC1_CHANNEL_6 // ADC1 channel 6 is GPIO_NUM_34 which is pin 5 on the 38-pin device CDS to +3.3, 10K to GND
#define HALLSENSOR GPIO_NUM_35 // pin 6
// pins 36,39 are (internal, not using) HALL sensor related, 39 is the PPS on board rev 1 (no change rev 2)

// pins 3,4,5,6 and probably gpio 37,38 are input only
//ADC1_CH0 (GPIO 36) pin 3 on 38 pin dev **available input??**
//ADC1_CH1 (GPIO 37)
//ADC1_CH2 (GPIO 38)
//ADC1_CH3 (GPIO 39) pin 4 on 38 pin dev PPS input
//ADC1_CH4 (GPIO 32)
//ADC1_CH5 (GPIO 33)
//ADC1_CH6 (GPIO 34) pin 5 on 38 pin dev AMBIENT input
//ADC1_CH7 (GPIO 35) pin 6 on 38 pin dev external HALLSENSOR input
//ADC2_CH0 (GPIO 4)
//ADC2_CH1 (GPIO 0)
//ADC2_CH2 (GPIO 2)
//ADC2_CH3 (GPIO 15)
//ADC2_CH4 (GPIO 13)
//ADC2_CH5 (GPIO 12)
//ADC2_CH6 (GPIO 14)
//ADC2_CH7 (GPIO 27)
//ADC2_CH8 (GPIO 25)
//ADC2_CH9 (GPIO 26)


#define I2C_MASTER_SCL_IO        22 /* was  21 */          /*!< gpio number for I2C master clock IO21*/
#define I2C_MASTER_SDA_IO        21 /* was  15 */          /*!< gpio number for I2C master data  IO15*/
#define I2C_MASTER_NUM              I2C_NUM_0 /* was 1 */   /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE   0           /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0           /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ          10000      /*!< I2C master clock frequency */


#define TIMER_DIVIDER         2//16  //  Hardware timer clock divider 2..65536
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
//#define TIMER_INTERVAL0_SEC   (7.0) // sample test interval for the first timer
#define TIMER_MULTIPLEX_SECONDS  (.00008)//min= (.000008)   // refresh rate
//#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
//#define TEST_WITH_RELOAD      1        // testing will be done with auto reload



//extern void IRAM_ATTR timer_group0_Multiplexer_isr(void *para);
extern void multiplexer_tg0_timer_init(int timer_idx,/* bool auto_reload,*/ double timer_interval_sec);
extern void timer_clock_task(void *arg);

extern unsigned char vals[NDIGITS];
extern const unsigned char ascii[256];
extern const unsigned char segs[NSEGMENTS];
extern const unsigned char digs[NDIGITS];

extern void say4(char *t,int decimals);

