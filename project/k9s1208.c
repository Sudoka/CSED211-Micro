//====================================================================
// File Name : k9s1208.c
// Function  : SmartMedia SAMSUNG 64MB (K9S1208) Test Program
// Program   : Weon-Tark Kang (Tark)
// Modify    : On-Pil Shin (SOP)
// Date      : October 23, 2002
// Version   : 0.0
// History
//   0.0 : Programming start -> Tark
//       (20030324) : Added K9S1208 Erase -> SOP
//====================================================================

#include <string.h>
#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h" 

#define BAD_CHECK   (1)
#define ECC_CHECK   (0)

//*************** H/W dependent functions ***************
void __RdPage528(U8 *pPage);
void __WrPage528(U8 *pPage);

static U16 NF_CheckId(void);
static int NF_EraseBlock(U32 blockNum);
static int NF_ReadPage(U32 block,U32 page,U8 *buffer);
static int NF_WritePage(U32 block,U32 page,U8 *buffer);
    //buffer size is 512 bytes
static int NF_IsBadBlock(U32 block);
static int NF_MarkBadBlock(U32 block);
static void NF_Reset(void);
static void NF_Init(void);
//*******************************************************

int DownloadData(void);

static void InputTargetBlock(void);

static U32 srcAddress;
static U32 targetBlock;     // Block number (0 ~ 4095)
static U32 targetSize;      // Total byte size 

U32 downloadAddress;
U32 downloadProgramSize=0x0;

void K9S1208_Program(void)
{
//    unsigned long interrupt_reservoir;
    int i;
    int programError=0;
    U8 *srcPt,*saveSrcPt;
    U32 blockIndex;

    downloadAddress=0x30100000;
    Uart_Printf("\n[SMC(K9S1208V0M) NAND Flash writing program]\n");
    //Uart_Printf("The program buffer: 0x30100000~0x31ffffff\n");

//tark
#if 0
    if(!DownloadData())
    {
        Uart_Printf("Download Fail\n");
        return;
    }
#endif

    NF_Init();


    rINTMSK = BIT_ALLMSK;   
    targetSize=downloadProgramSize;
    if(targetSize==0)
    {
        srcAddress=0x30100000; 
        Uart_Printf("the data must be downloaded using ICE from %xh\n",srcAddress);         
    }
    else 
        srcAddress=downloadAddress+4; //to discard the data head for the size
        
    InputTargetBlock();

    Uart_Printf("source base address(0x3010000x)=0x%x\n",srcAddress);
    Uart_Printf("target start block number      =%d\n",targetBlock);
    Uart_Printf("target size        (0x4000*n)  =0x%x\n",targetSize);

    srcPt=(U8 *)srcAddress;
    blockIndex=targetBlock;
    while(1)
    {
        saveSrcPt=srcPt;    

        #if BAD_CHECK       
            if(NF_IsBadBlock(blockIndex))   // 1:bad 0:good
            {
                blockIndex++;   // for next block
                continue;
            }
        #endif
        
        if(!NF_EraseBlock(blockIndex))
        {
            blockIndex++;   // for next block
            continue;
        }
        
        for(i=0;i<32;i++)
        {
            if(!NF_WritePage(blockIndex,i,srcPt))// block num, page num, buffer
            {
                programError=1;
                break;
            }
            #if ECC_CHECK    
                if(!NF_ReadPage(blockIndex,i,srcPt))
                {
                    Uart_Printf("ECC Error(block=%d,page=%d!!!\n",blockIndex,i);
                }
            #endif      
            srcPt+=512; // Increase buffer addr one pase size
            Uart_Printf(".");
            if((U32)srcPt>=(srcAddress+targetSize)) // Check end of buffer
                break;  // Exit for loop
        }
        if(programError==1)
        {
            blockIndex++;
            srcPt=saveSrcPt;
            programError=0;
            continue;
        }
        if((U32)srcPt>=(srcAddress+targetSize))
            break;  // Exit while loop
        blockIndex++;
    }
}

