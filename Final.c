#include "string.h"
#include "msp430g2553.h"
#include <intrinsics.h>
union reg {
 struct bit {
 unsigned char b0:1;
 unsigned char b1:1;
 unsigned char b2:1;
 unsigned char b3:1;
 unsigned char b4:1;
 unsigned char b5:1;
 unsigned char b6:1;
 unsigned char b7:1;
 }_BIT;
 unsigned char _BYTE;
};
union reg* __P1_DIRECT = (union reg*)0x22 ;  
union reg* __P1_OUT = (union reg*)0x21 ;    
//Dinh nghia vùng nho 0x21 là _P1_OUT, dung dieu khien cac bit xuat cua Port 2. 
union reg* __P1_IN = (union reg*)0x20 ;     
//Dinh nghia vùng nho 0x20 là _P1_IN, dung dieu khien cac bit nhap cua Port 2.
#define dl_1 nop() //generates 1 microsecond
#define LCDd (P2OUT)
#define SEC 0x00
#define MIN 0x01
#define HR 0x02
#define DAY 0x03 //thanh ghi th?
#define DT 0x04  //thanh ghi ngay
#define MTH 0x05
#define YEAR 0x06
#define PH 0x08   //dia chi tren RAM (tu 08h den 3Fh)
#define GIO 0x0A  
#define NGY 0x0C
#define THG 0x0E
#define ON 0x10
#define A_P 0x12
#define ON1 0x14
#define LAST 0x16
#define RS (__P1_OUT->_BIT.b0)
#define EN (__P1_OUT->_BIT.b1)
#define SCLO (__P1_OUT->_BIT.b6)
#define SDAIN (__P1_IN->_BIT.b7)
#define SDAO (__P1_OUT->_BIT.b7)
#define Down (__P1_OUT->_BIT.b2)
char sec,h24,min,day,dt,mth,y,y_20,set,ph,gio,ngy,thg,chu=0,last=0,last1,on,ona,on1,a_p=0,a,min1,day1,mth1,gio1,ph1;
char *thu[]={"SAT","SUN","MON","TUE","WED","THU","FRI","SAT"};
char *st[]={"   CHINH GIO    ","  HEN GIO TUOI   "};
char *str[]={"OFF","ON "};
unsigned char data_cgrpm[]= {
0x00,0x0E,0x15,0x17,0x11,0X0E,0x00,0x00, //dia chi chuong 
0x00,0x12,0x09,0x09,0x09,0x12,0x00, 0x00, }; //dia chi ))
void nop(void);
char chp=0;
void nop(void)
{
 int i=20;
 while(i--);
 }
//****************************************************************
void I2C_start(void)        //gui tin hieu start
{
 __P1_DIRECT-> _BIT.b6 =1;  //dinh nghia P1.6 P1.7 la out
 __P1_DIRECT->_BIT.b7 =1;   //dinh nghia moi dung dc SDAO vs SCLO
 SDAO = 1; SCLO = 1;        //cho SDA va SCL len muc 1
 dl_1;                      //delay 1 micro sec
 SDAO = 0; dl_1;
 SCLO = 0; dl_1;
}
void I2C_stop(void)          //gui tin hieu stop
{
 __P1_DIRECT->_BIT.b6 =1;    //dinh nghia P1.6 la out
 __P1_DIRECT->_BIT.b7 =1;
 SCLO = 1; SCLO = 0;
 SDAO = 0; dl_1;
 SCLO = 1; dl_1;             //set SDL va SDA len 1
 SDAO = 1;
}
unsigned char I2C_w(unsigned char dat) //ham I2C ghi 1 byte
{
 __P1_DIRECT->_BIT.b6 =1;     //dinh nghia P1.6 la out
 __P1_DIRECT->_BIT.b7 =1;
 unsigned char i;
 for (i=0;i<8;i++)
 {
 SDAO = (dat & 0x80) ? 1:0; //xem bit MSB co gia tri hay khong, neu co thi SDA=1
 SCLO=1;SCLO=0;    
 dat<<=1;                   //dich dat sang trai 1 vi tri
 }
 SCLO = 1; dl_1;
 SCLO = 0;
 return dat;
}
unsigned char I2C_r(void)   //ham I2C doc mot byte
{
 __P1_DIRECT->_BIT.b6 =1;   //dinh nghia de chan SCL la ngo ra
 __P1_DIRECT->_BIT.b7 =0;   //dinh nghia de chan SDA la ngo vao
 char rd_bit;
 unsigned char i, dat;
 dat = 0x00;
 for(i=0;i<8;i++)          /* For loop read data 1 byte */
 {
 dl_1;
 SCLO = 1; dl_1;            //SCL len 1
 rd_bit = SDAIN;            /* Keep for check acknowledge */
 dat = dat<<1 ; 
 dat = dat | rd_bit;       /* Keep bit data in dat */
 SCLO = 0;                 /* Clear SCL */
 }
 return dat;
}
//**********************************************************************
//********
//**********************************************************************
//********
// CAC CHUONG TRINH CON CHO DS1307
//**********************************************************************
//********
unsigned char DS_r(unsigned char addr)  //c.trinh lay du lieu tu DS1307
{
 unsigned int temp,ret;
 I2C_start(); /* Start i2c bus */
 I2C_w(0xD0); /* Connect to DS1307 */
 I2C_w(addr); /* Request RAM address on DS1307 */
 I2C_start(); /* Start i2c bus */
 I2C_w(0XD1); /* Connect to DS1307 for Read */
 ret = I2C_r(); /* Receive data */
 I2C_stop();
 //**********************************************************
 temp = ret; /*BCD to HEX*/
 ret = (((ret/16)*10)+ (temp & 0x0f)); /*for Led 7seg*/
 //**********************************************************
 return ret;
}

