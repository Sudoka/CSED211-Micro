/*****************************************
  NAME: stone.c
  DESC: NAND Internal RAM test (4KByte)
  HISTORY:
  2002.03.07: ver 0.0
  July 12, 2002 : Shin, On Pil
 *****************************************/

#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "stone.h"


void Test_ISram(void)
{
    U32 i, j, error=0;
    U32 *pStepSram=(void *)0x40000000;

    Uart_Printf("[Internal Stepping Stone test (4Kbyte)]\n");
    Uart_Printf("\nTest Pattern : 0x0 , 0x55555555 , 0xaaaaaaaa , 0xffffffff\n\n");    
    for(i=0;i<0x1000/4;i++)
	*(pStepSram+i)=0x0;
	
	Uart_Printf("Waitting 1 Sec \n");
	Delay(10000);
	Uart_Printf("Read Start\n");

    for(i=0;i<0x1000/4;i++)
    {
	    j=*(pStepSram+i);
	    if(j!=0x0)
	    {
	        Uart_Printf("Error --> Address= 0x%8x  , Write= 0x0000000  , Read= 0x%8x\n",(pStepSram+i),j);
	        error++;
	    }
//	    Uart_Printf("0");
    }
    
//---------------------------------------------------------------------------------------------    
    for(i=0;i<0x1000/4;i++)
	*(pStepSram+i)=0x55555555;

	Uart_Printf("\nWaitting 1 Sec \n");
	Delay(10000);
	Uart_Printf("Read Start\n");

    for(i=0;i<0x1000/4;i++)
    {
	    j=*(pStepSram+i);
	    if(j!=0x55555555)
	    {
	        Uart_Printf("Error --> Address= 0x%8x  , Write= 0x55555555  , Read= 0x%8x\n",(pStepSram+i),j);
	        error++;
	    }
//	    Uart_Printf("5");	    
    }
     
//---------------------------------------------------------------------------------------------
    for(i=0;i<0x1000/4;i++)
	*(pStepSram+i)=0xaaaaaaaa;

	Uart_Printf("\nWaitting 1 Sec \n");
	Delay(10000);
	Uart_Printf("Read Start\n");

    for(i=0;i<0x1000/4;i++)
    {
	    j=*(pStepSram+i);
	    if(j!=0xaaaaaaaa)
	    {
	        Uart_Printf("Error --> Address= 0x%8x  , Write= 0xAAAAAAAA  , Read= 0x%8x\n",(pStepSram+i),j);
	        error++;
	    }
//	    Uart_Printf("a");	    
    }

//---------------------------------------------------------------------------------------------
    for(i=0;i<0x1000/4;i++)
	*(pStepSram+i)=0xffffffff;

	Uart_Printf("\nWaitting 1 Sec \n");
	Delay(10000);
	Uart_Printf("Read Start\n");

    for(i=0;i<0x1000/4;i++)
    {
	    j=*(pStepSram+i);
	    if(j!=0xffffffff)
	    {
	        Uart_Printf("Error --> Address= 0x%8x  , Write= 0xFFFFFFFF  , Read= 0x%8x\n",(pStepSram+i),j);
	        error++;
	    }
//	    Uart_Printf("f");
    }

    if(error)
	    Uart_Printf("\nFailed Total Count : %3d\n",error);   
    else
	    Uart_Printf("\nInternal Stepping Stone is OK.\n");
}