;=============================================================================
;				매크로
;=============================================================================
CURSOR	MACRO	column, row
	MOV	AH, 02h
	MOV	BH, 00
	MOV	DL, column
	MOV	DH, row
	INT 	10h
	ENDM

;=============================================================================
;				코드 세그먼트
;=============================================================================
CODE	SEGMENT
MAIN:
	ASSUME	CS:CODE, DS:DATA
	MOV	AX, DATA
	MOV	DS, AX

	MOV	AH, 0Fh
	INT	10h

	MOV	MODE, AL
	MOV	AL, 13h
	MOV	AH, 00h
	INT	10H		; 그래픽 모드(320*200, 256색)

	CLD			; 방향 플래그 clear

START:
;최초 변수 초기화
	MOV	SCORE, 0
	MOV	LIFE, START_LIFE
	MOV	STAGE, 0
	MOV	F_RECHARGE, S_F_RECHARGE

	CALL	CLS

	CURSOR	7,10		; 커서 이동
	MOV	AH, 09h
	MOV	DX, OFFSET Msg_GameName	; 시작 메세지 출력
	INT	21h

	CURSOR	13,14		; 커서 이동
	MOV	AH, 09h
	MOV	DX, OFFSET Msg_AnyKey	; 시작 메세지 출력
	INT	21h

	MOV	AH, 00h
	INT	16h		; 키보드 키 입력 대기

NEXT_STAGE:
	CMP	STAGE, MAX_STAGE
	JAE	ENDING

	SUB	F_RECHARGE, 5000
	CALL	CLS
	CALL	INIT_VAL
	INC	STAGE

	CURSOR	14,10		; 커서 이동
	MOV	BX, OFFSET Msg_Stage	; 시작 메세지 출력
	MOV	AL, STAGE
	ADD	AL, '0'
	MOV	[BX+10], AL
	MOV	AH, 09h
	MOV	DX, BX
	INT	21h

	MOV	AH, 00h
	INT	16h		; 키보드 키 입력 대기
	CALL	CLS

PRO_1:
	MOV	AX, Time_Alien
	CMP	AX, F_ALIEN
	JB	CHECK_UFO

	CMP	CL_FLAG, 1
	JNE	SKIP_CLS
	CALL	CLS		; clear screen
	DEC	CL_FLAG
SKIP_CLS:
	CALL	Print_Score	; 점수 출력
	CALL	Print_State
	CALL	DRAW_UNIT	; 아군 유닛 그리기

; =========== 적군 출력하는 부분 시작
	MOV	DI, OFFSET ENEMY
	CMP	FLAG, 1
	JZ	L_FLAG1
	MOV	FLAG, 1
	MOV	M_OFF, 169
	JMP	L_FLAG2
L_FLAG1:
	MOV	M_OFF, 0
	MOV	FLAG, 0
L_FLAG2:

	MOV	YOFFSET, 0
	XOR	CX, CX
PRO_2:
	PUSH	CX
	MOV	XOFFSET, 0
	MOV	SI, OFFSET ALIEN1
	ADD	SI, M_OFF

	MOV	BX, OFFSET R_KIND
	ADD	BX, CX
	MOV	AL, [BX]
	XOR	AH, AH
	MOV	DX, 338
	MUL	DX
	ADD	SI, AX

	MOV	BX, OFFSET R_COLOR
	ADD	BX, CX
	MOV	AL, [BX]
	MOV	Line_Color, AL

	MOV	CX, COL
PRO_3:
	PUSH	CX

	MOV	AL, [DI]
	CMP	AL, 0
	JBE	PRO_SKIP	; 적군이 죽었으면 SKIP

	MOV	AX, XH
	ADD	AX, XOFFSET

	CMP	DOWN, 0
	JA	PRO_32
	MOV	BX, YH
	ADD	BX, YOFFSET
	CMP	BX, DOWN_BOUND
	JAE	GAME_OVER	; 한계점이하로 내려가면 게임 종료

	CMP	DIRE, 1		; 이동방향
	JE	PRO_31
	CMP	AX, 0		; left bound
	JA	PRO_32
	MOV	DIRE, 1		; 오른쪽
	MOV	DOWN, DOWN_COUNT
	JMP	PRO_32
PRO_31:
	CMP	AX, 300		; right bound
	JB	PRO_32
	MOV	DIRE, 0		; 왼쪽
	MOV	DOWN, DOWN_COUNT
PRO_32:
	CALL	IMAGE		; 적군 유닛 그리기
	SUB	SI, 169
PRO_SKIP:
	INC	DI
	ADD	XOFFSET, 18
	POP	CX
	LOOP	PRO_3

	ADD	YOFFSET, 18
	POP	CX
	INC	CX
	CMP	CX, ROW
	JB	PRO_2
; ----------------------- 적군 출력 부분 끝

;================== UFO 출현 부분 시작
CHECK_UFO:
	CMP	Time_UFO, F_UFO
	JB	MOVE_UFO
	MOV	Time_UFO, 0
	CMP	UFO_FLAG, 1
	JAE	MOVE_UFO

	CALL	RANDOM_UFO