void DS_W (unsigned char addr,unsigned char dat)//c.trinh ghi du lieu len DS1307
{
 unsigned int temp;
 //**********************************************
 temp = dat ; /*HEX to BCD*/
 dat = (((dat/10)*16)|(temp %10)); /*for Led 7seg*/
 //**********************************************
 I2C_start(); /* Start i2c bus */
 I2C_w(0XD0); /* Connect to DS1307 */
 I2C_w(addr); /* Request RAM address on DS1307 */
 I2C_w(dat); /* Connect to DS1307 for Read */   //ghi du lieu dat len DS1307
 I2C_stop();
}

void R_DS()       //chuong trinh lay thong tin gio, ngay tu DS1307
{ 
sec = DS_r(SEC);  //doc giay tu DS1307 (ham DS_r la ham doc du lieu tu DS1307)
 min = DS_r(MIN);  //SEC, MIN HR, DAY,...la dia chi cua cac thanh ghi tren 
 h24 = DS_r(HR);    //DS1307 da define o phan dau)
 day = DS_r(DAY);
 dt = DS_r(DT);
 mth = DS_r(MTH);
 y = DS_r(YEAR);

 ph = DS_r(PH);     //doc tai dia chi PH tren RAM r luu vao bien ph
 gio = DS_r(GIO);
 ngy = DS_r(NGY);
 thg = DS_r(THG);
 on = DS_r(ON);
 a_p = DS_r(A_P);
 on1 = DS_r(ON1);
 last= DS_r(LAST);
}
void W_DS()       //chuong trinh ghi thong tin gio, ngay len DS1307
{
 DS_W(SEC,sec);  //ghi du lieu sec len thanh ghi SEC tren DS1307
 //DS_W(MIN,min1);  
 DS_W(HR,h24);
 //DS_W(DAY,day1);
 DS_W(DT,dt);
 //DS_W(MTH,mth1);
 DS_W(YEAR,y);

 DS_W(PH,ph);     //ghi du lieu ph len RAM
 DS_W(GIO,gio);
 DS_W(NGY,ngy);
 DS_W(THG,thg);
 DS_W(ON,on);
 DS_W(A_P,a_p);
 DS_W(ON1,on1);
 DS_W(LAST,last);
 
}

//********************************************************************
void dl(unsigned int time)
{
 while(time--);
}
//******************************************************************

// LCD 
void wait(void)
{
 int i;
 for(i=0;i<1000;i++)
 {
 }
 EN=0;// Dua xung cao xuong thap de chot
}

void LCDct(char x)        //chuong trinh dung de ghi lenh len LCD
{
 EN=1;           // Dua chan cho fep len cao
 RS=0;           // Chon thanh ghi lenh
 LCDd=x;         // LCDd la P2OUT da define o phan dau 
 EN=0;           // Xung cao xuong thap
 wait();         // Doi LCD san sang
}

