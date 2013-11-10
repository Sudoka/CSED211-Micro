//=============================================================================
// File Name : Timer.c
// Function  : S3C2410 Watch-Dog and Timer
// Program   : Shin, On Pil (SOP)
// Date      : May 21, 2002
// Version   : 0.0
// History
//   0.0 : Programming start (February 27,2002) -> SOP
//=============================================================================

#include <string.h>
#include "2410addr.h"
#include "2410lib.h"
#include "timer.h"

void __irq Timer0Done(void);
void __irq Timer1Done(void);
void __irq Timer2Done(void);
void __irq Timer3Done(void);
void __irq Timer4Done(void);
void __irq Wdt_Int(void);

//=========================================================================
//      SMDK2410 TOUT configuration
//  GPB4=TCLK0, GPB3=TOUT3, GPB2=TOUT2, GPB1=TOUT1, GPB0=TOUT0, GPG11=TCLK1
//  GPH9=CLKOUT0
//
//Timer input clock frequency = PCLK/{prescaler value+1}/{divider value}
//{prescaler value} = 0 ~ 255   , {divider value} = 2,4,8,16
//
//  PWM Timer TOUT0/1/2/3 Test
//=========================================================================
void Test_Timer(void)
{
    int save_B,save_G,save_H,save_PB,save_PG,save_PH,save_MI;
    char key;
    
    Uart_Printf("[ TOUT 0,1,2,3 Test ]\n\n");    
//    Uart_Printf("= Current Port Setting List =\n");
//    Uart_Printf("rGPBCON = 0x%8x,   rGPGCON = 0x%8x,   rGPHCON= 0x%8x,   rMISCCR = 0x%5x\n" ,rGPBCON,rGPGCON,rGPHCON,rMISCCR);
//    Uart_Printf("rGPBUP  = 0x%8x,   rGPGUP  = 0x%8x,   rGPHUP = 0x%8x\n\n",rGPBUP , rGPGUP , rGPHUP);    

    save_B  = rGPBCON;
    save_G  = rGPGCON;
    save_H  = rGPHCON;    
    save_PB = rGPBUP;
    save_PG = rGPGUP;
    save_PH = rGPHUP;
    save_MI = rMISCCR;   
    
    rGPBUP  = rGPBUP  & ~(0x1f)     | 0x1f;         //GPB4 ~ 0
    rGPBCON = rGPBCON & ~(0x3ff)    | 0x2aa;        //Function Setting TCLK0, TOUT3 ~ 0   
    rGPGUP  = rGPGUP  & ~(0x800)    | 0x800;        //GPG11
    rGPGCON = rGPGCON & ~(0xc00000) | 0xc00000;     //TCLK1
    rGPHUP  = rGPHUP  & ~(0x200)    | 0x200;        //GPH9
    rGPHCON = rGPHCON & ~(0x3<<18)  | (0x2<<18);    //CLKOUT0    
    rMISCCR = rMISCCR & ~(0xf0)     | 0x40;         //Select PCLK with CLKOUT0

//    Uart_Printf("= Changed Port Setting List =\n");    
//    Uart_Printf("rGPBCON = 0x%8x,   rGPGCON = 0x%8x,   rGPHCON= 0x%8x,   rMISCCR = 0x%5x\n" ,rGPBCON,rGPGCON,rGPHCON,rMISCCR);
//    Uart_Printf("rGPBUP  = 0x%8x,   rGPGUP  = 0x%8x,   rGPHUP = 0x%8x\n\n",rGPBUP , rGPGUP , rGPHUP);
    Uart_Printf("[ Select Timer Clock ]\n");
    Uart_Printf("a. PCLK        b. External TOUT[0,1,2,3]\n");
    Uart_Printf("\nSelect the function to test : ");
    key = Uart_Getch();
    Uart_Printf("%c\n\n",key);    

    rTCFG0 = rTCFG0 & ~(0xffffff) | 0x10000;                 //Dead zone=1,Prescaler1=0,Prescaler0=0

    switch(key)
    {
        case 'a':
            rTCFG1 = 0x0;             //All Interrupt, MUX 4 ~ 0 : 1/2
            Uart_Printf("PCLK Check Selected\n");            
            Uart_Printf("Probing PCLK : CON15 - 26\n");
            break;
        case 'b':
            rTCFG0 = rTCFG0 & ~(0xffffff) | 0x44444;         //Dead zone=4,Prescaler1=68(0x44),Prescaler0=68(0x44)
//            Uart_Printf("rTCFG0 = 0x%6x   <= Timer configuration register0.\n",rTCFG0);            
            Uart_Printf("External TOUT[0,1,2,3] Check Selected\n");            
//            Uart_Printf("Probing TCLK 0 : U16 - 14\n");
//            Uart_Printf("Probing TCLK 1 : S4  -  1\n");
            break;
        default:
            rGPBCON = save_B; 
            rGPGCON = save_G;       
            rGPHCON = save_H;           
            rGPBUP  = save_PB;
            rGPGUP  = save_PG;
            rGPHUP  = save_PH;
            rMISCCR = save_MI;
            return;
    }
                                                //(1/(PCLK/(Prescaler+1)/divider) * count(Max 65535) = Timer clock (Frequency)
    rTCNTB0 = rTCNTB0 & ~(0xffff) | 2000;       //(1/(50MHz/69/2))  *  2000 =  5.5200 msec (181.159  Hz)
                                                //(1/(50.7MHz/69/2))*  2000 =  5.4437 msec (183.698  Hz)
                                                //(1/(50MHz/1/2))   *  2000 =  0.0800 msec ( 12.500 KHz)
                                                //(1/(50.7MHz/1/2)) *  2000 =  0.0788 msec ( 12.690 KHz)
    rTCNTB1 = rTCNTB1 & ~(0xffff) | 4000;       //(0.0000027600003) *  4000 = 11.0400 msec ( 90.579  Hz)
                                                //(0.0000027218935) *  4000 = 10.8875 msec ( 91.848  Hz)
                                                //(0.00000004)      *  4000 =  0.1600 msec (  6.250 KHz)
                                                //(0.0000000394477) *  4000 =  0.1577 msec (  6.337 KHz)
    rTCNTB2 = rTCNTB2 & ~(0xffff) | 5000;       //(0.0000027600003) *  5000 = 13.8000 msec ( 72.463  Hz)
                                                //(0.0000027218935) *  5000 = 13.6094 msec ( 73.478  Hz)
                                                //(0.00000004)      *  5000 =  0.2000 msec (  5.000 KHz)
                                                //(0.0000000394477) *  5000 =  0.1972 msec (  5.070 KHz)
    rTCNTB3 = rTCNTB2 & ~(0xffff) | 10000;      //(0.0000027600003) * 10000 = 27.6000 msec ( 36.231  Hz)
                                                //(0.0000027218935) * 10000 = 27.2189 msec ( 36.739  Hz)
                                                //(0.00000004)      * 10000 =  0.4000 msec (  2.500 KHz)
                                                //(0.0000000394477) * 10000 =  0.3944 msec (  2.535 KHz)
    
//    Uart_Printf("rTCNTB0 = %d   <= Timer 0 counter buffer register.\n",rTCNTB0);

    rTCMPB0 =  2000 - 1000;      //(H/L)duty 50%
    rTCMPB1 =  4000 - 2000;
    rTCMPB2 =  5000 - 2500;
    rTCMPB3 = 10000 - 5000;

//    Uart_Printf("rTCMPB0 = %d   <= Timer 0 compare buffer register.\n",rTCMPB0);
//    Uart_Printf("rTCON  = 0x%6x (Before)  <= Timer control register.\n",rTCON); 

     // [22:20] [19:16] [15:12] [11:8] [7:4] [3:0]
     //   110     1010   1010    1010   0000  1010
    rTCON   = rTCON & ~(0xffffff) | 0x6aaa0a;  //Auto reload, Inverter off, Manual update, Stop, Dead zone disable

//    Uart_Printf("rTCON  = 0x%6x (After)   <= Timer control register.(0x6aaa0a)\n",rTCON);
//    Uart_Printf("rTCON  = 0x%6x (Before)  <= Timer control register.\n",rTCON);  
    
     // [22:20] [19:16] [15:12] [11:8] [7:4] [3:0]
     //   101     1001   1001    1001   0000  1001
    rTCON   = rTCON & ~(0xffffff) | 0x599909;  //Auto reload, Inverter off, No operation, Start, Dead zone disable

//    Uart_Printf("rTCON  = 0x%6x (After)   <= Timer control register.(0x599909)\n\n",rTCON);
    
    Uart_Printf("Probing TOUT 0 : J10 -  2\n");
    Uart_Printf("Probing TOUT 1 : U20 -  1\n");
    Uart_Printf("Probing TOUT 2 : U16 - 13\n");
    Uart_Printf("Probing TOUT 3 : U16 - 15\n");    
    
    if(key=='a' && PCLK==50000000)              //Oscilloscope Measurement OK, May 21, 2002 SOP
    {
        Uart_Printf("PCLK 50MHz, Timer TOUT 0 : 0.08 msec (12.50 KHz)\n");
        Uart_Printf("PCLK 50MHz, Timer TOUT 1 : 0.16 msec ( 6.25 KHz)\n");
        Uart_Printf("PCLK 50MHz, Timer TOUT 2 : 0.20 msec ( 5.00 KHz)\n");
        Uart_Printf("PCLK 50MHz, Timer TOUT 3 : 0.40 msec ( 2.50 KHz)\n");                        
    }  

    else if(key=='a' && (PCLK==(202800000/4)))  //Oscilloscope Measurement OK, May 21, 2002 SOP
    {
        Uart_Printf("PCLK 50.7MHz, Timer TOUT 0 : 0.0788 msec ( 12.690 KHz)\n");
        Uart_Printf("PCLK 50.7MHz, Timer TOUT 1 : 0.1577 msec (  6.337 KHz)\n");
        Uart_Printf("PCLK 50.7MHz, Timer TOUT 2 : 0.1972 msec (  5.070 KHz)\n");
        Uart_Printf("PCLK 50.7MHz, Timer TOUT 3 : 0.3944 msec (  2.535 KHz)\n");        
    }    
      
    else if(key=='b' && PCLK==50000000)         //Oscilloscope Measurement OK, May 21, 2002 SOP
    {
        Uart_Printf("PCLK 50MHz, Timer TOUT 0 :  5.5200 msec (181.159 Hz)\n");
        Uart_Printf("PCLK 50MHz, Timer TOUT 1 : 11.0400 msec ( 90.579 Hz)\n");
        Uart_Printf("PCLK 50MHz, Timer TOUT 2 : 13.8000 msec ( 72.463 Hz)\n");
        Uart_Printf("PCLK 50MHz, Timer TOUT 3 : 27.6000 msec ( 36.231 Hz)\n");                        
    }
    
    else if(key=='b' && (PCLK==(202800000/4)))  //Oscilloscope Measurement OK, May 21, 2002 SOP
    {
        Uart_Printf("PCLK 50.7MHz, Timer TOUT 0 :  5.4437 msec (183.698 Hz)\n");
        Uart_Printf("PCLK 50.7MHz, Timer TOUT 1 : 10.8875 msec ( 91.848 Hz)\n");
        Uart_Printf("PCLK 50.7MHz, Timer TOUT 2 : 13.6094 msec ( 73.478 Hz)\n");
        Uart_Printf("PCLK 50.7MHz, Timer TOUT 3 : 27.2189 msec ( 36.739 Hz)\n");                        
    }    
    
    Uart_Printf("\nCheck PWM (Pulse Width Modulation) Output\n");
    Uart_Printf("Press any key to exit.\n");
    Uart_Getch();   
                        //Stop timer
    rTCON   = 0x0;      //One-shot, Inverter off, No operation, Dead zone disable, Stop

//    Uart_Printf("rTCON  = 0x%6x   <= Timer control register.(0x0)\n",rTCON);

    rGPBCON = save_B; 
    rGPGCON = save_G;       
    rGPHCON = save_H;     
    rGPBUP  = save_PB;
    rGPGUP  = save_PG;
    rGPHUP  = save_PH;
    rMISCCR = save_MI;
}


