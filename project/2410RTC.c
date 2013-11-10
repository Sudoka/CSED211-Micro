//====================================================================
// File Name : 2410RTC.c
// Function  : S3C2410 RTC Test Program
// Program   : Shin, On Pil (SOP)
// Date      : May 06, 2002
// Version   : 0.0
// History
//   0.0 : Programming start (March 11, 2002) -> SOP
//====================================================================

#include "2410addr.h"
#include "2410lib.h"
#include "2410RTC.h"

char *day[8] = {" ","Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
volatile int isRtcInt, isInit = 2;              //April 01, 2002 SOP
volatile unsigned int sec_tick;

//=======================================
//Period = (n + 1) / 128 second
//   n : Tick time count value (1~127)
//=======================================
//********************[ Display_Rtc ]*********************************
void Display_Rtc(void)
{
//    int year,tmp;
    int year,tmp,key;                   
    int month,date,weekday,hour,min,sec;

    Uart_Printf("[ Display RTC Test ]\n");
    Uart_Printf("0. RTC Initialize     1. RTC Time Setting    2. Only RTC Display\n\n");
    Uart_Printf("Selet : ");
    key = Uart_GetIntNum();
    Uart_Printf("\n\n");
    
    isInit = key;

    if(isInit == 0)
    {
        Rtc_Init();
        isInit = 2;
    }
    
    else if(isInit == 1)
    {
        Rtc_TimeSet();
        isInit = 2;        
    }

    rRTCCON = 0x01;    //No reset, Merge BCD counters, 1/32768, RTC Control enable
//  Uart_Printf("This test should be excuted once RTC test(Alarm) for RTC initialization\n");
    Uart_Printf("Press any key to exit.\n\n");
    while(!Uart_GetKey())
    {    
        while(1)
        {
            if(rBCDYEAR==0x99) 
                year = 0x1999;
            else 
                year    = 0x2000 + rBCDYEAR;
            month   = rBCDMON;
            weekday = rBCDDAY;
            date    = rBCDDATE;
            hour    = rBCDHOUR;
            min     = rBCDMIN;
            sec     = rBCDSEC;
            
            if(sec!=tmp)                //Same time is not display
            {
                tmp = sec;
                break;
            }         
        }
        Uart_Printf("%2x : %2x : %2x  %10s,  %2x/%2x/%4x\n",hour,min,sec,day[weekday],month,date,year);
//        Uart_Printf("%2x : %2x : %2x  %9s,  %2x/%2x/%4x     rBCDDATE = %x, weekday= %x\n",hour,min,sec,day[weekday],month,date,year,rBCDDATE,weekday);        
    }
    rRTCCON = 0x0; //No reset, Merge BCD counters, 1/32768, RTC Control disable(for power consumption)
}

//***********************[ RndRst_Rtc ]*********************************
//Round boundary 30, 40 or 50 Sec
//For example, when the current time is 23:37:47 and the round baundary is selected to 40 sec,
//the round reset changes the current time is 23:38:00.
void RndRst_Rtc(void)
{
    int year;
    int month,date,weekday,hour,min,sec,tmp;
    unsigned int save_GPFCON;

    save_GPFCON = rGPFCON;

    rEXTINT0 = 0x2;  //Falling edge triggered
    rGPFCON  = 0x2;  //EINT0

    pISR_EINT0 = (unsigned int)EINT0_int;
    rINTMSK    = ~(BIT_EINT0);

    if(isInit==0)
    {
        Rtc_Init();
        isInit = 1;
    }

    rRTCCON = 0x01;    //No reset, Merge BCD counters, 1/32768, RTC Control enable
    Uart_Printf("Press any key to exit.\n\n");
    Uart_Printf("Press EINT0 key to test round reset.\n");
    
    while(!Uart_GetKey())
    {
        while(1)
        {
            if(rBCDYEAR == 0x99) 
                year = 0x1999;
            else 
                year    = 0x2000 + rBCDYEAR;
            month   = rBCDMON;
            weekday = rBCDDAY;
            date    = rBCDDATE;
            hour    = rBCDHOUR;
            min     = rBCDMIN;
            sec     = rBCDSEC;
            
            if(sec!=tmp)
            {
                tmp = sec;
                break;
            }
        }      
        Uart_Printf("%2x : %2x : %2x  %10s,  %2x/%2x/%4x\n",hour,min,sec,day[weekday],month,date,year);
    }
    rRTCCON = 0x0;   //No reset, Merge BCD counters, 1/32768, RTC Control disable
    rGPFCON = save_GPFCON;
}

//************************[ Test_Rtc_Alarm ]******************************
void Test_Rtc_Alarm(void)  
{
    Uart_Printf("[ RTC Alarm Test for S3C2410 ]\n");

    Rtc_Init();

    rRTCCON  = 0x01;                    //No reset, Merge BCD counters, 1/32768, RTC Control enable
    rALMYEAR = TESTYEAR2 ;
    rALMMON  = TESTMONTH2;
    rALMDATE = TESTDATE2  ;
    rALMHOUR = TESTHOUR2 ;
    rALMMIN  = TESTMIN2  ;
    rALMSEC  = TESTSEC2 + 9; 
    isRtcInt = 0;
    pISR_RTC = (unsigned int)Rtc_Int;
    rRTCALM  = 0x7f;                    //Global,Year,Month,Day,Hour,Minute,Second alarm enable
    rRTCCON  = 0x0;                     //No reset, Merge BCD counters, 1/32768, RTC Control disable
    rINTMSK  = ~(BIT_RTC);

    while(isRtcInt==0);
    
    rINTMSK = BIT_ALLMSK;
//  rRTCCON = 0x0;  //RTC  Control disable(for power consumption), 1/32768, Normal(merge), No reset
}

//************************[ Rtc_Init ]*********************************
void Rtc_Init(void)
{
    rRTCCON  = rRTCCON  & ~(0xf)  | 0x1;            //No reset, Merge BCD counters, 1/32768, RTC Control enable
    
    rBCDYEAR = rBCDYEAR & ~(0xff) | TESTYEAR;
    rBCDMON  = rBCDMON  & ~(0x1f) | TESTMONTH;
    rBCDDATE = rBCDDATE & ~(0x3f) | TESTDATE;         
    rBCDDAY  = rBCDDAY  & ~(0x7)  | TESTDAY;       //SUN:1 MON:2 TUE:3 WED:4 THU:5 FRI:6 SAT:7
    rBCDHOUR = rBCDHOUR & ~(0x3f) | TESTHOUR;
    rBCDMIN  = rBCDMIN  & ~(0x7f) | TESTMIN;
    rBCDSEC  = rBCDSEC  & ~(0x7f) | TESTSEC;
    
    rRTCCON  = 0x0;             //No reset, Merge BCD counters, 1/32768, RTC Control disable    
}

//************************[ Rtc_TimeSet ]*********************************
void Rtc_TimeSet(void)
{
    int syear,smonth,sdate,shour,smin,ssec;
    int sday;

    Uart_Printf("[ RTC Time Setting ]\n");
    Rtc_Init();         //RTC Initialize
    Uart_Printf("RTC Time Initialized ...\n");
    
    Uart_Printf("Year (Two digit the latest)[0x??] : ");
    syear = Uart_GetIntNum();
    
    Uart_Printf("Month                      [0x??] : ");
    smonth = Uart_GetIntNum();
    
    Uart_Printf("Date                       [0x??] : ");
    sdate = Uart_GetIntNum();
          
    Uart_Printf("\n1:Sunday  2:Monday  3:Thesday  4:Wednesday  5:Thursday  6:Friday  7:Saturday\n");
    Uart_Printf("Day of the week                   : ");
    sday = Uart_GetIntNum();
    
    Uart_Printf("Hour                       [0x??] : ");
    shour = Uart_GetIntNum();
    
    Uart_Printf("Minute                     [0x??] : ");
    smin = Uart_GetIntNum();
    
    Uart_Printf("Second                     [0x??] : ");
    ssec = Uart_GetIntNum();
    
    rRTCCON  = rRTCCON  & ~(0xf)  | 0x1;            //No reset, Merge BCD counters, 1/32768, RTC Control enable
    
    rBCDYEAR = rBCDYEAR & ~(0xff) | syear;
    rBCDMON  = rBCDMON  & ~(0x1f) | smonth;
    rBCDDAY  = rBCDDAY  & ~(0x7)  | sday;           //SUN:1 MON:2 TUE:3 WED:4 THU:5 FRI:6 SAT:7
    rBCDDATE = rBCDDATE & ~(0x3f) | sdate;
    rBCDHOUR = rBCDHOUR & ~(0x3f) | shour;
    rBCDMIN  = rBCDMIN  & ~(0x7f) | smin;
    rBCDSEC  = rBCDSEC  & ~(0x7f) | ssec;
    
    rRTCCON  = 0x0;    //No reset, Merge BCD counters, 1/32768, RTC Control disable    
}

//************************[ Test_Rtc_Tick ]****************************
void Test_Rtc_Tick(void)
{
    Uart_Printf("[ RTC Tick interrupt(1 sec) test for S3C2410 ]\n");
    Uart_Printf("Press any key to exit.\n"); 
    Uart_Printf("\n");  
    Uart_Printf("\n");      
    Uart_Printf("   ");    
        
    pISR_TICK = (unsigned)Rtc_Tick;
    sec_tick  = 1;
    rINTMSK   = ~(BIT_TICK); 
    rRTCCON   = 0x0;            //No reset[3], Merge BCD counters[2], BCD clock select XTAL[1], RTC Control disable[0]
    rTICNT    = (1<<7) + 127;   //Tick time interrupt enable, Tick time count value 127
                                //Period = (n + 1) / 128 second    n:Tick time count value(1~127)
    Uart_Getch();
    
    rINTMSK   = BIT_ALLMSK;
    rRTCCON   = 0x0;            //No reset[3], Merge BCD counters[2], BCD clock select XTAL[1], RTC Control disable[0]
}

//-----------------------------------------------------------------------
void __irq EINT0_int(void)
{
    rSRCPND = BIT_EINT0;   
    rINTPND = BIT_EINT0;
    rINTPND;

    rRTCRST = (1<<3) | 3;   //Round second reset enable, over than 30 sec
}

//-----------------------------------------------------------------------
void __irq Rtc_Int(void)
{
    rSRCPND = BIT_RTC;   
    rINTPND = BIT_RTC;
    rINTPND;

    Uart_Printf("RTC Alarm Interrupt O.K.\n");
    isRtcInt = 1;  
}

//---------------------------------------------------------------------
void __irq Rtc_Tick(void)
{
    rSRCPND = BIT_TICK;   
    rINTPND = BIT_TICK;
    rINTPND;

    Uart_Printf("\b\b\b\b\b\b\b%03d sec",sec_tick++);
}