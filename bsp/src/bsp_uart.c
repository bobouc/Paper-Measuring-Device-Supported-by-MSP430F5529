#include "bsp.h"

#define DISABLE_INT   UART_SR=__get_SR_register(); _DINT()
#define RESTORE_INT   if(UART_SR & GIE)            _EINT()
unsigned char UART_SR;

unsigned char UART_TXBUF[UART_TXBUFSIZE];       //发送FIFO数组
unsigned char UART_RXBUF[UART_RXBUFSIZE];       //接收FIFO数组
unsigned char UART_TXIndexR = 0;                //发送FIFO的读指针
unsigned char UART_TXIndexW = 0;                //发送FIFO的写指针
unsigned char UART_RXIndexR = 0;                //接收FIFO的读指针
unsigned char UART_RXIndexW = 0;                //接收FIFO的写指针
unsigned char UART_OutLen = 0;                  //发送FIFO内待发送的字节数
unsigned char UART_InLen = 0;                   //接收FIFO内待读取的字节数

const int UCBRS_Table[8] = {UCBRS_0,UCBRS_1,UCBRS_2,UCBRS_3,UCBRS_4,UCBRS_5,UCBRS_6,UCBRS_7};
/*******************************************************************************
函数名称：UART_Init()
函数功能：UART初始化
入口参数：
  Baud：      波特率    (300~115200)
  Parity：    奇偶校验位('n'=无校验  'p'=偶校验  'o'=奇校验)
  DatsBits：  数据位位数(7或8)
  StopBits：  停止位位数(1或2)
出口参数：返回1表示初始化成功，返回0表示初始化失败
应用范例：UART_Init(9600,'n',8,1);
函数说明：
*******************************************************************************/
char UART_Init(long int Baud,char Parity,char DataBits,char StopBits)
{
  unsigned long int fSMCLK = 4000000;         //串口的时钟源选择的是辅助时钟,所以此处的值要随实际的辅助时钟而改变
  unsigned int UCBRS_Index;
  UART_TXD_SEL |= UART_TXD_PIN;               //配置IO口
  UART_RXD_SEL |= UART_RXD_PIN;
  UCAxCTL1 |= UCSWRST;
  
  UCAxCTL0 &=~(UCPEN+UCPAR+UC7BIT+UCSPB);     //清除之前设置
  //------------------------------设置波特率------------------------------
  if (Baud > 115200)  return (0);
  else if (Baud >= 9600)
  {
    UCAxCTL1 |= UCSSEL__SMCLK;
    UCAxBRW = fSMCLK/Baud;
    UCBRS_Index = fSMCLK*8/Baud - 8*UCA1BRW;
  }
  else if (Baud >= 300)
  {
    UCAxCTL1 |= UCSSEL__ACLK;
    UCAxBRW = F_ACLK/Baud;
    UCBRS_Index = F_ACLK*8/Baud - 8*UCA1BRW;
  }
  else return (0);
  //------------------------------设置校验位------------------------------
  switch (Parity)
  {
  case 'n':case 'N':  UCAxCTL0 &=~UCPEN;break;          //无校验
  case 'p':case 'P':  UCAxCTL0 |= UCPEN+UCPAR;break;    //偶校验
  case 'o':case 'O':  UCAxCTL0 |= UCPEN;break;          //奇校验
  default:  return (0);                                 //参数错误
  }
  //------------------------------设置数据位------------------------------
  switch (DataBits)
  {
  case 7:case '7':    UCAxCTL0 |= UC7BIT;break;         //7位数据位
  case 8:case '8':    UCAxCTL0 &=~UC7BIT;break;         //8位数据位
  default:  return (0);                                 //参数错误
  }
  //------------------------------设置停止位------------------------------
  switch (StopBits)
  {
  case 1:case '1':    UCAxCTL0 &=~UCSPB;break;          //1位停止位
  case 2:case '2':    UCAxCTL0 |= UCSPB;break;          //2位停止位
  default:  return (0);                                 //参数错误
  }
   
  UCAxMCTL = UCBRS_Table[UCBRS_Index]+UCBRF_0;
  UCAxCTL1 &= ~UCSWRST;
  UCAxIE |= UCRXIE;           //开启接收中断
  
  return (1);                 //初始化成功
}

