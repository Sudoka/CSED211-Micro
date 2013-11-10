/**********************************************
  NAME: pwr_c.c
  DESC: 2410 power consumption measurement code
  HISTORY:
  04.09.2002:purnnamu: first release for 2410
  08.14.2002:On-Pil Shin : Added test Condition Setting Table
  10.29.2002:On-Pil Shin
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

// Test condition Setting
//            CPU_WORK    DMA2_ON    UNUSED_BLOCK_ON
//======================================================
// Max_Q    QUICK_SORT       1              1
// Typ_Q    QUICK_SORT       0              1
// Min_Q    QUICK_SORT       0              0
//------------------------------------------------------
// Max_D    DHRYSTONE21      1              1
// Typ_D    DHRYSTONE21      0              1
// Min_D    DHRYSTONE21      0              0
//

//Options for current measurement
#define QUICK_SORT		(1)
#define DHRYSTONE21   	(2)
#define CPU_WORK		QUICK_SORT  //DHRYSTONE21 , QUICK_SORT
//#define CPU_WORK		DHRYSTONE21 //DHRYSTONE21 , QUICK_SORT

#define DMA2_ON			    (0)
//#define DMA2_ON			    (1)
//#define UNUSED_BLCOK_ON		(0)
#define UNUSED_BLCOK_ON		(1)

void PWR_Lcd_Tft_16Bit_240320_On(void);
static void Lcd_Port_Init(void);
void dhrystone21(void);
void PWR_StartTimer(void);
void StartDmaMem2Mem(void);
void PWR_StartIIS(void);
int compare1(const void *a,const void *b);
int compare2(const void *a,const void *b);

static int dim[0x4000];  //64KB,which is bigger than D-Cache(16KB)

#define NUM_SORT_DATA	0x10000
static int dim2[NUM_SORT_DATA];  //128KB,which is bigger than D-Cache(16KB)

#define TMP_FCLK	(266000000)
//#define TMP_FCLK	(237000000)
//#define TMP_FCLK	(226000000)
//#define TMP_FCLK	(214500000)
//#define TMP_FCLK	(202800000)
//#define TMP_FCLK	(180000000)
//#define TMP_FCLK	(152000000)
//#define TMP_FCLK	(135000000)
//#define TMP_FCLK	(101250000)
//#define TMP_FCLK	( 50700000)

void MeasurePowerConsumption(void)
{
    int i,j;
    volatile int a,b,c;

    Uart_Printf("[ Measure the power consumption of S3C2410 ]\n");
    Uart_Printf("FCLK is changed to %5.1lfMhz.\n",TMP_FCLK/1000000.);
    
    MMU_Init();
    ChangeClockDivider(1,1);	    // 1:2:4 
//    ChangeClockDivider(0,1);	    // 1:1:2    
#if TMP_FCLK==266000000
    ChangeMPllValue(125,4,0); // TMP_FCLK=237000000Hz    
    Uart_Init(266000000/4,115200);
#endif       
#if TMP_FCLK==237000000
    ChangeMPllValue(150,2,1); // TMP_FCLK=237000000Hz    
    Uart_Init(237000000/4,115200);
#endif
#if TMP_FCLK==226000000
    ChangeMPllValue(105,1,1); // TMP_FCLK=226000000Hz    
    Uart_Init(226000000/4,115200);
#endif
#if TMP_FCLK==214500000
    ChangeMPllValue(135,2,1); // TMP_FCLK=214500000Hz    
    Uart_Init(214500000/4,115200);
#endif
#if TMP_FCLK==202800000   
    ChangeMPllValue(161,3,1); // TMP_FCLK=202800000Hz    
    Uart_Init(202800000/4,115200);
#endif    
#if TMP_FCLK==180000000   
    ChangeMPllValue(82,1,1);    // TMP_FCLK=180000000Hz    
    Uart_Init(180000000/4,115200);
#endif    
#if TMP_FCLK==152000000
    ChangeMPllValue(68,1,1); // TMP_FCLK=152000000Hz    
    Uart_Init(152000000/4,115200);
#endif
#if TMP_FCLK==135000000
    ChangeMPllValue(82,2,1);  // FCLK=135000000Hz 
    Uart_Init(135000000/4,115200);
#endif
#if TMP_FCLK==101250000
    ChangeMPllValue(127,2,2); // TMP_FCLK=101250000Hz    
    Uart_Init(101250000/2,115200);
#endif
#if TMP_FCLK==50700000
    ChangeMPllValue(161,3,3); // TMP_FCLK=50700000Hz    
    Uart_Init(50700000/4,115200);
#endif
    
    rGPHCON = rGPHCON&~(3<<18|3<<20)|(1<<18|1<<20);  //GPH9:CLKOUT0->OUT,GPH10:CLKOUT1->OUT

    #if 0
    rGPHCON = rGPHCON&~(3<<18)|(2<<18); //CLKOUT0
    rMISCCR = rMISCCR&~(7<<4)|(4<<4);  //PCLK=45Mhz @FCLK=180Mhz
    #endif

    #if UNUSED_BLCOK_ON==0
    rCLKCON=rCLKCON&~(1<<18|1<<16|1<<12|1<<11|1<<9|1<<4); //SPI,IIC,UART1,2,SDI,NFC
    #endif

    #if DMA2_ON==1
    StartDmaMem2Mem();
    #endif 
        
    
    PWR_StartTimer();

    Uart_Printf("TFT-LCD 240x320 16bit is running.\n");
    Lcd_PowerEnable(0,1);
    PWR_Lcd_Tft_16Bit_240320_On();
    
    PWR_StartIIS();
    
#if CPU_WORK==QUICK_SORT
    Uart_Printf("CPU JOB : quick sort of 65536 intteger data.\n");
    //qsort;
    for(i=0;i<NUM_SORT_DATA;i++)dim2[i]=i;
    i=0;    
    while(1)
    {
    	qsort(dim2,NUM_SORT_DATA,sizeof(int),compare1);
    	qsort(dim2,NUM_SORT_DATA,sizeof(int),compare2);
    	Uart_Printf(",");
    }
#else
    Uart_Printf("CPU JOB : Dhrystone 2.1 code.\n");
    while(1)
    {
    	dhrystone21();
    	Uart_Printf(".");
    }

#endif
    Uart_Printf("End of the Measurement\n");	
}


int compare1(const void *a,const void *b)
{
    
    if(*(int *)a>*(int *)b)
    	return 1;
    else if(*(int *)a==*(int *)b)
    	return 0;
    else 
    	return -1;
}

int compare2(const void *a,const void *b)
{
    
    if(*(int *)a>*(int *)b)
    	return -1;
    else if(*(int *)a==*(int *)b)
    	return 0;
    else 
    	return 1;
}

void DoQsort(void) //function for SLOW mode current current consumption
{
    	qsort(dim2,NUM_SORT_DATA,sizeof(int),compare1);
    	qsort(dim2,NUM_SORT_DATA,sizeof(int),compare2);
}

static unsigned save_rGPCUP,save_rGPCDAT,save_rGPCCON;
static unsigned save_rGPDUP,save_rGPDDAT,save_rGPDCON; 


static void Lcd_Port_Init(void)
{
    save_rGPCCON=rGPCCON;
    save_rGPCDAT=rGPCDAT;
    save_rGPCUP=rGPCUP;
    save_rGPDCON=rGPDCON;
    save_rGPDDAT=rGPDDAT;
    save_rGPDUP=rGPDUP;
    rGPCUP=0xffffffff; // Disable Pull-up register
    rGPCCON=0xaaaaaaaa; //Initialize VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND 
    rGPDUP=0xffffffff; // Disable Pull-up register
    rGPDCON=0xaaaaaaaa; //Initialize VD[23:8]
}


void PWR_Lcd_Tft_16Bit_240320_On(void)
{
    int i,j,k;
    Lcd_Port_Init();
    //Lcd_Lpc3600Enable(); // Enable LPC3600
    Lcd_Init(MODE_TFT_16BIT_240320);
    Glib_Init(MODE_TFT_16BIT_240320);
    Lcd_EnvidOnOff(1);

    Glib_ClearScr(0, MODE_TFT_16BIT_240320);
    Glib_Rectangle(0,0,239,319,0x07e0);   
    Glib_FilledRectangle(0,0,20,20,65535);   
    Glib_Rectangle(220,300,239,319,65535);   
    Glib_Line(0,0,239,319,0x1f);        
    Glib_Line(239,0,0,319,0xf800);
    Uart_Printf("TFT 64K color mode test 2!\n");
}

void PWR_StartTimer(void)
{
    Uart_Printf("All 5 timers are running.\n");
    
    rTCFG0=0x8080;   //Prescaler=128; 
    rTCFG1=0x11111;  //Devider=1/4;
    rTCNTB0=0x8000;
    rTCMPB0=0x4000;
    rTCNTB1=0x8000;
    rTCMPB1=0x4000;
    rTCNTB2=0x8000;
    rTCMPB2=0x4000;
    rTCNTB3=0x8000;
    rTCMPB3=0x4000;
    rTCNTB4=0x8000;
    rTCON=0x6aaa0a; //manual update
    rTCON=0x599909; //run
}



static int buf1[1024],buf2[1024];

void __irq PWR_IsrDma0(void)
{
    ClearPending(BIT_DMA0);
    rDMASKTRIG0=0x3; 	
    //Uart_Printf("$");
}


void StartDmaMem2Mem(void)
{
    Uart_Printf("Seamless DMA0 SDRAM -> SDRAM transfer is running.\n");
    
    pISR_DMA0=(unsigned int)PWR_IsrDma0;	
    rINTMSK=rINTMSK&~(BIT_DMA0);
 
    rDISRC0=(U32)buf1;
    rDISRCC0=(0<<1)|(0<<0); //src=AHB,increment
    rDIDST0=(U32)buf2;
    rDIDSTC0=(0<<1)|(0<<0); //dst=AHB,increment
    rDCON0=(1<<31)|(1<<29)|(1<<27)|(2<<20)|(1024);
    	//handshake,done int. enable,whole mode,DSZ=word,TC=1024
    rDMASKTRIG0=0x3; 	    	
}	

static U16 buf3[4096];

void __irq PWR_IsrDma2(void)
{
    ClearPending(BIT_DMA2);
    //Uart_Printf("#");
}

void PWR_StartIIS(void)
{
    Uart_Printf("IIS(DMA2) is running(CDCLK=16.9Mhz,LRCK=44.1Khz).\n");

    pISR_DMA2=(unsigned int)PWR_IsrDma2;	
    rINTMSK=rINTMSK&~(BIT_DMA2);

    rGPECON=rGPECON&~(3<<8|3<<4|3<<2|3<<0)|(2<<8|2<<4|2<<2|2<<0); //I2SSDO,CDCLK,I2SSCLK,I2SLRCK
        
    rDISRC2=(U32)buf3; 
    rDISRCC2=(0<<1)+(0<<0);//src=AHB,increment
    rDIDST2=((U32) IISFIFO );
    rDIDSTC2=(1<<1)+(1<<0); //APB,fix
    rDCON2=(1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(0<<24)|(1<<23)|(0<<22)|(1<<20)|(4096);
    //handshake, sync PCLK, TC int, single tx, single service, I2SSDO, I2S request, 
    //auto-reload, half-word,
    rDMASKTRIG2=(1<<1)+0;    //DMA2 channel on, no-sw trigger 

    rIISPSR=(2<<5)|(2<<0); //IIS_LRCK=44.1Khz @384fs,PCLK=50Mhz.
    
    rIISCON=(1<<5)+(1<<2)+(1<<1);
    //Tx DMA enable,Rx idle,prescaler enable
    rIISMOD=(2<<6)|(1<<3)|(1<<2)|(1<<0);
    //Master,Tx,L-ch=low,iis,16bit ch.,CDCLK=384fs,IISCLK=32fs

    rIISFCON=(1<<15)+(1<<13);	//Tx DMA,Tx FIFO enable

    rIISCON |=0x1;
}
/*
 ****************************************************************************
 *
 *                   "DHRYSTONE" Benchmark Program
 *                   -----------------------------
 *                                                                            
 *  Version:    C, Version 2.1
 *                                                                            
 *  File:       dhry.h (part 1 of 3)
 *
 *  Date:       May 25, 1988
 *
 *  Author:     Reinhold P. Weicker
 *                      Siemens AG, E STE 35
 *                      Postfach 3240
 *                      8520 Erlangen
 *                      Germany (West)
 *                              Phone:  [xxx-49]-9131-7-20330
 *                                      (8-17 Central European Time)
 *                              Usenet: ..!mcvax!unido!estevax!weicker
 *
 *              Original Version (in Ada) published in
 *              "Communications of the ACM" vol. 27., no. 10 (Oct. 1984),
 *              pp. 1013 - 1030, together with the statistics
 *              on which the distribution of statements etc. is based.
 *
 *              In this C version, the following C library functions are used:
 *              - strcpy, strcmp (inside the measurement loop)
 *              - printf, scanf (outside the measurement loop)
 *              In addition, Berkeley UNIX system calls "times ()" or "time ()"
 *              are used for execution time measurement. For measurements
 *              on other systems, these calls have to be changed.
 *
 *  Collection of Results:
 *              Reinhold Weicker (address see above) and
 *              
 *              Rick Richardson
 *              PC Research. Inc.
 *              94 Apple Orchard Drive
 *              Tinton Falls, NJ 07724
 *                      Phone:  (201) 389-8963 (9-17 EST)               
 *                      Usenet: ...!uunet!pcrat!rick
 *
 *      Please send results to Rick Richardson and/or Reinhold Weicker.
 *      Complete information should be given on hardware and software used.
 *      Hardware information includes: Machine type, CPU, type and size
 *      of caches; for microprocessors: clock frequency, memory speed
 *      (number of wait states).
 *      Software information includes: Compiler (and runtime library)
 *      manufacturer and version, compilation switches, OS version.
 *      The Operating System version may give an indication about the
 *      compiler; Dhrystone itself performs no OS calls in the measurement loop.
 *
 *      The complete output generated by the program should be mailed
 *      such that at least some checks for correctness can be made.
 *
 ***************************************************************************
 */

