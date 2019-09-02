#ifndef __BSP_FDC2214_H_
#define __BSP_FDC2214_H_

void WriteRegfdc2214(unsigned char add,unsigned int value);
unsigned int ReadRegfdc2214(unsigned char add);
void InitSinglefdc2214(void);
void InitMultifdc2214(void);//Ë«Í¨µÀ;


#endif