;------------------ UFO 출현 부분 끝

;================== UFO 출력부분 시작
MOVE_UFO:
	CMP	Time_UFO_Move, F_UFO_MOVE
	JB	CHECK_RECHARGE
	MOV	Time_UFO_Move, 0
	CMP	UFO_FLAG, 1
	JB	CHECK_RECHARGE
	CALL	DRAW_UFO		; UFO 그리기
;------------------ UFO 출력부분 끝

CHECK_RECHARGE:
	MOV	AX, Time_Recharge
	CMP	AX, F_RECHARGE
	JB	CHECK_MISSILE
	MOV	Time_Recharge, 0

	MOV	SI, OFFSET emF
	MOV	DI, OFFSET emX
	MOV	BX, OFFSET emY

CHECK_FLAGS:
	MOV	AL, [SI]
	INC	SI
	ADD	DI, 2
	ADD	BX, 2
	CMP	AL, 1
	JAE	CHECK_FLAGS
	DEC	SI
	SUB	DI, 2
	SUB	BX, 2

;	CALL	RANDOM_SHOOT
;	CMP	RND_COL, 99
;	JAE	CHECK_MISSILE

	PUSH	BX
RANDOMIZE:
	CALL	RANDOM
	MOV	BX, OFFSET ENEMY
	ADD	BX, RND_COL
	XOR	DX, DX
COMPARE:
	MOV	AH, [BX]
	ADD	BX, COL
	INC	DL
	CMP	AH, 0
	JBE	PASS
	MOV	AL, 1
	MOV	[SI], AL
	MOV	AL, DL
PASS:
	CMP	DX, ROW
	JB	COMPARE
	MOV	DL, [SI]
	CMP	DL, 1
	JNE	RANDOMIZE

	POP	BX

	XOR	AH, AH
	MOV	DL, 18
	MUL	DL
	ADD	AX, YH
	MOV	[BX], AX
	MOV	AX, RND_COL
	MOV	DL, 18
	MUL	DL
	ADD	AX, XH
	ADD	AX, 6
	MOV	[DI], AX

CHECK_MISSILE:
; ======================= 미사일 출력 부분 시작
	CMP	Flag_Shot, 1
	JNE	CHECK_ENEMY_MISSILE
	CMP	Time_Missile, F_MISSILE
	JB	CHECK_ENEMY_MISSILE
	MOV	Time_Missile, 0

	CALL	DRAW_MISSILE	; 미사일 출력
; ------------------------ 미사일 출력 부분 끝

	CMP	COUNT, 0	; 적이 다 죽었을 경우
	JE	NEXT_STAGE	; 다음 단계로 이동

CHECK_ENEMY_MISSILE:
; ======================== 적군 미사일 발사 부분 시작
	CMP	Time_E_Missile, F_E_MISSILE
	JB	CHECK_ALIEN_MOVE
	MOV	Time_E_Missile, 0

	PUSH	CX
	MOV	SI, OFFSET emF
	MOV	DI, OFFSET emX
	MOV	BX, OFFSET emY
	MOV	CX, 10
R10:
	MOV	AL, [SI]
	CMP	AL, 1
	JB	NO_SHOOT
	MOV	AX, [DI]
	MOV	eX, AX
	MOV	AX, [BX]
	MOV	eY, AX
	PUSH	BX
	CALL	DRAW_E_MISSILE

	POP	BX
	MOV	AX, eY
	MOV	[BX], AX
NO_SHOOT:
	INC	SI
	ADD	DI, 2
	ADD	BX, 2
	LOOP	R10

	POP	CX
; ------------------------ 적군 미사일 발사 부분 끝

	CMP	LIFE, 0
	JBE	GAME_OVER

CHECK_ALIEN_MOVE:
	MOV	AX, Time_Alien
	CMP	AX, F_ALIEN
	JB	NO_MOVE
	MOV	Time_Alien, 0

; ======================= 적군 이동 부분 시작
	CMP	DOWN, 0		; 아래로 이동
	JBE	MOVE_RIGHT
	ADD	YH, 2
	DEC	DOWN
	JMP	NO_MOVE
MOVE_RIGHT:
	CMP	DIRE, 1		; 이동방향 좌우에 따라 열 이동
	JE	MOVE_LEFT
	DEC	XH
	JMP	NO_MOVE
MOVE_LEFT:
	INC	XH
; ------------------------ 적군 이동 부분 끝

NO_MOVE:
	INC	Time_Alien
	INC	Time_Missile
	INC	Time_Recharge
	INC	Time_E_Missile
	INC	Time_UFO
	INC	Time_UFO_Move

; ========== 시간 지연 부분
	MOV	CX, 30000
DELAY:	LOOP	DELAY
; -------------------------

;========== 사용자 입력
	MOV	AH, 01h
	INT	16H		; 키보드 상태 확인
	JZ	NO_KEY		; 키보드가 안 눌렸으면 분기
	MOV	AH, 00h
	INT	16h		; 키보드로부터 문자 입력받음

	CMP	AL, 'q'
	JE	END_GAME	; 입력키 = q이면 loop 탈출(종료)

