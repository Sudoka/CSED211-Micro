/*************************************************
  NAME: PowerOff.c
  DESC: Power Off mode test
  HISTORY:
  03.21.2002:purnnamu: first release for S3C2410X
  05.10.2002:SOP RTC Name Change (DAY,DATE)
  08.24.2002:purnnamu
    LCDVFn is connected the analog circuit in LCD. 
    So, this signal should be output L.
  01.27.2003:SOP Reconfiguration GPG10 Output -> Input 
             Power-Off mode is fixed SD Card increment current (4mA -> 35mA)
  02.25.2003:SOP Reconfiguration GPF3/GPG0,1,4,5,10 Pull-Up Disable -> Enable 
  05.06.2003:SOP 
  06.13.2003:SOP           
 *************************************************/
 
#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h" 

#include "lcdlib.h"
#include "PowerOff.h"

#define TESTYEAR 	(0x00)
#define TESTMONTH 	(0x12)
#define TESTDATE    (0x31)
#define TESTDAY     (0x06)  // SUN:1 MON:2 TUE:3 WED:4 THU:5 FRI:6 SAT:7   
#define TESTHOUR	(0x23)
#define TESTMIN		(0x59)
#define TESTSEC		(0x59)

#define TESTYEAR2	(0x01)
#define TESTMONTH2 	(0x01)
#define TESTDATE2	(0x01)
#define TESTDAY2	(0x07)  // SUN:1 MON:2 TUE:3 WED:4 THU:5 FRI:6 SAT:7   
#define TESTHOUR2	(0x00)
#define TESTMIN2	(0x00)
#define TESTSEC2	(0x00)


void SelectEintType(void);


//================================================================================
static void __irq Eint0Int(void)
{
    ClearPending(BIT_EINT0);
    Uart_Printf("[WU]EINT0 interrupt is occurred for STOP/POWER_OFF mode wake-up.\n");
}


//================================================================================
static void __irq Eint8_23Int(void)
{
    rEINTPEND=(1<<11); //EINT11 sub-pending bit.	
    ClearPending(BIT_EINT8_23);
    Uart_Printf("[WU]EINT8_23 interrupt is occurred for STOP/POWER_OFF mode wake-up.\n");
}


//================================================================================
static void __irq Eint8_23Int_100Hz(void)
{
    rEINTPEND=(1<<11); //EINT11 sub-pending bit.	
    ClearPending(BIT_EINT8_23);
    Uart_Printf("wui\n");
}


//================================================================================
static void __irq AlarmInt(void)
{
    ClearPending(BIT_RTC);
    Uart_Printf("[WU]ALARM interrupt is occurred for STOP/POWER_OFF mode wake-up.\n");
}



#define CHECK_SDRAM_SELFREFRESH		(1)
#define EXTERNAL_BUS_HOLDER		(1)  
    //The SMDK2410X board has externa databus buffer with bus hold function.
#define CHOOSE_EINT_TYPE		(1)
#define GPIO_RESTORE			(0)
#define ALARM_WAKEUP			(1)


//================================================================================
void Test_InitSDRAM(void)
//Initialize SDRAM for SDRAM self-refresh test.
{
    int i;
    
    Uart_Printf("[SDRAM Initialization]\n");
    Uart_Printf("Fill SDRAM for self-refresh test.\n");
    for(i=_NONCACHE_STARTADDRESS;i<(_NONCACHE_STARTADDRESS+0x400000);i+=4)
    	*((U32 *)i)=i^0x55555555;
    Uart_Printf("Filling SDRAM is completed.\n");

    *((volatile U32 *)0x33e00000)=0x0;  //for Initialize the counter in Check_PowerOffWakeUp_100Hz(). 
}


