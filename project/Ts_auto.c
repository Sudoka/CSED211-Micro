//====================================================================
// File Name : Ts_auto.c
// Function  : S3C2410 Touch Screen Panel Auto Test
// Source    : Weon-Tark Kang
// Modify    : On-Pil Shin (SOP)
// Date      : December 07, 2002
// Version   : 0.0
// History
//   0.0 : First release to customer -> Tark
//   0.1 :                           -> SOP 2002.12.07
//====================================================================

#include <string.h>
#include "2410addr.h"
#include "2410lib.h"
#include "Ts_auto.h"

#define ADCPRS 39

#define ITERATION 5
unsigned int buf[ITERATION][2];

void __irq Adc_or_TsAuto(void)
{
    int i,j;
    
    rINTSUBMSK |= (BIT_SUB_ADC | BIT_SUB_TC);     //Mask sub interrupt (ADC and TC) 

    Uart_Printf("\nStylus Down!!\n");
        
      //Auto X-Position and Y-Position Read
    rADCTSC=(1<<7)|(1<<6)|(0<<5)|(1<<4)|(1<<3)|(1<<2)|(0);
          //[7] YM=GND, [6] YP is connected with AIN[5], [5] XM=Hi-Z, [4] XP is connected with AIN[7]
          //[3] XP pull-up disable, [2] Auto(sequential) X/Y position conversion mode, [1:0] No operation mode    

    for(i=0;i<ITERATION;i++)
    {
        rADCTSC  = (1<<7)|(1<<6)|(0<<5)|(1<<4)|(1<<3)|(1<<2)|(0);            
        rADCCON |= 0x1;             //Start Auto conversion
        while(rADCCON & 0x1);       //Check if Enable_start is low
        while(!(0x8000&rADCCON));   //Check ECFLG
    
        buf[i][0] = (0x3ff&rADCDAT0);
        buf[i][1] = (0x3ff&rADCDAT1);
    }

    for(i=0;i<ITERATION;i++)    
        Uart_Printf("X, Y Position is (%04d , %04d)\n", buf[i][0], buf[i][1]);
         
    rADCTSC = (1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);  
      //[7] YM=GND, [6] YP is connected with AIN[5], [5] XM=Hi-Z, [4] XP is connected with AIN[7]
      //[3] XP pull-up enable, [2] Normal ADC conversion, [1:0] Waiting for interrupt mode                
    rSUBSRCPND |= BIT_SUB_TC;
    rINTSUBMSK  =~(BIT_SUB_TC);   //Unmask sub interrupt (TC)     
    ClearPending(BIT_ADC);
}
            
//===============================================================================
void Ts_Auto(void)
{
    Uart_Printf("[ Touch Screen Test ]\n");
    Uart_Printf("Auto X/Y position conversion mode test\n");

    rADCDLY = (50000);    // ADC Start or Interval Delay

    rADCCON = (1<<14)|(ADCPRS<<6)|(0<<3)|(0<<2)|(0<<1)|(0); 
      // Enable Prescaler,Prescaler,AIN5/7 fix,Normal,Disable read start,No operation
    rADCTSC=(0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);//tark
      // Down,YM:GND,YP:AIN5,XM:Hi-z,XP:AIN7,XP pullup En,Normal,Waiting for interrupt mode

    pISR_ADC   = (unsigned)Adc_or_TsAuto;
    rINTMSK    =~(BIT_ADC);
    rINTSUBMSK =~(BIT_SUB_TC);

    Uart_Printf("\nType any key to exit!!!\n");
    Uart_Printf("\nStylus Down, please...... \n");
    Uart_Getch();

    rINTSUBMSK |= BIT_SUB_TC;
    rINTMSK    |= BIT_ADC;
}