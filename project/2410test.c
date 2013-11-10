//====================================================================
// File Name : 2410test.c
// Function  : S3C2410 Test Main Menu
// Program   : Shin, On Pil (SOP)
// Date      : June 13, 2003
// Version   : 0.0
// History
//   0.0 : Programming start (February 20,2002) -> SOP
//   1.0 (20020530) : First release for customer
//   1.1 (20020801) : Strata NOR Flash Added and etc. -> SOP
//   1.2 (20020930) : Added IIS Slave mode Test Menu. -> SOP
//       (20021208) : IIS Record & Play Test
//       (20030324) : Added K9S1208 Erase -> SOP
//       (20030331) : Added Test_Cache -> SOP
//       (20030613) : SOP
//====================================================================

#include <stdlib.h>
#include <string.h>

//Shin, On Pil
#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h"

#include "2410etc.h"
#include "2410IIC.h"
#include "2410iis.h"
#include "2410int.h"
#include "2410RTC.h"
#include "2410swi.h"
//#include "lcdpwr.h"
#include "timer.h"

//Lee, Sanug Jo
#include "cpuspeed.h"

//Kang, Weon Tark
#include "adc.h"
#include "dma.h"
#include "dma2.h"
#include "eint.h"
#include "extdma.h"
#include "k9s1208.h"
#include "mmu.h"
#include "nwait.h"
#include "sdi.h"
#include "stone.h"
#include "ts_auto.h"
#include "ts_sep.h"
#include "usbfifo.h"

//Shin, Jeong Seuk
#include "IrDA.h"
#include "lcd.h"
#include "lcdlib.h"
#include "glib.h"
#include "palette.h"
#include "spi.h"
#include "uart0.h"
#include "uart1.h"
#include "uart2.h"

//Kong, In Wook
//#include "iis_dual.h"
#include "etc.h"
#include "flash.h"
#include "idle.h"
#include "pd6710.h"
//#include "pll.h"
#include "power.h"
#include "pwr_c.h"
#include "PowerOff.h"

//Jeon, Chan Sik
#include "cache.h"

//±Ëµø±‘, ±ËΩ√»∆
#include "game.h"

void Isr_Init(void);
void HaltUndef(void);
void HaltSwi(void);
void HaltPabort(void);
void HaltDabort(void);

