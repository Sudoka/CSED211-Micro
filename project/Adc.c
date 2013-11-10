//====================================================================
// File Name : Adc.c
// Function  : S3C2410 ADC Test 
// Program   : Kang, Weon Tark 
// Date      : May 22, 2002
// Version   : 0.0
// History
//   0.0 : Programming start (March 29,2002) -> KWT
//         ADC Test                          -> May 15, 2002 SOP
//====================================================================
#include <string.h>
#include "2410addr.h"
#include "2410lib.h"
#include "adc.h"
#include "def.h"

#define REQCNT 100              //May 08, 2002 SOP
#define ADC_FREQ 2500000
//#define ADC_FREQ   1250000
//#define LOOP 1
#define LOOP 10000

void __irq DMA0_Done(void);	//Declare Prototype function

int ReadAdc(int ch);	        //Return type is int, Declare Prototype function
volatile U32 preScaler;

//==================================================================================
void Test_Adc(void)
{
    int i,key;
    int a0=0,a1=0,a2=0,a3=0,a4=0,a5=0,a6=0,a7=0; //Initialize variables
    
    Uart_Printf("[ ADC_IN Test ]\n");
    Uart_Printf("0. Dispaly Count 100     1. Continued ...\n\n");
    Uart_Printf("Selet : ");
    key = Uart_GetIntNum();
    Uart_Printf("\n\n");    
        
    Uart_Printf("The ADC_IN are adjusted to the following values.\n");	      
    Uart_Printf("Push any key to exit!!!\n");	 

    preScaler = ADC_FREQ;
    Uart_Printf("ADC conv. freq. = %dHz\n",preScaler);
    preScaler = PCLK/ADC_FREQ -1;               //PCLK:50.7MHz
    //ADC conversion time = 5CYCLES*(1/(ADC Freq.)), ADC Freq. = PCLK/(ADCPSR+1)
    
    Uart_Printf("PCLK/ADC_FREQ - 1 = %d\n",preScaler);
    
    if (key == 0)
    {
        Uart_Printf("[ AIN2 ]\n");                          //May 08, 2002 SOP   
    
        for(i=0;i<REQCNT;i++)                               //May 08, 2002 SOP
        {
//	    a0=ReadAdc(0);
//	    a1=ReadAdc(1);
	    a2=ReadAdc(2);
//	    a3=ReadAdc(3);
//	    a4=ReadAdc(4);
//	    a5=ReadAdc(5);
//	    a6=ReadAdc(6);
//	    a7=ReadAdc(7);

	    Uart_Printf("%04d\n",a2);	                //May 08, 2002 SOP
        }
    }
    
    else if(key == 1)
    {
        while(Uart_GetKey()==0)
        {
	    a0=ReadAdc(0);
	    a1=ReadAdc(1);
	    a2=ReadAdc(2);
	    a3=ReadAdc(3);
	    a4=ReadAdc(4);
	    a5=ReadAdc(5);
	    a6=ReadAdc(6);
	    a7=ReadAdc(7);

	    Uart_Printf("AIN0: %04d AIN1: %04d AIN2: %04d AIN3: %04d AIN4: %04d AIN5: %04d AIN6: %04d AIN7: %04d\n", a0,a1,a2,a3,a4,a5,a6,a7);
        }        
    }
    rADCCON=(0<<14)|(19<<6)|(7<<3)|(1<<2);  //stand by mode to reduce power consumption	

    Uart_Printf("\nrADCCON = 0x%x\n", rADCCON);
}

//==================================================================================		
int ReadAdc(int ch)
{
    int i;
    static int prevCh=-1;

    rADCCON = (1<<14)|(preScaler<<6)|(ch<<3);	//setup channel

    if(prevCh!=ch)
    {
	rADCCON = (1<<14)|(preScaler<<6)|(ch<<3);   //setup channel
	for(i=0;i<LOOP;i++);	//delay to set up the next channel
	prevCh=ch;
    }
    rADCCON|=0x1;   //start ADC

    while(rADCCON & 0x1);	//check if Enable_start is low
    while(!(rADCCON & 0x8000));	//check if EC(End of Conversion) flag is high

    return ( (int)rADCDAT0 & 0x3ff );
}