//================================================================================
void SelectEintType(void)
{

    int extintMode;

    Uart_Printf("1.L-level  2.H-level  3.F-edge  4.R-edge  5.B-edge\n");
    Uart_Printf("Select the EINT0 trigger type:");    


    extintMode=Uart_Getch();

    rGPFCON=rGPFCON & ~(3<<0)|(2<<0); //PF0=EINT0
    
    switch(extintMode)
    {
    case '1':
    	rEXTINT0=rEXTINT0&~(7<<0)|(0x0<<0); //EINT0=low level triggered
    	Uart_Printf("L-level\n");
    	break;
    case '2':
    	rEXTINT0=rEXTINT0&~(7<<0)|(0x1<<0); //EINT0=high level triggered
    	Uart_Printf("H-level\n");
    	break;
    case '3':
    	rEXTINT0=rEXTINT0&~(7<<0)|(0x2<<0);  //EINT0=falling edge triggered
    	Uart_Printf("F-edge\n");
    	break;
    case '4':
    	rEXTINT0=rEXTINT0&~(7<<0)|(0x4<<0); //EINT0=rising edge triggered
    	Uart_Printf("R-edge\n");
     	break;
    case '5':
    	rEXTINT0=rEXTINT0&~(7<<0)|(0x6<<0); //EINT0=both edge triggered
    	Uart_Printf("B-edge\n");
    	break;
    default:
    	break;
    }
    
}