KEY_1:
	CMP	AH, 75		; 입력키 = 왼쪽 화살표
	JNE	KEY_2
	CMP	sX, 0
	JBE	NO_KEY		; 현재 위치가 left bound이면 점프
	DEC	sX
	CALL	DRAW_UNIT
	JMP	NO_KEY

KEY_2:
	CMP	AH, 77		; 입력키 = 오른쪽 화살표
	JNE	KEY_3
	CMP	sX, 300
	JAE	NO_KEY		; 현재 위치가 right bound이면 점프
	INC	sX
	CALL	DRAW_UNIT
	JMP	NO_KEY

KEY_3:				; 미사일 발사
	CMP	AL, 32		; 입력키 = space bar
	JNE	KEY_4
	CMP	Flag_Shot, 1
	JE	NO_KEY		; 이미 발사했으면 SKIP
	MOV	Flag_Shot, 1
	MOV	AX, sX
	ADD	AX, 8
	MOV	mX, AX
	MOV	mY, 160

KEY_4:
	CMP	AL, 'n'
;	MOV	CL_FLAG, 3
	JE	NEXT_STAGE

NO_KEY:
	JMP	PRO_1

;===================== 게임 오버 =========================
GAME_OVER:
	CALL	CLS
	CURSOR	12, 8
	MOV	AH, 09h
	MOV	DX, OFFSET Msg_GameOver
	INT	21h
	JMP	COMPARE_SCORE

ENDING:
	CALL	CLS
	CURSOR	15, 8
	MOV	AH, 09h
	MOV	DX, OFFSET Msg_Ending
	INT	21h

COMPARE_SCORE:
	CURSOR	12, 12
	MOV	AH, 09h
	MOV	DX, OFFSET Msg_LastScore
	INT	21h
	MOV	DX, OFFSET Int_Score
	INT	21h
	
	MOV	AX, SCORE
	CMP	AX, T_SCORE
	JB	SKIP_RECORD

	MOV	T_SCORE, AX
	MOV	SI, OFFSET Int_Score
	MOV	DI, OFFSET Int_TopScore
	MOV	CX, 6

COPY_SCORE:
	MOV	AL, [SI]
	MOV	[DI], AL
	MOV	AL, ' '
	MOV	[SI], AL
	INC	SI
	INC	DI
	LOOP	COPY_SCORE

	CURSOR	15, 14
	MOV	AH, 09h
	MOV	DX, OFFSET Msg_NewRecord	; 기록갱신 문자 출력
	INT	21h

SKIP_RECORD:
	CURSOR	5, 20
	MOV	AH, 09h
	MOV	DX, OFFSET Msg_Restart
	INT	21h

ANSWER:
	MOV	AH, 00h
	INT	16h		; 키보드로부터 문자 입력받음
	CMP	AL, 32
	JE	START		; 입력키가 스페이스바이면 재시작
	CMP	AL, 'q'
	JNE	ANSWER		; 입력키 = q이면 종료 아니면 계속 입력 받음

;===================== 게임 종료 =========================
END_GAME:
	CALL	CLS

	CURSOR	15, 10
	MOV	AH, 09h
	MOV	DX, OFFSET Msg_End
	INT	21h

	MOV	AH, 00H		; 사용자 키 입력
	INT	16H

	MOV	AL, MODE	; 원래 모드로 복귀
	MOV	AH, 00H
	INT	10H

	MOV	AH, 4CH		; 프로그램 종료
	INT	21H

;=============================================================================
;				프로시저
;=============================================================================

;==== 변수 초기화 ====
INIT_VAL	PROC	NEAR
	MOV	FLAG, 0
	MOV	CL_FLAG, 0
	MOV	Flag_Shot, 0
	MOV	UFO_FLAG, 0
	MOV	DIRE, 1		; 적군 이동방향 (좌:1, 우:0)
	MOV	emF, 0

	MOV	BX, OFFSET E_ROWS
	MOV	DL, STAGE
	MOV	DH, 0

	ADD	BX, DX
	MOV	AL, [BX]
	XOR	AH, AH
	MOV	ROW, AX		; 적군 행 수

	MOV	BX, OFFSET E_COLS
	ADD	BX, DX
	MOV	AL, [BX]
	XOR	AH, AH
	MOV	COL, AX		; 적군 열 수

	MOV	XH, 0
	MOV	YH, 35		; 적군 시작 위치 좌표(XH, YH)
	MOV	sX, 150
	MOV	sY, 165		; 아군 시작 좌표
	MOV	DOWN, 0

	MOV	F_ALIEN, 6000
	MOV	Time_Alien, 0
	MOV	Time_Missile, 0
	MOV	Time_Recharge, 0
	MOV	Time_E_Missile, 0
	MOV	Time_UFO, 0
	MOV	Time_UFO_Move, 0

	MOV	AX, ROW
	MOV	DX, COL
	MUL	DX
	MOV	COUNT, AX	; 총 적군 수

	MOV	BX, OFFSET ENEMY
	MOV	SI, OFFSET R_KIND
	MOV	DI, OFFSET E_INFO
	XOR	AH, AH
	MOV	AL, STAGE
	MOV	DL, 15
	MUL	DL
	ADD	DI, AX

	XOR	CX, CX