void * function[][2]=
{
//ADC, TSP
    (void *)Test_Adc,                       "ADC                 ",
    (void *)Test_DMA_Adc,                   "ADC with DMA        ",
    (void *)Ts_Sep,                         "ADC TSP Seperate    ",
    (void *)Ts_Auto,                        "ADC TSP Auto        ",
//DMA   
    (void *)Test_DMA,                       "DMA M2M             ",
    (void *)Test_DMAWorst,                  "DMA Worst Test      ",
    (void *)Test_Dma0Xdreq,                 "External DMA        ",    
//EINT  
    (void *)Test_Eint,                      "External Interrupt  ",
//IIC   
    (void *)Test_Iic,                       "IIC(KS24C080)INT    ",
    (void *)Test_Iic2,                      "IIC(KS24C080)POL    ",
//IIS   
//    (void *)Test_IisRecPlay,                "IIS Reco&Play Test  ",
//    (void *)Record_Iis,                     "Reco IIS UDA1341    ",
//    (void *)Test_Iis,                       "Play IIS UDA1341    ",
//    (void *)Iis_Tx,                         "IIS Slave Set Test  ",
//    (void *)Iis_Rx,                         "IIS Master Set Test ",            
//Interrupt     
    (void *)Test_Fiq,                       "FIQ Interrupt       ",
    (void *)Change_IntPriorities,           "Change INT Priority ",
//IrDA  
    (void *)Test_IrDA_Rx,                   "UART2 IrDA Rx       ",
    (void *)Test_IrDA_Tx,                   "UART2 IrDA Tx       ",
//LCD   
    (void *)Test_PaletteMemory,             "LCD Palette RAM     ",
    (void *)Test_Lcd_Stn_1Bit,              "STN 1Bit            ",
    (void *)Test_Lcd_Stn_2Bit,              "STN 2Bit            ",
    (void *)Test_Lcd_Stn_4Bit,              "STN 4Bit            ",    

    (void *)Test_Lcd_Cstn_8Bit,             "CSTN  8Bit          ",
    (void *)Test_Lcd_Cstn_8Bit_On,          "CSTN  8Bit On       ",    
    (void *)Test_Lcd_Cstn_12Bit,            "CSTN 12Bit          ",

    (void *)Test_Lcd_Tft_8Bit_240320,       "TFT240320  8Bit     ",
    (void *)Test_Lcd_Tft_8Bit_240320_On,    "TFT240320  8Bit On  ",    
    (void *)Test_Lcd_Tft_16Bit_240320,      "TFT240320 16Bit     ",
//  (void *)Test_Lcd_Tft_8Bit_240320_Bmp,   "TFT240320 Bmp       ",

    (void *)Test_Lcd_Tft_1Bit_640480,       "TFT640480  1Bit     ",
    (void *)Test_Lcd_Tft_8Bit_640480,       "TFT640480  8Bit     ",
    (void *)Test_Lcd_Tft_16Bit_640480,      "TFT640480 16Bit     ",

    (void *)Test_Lcd_Tft_8Bit_640480_Bswp,  "TFT640480 BSWP      ",
    (void *)Test_Lcd_Tft_8Bit_640480_Palette, "TFT640480 Palette   ",  
    (void *)Test_Lcd_Tft_16Bit_640480_Hwswp,"TFT640480 HWSWP     ",

//  (void *)Test_Lcd_PowerOnOFF,            "LCD Power On/Off    ",    
//Memory
//  (void *)WriteMemory,                    "MEMORY Write        ",
//  (void *)ReadMemory,                     "MEMORY Read         ",
//MPLL
//    (void *)Test_PLL,                       "MPLL Change         ",
//    (void *)ChangePLL,                      "MPLL MPS Change     ",
//    (void *)Test_PllOnOff,                  "MPLL On/Off         ",  
//PMS
    (void *)Test_SlowMode,                  "PMS Slow            ",     
    (void *)Test_HoldMode,                  "PMS Hold            ",     
    (void *)Test_IdleMode,                  "PMS Idle            ",
    (void *)Test_MMUIdleMode,               "PMS Idle(MMU)       ",
    (void *)Test_IdleModeHard,              "PMS Idle Hard       ",
    (void *)Test_InitSDRAM,                 "PMS SDRAM Init      ",
    (void *)Test_PowerOffMode,              "PMS Power-Off       ",     
    (void *)Test_PowerOffMode_100Hz,        "PMS Power-Off 100Hz ",
    (void *)MeasurePowerConsumption,        "PMS Measure Power   ",    
//RTC
    (void *)Test_Rtc_Alarm,                 "RTC Alarm           ",
    (void *)Display_Rtc,                    "RTC Display         ",    
    (void *)RndRst_Rtc,                     "RTC Round Reset     ",    
    (void *)Test_Rtc_Tick,                  "RTC Tick            ",
//SDI
    (void *)Test_SDI,                       "SDI Write/Read      ",
//  (void *)MMC_Protect,                    "SDI MMC Protect     ",
//  (void *)MMC_Dma_StrRd,                  "SDI MMC Read        ",
//  (void *)MMC_Dma_StrWt,                  "SDI MMC Write       ", 
//SPI
    (void*) Test_Spi_MS_int,                "SPI0 RxTx Int       ",
    (void *)Test_Spi_MS_poll,               "SPI0 RxTx POLL      ",
    (void *)Test_Spi_M_Tx_DMA1,             "SPI0 Master Tx DMA1 ",
    (void *)Test_Spi_S_Rx_DMA1,             "SPI0 Slave Rx DMA1  ",
    (void *)Test_Spi_M_Rx_DMA1,             "SPI0 Master Rx DMA1 ",
    (void *)Test_Spi_S_Tx_DMA1,             "SPI0 Slave Tx DMA1  ",
    (void *)Test_Spi_M_Int,                 "SPI0 Master RxTx INT",
    (void *)Test_Spi_S_Int,                 "SPI0 Slave RxTx INT ",
//Timer
//  (void *)Test_TimerChannel,              "Timer Channel       ",
//  (void *)Test_TimerDma,                  "Timer DMA           ",
    (void *)Test_TimerInt,                  "Timer Interrupt     ",
    (void *)Test_Timer,                     "Timer Tout          ",
//UART
    (void *)Test_Uart0_Int,                 "UART0 Rx/Tx Int     ",
    (void *)Test_Uart0_Dma,                 "UART0 Rx/Tx DMA     ",
    (void *)Test_Uart0_Fifo,                "UART0 Rx/Tx FIFO    ",
    (void *)Test_Uart0_AfcTx,               "UART0 AFC Tx        ",
    (void *)Test_Uart0_AfcRx,               "UART0 AFC Rx        ",

    (void *)Test_Uart1_Int,                 "UART1 Rx/Tx Int     ",
    (void *)Test_Uart1_Dma,                 "UART1 Rx/Tx DMA     ",
    (void *)Test_Uart1_Fifo,                "UART1 Rx/Tx FIFO    ",
    (void *)Test_Uart1_AfcTx,               "UART1 AFC Tx        ",
    (void *)Test_Uart1_AfcRx,               "UART1 AFC Rx        ",

    (void *)Test_Uart2_Int,                 "UART2 Rx/Tx Int     ",
    (void *)Test_Uart2_Dma,                 "UART2 Rx/Tx DMA     ",
    (void *)Test_Uart2_Fifo,                "UART2 Rx/Tx FIFO    ",
//USB
    (void *)Test_USBFIFO,                   "USB FIFO Test       ",
//WDT
    (void *)Test_WDT_IntReq,                "WDT INT Request     ",
//ETC           
    (void *)Test_CpuSpeed,                  "Core Speed(LED)     ",
    (void *)Test_XBREQ,                     "External Bus Reqest ",
    (void *)Test_NonalignedAccess,          "NonAlgined Access   ",
    (void *)Test_PD6710,                    "PC Card (PD6710)    ",    
    (void *)ReadPageMode,                   "Read Page Mode      ",
    (void *)Test_SwiIrq,                    "SWI                 ",
    (void *)Test_WaitPin,                   "External Wait       ",
    (void *)Test_ISram,                     "Stone Test          ",  
    (void *)Test_NecInterrupt,              "ETC NEC Int         ",
    (void *)Test_BattFaultInterrupt,        "nBATT_FAULT int     ",  
    (void *)Test_Cache,                     "Core Cache Test     ",        
//NAND, NOR Flash
    (void *)K9S1208_PrintBadBlockNum,       "NAND View Bad Block ",
    (void *)K9S1208_PrintBlock,             "NAND View Page      ",
    (void *)K9S1208_Program,                "NAND Write          ", 
    (void *)K9S1208_Erase,                  "NAND Erase          ",  
    (void *)TestECC,                        "NAND ECC            ", 
    (void *)ProgramFlash,                   "NOR Flash Program   ",    
    (void *)gamestart,						"Space Invader		 ",
	0
};

