
#ifndef __BSP_H_
#define __BSP_H_

#include<msp430f5529.h>
#include "bsp_sysclk.h"
#include "bsp_uart.h"
#include "bsp_iic.h"
#include "bsp_fdc2214.h"

#define F_CPU  8000000        //根据当前主频(MCLK)计算，保证延时不会随主频变化(这是利用430内部自带的延时，很精准)
#define delay_us(A)  __delay_cycles( F_CPU/1000000.0 *A )
#define delay_ms(A)  __delay_cycles( F_CPU/1000.0 *A )
#define delay_s(A)   __delay_cycles( F_CPU/1.0*A )




extern void bsp_Init(void);
#endif 