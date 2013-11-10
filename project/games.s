	;********************************
	;*	SPACE INVADER CODE	*
	;*	By 김동규, 김시훈	*
	;*	For Microprocessor A+	*
	;********************************
	
	;=================
	; MACROS
	;=================
	
	;MOV MACRO
	;BYTE
	MACRO
$label	BMOV	$a, $b
	LDR	r2, =$a
	MOV	r3, $b
	STRB	r3, [r2]
	MEND

	;HALF WORD
	MACRO
$label	HMOV	$a, $b
	LDR	r2, =$a
	MOV	r3, $b
	STRH	r3, [r2]
	MEND

	;++ MACRO
	;BYTE
	MACRO
$label	BINC	$a
	LDR	r2, =$a
	LDRB	r3, [r2]
	ADD	r3, r3, #1
	STRB	r3, [r2]
	MEND
	
	;HALF WORD
	MACRO
$label	HINC	$a
	LDR	r2, =$a
	LDRH	r3, [r2]
	ADD	r3, r3, #1
	STRH	r3, [r2]
	MEND

	;--MACRO
	;BYTE
	MACRO
$label	BDEC	$a
	LDR	r2, =$a
	LDRB	r3, [r2]
	SUB	r3, r3, #1
	STRB	r3, [r2]
	MEND
	
	;HALF WORD
	MACRO
$label	HDEC	$a
	LDR	r2, =$a
	LDRH	r3, [r2]
	SUB	r3, r3, #1
	STRH	r3, [r2]
	MEND


	;=================
	; LCD CONTROLLER
	;=================

LCDFRAMEBUFFER	EQU	0x33800000	;_NONCACHE_STARTADDRESS 
LCD_XSIZE	EQU 	240	
LCD_YSIZE	EQU 	320


	;=================
	; Define register
	;=================

buffer		RN	0		;r0 => start addr of lcd buffer
score		RN	1		;r1 => for score
main		RN	12		;r12 => branch point of main


	;=================
	; CODE AREA
	;=================

	AREA	Space_Invader, CODE, READONLY


	EXPORT	START

;============================
; START SPACE INVADER!
;============================
START
	MOV	main, lr		;to return to main function later
	MOV	buffer, #LCDFRAMEBUFFER	;r0 <= LCDFRAMBUFFER
	MOV	r2, #LCD_XSIZE		;r2 <= width of LCD
	MOV	r3, #LCD_YSIZE		;r3 <= height of LCD

NEXT_STAGE
	BL	CLEARSCR		;call CLEARSCR
	LDR	r2, =STAGE		;r2 <= addr of STAGE
	LDRB	r3, [r2]		;r3 <= STAGE
	CMP	r3, #MAX_STAGE		;STAGE = MAX_STAGE?
	BGE	ENDING			;then Go to ENDING

	BL	INIT_VAL		;call INIT_VAL
	BINC	STAGE			;STAGE++

	BL	DRAW_SPACESHIP		;Draw my spaceship

PLAYING
	LDR	r2, =CL_FLAG		;r2 <= addr of CL_FLAG
	LDRB	r3, [r2]		;r3 <= CL_FLAG
	CMP	r3, #1			;If clear screen flag is set,
	BNE	CHECK_DRAW_SPACESHIP	;then clear screen
	BL	CLEARSCR
	BL	DRAW_SPACESHIP		;Draw my spaceship
	BL	DRAW_ENEMY		;Draw enermies

CHECK_DRAW_SPACESHIP
	LDR	r2, =Draw_Unit_Flag	;r2 <= addr of Draw_Unit_Flag
	LDRB	r3, [r2]		;r3 <= Draw_Unit_Flag
	CMP	r3, #1			;r3 == 1?
	BNE	CHECK_ENEMY_MOVE	;then, branch
	MOV	r3, #0			;r3 <= 0
	STRB	r3, [r2]		;Draw_Unit_Flag <= r3
	BL	DRAW_SPACESHIP		;call DRAW_SPACESHIP

CHECK_ENEMY_MOVE
	LDR	r2, =Time_Alien		;r2 <= addr of Time_Alien
	LDRH	r3, [r2]		;r3 <= Time_Alien	
	LDR	r4, =F_ALIEN		;r4 <= adrr of F_ALIEN
	LDRH	r5, [r4]		;r5 <= F_ALIEN
	CMP	r3, r5			;r3 <= r5?
	BLS	ENEMY_NO_MOVE		;then, branch
	MOV	r4, #0			;r4 <= 0
	STRH	r4, [r2]		;Time_Alien <= 0

	LDR	r4, =FLAG		;r4 <= addr of FLAG
	LDRB	r5, [r4]		;r5 <= FLAG
	CMP	r5, #1			;r5 == 1?
	BEQ	CLEAR_MOTION_FLAG	;then, branch
	MOV	r5, #1			;r5 <= 1;
	B	SET_MOTION_FLAG		;Unconditional branch