void LCDinit(void)     //chuong trinh thiet lap che do hoat dong cho LCD
{
 LCDct(0x30);     //Che do 8 bit.
 LCDct(0x38);     // 2 dong va ma tran 5x7
 LCDct(0x0C);     // Bat con tro
 LCDct(0x06);     // vi tri con tro di chuyen theo su tang giam
 LCDct(0x01);     // Xoa man hinh
}

void Lw(char c)  //chuong trinh ghi 1 ki tu (8 bit) len LCD
{
 EN=1;           // Cho fep muc cao
 RS=1;           // Ghi du lieu
 LCDd=c;         // P2OUT xuat c
 EN=0;           // Xung cao xuong thap
 wait();         // Cho
}

void pc(void)  //chuong trinh ghi dia chi bieu tuong chuong va )) len CGRAM
{
unsigned char i,j;
 j = 0x40;       //dia chi bat dau cua CGRAM
 for(i = 0; i < 16; i++)
 {
 LCDct(j);       //ghi vao dia chi cua CGRAM
 Lw(data_cgrpm[i]);
 j++; 
}
}


void LCDps(char *s,unsigned char row)  //chuong trinh ghi chuoi ki tu
{                                      //len LCD, co the chon dong
unsigned char len;
if(row==1) LCDct(0x80);     //dua con tro ve dau dong 1  
else LCDct(0xC0);           //dua con tro ve dau dong 2
 len=strlen(s);             //Lay do dai bien duoc tro boi con tro
 while(len!=0)              //Khi do dai van con
 {
 Lw(*s);                    //Ghi ra LCD gia tri duoc tro boi con tro
 s++;                       // Tang con tro
 len--;                     //Tru do dai
 }
}


