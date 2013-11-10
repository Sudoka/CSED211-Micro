 AREA |C$$code|, CODE, READONLY


  MACRO
       MOV_PC_LR
       [ THUMBCODE
           bx lr
       |
           mov pc,lr
       ]
   MEND

INDEX         EQU 0x31000000
SEGMENT     EQU 0x31000004
WORD         EQU 0x31000008

; MMU Parameter set
LOCK_BASE_LSB	EQU	0x1A
LOCK_VICT_LSB	EQU	0x14
P_STATE_LSB	       EQU	0x0
P_ENTRY_LSB	       EQU	0x4
VATAG_LSB	       EQU	0xA
VASIZE_LSB	       EQU	0x6
VALID_LSB	       EQU	0x5
DOMAIN8_LSB	EQU	0xE
DOMAIN_LSB	       EQU	0x6
NCACHE_LSB	       EQU	0x5
NBUFF_LSB	       EQU	0x4
ACCESS_LSB	       EQU	0x0
PATAG_LSB	       EQU	0xA
PASIZE_LSB	       EQU	0x6

; Cache Parameter set
TAG_LSB		EQU	0x8
SEG_LSB		EQU	0x5
VLD_LSB		EQU	0x4 ; Valid bit
DE_LSB		EQU	0x3 ; Dirty Even bit
DO_LSB		EQU	0x2 ; Dirty Odd bit
WB_LSB		EQU	0x1 ; Write Back bit
WORD_LSB	EQU	0x2  
LOCK_LSB	EQU	0x1A
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Dcache Test ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  EXPORT DCache_Test
DCache_Test	   
              stmfd sp!,{r1-r12}
				; MAIN PROGRAM HERE
		;///////////////////////////////////////////////////
		; Variable initialize
		; R4 : Next Pattern	[~PAT or PAT]
		; R5 : Current Pattern	[PAT or ~PAT]
		; R6 : INC/DEC Value
		; R7 : Index		; 0x0 <-> 0x63
		; R8 : Seg 		; 0x0 <-> 0x7
		; R9 : Word 		; 0x0 <-> 0x7
		; R10 : PAT
		; R11 : ~PAT
		; R12 : LOOP4		[ INC(PAT)->INC(~PAT)->DEC(PAT)->DEC(~PAT) ]
		; R3 : Pattern Type Select [ 0 - 5 ]

		;/////////////////////////
		; Program Variables
		;/////////////////////////
		MOV	R3, #0		; Pattern Select
		;/////////////////////////
StartComp

StartPat
		MOV	R8, #0 
		CMP	R3, #0
		LDREQ	R10, =0x00000000
		LDREQ	R11, =0xFFFFFFFF
		CMP	R3, #1
		LDREQ	R10, =0x0000FFFF
		LDREQ	R11, =0xFFFF0000
		CMP	R3, #2
		LDREQ	R10, =0x00FF00FF
		LDREQ	R11, =0xFF00FF00
		CMP	R3, #3
		LDREQ	R10, =0x0F0F0F0F
		LDREQ	R11, =0xF0F0F0F0
		CMP	R3, #4
		LDREQ	R10, =0x33333333
		LDREQ	R11, =0xCCCCCCCC
		CMP	R3, #5
		LDREQ	R10, =0xAAAAAAAA
		LDREQ	R11, =0x55555555

		;//////////////////////////
		; PRE WRITE RAM DATA
		;//////////////////////////
StartSeg
		MOV	R12, #0 
		MOV	R7, #0 
PStartIndex
		MOV	R9, #0 
		MOV	R5, R10 ; Pattern Set
PStartWord
		;///////////////////////////////////////////////////
		; Load DCache victim and lockdown base 
		MOV	R0, R7, LSL #LOCK_LSB
		MCR	p15,0,r0,c9,c0,0 ; D
		;MCR	p15,0,r0,c9,c0,1 ; I

		; Do DCache CAM write to 
		MOV	R1, R5, LSL #8		; CAM Tag
		ORR	R1, R1, R7		; Index ORed
		MOV	R0, R1, LSL #TAG_LSB
		ORR	R0, R0, R8,LSL #SEG_LSB	; Segment
		ORR	R0, R0, #0x1E
		MCR	p15,2,R0,c15,c6,6 	; D CAM write
		;MCR	p15,2,R0,c15,c5,6 	; I CAM write

		; Reload DCache lock-down pointer because it will have incremented
		MOV	R0, R7, LSL #LOCK_LSB	; Index
		MCR	p15,0,R0,c9,c0,0	; D Write victim & lockdown
		;MCR	p15,0,R0,c9,c0,1	; I Write victim & lockdown

		; Do DCache RAM write 
		MOV	R0, R5			; RAM data
		MCR	p15,3,R0,c15,c2,0	; Write RAM data to c15.C.D
		;MCR	p15,3,R0,c15,c1,0	; Write RAM data to c15.C.I

		MOV	R0, R8, LSL #SEG_LSB	;Segment
		ORR	R0, R0, R9,LSL #WORD_LSB ; Word
		MCR	p15,2,R0,c15,c10,6	; RAM write from c15.C.D
		;MCR	p15,2,R0,c15,c9,6	; RAM write from c15.C.D

		; Variable Update	
		CMP	R9, #7
		BEQ	PEndWord	
		ADD	R9, R9, #1 		; Word ++
		B	PStartWord