/*******************************************************************************
函数名称：UART_PutChar()
函数功能：UART 发送一个字节
入口参数：
  Chr：   待发送的一字节数据
出口参数：返回1表示发送成功，返回0表示发送失败
应用范例：UART_PutChar('a');
函数说明：发送过程中，不阻塞CPU运行
*******************************************************************************/
char UART_PutChar(unsigned char Chr)
{
  if (UART_OutLen == UART_TXBUFSIZE)  return(0);
  if (UART_OutLen == 0)  UCAxIFG |= UCTXIFG; //若为第一个字节,人为制造第一次发送中断
  DISABLE_INT;
  UART_OutLen++;
  UART_TXBUF[UART_TXIndexW] = Chr;          //发送数据通过写指针写入FIFO
  if (++UART_TXIndexW >= UART_TXBUFSIZE)  UART_TXIndexW = 0;
  RESTORE_INT;
  UCAxIE |= UCTXIE;                         //开启发送中断
  return (1);
}

/*******************************************************************************
函数名称：UART_GetChar()
函数功能：UART 读取一个字节
入口参数：
  *Chr：  读取数据所存放的地址指针
出口参数：返回1表示读取成功，返回0表示读取失败
应用范例：if (ch = UART_GetChar()) ...;
函数说明：读取过程中，不阻塞CPU运行
*******************************************************************************/
char UART_GetChar(unsigned char *Chr)
{
  if (UART_InLen == 0) return (0);          //若FIFO内无数据，返回0
  DISABLE_INT;
  UART_InLen--;
  *Chr = UART_RXBUF[UART_RXIndexR];         //通过读指针从FIFO中读取数据并接收
  if (++UART_RXIndexR >= UART_RXBUFSIZE)  UART_RXIndexR = 0;
  RESTORE_INT;
  return (1);
}

/*******************************************************************************
函数名称：UART_WaitChar()
函数功能：UART 等待一个字节
入口参数：
出口参数：
应用范例：
函数说明：
*******************************************************************************/
char UART_WaitChar()
{ 
//  DIR485_IN;
//  while((IFG1&URXIFG0)==0)WDTCTL=WDT_ARST_1000;//喂狗 
//  return(RXBUF0);
  return (1);
}

/*******************************************************************************
函数名称：UART_GetCharsInRxBuf()
函数功能：获取FIFO内已接收的数据字节数
入口参数：无
出口参数：FIFO内已接收到的数据字节数
*******************************************************************************/
unsigned int UART_GetCharsInRxBuf(void)
{
  return (UART_InLen);
}

/*******************************************************************************
函数名称：UART_ClrRxBuf()
函数功能：清空接收FIFO
入口参数：无
出口参数：无
*******************************************************************************/
void UART_ClrRxBuf(void)
{
  DISABLE_INT;
  UART_InLen = 0;
  UART_RXIndexR = 0;
  UART_RXIndexW = 0;
  RESTORE_INT;
}

/*******************************************************************************
函数名称：UART_IsTxEmpty()
函数功能：判断发送FIFO是否为空
入口参数：无
出口参数：返回1表示发送FIFO为空，返回0表示发送FIFO非空
*******************************************************************************/
char UART_IsTxEmpty(void)
{
  if (UART_OutLen == 0)  return(1);
  else  return(0);
}


/*******************************************************************************
函数名称：UART_ISR()
函数功能：UART 接收/发送中断服务程序
*******************************************************************************/
#pragma vector=USCI_Ax_VECTOR
__interrupt void UART_Ax_ISR(void)
{
  switch(__even_in_range(UCAxIV,4))
  {
  case 0:break;                 // Vector 0 - no interrupt
  case 2:                       // Vector 2 - RXIFG
    UART_InLen++;
    UART_RXBUF[UART_RXIndexW] = UCAxRXBUF;    //接收数据通过写指针写入FIFO
    if (++UART_RXIndexW >= UART_RXBUFSIZE)  UART_RXIndexW = 0;
    break;
  case 4:                       // Vector 4 - TXIFG
    if (UART_OutLen > 0)                      //若FIFO内有待发送的数据
    {
      UART_OutLen--;
      UCAxTXBUF = UART_TXBUF[UART_TXIndexR];  //通过读指针从FIFO中读取数据并发送
      while (!(UCAxIFG & UCTXIFG));
      if (++UART_TXIndexR >= UART_TXBUFSIZE)  UART_TXIndexR = 0;
    }
    else UCAxIE &=~UCTXIE;                    //数据发送完毕，关闭发送中断
    break;
  default: break;
  }  
}