void LCDp(char *s)    //giong chuong trinh tren nhung khong 
{                     //chon duoc dong can ghi tren LCD
unsigned char len;
 len=strlen(s);// Lay do dai bien duoc tro boi con tro
 while(len!=0)// Khi do dai van con
 {
 Lw(*s);// Ghi ra LCD gia tri duoc tro boi con tro
 s++;// Tang con tro
 len--;// Tru do dai
 }
}
//******************************************************************
void Set_Clock(void) // chuong trinh kiem tra va hieu chinh gia tri cua gio,ph,giay
{
 unsigned char y1;
 y1=(y%4)*100;   //dung cho nam nhuan
 if(sec>59) sec=0;
 if(min>59)  min=0; 
 if(h24>23) h24=0;
 if(day>7) day=1;
 if((mth==2)&&(dt>29)&&(y1==0)) { dt=1; }
 else if((mth==2)&&(dt>28)&&(y1!=0)) { dt=1; }
 else
 if(((mth==4)||(mth==6)||(mth==9)||(mth==11))&&(dt>30)&&(y!=0)){ dt=1; }
 else if(dt>31) { dt=1; }
 if(mth>12) { mth=1; }
 if(y>99) { y_20++; y=0; if(y_20>99){y_20=0;} }
 if(ph>59) { ph=0; }
 if(gio>23) { gio=0; }
 if(last>59) last=0;
 if((thg==2)&&(ngy>29)) { ngy=1; }
 else if(((thg==4)||(thg==6)||(thg==9)||(thg==11))&&(ngy>30))
 {ngy=1; }
 else if(ngy>31) { ngy=1; }
 if(thg>12) { thg=1; }
 if(a_p!=a){ DS_W(A_P,a_p);}
 a=a_p;
 //hieu chinh hen gio
if((ph+last)>60)
{last1=(ph+last)-60; gio1=(gio+1); ph1=0;}
else {last1=last; gio1=gio; ph1=ph;}
}
//**********************************************************************
#pragma vector = TIMER0_A0_VECTOR //tao TIMER 1s de nhap nhay
__interrupt void TA0_ISR ( void )
{     chp=chp^1; }
//**********************************************************************
void hthi(char x,char yx)   //hien thi so chuc khi biet dang o set may
  {if((set==x)&&(chp==1))
    LCDp("  ");
  else
  {
    Lw((yx/10)+48);  //Lw la c.trinh ghi 1 ki tu 8bit len LCD, hang chuc cua ph
    Lw((yx%10)+48); // +48 la ra thap phan cua ma ASCII 
  }
}
//****************************************************************
void hienthi(void)
{
  /////////////////////////////dang o phan chu//////////////////////
if(set==1)  
     { if((chu==0)&&(chp==1)){LCDps("  >",1);} else LCDps(st[0],1) ;
       if((chu==1)&&(chp==1)){LCDps(" >",2);} else LCDps(st[1],2) ;}
///////////////////dang o chinh gio hoac hen gio hoac khong lam gi (set=0)
if(set!=1)  
{
  if(chu==0) //dang chinh gio hoac khong lam gi
  {
    if((set==2)&&(chp==1)) //con tro dang o AM/PM 24H va dang chinh gio
      LCDps("    ",1);
    else 
    {if(a_p==0)  //a_p=0 la chua dat AM/PM, tuc gio dang hien thi dang 24h
        {if (set==0)LCDps("    ",1); //neu che do 24h thi khi moi mo (set=0) khong can hien 24h
        else LCDps("24H ",1); //set khac 0 la o cac che do cai dat, thi hien 2H
        }
    else  //a_p=1 la da hien thi theo AM/PM r
      if(h24>12) LCDps("PM  ",1); 
      else LCDps("AM  ",1);
     }
  
    if((set==4)&&(chp==1)) //con tro dang o gio, 
      LCDp("  ");
    else 
      {if(a_p==0) //neu dang gio 24 thi doi ra chuc don vi r hien
         {Lw((h24/10)+48);
         Lw((h24%10)+48);
         } 
       else 
      { if(h24>12) //doi gio 24 ra gio 12 r thuc hien tuong tu nhu tren
         {Lw(((h24-12)/10)+48);
         Lw(((h24-12)%10)+48);
         }
    else
        {
          Lw((h24/10)+48);
          Lw((h24%10)+48);
        }
     }
    }
Lw(58);      //dau hai cham
hthi(3,min); //set bang 3 dang o phut, tac min ra thanh 2 phan de ghi len LCD 
Lw(58);      //dau hai cham
Lw((sec/10)+48);            //ghi giay hang chuc
Lw((sec%10)+48);//LCDp(" ");  //ghi giay hang don vi
////////////////////chuong/////////////////////////////////////
if(on==1||ona==1)
     {
            if(Down==0)
                {Lw(32);Lw(0);Lw(1);} //chuong co chop          
            else    //chua toi gio hen
            {Lw(32);Lw(0);Lw(32);} //chuong khong chop
    } 
else LCDp("   ");

/////////////////////////////////////////////////////////////////
if((set==5)&&(chp==1)) //dang o thu
   {LCDps("   ",2);}
else
   {LCDps(thu[day],2);}
Lw(32);  //hien dau cach
hthi(6,dt);  //dang o ngay
Lw(45);//hien dau -
hthi(7,mth); //dang o thang
Lw(45);//hien dau -
if((set==8)&&(chp==1)) //dang o nam
LCDp("    ");
else
    {
      Lw((y_20/10)+48); //ghi y_20 xong r ghi y
      Lw((y_20%10)+48);
      Lw((y/10)+48);
      Lw((y%10)+48);
    }

}


////////dang hen gio/////////////////////////////////////////////////////////////
if(chu==1)  //dang hen gio
{
       if((set==2)&&(chp==1)) //dang o ON/OFF
         {LCDps("   ",1);} 
       else {LCDps(str[on],1);}  //hien ON/OFF
       LCDp(" ");      //khoang trang giua ON/OFF va gio
if((set==4)&&(chp==1))   //dang o gio
LCDp("  ");
else
   {if(a_p==0)
    {
      Lw((gio/10)+48);
      Lw((gio%10)+48);
    }
   else
      {
        if(gio>12)
             {
               Lw(((gio-12)/10)+48);
               Lw(((gio-12)%10)+48);
             }
        else
             {
               Lw((gio/10)+48);
               Lw((gio%10)+48);}
      }
   }
Lw(58);       //dau hai cham
hthi(3,ph);   //set=3 dang o phut, tach ph ra thanh 2 so chuc va don vi de in ll len LCD
LCDp("   ");
hthi(5,last);
LCDp("  ");
if((set==6)&&(chp==1)) //dang o ON/OFF
         {LCDps("   ",2);} 
       else {LCDps(str[ona],2);}  //hien ON/OFF
       LCDp(" ");      //khoang trang giua ON/OFF va gio
hthi(7,ngy);  //dang o ngay
Lw(45);//hien dau -
hthi(8,thg); //dang o thang
LCDp("       ");
}}}