//================================================================================
void ConfigPowerOffGPIO(void)
{

    // Check point
    // 1) NC pin: input pull-up on 
    // 2) If input is driver externally: input pull-up off
    // 3) If a connected component draws some current: output low.
    // 4) If a connected component draws no current: output high.
    
    //chip # = 5

    //CAUTION:Follow the configuration order for setting the ports. 
    // 1) setting value(GPnDAT) 
    // 2) setting control register  (GPnCON)
    // 3) configure pull-up resistor(GPnUP)  

    //32bit data bus configuration  
    //*** PORT A GROUP
    //Ports  : GPA22 GPA21  GPA20 GPA19 GPA18 GPA17 GPA16 GPA15 GPA14 GPA13 GPA12  
    //Signal : nFCE nRSTOUT nFRE  nFWE  ALE   CLE   nGCS5 nGCS4 nGCS3 nGCS2 nGCS1 
    //Binary : 1     1      1,    1     1     1     1,    1     1     1     1,
    //-------------------------------------------------------------------------------------------
    //Ports  : GPA11  GPA10  GPA9   GPA8   GPA7   GPA6   GPA5   GPA4   GPA3   GPA2   GPA1   GPA0
    //Signal : ADDR26 ADDR25 ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0 
    //Binary : 1      1      1      1,     1      1      1      1,     1      1      1      1         
    rGPACON = 0x7fffff; 

    //**** PORT B GROUP
    //Ports  : GPB10   GPB9    GPB8    GPB7    GPB6    GPB5     GPB4    GPB3   GPB2   GPB1       GPB0
    //Signal : nXDREQ0 nXDACK0 nXDREQ1 nXDACK1 nSS_KBD nDIS_OFF L3CLOCK L3DATA L3MODE nIrDATXDEN Keyboard
    //Setting: INPUT   OUTPUT  INPUT   OUTPUT  INPUT   OUT      OUT     OUT    OUT    INPUT      INPUT 
    //Binary : 00,     01      00,     01      00,     01       01,     01     01,    00         00  
    //PU_OFF :  0       1       0,      1      1(ext)  1(*)     1,      1      1      1(ext)     1(ext)           
    //*:nDIS_OFF:4.7K external pull-down resistor                                 
    rGPBDAT=  0x0|(1<<9)|(1<<7)|(0<<5)|(1<<4)|(1<<3)|(1<<2);
    rGPBCON = 0x044550;  
    rGPBUP  = 0x2ff;   //0x2fd->2ff, 3uA is reduced. Why? 

    //PORT C GROUP
    //Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8 GPC7  GPC6   GPC5   GPC4 GPC3 GPC2  GPC1 GPC0
    //Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND  
    //Setting: IN    IN    IN    IN    IN    IN    IN   IN   OUT   OUT    OUT    IN   IN   IN    IN   IN
    //Binary : 00    00,   00    00,   00    00,   00   00,  01    01,    01     00,  00   00,   00   00
    //PU_OFF :  0     0     0     0,    0     0     0    0,   1     1      1      0,   0    0     0    0
    rGPCDAT = 0x0;
    rGPCCON = 0x00005400;  //0x00000000;	
    rGPCUP  = 0x00e0;      //0x0000;     
    //LCDVFn is connected the analog circuit in LCD. So, this signal should be output L.
    
/*    
    //PORT C GROUP
    //Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8 GPC7  GPC6   GPC5   GPC4 GPC3 GPC2  GPC1 GPC0
    //Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND  
    //Setting: IN    IN    IN    IN    IN    IN    IN   IN   OUT   OUT    OUT    IN   IN   IN    IN   IN
    //Binary : 00    00,   00    00,   00    00,   00   00,  01    01,    01     00,  00   00,   00   00
    //PU_OFF :  0     0     0     0,    0     0     0    0,   1     1      1      0,   0    0     0    0
    rGPCDAT = 0x0|(1<<7)|(1<<6)|(1<<5);
    rGPCCON = 0x00005400;  //0x00000000;	
    rGPCUP  = 0x00e0;      //0x0000;     
    //LCDVFn is connected the analog circuit in LCD. So, this signal should be output L.    
*/    
    
    //*** PORT D GROUP
    //Ports  : GPD15 GPD14 GPD13 GPD12 GPD11 GPD10 GPD9 GPD8 GPD7 GPD6 GPD5 GPD4 GPD3 GPD2 GPD1 GPD0
    //Signal : VD23  VD22  VD21  VD20  VD19  VD18  VD17 VD16 VD15 VD14 VD13 VD12 VD11 VD10 VD9  VD8
    //Setting: IN    IN    IN    IN    IN    IN    IN   IN   IN   IN   IN   IN   IN   IN   IN   IN
    //Binary : 00    00,   00    00,   00    00,   00   00,  00   00,  00   00,  00   00,  00   00
    //PU_OFF :  0     0     0     0,    0     0     0    0,   0    0    0    0,   0    0    0    0
    rGPDDAT=  0x0;
    rGPDCON = 0x0;	
    rGPDUP  = 0x0;    

    //*** PORT E GROUP
    //Ports  : GPE15  GPE14  GPE13   GPE12    GPE11    GPE10   GPE9    GPE8    GPE7    GPE6  GPE5  GPE4  
    //Signal : IICSDA IICSCL SPICLK0 SPIMOSI0 SPIMISO0 SDDATA3 SDDATA2 SDDATA1 SDDATA0 SDCMD SDCLK I2SSDO 
    //Setting: IN     IN     IN      IN       IN       IN      IN      IN      IN      IN    IN    OUT
    //Binary : 00     00,    00      00,      00       00,     00      00,     00      00,   00    01,     
    //PU_OFF :  1-ext  1-ext  0       0,       0        0       0       0,      0       0     0     1,
    //------------------------------------------------------------------------------------------------
    //Ports  : GPE3   GPE2  GPE1    GPE0    
    //Signal : I2SSDI CDCLK I2SSCLK I2SLRCK     
    //Setting: IN     OUT   OUT     OUT
    //Binary : 00     01,   01      01
    //PU_OFF :  1-ext  1     1       1
    rGPEDAT = 0x0|(1<<4)|(1<<2)|(1<<1)|(1<<0);
    rGPECON = 0x00000115;	
    rGPEUP  = 0xc01f;     

    //*** PORT F GROUP
    //Ports  : GPF7   GPF6   GPF5   GPF4   GPF3        GPF2  GPF1   GPF0
    //Signal : nLED_8 nLED_4 nLED_2 nLED_1 nIRQ_PCMCIA EINT2 KBDINT EINT0
    //Setting: Output Output Output Output IN          IN    IN     EINT0
    //Binary : 01     01,    01     01,    00          00,   00     10
    //PU_OFF :  1      1      1      1,     0-ext       1-ext 1-ext  1-ext
    rGPFDAT = 0x0  |(0xf<<4);
    rGPFCON = 0x5502;
    rGPFUP  = 0xf7;   //nIRQ_PCMCIA=Hi-Z Before initialize     
                            
    //PORT G GROUP
    //Ports  : GPG15 GPG14 GPG13 GPG12 GPG11  GPG10    GPG9     GPG8     GPG7      GPG6    
    //Signal : nYPON YMON  nXPON XMON  EINT19 DMAMODE1 DMAMODE0 DMASTART KBDSPICLK KBDSPIMOSI
    //Setting: OUT   OUT   OUT   OUT   OUT    IN       OUT      OUT      OUT       OUT
    //Binary : 01    01,   01    01,   01-dbg 00,      01       01,      01        01
    //PU_OFF :  1     1     1     1,    1-ext  0        1        1,       1         1
    //---------------------------------------------------------------------------------------
    //Ports  : GPG5       GPG4      GPG3   GPG2    GPG1    GPG0    
    //Signal : KBDSPIMISO LCD_PWREN EINT11 nSS_SPI IRQ_LAN IRQ_PCMCIA
    //Setting: IN         IN        EINT11 IN      IN      IN
    //Binary : 00         00,       10     00,     00      00
    //PU_OFF :  0-ext      0,        1-ext  0       0       0
    rGPGDAT = 0x0 |(1<<11)|(1<<15)|(1<<14)|(1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<7)|(1<<6) ;
    rGPGCON = 0x55455080;   //GPG11=OUT  //for debug
    rGPGUP  = 0xfbc8;    //IRQ_LAN=Hi-Z, IRQ_PCMCIA=Hi-Z, KBDSPIMISO=Hi-Z before initialaize

    //*** PORT H GROUP
    //Ports  : GPH10   GPH9    GPH8 GPH7  GPH6  GPH5 GPH4 GPH3 GPH2 GPH1  GPH0 
    //Signal : CLKOUT1 CLKOUT0 UCLK nCTS1 nRTS1 RXD1 TXD1 RXD0 TXD0 nRTS0 nCTS0
    //Setting: IN      IN      IN   IN    IN    IN   OUT  RXD0 TXD0 OUT   IN
    //Binary : 00,     00      00,  00    00,   00   01,  10   10,  01    00
    //PU_OFF :  0       0       0,   1-ext 1-ext 1-ext 1, 1-ext 1    1     1-ext

#if 1
    rGPHDAT = 0x0|(1<<6)|(1<<1)|(1<<4);    
    rGPHCON = 0x0001a4; 		   //0x0011a4->0x0001a4 reduces 12uA why -> MAX3232C may sink 12uA.
#else
    //rGPHDAT = 0x0|(1<<6)|(1<<1)|(1<<4);  
    //rGPHCON = 0x0011a4; 
    rGPHDAT = 0x0|(0<<6)|(1<<1)|(1<<4);  //(1<<6)->(0<<6) reduces 12uA (MAX3232C may sink 12uA.)
    rGPHCON = 0x0011a4; 
#endif    
    rGPHUP  = 0x0ff;    // The pull up function is disabled GPH[10:0]

    
    //External interrupt will be falling edge triggered. 
    rEXTINT0 = 0x22222222;    // EINT[7:0]
    rEXTINT1 = 0x22222222;    // EINT[15:8]
    rEXTINT2 = 0x22222222;    // EINT[23:16]
}


