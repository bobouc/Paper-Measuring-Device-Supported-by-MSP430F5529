#ifndef __BSP_IIC_H_
#define __BSP_IIC_H_
//IO方向设置

#define IIC_SDA_IN()   P4DIR &=~ BIT1           //SDA设置为输入模式
#define IIC_SDA_OUT()  P4DIR |= BIT1            //SDA设置为输出模式

//IO操作函数
#define IIC_SCL_L      P4OUT &=~ BIT2           //设置SCL为低电平
#define IIC_SCL_H      P4OUT |= BIT2            //设置SCL为高电平
#define IIC_SDA_L      P4OUT &=~ BIT1           //设置SDA为低电平
#define IIC_SDA_H      P4OUT |= BIT1            //设置SDA为高电平
#define IIC_READ_SDA   P4IN&BIT1                //输入SDA


//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				        //发送IIC开始信号
void IIC_Stop(void);	  			      //发送IIC停止信号
void IIC_Send_Byte(unsigned char txd);			    //IIC发送一个字节
unsigned char IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
unsigned char IIC_Wait_Ack(void); 				      //IIC等待ACK信号
void IIC_Ack(void);					        //IIC发送ACK信号
void IIC_NAck(void);				        //IIC不发送ACK信号

#endif