//==================================================================================
void K9S1208_Erase(void)
{
    int i;
    U32 blockIndex=0;

    Uart_Printf("\n[ SMC(K9S1208V0M) NAND Flash Erase Program ]\n");
    Uart_Printf("\nNow NAND Flash Erasing ....\n");    

    NF_Init();

    rINTMSK = BIT_ALLMSK;   

    for(blockIndex=0;blockIndex<4096;blockIndex++)
    {
        #if BAD_CHECK       
            if(NF_IsBadBlock(blockIndex))   // 1:bad 0:good
            {
                blockIndex++;   // for next block
                continue;
            }
        #endif
        
        if(!NF_EraseBlock(blockIndex))
        {
            blockIndex++;   // for next block
            continue;
        }       
    }
    Uart_Printf("\nSMC(K9S1208V0M) NAND Flash Erase Completed !!!\n");    
}


//==================================================================================

static void InputTargetBlock(void)
{
    Uart_Printf("\nSource size:0h~%xh\n",downloadProgramSize);
    Uart_Printf("\nAvailable target block number: 0~4095\n");
    Uart_Printf("Input target block number:");
    targetBlock=Uart_GetIntNum();   // Block number(0~4095)
    if(targetSize==0)
    {
        Uart_Printf("Input target size(0x4000*n):");
        targetSize=Uart_GetIntNum();    // Total byte size
    }
}


void K9S1208_PrintBadBlockNum(void)
{
    int i;
    U16 id;

    Uart_Printf("\n[SMC(K9S1208V0M) NAND Flash bad block check]\n");
    
    NF_Init();
    id=NF_CheckId();
    Uart_Printf("ID=%x(0xec76)\n",id);
    if(id!=0xec76)
    return;
    
    for(i=0;i<4096;i++)
    {
    NF_IsBadBlock(i);   // Print bad block
    }
}


void K9S1208_PrintBlock(void)// Printf one page
{
    int i,j;
    U16 id;
    U32 block,page;
    U8  buffer[512];

    Uart_Printf("\n[SMC(K9S1208V0M) NAND Flash block read]\n"); 
    
    NF_Init();
    id=NF_CheckId();
    Uart_Printf("ID=%x(0xec76)\n",id);
    if(id!=0xec76)
    return;

    Uart_Printf("Input target block number :");
    block=Uart_GetIntNum();
    Uart_Printf("Input target page number :");   
    page=Uart_GetIntNum();

    NF_ReadPage(block,page,buffer);
    Uart_Printf("block=%d,page=%d:",block,page);
    for(j=0;j<512;j++)
    {
        if(j%16==0)
        Uart_Printf("\n%3xh:",j);
        Uart_Printf("%02x ",buffer[j]);
    }
    Uart_Printf("\n");      
}


//*************************************************
//*************************************************
//**           H/W dependent functions           **
//************************************************* 
//*************************************************

//The code is made for bi-endian mode

// block0: reserved for boot strap
// block1~4095: used for OS image
// badblock SE: xx xx xx xx xx 00 ....
// good block SE: ECC0 ECC1 ECC2 FF FF FF ....

#define WRITEVERIFY  (0)  //verifing is enable at writing.

/*
#define FC_CMD()    {rPDATA|=CLE;rPDATA&=~(ALE|CE);}
#define FC_ADDR()   {rPDATA|=ALE;rPDATA&=~(CLE|CE);}
#define FC_DATA()   {rPDATA&=~(ALE|CLE|CE);}
#define FC_INACTIVE() {rPDATA|=CE;rPDATA&=~(ALE|CLE);}
*/

#define NF_CMD(cmd) 	{rNFCMD=cmd;}
#define NF_ADDR(addr)   {rNFADDR=addr;} 
#define NF_nFCE_L() 	{rNFCONF&=~(1<<11);}
#define NF_nFCE_H() 	{rNFCONF|=(1<<11);}
#define NF_RSTECC() 	{rNFCONF|=(1<<12);}
#define NF_RDDATA()     (rNFDATA)
#define NF_WRDATA(data) {rNFDATA=data;}

#define NF_WAITRB()    {while(!(rNFSTAT&(1<<0)));} 
        //wait tWB and check F_RNB pin.   

#define ID_K9S1208V0M   0xec76

