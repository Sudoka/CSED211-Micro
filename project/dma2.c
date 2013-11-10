/*****************************************
  NAME: dma.c
  DESC: DMA memory2memory test
  HISTORY:
  2001.03.31:purnnamu: draft ver 0.0
 *****************************************/

#include <string.h>
#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h" 

static void __irq Dma0Done(void);
static void __irq Dma1Done(void);
static void __irq Dma2Done(void);
static void __irq Dma3Done(void);

void DMA0123_M2M(int ch,int srcAddr,int dstAddr,int tc,int dsz,int burst);

typedef struct tagDMA
{
    volatile U32 DISRC;	    //0x0
    volatile U32 DISRCC;    //0x4
    volatile U32 DIDST;	    //0x8
    volatile U32 DIDSTC;    //0xc
    volatile U32 DCON;	    //0x10
    volatile U32 DSTAT;	    //0x14
    volatile U32 DCSRC;	    //0x18
    volatile U32 DCDST;	    //0x1c
    volatile U32 DMASKTRIG; //0x20
}DMA;


static volatile int dmaDone;

void Test_DMAWorst(void)
{
    int i;

    Uart_Printf("[DMA0123 MEM2MEM Worst Test]\n");
    dmaDone=0;

    for(i=_NONCACHE_STARTADDRESS;i<_NONCACHE_STARTADDRESS+0x800000;i+=4)
    {
    	*((U32 *)i)=i^0x55aa5aa5;
    }

    //Start DMA ch0,1,2,3 simultaneously.
    DMA0123_M2M(0,_NONCACHE_STARTADDRESS,_NONCACHE_STARTADDRESS+0x800000,0xfffff,0,0); //byte,single
}




void DMA0123_M2M(int ch,int srcAddr,int dstAddr,int tc,int dsz,int burst)
{
    int i,time;
    volatile U32 memSum0=0,memSum1=0;
    DMA *pDMA;
    int length;
    
    length=tc*(burst ? 4:1)*((dsz==0)+(dsz==1)*2+(dsz==2)*4);

    switch(ch)
    {
    case 0:
    	pISR_DMA0=(int)Dma0Done;
    	rINTMSK&=~(BIT_DMA0);  
    	pDMA=(void *)0x4b000000;
    	break;
    case 1:
    	pISR_DMA1=(int)Dma1Done;
    	rINTMSK&=~(BIT_DMA1);  
    	pDMA=(void *)0x4b000040;
    	break;
    case 2:
	pISR_DMA2=(int)Dma2Done;
    	rINTMSK&=~(BIT_DMA2);  
    	pDMA=(void *)0x4b000080;
	break;
    case 3:
        pISR_DMA3=(int)Dma3Done;
       	rINTMSK&=~(BIT_DMA3);  
       	pDMA=(void *)0x4b0000c0;
        break;
    }
                                                                                                                            
    pDMA->DISRC=srcAddr;
    pDMA->DISRCC=(0<<1)|(0<<0); // inc,AHB
    pDMA->DIDST=dstAddr;
    pDMA->DIDSTC=(0<<1)|(0<<0); // inc,AHB
    pDMA->DCON=tc|(1<<31)|(1<<30)|(1<<29)|(burst<<28)|(1<<27)|\
    	        (0<<23)|(1<<22)|(dsz<<20)|(tc);
    		//HS,AHB,TC interrupt,whole, SW request mode,relaod off
    //pDMA->DMASKTRIG=(1<<1)|1; //DMA on, SW_TRIG

    Uart_Printf(".");

    switch(ch)
    {
    case 0:
        //DMA Ch 1
	DMA0123_M2M(1,_NONCACHE_STARTADDRESS+0x200000,_NONCACHE_STARTADDRESS+0xa00000,0xfffff,1,0); //halfword,single
	break;
    case 1:
    	//DMA Ch 2
    	DMA0123_M2M(2,_NONCACHE_STARTADDRESS+0x400000,_NONCACHE_STARTADDRESS+0xc00000,0x80000,2,0); //word,single
        break;
    case 2:
        //DMA Ch 3
        DMA0123_M2M(3,_NONCACHE_STARTADDRESS+0x600000,_NONCACHE_STARTADDRESS+0xe00000,0x20000,2,1); //word,burst
        break;
    case 3:
    	rDMASKTRIG0=(1<<1)|1;
    	rDMASKTRIG1=(1<<1)|1;
    	rDMASKTRIG2=(1<<1)|1;
    	rDMASKTRIG3=(1<<1)|1;
    	break;	
    }
    
    while(dmaDone!=0xf);
  
    rINTMSK=BIT_ALLMSK;

    for(i=srcAddr;i<(srcAddr+length);i+=4)
    {
    	memSum0+=i^0x55aa5aa5;
    }
    for(i=dstAddr;i<dstAddr+length;i+=4)
    {
    	memSum1+=*((U32 *)i)=i^0x55aa5aa5;
    }

    Uart_Printf("\nDMA%d %8xh->%8xh,size=%xh(tc=%xh),dsz=%d,burst=%d\n",ch,
    		srcAddr,dstAddr,length,tc,dsz,burst);
    Uart_Printf("memSum0=%x,memSum1=%x\n",memSum0,memSum1);
    if(memSum0==memSum1)
    	Uart_Printf("DMA test result--------------------------------------O.K.\n");
    else 
    	Uart_Printf("DMA test result--------------------------------------ERROR!!!\n");

}


static void __irq Dma0Done(void)
{
    ClearPending(BIT_DMA0);
    dmaDone+=1;
}

static void __irq Dma1Done(void)
{
    ClearPending(BIT_DMA1);
    dmaDone+=2;
}

static void __irq Dma2Done(void)
{
    ClearPending(BIT_DMA2);
    dmaDone+=4;
}

static void __irq Dma3Done(void)
{
    ClearPending(BIT_DMA3);
    dmaDone+=8;
}

