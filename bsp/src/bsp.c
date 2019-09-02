#include "bsp.h"

void bsp_Init(void)
{
  init_CLK();
  UART_Init(9600,'n',8,1);
  IIC_Init();
  InitSinglefdc2214();
}