INIT_ROW:
	PUSH	CX

	MOV	AL, [DI]
	MOV	[SI], AL	; 모양

	PUSH	BX
	MOV	BX, OFFSET R_COLOR
	ADD	BX, CX
	MOV	AL, [DI+2]	; 색상
	MOV	[BX], AL
	POP	BX

	MOV	CX, COL
INIT_COL:
	MOV	AL, [DI+1]	; 에너지
	MOV	[BX], AL
	INC	BX
	LOOP	INIT_COL

	POP	CX
	ADD	DI, 3
	INC	SI
	INC	CX
	CMP	CX, ROW
	JB	INIT_ROW

	MOV	CX, 10
	MOV	SI, OFFSET emF
CLEAN:
	MOV	[SI], 0
	INC	SI
	LOOP	CLEAN
	RET
INIT_VAL	ENDP

;==== 화면 지우기 ====
CLS	PROC	NEAR
	MOV	BX, 64000	; 320*200=64000
	MOV	AX, 0A000H
	MOV	ES, AX

L_CLS:
	MOV	ES:[BX], 0h	; 화면 전체에 검정색 점을 찍는다.
	DEC	BX
	JNZ	L_CLS
	RET
CLS	ENDP

;==== 점수 출력 ====
Print_Score	PROC	NEAR
	CURSOR	3, 0
	MOV	AH, 09h
	MOV	DX, OFFSET Str_Score
	INT	21h

;현재 점수 출력
	MOV	AX, SCORE
	MOV	SI, OFFSET Int_Score
	ADD	SI, 5
	XOR	DX, DX
	MOV	BX, 10

REPEAT_DIV:
	DIV	BX
	ADD	DL, '0'
	MOV	[SI], DL
	DEC	SI
	XOR	DX, DX
	CMP	AX, 0
	JA	REPEAT_DIV

	CURSOR	2, 1
	MOV	AH, 09h
	MOV	DX, OFFSET Int_Score
	INT	21h

;최고 점수 출력
	CURSOR	16, 1
	MOV	AX, SCORE
	CMP	AX, T_SCORE
	JAE	L_SCORE
	MOV	DX, OFFSET Int_TopScore
	JMP	END_SCORE
L_SCORE:
	MOV	DX, OFFSET Int_Score
END_SCORE:
	MOV	AH, 09h
	INT	21h
	RET
Print_Score	ENDP

;==== 상태바 출력 ====
Print_State	PROC	NEAR
	MOV	CO, 15
	MOV	Y, 180
	MOV	X, 0
	MOV	CX, 310
W_LINE:
	INC	X
	CALL	PSET		; 흰 선 그리기
	LOOP	W_LINE

	CURSOR	1, 23
	MOV	AH, 02h
	MOV	DL, LIFE
	ADD	DL, '0'
	INT	21h		; 생명 출력(숫자)

	CMP	LIFE, 1
	JBE	END_STATE
	PUSH	sX
	PUSH	sY
	MOV	sX, 18
	MOV	sY, 181
	MOV	CL, LIFE
	XOR	CH, CH
	DEC	CL
DRAW_LIFE:
	PUSH	CX
	CALL	DRAW_UNIT	
	POP	CX
	ADD	sX, 18
	LOOP	DRAW_LIFE
	POP	sY
	POP	sX
END_STATE:
	RET
Print_State	ENDP

;==== 적군 출력 ====
IMAGE	PROC	NEAR
	MOV	AX, YH
	ADD	AX, YOFFSET
	MOV	Y, AX
	MOV	CX, 13		; 행
L_IM1:
	PUSH	CX
	MOV	AX, XH
	ADD	AX, XOFFSET
	MOV	X, AX
	MOV	CX, 13		; 열
L_IM2:
	LODSB
	CMP	AL, 0
	JE	IM_SKIP

	MOV	AL, Line_Color
	MOV	BL, [DI]
	CMP	BL, 1
	JBE	IM_SKIP
	ADD	AL, 8
IM_SKIP:
	MOV	CO, AL		; CO = 색상

	CALL	PSET

	INC	X
	LOOP	L_IM2

	INC	Y
	POP	CX
	LOOP	L_IM1
	RET
IMAGE	ENDP

;==== 아군 출력 ====
DRAW_UNIT	PROC	NEAR
	MOV	SI, OFFSET UNIT
	MOV	AX, sY
	MOV	Y, AX
	MOV	CX, 13		; 행
L_DRAW1:
	PUSH	CX
	MOV	AX, sX
	MOV	X, AX
	MOV	CX, 17		; 열
