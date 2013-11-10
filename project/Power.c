/*****************************************
  NAME: power.c
  DESC: SLOW,HOLD mode test
  HISTORY:
  03.23.2002:purnnamu: first release for 2410
 *****************************************/
 
#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h" 

#include "lcdlib.h"
#include "pwr_c.h"

#define FIN 	12000000

void __irq SlowEint0Int(void);

volatile int slowExit;


void Test_SlowMode(void)
{
    int i;
    unsigned int saveREFRESH;

    slowExit=0;
    rGPFCON=rGPFCON & ~(3<<0)|(2<<0); //PF0=EINT0
    rEXTINT0=rEXTINT0&(7<<0)|(0x2<<0); //EINT0=falling edge triggered

    pISR_EINT0=(U32)SlowEint0Int;
    rINTMSK=~BIT_EINT0;

    Uart_Printf("[ SLOW MODE TEST ]\n");
    Uart_Printf("1)Entering SLOW mode.\n");
    Uart_Printf("  LEDs are flickered by 200ms period at %dMhz 20 times.\n",FCLK/1000000);
    Uart_Printf("  But, The period will be about %lfms.\n",200*FCLK/12000000.);
    Uart_Printf("  Press EINT0 key to exit SLOW mode\n");
    Uart_TxEmpty(0);	//To avoid being crushed the character

    for(i=0;i<20;i++)
    {
	Led_Display(0xf);
	Delay(1000);
	Led_Display(0x0);
	Delay(1000);
    }

    rCLKSLOW=0|(1<<4)|(1<<5)|(1<<7); //FCLK=FIN/1,SLOW mode,MPLL=off,UPLL=off 

    saveREFRESH=rREFRESH;
    rREFRESH=(1<<23)|(U32)(2048+1-12*15.6); //Trp=2clk,Trc=4clk
        //The other memory control register should be optimized for SLOW mode.
        
    while(!slowExit)
    {
	Led_Display(0xf);
	DoQsort(); 
	Led_Display(0x0);
	DoQsort();
    }
    
    rINTMSK=BIT_ALLMSK;

    rCLKSLOW=0|(1<<4)|(0<<5);//PLL on,MPLL=on
    for(i=0;i<2048;i++); //S/W MPLL lock-time
    rCLKSLOW=0|(0<<4)|(0<<5);//NORMAL mode,PLL on,MPLL=on
}

void __irq SlowEint0Int(void)
{
    ClearPending(BIT_EINT0);
    slowExit=1;
}


void Test_HoldMode(void)
{
  
    Uart_Printf("[ Power Saving Mode by Stopping Each Block ]\n");
    Uart_Printf("LCD ENVID is turned off.\n");
    Lcd_EnvidOnOff(0);
    Uart_Printf("Check the current cunsumption. Type any key to proceed.\n");	
    //Except GPIO,UART0
    rCLKCON=0x7fff0;
    Uart_Getch();

    Uart_Printf("NFC off.\n");
    rCLKCON&=~(1<<4);
    Uart_Getch();

    Uart_Printf("LCD off.\n");
    rCLKCON&=~(1<<5);
    Lcd_EnvidOnOff(0);
    Uart_Getch();
    
    Uart_Printf("USB host off(DP0/DN0 is suspended).\n");
    rCLKCON&=~(1<<6);
    rMISCCR|=(1<<12); //USB port0 = suspend
    Uart_Getch();
    
    Uart_Printf("USB device off(DP1/DN1 is also suspended).\n");
    rCLKCON&=~(1<<7);
    rMISCCR|=(1<<13); //USB port1 = suspend
    Uart_Getch();

    Uart_Printf("PWM Timer off.\n");
    rCLKCON&=~(1<<8);
    Uart_Getch();

    Uart_Printf("SDI off.\n");
    rCLKCON&=~(1<<9);
    Uart_Getch();

    Uart_Printf("UART1 off.\n");
    rCLKCON&=~(1<<11);
    Uart_Getch();

    Uart_Printf("UART2 off.\n");
    rCLKCON&=~(1<<12);
    Uart_Getch();

    Uart_Printf("RTC_control off.\n");
    rCLKCON&=~(1<<14);
    Uart_Getch();
    
    Uart_Printf("ADC off.\n");
    rADCCON|=(1<<2);
    rCLKCON&=~(1<<15);
    Uart_Getch();
    
    Uart_Printf("IIC off.\n");
    rCLKCON&=~(1<<16);
    Uart_Getch();
    
    Uart_Printf("IIS off.\n");
    rCLKCON&=~(1<<17);
    Uart_Getch();
    
    Uart_Printf("SPI off.\n");
    rCLKCON&=~(1<<18);
    Uart_Getch();
    
    Uart_Printf("Return to Normal Mode.\n");	    
    rCLKCON=0xffff0; 
    rADCCON&=(1<<2);
    rMISCCR&=~((1<<12)|(1<<13)); //USB port0,1 = normal
    Lcd_EnvidOnOff(1);
}