/* Compiler and system dependent definitions: */

#define Mic_secs_Per_Second     1000000.0
                /* Berkeley UNIX C returns process times in seconds/HZ */

#ifdef  NOSTRUCTASSIGN
#define structassign(d, s)      memcpy(&(d), &(s), sizeof(d))
#else
#define structassign(d, s)      d = s
#endif

#ifdef  NOENUM
#define Ident_1 0
#define Ident_2 1
#define Ident_3 2
#define Ident_4 3
#define Ident_5 4
  typedef int   Enumeration;
#else
  typedef       enum    {Ident_1, Ident_2, Ident_3, Ident_4, Ident_5}
                Enumeration;
#endif
        /* for boolean and enumeration types in Ada, Pascal */

/* General definitions: */

#include <stdio.h>
                /* for strcpy, strcmp */

#define Null 0 
                /* Value of a Null pointer */
#define true  1
#define false 0

typedef int     One_Thirty;
typedef int     One_Fifty;
typedef char    Capital_Letter;
typedef int     Boolean;
typedef char    Str_30 [31];
typedef int     Arr_1_Dim [50];
typedef int     Arr_2_Dim [50] [50];

typedef struct record 
    {
    struct record *Ptr_Comp;
    Enumeration    Discr;
    union {
          struct {
                  Enumeration Enum_Comp;
                  int         Int_Comp;
                  char        Str_Comp [31];
                  } var_1;
          struct {
                  Enumeration E_Comp_2;
                  char        Str_2_Comp [31];
                  } var_2;
          struct {
                  char        Ch_1_Comp;
                  char        Ch_2_Comp;
                  } var_3;
          } variant;
      } Rec_Type, *Rec_Pointer;