PEndWord
		CMP	R7, #63
		BEQ	PEndIndex	
		ADD	R7, R7, #1 		; Index ++
		B	PStartIndex
PEndIndex

		;////////////////////////////////
		; MAIN READ-COMPARE-WRITE RAM DATA
		;////////////////////////////////
              
StartLoop4
		
               
		CMP	R12, #0
		LDREQ	R6, =0x00000001
		MOVEQ	R7, #0
		CMP	R12, #1
		LDREQ	R6, =0x00000001
		MOVEQ	R7, #0
		CMP	R12, #2
		LDREQ	R6, =0xFFFFFFFF
		MOVEQ	R7, #63
		CMP	R12, #3
		LDREQ	R6, =0xFFFFFFFF
		MOVEQ	R7, #63
StartIndex
		
		CMP	R12, #0
		MOVEQ	R5, R10
		MOVEQ	R4, R11
		MOVEQ	R9, #0
		CMP	R12, #1
		MOVEQ	R5, R11
		MOVEQ	R4, R10
		MOVEQ	R9, #0
		CMP	R12, #2
		MOVEQ	R5, R10
		MOVEQ	R4, R11
		MOVEQ	R9, #7
		CMP	R12, #3
		MOVEQ	R5, R11
		MOVEQ	R4, R10
		MOVEQ	R9, #7
StartWord
		;///////////////////////////////////////////////////
		; Load DCache victim and lockdown base 
		MOV	R0, R7, LSL #LOCK_LSB	; Index
		MCR	p15,0,r0,c9,c0,0 	; D Write victim & lockdown
		;MCR	p15,0,r0,c9,c0,1	; I Write victim & lockdown

		; Do DCache CAM write to 
		MOV	R1, R5, LSL #8		; CAM Tag
		ORR	R1, R1, R7		; Index ORed
		MOV	R0, R1, LSL #TAG_LSB
		ORR	R0, R0, R8,LSL #SEG_LSB	; Segment
		ORR	R0, R0, #0x1E
		MCR	p15,2,R0,c15,c6,6 	; D CAM write
		;MCR	p15,2,R0,c15,c5,6 	; I CAM write

		; Reload DCache lock-down pointer because it will have incremented
		MOV	R0, R7, LSL #LOCK_LSB	; Index
		MCR	p15,0,R0,c9,c0,0	; D Write victim & lockdown
		;MCR	p15,0,R0,c9,c0,1	; I Write victim & lockdown

		; Clear c15.C.D to prove that data comes back from DCache
		MOV	R0, #0
		MCR	p15,3,R0,c15,c2,0	; Write c15.C.D
		;MCR	p15,3,R0,c15,c1,0	; Write c15.C.I

		; Do a CAM match, RAM read to c15.C.[D/I]
		MOV	R1, R5, LSL #8		; CAM Tag
		ORR	R1, R1, R7		; Index ORed
		MOV	R0, R1, LSL #TAG_LSB	; TAG
		ORR	R0,R0,R8,LSL #SEG_LSB	; Segment
		ORR	R0,R0,R9, LSL #WORD_LSB	; Word
		MCR	p15,2,R0,c15,c6,5	; CAM match, D. RAM read
		;MCR	p15,2,R0,c15,c5,5	; CAM match, I. RAM read

		; Read c15.C.D and compare with expected data.
		; Note that the top 2 bits of the RAM Data returned from the CAM match
		; give the Hot and Miss information [31:30] = [Miss,Hit]
		MRC	p15,3,R0,c15,c2,0	; Read c15.C.D
		;MRC	p15,3,R0,c15,c1,0	; Read c15.C.I

		;MOV	R2, #0			; Var. Init.
		;LDR	R3, =0x08000FF0		; BANK2 Addr.
		;ORR	R2,R2,R7, LSL #16	; Index	
		;ORR	R2,R2,R8, LSL #8	; Seg	
		;ORR	R2,R2,R9, LSL #0	; Word
		;STR	R2, [R3]		; Monitor Out

		; Check the RAM data	--------------
		MOV	R0, R0, LSL #2		; Remove bits [31:30]
		MOV	R1, R5			; Expected data
		MOV	R1, R1, LSL #2		; Remove bits [31:30]
		CMP	R0, R1
		BNE	ERROR

		; Reload DCache lock-down pointer because it will have incremented
		MOV	R0, R7, LSL #LOCK_LSB	; Index
		MCR	p15,0,R0,c9,c0,0	; D Write victim & lockdown
		;MCR	p15,0,R0,c9,c0,1	; I Write victim & lockdown

		; Do DCache RAM write	--------------
		MOV	R0, R4			; RAM data (R4: Next PAT)
		MCR	p15,3,R0,c15,c2,0	; Write RAM data to c15.C.D
		;MCR	p15,3,R0,c15,c1,0	; Write RAM data to c15.C.I

		MOV	R0, R8, LSL #SEG_LSB	;Segment
		ORR	R0, R0, R9,LSL #WORD_LSB ; Word
		MCR	p15,2,R0,c15,c10,6	; RAM write from c15.C.D
		;MCR	p15,2,R0,c15,c9,6	; RAM write from c15.C.D

		;--------------------
		; Variables Update	
		;--------------------
		
		CMP	R12, #0
		CMPEQ	R9, #7
		BEQ	EndWord
		CMP	R12, #1
		CMPEQ	R9, #7
		BEQ	EndWord
		CMP	R12, #2
		CMPEQ	R9, #0
		BEQ	EndWord
		CMP	R12, #3
		CMPEQ	R9, #0
		BEQ	EndWord
		ADD	R9, R9, R6 ; Word ++/-- 	[0<->7]
		B	StartWord
