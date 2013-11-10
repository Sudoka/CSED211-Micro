/************************************************ 
  NAME    : CPUSPEED.C
  DESC	  : Analyze where the speed bottleneck is.
  	    1) the code runs only on the cache.
  Revision: 2001.5.17: purnnamu: draft
  Revision: 2003.3.xx: DonGo: modified for 5410.
 ************************************************/
#include <string.h>
#if 0	//5410
#include "5410addr.h"
#include "5410lib.h"
#else	//2410
#include "2410addr.h"
#include "2410lib.h"
#endif
#include "def.h"
#include "option.h"

#include "mmu.h"
#include "cpuspeed.h"

void CpuSpeedFunc1(void);
void CpuSpeedFunc2(void);

#define	LED_DISPLAY(data)	    (rGPFDAT = (rGPFDAT & ~(0xf<<4)) | ((~data & 0xf)<<4))


#define	WHICH_CPU	2410	// or 2410.

#if WHICH_CPU==5410
#define	TEST_STADDR		(0x60f00000)
#else	//2410
#define	TEST_STADDR		(0x30f00000)
#endif
#define	TEST_ENDADDR	(TEST_STADDR+0xff)	//256 bytes



void Test_CpuSpeed(void)
{
    int i,j,base;
    U32 uLockPt,bypass;

	// added for testing 2410.
    Uart_Printf("[CPU Core Speed Test]\n");

	// Set MMU enable and on/off I/D-cache.
	Uart_Printf("[MMU enable]\n");
	MMU_EnableMMU();
    Uart_Printf("[ICache enable]\n");
	MMU_EnableICache();
    Uart_Printf("[DCache enable]\n");
	MMU_EnableDCache(); //DCache should be turned on after MMU is turned on.

    Uart_Printf("[FCLK:HCLK:PCLK] = [%d:%d:%d]MHz\n", FCLK/1000000, HCLK/1000000, PCLK/1000000);
    Uart_Printf("DCache locked area: %xH~%xH\n", TEST_STADDR, TEST_ENDADDR);
    Uart_Printf("ICache locked area: %x~%x(256B boundary)\n",
    	(U32)CpuSpeedFunc1,(U32)CpuSpeedFunc2);
    Uart_Printf("LCD is disabled.\n");
    //LCD_DisplayControl(0);
    rLCDCON1&=~1; // ENVID=OFF
    LED_DISPLAY(0x1);	// LED 1

    //Uart_Printf("Press any key.\n");
    //Uart_Getch();

 	Uart_Printf("Cache lock-down.\n");
	
	
    //========== ICache lock-down ==========
    MMU_SetICacheLockdownBase(10<<26);  	// The following code will be filled between cache line 10~63.
    base=10;
    bypass=1;
    uLockPt=(U32)CpuSpeedFunc1&0xffffffe0;

    for(;uLockPt<(U32)CpuSpeedFunc2;uLockPt+=0x20)
    {
        if(((uLockPt%0x100)==0)&&(uLockPt>(U32)CpuSpeedFunc1)) base++;
        #if WHICH_CPU==5410
			MMU_InvalidateICacheVA(uLockPt);
		#else	// 2410
			MMU_InvalidateICacheMVA(uLockPt);
		#endif
        if(bypass==1) MMU_SetICacheLockdownBase(base<<26);  
		#if WHICH_CPU==5410
	    	MMU_PrefetchICacheVA(uLockPt);
		#else	//2410
			MMU_PrefetchICacheMVA(uLockPt);
		#endif
    	if(bypass==1) //to put the current code outside base 9
    	{
    	    bypass=0;
    	    base=0;
            uLockPt-=0x20; //restore uLockPt
    	}
    }
    base++;
    MMU_SetICacheLockdownBase(base<<26);  // 256

    if(base>10)
    	Uart_Printf("ERROR:ICache lockdown base overflow\n");
    
    Uart_Printf("lockdown ICache line=0~%d\n",base-1);


    //========== DCache lock-down ==========
    base=0;
    uLockPt=(U32)CpuSpeedFunc1&0xffffffe0;

    //Function should be cached in DCache because of the literal pool(LDR Rn,=0xxxxx). ??
    for(;uLockPt<(U32)CpuSpeedFunc2;uLockPt+=0x20)
    {
    	if(((uLockPt%0x100)==0)&&(uLockPt>(U32)CpuSpeedFunc1))
    	    base++;
		#if WHICH_CPU==5410
    	MMU_CleanInvalidateDCacheVA(uLockPt);
		#else	//2410
    	MMU_CleanInvalidateDCacheMVA(uLockPt);
		#endif

        MMU_SetDCacheLockdownBase(base<<26);  
	    *((volatile U32 *)(uLockPt));
    }
    base++;
    MMU_SetDCacheLockdownBase(base<<26);  


    for(i=TEST_STADDR;i<TEST_ENDADDR;i+=4)*((U32 *)i)=0x55555555;
    
    for(i=0;i<0x100;i+=0x20)
    {
		#if WHICH_CPU==5410
      	MMU_CleanInvalidateDCacheVA(TEST_STADDR+i);
		#else	//2410
      	MMU_CleanInvalidateDCacheMVA(TEST_STADDR+i);
		#endif

        MMU_SetDCacheLockdownBase(base<<26);  
        *((volatile U32 *)(TEST_STADDR+i));
    }

    base++;
    MMU_SetDCacheLockdownBase(base<<26);  


   	
    Uart_Printf("lockdown DCache line=0~%d\n",base-1);


    //========== Check the line is really cache-filled ==========
#if 1
    for(uLockPt=(U32)CpuSpeedFunc1;uLockPt<(U32)CpuSpeedFunc2-4*8;uLockPt+=4)
    {
	//*((U32 *)uLockPt)=0xffffffff; //*((U32 *)uLockPt);
	*((U32 *)uLockPt)=*((U32 *)uLockPt);
    }
#endif    	
// SDRAM Self refresh
	
	LED_DISPLAY(0x2);

	//rPWRSAV |= 1<<2;	// SDRAM1 self refresh.
    CpuSpeedFunc1();

}    



