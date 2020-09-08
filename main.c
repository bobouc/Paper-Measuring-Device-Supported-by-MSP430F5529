#include "bsp.h"
#include "math.h"
#include "string.h"
#define uchar unsigned char
unsigned int kong,jiandao,shitou,bu,one,five,ten;
uchar spkshitou=0,spkjiandao=0,spkbu=0,spkone=0,spktwo=0,spkthree=0,spkfour=0,spkfive=0;
uchar P11num = 0;
uchar P11num1 = 0;
uchar taba[]=   {"    校准模式    "};
uchar tabb[]=   {"    计数模式    "};
uchar tablearnone[]=   {"    纸张数1    "};
uchar tablearnfive[]=   {"    纸张数5    "};
uchar tablearnten[]=   {"   纸张数10  "};
uchar tabkong[]={"      短路      "};
uchar tabopen[]={"      开路      "};
uchar tabend[]={"    校准完成    "};

uchar tabinit[]={"    正在校验    "};

uchar tabledata[]={"                "};
uchar welcome0[]={"欢迎使用"};
uchar welcome1[]={"纸张计数装置"};
uchar mode = 0;
unsigned long temp,tem[20]={0};
unsigned long  temp1,temp2,tempand,tempand_pre, tempand_aft,tempand_abs ;   //10
//unsigned int  pre_value[]={48900,4660,5639,6215,6606,6880,7110,7267,7407,7514,7606,
//                                  7680,7747,7810,7869,7924,7977,8021,8059,8092,8122,
//                                  8149,8176,8201,8224,8245,8266,8285,8302,8322,8336,
//                                  8352};
unsigned int pre_value[50] = {0};
unsigned int read_value[50] = {0};
unsigned int *pFlash;
unsigned char kongstate=0;   
unsigned char num=0;
void delay5ms(void)
{
  unsigned int i = 4000;   //40000
  while(i!=0)
  {
    i--;
  }
}

void write_cmd(uchar cmd)
{
  P1OUT&= ~BIT4;
  P1OUT&= ~BIT6;
  P1OUT|=  BIT5;
  delay5ms();
  P3OUT = cmd;
  delay5ms();
  P1OUT&= ~BIT5;
}

void write_dat(uchar dat)
{
  P1OUT|= BIT4;
  P1OUT&= ~BIT6;
  P1OUT|=  BIT5;
  delay5ms();
  P3OUT = dat;
  delay5ms();
  P1OUT&= ~BIT5;
}

void  lcd_pos(uchar x,uchar y)
{
  uchar pos;
  if(x == 0)
  {x = 0x80;}
  else if (x == 1)
  {x = 0x90;}
  else if (x == 2)
  {x = 0x88;}
  else if (x == 3)
  {x = 0x98;}
  pos = x + y;
  write_cmd(pos);
}

void LCD_init(void)
{
  write_cmd(0x38);
  delay5ms();
  write_cmd(0x0C);
  delay5ms();
  write_cmd(0x01);
  delay5ms();
}

void init_port(void)
{
  P1DIR &= ~BIT1;
  P2DIR &= ~BIT1;
  P2DIR |= BIT5;
  P1IES |= BIT1;
  P2IES |= BIT1;
  P1OUT |= BIT1;
  P2OUT |= BIT1;
  P2OUT |= BIT5; //蜂鸣器
  P1REN |= BIT1;
  P2REN |= BIT1;
  P1IE  |= BIT1;
  P2IE  |= BIT1;
  P1IFG &= ~BIT1;
  P2IFG &= ~BIT1;
  P2IFG = 0;
  P1IFG = 0;
  
}
#pragma vector = PORT2_VECTOR
__interrupt void Port2_ISR(void)                       //模式转换
{
 // while(P2IN&BIT1 == 0);
 delay_ms(500); //200
  mode++;
  if(mode >= 3)
    mode =1;
  if(mode == 1)         //校准模式
  {
    lcd_pos(0,0);
        for(int i = 0;i<16;i++)
        {
          write_dat(taba[i]);     
        }
    lcd_pos(1,0);
        for(int i = 0;i<16;i++)
        {
          write_dat(tabinit[i]);     
        }
    lcd_pos(2,0);
    for(int i = 0;i<16;i++)
    {
      write_dat(tabledata[i]);     
    }        
    
        
  }
  
    if(mode == 2)       //计数模式       
  {
      lcd_pos(0,0);
        for(int i = 0;i<16;i++)
        {
          write_dat(tabb[i]);     
        }
      lcd_pos(1,0);
      for(int i = 0;i<16;i++)
        {
          write_dat(tabledata[i]);    //空格冲掉 
        }
      lcd_pos(2,0);
      for(int i = 0;i<16;i++)
        {
          write_dat(tabledata[i]);    //空格冲掉 
        }      
      pFlash = (unsigned int *)0x1800;    
      for(int i = 0; i < 50; i++){
        read_value[i] = *pFlash++;      
      }
      
  }
      P2IFG = 0;
}