L_DRAW2:
	LODSB
	MOV	CO, AL		; CO = 색상
	CALL	PSET

	INC	X
	LOOP	L_DRAW2

	INC	Y
	POP	CX
	LOOP	L_DRAW1
	RET
DRAW_UNIT	ENDP

;==== 미사일 출력 ====
DRAW_MISSILE	PROC	NEAR
	CMP	mY, UP_BOUND
	JBE	DEL_MISSILE

; 미사일 적중을 판단하는 부분
	MOV	AX, 0A000H
	MOV	ES, AX

	MOV	AX, mY
	SUB	AX, 4
	MOV	BX, 320
	MUL	BX

	ADD	AX, mX
	MOV	BX, AX

	MOV	AL, ES:[BX]	; 픽셀의 값을 읽어온다.
	CMP	AL, 0
	JE	DRAW_M

; 미사일 적중시 처리 부분
	CALL	MISSILE_HIT
;	CALL	CLS
	JMP	DEL_MISSILE

DRAW_M:				; 미사일을 화면에 그리는 부분
	MOV	AX, mX
	MOV	X, AX
	MOV	CO, 0
	MOV	AX, mY
	MOV	Y, AX
	CALL	PSET
	DEC	Y
	MOV	CO, 10
	CALL	PSET
	DEC	Y
	CALL	PSET
	DEC	Y
	CALL	PSET

	DEC	mY		; 미사일 이동
	JMP	END_MISSILE

DEL_MISSILE:
	MOV	Flag_Shot, 0	; 미사일 삭제
	MOV	Time_Missile, 0

	MOV	AX, mX
	MOV	X, AX
	MOV	CO, 0
	MOV	AX, mY
	MOV	Y, AX
	CALL	PSET
	DEC	Y
	CALL	PSET
	DEC	Y
	CALL	PSET

	MOV	AX, SCORE
	CMP	AX, T_SCORE
	JBE	END_MISSILE
	MOV	T_SCORE, AX
END_MISSILE:
	RET
DRAW_MISSILE	ENDP

;==== 적 미사일 출력 ====
DRAW_E_MISSILE	PROC	NEAR
	CMP	eY, 179
	JAE	DEL_E_MISSILE

; 미사일 적중을 판단하는 부분
	MOV	AX, 0A000H
	MOV	ES, AX

	MOV	AX, eY
	MOV	BX, 320
	MUL	BX

	ADD	AX, eX
	MOV	BX, AX

	MOV	AL, ES:[BX]	; 픽셀의 값을 읽어온다.
	CMP	AL, 0
	JE	DRAW_EM

; 적 미사일 적중시 처리 부분
	CALL	ENEMY_HIT
	JMP	DEL_E_MISSILE

DRAW_EM:			; 미사일을 화면에 그리는 부분
	MOV	AX, eX
	MOV	X, AX
	MOV	CO, 14
	MOV	AX, eY
	MOV	Y, AX
	CALL	PSET
	DEC	Y
	CALL	PSET
	DEC	Y
	CALL	PSET
	DEC	Y
	MOV	CO, 0
	CALL	PSET

	INC	eY		; 미사일 이동
	JMP	END_E_MISSILE

DEL_E_MISSILE:
	XOR	AL, AL
	MOV	[SI], AL	; 미사일 삭제

	MOV	AX, eX
	MOV	X, AX
	MOV	CO, 0
	MOV	AX, eY
	MOV	Y, AX
	CALL	PSET
	DEC	Y
	CALL	PSET
	DEC	Y
	CALL	PSET
	DEC	Y
	CALL	PSET

END_E_MISSILE:
	RET
DRAW_E_MISSILE	ENDP

;==== 적 파괴 ====
MISSILE_HIT	PROC	NEAR
	MOV	AX, mY
	CMP	AX, YH
	JBE	UFO_HIT
	
	MOV	AX, mX		; 열 계산
	SUB	AX, XH
	XOR	DX, DX
	MOV	BX, 18
	DIV	BX

	CMP	AX, 0		; 해당 범위 확인
	JB	M_EXPLOSION
	CMP	AX, COL
	JAE	M_EXPLOSION
	MOV	Temp, AX	; 임시변수에 열 값 저장
	MUL	BX
	MOV	XOFFSET, AX

	MOV	AX, mY		; 행 계산
	SUB	AX, 4
	SUB	AX, YH
	DIV	BX

	MOV	SI, OFFSET R_COLOR
	ADD	SI, AX
	MOV	DH, [SI]
	MOV	Line_Color, DH
	MOV	SI, OFFSET R_KIND
	ADD	SI, AX
	MOV	DH, [SI]
	MOV	BONUS, DH

	CMP	AX, 0		; 해당 범위 확인
	JB	M_EXPLOSION
	CMP	AX, ROW
	JAE	M_EXPLOSION
	PUSH	AX
	MUL	BX
	MOV	YOFFSET, AX
	POP	AX

	MOV	DX, COL		; 합산
	MUL	DX
	ADD	AX, Temp