//========================================================
// Timer Interrupt 0/1/2/3/4 test
//========================================================
volatile int variable0,variable1,variable2,variable3,variable4;
void Test_TimerInt(void)
{
    variable0 = 0;variable1 = 0;variable2 = 0;variable3 = 0;variable4 = 0;

//    Uart_Printf("rINTMSK (Before) = 0x%8x\n",rINTMSK);
    rINTMSK = ~(BIT_TIMER4 | BIT_TIMER3 | BIT_TIMER2 | BIT_TIMER1 | BIT_TIMER0);
//    Uart_Printf("rINTMSK (After)  = 0x%8x   <= Timer4,3,2,1 Bit[14:10]\n",rINTMSK);

    pISR_TIMER0 = (int)Timer0Done;
    pISR_TIMER1 = (int)Timer1Done;
    pISR_TIMER2 = (int)Timer2Done;
    pISR_TIMER3 = (int)Timer3Done;
    pISR_TIMER4 = (int)Timer4Done;

    Uart_Printf("\n[ Timer 0,1,2,3,4 Interrupt Test ]\n\n");

    rTCFG0 = rTCFG0 & ~(0xffffff) | 0x000f0f;         //Dead zone=0,Prescaler1=15(0x0f),Prescaler0=15(0x0f)
    rTCFG1  =rTCFG1 & ~(0xffffff) | 0x001233;         //All interrupt,Mux4=1/2,Mux3=1/4,Mux2=1/8,Mux1=1/16,Mux0=1/16

     //Timer input clock frequency = PCLK/(prescaler value+1)/(divider value)
    rTCNTB0 = 0xffff;           //(1/(50MHz/16/16)) * 0xffff (65535) = 0.334s ( 2.994Hz)
    rTCNTB1 = 0xffff;           //(1/(50MHz/16/16)) * 0xffff (65535) = 0.334s ( 2.994Hz)
    rTCNTB2 = 0xffff;           //(1/(50MHz/16/8 )) * 0xffff (65535) = 0.163s ( 6.135Hz)
    rTCNTB3 = 0xffff;           //(1/(50MHz/16/4 )) * 0xffff (65535) = 0.078s (12.820Hz)
    rTCNTB4 = 0xffff;           //(1/(50MHz/16/2 )) * 0xffff (65535) = 0.039s (25.641Hz)
    
     // [22:20] [19:16] [15:12] [11:8] [7:4] [3:0]
     //   110     1010   1010    1010   0000  0010
//    rTCON   = 0x6aaa02; //Auto reload (T0=One-shot), Inverter off, Manual update, Dead zone disable, Stop
  
    rTCON   = rTCON & ~(0xffffff) | 0x6aaa0a;         //Auto reload, Inverter off, Manual update, Dead zone disable, Stop  
    
     // [22:20] [19:16] [15:12] [11:8] [7:4] [3:0]
     //   101     1001   1001    1001   0000  0001      
    rTCON   = rTCON & ~(0xffffff) | 0x599901;         //Auto reload(T0=One-shot),Inverter off,No operation,Dead zone disable,Start
/*
    Uart_Printf("Probing TOUT 0 : J10 -  2\n");
    Uart_Printf("Probing TOUT 1 : U20 -  1\n");
    Uart_Printf("Probing TOUT 2 : U16 - 13\n");
    Uart_Printf("Probing TOUT 3 : U16 - 15\n\n");

    if(PCLK==50000000)
    {
        Uart_Printf("PCLK 50MHz, Timer TOUT 0 : 0.334s ( 2.994Hz)\n");
        Uart_Printf("PCLK 50MHz, Timer TOUT 1 : 0.334s ( 2.994Hz)\n");
        Uart_Printf("PCLK 50MHz, Timer TOUT 2 : 0.163s ( 6.135Hz)\n");
        Uart_Printf("PCLK 50MHz, Timer TOUT 3 : 0.078s (12.820Hz)\n");    
        Uart_Printf("PCLK 50MHz, Timer TOUT 4 : 0.039s (25.641Hz)     <= No Pin Out\n\n");
    }
*/
    while(variable0 == 0);     
    
    Delay(1);                   //To compensate timer error(<1 tick period)
    rTCON   = 0x0;              //One-shot, Inverter off, No operation, Dead zone disable, Stop 
    
    if(variable4==8 && variable3==4 && variable2==2 && variable1==1 && variable0==1)
    {
        Uart_Printf("Timer 0,1,2,3,4 Interrupt Test --> OK\n");
    }        
    else
    {
        Uart_Printf("Timer 0,1,2,3,4 Interrupt Test --> Fail............\n");
    }    
    Uart_Printf("Timer0 - %d (=1),  Timer1 - %d (=1),  Timer2 - %d (=2),  Timer3 - %d (=4),  Timer4 - %d (=8)\n",
        variable0,variable1,variable2,variable3,variable4);        
//    Uart_Printf("Press any key to exit.....\n");      
//    while(!Uart_Getch());       //Key board press check    

//    Uart_Printf("rINTMSK (Before) = 0x%8x\n",rINTMSK);
    rINTMSK |= (BIT_TIMER4 | BIT_TIMER3 | BIT_TIMER2 | BIT_TIMER1 | BIT_TIMER0);
//    Uart_Printf("rINTMSK (After)  = 0x%8x\n",rINTMSK);    
}