/*
 ****************************************************************************
 *
 *                   "DHRYSTONE" Benchmark Program
 *                   -----------------------------
 *                                                                            
 *  Version:    C, Version 2.1
 *                                                                            
 *  File:       dhry_1.c (part 2 of 3)
 *
 *  Date:       May 25, 1988
 *
 *  Author:     Reinhold P. Weicker
 *
 ****************************************************************************
 */

#include <string.h>


/* Global Variables: */

Rec_Pointer     Ptr_Glob,
                Next_Ptr_Glob;
int             Int_Glob;
Boolean         Bool_Glob;
char            Ch_1_Glob,
                Ch_2_Glob;
int             Arr_1_Glob [50];
int             Arr_2_Glob [50] [50];

/* extern char     *malloc (); */

Enumeration     Func_1 ();
  /* forward declaration necessary since Enumeration may not simply be int */

#ifndef REG
        Boolean Reg = false;
#define REG
        /* REG becomes defined as empty */
        /* i.e. no register variables   */
#else
        Boolean Reg = true;
#endif


void dhrystone21(void)
/*****/

  /* main program, corresponds to procedures        */
  /* Main and Proc_0 in the Ada version             */
{
        One_Fifty       Int_1_Loc;
  REG   One_Fifty       Int_2_Loc;
        One_Fifty       Int_3_Loc;
  REG   char            Ch_Index;
        Enumeration     Enum_Loc;
        Str_30          Str_1_Loc;
        Str_30          Str_2_Loc;
  REG   int             Run_Index;
  REG   int             Number_Of_Runs;

  /* Initializations */

  Next_Ptr_Glob = (Rec_Pointer) malloc (sizeof (Rec_Type));
  Ptr_Glob = (Rec_Pointer) malloc (sizeof (Rec_Type));

  Ptr_Glob->Ptr_Comp                    = Next_Ptr_Glob;
  Ptr_Glob->Discr                       = Ident_1;
  Ptr_Glob->variant.var_1.Enum_Comp     = Ident_3;
  Ptr_Glob->variant.var_1.Int_Comp      = 40;
  strcpy (Ptr_Glob->variant.var_1.Str_Comp, 
          "DHRYSTONE PROGRAM, SOME STRING");
  strcpy (Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING");

  Arr_2_Glob [8][7] = 10;
        /* Was missing in published program. Without this statement,    */
        /* Arr_2_Glob [8][7] would have an undefined value.             */
        /* Warning: With 16-Bit processors and Number_Of_Runs > 32000,  */
        /* overflow may occur for this array element.                   */

  Number_Of_Runs = 10000;

  for (Run_Index = 1; Run_Index <= Number_Of_Runs; ++Run_Index)
  {

    Proc_5();
    Proc_4();
      /* Ch_1_Glob == 'A', Ch_2_Glob == 'B', Bool_Glob == true */
    Int_1_Loc = 2;
    Int_2_Loc = 3;
    strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
    Enum_Loc = Ident_2;
    Bool_Glob = ! Func_2 (Str_1_Loc, Str_2_Loc);
      /* Bool_Glob == 1 */
    while (Int_1_Loc < Int_2_Loc)  /* loop body executed once */
    {
      Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
        /* Int_3_Loc == 7 */
      Proc_7 (Int_1_Loc, Int_2_Loc, &Int_3_Loc);
        /* Int_3_Loc == 7 */
      Int_1_Loc += 1;
    } /* while */
      /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    Proc_8 (Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
      /* Int_Glob == 5 */
    Proc_1 (Ptr_Glob);
    for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index)
                             /* loop body executed twice */
    {
      if (Enum_Loc == Func_1 (Ch_Index, 'C'))
          /* then, not executed */
        {
        Proc_6 (Ident_1, &Enum_Loc);
        strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 3'RD STRING");
        Int_2_Loc = Run_Index;
        Int_Glob = Run_Index;
        }
    }
      /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    Int_2_Loc = Int_2_Loc * Int_1_Loc;
    Int_1_Loc = Int_2_Loc / Int_3_Loc;
    Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
      /* Int_1_Loc == 1, Int_2_Loc == 13, Int_3_Loc == 7 */
    Proc_2 (&Int_1_Loc);
      /* Int_1_Loc == 5 */
  } /* loop "for Run_Index" */
}


Proc_1 (Ptr_Val_Par)
/******************/

REG Rec_Pointer Ptr_Val_Par;
    /* executed once */
{
  REG Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp;  
                                        /* == Ptr_Glob_Next */
  /* Local variable, initialized with Ptr_Val_Par->Ptr_Comp,    */
  /* corresponds to "rename" in Ada, "with" in Pascal           */
  
  structassign (*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob); 
  Ptr_Val_Par->variant.var_1.Int_Comp = 5;
  Next_Record->variant.var_1.Int_Comp 
        = Ptr_Val_Par->variant.var_1.Int_Comp;
  Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
  Proc_3 (&Next_Record->Ptr_Comp);
    /* Ptr_Val_Par->Ptr_Comp->Ptr_Comp 
                        == Ptr_Glob->Ptr_Comp */
  if (Next_Record->Discr == Ident_1)
    /* then, executed */
  {
    Next_Record->variant.var_1.Int_Comp = 6;
    Proc_6 (Ptr_Val_Par->variant.var_1.Enum_Comp, 
           &Next_Record->variant.var_1.Enum_Comp);
    Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
    Proc_7 (Next_Record->variant.var_1.Int_Comp, 10, 
           &Next_Record->variant.var_1.Int_Comp);
  }
  else /* not executed */
    structassign (*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
} /* Proc_1 */


Proc_2 (Int_Par_Ref)
/******************/
    /* executed once */
    /* *Int_Par_Ref == 1, becomes 4 */

One_Fifty   *Int_Par_Ref;
{
  One_Fifty  Int_Loc;  
  Enumeration   Enum_Loc;

  Int_Loc = *Int_Par_Ref + 10;
  do /* executed once */
    if (Ch_1_Glob == 'A')
      /* then, executed */
    {
      Int_Loc -= 1;
      *Int_Par_Ref = Int_Loc - Int_Glob;
      Enum_Loc = Ident_1;
    } /* if */
  while (Enum_Loc != Ident_1); /* true */
} /* Proc_2 */


Proc_3 (Ptr_Ref_Par)
/******************/
    /* executed once */
    /* Ptr_Ref_Par becomes Ptr_Glob */

Rec_Pointer *Ptr_Ref_Par;

{
  if (Ptr_Glob != Null)
    /* then, executed */
    *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
  Proc_7 (10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
} /* Proc_3 */


Proc_4 () /* without parameters */
/*******/
    /* executed once */
{
  Boolean Bool_Loc;

  Bool_Loc = Ch_1_Glob == 'A';
  Bool_Glob = Bool_Loc | Bool_Glob;
  Ch_2_Glob = 'B';
} /* Proc_4 */


Proc_5 () /* without parameters */
/*******/
    /* executed once */
{
  Ch_1_Glob = 'A';
  Bool_Glob = false;
} /* Proc_5 */


        /* Procedure for the assignment of structures,          */
        /* if the C compiler doesn't support this feature       */
#ifdef  NOSTRUCTASSIGN
memcpy (d, s, l)
register char   *d;
register char   *s;
register int    l;
{
        while (l--) *d++ = *s++;
}
#endif


/*
 ****************************************************************************
 *
 *                   "DHRYSTONE" Benchmark Program
 *                   -----------------------------
 *                                                                            
 *  Version:    C, Version 2.1
 *                                                                            
 *  File:       dhry_2.c (part 3 of 3)
 *
 *  Date:       May 25, 1988
 *
 *  Author:     Reinhold P. Weicker
 *
 ****************************************************************************
 */

#ifndef REG
#define REG
        /* REG becomes defined as empty */
        /* i.e. no register variables   */
#endif

extern  int     Int_Glob;
extern  char    Ch_1_Glob;


Proc_6 (Enum_Val_Par, Enum_Ref_Par)
/*********************************/
    /* executed once */
    /* Enum_Val_Par == Ident_3, Enum_Ref_Par becomes Ident_2 */

Enumeration  Enum_Val_Par;
Enumeration *Enum_Ref_Par;
{
  *Enum_Ref_Par = Enum_Val_Par;
  if (! Func_3 (Enum_Val_Par))
    /* then, not executed */
    *Enum_Ref_Par = Ident_4;
  switch (Enum_Val_Par)
  {
    case Ident_1: 
      *Enum_Ref_Par = Ident_1;
      break;
    case Ident_2: 
      if (Int_Glob > 100)
        /* then */
      *Enum_Ref_Par = Ident_1;
      else *Enum_Ref_Par = Ident_4;
      break;
    case Ident_3: /* executed */
      *Enum_Ref_Par = Ident_2;
      break;
    case Ident_4: break;
    case Ident_5: 
      *Enum_Ref_Par = Ident_3;
      break;
  } /* switch */
} /* Proc_6 */


Proc_7 (Int_1_Par_Val, Int_2_Par_Val, Int_Par_Ref)
/**********************************************/
    /* executed three times                                      */ 
    /* first call:      Int_1_Par_Val == 2, Int_2_Par_Val == 3,  */
    /*                  Int_Par_Ref becomes 7                    */
    /* second call:     Int_1_Par_Val == 10, Int_2_Par_Val == 5, */
    /*                  Int_Par_Ref becomes 17                   */
    /* third call:      Int_1_Par_Val == 6, Int_2_Par_Val == 10, */
    /*                  Int_Par_Ref becomes 18                   */
One_Fifty       Int_1_Par_Val;
One_Fifty       Int_2_Par_Val;
One_Fifty      *Int_Par_Ref;
{
  One_Fifty Int_Loc;

  Int_Loc = Int_1_Par_Val + 2;
  *Int_Par_Ref = Int_2_Par_Val + Int_Loc;
} /* Proc_7 */


Proc_8 (Arr_1_Par_Ref, Arr_2_Par_Ref, Int_1_Par_Val, Int_2_Par_Val)
/*********************************************************************/
    /* executed once      */
    /* Int_Par_Val_1 == 3 */
    /* Int_Par_Val_2 == 7 */
Arr_1_Dim       Arr_1_Par_Ref;
Arr_2_Dim       Arr_2_Par_Ref;
int             Int_1_Par_Val;
int             Int_2_Par_Val;
{
  REG One_Fifty Int_Index;
  REG One_Fifty Int_Loc;

  Int_Loc = Int_1_Par_Val + 5;
  Arr_1_Par_Ref [Int_Loc] = Int_2_Par_Val;
  Arr_1_Par_Ref [Int_Loc+1] = Arr_1_Par_Ref [Int_Loc];
  Arr_1_Par_Ref [Int_Loc+30] = Int_Loc;
  for (Int_Index = Int_Loc; Int_Index <= Int_Loc+1; ++Int_Index)
    Arr_2_Par_Ref [Int_Loc] [Int_Index] = Int_Loc;
  Arr_2_Par_Ref [Int_Loc] [Int_Loc-1] += 1;
  Arr_2_Par_Ref [Int_Loc+20] [Int_Loc] = Arr_1_Par_Ref [Int_Loc];
  Int_Glob = 5;
} /* Proc_8 */


Enumeration Func_1 (Ch_1_Par_Val, Ch_2_Par_Val)
/*************************************************/
    /* executed three times                                         */
    /* first call:      Ch_1_Par_Val == 'H', Ch_2_Par_Val == 'R'    */
    /* second call:     Ch_1_Par_Val == 'A', Ch_2_Par_Val == 'C'    */
    /* third call:      Ch_1_Par_Val == 'B', Ch_2_Par_Val == 'C'    */

Capital_Letter   Ch_1_Par_Val;
Capital_Letter   Ch_2_Par_Val;
{
  Capital_Letter        Ch_1_Loc;
  Capital_Letter        Ch_2_Loc;

  Ch_1_Loc = Ch_1_Par_Val;
  Ch_2_Loc = Ch_1_Loc;
  if (Ch_2_Loc != Ch_2_Par_Val)
    /* then, executed */
    return (Ident_1);
  else  /* not executed */
  {
    Ch_1_Glob = Ch_1_Loc;
    return (Ident_2);
   }
} /* Func_1 */


Boolean Func_2 (Str_1_Par_Ref, Str_2_Par_Ref)
/*************************************************/
    /* executed once */
    /* Str_1_Par_Ref == "DHRYSTONE PROGRAM, 1'ST STRING" */
    /* Str_2_Par_Ref == "DHRYSTONE PROGRAM, 2'ND STRING" */

Str_30  Str_1_Par_Ref;
Str_30  Str_2_Par_Ref;
{
  REG One_Thirty        Int_Loc;
      Capital_Letter    Ch_Loc;

  Int_Loc = 2;
  while (Int_Loc <= 2) /* loop body executed once */
    if (Func_1 (Str_1_Par_Ref[Int_Loc],
                Str_2_Par_Ref[Int_Loc+1]) == Ident_1)
      /* then, executed */
    {
      Ch_Loc = 'A';
      Int_Loc += 1;
    } /* if, while */
  if (Ch_Loc >= 'W' && Ch_Loc < 'Z')
    /* then, not executed */
    Int_Loc = 7;
  if (Ch_Loc == 'R')
    /* then, not executed */
    return (true);
  else /* executed */
  {
    if (strcmp (Str_1_Par_Ref, Str_2_Par_Ref) > 0)
      /* then, not executed */
    {
      Int_Loc += 7;
      Int_Glob = Int_Loc;
      return (true);
    }
    else /* executed */
      return (false);
  } /* if Ch_Loc */
} /* Func_2 */


Boolean Func_3 (Enum_Par_Val)
/***************************/
    /* executed once        */
    /* Enum_Par_Val == Ident_3 */
Enumeration Enum_Par_Val;
{
  Enumeration Enum_Loc;

  Enum_Loc = Enum_Par_Val;
  if (Enum_Loc == Ident_3)
    /* then, executed */
    return (true);
  else /* not executed */
    return (false);
} /* Func_3 */