CLEAR_MOTION_FLAG
	MOV	r5, #0			;r5 <= 0;
SET_MOTION_FLAG
	STRB	r5, [r4]		;FLAG <= r5

	BL	MOVE_ENEMY		;then call MOVE_ENEMY
	BL	DRAW_ENEMY		;Draw enermies
ENEMY_NO_MOVE

	LDR	r2, =Flag_Shot		;r2 <= addr of Flag_Shot
	LDRB	r3, [r2]		;r3 <= Flag_Shot
	CMP	r3, #1			;r3 == 1?			
	BNE	CHECK_ENEMY_MISSILE	;if not, branch
	LDR	r2, =Time_Missile	;r2 <= addr of Time_Missile
	LDRH	r3, [r2]		;r3 <= Time_Missile
	CMP	r3, #F_MISSILE		;r3 < F_MISSILE?
	BLT	CHECK_ENEMY_MISSILE	;then, branch
	HMOV	Time_Missile, #0	;then, Time_Missile <= 0
	BL	DRAW_USER_MISSILE	;then, call DRAW_USER_MISSILE

CHECK_ENEMY_MISSILE
;	LDR	r2, =Flag_E_Shot	;r2 <= addr of Flag_E_Shot
;	LDRB	r3, [r2]		;r3 <= Flag_Shot
;	CMP	r3, #1			;r3 == 1?			
;	BNE	TIMERS			;if not, branch
;	LDR	r2, =Time_E_Missile	;r2 <= addr of Time_E_Missile
;	LDRH	r3, [r2]		;r3 <= Time_Enemy_Missile
;	CMP	r3, #F_E_MISSILE	;r3 >= F_E_MISSILE?
;	BLGE	DRAW_ENEMY_MISSILE	;then, call DRAW_ENEMY_MISSILE

TIMERS
	HINC	Time_Alien		;Time_Alien++
	HINC	Time_Missile		;Time_Missile++
	HINC	Time_E_Missile		;Time_E_Missile++
	MOV	r2, #0			;r2 = 0
DELAY
	ADD	r2, r2, #1		;r2 = r2 + 1
	CMP	r2, #0xff << 4		;r2 == 2^12?
	BNE	DELAY			;if not, delay

	B	PLAYING			;Infinite loop
	
ENDING
	MOV	pc, main		;return to main

	
;============================
; Sub Routine for CLEARSCREAN
;============================

CLEARSCR
	MOV	r2, #LCD_XSIZE
	MOV	r3, #LCD_YSIZE
	MUL	r4, r2, r3		;r4 <= width * height
	MOV	r5, #0			;r5 <= blue
	MOV	r6, #0			;for loop
CLEARSCR_LOOP
	STRB	r5, [buffer, r6]	;dot black
	ADD	r6, r6, #1		;r6 = r6 + 1
	CMP	r6, r4			;r6 = r4?
	BNE	CLEARSCR_LOOP		;if not, loop
	BMOV	CL_FLAG, #0		;off cear screen flag
	MOV	pc, lr			;return


;============================
; Put_Pixel Sub Routine
;============================

PUT_PIXEL	;r9 = offset,	r10 = color
	AND	r2, r9, #0xfffffffc	;r2 = r10 & ~(0x11)
	AND	r3, r9, #0x3		;r3 = lower two bits of offset

	LDR	r4, [buffer, r2]	;r4 <= [lcd buffer + offset & ~(0x11)]

	;set clear bit
	CMP	r3, #0
	MOVEQ	r5, #0xff << 24
	MOVEQ	r10, r10, LSL #24

	CMP	r3, #1
	MOVEQ	r5, #0xff << 16
	MOVEQ	r10, r10, LSL #16

	CMP	r3, #2
	MOVEQ	r5, #0xff << 8
	MOVEQ	r10, r10, LSL #8

	CMP	r3, #3
	MOVEQ	r5, #0xff

	;r6 <= 0xffffffff
	MOV	r6, #0xff << 24
	ORR	r6, r6, #0xff << 16
	ORR	r6, r6, #0xff << 8
	ORR	r6, r6, #0xff 
	EOR	r5, r5, r6		;r5 <= ~r5
	AND	r4, r4, r5		;r4 <= r4 & r5

	ORR	r4, r4, r10		;r4 <= r4 | r1
	STR	r4, [buffer, r2]	;complete to put pixel

	MOV	pc, lr			;return


;==========================================
; Sub Routine for INITIATE VALUES FOR STAGE
;==========================================