#if 1
// HCLK=100Mhz
#define TACLS       0  //1clk(0ns) 
#define TWRPH0      3  //3clk(25ns)
#define TWRPH1      0  //1clk(10ns)  //TACLS+TWRPH0+TWRPH1>=50ns
#else
// HCLK=50Mhz
#define TACLS       0  //1clk(0ns)
#define TWRPH0      1  //2clk(25ns)
#define TWRPH1      0  //1clk(10ns)
#endif


static U8 seBuf[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

// 1block=(512+16)bytes x 32pages
// 4096block

// A[23:14][13:9]
//  block   page

static int NF_EraseBlock(U32 block)
{
    U32 blockPage=(block<<5);
    int i;

#if BAD_CHECK
    if(NF_IsBadBlock(block))
    	return 0;
#endif

    NF_nFCE_L();
    
    NF_CMD(0x60);   // Erase one block 1st command

    NF_ADDR(blockPage&0xff);        // Page number=0
    NF_ADDR((blockPage>>8)&0xff);   
    NF_ADDR((blockPage>>16)&0xff);

    NF_CMD(0xd0);   // Erase one blcok 2nd command
    
   for(i=0;i<10;i++); //wait tWB(100ns)//??????

    NF_WAITRB();    // Wait tBERS max 3ms.
    NF_CMD(0x70);   // Read status command

    if (NF_RDDATA()&0x1) // Erase error
    {   
        NF_nFCE_H();
    	Uart_Printf("[ERASE_ERROR:block#=%d]\n",block);
    	NF_MarkBadBlock(block);
    	return 0;
    }
    else 
    {
        NF_nFCE_H();
        return 1;
    }
}


static int NF_IsBadBlock(U32 block)
{
    int i;
    unsigned int blockPage;
    U8 data;
    
    
    blockPage=(block<<5);   // For 2'nd cycle I/O[7:5] 
    
    NF_nFCE_L();    
    NF_CMD(0x50);       // Spare array read command
    NF_ADDR(517&0xf);       // Read the mark of bad block in spare array(M addr=5) 
    NF_ADDR(blockPage&0xff);    // The mark of bad block is in 0 page
    NF_ADDR((blockPage>>8)&0xff);   // For block number A[24:17]
    NF_ADDR((blockPage>>16)&0xff);  // For block number A[25]

   for(i=0;i<10;i++);   // wait tWB(100ns) //?????
    
    NF_WAITRB();    // Wait tR(max 12us)
    
    data=NF_RDDATA();

    NF_nFCE_H();    

    if(data!=0xff)
    {
        Uart_Printf("[block %d has been marked as a bad block(%x)]\n",block,data);
        return 1;
    }
    else
    {
        return 0;
    }
}


static int NF_MarkBadBlock(U32 block)
{
    int i;
    U32 blockPage=(block<<5);
 
    seBuf[0]=0xff;
    seBuf[1]=0xff;    
    seBuf[2]=0xff;    
    seBuf[5]=0x44;   // Bad blcok mark=0
    
    NF_nFCE_L(); 
    NF_CMD(0x50);   //????
    NF_CMD(0x80);   // Write 1st command
    
    NF_ADDR(0x0);           // The mark of bad block is 
    NF_ADDR(blockPage&0xff);        // marked 5th spare array 
    NF_ADDR((blockPage>>8)&0xff);   // in the 1st page.
    NF_ADDR((blockPage>>16)&0xff);  //
    
    for(i=0;i<16;i++)
    {
    NF_WRDATA(seBuf[i]);    // Write spare array
    }

    NF_CMD(0x10);   // Write 2nd command
    
    for(i=0;i<10;i++);  //tWB = 100ns. ///???????

    NF_WAITRB();      // Wait tPROG(200~500us)
  
    NF_CMD(0x70);
    
    for(i=0;i<3;i++);  //twhr=60ns////??????
    
    if (NF_RDDATA()&0x1) // Spare arrray write error
    {   
        NF_nFCE_H();
        Uart_Printf("[Program error is occurred but ignored]\n");
    }
    else 
    {
        NF_nFCE_H();
    }

    Uart_Printf("[block #%d is marked as a bad block]\n",block);
    return 1;
}


static int NF_ReadPage(U32 block,U32 page,U8 *buffer)
{
    int i;
    unsigned int blockPage;
    U8 ecc0,ecc1,ecc2;
    U8 *bufPt=buffer;
    U8 se[16];     
    
    page=page&0x1f;
    blockPage=(block<<5)+page;
    NF_RSTECC();    // Initialize ECC
    
    NF_nFCE_L();    
    NF_CMD(0x00);   // Read command
    NF_ADDR(0);     // Column = 0
    NF_ADDR(blockPage&0xff);        //
    NF_ADDR((blockPage>>8)&0xff);   // Block & Page num.
    NF_ADDR((blockPage>>16)&0xff);  //

    for(i=0;i<10;i++); //wait tWB(100ns)/////??????
    
    NF_WAITRB();    // Wait tR(max 12us)
    for(i=0;i<512;i++)
    {
        *bufPt++=NF_RDDATA();   // Read one page
    }
    ecc0=rNFECC0;
    ecc1=rNFECC1;
    ecc2=rNFECC2;
    for(i=0;i<16;i++)
    {
        se[i]=NF_RDDATA();  // Read spare array
    }
    
    //__RdPage528(pPage);
    NF_nFCE_H();    

    if(ecc0==se[0] && ecc1==se[1] && ecc2==se[2])
    {
    Uart_Printf("[ECC OK:%x,%x,%x]\n",se[0],se[1],se[2]);
        return 1;
    }
    else
    {
    Uart_Printf("[ECC ERROR(RD):read:%x,%x,%x, reg:%x,%x,%x]\n",
        se[0],se[1],se[2],ecc0,ecc1,ecc2);
        return 0;
    }       
}


static int NF_WritePage(U32 block,U32 page,U8 *buffer)
{
    int i;
    U32 blockPage=(block<<5)+page;
    U8 *bufPt=buffer;

    NF_RSTECC();    // Initialize ECC
    
    NF_nFCE_L(); 
    NF_CMD(0x0);//??????
    NF_CMD(0x80);   // Write 1st command
    NF_ADDR(0);             // Column 0
    NF_ADDR(blockPage&0xff);        //
    NF_ADDR((blockPage>>8)&0xff);   // Block & page num.
    NF_ADDR((blockPage>>16)&0xff);  //

    for(i=0;i<512;i++)
    {
    NF_WRDATA(*bufPt++);    // Write one page to NFM from buffer
    }  
    
    seBuf[0]=rNFECC0;
    seBuf[1]=rNFECC1;
    seBuf[2]=rNFECC2;
    seBuf[5]=0xff;      // Marking good block
    
    for(i=0;i<16;i++)
    {
    NF_WRDATA(seBuf[i]);    // Write spare array(ECC and Mark)
    }  

    NF_CMD(0x10);   // Write 2nd command
    
    for(i=0;i<10;i++);  //tWB = 100ns. ////??????

    NF_WAITRB();    //wait tPROG 200~500us;
 
    NF_CMD(0x70);   // Read status command   
    
    for(i=0;i<3;i++);  //twhr=60ns
    
    if (NF_RDDATA()&0x1) // Page write error
    {   
        NF_nFCE_H();
    Uart_Printf("[PROGRAM_ERROR:block#=%d]\n",block);
    NF_MarkBadBlock(block);
    return 0;
    }
    else 
    {
        NF_nFCE_H();
    #if (WRITEVERIFY==1)
    //return NF_VerifyPage(block,page,pPage);   
    #else
    return 1;
    #endif
    }
}



static U16 NF_CheckId(void)
{
    int i;
    U16 id;
    
    NF_nFCE_L();
    
    NF_CMD(0x90);
    NF_ADDR(0x0);
    
    for(i=0;i<10;i++); //wait tWB(100ns)////?????
    
    id=NF_RDDATA()<<8;  // Maker code(K9S1208V:0xec)
    id|=NF_RDDATA();    // Devide code(K9S1208V:0x76)
    
    NF_nFCE_H();
    
    return id;
}


static void NF_Reset(void)
{
    int i;
    U16 id;
    
    
    NF_nFCE_L();

    NF_CMD(0xFF);   //reset command

    for(i=0;i<10;i++);  //tWB = 100ns. //??????

    NF_WAITRB();      //wait 200~500us;
     
    NF_nFCE_H();
}



static void NF_Init(void)
{
    rNFCONF=(1<<15)|(1<<14)|(1<<13)|(1<<12)|(1<<11)|(TACLS<<8)|(TWRPH0<<4)|(TWRPH1<<0); 
    // 1  1    1     1,   1      xxx,  r xxx,   r xxx        
    // En 512B 4step ECCR nFCE=H tACLS   tWRPH0   tWRPH1
    
    NF_Reset();
}

int DownloadData(void)
{
    int i,tmp;
    U16 checkSum=0,dnCS;
    U32 fileSize=10;
    U8 *downPt;

    Uart_Printf("Download through UART0 from 0x30100000.\n");

    downPt=(U8 *)downloadAddress;
  
    Uart_Printf("\ndownloadAddress=%x\n",downloadAddress);

    Uart_Printf("Download the plain binary file to be written\n");
    Uart_Printf("The file format: <n+6>(4)+(n)+CS(2)\n");
    Uart_Printf("Download methods: 115.2KBPS,8Bit,NP,1STOP\n");
 
    Uart_Printf("\nSTATUS:");

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
    
    if((i%1000)==0)WrUTXH0('#');
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
        Uart_Printf("Checksum Error!!! MEM:%x DN:%x\n",checkSum,dnCS);
    return 0;
    }

    Uart_Printf("\nDownload O.K.\n");
    
    return 1;
}