/*===================以下是串口终端设备接口函数库==========================*/

#define LINE_LENGTH 20          /* 行缓冲区大小，决定每行最多输入的字符数*/

/*标准终端设备中，特殊ASCII码定义，请勿修改*/
#define In_BACKSP 0x08          /* ASCII  <--  (退格键)  */
#define In_DELETE 0x7F          /* ASCII <DEL> (DEL 键)  */
#define In_EOL '\r'             /* ASCII <CR>  (回车键)  */
#define In_SKIP '\3'            /* ASCII control-C */
#define In_EOF '\x1A'           /* ASCII control-Z */

#define Out_DELETE "\x8 \x8"    /* VT100 backspace and clear */
#define Out_SKIP "^C\n"         /* ^C and new line */
#define Out_EOF "^Z"            /* ^Z and return EOF */
#include "stdio.h"

/****************************************************************************
函数名称：putchar()
函数功能：向标准终端设备发送一字节数据(1个ASCII字符)
入口参数：
  ch：    待发送的字符  
出口参数：发出的字符
函数说明：printf函数会调用该函数作为底层输出。这里从串口输出字符到PC机的超级终端
          软件上，printf的结果将打印到超级终端上。若修改该函数，将字符以其他方式
          输出，如显示到LCD上，printf的结果将显示在LCD上。
****************************************************************************/
int putchar(int ch)
{
  if (ch == '\n')        //  '\n'(回车)扩展成 '\n''\r' (回车+换行) 
  {
    UART_PutChar(0x0d) ; //'\r'
  }
  UART_PutChar(ch);      //从串口发出数据
  return (ch);
}
/****************************************************************************
函数名称：put_message()
函数功能：向标准终端设备发送一个字符串
入口参数：
  *s：    字符串(数组)头指针(数组名)
出口参数：无
****************************************************************************/
static void put_message(char *s)
{
  while (*s)        //当前字符不为空 (字符串以0x00结尾)
    putchar(*s++);  //输出一个字符，指针指向下一字符
}

/****************************************************************************
函数名称：getchar()
函数功能：从标准终端设备接收一字节数据(1个ASCII字符)
入口参数：无
出口参数：收到的字符
函数说明：scanf函数会调用该函数作为底层输入。这里从PC机键盘借助超级终端软件通过
          串口输入字符到单片机上。scanf函数的输入即源自PC机键盘。若修改该函数，
          将字符以其他方式输入，如单片机IO口，可用按钮向scanf函数输入数据。本函
          数带有缓存，能够处理退格等删除操作。若不需要删除操作，可直接调用
          UART_GetChar()函数。
****************************************************************************/
int getchar(void)
{
  static char io_buffer[LINE_LENGTH + 2];     /* Where to put chars */
  static int ptr;                             /* Pointer in buffer */
  unsigned char c;

  for (;;)
  {
    if (io_buffer[ptr])
      return (io_buffer[ptr++]);
    ptr = 0;
    for (;;)
    {
      if (UART_GetChar(&c))
      {
        if (c == In_EOF && !ptr)
        {
          put_message(Out_EOF);
          return EOF;
        }
        if ((c == In_DELETE)||(c==In_BACKSP))
        {
          if (ptr)
          {
            ptr--;
            put_message(Out_DELETE);
          }
        }
        else if (c == In_SKIP)
        {
          put_message(Out_SKIP);
          ptr = LINE_LENGTH + 1;  /* Where there always is a zero... */
          break;
        }
        else if (c == In_EOL)
        {
          putchar(io_buffer[ptr++] = '\n');
          io_buffer[ptr] = 0;
          ptr = 0;
          break;
        }
        else if (ptr < LINE_LENGTH)
        {
          if (c >= ' ')
          {
            putchar(io_buffer[ptr++] = c);
          }
        }
        else
        {
          putchar('\7');
        }
      }
    }
  }
}