INIT_VAL
	HMOV	F_RECHARGE, #0x7f << 4	; 적 미사일 장전 주기
	BMOV	FLAG, #0		; 적 모션
	BMOV	CL_FLAG, #0		; CLRSCR Flag
	BMOV	Flag_Shot, #0		; 아군 미사일 발사 유무
	BMOV	Flag_E_Shot, #0		; 적군 미사일 발사 유무
	BMOV	Draw_Unit_Flag, #0	; 아군 유닛 다시 그리기 유무
	BMOV	DIRE, #1		; 적군 이동방향 (좌:1, 우:0)

	;적군 시작 위치 좌표(XH, YH)
	HMOV	XH, #0
	HMOV	YH, #35		

	; 아군 시작 좌표
	HMOV	sX, #152
	HMOV	sY, #280	
	
	BMOV	DOWN, #0		; 적이 아래로 가는 회수
	HMOV	F_ALIEN, #0xff << 2	; 적 이동 주기
	;Timers
	HMOV	Time_Alien, #0
	HMOV	Time_Missile, #0
	HMOV	Time_Recharge, #0
	HMOV	Time_E_Missile, #0

	;총 적군수
	LDR	r2, =ROW
	LDR	r3, =COL
	LDRH	r9, [r2]		;r9 = ROW
	LDRH	r10, [r3]		;r10 = COL
	MUL	r4, r9, r10
	HMOV	COUNT, r4

	;적 정보 초기화

	

	
	MOV	pc, lr			;return


;==================================
; Sub Routine for Draw my spaceship
;==================================

DRAW_SPACESHIP
	LDR	r4, =SPACESHIP		;r4 <= addr of SPACESHIP

	LDR	r6, =sY			;r6 <= addr of sY
	LDRH	r7, [r6]		;r7 <= sY

	MOV	r6, #LCD_XSIZE		;r6 <= LCD_XSIZE(240)
	MUL	r5, r6, r7		;r5 <= sY * LCD_XSIZE

	LDR	r6, =sX			;r6 = adrr of sX
	LDRH	r7, [r6]		;r7 <= sX
	ADD	r5, r5, r7		;r5 <= sY * LCD_XSIZE + sX

	MOV	r2, #0			;r2 = i
DRAW_SPACESHIP_COL			;for(i = 0 ; i < 12 ; i++)
	MOV	r6, #LCD_XSIZE		;r6 <= LCD_SIZE(240)
	MUL	r9, r6, r2		;r9 <= 240 * i
	ADD	r9, r9, r5		;r9 <= sY * LCD_XSIZE + sX + 240 * i
	
	MOV	r3, #0			;r3 = j
DRAW_SPACESHIP_ROW			;for(j = 0 ; j < 16 ; j++)
	LDRB	r10, [r4], #1		;r10 <= SPACESHIP[i][j]
	STMFA	sp!, {r2 - r8, lr}	;push r2 - r8, lr
	BL	PUT_PIXEL		;call PUT_PIXEL
	LDMFA	sp!, {r2 - r8, lr}	;pup r2-r8, lr
	ADD	r9, r9, #1		;r9 <= sY * LCD_XSIZE + sX + 240 * i + j
	
	ADD	r3, r3, #1		;r3++
	CMP	r3, #23			;r3 = 23?
	BNE	DRAW_SPACESHIP_ROW	;if not, loop

	ADD	r2, r2, #1		;r2++
	CMP	r2, #15			;r2 = 15 ?
	BNE	DRAW_SPACESHIP_COL	;if not, loop

	MOV	pc, lr			;return



;==================================
; Sub Routine for Draw Enemies
;==================================

DRAW_ENEMY
	LDR	r6, =YH			;r6 <= addr of YH
	LDRH	r7, [r6]		;r7 <= YH
	
	STMFA	sp!, {score}
	LDR	score, =ENEMY		;r7 <= addr of ENEMY

	MOV	r10, #0			;r10 = l
DRAW_ENEMY_LOOP2			;(for l = 0 ; l < COL ; l++)
	CMP	r10, #0			;r10 == 0?
	LDREQ	r4, =ALIEN1		;r4 = addr of ALIEN1
	CMP	r10, #1			;r10 == 1?
	LDREQ	r4, =ALIEN2		;r4 = addr of ALIEN2
	CMP	r10, #2			;r10 == 2?
	LDREQ	r4, =ALIEN3		;r4 = addr of ALIEN3
	CMP	r10, #3			;r10 == 3?
	LDREQ	r4, =ALIEN4		;r4 = addr of ALIEN4
	CMP	r10, #4			;r10 == 4?
	LDREQ	r4, =ALIEN5		;r4 = addr of ALIEN5

; 모션 변환 추가 코드 시작
	LDR	r5, =FLAG		; r5 <= addr of FLAG
	LDRB	r6, [r5]		; r6 <= FLAG
	CMP	r6, #1			; r6 == 1?
	ADDEQ	r4, r4, #255		; r4 <= r4 + (17*15)