void main( void )
{
 // Stop watchdog timer to prevent time out reset
 WDTCTL = WDTPW + WDTHOLD;
 BCSCTL1 = CALBC1_1MHZ; //dinh nghia dao dong noi 1MHz
 DCOCTL = CALDCO_1MHZ;
 TACCTL0=CCIE; // Cho phép ngat Timer_A
 TACCR0 = 9999; // Cho chu ki là 10000
 TACTL = MC_1|ID_0 |TASSEL_2|TACLR; //dem len, chia 1, xung dd noi, cho phep reset timer
 __enable_interrupt ();
 P2DIR = 0xFF;
 P2SEL&=~(BIT6+BIT7);      //set bit 2.6 2.7 la IO
 P1DIR =(BIT0+BIT1+BIT2);  //1.0, 1.1, 1.2 la out
 P1DIR &=~ (BIT3+BIT4+BIT5);  //1.3, 1.4, 1.5 la in

 P1IE |=BIT3+BIT4+BIT5;     //Cho phép ng?t ? P1.3
 P1IES |=BIT3+BIT4+BIT5;   //Ng?t c?nh xu?ng,
 P1IFG &=~(BIT3+BIT4+BIT5);           //Xóa c? ng?t
 __bis_SR_register(GIE);
LCDinit();  //thiet lap che do hoat dong cho LCD
y_20=20;
set=0;
Down=1;

while(1)
{
if(set==0) { R_DS(); min1=min; day1=day; mth1=mth;}  //doc thong tin tu DS1307
else { sec = DS_r(SEC); }
if(on==1)  //da hen gio
pc();      //ghi bieu tuong dong ho va )) len CGRAM cua DS1307
hienthi();  


////////////////////
if(on==1||ona==1)
     {if(((min==ph)&&(h24==gio)&&(ona==0)) || ((min==ph)&&(h24==gio)&&(day==ngy)&&(mth==thg))) //neu toi gio hen
          
     {Down=0;}
     if((min==(ph1+last1))&&(h24==gio1)) //neu toi gio hen
          
     {Down=1;}
     }
}
}
#pragma vector=PORT1_VECTOR
__interrupt void mode(void)
{
  if(P1IFG&BIT3) //nut MODE
     {set++;
     if(set==9) set=2;
      P1IFG &=~BIT3;
     }
  if(P1IFG&BIT5) //nut Ok
    {
       set=0;
       chu=0;
       if(sec>59) sec=0;
       W_DS(); //c.trinh ghi thong tin ngay, gio,...len DS1307
       LCDct(0x01);
       P1IFG &=~BIT5;
    }
  if(P1IFG&BIT4) //nut UP
     {
       if (set==1) {chu++; if(chu>1) chu=0;}
       if (chu==1) //che do hen gio
         {
             if(set==2) 
                    {
                     on=on++; if(on>1) on=0; last=0;
                     if(on==0) {ph=min+1; gio=h24; ngy=dt; thg=mth;}
                    }
             if(set==3) ph=ph++;
             if(set==4) gio=gio++;
             if(set==5) last=last++;
             if(set==6) {ona=ona++; if(ona>1) ona=0;}
             if(set==7) ngy=ngy++;
             if(set==8) thg=thg++;
         
         }
       if (chu==0)  //che do chinh gio
       {
             if(set==2) {a_p=a_p++; if(a_p>1)a_p=0;}
             if(set==3) {min=min++; min1=min; DS_W(MIN,min1);}
             if(set==4)  h24=h24++;
             if(set==5) {day=day+1; day1=day; DS_W(DAY,day1);}
             if(set==6)  dt=dt+1;
             if(set==7) {mth=mth+1; mth1=mth; DS_W(MTH,mth1);}
             if(set==8)  y=y++;
       
       }
       
       Set_Clock(); //hieu chinh lai ngay gio
     
       
       P1IFG &=~BIT4;
     }
  
}