//===================================================================
void Main(void)
{
    int i;
    
    Led_Display(15);
    
    MMU_Init();
   
#if ADS10   
   // __rt_lib_init();                //for ADS 1.0
#endif
    
//    ChangeClockDivider(0,0);          // 1:1:1    
//    ChangeClockDivider(0,1);          // 1:1:2    
//    rCLKDIVN |= (1<<2);                     // 1:4:4
//    ChangeClockDivider(1,0);          // 1:2:2 
    ChangeClockDivider(1,1);          // 1:2:4    

//    ChangeMPllValue(0xa1,0x3,0x3);    // FCLK=50.7MHz    
//    ChangeMPllValue(0x7f,0x2,0x2);    // FCLK=101.25MHz
//    ChangeMPllValue(0x96,0x5,0x1);    // FCLK=135428571Hz 
//    ChangeMPllValue(0x2a,0x1,0x0);    // FCLK=200MHz   
//    ChangeMPllValue(0x5c,0x1,0x1);    // FCLK=200MHz
//    ChangeMPllValue(0x5c,0x4,0x0);    // FCLK=200MHz
//    ChangeMPllValue(0x8e,0x7,0x0);    // FCLK=200MHz        
    ChangeMPllValue(0xa1,0x3,0x1);    // FCLK=202.8MHz  
//    ChangeMPllValue(0x66,0x1,0x1);    // FCLK=220MHz   
//    ChangeMPllValue(0x69,0x1,0x1);    // FCLK=226MHz       
//    ChangeMPllValue(0x96,0x2,0x1);    // FCLK=237MHz
//    ChangeMPllValue(0x7d,0x4,0x0);    // FCLK=266MHz      
    
    Port_Init();
    Isr_Init();
//    Rtc_Init();
    Uart_Init(0,115200);
//    Uart_Init(FCLK/4,115200);    
    Uart_Select(0);

    //Check whether or not the POWER_OFF wake-up.
    Delay(0);	//calibrate Delay()
    Check_PowerOffWakeUp();  //It's needed for power-off STOP mode test.   

//SJS July 15, 2002
    //Turn on LCD. All test will be done while the LCD is turned on.
    //PWR_Lcd_Tft_16Bit_240320_On();

    //Save the wasted power consumption on GPIO.
    rIISPSR=(2<<5)|(2<<0); //IIS_LRCK=44.1Khz @384fs,PCLK=50Mhz.
    rGPHCON = rGPHCON & ~(0xf<<18)|(0x5<<18);   //CLKOUT 0,1=OUTPUT to reduce the power consumption.

//SJS July 15, 2002
    //Turn on LCD. All test will be done while the LCD is turned on.
    Test_Lcd_Tft_8Bit_240320_On();

    while(1)
    {
        i = 0;

     
        //GPG4 Output Port [9:8] 00      -> LCD power off
//        rGPGCON = (rGPGCON & 0xfffffcff) | (1<<8);
//        rGPGDAT &= 0xffef;        

        Uart_Printf("\n\nSMDK2410 Board (AP S3C2410X01) Test Program Ver 1.2(20030613) FCLK = %d Hz\n\n", FCLK);
      
        while(1)
        {   //display menu
            Uart_Printf("%2d:%s",i,function[i][1]);
            i++;
            if((int)(function[i][0])==0)
            {
                Uart_Printf("\n");
                break;
            }
            if((i%4)==0)
            Uart_Printf("\n");
        }
        
        Uart_Printf("\nSelect the function to test : ");
        i = Uart_GetIntNum();
        Uart_Printf("\n");

        //GPG4 Output Port [9:8] 01      -> LCD power On
        rGPGCON = (rGPGCON & 0xfffffcff) | (1<<8);
        rGPGDAT = (rGPGDAT & 0xffef) | (1<<4);        
        
        if(i>=0 && (i<(sizeof(function)/8)) ) 
            ( (void (*)(void)) (function[i][0]) )();            
    }
}

