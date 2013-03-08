	CLS
	CALL lab2
	SCD #4
	LD v8, #0xFFF
loop5:
	SCR
	SCL
	SE Va,v6
	JMP loop1
	ADD VF,Vf
	ADD V4,#0b100
	LD V4,v5

loop2:	ADD v3,v2
	SNE v3,#0
	OR Vf,Ve
	AND VE,VE
	XOR VE,VF
	JMP loop2
	SUB VA,va
	SUBN Va,VA
	SHR VA
	SHR VA,VB
	LD I,lab1
	SHL VB,VB
	JMP V0, lab1
	RND  V5, #0xFF	
	DRW  V1,v2,#3
	SKNP  VE
	SKP  vb
	LD   vc, DT
	LD Va,K
	LD DT,Vb
	LD     st,VC
	ADD    i, VF
	LD     f, V3
	ld     b, Ve
	ld [i], V3
	ld V3,[i]
	EXIT 

lab1:
	RET
lab2:
	RET

lab3:	DB	1