EndWord
		CMP	R12, #0
		CMPEQ	R7, #63
		BEQ	EndIndex
		CMP	R12, #1
		CMPEQ	R7, #63
		BEQ	EndIndex
		CMP	R12, #2
		CMPEQ	R7, #0
		BEQ	EndIndex
		CMP	R12, #3
		CMPEQ	R7, #0
		BEQ	EndIndex
		ADD	R7, R7, R6 ; Index ++/--	[0<->63]
		B	StartIndex
EndIndex
		CMP	R12, #3
		BEQ	EndLoop4
		ADD	R12, R12, #1 ; LOOP4 ++		[0->3]
		B	StartLoop4
EndLoop4
		CMP	R8, #7
		BEQ	EndSeg
		ADD	R8, R8, #1 ; Seg ++		[0->7]
		B	StartSeg
EndSeg
		CMP	R3, #5
		BEQ	EndPat
		ADD	R3, R3, #1 ; Pattern ++	[0->5]
		B	StartPat
EndPat		

EndComp
;Caching_Test_End	

    MOV R0, #1
            B  EndFunc
            
ERROR  MOV R0, #0
             ldr	r1,=INDEX       
	       mov r2,r7       
	       str	r2,[r1]

	       ldr	r1,=SEGMENT       
	      mov r2,r8       
	       str	r2,[r1]

	       ldr	r1,=WORD       
	       mov r2,r9         
	       str	r2,[r1]
EndFunc
              ldmfd sp!,{r1-r12}
              mov	pc,lr


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Icache Test ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  EXPORT ICache_Test
ICache_Test	   
              stmfd sp!,{r1-r12}
				; MAIN PROGRAM HERE
		;///////////////////////////////////////////////////
		; Variable initialize
		; R4 : Next Pattern	[~PAT or PAT]
		; R5 : Current Pattern	[PAT or ~PAT]
		; R6 : INC/DEC Value
		; R7 : Index		; 0x0 <-> 0x63
		; R8 : Seg 		; 0x0 <-> 0x7
		; R9 : Word 		; 0x0 <-> 0x7
		; R10 : PAT
		; R11 : ~PAT
		; R12 : LOOP4		[ INC(PAT)->INC(~PAT)->DEC(PAT)->DEC(~PAT) ]
		; R3 : Pattern Type Select [ 0 - 5 ]

		;/////////////////////////
		; Program Variables
		;/////////////////////////
		MOV	R3, #0		; Pattern Select
		;/////////////////////////
StartComp1

StartPat1
		MOV	R8, #0 
		CMP	R3, #0
		LDREQ	R10, =0x00000000
		LDREQ	R11, =0xFFFFFFFF
		CMP	R3, #1
		LDREQ	R10, =0x0000FFFF
		LDREQ	R11, =0xFFFF0000
		CMP	R3, #2
		LDREQ	R10, =0x00FF00FF
		LDREQ	R11, =0xFF00FF00
		CMP	R3, #3
		LDREQ	R10, =0x0F0F0F0F
		LDREQ	R11, =0xF0F0F0F0
		CMP	R3, #4
		LDREQ	R10, =0x33333333
		LDREQ	R11, =0xCCCCCCCC
		CMP	R3, #5
		LDREQ	R10, =0xAAAAAAAA
		LDREQ	R11, =0x55555555

		;//////////////////////////
		; PRE WRITE RAM DATA
		;//////////////////////////
