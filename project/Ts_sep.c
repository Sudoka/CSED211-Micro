#include <string.h>
#include "2410addr.h"
#include "2410lib.h"
#include "Ts_sep.h"
#include "def.h"

#define LOOP 1
#define ADCPRS 39


void __irq Adc_or_TsSep(void)
{
    int i;
    U32 Pt[6];
    
    rINTSUBMSK |= (BIT_SUB_ADC|BIT_SUB_TC); // Mask sub interrupt (ADC and TC) 

      // TC(Touch screen Control) Interrupt 
    if(rADCTSC & 0x100)
    {
        Uart_Printf("\nStylus Up!!\n");
        rADCTSC &= 0xff;    // Set stylus down interrupt
    }
    else 
    {
        Uart_Printf("\nStylus Down!!\n");
           
          // <X-Position Read>
        rADCTSC=(0<<8)|(0<<7)|(1<<6)|(1<<5)|(0<<4)|(1<<3)|(0<<2)|(1);
          // Down,Hi-Z,AIN5,GND,Ext vlt,Pullup Dis,Normal,X-position
        for(i=0;i<LOOP;i++);            //delay to set up the next channel
        for(i=0;i<5;i++)
        {
            rADCCON|=0x1;               // Start X-position conversion
            while(rADCCON & 0x1);       // Check if Enable_start is low
            while(!(0x8000&rADCCON));   // Check ECFLG
            Pt[i]=(0x3ff&rADCDAT0);
        }
        Pt[5]=(Pt[0]+Pt[1]+Pt[2]+Pt[3]+Pt[4])/5;
        Uart_Printf("X-Posion[AIN5] is %04d\n", Pt[5]);

          // <Y-Position Read>
        rADCTSC=(0<<8)|(0<<7)|(1<<6)|(1<<5)|(0<<4)|(1<<3)|(0<<2)|(2);
          // Down,GND,Ext vlt,Hi-Z,AIN7,Pullup Dis,Normal,Y-position
        for(i=0;i<LOOP;i++);            //delay to set up the next channel
        for(i=0;i<5;i++)
        {
            rADCCON|=0x1;               // Start X-position conversion
            while(rADCCON & 0x1);       // Check if Enable_start is low
            while(!(0x8000&rADCCON));   // Check ECFLG
            Pt[i]=(0x3ff&rADCDAT1);
        }
        Pt[5]=(Pt[0]+Pt[1]+Pt[2]+Pt[3]+Pt[4])/5;
        Uart_Printf("Y-Posion[AIN7] is %04d\n", Pt[5]);
        rADCTSC=(1<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
          // Up,GND,AIN,Hi-z,AIN,Pullup En,Normal,Waiting mode
    }

    rSUBSRCPND |= BIT_SUB_TC;
    rINTSUBMSK =~ (BIT_SUB_TC);         // Unmask sub interrupt (TC)     
    ClearPending(BIT_ADC);
}
            

void Ts_Sep(void)
{
    Uart_Printf("[Touch Screen Test.]\n");
    Uart_Printf("Separate X/Y position conversion mode test\n");

    rADCDLY = (50000);  // ADC Start or Interval Delay

    rADCCON = (1<<14)|(ADCPRS<<6)|(0<<3)|(0<<2)|(0<<1)|(0);
      // Enable Prescaler,Prescaler,AIN7/5 fix,Normal,Disable read start,No operation
    rADCTSC = (0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
      // Down,YM:GND,YP:AIN5,XM:Hi-z,XP:AIN7,XP pullup En,Normal,Waiting for interrupt mode

    pISR_ADC   = (unsigned)Adc_or_TsSep;
    rINTMSK    =~(BIT_ADC);
    rINTSUBMSK =~(BIT_SUB_TC);

    Uart_Printf("\nType any key to exit!!!\n");
    Uart_Printf("\nStylus Down, please...... \n");
    
    Uart_Getch();

    rINTSUBMSK |= BIT_SUB_TC;
    rINTMSK    |= BIT_ADC;
    Uart_Printf("[Touch Screen Test.]\n");
}