//================================================================================
void SetAlarmWakeUp(void)
{
    rRTCCON = 0x1;	// R/W enable, 1/32768, Normal(merge), No reset

    rBCDYEAR = TESTYEAR;
    rBCDMON  = TESTMONTH;
    rBCDDAY  = TESTDAY;	// SUN:1 MON:2 TUE:3 WED:4 THU:5 FRI:6 SAT:7
    rBCDDATE = TESTDATE;
    rBCDHOUR = TESTHOUR;
    rBCDMIN  = TESTMIN;
    rBCDSEC  = TESTSEC;
    
    rALMYEAR=TESTYEAR2 ;
    rALMMON =TESTMONTH2;
    rALMDATE=TESTDATE2  ;
    rALMHOUR=TESTHOUR2 ;
    rALMMIN =TESTMIN2  ;
    rALMSEC =TESTSEC2+9;

    rRTCALM=0x7f;
    
    rRTCCON=0x0;   // R/W disable, 1/32768, Normal(merge), No reset
}

// S3C2400  - S3C2410
//  GPA11  -> GPG3(EINT11)
//  GPE3   <- GPG11(EINT19)

//================================================================================
void Check_PowerOffWakeUp(void)
{
    Uart_Printf("[Power off related registers]\n");
    Uart_Printf("GSTATUS2: PWRST=%d OFFRST=%d WDTRST=%d\n", 
    			(rGSTATUS2&(1<<0))!=0,
    			(rGSTATUS2&(1<<1))!=0,
    			(rGSTATUS2&(1<<2))!=0);
    Uart_Printf("GSTATUS3:%x(0x0,0x%x), GSTATUS4=%x(0x0,0xaaaaaaaa)\n",
    	rGSTATUS3,(U32)StartPointAfterPowerOffWakeUp,rGSTATUS4);
    if(rGSTATUS2&(1<<0))
    {
    	Uart_Printf("Power On Reset\n\n");
    	rGSTATUS2=(1<<0);
    	return;
    }
    if(rGSTATUS2&(1<<1))
    {
    	Uart_Printf("POWER OFF Wake-up\n\n");
    	rGSTATUS2=(1<<1);
    	Uart_Printf("EINTPEND=0x%x\n",rEINTPEND);
    	Uart_Printf("SRCPND=0x%x\n",rSRCPND);

    	Test_PowerOffMode();
    }
    if(rGSTATUS2&(1<<2))
    {
    	Uart_Printf("Watchdog Reset\n\n");
    	rGSTATUS2=(1<<2);
    	return;
    }
    
}