StartSeg1
		MOV	R12, #0 
		MOV	R7, #0 
PStartIndex1
		MOV	R9, #0 
		MOV	R5, R10 ; Pattern Set
PStartWord1
		;///////////////////////////////////////////////////
		; Load DCache victim and lockdown base 
		MOV	R0, R7, LSL #LOCK_LSB
		;MCR	p15,0,r0,c9,c0,0 ; D
		MCR	p15,0,r0,c9,c0,1 ; I

		; Do DCache CAM write to 
		MOV	R1, R5, LSL #8		; CAM Tag
		ORR	R1, R1, R7		; Index ORed
		MOV	R0, R1, LSL #TAG_LSB
		ORR	R0, R0, R8,LSL #SEG_LSB	; Segment
		ORR	R0, R0, #0x1E
		;MCR	p15,2,R0,c15,c6,6 	; D CAM write
		MCR	p15,2,R0,c15,c5,6 	; I CAM write

		; Reload DCache lock-down pointer because it will have incremented
		MOV	R0, R7, LSL #LOCK_LSB	; Index
		;MCR	p15,0,R0,c9,c0,0	; D Write victim & lockdown
		MCR	p15,0,R0,c9,c0,1	; I Write victim & lockdown

		; Do DCache RAM write 
		MOV	R0, R5			; RAM data
		;MCR	p15,3,R0,c15,c2,0	; Write RAM data to c15.C.D
		MCR	p15,3,R0,c15,c1,0	; Write RAM data to c15.C.I

		MOV	R0, R8, LSL #SEG_LSB	;Segment
		ORR	R0, R0, R9,LSL #WORD_LSB ; Word
		;MCR	p15,2,R0,c15,c10,6	; RAM write from c15.C.D
		MCR	p15,2,R0,c15,c9,6	; RAM write from c15.C.D

		; Variable Update	
		CMP	R9, #7
		BEQ	PEndWord1	
		ADD	R9, R9, #1 		; Word ++
		B	PStartWord1
PEndWord1
		CMP	R7, #63
		BEQ	PEndIndex1	
		ADD	R7, R7, #1 		; Index ++
		B	PStartIndex1
PEndIndex1

		;////////////////////////////////
		; MAIN READ-COMPARE-WRITE RAM DATA
		;////////////////////////////////
StartLoop41
		CMP	R12, #0
		LDREQ	R6, =0x00000001
		MOVEQ	R7, #0
		CMP	R12, #1
		LDREQ	R6, =0x00000001
		MOVEQ	R7, #0
		CMP	R12, #2
		LDREQ	R6, =0xFFFFFFFF
		MOVEQ	R7, #63
		CMP	R12, #3
		LDREQ	R6, =0xFFFFFFFF
		MOVEQ	R7, #63
StartIndex1
		
		CMP	R12, #0
		MOVEQ	R5, R10
		MOVEQ	R4, R11
		MOVEQ	R9, #0
		CMP	R12, #1
		MOVEQ	R5, R11
		MOVEQ	R4, R10
		MOVEQ	R9, #0
		CMP	R12, #2
		MOVEQ	R5, R10
		MOVEQ	R4, R11
		MOVEQ	R9, #7
		CMP	R12, #3
		MOVEQ	R5, R11
		MOVEQ	R4, R10
		MOVEQ	R9, #7
