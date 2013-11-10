/*****************************************
  NAME: cache.c
  DESC: CACHE test
  HISTORY:
  2002.03.07: ver 0.0
  2003.03.31: added Test_Cache()
 *****************************************/

#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h" 
#include "mmu.h"

static void ICache_InitMMU(void);
void MoveSDRAM2ICache(U32 addr);

void Test_ICache(void)
{
    int i;
    U32 *pCacheableData=(void *)0x30200000;
    U32 *pt; 

    ICache_InitMMU();

     
    for(i=0;i<0x4000/4;i++)
	*(pCacheableData+i)=i;

    MoveSDRAM2ICache((U32)pCacheableData);

    *(pCacheableData+0)=0x55555555;
    *(pCacheableData+0)=0xaaaaaaaa;

    for(i=0;i<0x4000/4;i++)
    {
    	Uart_Printf("%8x  ",*(pCacheableData+i));
    }
}       


void MoveSDRAM2ICache(U32 addr)
{
    int i,j,pt;
    MMU_DisableICache();
    MMU_InvalidateICache();
    //MMU_EnableICache();
    pt=addr;
    for(i=0;i<64;i++)
    	for(j=0;j<8;j++)
    	{
    	    MMU_SetICacheLockdownBase((U32)i<<26);
    	    MMU_PrefetchICacheMVA(pt); 
	    pt+=32;
    	}
    MMU_EnableICache();
}    	

/********************************************************************
  0x31000000~0x310fffff will be non-cachable area for the ICache test.
 ********************************************************************/
static void ICache_InitMMU(void)
{
    int i,j;
    
    MMU_DisableDCache();
    MMU_DisableICache();

    //If write-back is used,the DCache should be cleared.
    for(i=0;i<64;i++)
    	for(j=0;j<8;j++)
    	    MMU_CleanInvalidateDCacheIndex((i<<26)|(j<<5));
    MMU_InvalidateICache();
    
    #if 0
    //To complete MMU_Init() fast, Icache may be turned on here.
    MMU_EnableICache(); 
    #endif
    
    MMU_DisableMMU();
    MMU_InvalidateTLB();

    MMU_SetMTT(0x00000000,0x01f00000,0x00000000,RW_CNB);  //bank0
    MMU_SetMTT(0x08000000,0x03f00000,0x08000000,RW_CNB);  //bank1
    MMU_SetMTT(0x10000000,0x05f00000,0x10000000,RW_NCNB); //bank2
    MMU_SetMTT(0x18000000,0x07f00000,0x18000000,RW_NCNB); //bank3
    MMU_SetMTT(0x20000000,0x09f00000,0x20000000,RW_NCNB); //bank4
    MMU_SetMTT(0x28000000,0x0bf00000,0x28000000,RW_NCNB); //bank5
    
    MMU_SetMTT(0x30000000,0x0c000000,0x30000000,RW_NCNB); //bank6-1
    MMU_SetMTT(0x31000000,0x0cf00000,0x31000000,RW_CB);	  //bank6-1
    MMU_SetMTT(0x33ff0000,0x0df00000,0x33ff0000,RW_NCNB); //bank6-2

    MMU_SetMTT(0x38000000,0x0ff00000,0x38000000,RW_NCNB); //bank7

    MMU_SetMTT(0x10000000,0x13f00000,0x10000000,RW_FAULT);//not used
    MMU_SetMTT(0x48000000,0x5af00000,0x48000000,RW_NCNB); //SFR
    MMU_SetMTT(0x5b000000,0xfff00000,0x5b000000,RW_FAULT);//not used

    MMU_SetTTBase(_MMUTT_STARTADDRESS);
    MMU_SetDomain(0x55555550|DOMAIN1_ATTR|DOMAIN0_ATTR); 
    	//DOMAIN1: no_access, DOMAIN0,2~15=client(AP is checked)
    MMU_SetProcessId(0x0);
    MMU_EnableAlignFault();
    
    MMU_EnableMMU();
    MMU_EnableICache();
    MMU_EnableDCache(); //DCache should be turned on after MMU is turned on.
}    


void Test_Cache(void)
{
   int temp=0, temp1=0, temp2=0, temp3=0, i, index, segment, word;
     MMU_DisableMMU();
     MMU_DisableICache();
     MMU_DisableDCache();
  for(i=0; i<3; i++){
 
   temp=DCache_Test();
   if (temp)
  	 Uart_Printf("DCache O.K.\n");
   else  {
	Uart_Printf("DCache FAIL\n");
       index=*(unsigned int *)(0x31000000);
	segment=*(unsigned int *)(0x31000004);
	word=*(unsigned int *)(0x31000008);
       Uart_Printf("Index: %d           Segment: %d          Word: %d\n", index, segment, word);
   }
   temp1=ICache_Test();
   if (temp1)
	Uart_Printf("ICache O.K.\n");
   else	{
   	Uart_Printf("ICache FAIL\n");
         index=*(unsigned int *)(0x31000000);
	segment=*(unsigned int *)(0x31000004);
	word=*(unsigned int *)(0x31000008);
       Uart_Printf("Index: %d           Segment: %d          Word: %d\n", index, segment, word);
   	}
   
   temp2=DMMU_Test();
   if (temp2)
	Uart_Printf("DMMU Test O.K.\n");
   else	{
   	Uart_Printf("DMMU Test FAIL\n");
         index=*(unsigned int *)(0x31000000);
	  Uart_Printf("DMMU Index: %d\n", index);
   	}

    temp3=IMMU_Test();
   if (temp3)
	Uart_Printf("IMMU Test O.K.\n");
   else	{
   	Uart_Printf("IMMU Test FAIL\n");
         index=*(unsigned int *)(0x31000000);
	  Uart_Printf("IMMU Index: %d\n", index);
   	}
  	} 
   while(1);
}
