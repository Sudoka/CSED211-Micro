/**********************************************
  NAME: etc.c
  DESC: miscellaneous tests
  HISTORY:
  04.30.2002:purnnamu: first release for 2410
 **********************************************/
 
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h"

#include "mmu.h"

#include "lcdlib.h"
volatile int a0,a1,a2,intDone;

void __irq EtcIsr_Uart1(void)
{
    a0=rINTOFFSET;
    rSUBSRCPND=BIT_SUB_TXD1;
    ClearPending(BIT_UART1);
    rUTXH1='#';
    a1=rINTOFFSET;
    rINTPND=BIT_UART1;
    a2=rINTOFFSET;
    intDone=1;
}

void Test_NecInterrupt(void)
{
	Uart_Printf("NEC interrupt test.\n");

	intDone=0;
	rINTMSK=BIT_ALLMSK;
	pISR_UART1=(unsigned)EtcIsr_Uart1;
	rINTSUBMSK=rINTSUBMSK&~(BIT_SUB_TXD1);
	rINTMSK=rINTMSK&~(BIT_UART1);
	while(intDone==0);
	rINTMSK=BIT_ALLMSK;
	
	Uart_Printf("rINTOFFSET #1=%x\n",a0);
	Uart_Printf("rINTOFFSET #2=%x\n",a1);
	Uart_Printf("rINTOFFSET #3=%x\n",a2);
}


