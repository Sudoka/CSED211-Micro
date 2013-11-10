//====================================================================
// File Name : 2410int.c
// Function  : S3C2410 FIQ, Priority Interrupt Test
// Program   : Shin, On Pil (SOP)
// Date      : May 03, 2002
// Version   : 0.0
// History
//   0.0 : Programming start (March 12, 2002) -> SOP
//====================================================================

#include <string.h>
#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h" 

volatile int fiqCnt0, fiqCnt1;

static void __irq T0Int(void)           //FIQ
{
    ClearPending(BIT_TIMER0);
    fiqCnt0 += 1;
}

//--------------------------------------------------------------------
static void __irq T1Int(void)           //IRQ
{
    ClearPending(BIT_TIMER1);           //Timer1 Clear Interrupt
    fiqCnt1 += 1;
    
    if(fiqCnt0!=fiqCnt1)
        fiqCnt1 = 0; 
    if(fiqCnt0==10000)                  //Because FIQ interrupt has higher priority,
        rTCON   = 0x0;                  //fiqCnt0 will always be same with fiqCnt1.
}

//**********************[ Test_Fiq ]**********************************
void Test_Fiq(void)
{
    int intmod;

    Uart_Printf("[ FIQ Interrupt Test ]\n");
    fiqCnt0     = 0;
    fiqCnt1     = 0;    
    pISR_FIQ    = (U32)T0Int;                   //Timer0 FIQ interrupt
    pISR_TIMER1 = (U32)T1Int;                   //Timer1 Interrupt
    intmod      = rINTMOD;                      //Interrupt Mode backup
    rINTMOD     = BIT_TIMER0;                   //Timer0 FIQ mode
    rSRCPND     = BIT_TIMER0 | BIT_TIMER1;      //Timer0,1 To clear the previous pending states
    rINTPND     = BIT_TIMER0 | BIT_TIMER1;      //Timer0,1 Interrupt request

        //PWM Timer control
        //==============================================================================
        //Timer input clock Frequency = PCLK /{prescaler value + 1} / {divider value}
        // {prescaler value} = 0 ~ 255
        // {divider value}   = 2,4,8,16
        //==============================================================================        
    rTCFG0  = 0x0;      //Dead zone length, Prescaler0=Timer0,1  Prescaler1=Timer2,3,4
    rTCFG1  = 0x0;      //Select DMA request channel All Interrupt, Select MUX input for PWM Timer0~4 = 1/2    
    rTCNTB0 = 2570;     //0xa0a, Timer0 count buffer register
    rTCNTB1 = 2570;     //0xa0a, Timer1 count buffer register
    rTCON   = 0xa0a;    //1010 0000 1010 Timer0,1:Auto-Reload,Inverter off, Update TCNTB0,1, TCMPB0,1, Stop
    rTCON   = 0x909;    //1001 0000 1001 Timer0,1:Auto-Reload,Inverter off, No operation, Start for Timer0,1
    
    rINTMSK = ~(BIT_TIMER0 | BIT_TIMER1);       //Timer0,1 Interrupt service is available

    while(fiqCnt1<10000)
    {
        if((fiqCnt1%100)==0)   
           Uart_Printf(".");
    }
    rTCON = 0x0;        //One-shot, Inverter off, No operation, Dead zone disable, Stop
    Delay(1);
    Uart_Printf("\nfiqCnt0 = %d, fiqCnt1 = %d (10000,10000)\n",fiqCnt0,fiqCnt1);
    rINTMOD = intmod;
    rINTMSK = BIT_ALLMSK;
}

//***************[ Change_IntPriorities ]*************************
void Change_IntPriorities(void)
{
    int key;

//    rINTMOD = 0x0;
//    rINTMSK = ~(BIT_ALLMSK);
//    rINTSUBMSK = ~(BIT_SUB_ALLMSK);
    
    Uart_Printf("[ Interrupt Controller Priority Policy ]\n\n");
    Uart_Printf("1. Reverse Priority   2. Priority Rotate Off   3. Default\n\n");
    Uart_Printf("Select : ");
    key = Uart_Getch();
    Uart_Printf("%c\n\n",key);    
    
    switch(key)
    {
       case '1':
          Uart_Printf("Reverse priority is selected.\n");
          rPRIORITY = 0x7f;             //Arbiter 0 to 6 group priority rotate enable
          rPRIORITY = (0x3fff<<7);      //Arbiter 0 and 5 are group priority order set.(REQ 4-1-2-3)
                                        //Arbiter 1 to 4 and 6 are group priority order set.(REQ 0-4-1-2-3-5)
          break;
       case '2':
          Uart_Printf("Priority rotate off is selected.\n");
          rPRIORITY = 0x0;              //Arbiter 0 to 6 group priority does not rotate
          break;
       case '3':
          Uart_Printf("Default mode is selected.\n");
          rPRIORITY = 0x7f;             //Arbiter 0 to 6 group priority rotate enable
          break;
       default:
          break;
    }
    
//    rINTMSK = BIT_ALLMSK;
//    rINTSUBMSK = BIT_SUB_ALLMSK;    
}