StartWord1
		;///////////////////////////////////////////////////
		; Load DCache victim and lockdown base 
		MOV	R0, R7, LSL #LOCK_LSB	; Index
		;MCR	p15,0,r0,c9,c0,0 	; D Write victim & lockdown
		MCR	p15,0,r0,c9,c0,1	; I Write victim & lockdown

		; Do DCache CAM write to 
		MOV	R1, R5, LSL #8		; CAM Tag
		ORR	R1, R1, R7		; Index ORed
		MOV	R0, R1, LSL #TAG_LSB
		ORR	R0, R0, R8,LSL #SEG_LSB	; Segment
		ORR	R0, R0, #0x1E
		;MCR	p15,2,R0,c15,c6,6 	; D CAM write
		MCR	p15,2,R0,c15,c5,6 	; I CAM write

		; Reload DCache lock-down pointer because it will have incremented
		MOV	R0, R7, LSL #LOCK_LSB	; Index
		;MCR	p15,0,R0,c9,c0,0	; D Write victim & lockdown
		MCR	p15,0,R0,c9,c0,1	; I Write victim & lockdown

		; Clear c15.C.D to prove that data comes back from DCache
		MOV	R0, #0
		;MCR	p15,3,R0,c15,c2,0	; Write c15.C.D
		MCR	p15,3,R0,c15,c1,0	; Write c15.C.I

		; Do a CAM match, RAM read to c15.C.[D/I]
		MOV	R1, R5, LSL #8		; CAM Tag
		ORR	R1, R1, R7		; Index ORed
		MOV	R0, R1, LSL #TAG_LSB	; TAG
		ORR	R0,R0,R8,LSL #SEG_LSB	; Segment
		ORR	R0,R0,R9, LSL #WORD_LSB	; Word
		;MCR	p15,2,R0,c15,c6,5	; CAM match, D. RAM read
		MCR	p15,2,R0,c15,c5,5	; CAM match, I. RAM read

		; Read c15.C.D and compare with expected data.
		; Note that the top 2 bits of the RAM Data returned from the CAM match
		; give the Hot and Miss information [31:30] = [Miss,Hit]
		;MRC	p15,3,R0,c15,c2,0	; Read c15.C.D
		MRC	p15,3,R0,c15,c1,0	; Read c15.C.I

		;MOV	R2, #0			; Var. Init.
		;LDR	R3, =0x08000FF0		; BANK2 Addr.
		;ORR	R2,R2,R7, LSL #16	; Index	
		;ORR	R2,R2,R8, LSL #8	; Seg	
		;ORR	R2,R2,R9, LSL #0	; Word
		;STR	R2, [R3]		; Monitor Out

		; Check the RAM data	--------------
		MOV	R0, R0, LSL #2		; Remove bits [31:30]
		MOV	R1, R5			; Expected data
		MOV	R1, R1, LSL #2		; Remove bits [31:30]
		CMP	R0, R1
		BNE	ERROR

		; Reload DCache lock-down pointer because it will have incremented
		MOV	R0, R7, LSL #LOCK_LSB	; Index
		;MCR	p15,0,R0,c9,c0,0	; D Write victim & lockdown
		MCR	p15,0,R0,c9,c0,1	; I Write victim & lockdown

		; Do DCache RAM write	--------------
		MOV	R0, R4			; RAM data (R4: Next PAT)
		;MCR	p15,3,R0,c15,c2,0	; Write RAM data to c15.C.D
		MCR	p15,3,R0,c15,c1,0	; Write RAM data to c15.C.I

		MOV	R0, R8, LSL #SEG_LSB	;Segment
		ORR	R0, R0, R9,LSL #WORD_LSB ; Word
		;MCR	p15,2,R0,c15,c10,6	; RAM write from c15.C.D
		MCR	p15,2,R0,c15,c9,6	; RAM write from c15.C.D

		;--------------------
		; Variables Update	
		;--------------------
		
		CMP	R12, #0
		CMPEQ	R9, #7
		BEQ	EndWord1
		CMP	R12, #1
		CMPEQ	R9, #7
		BEQ	EndWord1
		CMP	R12, #2
		CMPEQ	R9, #0
		BEQ	EndWord1
		CMP	R12, #3
		CMPEQ	R9, #0
		BEQ	EndWord1
		ADD	R9, R9, R6 ; Word ++/-- 	[0<->7]
		B	StartWord1
EndWord1
		CMP	R12, #0
		CMPEQ	R7, #63
		BEQ	EndIndex1
		CMP	R12, #1
		CMPEQ	R7, #63
		BEQ	EndIndex1
		CMP	R12, #2
		CMPEQ	R7, #0
		BEQ	EndIndex1
		CMP	R12, #3
		CMPEQ	R7, #0
		BEQ	EndIndex1
		ADD	R7, R7, R6 ; Index ++/--	[0<->63]
		B	StartIndex1
EndIndex1
		CMP	R12, #3
		BEQ	EndLoop41
		ADD	R12, R12, #1 ; LOOP4 ++		[0->3]
		B	StartLoop41
EndLoop41
		CMP	R8, #7
		BEQ	EndSeg1
		ADD	R8, R8, #1 ; Seg ++		[0->7]
		B	StartSeg1
EndSeg1
		CMP	R3, #5
		BEQ	EndPat1
		ADD	R3, R3, #1 ; Pattern ++	[0->5]
		B	StartPat1
EndPat1		

EndComp1
;Caching_Test_End	

    MOV R0, #1
            B  EndFunc1
            
ERROR1  MOV R0, #0
              ldr	r1,=INDEX       
	       mov r2,r7       
	       str	r2,[r1]

	       ldr	r1,=SEGMENT       
	      mov r2,r8       
	       str	r2,[r1]

	       ldr	r1,=WORD       
	       mov r2,r9         
	       str	r2,[r1]
