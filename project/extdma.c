#include <string.h>
#include "2410addr.h"
#include "2410lib.h"
#include "option.h"
#include "def.h"

void _Dma0XdreqHandshakeUnit8bit(void);	    //SET_XDREQ16()
void _Dma0XdreqHandshakeUnit16bit(void);    //SET_XDREQ16()
void _Dma0XdreqHandshakeUnit32bit(void);    //SET_XDREQ16()
void _Dma0XdreqHandshakeBurst16bit(void);   //SET_XDREQ16()

void _Dma0XdreqDemandUnit16bit(void);	    //SET_XDREQDm()
void _Dma0XdreqDemandBurst16bit(void);	    //SET_XDREQDm()

void _Dma0XdreqWholeUnit16bit(void);	    //SET_XDREQ1()
void _Dma0XdreqWholeBurst16bit(void);	    //SET_XDREQ1()


#define BUS8		(0)
#define BUS16		(1)
#define BUS32		(2)

//GPG9  = DMAMODE0
//GPG10 = DMAMODE1
//GPG8  = nDMASTART

#define SET_XDREQ16()	rGPGDAT=(rGPGDAT&~(3<<9))|(0x1<<9)    //xxx01xxx handshake
#define SET_XDREQ1()	rGPGDAT=(rGPGDAT&~(3<<9))|(0x2<<9)    //xxx10xxx whole
#define START_XDREQ()	{rGPGDAT=(rGPGDAT&~(1<<8))|(0x0<<8);\
			rGPGDAT=(rGPGDAT&~(1<<8))|(0x1<<8);} //xxx1xxx -> xxx0xxx -> xxx1xxx
#define SET_XDREQDm()	rGPGDAT=(rGPGDAT&~(3<<9))|(0x3<<9)    //xxx11xxx demand
//DMAMODE[1:0]:	01b= 16 time requests   
//		10b= one time request
//		11b= demand request

volatile int isDma0Done;

//Verilog code for data bus pins of the EPM7032AE
//assign data[3:0] = (!nGCS && !nOE ) ? wdata:4'bz;


void __irq IsrDma0Done(void)
{
    ClearPending(BIT_DMA0);
    isDma0Done=1;
}


void Test_Dma0Xdreq(void)
{
    U32 savePLLCON,saveCLKDIVN;


    rINTMSK=BIT_ALLMSK;
    pISR_DMA0=(U32)IsrDma0Done;
    rINTMSK=~(BIT_DMA0);

    saveCLKDIVN=rCLKDIVN;
    savePLLCON=rMPLLCON;

    Uart_TxEmpty(0);
    ChangeClockDivider(1,1);	    // 1:2:4 
    ChangeMPllValue(0x5c,0x4,0x1);  //Fin=12MHz FCLK=100MHz
    Uart_Init(25000000,115200);

    rGPBCON=(rGPBCON & (~(0xf<<18))) | (0xa<<18);   //GPB9=nXDACK0, GPB10=nXDREQ0
    //rMISCCR=(rMISCCR & (~(0xf<<4))) | (0x3<<4);   // reset value = nRSTCON bit is high
    rMISCCR=0x330;	//HCLK
    rGPHCON=(rGPHCON & (~(0x3<<18))) | (0x2<<18);   //GPH9=CLKOUT0
    
    rGPGDAT &= (0x9<<8);    // DMAMODE[1:0]=00
    rGPGDAT |= (0x1<<8);    // nDMASTART=1;
    rGPGCON=(rGPGCON & (~(0x3f<<16))) | (0x15<<16);   //GPG8=nDMASTART,GPG9=DMAMODE0,GPG10=DMAMODE1

    Uart_Printf("Test Menu for Dma0 nXDREQ/nXDACK\n");
    Uart_Printf("1: Single/handshake/unit/8bit   src:GCS0(16)  dst:SDRAM(32)\n");
    Uart_Printf("2: Single/handshake/unit/16bit  src:GCS0(16)  dst:SDRAM(32)\n");
    Uart_Printf("3: Single/handshake/unit/32bit  src:GCS0(16)  dst:SDRAM(32)\n");
    Uart_Printf("4: Single/handshake/burst/16bit src:GCS0(16)  dst:SDRAM(32)\n");
    Uart_Printf("5: Whole/handshake/unit/16bit   src:GCS0(16)  dst:SDRAM(32)\n");
    Uart_Printf("6: Whole/handshake/burst/16bit  src:GCS0(16)  dst:SDRAM(32)\n");
//    Uart_Printf("7: Single/Demand/unit/16bit     src:GCS0(16)  dst:SDRAM(32)\n");
//    Uart_Printf("8: Single/Demand/burst/16bit    src:GCS0(16)  dst:SDRAM(32)\n");
    Uart_Printf("Select the item?\n");

    switch(Uart_Getch())
    {
    case '1':
    	_Dma0XdreqHandshakeUnit8bit();
    	break;
    case '2':
    	_Dma0XdreqHandshakeUnit16bit();
    	break;
    case '3':
    	_Dma0XdreqHandshakeUnit32bit();
    	break;
    case '4':
    	_Dma0XdreqHandshakeBurst16bit();
    	break;
    case '5':
    	_Dma0XdreqWholeUnit16bit();
    	break;
    case '6':
    	_Dma0XdreqWholeBurst16bit();
    	break;
/*    case '7':
	_Dma0XdreqDemandUnit16bit();
	break;
    case '8':
	_Dma0XdreqDemandBurst16bit();
	break;*/
    default:
    	break;
    }
    rINTMSK=BIT_ALLMSK;

    Uart_TxEmpty(0);
    rCLKDIVN=saveCLKDIVN;
    rMPLLCON=savePLLCON;
    Uart_Init(0,115200);
}