//========================================================
// Timer Interrupt Request 
//========================================================
void __irq Timer0Done(void)
{
    rSRCPND = BIT_TIMER0;       //Clear pending bit
    rINTPND = BIT_TIMER0;
    rINTPND;                    //Prevent an double interrupt pending
    variable0++;    
}

//---------------------------------------------------------------------
void __irq Timer1Done(void)
{
    rSRCPND = BIT_TIMER1;       //Clear pending bit
    rINTPND = BIT_TIMER1;
    rINTPND;
    variable1++;    
}

//---------------------------------------------------------------------
void __irq Timer2Done(void)
{
    rSRCPND = BIT_TIMER2;       //Clear pending bit
    rINTPND = BIT_TIMER2;
    rINTPND;
    variable2++;    
}

//---------------------------------------------------------------------
void __irq Timer3Done(void)
{
    rSRCPND = BIT_TIMER3;       //Clear pending bit
    rINTPND = BIT_TIMER3;
    rINTPND;
    variable3++;    
}

//---------------------------------------------------------------------
void __irq Timer4Done(void)
{
    rSRCPND = BIT_TIMER4;       //Clear pending bit
    rINTPND = BIT_TIMER4;
    rINTPND;
    variable4++;    
}

/*
//========================================================
// Timer DMA 0/1/2/3/4 test
//========================================================
void Test_TimerDma(void)
{
    variable0 = 0;variable1 = 0;variable2 = 0;variable3 = 0;variable4 = 0;
            
    rINTMSK = ~(BIT_TIMER4 | BIT_TIMER3 | BIT_TIMER2 | BIT_TIMER1 | BIT_TIMER0);   

    pISR_TIMER0 = (int)Timer0Done;
    pISR_TIMER1 = (int)Timer1Done;
    pISR_TIMER2 = (int)Timer2Done;
    pISR_TIMER3 = (int)Timer3Done;
    pISR_TIMER4 = (int)Timer4Done;

    Uart_Printf("[ Timer 0,1,2,3,4 DMA Test ]\n");

    rTCFG0  = 0x000f0f; //Dead zone=0,Prescaler1=15(0x0f),Prescaler0=15(0x0f)
    rTCFG1  = 0x001233; //All interrupt,Mux4=1/2,Mux3=1/4,Mux2=1/8,Mux1=1/16,Mux0=1/16    
    rTCFG1  = 0x101233; //DMA Mode Timer0,Mux4=1/2,Mux3=1/4,Mux2=1/8,Mux1=1/16,Mux0=1/16
    rTCFG1  = 0x201233; //DMA Mode Timer1,Mux4=1/2,Mux3=1/4,Mux2=1/8,Mux1=1/16,Mux0=1/16
    rTCFG1  = 0x301233; //DMA Mode Timer2,Mux4=1/2,Mux3=1/4,Mux2=1/8,Mux1=1/16,Mux0=1/16
    rTCFG1  = 0x401233; //DMA Mode Timer3,Mux4=1/2,Mux3=1/4,Mux2=1/8,Mux1=1/16,Mux0=1/16
    rTCFG1  = 0x501233; //DMA Mode Timer4,Mux4=1/2,Mux3=1/4,Mux2=1/8,Mux1=1/16,Mux0=1/16    

     //Timer input clock frequency = PCLK/{prescaler value+1}/{divider value}
     //{prescaler value} = 0 ~ 255   , {divider value} = 2,4,8,16
    rTCNTB0 = 0xffff;   //(1/(60MHz/16/16)) * 0xffff (65535) = 0.275s
    rTCNTB1 = 0xffff;   //(1/(60MHz/16/16)) * 0xffff (65535) = 0.275s
    rTCNTB2 = 0xffff;   //(1/(60MHz/16/8 )) * 0xffff (65535) = 0.138s
    rTCNTB3 = 0xffff;   //(1/(60MHz/16/4 )) * 0xffff (65535) = 0.065s
    rTCNTB4 = 0xffff;   //(1/(60MHz/16/2 )) * 0xffff (65535) = 0.033s

     // [22:20] [19:16] [15:12] [11:8] [7:4] [3:0]
     //   110     1010   1010    1010   0000  0010
//    rTCON   = 0x6aaa02; //Auto reload (T0=One-shot), Inverter off, Manual update, Dead zone disable, Stop
    rTCON   = 0x6aaa0a; //Auto reload, Inverter off, Manual update, Dead zone disable, Stop  
    
     // [22:20] [19:16] [15:12] [11:8] [7:4] [3:0]
     //   101     1001   1001    1001   0000  0001      
    rTCON   = 0x599901; //Auto reload(T0=One-shot),Inverter off,No operation,Dead zone disable,Start
    rTCON   = 0x0;      //One-shot, Inverter off, No operation, Dead zone disable, Stop

    while(variable0 == 0);     
    
    Delay(1);           //To compensate timer error(<1 tick period)
    rTCON   = 0x0;      //One-shot, Inverter off, No operation, Dead zone disable, Stop

    if(variable4==8 && variable3==4 && variable2==2 && variable1==1 && variable0==1)
           Uart_Printf("Timer 0,1,2,3,4 DMA Test --> OK\n");
    else
           Uart_Printf("Timer 0,1,2,3,4 DMA Test --> Fail............\n");
    Uart_Printf("Timer0 - %d(1),  Timer1 - %d(1),  Timer2 - %d(2),  Timer3 - %d(4),  Timer4 - %d(8)\n",
        variable0,variable1,variable2,variable3,variable4);

    rINTMSK |= (BIT_TIMER4 | BIT_TIMER3 | BIT_TIMER2 | BIT_TIMER1 | BIT_TIMER0);
}
*/