EndFunc1
              ldmfd sp!,{r1-r12}
              mov	pc,lr


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DMMU Test PROGRAM ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  EXPORT DMMU_Test
DMMU_Test	   
              stmfd sp!,{r1-r12}
		

		; ---------------------------------------------------------------------
		; MAIN PROGRAM ( DMMU Test )
		; ---------------------------------------------------------------------

		; ---------------------------------------------------------------------
		; Variables initialize
		; ---------------------------------------------------------------------
		; R4  : March10 Next Pattern Data
		; R5  : March10 Current Pattern Data
		; R6  : Inc or Dec Value( 1: INC, -1: DEC)
		; R7  : I or D MMU Test ( 1: IMMU, 0: DMMU)
		; R8  : Index [ 0 - 63 ] 
		; R10 : March10 Normal Pattern
		; R11 : March10 Inverted Pattern
		; R12 : LOOP4 Counter [0-3] [ INC(PAT)->INC(~PAT)->DEC(PAT)->DEC(PAT) ]
		; RR9 : March10 Pattern Select Count [0-5]
		; ---------------------------------------------------------------------
StartComp_M

InitRount_M
		MOV	 R9, #0
StartPat_M
		MOV	 R12, #0

		CMP	 R9, #0
		LDREQ	R10, =0x00000000
		LDREQ	R11, =0xFFFFFFFF
		CMP	 R9, #1
		LDREQ	R10, =0x0000FFFF
		LDREQ	R11, =0xFFFF0000
		CMP	 R9, #2
		LDREQ	R10, =0x00FF00FF
		LDREQ	R11, =0xFF00FF00
		CMP	 R9, #3
		LDREQ	R10, =0x0F0F0F0F
		LDREQ	R11, =0xF0F0F0F0
		CMP	 R9, #4
		LDREQ	R10, =0x33333333
		LDREQ	R11, =0xCCCCCCCC
		CMP	 R9, #5
		LDREQ	R10, =0xAAAAAAAA
		LDREQ	R11, =0x55555555

		MOV	R8, #0 			; for Pre-Write RAM Data

PStartIndex_M
		; -----------------------------
		; ---- PRE-RAM DATA WRITE -----
		; -----------------------------
		MOV	R5, R10

		; Load the DMMU lock-down pointer 
		MOV	R0, R8, LSL #LOCK_BASE_LSB	; Base
		ORR	R0, R0, R8, LSL #LOCK_VICT_LSB	; Victim
		ORR	R0, R0, #0 :SHL: P_STATE_LSB	; Preserve
		MCR	p15,0,r0,c10,c0,0 		

		; CAM write to index (R5 <<8 | R8) 
                MOV     R0, R5, LSL #8			; CAM Tag
                ORR     R0, R0, R8			; Index ORed
		MOV	R0, R0, LSL #VATAG_LSB		; MVA Tag
		ORR	R0, R0, #7 :SHL: 4	
		MCR	p15,4,r0,c15,c6,0

		CMP	R8,  #63
		BEQ	PEndIndex_M	
		ADD	R8, R8, #1	; Index ++

		B	PStartIndex_M
PEndIndex_M


StartLoop4_M
		CMP	R12, #0
		MOVEQ	R8,  #0
		LDREQ	R6,  =0x00000001
		MOVEQ	R5, R10
		MOVEQ	R4, R11
		CMP	R12, #1
		MOVEQ	R8,  #0
		LDREQ	R6,  =0x00000001
		MOVEQ	R5, R11
		MOVEQ	R4, R10
		CMP	R12, #2
		MOVEQ	R8, #63
		;MOVEQ	R8, #3
		LDREQ	R6,  =0xFFFFFFFF
		MOVEQ	R5, R10
		MOVEQ	R4, R11
		CMP	R12, #3
		MOVEQ	R8, #63
		;MOVEQ	R8, #3
		LDREQ	R6,  =0xFFFFFFFF
		MOVEQ	R5, R11
		MOVEQ	R4, R10