; 폭파
	MOV	DI, OFFSET ENEMY
	ADD	DI, AX
	MOV	AL, [DI]
	CMP	AL, 0
	JBE	M_EXPLOSION
	DEC	AL
	MOV	[DI], AL
	CMP	AL, 0
	JA	END_DEL
	DEC	COUNT
	MOV	SI, OFFSET BANG
	CALL	IMAGE

	ADD	SCORE, 10
	MOV	AL, BONUS
	MOV	DL, 10
	MUL	DL
	ADD	SCORE, AX

	SUB	F_ALIEN, 100
	MOV	CL_FLAG, 1
	JMP	END_DEL

UFO_HIT:
	ADD	SCORE, 100
	INC	LIFE
	MOV	UFO_FLAG, 0
;	CALL	UFO_EXPLOSION
	MOV	CL_FLAG, 0

M_EXPLOSION:
	;만들든지 말든지..	

END_DEL:
	RET
MISSILE_HIT	ENDP

;==== 적군의 공격 적중 ====
ENEMY_HIT	PROC	NEAR
	MOV	AX, eY
	CMP	AX, sY
	JBE	END_HIT
	MOV	AX, eX
	CMP	AX, sX
	JBE	END_HIT

	DEC	LIFE
	CALL	DESTROY
END_HIT:
	RET
ENEMY_HIT	ENDP

;==== 아군 파괴 애니메이션 ====
DESTROY	PROC	NEAR
	PUSH	CX
	PUSH	SI
	MOV	CX, 8
	MOV	D_FLAG, 0
	MOV	SI, OFFSET EXPLOSION
DRAW_DESTROY:
	PUSH	CX
	MOV	AX, sY
	MOV	Y, AX
	MOV	CX, 13		; 행

DRAW_ROW:
	PUSH	CX
	MOV	AX, sX
	MOV	X, AX
	MOV	CX, 17		; 열

DRAW_COL:
	LODSB
	MOV	CO, AL		; CO = 색상
	CALL	PSET
	INC	X
	LOOP	DRAW_COL

	INC	Y
	POP	CX
	LOOP	DRAW_ROW

	MOV	CX, 50000
D_DELAY1:
	PUSH	CX
	MOV	CX, 3000
D_DELAY2:
	LOOP	D_DELAY2
	POP	CX
	LOOP	D_DELAY1

	CMP	D_FLAG, 1
	JNE	SKIP_M
	SUB	SI, 442
	MOV	D_FLAG, 0
	JMP	SKIP_M2
SKIP_M:
	MOV	D_FLAG, 1
SKIP_M2:
	POP	CX
	LOOP	DRAW_DESTROY
	POP	SI
	POP	CX
	MOV	CL_FLAG, 1
	RET
DESTROY	ENDP

;==== UFO 그리기 =====
DRAW_UFO	PROC	NEAR
	CMP	UFO_FLAG, 1
	JE	UFO_RIGHT_BOUND
	CMP	uX, 0
	JAE	DRAW_UFO_START
	MOV	UFO_FLAG, 0
	MOV	CO, 0
	JMP	DRAW_UFO_START
UFO_RIGHT_BOUND:
	CMP	uX, 300
	JBE	DRAW_UFO_START
	MOV	UFO_FLAG, 0
	MOV	CO, 0

DRAW_UFO_START:
	MOV	SI, OFFSET UFO
	MOV	AX, uY
	MOV	Y, AX
	MOV	CX, 13		; 행

DRAW_UFO_ROW:
	PUSH	CX
	MOV	AX, uX
	MOV	X, AX
	MOV	CX, 17		; 열
DRAW_UFO_COL:
	CMP	UFO_FLAG, 0
	JE	NO_READ_UFO
	LODSB
	MOV	CO, AL		; CO = 색상
NO_READ_UFO:
	CALL	PSET
	INC	X
	LOOP	DRAW_UFO_COL
	INC	Y
	POP	CX
	LOOP	DRAW_UFO_ROW

	CMP	UFO_FLAG, 1
	JE	INC_uX
	DEC	uX
	JMP	END_DRAW_UFO
INC_uX:
	INC	uX

END_DRAW_UFO:
	RET
DRAW_UFO	ENDP

;==== 점 찍기 ====
PSET	PROC	NEAR
	MOV	AX, 0A000H
	MOV	ES, AX

	MOV	AX, Y
	MOV	BX, 320
	MUL	BX

	ADD	AX, X
	MOV	BX, AX
	MOV	AL, CO		; 색상

	MOV	ES:[BX], AL

	RET
PSET	ENDP

;===== 난수 생성 =======
RANDOM	PROC	NEAR
	PUSH	CX
	ADD	RND, 9248h
	MOV	CL, 7
	ROR	RND, CL
	MOV	AX, RND
	AND	AX, 00FFh

	MOV	DX, COL
	DIV	DL
	MOV	AL, AH
	XOR	AH, AH
	MOV	RND_COL, AX
	POP	CX
	RET
RANDOM	ENDP