//================================================================================
void Test_PowerOffMode(void)
{
   
    int i;
    U32 portStatus[33];
    int error=0;
    int mode;
    Uart_Printf("[POWER OFF Mode Test]\n");

    Uart_Printf("nBATT_FLT pin should be tested.\n");
    
#if ALARM_WAKEUP
    Uart_Printf("S3C2410 will wake up by RTC alarm(10 sec) or EINT0 .\n");
#else
    Uart_Printf("S3C2410 will wake up by EINT0 .\n");
#endif

#if CHECK_SDRAM_SELFREFRESH	
    Uart_Printf("Check SDRAM for self-refresh test\n");
    for(i=_NONCACHE_STARTADDRESS;i<(_NONCACHE_STARTADDRESS+0x400000);i+=4)
    {
    	if(*((U32 *)i)!=(i^0x55555555))
    	{
    	    Uart_Printf("Mem Error:%x=%x(%x)\n",i,*((U32 *)i),i^0x55555555);
    	    error++;
    	}
    	if(error>20)break;
    }
    if(error)
    	Uart_Printf("SDRAM self-refresh test:FAILED\n"); 
    else 
    	Uart_Printf("SDRAM self-refresh test:O.K.\n");	
#endif

    Uart_TxEmpty(0);	//Wait until UART0 Tx buffer empty.
       
    // Saving the port configurations is meaningless.
    //for(i=0;i<33;i++)
    //	portStatus[i]=*( (volatile U32 *)0x56000000 + i);
    ConfigPowerOffGPIO();

#if CHOOSE_EINT_TYPE      
    SelectEintType();
#else
    rGPFCON=rGPFCON & ~(3<<0)|(2<<0); //PF0=EINT0
    rEXTINT0=rEXTINT0&(7<<0)|(0x2<<0);  //EINT0=falling edge triggered
#endif

#if 1	//test whether or not the interrupt pending bit retain the wake-up source.
    pISR_EINT0=(U32)Eint0Int;
    pISR_EINT8_23=(U32)Eint8_23Int;
    pISR_RTC=(U32)AlarmInt;

    rINTMSK = ~(BIT_EINT0|BIT_EINT8_23|BIT_RTC);
    for(i=0;i<100;i++);

    rSRCPND = BIT_EINT0|BIT_EINT8_23|BIT_RTC;
    rINTPND = BIT_EINT0|BIT_EINT8_23|BIT_RTC;

    rEINTPEND=rEINTPEND;
    rSRCPND=rSRCPND;
    rINTPND=rINTPND;

#endif     

    rEINTMASK = rEINTMASK&~(1<<11); //SRCPND:EINT8_23 will be set by EINT11 after wake-up.
    rINTMSK = BIT_ALLMSK;
        // NOTE: Any interrupt can't be used in STOP mode 
        //       because SDRAM is in self-refresh mode and ISR code will access SDRAM.

#if ALARM_WAKEUP    
    SetAlarmWakeUp();
#endif

    rRTCCON=0x0;   // R/W disable, 1/32768, Normal(merge), No reset
    rADCCON|=(1<<2);

    rMISCCR|=(1<<12); //USB port0 = suspend
    rMISCCR|=(1<<13); //USB port1 = suspend

    rMISCCR|=(1<<2); //Previous state at STOP(?) mode (???)
    
    rGSTATUS3=(U32)StartPointAfterPowerOffWakeUp;
    rGSTATUS4=0xaaaaaaaa;

#if EXTERNAL_BUS_HOLDER
    //D[31:0] pull-up off. The data bus will not be float by the external bus holder.
    //If the pull-up resitsers are turned on,
    //there will be the leakage current through the pull-up resister
    rMISCCR=rMISCCR|(3<<0); 
#else
    //D[31:0] pull-up on. The data bus will not be float by pull-up resister. 
    rMISCCR=rMISCCR&~(3<<0); 
#endif

    Lcd_EnvidOnOff(0); //Before entering STOP mode, LCD must be off    

    Uart_Printf("\nNow, I am entering POWER_OFF mode.\n");
    Uart_TxEmpty(0);	//Wait until UART0 Tx buffer empty.


    //=================================================================
    //  VERY IMPORTANT NOTE 
    //  To enter STOP/SLIDLE mode, MMU TLB-fill operation should be prohibited
    //   because MTT table is in SDRAM and SDRAM is in the self-refresh mode.
    //  So, we will fill TLB before entering SDRAM self-refresh 
    //   instead of disabling MMU.
    rREFRESH;//To fill TLB for the special register used in EnterPWDN
    rCLKCON;
    //=================================================================

    EnterPWDN(0x7fff8); //POWER_OFF mode
    //Never return here. 
}