#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)                //数据写入, 数量判别
{
   //while(P1IN&BIT1 == 0);
  delay_ms(500);

  if(mode == 2 ){
    P11num1 = 1;
      num++;
  
  }

    //校准模式
  if(mode == 1 ){
    pre_value[P11num] = (unsigned int)temp;
    lcd_pos(2,0);
    for(int i = 0;i<16;i++)
    {
      write_dat(tabledata[i]);     
    }       
    if(P11num == 0){
      lcd_pos(2,0);
      for(int i = 0;i<16;i++)
      {
        write_dat(tabkong[i]);   //显示短路  
      }
    }
    else if(P11num == 1){
      lcd_pos(2,0);
      for(int i = 0;i<16;i++)
      {
        write_dat(tabopen[i]);  //显示开路   
      }  
    }
    else{
      lcd_pos(2,3);                             //P11num = 2, 显示1，说明录入1完成
      write_dat((P11num-1)/10+0x30);
      write_dat((P11num-1)%10+0x30);
    }
  }
  
  if(P11num == 43){                            //P11num = 3, 显示2，录入2完成； P11num = 44, 显示43， 录入43完成。 
      pFlash = (unsigned int *)0x1800;
      FCTL3 = FWKEY;
      FCTL1 = FWKEY + ERASE;
      *pFlash = 0;
      while(FCTL3 & BUSY);
      pFlash = (unsigned int *)0x1800;
      FCTL1 = FWKEY + WRT;

      for(int i = 0; i < P11num; i++){
        *pFlash++ = pre_value[i];
      }
      FCTL1 = FWKEY;
      FCTL3 = FWKEY + LOCK; 
      write_dat('E');write_dat('n');write_dat('d'); 
  } 
  
    if(mode == 1)
    {
      P11num++;                         //短路 P11num = 0; 开路 1时, P11num = 3; 10时, P11num = 
    }   
        P1IFG = 0;   
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
  unsigned int sheet,sheetnum,i;
  TA0IV=0;      //中断标志位自动清零
  TA0R=32768;
  if(mode == 2&&P11num1 == 1)
  {
    if(temp)
      {
        
       printf("%lu\n",temp);
       i=0;       
       if((temp>read_value[i]-10000)&&(temp<read_value[i]+1000))
        {
          lcd_pos(2,0);
          for(int i = 0;i<16;i++)
         {
          write_dat(tabkong[i]);     
          }
          P2OUT &= ~BIT5;delay_ms(300);P2OUT |= BIT5;
        }
       else if(temp>(read_value[1]-100)&&temp<(read_value[1]+100)){
          lcd_pos(2,0);
          for(int i = 0;i<16;i++)
         {
          write_dat(tabopen[i]);     
          }
          P2OUT &= ~BIT5;delay_ms(300);P2OUT |= BIT5;             
       }       
       else
       {      for(i=2;i<=51;i++)
              {
               if(temp<=read_value[i]){sheet=i; break;}
              }           
              if(fabs(read_value[sheet]-temp)<fabs(temp-read_value[sheet-1]))
               sheetnum=sheet-1;        
              else
               sheetnum=sheet-2;
              printf("%d\n",sheetnum);
              lcd_pos(2,0);
              for(int i = 0;i<16;i++)
              {
                write_dat(tabledata[i]);     
              }
              lcd_pos(2,3);
              write_dat(sheetnum/10+0x30);        
              write_dat(sheetnum%10+0x30);
              P2OUT &= ~BIT5;delay_ms(300);P2OUT |= BIT5;
       }       

      
      }   
    }
    P11num1 = 0;
    
}
int main( void )
{
    unsigned int i,j;
    unsigned long a;
    unsigned long temm[7];
    WDTCTL = WDTPW + WDTHOLD;
    
    P1DIR  = BIT4 + BIT5 + BIT6;
    P3DIR  = 0xff;
    TA0R=32768;
    TA0CTL|=TACLR+TAIE;                //计数方式 对TACLR置位，开启中断并清零
    TA0CTL|=TASSEL_1+ID__2+MC_0+TAIE;         //选择SCLK32.768KHZ作为时钟，选用连续模式，并开启中断          //MC_0 stop TIMER0_A0      
    _EINT();   
    
    LCD_init();
    init_port();
    bsp_Init();
    
    lcd_pos(0,2);
    for(uchar i=0;i<8;i++)
      write_dat(welcome0[i]);
    lcd_pos(1,1);
    for(uchar i=0;i<12;i++)
      write_dat(welcome1[i]);
    while(1){
        if(mode == 2&&P11num1 == 1){

        TA0CTL|=TASSEL_1+ID__2+MC_2+TAIE;         //定时器开始计数   
        } 
        
        temp1 = ReadRegfdc2214(0x00);//读取第一通道的值
        temp2 = ReadRegfdc2214(0x01);
        tempand  = temp1*65536 + temp2;
        tempand = tempand/1000;   
//        for(i=0;i<19;i++)
//          tem[i]=tem[i+1];
//        tem[19]=tempand;
//        temp=0;
//        for(i=0;i<20;i++)
//          temp=temp+tem[i]; 
//        temp=temp/20; 
        for(i=0;i<6;i++)
          temm[i]=tem[i];
        for(i=0;i<6;i++)
          tem[i]=tem[i+1];
        tem[6]=tempand;
        
        for(i=0;i<6;i++)
        {
          for(j=0;j<6-i;j++)
          {
            if(temm[j]>temm[j+1])
            {
              a=temm[j];temm[j]=temm[j+1];temm[j+1]=a;
            }
          }
        }
        temp=temm[3];
        printf("%lu\n",temp);
        delay_ms(50);
    }           
}