StartIndex_M
		;///////////////////////////////////////////////////
		
		; Load the DMMU lock-down pointer 
		MOV	R0, R8, LSL #LOCK_BASE_LSB	; Base
		ORR	R0, R0, R8, LSL #LOCK_VICT_LSB	; Victim
		ORR	R0, R0, #0 :SHL: P_STATE_LSB	; Preserve
		MCR	p15,0,r0,c10,c0,0		

		; D CAM read to C15.M.D
		MCR	p15,4,r0,c15,c6,4	
		; Read C15.M.D to r1 
		MRC	p15,4,r1,c15,c2,6	; R1 <- DCAM	

                MOV     R0, R5, LSL #8			; CAM Tag
                ORR     R0, R0, R8			; Index ORed
		MOV	R0, R0, LSL #VATAG_LSB		; MVA Tag
		ORR	R0, R0, #7 :SHL: P_ENTRY_LSB	

		CMP	R1, R0
		;BEQ	ERROR_M
		BNE	ERROR_M

		; ------------------------------------------------
		; Write Next RAM Data
		; ------------------------------------------------

		; Load the DMMU lock-down pointer 
		MOV	R0, R8, LSL #LOCK_BASE_LSB	; Base
		ORR	R0, R0, R8, LSL #LOCK_VICT_LSB	; Victim
		ORR	R0, R0, #0 :SHL: P_STATE_LSB	; Preserve
		MCR	p15,0,r0,c10,c0,0 		

		; CAM write to 
                MOV     R0, R4, LSL #8			; CAM Tag
                ORR     R0, R0, R8			; Index ORed
		MOV	R0, R0, LSL #VATAG_LSB		; MVA Tag
		ORR	R0, R0, #7 :SHL: P_ENTRY_LSB	
		MCR	p15,4,r0,c15,c6,0

		
		; ----------------
		; Variables Update	
		; ----------------
		CMP	R12, #0
		CMPEQ	R8, #63
		BEQ	EndIndex_M	
		CMP	R12, #1
		CMPEQ	R8, #63
		BEQ	EndIndex_M	
		CMP	R12, #2
		CMPEQ	R8,  #0
		BEQ	EndIndex_M	
		CMP	R12, #3
		CMPEQ	R8,  #0
		BEQ	EndIndex_M	
		ADD	R8, R8, R6	; Index ++/--
		B	StartIndex_M
EndIndex_M
		CMP	R12, #3	
		BEQ	EndLoop4_M	
		ADD	R12, R12, #1	; Loop ++
		B	StartLoop4_M
EndLoop4_M
		CMP	R9, #5
		BEQ	EndPat_M	
		ADD	 R9, R9, #1	; Pattern ++
		B	StartPat_M
EndPat_M

EndComp_M
 MOV R0, #1
            B  EndFunc_M
            
ERROR_M  MOV R0, #0
              ldr	r1,=INDEX       
	       mov r2,r8       
	       str	r2,[r1]

	      
EndFunc_M
              ldmfd sp!,{r1-r12}
              mov	pc,lr
	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; IMMU Test PROGRAM ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  EXPORT IMMU_Test
IMMU_Test	   
              stmfd sp!,{r1-r12}
		

		; ---------------------------------------------------------------------
		; MAIN PROGRAM ( IMMU TEST )
		; ---------------------------------------------------------------------

		; ---------------------------------------------------------------------
		; Variables initialize
		; ---------------------------------------------------------------------
		; R4  : March10 Next Pattern Data
		; R5  : March10 Current Pattern Data
		; R6  : Inc or Dec Value( 1: INC, -1: DEC)
		; R7  : I or D MMU Test ( 1: IMMU, 0: DMMU) * Not used
		; R8  : Index [ 0 - 63 ] 
		; R10 : March10 Normal Pattern
		; R11 : March10 Inverted Pattern
		; R12 : LOOP4 Counter [0-3] [ INC(PAT)->INC(~PAT)->DEC(PAT)->DEC(PAT) ]
		; R9 : March10 Pattern Select Count [0-5]
		; ---------------------------------------------------------------------
StartComp_I

InitRount_I
		MOV	R9, #0
StartPat_I
		MOV	R12, #0

		CMP	R9, #0
		LDREQ	R10, =0x00000000
		LDREQ	R11, =0xFFFFFFFF
		CMP	R9, #1
		LDREQ	R10, =0x0000FFFF
		LDREQ	R11, =0xFFFF0000
		CMP	R9, #2
		LDREQ	R10, =0x00FF00FF
		LDREQ	R11, =0xFF00FF00
		CMP	R9, #3
		LDREQ	R10, =0x0F0F0F0F
		LDREQ	R11, =0xF0F0F0F0
		CMP	R9, #4
		LDREQ	R10, =0x33333333
		LDREQ	R11, =0xCCCCCCCC
		CMP	R9, #5
		LDREQ	R10, =0xAAAAAAAA
		LDREQ	R11, =0x55555555

		MOV	R8, #0 			; for Pre-Write RAM Data

