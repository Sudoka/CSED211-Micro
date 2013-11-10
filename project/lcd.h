#ifndef __LCD_H__
#define __LCD_H__

void Lcd_Port_Init(void);
void Lcd_Port_Return(void);
void Lcd_Palette1Bit_Init(void);
void Lcd_Palette8Bit_Init(void);
void __irq Lcd_Int_Frame(void);
void __irq Lcd_Int_Fifo(void);
void __irq Lcd_Int_Fifo_640480(void);

void Test_Lcd_Stn_1Bit(void);
void Test_Lcd_Stn_2Bit(void);
void Test_Lcd_Stn_4Bit(void);
void Test_Lcd_Cstn_8Bit(void);
void Test_Lcd_Cstn_12Bit(void);
void Test_Lcd_Cstn_8Bit_On(void);
void Test_Lcd_Tft_8Bit_240320(void);
void Test_Lcd_Tft_8Bit_240320_On(void);
void Test_Lcd_Tft_16Bit_240320(void);
void Test_Lcd_Tft_1Bit_640480(void);
void Test_Lcd_Tft_8Bit_640480(void);
void Test_Lcd_Tft_16Bit_640480(void);
void Test_Lcd_Tft_24Bit_640480(void);
void Test_Lcd_Tft_8Bit_640480_Palette(void);
void Test_Lcd_Tft_8Bit_640480_Bswp(void);
void Test_Lcd_Tft_16Bit_640480_Hwswp(void);
void Test_Lcd_Tft_16Bit_640480_Bmp(void);
void Test_Lcd_Tft_1Bit_800600(void);
void Test_Lcd_Tft_8Bit_800600(void);
void Test_Lcd_Tft_16Bit_800600(void);

#endif /*__LCD_H__*/