; 모션 변환 추가 코드 끝

	STMFA	sp!, {r10, r7}		;!!push r10, r7!!
	STMFA	sp!, {r4}		;push r4

	MOV	r6, #LCD_XSIZE		;r6 <= LCD_XSIZE(240)
	MUL	r5, r6, r7		;r5 <= YH * LCD_XSIZE
	LDR	r6, =XH			;r6 = adrr of XH
	LDRH	r7, [r6]		;r7 <= XH
	ADD	r5, r5, r7		;r5 <= YH * LCD_XSIZE + XH

;	LDR	r7, =ENEMY		;r7 <= addr of ENEMY

	MOV	r11, #0			;r11 = k
DRAW_ENEMY_LOOP1			;for(k = 0 ; k < ROW ; k++)
	LDRB	r6, [score], #1		;r6 = ENEMY[k]
	CMP	r6, #0			;r6 == 0?
	BEQ	SKIP_DRAW_ENEMY1	;then, branch

	LDR	r2, =XH			;r2 <= adrr of XH
	LDRH	r3, [r2]		;r3 <= XH
	MOV	r4, #20			;r4 <= 20
	MUL	r2, r11, r4		;r2 <= k * 20
	ADD	r2, r2, r3		;r2 <= k * 20 + XH
	LDR	r3, =DOWN		;r3 <= addr of DOWN
	LDRB	r4, [r3]		;r4 <= DOWN
	CMP	r4, #0			;DOWN == 0?
	BNE	SKIP_BOUND_CMP1		;if not, branch

	LDR	r3, =DIRE		;r3 <= addr of DIRE
	LDRB	r4, [r3]		;r6 <= DIRE
	CMP	r4, #0			;DIRE == 0?
	BNE	CHK_RIGHT_BOUND1	;if not, branch

	CMP	r2, #0			;k * 16 + XH == 0?
	BNE	SKIP_BOUND_CMP1		;if not, branch					

	MOV	r8, #1			;r8 <= 1
	STRB	r8, [r3]		;DIREC <= 1

	LDR	r6, =DOWN		;r6 <= addr of DOWN
	MOV	r8, #4			;r8 <= 4
	STRB	r8, [r6]		;DOWN <= 4
	B	SKIP_BOUND_CMP1		;unconditional branch

CHK_RIGHT_BOUND1
	CMP	r2, #230		;r2 <= 223?
	BLE	SKIP_BOUND_CMP1		;then branch
	
	MOV	r8, #0			;r8 <= 0
	STRB	r8, [r3]		;DIRE <= 0

	LDR	r6, =DOWN		;r6 <= addr of DOWN
	MOV	r8, #4			;r8 <= 4
	STRB	r8, [r6]		;DOWN <= 4

SKIP_BOUND_CMP1
	LDMFA	sp!, {r4}		;pop r4
	STMFA	sp!, {r4}		;push r4
;	LDR	r4, =ALIEN1		;r4 <= addr of ALIEN1
	
	MOV	r2, #0			;r2 = i
DRAW_ENEMY_COL1				;for(i = 0 ; i < 12 ; i++)
	MOV	r6, #LCD_XSIZE		;r6 <= LCD_SIZE(240)
	MUL	r9, r6, r2		;r9 <= 240 * i
	ADD	r9, r9, r5		;r9 <= YH * LCD_XSIZE + XH + 20 * k + 240 * i
	
	MOV	r3, #0			;r3 = j
DRAW_ENEMY_ROW1				;for(j = 0 ; j < 16 ; j++)
	LDRB	r10, [r4], #1		;r10 <= ALIENl[i][j]
	STMFA	sp!, {r2 - r8, lr}	;push r2 - r8, lr
	BL	PUT_PIXEL		;call PUT_PIXEL
	LDMFA	sp!, {r2 - r8, lr}	;pup r2-r8, lr
	ADD	r9, r9, #1		;r9 <= YH * LCD_XSIZE + XH + 20 * k + 240 * i + j

	ADD	r3, r3, #1		;r3++
	CMP	r3, #17			;r3 = 17 ?
	BNE	DRAW_ENEMY_ROW1		;if not, loop

	ADD	r2, r2, #1		;r2++
	CMP	r2, #15			;r2 = 15 ?
	BNE	DRAW_ENEMY_COL1		;if not, loop

SKIP_DRAW_ENEMY1
	ADD	r5, r5, #16		;r5 = (YH * LCD_XSIZE + XH) + 16 * k
	ADD	r11, r11, #1		;r11++
	CMP	r11, #11		;r11 == COL ?

	BNE	DRAW_ENEMY_LOOP1	;if not, loop

	LDMFA	sp!, {r4}		;push r4
	LDMFA	sp!, {r10, r7}		;push r10, r7
	ADD	r10, r10, #1		;r10++
	ADD	r7, r7, #20		;YH = YH + 20
	CMP	r10, #5			;r10 == 5?
	BNE	DRAW_ENEMY_LOOP2	;if not, loop
	MOV	pc, lr			;return


;==================================
; Sub Routine for MOVE Enemies
;==================================