void _Dma0XdreqHandshakeUnit8bit(void)
{
    int i;
    static U8 *bufDst;
    
    Uart_Printf("[Dma0,Single,handShake,unit,8bit Test,B0->SDRAM]\n");
    //Uart_Printf("NOTE:Ignore bit[3:0]!!\n");//?????

    bufDst=(U8 *)0x31000000;//Non cacheable area

    for(i=0;i<16;i++)
	bufDst[i]=0;
    
    isDma0Done=0;

    rDISRC0=(int)(0x0);	// B0
    rDISRCC0=(0<<1)+(0<<0);	// AHB, inc
    rDIDST0=(U32)(bufDst);	// bufDst
    rDIDSTC0=(0<<1)+(0<<0);	// AHB, inc
    rDCON0=(1<<31)+(1<<30)+(1<<29)+(0<<28)+(0<<27)+(0<<24)+(1<<23)+(1<<22)+(0<<20)+16;
    //handshake, sync HCLK, TC int, single tx, single service, nXDREQ0, H/W request, 
    //auto-reload off, byte, 16
    rDMASKTRIG0=(0<<2)+(1<<1)+0;    //no-stop, DMA2 channel on, no-sw trigger 


    SET_XDREQ16();
    START_XDREQ();
    
    while(isDma0Done==0);
    
    for(i=0;i<16;i++)
	Uart_Printf( "%2x,",*((U8 *)0x0+i) );
    Uart_Printf("\n");
    for(i=0;i<16;i++)
	Uart_Printf("%2x,",bufDst[i]);
}

void _Dma0XdreqHandshakeUnit16bit(void)
{
    int i;
    static U16 *bufDst;

    Uart_Printf("[Dma0,Single,HandShake,unit,16bit Test,B0->SDRAM]\n");

    bufDst=(U16 *)0x31000000;//Non cacheable area

    for(i=0;i<16;i++)//{bufSrc[i]=i;bufDst[i]=0;}
	bufDst[i]=0;
    
    isDma0Done=0;

    rDISRC0=(int)(0x0);		// B0
    rDISRCC0=(0<<1)+(0<<0);	// AHB, inc
    rDIDST0=(U32)(bufDst);	// bufDst
    rDIDSTC0=(0<<1)+(0<<0);	// AHB, inc
    rDCON0=(1<<31)+(1<<30)+(1<<29)+(0<<28)+(0<<27)+(0<<24)+(1<<23)+(1<<22)+(1<<20)+16;
    //handshake, sync HCLK, TC int, single tx, single service, nXDREQ0, H/W request, 
    //auto-reload off, half word, 16hw
    rDMASKTRIG0=(0<<2)+(1<<1)+0;    //no-stop, DMA2 channel on, no-sw trigger 
    SET_XDREQ16();
    START_XDREQ();
    
    while(isDma0Done==0);

    for(i=0;i<16;i++)
	Uart_Printf("%4x,",*((U16 *)0x0+i));
    Uart_Printf("\n");
    for(i=0;i<16;i++)
	Uart_Printf("%4x,",*((U16 *)bufDst+i));
}

