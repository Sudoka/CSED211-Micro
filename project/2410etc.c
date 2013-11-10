//====================================================================
// File Name : 2410etc.c
// Function  : S3C2410 Read Page Mode, Nonaligned Test Program
// Program   : Shin, On Pil (SOP)
// Date      : March 20, 2002
// Version   : 0.0
// History
//   0.0 : Programming start (March 14, 2002) -> SOP
//====================================================================

#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h" 

void __irq PseudoRefresh(void);

//***************[ SdramPseudoRefresh ]**************************************
void SdramPseudoRefresh(void)
{
    rWTCON  = ((PCLK/1000000-1)<<8) | (2<<3) | (1<<2);
        //PCLK/1M,Watch-dog disable,1/64,Interrupt enable,Reset disable
    rWTDAT  = 64000/64;              //64ms
    rWTCNT  = 64000/64;              //64ms
    rWTCON |= (1<<5);                //Watch-dog timer start

    pISR_WDT = (U32)PseudoRefresh;
    rINTMSK  = rINTMSK  & ~(BIT_WDT);
    rREFRESH = rREFRESH & ~(1<<23); 
        //Disable SDRAM refresh. SDRAM refresh can be done by S/W.
}

//----------------------------------------------------------------------------
void __irq PseudoRefresh(void)
{
      //This routine is executed every 64ms.
    volatile U32 *pt=(volatile U32 *)0x30000000;
    int i;
    ClearPending(BIT_WDT);

    for(i=0;i<4096*4;i++)
    {
        *pt;
        pt += 512;
    }
    Uart_Printf(".");
}

//********************[ WriteMemory ]**************************************
void WriteMemory(void)
{
    int i, *pt;

    Uart_Printf("[ SDRAM Write Test(0x31000000~0x31bfffff, 12MBytes) ]\n");
    pt=(int *)0x31000000;
    for(i=0;i<0xc00000;i+=4)
    {
        *pt ++= i;
        if((i%0x10000)==0)
            Uart_Printf(".");
    }
    Uart_Printf("\nWriting is completed.\n");
}

//********************[ ReadMemory ]***************************************
void ReadMemory(void)
{
    int i,j,err = 0;
    int *pt;
    
    Uart_Printf("[ SDRAM Read Test(0x31000000~0x31bfffff, 12MBytes) ]\n");
    pt = (int *)0x31000000;
    for(i=0;i<0xc00000;i+=4)
    {
        j = *pt;
        if(j!=i)
        {
            Uart_Printf("%x = %x\n",(U32)pt,j);
            err++;
        }
        pt++;
        if((i%0x10000)==0)   
            Uart_Printf(".");
    }
    if(err==0)
        Uart_Printf("\nMemory read test : O.K.\n");
    else
        Uart_Printf("\nMemory read test : FAIL!!!\n");
}

//------------------------------------------------------------------------
#define BUS8            (0)
#define BUS16           (1)
#define BUS32           (2)
#define ENWAIT          (1)
#define SRAMBE03        (1)

#define B0_Tacs         (0x0)      // 0clk
#define B0_Tcos         (0x0)      // 0clk
#define B0_Tacc         (0x6)      //10clk
#define B0_Tcoh         (0x0)      // 0clk
#define B0_Tah          (0x0)      // 0clk
#define B0_Tacp         (0x3)      // 6clk
#define B0_PMC          (0x1)      //4 data page mode
//#define B0_PMC          (0x0)      //No page mode


//******************[ ReadPageMode ]**************************************
void ReadPageMode(void)
{
    int i, a[16];
    
    rGPHCON   = rGPHCON & ~(3<<18) | (2<<18);   //GPH9 = CLKOUT0
    rMISCCR   = rMISCCR & ~(7<<4)  | (3<<4);    //CLKOUT0=HCLK
    rBANKCON0 = (B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC);

    for(i=0;i<16;i+=4)
        a[i] = *((volatile U32 *)(0x0+i));

    for(i=0;i<16;i+=4)
        Uart_Printf("%8x ,",a[i]);
}

//******************[ Test_NonalignedAccess ]***************************
void Test_NonalignedAccess(void)
{
    __packed int *p;
    MMU_DisableAlignFault();
    p  = (__packed int *)0x31200001;
    *p = 0x12345678;
    Uart_Printf("Byte : %x, %x, %x, %x\n",*((char *)0x31200001),
        *((char *)0x31200002),*((char *)0x31200003),*((char *)0x31200004));
    Uart_Printf("Word : %x\n",*p);
}