MOVE_ENEMY
	LDR	r2, =DOWN		; r2 <= addr of DOWN
	LDRB	r3, [r2]		; r3 <= DOWN

	CMP	r3, #0			; r3 == 0?
	BNE	MOVE_ENEMY_DOWN		; if not, branch

	LDR	r2, =DIRE		; r2 <= addr of DIRE
	LDRB	r3, [r2]		; r3 <= DIRE
	CMP	r3, #0			; DIRE == 0?
	BEQ	MOVE_ENEMY_LEFT		; then, branch

MOVE_ENEMY_RIGHT
	HINC	XH			; XH++
	B	MOVE_ENEMY_END		; Unconditional branch

MOVE_ENEMY_LEFT
	HDEC	XH			; XH--
	B	MOVE_ENEMY_END		; Unconditional branch

MOVE_ENEMY_DOWN
	SUB	r3, r3, #1		; r3 <= r3 - 1
	STRB	r3, [r2]		; DOWN <= DOWN - 1
	LDR	r2, =YH			; r2 <= addr of YH
	LDRH	r3, [r2]		; r3 <= YH
	ADD	r3, r3, #2		; r3 = r3 + 2

;■■■■■■■■■■■■■■■■■■ 적군이 down bound 까지 내려오면 게임 종료 ■■■■■■■■■■■■■■■■■
	STRH	r3, [r2]		; YH <= YH + 2

MOVE_ENEMY_END
	LDMFA	sp!, {score}
	MOV	pc, lr			;return


;==================================
; Sub Routine for Draw User Missile
;==================================

DRAW_USER_MISSILE
	MOV	r11, lr			; r11 <= lr

	LDR	r2, =mY			; r2 <= addr of mY
	LDRH	r3, [r2]		; r3 <= mY

	LDR	r2, =mX			; r2 <= addr of mX
	LDRH	r4, [r2]		; r4 <= mX

	MOV	r7, #LCD_XSIZE		; r2 <= LCE_XSIZE(240)
	MUL	r5, r7, r3		; r5 <= mY * LCD_XSIZE
	ADD	r5, r5, r4		; r5 <= mY * LCD_XSIZE + mX

	MOV	r9, r5			; r9 <= r5
	CMP	r3, #20			; r3 <= 20?
	BLE	DRAW_USER_MISSILE_DEL	; then, branch

	; 미사일 적중을 판단하는 부분
	SUB	r9, r9, #960		; r9 <= r9 - LCD_XSIZE*4
	LDRB	r5, [buffer, r9]	;read pixel
	CMP	r5, #0			;r5 = 0
	BNE	CHECK_MISSILE_HIT	;if not, branch

DRAWING
	MOV	r10, #0			; r10 <= 0
	BL	PUT_PIXEL		; CALL PUT_PIXEL
	SUB	r9, r9, r7		; r9 <= r9 - LCD_XSIZE
	MOV	r10, #1			; r10 <= 1
	BL	PUT_PIXEL		; CALL PUT_PIXEL
	SUB	r9, r9, r7		; r9 <= r9 - LCD_XSIZE
	BL	PUT_PIXEL		; CALL PUT_PIXEL
	SUB	r9, r9, r7		; r9 <= r9 - LCD_XSIZE
	BL	PUT_PIXEL		; CALL PUT_PIXEL
	SUB	r9, r9, r7		; r9 <= r9 - LCD_XSIZE

	HDEC	mY
	B	DRAW_USER_MISSILE_END

CHECK_MISSILE_HIT
	LDR	r2, =mY			; r2 <= addr of mY
	LDRH	r3, [r2]		; r3 <= mY
	LDR	r2, =YH			; r2 <= addr of YH
	LDRH	r4, [r2]		; r4 <= YH
	SUB	r2, r3, r4		; r2 <= mY - YH

	LDR	r3, =mX			; r2 <= addr of mX
	LDRH	r4, [r3]		; r3 <= mX
	LDR	r3, =XH			; r2 <= addr of XH
	LDRH	r5, [r3]		; r4 <= XH
	SUB	r3, r4, r5		; r2 <= mX - XH

	MOV	r4, #0
CHECK_HIT_COL
	CMP	r2, #20			; r2 < 20?
	BLT	LOOP_OUT		; then, branch
	SUB	r2, r2, #20		; r2 <= r2 - 20
	ADD	r4, r4, #1		; r3 <= r3 + 1
	B	CHECK_HIT_COL		; Unconditional branch

LOOP_OUT
	MOV	r5, #0
CHECK_HIT_ROW
	CMP	r3, #20			; r3 < 20?
	BLT	LOOP_OUT2		; then, branch
	SUB	r3, r3, #20		; r3 <= r3 - 20
	ADD	r5, r5, #1		; r5 <= r5 + 1
	B	CHECK_HIT_ROW		; Unconditional branch