void _Dma0XdreqHandshakeUnit32bit(void)
{
    int i;
    static U32 *bufDst;

    Uart_Printf("[Dma0,HandShake,unit,32bit Test,B0->SDRAM]\n");

    bufDst=(U32 *)0x31000000;//Non cacheable area

    for(i=0;i<16;i++)
	bufDst[i]=0;
    
    isDma0Done=0;

    rDISRC0=(int)(0x0);		// B0
    rDISRCC0=(0<<1)+(0<<0);	// AHB, inc
    rDIDST0=(U32)(bufDst);	// bufDst
    rDIDSTC0=(0<<1)+(0<<0);	// AHB, inc
    rDCON0=(1<<31)+(1<<30)+(1<<29)+(0<<28)+(0<<27)+(0<<24)+(1<<23)+(1<<22)+(2<<20)+16;
    //handshake, sync HCLK, TC int, single tx, single service, nXDREQ0, H/W request, 
    //auto-reload off, word, 16w
    rDMASKTRIG0=(0<<2)+(1<<1)+0;    //no-stop, DMA2 channel on, no-sw trigger 
    
    SET_XDREQ16();
    START_XDREQ();
    
    while(isDma0Done==0);    	
    for(i=0;i<16;i++)
    {
    	Uart_Printf("%8x,",*((U32 *)0+i));
    	if(i%8==7)
	    Uart_Printf("\n");
    }
    
    for(i=0;i<16;i++)
    {
    	Uart_Printf("%8x,",bufDst[i]);
    	if(i%8==7)
	    Uart_Printf("\n");
    }
}

void _Dma0XdreqHandshakeBurst16bit(void)
{
    int i;
    static U16 *bufDst;

    Uart_Printf("[Dma0,HandShake,burst,16bit Test,B0->SDRAM]\n");

    bufDst=(U16 *)0x31000000;//Non cacheable area
    for(i=0;i<64;i++)
	bufDst[i]=0;
    
    isDma0Done=0;
    
    rDISRC0=(int)(0x0);		// B0
    rDISRCC0=(0<<1)+(0<<0);	// AHB, inc
    rDIDST0=(U32)(bufDst);	// bufDst
    rDIDSTC0=(0<<1)+(0<<0);	// AHB, inc
    rDCON0=(1<<31)+(1<<30)+(1<<29)+(1<<28)+(0<<27)+(0<<24)+(1<<23)+(1<<22)+(1<<20)+16;
    //handshake, sync HCLK, TC int, burst tx, single service, nXDREQ0, H/W request, 
    //auto-reload off, half-word, 64hw
    rDMASKTRIG0=(0<<2)+(1<<1)+0;    //no-stop, DMA2 channel on, no-sw trigger 

    SET_XDREQ16();
    START_XDREQ();
    
    while(isDma0Done==0);

    for(i=0;i<64;i++)
    {
	Uart_Printf("%4x,",*((U16 *)0x0+i));
	if(i%8==7)
	    Uart_Printf("\n");
    }
    Uart_Printf("\n");
    for(i=0;i<64;i++)
    {
	Uart_Printf("%4x,",*((U16 *)bufDst+i));
	if(i%8==7)Uart_Printf("\n");
    }
}

void _Dma0XdreqWholeUnit16bit(void)
{
    int i;
    static U16 *bufDst;


    Uart_Printf("[Dma0,Whole,HandShake,unit,16bit Test,B0->SDRAM]\n");
    
    bufDst=(U16 *)0x31000000;//Non cacheable area
    for(i=0;i<16;i++)
	bufDst[i]=0;
    
    isDma0Done=0;

    rDISRC0=(int)(0x0);		// B0
    rDISRCC0=(0<<1)+(0<<0);	// AHB, inc
    rDIDST0=(U32)(bufDst);	// bufDst
    rDIDSTC0=(0<<1)+(0<<0);	// AHB, inc
    rDCON0=(1<<31)+(1<<30)+(1<<29)+(0<<28)+(1<<27)+(0<<24)+(1<<23)+(1<<22)+(1<<20)+16;
    //handshake, sync HCLK, TC int, unit tx, whole service, nXDREQ0, H/W request, 
    //auto-reload off, half-word, 16hw
    rDMASKTRIG0=(0<<2)+(1<<1)+0;    //no-stop, DMA2 channel on, no-sw trigger 

    SET_XDREQ1();
    START_XDREQ();
    
    while(isDma0Done==0);

    for(i=0;i<16;i++)
    {
	Uart_Printf("%4x,",*((U16 *)0x0+i));
	if(i%8==7)
	    Uart_Printf("\n");
    }
    Uart_Printf("\n");
    for(i=0;i<16;i++)
    {
	Uart_Printf("%4x,",*((U16 *)bufDst+i));
	if(i%8==7)Uart_Printf("\n");
    }
}