char Check10(int data, char seq) // Check '10' pattern
{
    if(data & (0x2<< seq*2) )
    return 1;
    else
    return 0;
}

int TestECC(void)
{
    int i;
    U32 colECC=0,lowECC=0;
    U32 col=0, low=0;

    rNFCONF=(1<<15)|(1<<12);

    for(i=0;i<512;i++)
    NF_WRDATA(0x0); // Write one page
    
    seBuf[0]=rNFECC0;
    seBuf[1]=rNFECC1;
    seBuf[2]=rNFECC2;

    Uart_Printf("Old: ECC0=0x%x, ECC1=0x%x, ECC2=0x%x\n",seBuf[0],seBuf[1], seBuf[2]);

    rNFCONF=(1<<15)|(1<<12);

    for(i=0;i<200;i++)
    NF_WRDATA(0x0); // Write 200 byte
    NF_WRDATA(0x20);    // Write one btte(col: 5th, line: 200th)
    for(i=0;i<311;i++)
    NF_WRDATA(0x0); // Write 311 byte

    seBuf[3]=rNFECC0;
    seBuf[4]=rNFECC1;
    seBuf[5]=rNFECC2;

    Uart_Printf("New: ECC0=0x%x, ECC1=0x%x, ECC2=0x%x\n",seBuf[3],seBuf[4],seBuf[5]);

    seBuf[6]=seBuf[0]^seBuf[3];
    seBuf[7]=seBuf[1]^seBuf[4];
    seBuf[8]=seBuf[2]^seBuf[5];
    
    Uart_Printf("XOR: ECC0=0x%x, ECC1=0x%x, ECC2=0x%x\n",seBuf[6],seBuf[7],seBuf[8]);
/*
    for(i=4;i>0;i--)
    {
    if( seBuf[6]>>(i*2-2) == 0x0 | seBuf[6]>>(i*2-2) == 0x3 )
    {
        Uart_Printf("The error is above 2bits-Can not recover!!!\n");
        return 0;
    }
    }
*/
    colECC=seBuf[8]>>2;
    lowECC=((seBuf[8]&0x3)<<16)|(seBuf[7]<<8)|(seBuf[6]<<0);
    Uart_Printf("Low ECC=0x%x, Col ECC=0x%x\n", lowECC, colECC);

    // Column parity
    for(i=0;i<3;i++)
    col |= (Check10(colECC, i) <<i);

    Uart_Printf("col=%dth\n",col); 

    // Line parity
    for(i=0;i<9;i++)
    low |= (Check10(lowECC, i) <<i);
    Uart_Printf("low=%dth\n",low); 
}