LOOP_OUT2
;	LDR	r2, =COL		; r2 <= addr of COL
;	LDRH	r3, [r2]		; r3 <= COL
	MOV	r6, #11			; r6 <= r6COL
	MUL	r3, r4, r6		; r3 <= r4 * 5
	ADD	r3, r3, r5		; r3 <= r3 + r5

	LDR	r2, =ENEMY		; r2 <= addr of ENEMY
	ADD	r2, r2, r3		; r2 <= r2 + r3
	MOV	r3, #0			; r3 <= 0
	STRB	r3, [r2]		; ENEMY[r2] <= 0

	ADD	score, score, #50	; score <= score + 50
	HDEC	COUNT			; count--
;■■■■■■■■■■■■■■■■■■■■■■ 적군이 다 죽으면 게임 종료 ■■■■■■■■■■■■■■■■■■■
	BMOV	CL_FLAG, #1		; CL_FLAG <= 1
	LDR	r2, =F_ALIEN		; r2 <= addr of F_ALIEN
	LDRH	r3, [r2]		; r3 <= F_ALIEN
	SUB	r3, r3, #15		; r3 <= r3 - 15
	STRH	r3, [r2]		; F_ALIEN <= r3

DRAW_USER_MISSILE_DEL
	MOV	r10, #0			; r10 <= 0
	BL	PUT_PIXEL		; CALL PUT_PIXEL
	SUB	r9, r9, r7		; r9 <= r9 - LCD_XSIZE
	BL	PUT_PIXEL		; CALL PUT_PIXEL
	SUB	r9, r9, r7		; r9 <= r9 - LCD_XSIZE
	BL	PUT_PIXEL		; CALL PUT_PIXEL
	SUB	r9, r9, r7		; r9 <= r9 - LCD_XSIZE
	BL	PUT_PIXEL		; CALL PUT_PIXEL
	SUB	r9, r9, r7		; r9 <= r9 - LCD_XSIZE

	BMOV	Flag_Shot, #0

DRAW_USER_MISSILE_END
	MOV	lr, r11			; lr <= r11
	MOV	pc, lr			;return


	;=================
	; CODE AREA2
	;=================

	AREA	Space_Invader2, CODE, READONLY

	EXPORT	ISREINT0
	EXPORT	ISREINT2
	EXPORT	ISREINT11
	EXPORT	ISREINT19

;==================================
; Sub Routine for Draw ENEMY Missile
;==================================

DRAW_ENEMY_MISSILE

	MOV	pc, lr			;return	


;=====================================
; Interupt Sub Routine for EINT0(LEFT)
;=====================================

ISREINT0
	LDR	r2, =sX			;r2 <= addr of sX
	LDRH	r3, [r2]		;r3 <= sX
	CMP	r3, #0			;r3 <= 0?
	BLE	ISREINT0_SKIP		;then, branch

	SUB	r3, r3, #4		;r3 <= r3 - 4
	STRH	r3, [r2]		;sX <= r3

	BMOV	Draw_Unit_Flag, #1
ISREINT0_SKIP
	MOV	pc, lr			;return


;======================================
; Interupt Sub Routine for EINT2(RIGHT)
;======================================

ISREINT2
	LDR	r2, =sX			;r2 <= addr of sX
	LDRH	r3, [r2]		;r3 <= sX
	CMP	r3, #217		;r3 >= 0?
	BGE	ISREINT0_SKIP		;then, branch

	ADD	r3, r3, #4		;r3 <= r3 + 4
	STRH	r3, [r2]		;sX <= r3

	BMOV	Draw_Unit_Flag, #1
ISREINT2_SKIP
	MOV	pc, lr			;return


;=====================================
; Interupt Sub Routine for EINT11(Shot)
;=====================================

ISREINT11
	LDR	r2, =Flag_Shot		;r2 <= addr of Flag_Shot
	LDRB	r3, [r2]		;r3 <= Flag_Shot
	CMP	r3, #1			;r3 == 1?
	BEQ	ISREINT11_SKIP		;then, branch
	BMOV	Flag_Shot, #1		;set Flag_Shot
	LDR	r2, =sX			;r2 <= addr of sX
	LDR	r3, =sY			;r3 <= addr of sY
	LDRH	r4, [r2]		;r4 <= sX
	LDRH	r5, [r3]		;r5 <= sY
	LDR	r2, =mX			;r2 <= addr of mX
	LDR	r3, =mY			;r3 <= addr of mY

	ADD	r4, r4, #11		;r4 <= sX + 11
	ADD	r5, r5, #1		;r5 <= sX + 1
	STRH	r4, [r2]		;mX = sX + 11
	STRH	r5, [r3]		;mY = sY + 1

ISREINT11_SKIP
	MOV	pc, lr			;return


;================================
; Interupt Sub Routine for EINT19
;================================

ISREINT19
	MOV	pc, lr			;return



	;=================
	; DATA AREA
	;=================

	AREA	Space_Invader_Data, DATA, READWRITE

;================================
; DATAS
;================================