//===================================================================
void Isr_Init(void)
{
    pISR_UNDEF  = (unsigned)HaltUndef;
    pISR_SWI    = (unsigned)HaltSwi;
    pISR_PABORT = (unsigned)HaltPabort;
    pISR_DABORT = (unsigned)HaltDabort;
    
    rINTMOD     = 0x0;                     //All=IRQ mode
//    rINTCON=0x5;                           //Non-vectored,IRQ enable,FIQ disable    
    rINTMSK     = BIT_ALLMSK;              //All interrupt is masked.
    rINTSUBMSK  = BIT_SUB_ALLMSK;          //All sub-interrupt is masked. <- April 01, 2002 SOP

//    rINTSUBMSK  = ~(BIT_SUB_RXD0);         //Enable Rx0 Default value=0x7ff
//    rINTMSK     = ~(BIT_UART0);            //Enable UART0 Default value=0xffffffff    
    
//    pISR_UART0=(unsigned)RxInt;            //pISR_FIQ,pISR_IRQ must be initialized
}

//===================================================================
void HaltUndef(void)
{
    Uart_Printf("Undefined instruction exception.\n");
    while(1);
}

//===================================================================
void HaltSwi(void)
{
    Uart_Printf("SWI exception.\n");
    while(1);
}

//===================================================================
void HaltPabort(void)
{
    Uart_Printf("Pabort exception.\n");
    while(1);
}

//===================================================================
void HaltDabort(void)
{
    Uart_Printf("Dabort exception.\n");
    while(1);
}





/*
//=========================
    while(1)
    {
       Led_Display(3);
    }
//=========================    
*/
/*
//=========================
    while(1)
    {
       Led_Display(1);
       Delay(1500);   
       Led_Display(2);
       Delay(1500);             
       Led_Display(4);
       Delay(1500);   
       Led_Display(8);
       Delay(1500);                
    }
//=========================   
*/