//================================================================================
static void __irq BattFaultInt(void)
{
    ClearPending(BIT_BAT_FLT);
    Uart_Printf("nBATT_FAULT interrupt is occurred.\n");
}

//================================================================================
void Test_BattFaultInterrupt(void)
{
    Uart_Printf("[nBATT_FAULT interrupt Test]\n");
    Uart_Printf("Push any key to exit!\n");
    pISR_BAT_FLT=(unsigned)BattFaultInt;
    rINTMSK=rINTMSK&~(BIT_BAT_FLT);
    while(!Uart_GetKey());
    
}


/************************ POWER OFF Mode by 100hz test ************************/
//================================================================================
void Check_PowerOffWakeUp_100Hz(void)
{
/*
    Uart_Printf("GSTATUS2: PWRST=%d OFFRST=%d WDTRST=%d\n", 
    			(rGSTATUS2&(1<<0))!=0,
    			(rGSTATUS2&(1<<1))!=0,
    			(rGSTATUS2&(1<<2))!=0);

    Uart_Printf("GSTATUS3:%x(0x0,0x%x), GSTATUS4=%x(0x0,0xaaaaaaaa)\n",
    	rGSTATUS3,(U32)StartPointAfterPowerOffWakeUp,rGSTATUS4);
*/    			
    
    if(rGSTATUS2&(1<<0))
    {
    	Uart_Printf("Power On Reset\n\n");
    	rGSTATUS2=(1<<0);
    	return;
    }
    if(rGSTATUS2&(1<<1))
    {
        if( *((volatile U32 *)0x33e00000) != 0x12345678 )
        {
            *((volatile U32 *)0x33e00000)=0x12345678;
            *((volatile U32 *)0x33e00004)=0x0;
        }
        (*(volatile U32 *)0x33e00004)++;
        Uart_Printf("wu PO:%d\n",*((volatile U32 *)0x33e00004));
    	
    	if(rGSTATUS4!=0xaaaaaaaa)
    	{
    	    Uart_Printf("GSTATUS4 register error rGSTATUS4=%x\n",rGSTATUS4);
    	    while(1);
    	}
    	rGSTATUS2=(1<<1);
    	Test_PowerOffMode_100Hz();
    }
    if(rGSTATUS2&(1<<2))
    {
    	Uart_Printf("Watchdog Reset\n\n");
    	rGSTATUS2=(1<<2);
    	return;
    }
    
}

