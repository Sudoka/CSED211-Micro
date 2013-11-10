/*****************************************
  NAME: palette.c
  DESC: LCD palette RAM test 
  HISTORY:
  July 12, 2002 : Shin ,On Pil --> Error message display detailed address position.
 *****************************************/
#include <string.h>
 #include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
//#include "2410slib.h" 
#include "palette.h"

static void MarchCMinusM(int address,int unit,unsigned int pattern,unsigned int mask);
static void _MarchSub1M(unsigned int *pt,int unit,unsigned int pattern,int incdec,unsigned int mask);

int marchError;

void Test_PaletteMemory(void)
{
    Uart_Printf("Palette RAM test by March C-\n");
	
    marchError=0;

    *((U32 *)PALETTE)=0x1234;
    Uart_Printf("data = %x\n",*((U32 *)PALETTE));
    
    //00->01->11<->10

    //Data cache should be turned off.
    
    MarchCMinusM(PALETTE,255,0x33333333,0xffff);
    MarchCMinusM(PALETTE,255,0x55555555,0xffff);    
    MarchCMinusM(PALETTE,255,0x0000ffff,0xffff);
    MarchCMinusM(PALETTE,255,0x00ff00ff,0xffff);
    MarchCMinusM(PALETTE,255,0x0f0f0f0f,0xffff);
    MarchCMinusM(PALETTE,255,0xffffffff,0xffff);    
    Uart_Printf("P-RAM is tested.\n");
}


static void MarchCMinusM(int address,int unit,unsigned int pattern,unsigned int mask)
{
    int i;
    unsigned int *tempPt;
    
    tempPt=(unsigned int *)address;
    
    for(i=0;i<=unit;i++)
    {
//	    Uart_Printf("P= 0x%8x   : ",pattern);
//	    *tempPt++=pattern;
//	    Uart_Printf("A= 0x%8x,   P= 0x%8x\n",tempPt,*tempPt);
	    *tempPt=pattern;
	    tempPt+=1;
    }
    
	Uart_Printf("\nWaitting 1 Sec\n");
	Delay(10000);
	Uart_Printf("Start\n");
    
    _MarchSub1M((unsigned int *)address,unit,~pattern,1,mask);
    _MarchSub1M((unsigned int *)address,unit,pattern,1,mask);
    _MarchSub1M((unsigned int *)address+(unit*1),unit,~pattern,-1,mask);
    _MarchSub1M((unsigned int *)address+(unit*1),unit,pattern,-1,mask); 
    
    tempPt=(unsigned int *)address;
    for(i=0;i<=unit;i++)
    {
    
	    if( (mask & *tempPt)!=(pattern&mask) )
	    {
	        marchError=1;
//	        Uart_Printf("ERROR0 : RA= 0x%8x,    RP= 0x%8x\n",tempPt,*tempPt);
	        Uart_Printf("ERROR0 : Address= 0x%8x,    Write= 0x%8x,    Read= 0x%8x\n",tempPt,pattern,*tempPt);	        
	    }
//	    Uart_Printf("RA= 0x%8x,    RP= 0x%8x\n",tempPt,*tempPt);
	    tempPt+=0x1;	    
    }
    tempPt-=0x1;
//	Uart_Printf("Address= 0x%8x,    Write= 0x%8x,    Read= 0x%8x\n",tempPt,pattern,*tempPt);
}


static void _MarchSub1M(unsigned int *pt,int unit,unsigned int pattern,int incdec,unsigned int mask)
{
    unsigned int i,rp;
    unsigned int *pt2=pt;
    
    for(i=0;i<=unit;i++)
    {
	    rp=*pt;
	    //Uart_Printf("RP=%x",*pt);
	    *pt=pattern;
	    //Uart_Printf("j=%x,p=%x,pt=%x\n",rp,(~pattern),pt);
	
	    if( (mask&rp)!=(mask&(~pattern)) )
	    {
	        marchError=1;
//	        Uart_Printf("ERROR1 : Address= 0x%8x,    Write= 0x%8x,    Read= 0x%8x   pt2= 0x%8x \n",pt,(~pattern),rp,pt2);
	        Uart_Printf("ERROR1 : Address= 0x%8x,    Write= 0x%8x,    Read= 0x%8x\n",pt,(~pattern),rp);	        
	    }
	    pt+=incdec;
//	Uart_Printf(".");
    }
}