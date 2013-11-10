;*************************************************************
; NAME : NAND FLASH Subroutine for a410 bootstrap
; DATE : 18.FEB.2002
; DESC :
;  18.FEB.2002:purnnamu: modified for A410
;*************************************************************

A410_BASE_ADDR	EQU	0x2000000

	AREA |C$$code|, CODE, READONLY
	
	EXPORT	__RdPage512
__RdPage512
	;input:a1(r0)=pPage
	stmfd	sp!,{r1-r11}
	mov	a2,#A410_BASE_ADDR
	mov	a3,#0x200
0	ldmia	a2,{r4-r11}
	stmia	a1!,{r4-r11}
	subs	a3,a3,#32
	bne	%B0

	ldmfd	sp!,{r1-r11}
	mov		pc,lr
	
	

	EXPORT	__WrPage512
__WrPage512
	;input:a1(r0)=pPage
	stmfd	sp!,{r1-r11}
	mov 	a2,#A410_BASE_ADDR
	mov		a3,#0x200
0	ldmia	a1!,{r4-r11}
	stmia	a2,{r4-r11}
	subs	a3,a3,#32
	bne		%B0

	ldmfd	sp!,{r1-r11}
	mov		pc,lr

	END