//================================================================================
void Test_PowerOffMode_100Hz(void)
{
   
    int i;
    U32 portStatus[33];
    int error=0;
    int mode;
    Uart_Printf("[Power Off Mode 100Hz]\n");

    if(Uart_GetKey()!=0)
    {
    for(i=_NONCACHE_STARTADDRESS;i<(_NONCACHE_STARTADDRESS+0x400000);i+=4)
    {
    	if(*((U32 *)i)!=(i^0x55555555))
    	{
    	    Uart_Printf("Mem Error:%x=%x(%x)\n",i,*((U32 *)i),i^0x55555555);
    	    error++;
    	}
    	if(error>20)break;
    }
    if(error)
    	Uart_Printf("SDRAM self-refresh test:FAILED\n"); 
    else 
    	Uart_Printf("SDRAM self-refresh test:O.K.\n");	
    }

    Uart_TxEmpty(0);	//Wait until UART0 Tx buffer empty.
       
    // Saving the port configurations is meaningless.
    //for(i=0;i<33;i++)
    //	portStatus[i]=*( (volatile U32 *)0x56000000 + i);
    ConfigPowerOffGPIO();

#if 0
    SelectEintType();
#else
    rGPFCON=rGPFCON & ~(3<<0)|(2<<0); //PF0=EINT0
    rEXTINT0=rEXTINT0&(7<<0)|(0x2<<0);  //EINT0=falling edge triggered
#endif

#if 1	//test whether or not the interrupt pending bit retain the wake-up source.
    pISR_EINT0=(U32)Eint0Int;
    pISR_EINT8_23=(U32)Eint8_23Int_100Hz;
    pISR_RTC=(U32)AlarmInt;

    rINTMSK = ~(BIT_EINT0|BIT_EINT8_23|BIT_RTC);
    for(i=0;i<100;i++);

    rSRCPND = BIT_EINT0|BIT_EINT8_23|BIT_RTC;
    rINTPND = BIT_EINT0|BIT_EINT8_23|BIT_RTC;

    rEINTPEND=rEINTPEND;
    rSRCPND=rSRCPND;
    rINTPND=rINTPND;

#endif     

    rEINTMASK = rEINTMASK&~(1<<11); //SRCPND:EINT8_23 will be set by EINT11 after wake-up.
    rINTMSK = BIT_ALLMSK;
        // NOTE: Any interrupt can't be used in STOP mode 
        //       because SDRAM is in self-refresh mode and ISR code will access SDRAM.

#if ALARM_WAKEUP    
    SetAlarmWakeUp();
#endif

    rRTCCON=0x0;   // R/W disable, 1/32768, Normal(merge), No reset
    rADCCON|=(1<<2);

    rMISCCR|=(1<<12); //USB port0 = suspend
    rMISCCR|=(1<<13); //USB port1 = suspend

    rMISCCR|=(1<<2); //Previous state at STOP(?) mode (???)
    
    rGSTATUS3=(U32)StartPointAfterPowerOffWakeUp;
    rGSTATUS4=0xaaaaaaaa;

#if EXTERNAL_BUS_HOLDER
    //D[31:0] pull-up off. The data bus will not be float by the external bus holder.
    //If the pull-up resitsers are turned on,
    //there will be the leakage current through the pull-up resister
    rMISCCR=rMISCCR|(3<<0); 
#else
    //D[31:0] pull-up on. The data bus will not be float by pull-up resister. 
    rMISCCR=rMISCCR&~(3<<0); 
#endif

    Lcd_EnvidOnOff(0); //Before entering STOP mode, LCD must be off    
    Uart_Printf("e. PO\n\n");
    Uart_TxEmpty(0);	//Wait until UART0 Tx buffer empty.

    //=================================================================
    //  VERY IMPORTANT NOTE 
    //  To enter STOP/SLIDLE mode, MMU TLB-fill operation should be prohibited
    //   because MTT table is in SDRAM and SDRAM is in the self-refresh mode.
    //  So, we will fill TLB before entering SDRAM self-refresh 
    //   instead of disabling MMU.
    rREFRESH;	//To fill TLB for the special register used in EnterPWDN
    rCLKCON;
    //=================================================================

    EnterPWDN(0x7fff8); //POWER_OFF mode
    //Never return here. 
}