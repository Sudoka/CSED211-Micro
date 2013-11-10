#include <string.h>
#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"

void WaitRd8(void);
void WaitWr8(void);
void WaitRd32(void);
void WaitWr32(void);
void WaitRd16(void);
void WaitWr16(void);

#define BUS8		(0)
#define BUS16		(1)
#define BUS32		(2)
#define ENWAIT		(1)
#define SRAMBE03	(1)

#define B4_Tacs		(0x0)	//0clk
#define B4_Tcos		(0x0)	//0clk
#define B4_Tacc		(0x2)	//3clk
#define B4_Tcoh		(0x0)	//0clk
#define B4_Tah		(0x0)	//0clk
#define B4_Tacp		(0x0)	//2clk
#define B4_PMC		(0x0)	//no page mode


// Work-around with nWAIT.

void Test_WaitPin(void)
{
    U32 savePLLCON,saveCLKDIVN;

    saveCLKDIVN=rCLKDIVN;
    savePLLCON=rMPLLCON;

    Uart_TxEmpty(0);
    ChangeClockDivider(1,1);	    // 1:2:4 
    ChangeMPllValue(0x5c,0x4,0x1);  //Fin=12MHz FCLK=100MHz for ALTERA
    Uart_Init(25000000,115200);

    rMISCCR=0x330;	//HCLK
    rGPHCON=(rGPHCON & (~(0x3<<18))) | (0x2<<18);   //GPH9=CLKOUT0

    //WaitRd32();
    //WaitWr32();
    //WaitRd16();
    //WaitWr16();
    WaitRd8();
    //WaitWr8();

    Uart_TxEmpty(0);
    rCLKDIVN=saveCLKDIVN;
    rMPLLCON=savePLLCON;
    Uart_Init(0,115200);    
}

void WaitRd8(void)
{
    U8 readVal;
    int i;

    Uart_Printf("nGCS4,8-bit,read,nWAIT Test.\n");

    rBWSCON=rBWSCON & ~(0xf<<16)|(BUS8<<16)|(ENWAIT<<18); //nGCS4 
    rBANKCON4=((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC));

    for(i=0;i<2;i++); //wait until the bank configuration is in effect.

    readVal=(*(volatile U8 *)0x20000000);
    readVal&=0xf;//data[3:0]
    Uart_Printf("read_data=%x\n",readVal);
}


void WaitWr8(void)
{
    U8 writeVal=0xa;//data[3:0]
    int i;

    Uart_Printf("nGCS4,8-bit,write,nWAIT Test.\n");

    rBWSCON=rBWSCON & ~(0xf<<16)|(BUS8<<16)|(ENWAIT<<18); //nGCS4 
    rBANKCON4=((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC));

    for(i=0;i<2;i++); //wait until the bank configuration is in effect.

    *((volatile U8 *)0x20000000)=writeVal; 
}




void WaitRd16(void)
{
    U16 readVal;
    int i;

    Uart_Printf("nGCS4,16-bit,read,nWAIT Test.\n");

    rBWSCON=rBWSCON & ~(0xf<<16)|(BUS16<<16)|(ENWAIT<<18); //nGCS4 
    rBANKCON4=((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC));

    for(i=0;i<2;i++); //wait until the bank configuration is in effect.

    readVal=(*(volatile U16 *)0x20000000);
    readVal&=0xf;//data[3:0]
    Uart_Printf("read_data=%x\n",readVal);
}


void WaitWr16(void)
{
    U32 writeVal=0xa;//data[3:0]
    int i;

    Uart_Printf("nGCS4,16-bit,write,nWAIT Test.\n");

    rBWSCON=rBWSCON & ~(0xf<<16)|(BUS16<<16)|(ENWAIT<<18); //nGCS4 
    rBANKCON4=((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC));

    for(i=0;i<2;i++); //wait until the bank configuration is in effect.

    *((volatile U16 *)0x20000000)=writeVal; 
}




void WaitRd32(void)
{
    U32 readVal;
    int i;

    Uart_Printf("nGCS4,32-bit,read,nWAIT Test.\n");

    rBWSCON=rBWSCON & ~(0xf<<16)|(BUS32<<16)|(ENWAIT<<18); //nGCS4 
    rBANKCON4=((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC));

    for(i=0;i<2;i++); //wait until the bank configuration is in effect.

    readVal=(*(volatile U32 *)0x20000000);
    readVal&=0xf;//data[3:0]
    Uart_Printf("read_data=%x\n",readVal);
}


void WaitWr32(void)
{
    U32 writeVal=0xa;//data[3:0]
    int i;
    
    Uart_Printf("nGCS4,32-bit,write,nWAIT Test.\n");

    rBWSCON=rBWSCON & ~(0xf<<16)|(BUS32<<16)|(ENWAIT<<18); //nGCS4 
    rBANKCON4=((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC));

    for(i=0;i<2;i++); //wait until the bank configuration is in effect.

    *((volatile U32 *)0x20000000)=writeVal; 
}


void Test_XBREQ(void)
{
    U32 saveGPBCON,saveGPBUP;

    saveGPBCON=rGPBCON;
    rGPBCON=0x2800;

    Uart_Printf("Press any key!!\n");
    Uart_Getch();
    
    rGPBCON=saveGPBCON;
}