void _Dma0XdreqWholeBurst16bit(void)
{
    int i;
    static U16 *bufDst;

    Uart_Printf("[Dma0,whole,burst,16bit Test,B0->SDRAM]\n");

    bufDst=(U16 *)0x31000000;//Non cacheable area
    for(i=0;i<64;i++)
	bufDst[i]=0;
    
    isDma0Done=0;

    rDISRC0=(int)(0x0);		// B0
    rDISRCC0=(0<<1)+(0<<0);	// AHB, inc
    rDIDST0=(U32)(bufDst);	// bufDst
    rDIDSTC0=(0<<1)+(0<<0);	// AHB, inc
    rDCON0=(1<<31)+(1<<30)+(1<<29)+(1<<28)+(1<<27)+(0<<24)+(1<<23)+(1<<22)+(1<<20)+16;
    //handshake, sync HCLK, TC int, burst tx, whole service, nXDREQ0, H/W request, 
    //auto-reload off, half-word, 64hw
    rDMASKTRIG0=(0<<2)+(1<<1)+0;    //no-stop, DMA2 channel on, no-sw trigger 

    SET_XDREQ1();
    START_XDREQ();
    
    while(isDma0Done==0);
    
    for(i=0;i<64;i++)
    {
	Uart_Printf("%4x,",*((U16 *)0x0+i));
	if(i%8==7)
	    Uart_Printf("\n");
    }
    Uart_Printf("\n");

    for(i=0;i<64;i++)
    {
    	Uart_Printf("%4x,",bufDst[i]);
    	if(i%8==7)Uart_Printf("\n");
    }
}

// Demand test OK!! with another logic
void _Dma0XdreqDemandUnit16bit(void)
{
    int i;
    static U16 *bufDst;

    Uart_Printf("[Dma0,Single,Demand,unit,16bit Test,B0->SDRAM]\n");
    
    bufDst=(U16 *)0x31000000;//Non cacheable area
    for(i=0;i<16;i++)
	bufDst[i]=0;
    
    isDma0Done=0;

    rDISRC0=(int)(0x0);		// B0
    rDISRCC0=(0<<1)+(0<<0);	// AHB, inc
    rDIDST0=(U32)(bufDst);	// bufDst
    rDIDSTC0=(0<<1)+(0<<0);	// AHB, inc
    rDCON0=(0<<31)+(1<<30)+(1<<29)+(0<<28)+(0<<27)+(0<<24)+(1<<23)+(1<<22)+(1<<20)+16;
    //Demand, sync HCLK, TC int, unit tx, single service, nXDREQ0, H/W request, 
    //auto-reload off, half-word, 16hw
    rDMASKTRIG0=(0<<2)+(1<<1)+0;    //no-stop, DMA2 channel on, no-sw trigger 

    SET_XDREQDm();
    START_XDREQ();
    
    while(isDma0Done==0);

    for(i=0;i<16;i++)
    {
	Uart_Printf("%4x,",*((U16 *)0x0+i));
	if(i%8==7)
	    Uart_Printf("\n");
    }
    Uart_Printf("\n");
    for(i=0;i<16;i++)
    {
	Uart_Printf("%4x,",*((U16 *)bufDst+i));
	if(i%8==7)Uart_Printf("\n");
    }
}


void _Dma0XdreqDemandBurst16bit(void)
{
    int i;
    static U16 *bufDst;

    Uart_Printf("[Dma0,whole,burst,16bit Test,B0->SDRAM]\n");

    bufDst=(U16 *)0x31000000;//Non cacheable area
    for(i=0;i<64;i++)
	bufDst[i]=0;
    
    isDma0Done=0;

    rDISRC0=(int)(0x0);		// B0
    rDISRCC0=(0<<1)+(0<<0);	// AHB, inc
    rDIDST0=(U32)(bufDst);	// bufDst
    rDIDSTC0=(0<<1)+(0<<0);	// AHB, inc
    rDCON0=(0<<31)+(1<<30)+(1<<29)+(1<<28)+(0<<27)+(0<<24)+(1<<23)+(1<<22)+(1<<20)+16;
    //Demand, sync HCLK, TC int, burst tx, single service, nXDREQ0, H/W request, 
    //auto-reload off, half-word, 64hw
    rDMASKTRIG0=(0<<2)+(1<<1)+0;    //no-stop, DMA2 channel on, no-sw trigger 

    SET_XDREQDm();
    START_XDREQ();
    
    while(isDma0Done==0);
    
    for(i=0;i<64;i++)
    {
	Uart_Printf("%4x,",*((U16 *)0x0+i));
	if(i%8==7)
	    Uart_Printf("\n");
    }
    Uart_Printf("\n");

    for(i=0;i<64;i++)
    {
    	Uart_Printf("%4x,",bufDst[i]);
    	if(i%8==7)Uart_Printf("\n");
    }
}