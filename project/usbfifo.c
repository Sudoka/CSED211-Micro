//====================================================================
// File Name : usbfifo.c
// Function  : S3C2410 USB FIFO Test
// Program   : Kang, Weon Tark (KWT)
// Date      : July 19, 2002
// Version   : 0.0
// History
//   0.0 : Programming start (July 19, 2002) -> KWT
//====================================================================

#include <string.h>
#include "2410addr.h"
#include "2410lib.h"
#include "def.h"

void MarchCMinus32(int address,int unit,unsigned int pattern);
void _MarchSub1(unsigned int *pt,int unit,unsigned int pattern,int incdec);
int MarchError;

int Test_USBFIFO(void)
{
    int saveSyscfg;
    Uart_Printf("USB FIFO Cell Test by March C-\n");
    
    // Make test mode
    *(int *)0x52000280 = 0x1;
/*
    *(int *)0x52000100=0xffffffff;
    Uart_Printf("0x%x\n",*(int *)0x52000100);
*/
    MarchError=0;

    //00->01->11<->10

    MarchCMinus32(0x52000000,63,0x00000000); 
    MarchCMinus32(0x52000000,63,0x0f0f0f0f);
    MarchCMinus32(0x52000000,63,0x33333333);
    MarchCMinus32(0x52000000,63,0x55555555);
    MarchCMinus32(0x52000000,63,0xaaaaaaaa);

    if(MarchError==0)
    {
        Uart_Printf("USB FIFO is tested... OK!!\n");
        return 1; 
    }
    else
    {
        Uart_Printf("USB FIFO is tested... Fail!!\n");
        return 0;
    }
}

void MarchCMinus32(int address,int unit,unsigned int pattern)
{
    int i;
    unsigned int *tempPt;
    
    tempPt=(unsigned int *)address;
    
    for(i=0;i<=unit;i++)
    {
        //Uart_Printf("P=%x",pattern);
        *tempPt++=pattern;
        //Uart_Printf("A=%x,P=%x\n",tempPt,*tempPt);
    }
    
	Uart_Printf("\nWaitting 1 Sec\n");
	Delay(10000);
	Uart_Printf("Start\n");    
    
    _MarchSub1((unsigned int *)address,unit,~pattern,1);
    _MarchSub1((unsigned int *)address,unit,pattern,1);
    _MarchSub1((unsigned int *)address+unit,unit,~pattern,-1);
    _MarchSub1((unsigned int *)address+unit,unit,pattern,-1); 
    
    tempPt=(unsigned int *)address;
    for(i=0;i<=unit;i++)
    {
        if(*tempPt!=pattern)
        {
            MarchError=1;
//          Uart_Printf("ERROR0:%x\n",tempPt);
            Uart_Printf("ERROR0 : Address= 0x%8x,    Write= 0x%8x,    Read= 0x%8x\n",tempPt,pattern,*tempPt);           
        }
        tempPt++;
    }
    //Uart_Printf("RP=%x,RA=%x\n",*tempPt,tempPt);
}


void _MarchSub1(unsigned int *pt,int unit,unsigned int pattern,int incdec)
{
    unsigned int i,rp;

    for(i=0;i<=unit;i++)
    {
        rp=*pt;
        //Uart_Printf("RP=%x",*pt);
        *pt=pattern;
        //Uart_Printf("j=%x,p=%x,pt=%x\n",rp,(~pattern),pt);
        if(rp!=(~pattern))
        {
            MarchError=1;
//          Uart_Printf("ERROR1:A=%x,RP=%x,WP=%x \n",pt,rp,(~pattern));
            Uart_Printf("ERROR1 : Address= 0x%8x,    Write= 0x%8x,    Read= 0x%8x\n",pt,(~pattern),rp);         
        }
        pt+=incdec;
    }
}