;===== UFO 랜덤 생성 =====
RANDOM_UFO	PROC	NEAR
	ADD	RND, 9248h
	MOV	CL, 7
	ROR	RND, CL
	MOV	AX, RND
	AND	AX, 00FFh

	MOV	DL, 100
	DIV	DL
	CMP	AH, 10		; 1/10 퍼센트
	JA	END_RANDOM_UFO
	AND	AL, 1
	INC	AL
	MOV	UFO_FLAG, AL
	MOV	uY, 20
	CMP	AL, 1
	JE	SET_uX
	MOV	uX, 300
	JMP	END_RANDOM_UFO
SET_uX:
	MOV	uX, 0
END_RANDOM_UFO:
	RET
RANDOM_UFO	ENDP
CODE	ENDS

;=============================================================================
;				데이터 세그먼트
;=============================================================================
DATA	SEGMENT

LEFT		EQU	0
RIGHT		EQU	1
LEFT_BOUND	EQU	0		; 왼쪽 한계
RIGHT_BOUND	EQU	300		; 오른쪽 한계
UP_BOUND	EQU	20		; 위쪽 한계
DOWN_BOUND	EQU	140		; 아래쪽 한계
DOWN_COUNT	EQU	4		; 적이 한번에 아래로 내려오는 횟수
S_F_ALIEN	EQU	5000
S_F_RECHARGE	EQU	65000
F_MISSILE	EQU	150
F_UFO		EQU	50000
F_UFO_MOVE	EQU	600
F_E_MISSILE	EQU	400
MAX_STAGE	EQU	9		; 스테이지 수
START_LIFE	EQU	3		; 최초 시작시 아군 목숨

Msg_GameName	DB	"S P A C E    I N V A D E R$"
Msg_Stage	DB	"S T A G E  $"
Msg_AnyKey	DB	"Press Any Key$"
Msg_GameOver	DB	"G A M E    O V E R$"
Msg_LastScore	DB	"Your Score =$"
Msg_NewRecord	DB	"NEW RECORD !$"
Msg_Restart	DB	"Restart = Space bar,  Exit = q$"
Msg_Ending	DB	"ALL CLEAR !$"
Msg_End		DB	"Thank you !$"
Str_Score	DB	"SCORE       TOP-SCORE$"
Int_Score	DB	6 DUP(" "), "$"
Int_TopScore	DB	5 DUP(" "), "0$"

RND	DW	?	; 난수
RND_COL	DW	?	;
MODE	DB	?	; mode 선택
X	DW	?	; 픽셀 x좌표
Y	DW	?	; 픽셀 y좌표
XH	DW	?
YH	DW	?
sX	DW	?	; 아군 x좌표
sY	DW	?	; 아군 y좌표
mX	DW	?	; 미사일 x좌표
mY	DW	?	; 미사일 y좌표
eX	DW	?	; 적 미사일 x좌표
eY	DW	?	; 적 미사일 y좌표
uX	DW	?	; UFO x좌표
uY	DW	?	; UFO y좌표


ROW	DW	?	; 적군 행 수
COL	DW	?	; 적군 열 수
XOFFSET	DW	?
YOFFSET	DW	?
M_OFF	DW	?
COUNT	DW	?	; 적군의 수
CO	DB	?	; 색상값
Line_Color	DB	?	; 색상값
FLAG	DB	?	; 모션 움직임
D_FLAG	DB	?
CL_FLAG	DB	?
UFO_FLAG	DB	?
DOWN	DB	?	;
Flag_Shot	DB	?
Time_Missile	DW	?
Time_Alien	DW	?
Time_Recharge	DW	?
Time_E_Missile	DW	?
Time_UFO	DW	?
Time_UFO_Move	DW	?
F_ALIEN		DW	?
F_RECHARGE	DW	?
DIRE	DB	?	; 적군 진행 방향(좌, 우)
LIFE	DB	2	; 아군 생명
STAGE	DB	?	;
SCORE	DW	0	; 점수
T_SCORE	DW	0	; 최고 점수
BONUS	DB	?	; 보너스 점수
Temp	DW	?

; ==== 적군 상태
ENEMY	DB	100 DUP(?)
R_COLOR	DB	5 DUP(?)
R_KIND	DB	5 DUP(?)
E_ROWS	DB	3, 3, 3, 4, 4, 4, 5, 5, 5
E_COLS	DB	5, 6, 7, 7, 8, 9, 9, 10, 11
E_INFO	DB	0, 1, 7,	0, 1, 5,	1, 1, 6,	0, 0, 0,	0, 0, 0
	DB	0, 1, 7,	1, 1, 5, 	1, 1, 6, 	0, 0, 0, 	0, 0, 0
	DB	0, 2, 7,	1, 1, 5,	2, 1, 6,	0, 0, 0,	0, 0, 0
	DB	0, 1, 7,	0, 1, 5,	1, 1, 6,	2, 1, 1,	0, 0, 0
	DB	0, 2, 7,	1, 1, 5,	1, 1, 6,	2, 1, 1,	0, 0, 0
	DB	1, 2, 7,	1, 2, 5,	2, 1, 6,	3, 1, 1,	0, 0, 0
	DB	1, 2, 7,	1, 2, 5,	2, 1, 6,	2, 1, 1,	3, 1, 4
	DB	2, 2, 7,	2, 2, 5,	3, 2, 6,	3, 2, 1,	4, 1, 4
	DB	2, 2, 7,	3, 2, 5,	3, 2, 6,	4, 2, 1,	4, 2, 4
