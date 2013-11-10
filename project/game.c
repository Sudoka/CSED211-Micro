/**************************************************** 
  Project Name	: SPACE INVADER FOR ARM BOARD
  Menufacturer	: ±Ëµø±‘, ±ËΩ√»∆
  Class		: Microprocessor Structure and Programing
  Due Date	: 2009.12.29
 ****************************************************/

#include "2410addr.h"
#include "2410lib.h"
#include "def.h"
#include "lcd.h"
#include "lcdlib.h"
#include "glib.h"
#include "game.h"

void START(void);
void ISREINT0(void);
void ISREINT2(void);
void ISREINT11(void);
void ISREINT19(void);

static void __irq Eint0Int(void)
{
    ClearPending(BIT_EINT0);
	ISREINT0();
	Uart_Printf("EINT0 interrupt is occurred.\n");

}

static void __irq Eint2Int(void)
{
    ClearPending(BIT_EINT2);
	ISREINT2();
	Uart_Printf("EINT2 interrupt is occurred.\n");
}

static void __irq Eint11_19(void)
{
	if(rEINTPEND==(1<<11)){
		void ISREINT11(void);
		Uart_Printf("EINT11 interrupt is occurred.\n");
		rEINTPEND=(1<<11);
    }
    else if(rEINTPEND==(1<<19)){
		void ISREINT19(void);
		Uart_Printf("EINT19 interrupt is occurred.\n");
		rEINTPEND=(1<<19);
    }
    else{
		Uart_Printf("rEINTPEND=0x%x\n",rEINTPEND);
		rEINTPEND=(1<<19)|(1<<11);
    }
    ClearPending(BIT_EINT8_23);
}

void gamestart(void)
{
	//set LCD
	Lcd_Port_Init();
    Lcd_Palette8Bit_Init(); // Initialize 256 palette 
    Lcd_Init(MODE_TFT_8BIT_240320);
    Glib_Init(MODE_TFT_8BIT_240320);
    //Lcd_Lpc3600Enable(); // Enable LPC3600
    Lcd_PowerEnable(0, 1);
    Lcd_EnvidOnOff(1); // Enable ENVID Bit
	Uart_Printf("Setting LCD...\n");
    
	//set Buttons
    rGPFCON = (rGPFCON & 0xffcc)|(1<<5)|(1<<1);		//PF0/2 = EINT0/2
    rGPGCON = (rGPGCON & 0xff3fff3f)|(1<<23)|(1<<7);	//PG3/11 = EINT11/19
	rEXTINT0 = (rEXTINT0 & ~((7<<8)  | (0x7<<0))) | 0x2<<8 | 0x2<<0; //EINT0/2=falling edge triggered
	rEXTINT1 = (rEXTINT1 & ~(7<<12)) | 0x2<<12; //EINT11=falling edge triggered
	rEXTINT2 = (rEXTINT2 & ~(7<<12)) | 0x2<<12; //EINT19=falling edge triggered
	
	pISR_EINT0=(U32)Eint0Int;
	pISR_EINT2=(U32)Eint2Int;
	pISR_EINT8_23=(U32)Eint11_19;

    rEINTPEND = 0xffffff;
    rSRCPND = BIT_EINT0|BIT_EINT2|BIT_EINT8_23; //to clear the previous pending states
    rINTPND = BIT_EINT0|BIT_EINT2|BIT_EINT8_23;
    
    rEINTMASK=~( (1<<11)|(1<<19) );
    rINTMSK=~(BIT_EINT0|BIT_EINT2|BIT_EINT8_23);

	Uart_Printf("Setting External interupt...\n\n");

	START();	//call assembly code

	Uart_Printf("If you wanna stop, press any key.\n");

    Uart_Getch();
    
    rEINTMASK=0xffffff;
    rINTMSK=BIT_ALLMSK;
	Lcd_EnvidOnOff(0);
	Lcd_Port_Return();
	Uart_Printf("Good Bye!\n"); 
}