LEFT_BOUND	EQU	0		; 왼쪽 한계
RIGHT_BOUND	EQU	300		; 오른쪽 한계
UP_BOUND	EQU	20		; 위쪽 한계
DOWN_BOUND	EQU	140		; 아래쪽 한계


DOWN_COUNT	EQU	4		; 적이 한번에 아래로 내려오는 횟수
S_F_ALIEN	EQU	5000
S_F_RECHARGE	EQU	6400
F_MISSILE	EQU	40
F_E_MISSILE	EQU	80
MAX_STAGE	EQU	9		; 스테이지 수
START_LIFE	EQU	3		; 최초 시작시 아군 목숨

XH		DCW	0	; 적군 기준 x좌표
YH		DCW	0	; 적군 기준 y좌표
sX		DCW	0	; 아군 x좌표
sY		DCW	0	; 아군 y좌표
mX		DCW	0	; 미사일 x좌표
mY		DCW	0	; 미사일 y좌표
eX		DCW	0	; 적 미사일 x좌표
eY		DCW	0	; 적 미사일 y좌표

ROW		DCW	0	; 적군 행 수
COL		DCW	0	; 적군 열 수
COUNT		DCW	0	; 적군의 수

FLAG		DCB	0	; 모션 움직임
D_FLAG		DCB	0
CL_FLAG		DCB	0
Draw_Unit_Flag	DCB	0
DOWN		DCB	0	;
Flag_Shot	DCB	0
Flag_E_Shot	DCB	0	
Time_Missile	DCW	0
Time_Alien	DCW	0
Time_Recharge	DCW	0
Time_E_Missile	DCW	0
F_ALIEN		DCW	0
F_RECHARGE	DCW	65000
DIRE		DCB	0	; 적군 진행 방향(좌, 우)
LIFE		DCB	3	; 아군 생명
STAGE		DCB	0	;

; ==== 적군 상태
ENEMY		DCB	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
		DCB	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
		DCB	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
		DCB	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1

; ==== 아군 디자인(23*15)
SPACESHIP	DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0

;색상정보
;9 :아주 짙은 파랑(거의 검정)
;7 :은회색
;8 :연한 주황색		-
;6 :더 연한 주황색
;4 :연한 하늘색		-
;14:짙 파랑		-
;15:흰색		-
;10:갈색
;18:분홍색		-
;20;회색
;22:보라색		-
;24;연초록		-
; ==== 적군 디자인(17*15)
ALIEN1		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 15, 15, 15, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 15, 15, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 15, 15, 0, 15, 15, 15, 0, 15, 15, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 15, 15, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 15, 15, 0, 15, 15, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 15, 0, 15, 0, 15, 0, 15, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 15, 0, 0, 15, 0, 15, 0, 0, 15, 0, 0, 0, 0
		DCB	0, 0, 0, 15, 0, 0, 15, 0, 0, 0, 15, 0, 0, 15, 0, 0, 0

		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 15, 15, 15, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 15, 15, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 15, 15, 0, 15, 15, 15, 0, 15, 15, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 15, 15, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 15, 0, 15, 0, 15, 0, 15, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 15, 0, 0, 15, 0, 15, 0, 0, 15, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 15, 0, 0, 15, 0, 15, 0, 0, 15, 0, 0, 0, 0

ALIEN2		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 18, 0, 0, 18, 18, 18, 18, 18, 0, 0, 18, 0, 0, 0
		DCB	0, 0, 0, 18, 0, 18, 18, 18, 18, 18, 18, 18, 0, 18, 0, 0, 0
		DCB	0, 0, 0, 18, 18, 18, 0, 18, 18, 18, 0, 18, 18, 18, 0, 0, 0
		DCB	0, 0, 0, 0, 18, 18, 18, 18, 18, 18, 18, 18, 18, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 18, 18, 18, 0, 18, 18, 18, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0

		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 18, 18, 18, 18, 18, 18, 18, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 18, 18, 0, 18, 18, 18, 0, 18, 18, 0, 0, 0, 0
		DCB	0, 0, 0, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 0, 0, 0
		DCB	0, 0, 0, 18, 0, 18, 18, 0, 0, 0, 18, 18, 0, 18, 0, 0, 0
		DCB	0, 0, 0, 18, 0, 0, 18, 18, 18, 18, 18, 0, 0, 18, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0

ALIEN3		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 24, 24, 24, 24, 24, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0
		DCB	0, 0, 0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0
		DCB	0, 0, 0, 24, 24, 0, 24, 24, 24, 24, 24, 0, 24, 24, 0, 0, 0
		DCB	0, 0, 0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0
		DCB	0, 0, 0, 0, 24, 24, 24, 24, 0, 24, 24, 24, 24, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 24, 24, 0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0

		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 24, 24, 24, 24, 24, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0
		DCB	0, 0, 0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0
		DCB	0, 0, 0, 24, 24, 0, 24, 24, 24, 24, 24, 0, 24, 24, 0, 0, 0
		DCB	0, 0, 0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0
		DCB	0, 0, 0, 0, 24, 24, 24, 0, 0, 0, 24, 24, 24, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 24, 24, 0, 24, 24, 0, 0, 0, 0, 0, 0