emF	DB	10 DUP(?)	; 적군 미사일 발사 여부
emX	DW	10 DUP(?)	; 적군 미사일 X좌표
emY	DW	10 DUP(?)	; 적군 미사일 Y좌표

; ==== 아군 디자인(17*13)
UNIT	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0
	DB	0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0
	DB	0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0
	DB	0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0
	DB	0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0
	DB	0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0
	DB	0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0
	DB	0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

; ==== 적군 디자인(13*13)
ALIEN1	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
	DB	0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0
	DB	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0
	DB	0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0
	DB	0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0

	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
	DB	0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0
	DB	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
	DB	0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0
	DB	0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0
	DB	0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0

ALIEN2	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0
	DB	0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0
	DB	0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0
	DB	0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0
	DB	0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0
	DB	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
	DB	0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0
	DB	0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0
	DB	0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0

	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0
	DB	0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
	DB	0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0
	DB	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
	DB	0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0
	DB	0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0
	DB	0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0

ALIEN3	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
	DB	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
	DB	0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0
	DB	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
	DB	0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0
	DB	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
	DB	0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0
	DB	0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0
	DB	0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0

	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
	DB	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
	DB	0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0
	DB	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
	DB	0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0
	DB	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
	DB	0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0
	DB	0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0

ALIEN4	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
	DB	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
	DB	0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0
	DB	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
	DB	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
	DB	0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0
	DB	0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0
	DB	0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0
	DB	0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0
	DB	0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0

	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
	DB	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
	DB	0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0
	DB	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
	DB	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
	DB	0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0
	DB	0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0
	DB	0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0
	DB	0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0
	DB	0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0

ALIEN5	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0
	DB	0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0
	DB	0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0
	DB	0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0
	DB	0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0
	DB	0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0
	DB	0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0
	DB	0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0
	DB	0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0

	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0
	DB	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0
	DB	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
	DB	0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0
	DB	0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0
	DB	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
	DB	0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0
	DB	0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

; ==== 적 파괴시 폭발 모양(13*13)
BANG	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0
	DB	0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0
	DB	0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0
	DB	0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0
	DB	0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0
	DB	0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0
	DB	0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0
	DB	0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0
	DB	0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

; ==== 아군 파괴시 폭발 모양(17*13)
EXPLOSION	DB	0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0
	DB	0, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0
	DB	4, 0, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 4, 0, 0, 0, 0
	DB	0, 4, 0, 0, 0, 0, 4, 4, 0, 0, 0, 4, 6, 0, 0, 0, 0
	DB	0, 4, 6, 0, 0, 6, 4, 4, 4, 0, 4, 6, 0, 0, 0, 4, 0
	DB	0, 0, 4, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 0, 6, 0, 0
	DB	0, 0, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 0, 0
	DB	0, 0, 0, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 6, 0, 0, 0
	DB	4, 4, 0, 0, 6, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 0, 0
	DB	0, 4, 4, 4, 4, 6, 4, 4, 4, 6, 4, 4, 4, 6, 6, 4, 0
	DB	0, 0, 0, 6, 6, 4, 6, 4, 6, 4, 4, 4, 6, 4, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0
	DB	0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 4, 0, 0, 0, 0
	DB	0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0
	DB	0, 0, 4, 6, 0, 0, 0, 6, 0, 0, 4, 4, 0, 0, 6, 4, 0
	DB	0, 0, 0, 4, 4, 0, 0, 4, 0, 6, 4, 0, 0, 4, 0, 0, 0
	DB	6, 0, 0, 0, 4, 4, 0, 4, 4, 4, 4, 4, 4, 6, 0, 0, 6
	DB	4, 0, 0, 0, 4, 6, 4, 4, 4, 4, 4, 4, 4, 0, 0, 4, 6
	DB	0, 6, 0, 4, 4, 4, 6, 4, 4, 4, 4, 6, 0, 0, 6, 4, 0
	DB	0, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4, 4, 6, 4, 4, 0
	DB	0, 0, 4, 6, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 0, 0
	DB	0, 0, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 0, 0
	DB	0, 0, 0, 4, 4, 4, 6, 4, 6, 6, 4, 4, 4, 6, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

; ==== UFO 디자인
UFO	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 4, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 0, 0, 4, 0, 0, 0, 0
	DB	0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0, 4, 0, 0, 0, 0
	DB	0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0
	DB	0, 4, 4, 4, 0, 0, 4, 4, 4, 4, 4, 0, 0, 4, 4, 4, 0
	DB	0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0
	DB	0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 4, 0, 0, 4, 0, 0, 4, 0, 0, 0, 0, 0
	DB	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
DATA	ENDS
	END	MAIN