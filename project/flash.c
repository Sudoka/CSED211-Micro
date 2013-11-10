//====================================================================
// File Name : flash.c
// Function  : S3C2410 Flash Program
// Program   : Kong, In Wook (KIW)
// Date      : May 30, 2002
// Version   : 0.0
// History
//   0.0 : Programming start (May 30,2002) -> KIW
//         Arrangement source code(8/01/2002)-> SOP 
//====================================================================

#include <string.h>
#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h" 

#include "mmu.h"
#include "am29f800.h"
//#include "strata.h"
#include "strata32.h"

static int DownloadData(void);

#if 1
    // removed because of multiple definition
extern U32 downloadAddress; 
extern U32 downloadProgramSize;
#else
   U32 downloadAddress; 
   U32 downloadProgramSize;
#endif

//==========================================================================================
void *flashType[][2]=
{
    (void *)ProgramAM29F800,     "AM29LV800BB x1      ",
//  (void *)Program28F640J3A,    "28F640J3A x2        ",    
    (void *)Program28F128J3A,    "28F128J3A(16MB) x2  ",    
    0,0
};

//==========================================================================================
void ProgramFlash(void)
{
    int i=0,whichFlash;
    char key;

    Uart_Printf("\n[ NOR Flash Memory Writer Ver 0.1 ]\n\n");
    Uart_Printf("The program buffer : 0x31000000 ~ 0x33ff0000\n");
    
    downloadAddress=0x31000000;
    downloadProgramSize=0x0;

    //MMU_Init();
    ChangeRomCacheStatus(RW_NCNB);
    
    while(1)
    {   //display menu
        Uart_Printf("%c : %s",'a'+i,flashType[i][1]);
        i++;
        if((int)(flashType[i][0])==0)
        {
            Uart_Printf("\n");
            break;
        }
        if((i%4)==0) 
            Uart_Printf("\n");
    }

    Uart_Printf("Select the type of a flash memory ? ");
    whichFlash=Uart_Getch()-'a';
    Uart_Printf("%c\n",(whichFlash+'a'));
    //Uart_Printf("\n");

    if( i<0 || (i>=(sizeof(flashType)/8)) )
        return;

    Uart_Printf("Do you want to download through UART0 from 0x%x? [y/n] : ",downloadAddress);
    key=Uart_Getch();
    Uart_Printf("%c\n",key);
    if(key=='y')
    {
        if(!DownloadData())
            return;
    }

    ( (void (*)(void))(flashType[whichFlash][0]) )();
}

//==========================================================================================
static int DownloadData(void)
{
    int i,tmp;
    U16 checkSum=0,dnCS;
    U32 fileSize=10;
    U8 *downPt;

    downPt=(U8 *)downloadAddress;
   
    Uart_Printf("\ndownloadAddress = %x\n",downloadAddress);

    Uart_Printf("Download the plain binary file(.BHC) to be written\n");
    Uart_Printf("The file format : <n+6>(4)+(n)+CS(2)\n");
    Uart_Printf("To transmit .BIN file : wkocm2 xxx.BIN /1 /d:1\n");
    Uart_Printf("Download methods : COM:8Bit,NP,1STOP\n");
 
    Uart_Printf("\nSTATUS : ");
    rINTMSK=BIT_ALLMSK;
    
    tmp=RdURXH0(); //To remove overrun error state.

    i=0;    
    while(i<fileSize)
    {
        while(!(rUTRSTAT0&0x1));
            *(downPt+i)=RdURXH0();
        if(i==3)
        {
            fileSize=*((U8 *)(downloadAddress+0))+
            (*((U8 *)(downloadAddress+1))<<8)+
            (*((U8 *)(downloadAddress+2))<<16)+
            (*((U8 *)(downloadAddress+3))<<24);
        }
    
        if((i%1000)==0)
            WrUTXH0('#');
        i++;
    }

    downloadProgramSize=fileSize-6;

    for(i=4;i<(fileSize-2);i++)
    {
        checkSum+=*((U8 *)(i+downloadAddress));
    }

    dnCS=*((U8 *)(downloadAddress+fileSize-2))+
          (*( (U8 *)(downloadAddress+fileSize-1) )<<8);

    if(checkSum!=dnCS)
    {
        Uart_Printf("Checksum Error!!! MEM : %x  DN : %x\n",checkSum,dnCS);
        return 0;
    }

    Uart_Printf("\nDownload O.K.\n");
    return 1;
}