ALIEN4		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0, 0
		DCB	0, 0, 0, 8, 8, 0, 0, 8, 8, 8, 0, 0, 8, 8, 0, 0, 0
		DCB	0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0, 0, 0
		DCB	0, 0, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0, 0, 8, 0, 0, 0
		DCB	0, 0, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0, 0, 8, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0, 0, 0

		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0, 0
		DCB	0, 0, 0, 8, 8, 0, 0, 8, 8, 8, 0, 0, 8, 8, 0, 0, 0
		DCB	0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 0, 0, 8, 0, 8, 0, 0, 8, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 8, 0, 0, 8, 0, 8, 0, 0, 8, 0, 0, 0, 0
		DCB	0, 0, 0, 8, 0, 0, 8, 0, 0, 0, 8, 0, 0, 8, 0, 0, 0
		DCB	0, 0, 0, 8, 0, 0, 8, 0, 0, 0, 8, 0, 0, 8, 0, 0, 0
		DCB	0, 0, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0, 0, 8, 0, 0, 0

ALIEN5		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 14, 0, 0, 0, 0, 14, 0, 0, 0, 0, 14, 0, 0, 0
		DCB	0, 0, 0, 0, 14, 14, 0, 14, 14, 14, 0, 14, 14, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 14, 14, 14, 14, 14, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 14, 14, 14, 0, 14, 14, 14, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 14, 14, 14, 14, 0, 0, 0, 14, 14, 14, 14, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 14, 14, 0, 0, 0, 14, 14, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 14, 14, 14, 0, 14, 14, 14, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 14, 0, 14, 14, 14, 14, 14, 0, 14, 0, 0, 0, 0
		DCB	0, 0, 0, 14, 0, 0, 14, 0, 0, 0, 14, 0, 0, 14, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0

		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 14, 0, 14, 14, 14, 0, 14, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 14, 14, 14, 14, 14, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 14, 14, 14, 14, 14, 14, 14, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 14, 14, 14, 14, 0, 14, 14, 14, 14, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 14, 14, 14, 0, 14, 14, 14, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 14, 14, 14, 14, 14, 14, 14, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 14, 0, 14, 14, 14, 14, 14, 0, 14, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 14, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	
; ==== 적 파괴시 폭발 모양(17*15)
BANG		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0
		DCB	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

; ==== 아군 파괴시 폭발 모양(23*15)
EXPLOSION	DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 6, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 0, 4, 0, 0, 4, 0, 0, 0, 0, 0
		DCB	0, 0, 4, 0, 0, 0, 0, 0, 4, 6, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 4, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 4, 6, 0, 0, 6, 4, 4, 4, 4, 4, 4, 0, 4, 6, 0, 0, 0, 4, 0, 0
		DCB	0, 0, 0, 0, 4, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 6, 0, 0, 0
		DCB	0, 0, 0, 0, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 0, 0, 0, 0
		DCB	0, 0, 4, 4, 0, 0, 6, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0
		DCB	0, 0, 0, 4, 4, 4, 4, 6, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 6, 6, 4, 0, 0
		DCB	0, 0, 0, 0, 0, 6, 6, 4, 6, 4, 6, 4, 4, 4, 4, 4, 4, 6, 4, 0, 0, 0, 0

		DCB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 4, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 4, 0, 0, 4, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 6, 0, 0, 0, 6, 4, 0, 0, 0, 0, 4, 4, 6, 4, 0, 0, 0, 0, 0
		DCB	0, 0, 0, 0, 4, 6, 0, 0, 0, 6, 4, 0, 0, 0, 4, 4, 4, 0, 0, 6, 4, 0, 0
		DCB	0, 0, 0, 0, 0, 4, 4, 0, 0, 4, 4, 0, 0, 4, 6, 4, 0, 0, 4, 0, 0, 0, 0
		DCB	0, 0, 6, 0, 0, 0, 4, 4, 0, 4, 4, 4, 0, 4, 4, 4, 4, 4, 6, 0, 0, 0, 0
		DCB	0, 0, 4, 0, 0, 0, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 4, 0, 0
		DCB	0, 0, 0, 6, 0, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 0, 0, 6, 4, 0, 0
		DCB	0, 0, 0, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 4, 4, 6, 4, 0, 0, 0
		DCB	0, 0, 0, 0, 4, 6, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0
		DCB	0, 0, 0, 0, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 0, 0, 0
		DCB	0, 0, 0, 0, 0, 4, 4, 4, 6, 4, 6, 6, 4, 4, 4, 4, 4, 4, 6, 0, 0, 0, 0
	END