//================================================
// Watch-dog timer Interrupt Request Test
//================================================
volatile int isWdtInt;
void Test_WDT_IntReq(void)
{
      //t_watchdog = 1 / (PCLK / (Prescaler value + 1 ) / Division_factor)
    Uart_Printf("[ WatchDog Timer Interrupt Request Test ]\n");
    
//    Uart_Printf("rINTMSK (Before)  = 0x%8x\n",rINTMSK);
    rINTMSK  &= ~(BIT_WDT);           //Watch dog Interrupt service is available
//    Uart_Printf("rINTMSK (After )  = 0x%8x      <== INT_WDT : [9]\n",rINTMSK);
        
    pISR_WDT = (unsigned)Wdt_Int;
    isWdtInt = 0;

      //If PCLK = 50MHz
    rWTCON   = ((PCLK/1000000-1)<<8) | (3<<3) | (1<<2); //Prescaler=0x31(49),Clock division 128,Interrupt enable
    rWTDAT   = 8448 / 4;          //Count reload value, 2112(0x840) = 8448(0x2100) / 4(0x4) 
    rWTCNT   = 8448 / 4;
    rWTCON   = rWTCON | (1<<5);   //Watch-dog timer enable
//    Uart_Printf("rWTCON = 0x%4x  <== 0x313c\n",rWTCON);  //0011 0001 0011 1100
    while(isWdtInt != 10);
//    Uart_Printf("\nrWTCON = 0x%4x   <== After 'while(isWdtInt != 10)'\n",rWTCON);
    rWTCON = ((PCLK/1000000-1)<<8) | (3<<3) | (1);  //Prescaler=0x31(49),Clock division 128,Reset enable

//    Uart_Printf("\nrWTCON = 0x%4x  <== 0x3119\n",rWTCON);
     //Prescaler=0x31[15:8],00[7:6],Watch-dog timer disable[5],Division 128[4:3],Interrupt Disable[2],Reserved[1],Reset enable[0]
     //0011 0001 0001 1001
         
    Uart_Printf("\nI will restart after 2 sec.\n");
    rWTCNT = 8448 * 2;          //16896 (0x4200)
    rWTCON = rWTCON | (1<<5);   //1/40/128,Watch-dog timer Interrupt enable
    while(1);
//    Uart_Printf("rINTMSK (Before)  = 0x%8x\n",rINTMSK);
    rINTMSK |= BIT_WDT;         //WDT Interrupt Mask
//    Uart_Printf("rINTMSK (After )  = 0x%8x\n",rINTMSK);    
}

//---------------------------------------------------------------------
void __irq Wdt_Int(void)
{
    rSRCPND = BIT_WDT;  //Clear pending bit
    rINTPND = BIT_WDT;
    rINTPND;            //Prevent an double interrupt pending
    Uart_Printf("%d ",++isWdtInt);
}