void CpuSpeedFunc1(void)
{
    int i,j;
    i=0;
	
	LED_DISPLAY(0x3);

	//The following code should not use the stack memory.
    // because the stack memory is not DCache-locked.
    // It's should be checked using disassembly code.

	// chage refresh count.
    rREFRESH = (rREFRESH & ~0x3ff)  | 200;
    // Set clock frequency.
	ChangeClockDivider(1,1);		// 1:2:4
	//ChangeMPllValue(0xa1,0x3,0x1);	// FCLK=202.8MHz, refersh:473
	//ChangeMPllValue(0x66,0x1,0x1);	// FCLK=220MHz   
//	ChangeMPllValue(0x69,0x1,0x1);	// FCLK=226MHz       
	ChangeMPllValue(0x96,0x2,0x1);	// FCLK=237MHz, refresh:200


	// Set clock out pad.
	#if WHICH_CPU==5410
	//rGPDCON = (rGPDCON & ~(3<<30)) | (2<<30);	// GPD15  = CLKOUT.
	//rMISCCR = (rMISCCR & ~(7<<8)) | (2<<8);		// CLKOUT = FCLK
	#else	//2410
	rGPHCON = (rGPHCON & ~(3<<20)) | (2<<20);	// GPH10  = CLKOUT1.
	rMISCCR = (rMISCCR & ~(7<<8)) | (2<<8);		// CLKOUT1 = FCLK
	#endif
	
	// Start signal, Just for notification.
	#if 0
	rPWRSAV |= 1<<2;	// SDRAM1 self refresh.
	#endif
	
	LED_DISPLAY(0x4);
    for(i=0;i<9000000;i++);
    for(i=0;i<9000000;i++);

    while(1)
    {
        for(i=0;i<0x100;i+=4)
            *(volatile U32 *)(TEST_STADDR+i)=0x12345678*i+i;	// Write data.

        for(i=0;i<0x100;i+=4)
        {
            if(*(volatile U32 *)(TEST_STADDR+i)!=0x12345678*i+i)	// Error
            {
			// Display LED x101 and stop.
	        //rGPDDAT=rGPDDAT&~(0xf<<8)|(0x0<<8);
	        //rGPDDAT=rGPDDAT&~(0xf<<8)|(0x2<<8);
	        LED_DISPLAY(0x5);
	        

	        while(1);
            }
            *(volatile U32 *)(TEST_STADDR+i)=0x0;	// Clear memory.
        }

    //rGPDDAT=rGPDDAT&~(0xf<<8)|(0xf<<8);
    LED_DISPLAY(0xf);
	

        i=0;
        i++;
        i=i*0x12345678;	// i=1.
	if(i==0x12345678)
	    //rGPDDAT=rGPDDAT&~(0xf<<8)|(0x0<<8);	
		LED_DISPLAY(0x0);
	else
	    //rGPDDAT=rGPDDAT&~(0xf<<8)|(0x9<<8);
		LED_DISPLAY(0x3);
       
    }    
}

void CpuSpeedFunc2(void){}