PStartIndex_I
		; -----------------------------
		; ---- PRE-RAM DATA WRITE -----
		; -----------------------------
		MOV	R5, R10

		; Load the DMMU lock-down pointer 
		MOV	R0, R8, LSL #LOCK_BASE_LSB	; Base
		ORR	R0, R0, R8, LSL #LOCK_VICT_LSB	; Victim
		ORR	R0, R0, #0 :SHL: P_STATE_LSB	; Preserve
		MCR	p15,0,r0,c10,c0,1 		

		; CAM write to index (R5 <<8 | R8) 
                MOV     R0, R5, LSL #8			; CAM Tag
                ORR     R0, R0, R8			; Index ORed
		MOV	R0, R0, LSL #VATAG_LSB		; MVA Tag
		ORR	R0, R0, #7 :SHL: P_ENTRY_LSB	
		MCR	p15,4,r0,c15,c5,0

		CMP	R8,  #63
		BEQ	PEndIndex_I
		ADD	R8, R8, #1	; Index ++

		B	PStartIndex_I
PEndIndex_I

StartLoop4_I
		CMP	R12, #0
		MOVEQ	R8,  #0
		LDREQ	R6,  =0x00000001
		MOVEQ	R5, R10
		MOVEQ	R4, R11
		CMP	R12, #1
		MOVEQ	R8,  #0
		LDREQ	R6,  =0x00000001
		MOVEQ	R5, R11
		MOVEQ	R4, R10
		CMP	R12, #2
		MOVEQ	R8, #63
		;MOVEQ	R8, #3
		LDREQ	R6,  =0xFFFFFFFF
		MOVEQ	R5, R10
		MOVEQ	R4, R11
		CMP	R12, #3
		MOVEQ	R8, #63
		;MOVEQ	R8, #3
		LDREQ	R6,  =0xFFFFFFFF
		MOVEQ	R5, R11
		MOVEQ	R4, R10
StartIndex_I
		;///////////////////////////////////////////////////
		
		; Load the DMMU lock-down pointer 
		MOV	R0, R8, LSL #LOCK_BASE_LSB	; Base
		ORR	R0, R0, R8, LSL #LOCK_VICT_LSB	; Victim
		ORR	R0, R0, #0 :SHL: P_STATE_LSB	; Preserve
		MCR	p15,0,r0,c10,c0,1		

		; I CAM read to C15.M.I
		MCR	p15,4,r0,c15,c5,4	
		; Read C15.M.I to r1 
		MRC	p15,4,r1,c15,c1,6	; R1 <- ICAM	

                MOV     R0, R5, LSL #8			; CAM Tag
                ORR     R0, R0, R8			; Index ORed
		MOV	R0, R0, LSL #VATAG_LSB		; MVA Tag
		ORR	R0, R0, #7 :SHL: P_ENTRY_LSB	

		CMP	R1, R0
		;BEQ	ERROR_I
		BNE	ERROR_I

		; ------------------------------------------------
		; Write Next RAM Data
		; ------------------------------------------------

		; Load the DMMU lock-down pointer 
		MOV	R0, R8, LSL #LOCK_BASE_LSB	; Base
		ORR	R0, R0, R8, LSL #LOCK_VICT_LSB	; Victim
		ORR	R0, R0, #0 :SHL: P_STATE_LSB	; Preserve
		MCR	p15,0,r0,c10,c0,1 		

		; CAM write to 
                MOV     R0, R4, LSL #8			; CAM Tag
                ORR     R0, R0, R8			; Index ORed
		MOV	R0, R0, LSL #VATAG_LSB		; MVA Tag
		ORR	R0, R0, #7 :SHL: P_ENTRY_LSB	
		MCR	p15,4,r0,c15,c5,0

		
		; ----------------
		; Variables Update	
		; ----------------
		CMP	R12, #0
		CMPEQ	R8, #63
		BEQ	EndIndex_I
		CMP	R12, #1
		CMPEQ	R8, #63
		BEQ	EndIndex_I
		CMP	R12, #2
		CMPEQ	R8,  #0
		BEQ	EndIndex_I
		CMP	R12, #3
		CMPEQ	R8,  #0
		BEQ	EndIndex_I
		ADD	R8, R8, R6	; Index ++/--
		B	StartIndex_I
EndIndex_I
		CMP	R12, #3	
		BEQ	EndLoop4_I
		ADD	R12, R12, #1	; Loop ++
		B	StartLoop4_I
EndLoop4_I
		CMP	R9, #5
		BEQ	EndPat_I
		ADD	R9, R9, #1	; Pattern ++
		B	StartPat_I
EndPat_I
EndComp_I



Caching_Test_End_I	


 MOV R0, #1
            B  EndFunc_I
            
ERROR_I  MOV R0, #0
              ldr	r1,=INDEX       
	       mov r2,r8       
	       str	r2,[r1]

	      
EndFunc_I
              ldmfd sp!,{r1-r12}
              mov	pc,lr

	
 END