//==================================================================================
volatile char end_test;

void Test_DMA_Adc(void)
{
    unsigned int *dst,*tmp,i;
    int key;
    
    end_test=0;

    pISR_DMA0=(unsigned)DMA0_Done;	//Regist interrupt service routine
    rINTMSK=~(BIT_DMA0);		//Clear int mask bit. 

    Uart_Printf("Test of the 'Start by read' in ADC block.\n");
    Uart_Printf("Change the voltage of AIN2..(Sampling 100)\n");

    dst = (unsigned int *)0x31000000;	//Non-cacheable Area
    tmp = dst;
    for(i=0;i<REQCNT;i++)
	*tmp++=i;

    /***ADC init***/
    preScaler = PCLK/ADC_FREQ -1;
    rADCCON=(1<<14)|(preScaler<<6)|(2<<3)|0x2;	//normal,ADCPRS,AIN2,enable start by read

    /***DMA0 init***/
    rDISRC0=0x5800000c;		// rADCDAT0
    rDISRCC0=(1<<1)|(1<<0);	// APB,Fixed
    rDIDST0=(int)dst;		// dst
    rDIDSTC0=(0<<1)|(0<<0);	// AHB,++
    rDCON0=(1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(3<<24)|(1<<23)|(1<<22)|(2<<20)|REQCNT;
	    //handshake, sync PCLK, TC int, single tx, single service, Timer, Timer(H/W) request, 
	    //auto-reload off, word, REQCNT
    rDMASKTRIG0=(0<<2)|(1<<1)+0;    //no-stop, DMA0 channel on, no-SW trigger 
/*
    for(i=0;i<5;i++)
	Uart_Printf("%d:0x%x\n",i, *(int *)(0x4b000000+i*4) );
*/
    /***Timer0 init***/
    rTCFG0=255;		//prescaler0=255
//    rTCFG0=0;		//prescaler0=0
    rTCFG1=(1<<20) | 3;	//Timer0 DMA, div=16 
//    rTCNTB0=4900;	//(1/(50MHz/255/16))*4900=(1/(12254.9) * 4900)=0.0000816 * 4900 = 0.401 sec 
    rTCNTB0=1226;	//0.0000816 * 1226 = 0.1 sec
//    rTCNTB0=2452;	//0.0000816 * 2452 = 0.2 sec    
//    rTCNTB0=1716;	//0.0000816 * 1716 = 0.14 sec    
//    rTCNTB0=10;	        //(1/(50MHz/1/16))*10=(1/(3125000)*10)=0.0000003*10 = 0.000003 sec
//    rTCNTB0=100;	        //0.0000003*100 = 0.00003 sec    
//    rTCNTB0=2000;	        //(1/(50MHz/1/16))*2000=(1/(3125000)*2000)=0.0000003*2000 = 0.0006 sec
    rTCON=0xa;		//auto reload, manual update
    rTCON=0x9;		//Start Timer0
    rADCDAT0;		//Read out dummy data and start first ADC operation

    while(!end_test);

    tmp=dst;

    for(i=0;i<REQCNT;i++)
    {
	*tmp = (*tmp) & 0x3ff;
	tmp++;
    }

    for(i=0;i<REQCNT;i++)
        Uart_Printf("%04d\n",*dst++);                   //May 08, 2002 SOP
//	Uart_Printf("%02d=%04d\n",i,*dst++);
//	Uart_Printf("%02d=0x%x\n",i,*dst++);

    rINTMSK|=BIT_DMA0;
}

//==================================================================================
void __irq DMA0_Done(void)
{
    ClearPending(BIT_DMA0);

    rTCON=0x0;		//Stop
    end_test=1;		//set end flag
}
