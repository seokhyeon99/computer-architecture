#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int M[0x100000] = {0,};
int Reg[32] = {0,};
int PC = 0x0;
int fin_check = 0;
int R_count =0, J_count = 0, I_count = 0, M_count = 0, BT_count = 0, BNT_count = 0;
int cycle =  1;

void initialize();
int flip(int data);
void fetch();
void decode();
void execute();
char find_type(int opcode);
void control(int opcode, int read_data1, int v2, int imm, int func);
void memory_access();
void write_back();
void print_result();

typedef struct
{
        int ALUSrc;
	int RegDst;
	int Branch;
	int MemRead;
	int MemWrite;
	int MemtoReg;
	int RegWrite;
}control_signal;


typedef struct
{
	int PC;
	int inst;
}ifid_latch;

typedef struct
{
	int PC;
	int inst;
	int opcode, rs, rd, rt, shamt, func, addr, imm;
	int v1, v2;
	int simm;
	int WriteReg;
	char type;
	control_signal cs;
}idex_latch;

typedef struct
{
	int PC;
	int opcode;
	int v1, v2, v3;
	int rt;
	int simm;
	int WriteReg;
	int func;
	char type;
	control_signal cs;
}exmm_latch;

typedef struct
{
	int PC;
	int read_data;
	int v3;
	int WriteReg;
	char type;
	int func;
	int opcode;
	control_signal cs;
}mmwb_latch;

ifid_latch ifid[2];
idex_latch idex[2];
exmm_latch exmm[2];
mmwb_latch mmwb[2];

#define ADD_FUNC 0x20
#define ADDU_FUNC 0x21
#define SUB_FUNC 0x22
#define SUBU_FUNC 0x23
#define AND_FUNC 0x24
#define NOR_FUNC 0x27
#define OR_FUNC 0x25
#define JR_FUNC 0x8
#define JALR_FUNC 0x9
#define SLT_FUNC 0x2A
#define SLTU_FUNC 0X2B
#define SLL_FUNC 0X0
#define SRL_FUNC 0X2
#define ADDI 0x8
#define ADDIU 0x9
#define ANDI 0xC
#define ORI 0xD
#define BEQ 0x4
#define BNE 0x5
#define J 0x2
#define JAL 0x3
#define LUI 0xF
#define LW 0x23
#define SW 0x2B
#define SLTI 0xA
#define LL 0x30


void main(){
        FILE *fp=NULL;
	char *file;
        size_t ret = 0;
        int data = 0;
        int index = 0;
        int inst = 0;
        Reg[31] = -1;//ra
        Reg[29] = 0x100000;//sp
        //input file
	fp = fopen("input4.bin", "rb");
        if (fp == NULL){
                perror("no such input file");
        }
        while(1){
                //reading file to the eof
                ret = fread(&data, sizeof(data), 1, fp);
                
                //print out
                inst = flip(data);
                M[index/4] = inst;
                index = index + 4;
	if (ret == 0) break;
        }

	initialize();

	while(fin_check != -1){	   
	   fetch();
	   printf("[F]: PC : 0x%x, inst : 0x%X, Reg[2] : %d, Reg[3] : %d\n",ifid[0].PC,  ifid[0].inst, Reg[2], Reg[3]);    
	   write_back();
	   printf("[W] Reg[2] : %d, Reg[3] : %d\n", Reg[2], Reg[3]);
	   decode();
	   printf("[D] PC : 0x%x, inst : 0x%x, Reg[2] : %d, Reg[3] : %d\n",idex[0].PC, idex[0].inst, Reg[2], Reg[3]);
	   execute();
	   printf("[E] PC : 0x%x, Reg[2] : %d, Reg[3] : %d\n",exmm[0].PC, Reg[2], Reg[3]);
	   memory_access();
	   printf("[M] PC : 0x%x, Reg[2] : %d, Reg[3] : %d\n",mmwb[0].PC, Reg[2], Reg[3]);
	   
	   ifid[1] = ifid[0];
	   idex[1] = idex[0];
	   exmm[1] = exmm[0];
	   mmwb[1] = mmwb[0]; 
	   printf("-------------[cycle : %d]---------------------\n", cycle);
	   cycle++;
	}
	
	print_result();
        fclose(fp);
}
void initialize()
{
	memset(&ifid[0], 0, sizeof(ifid_latch));
	memset(&ifid[1], 0, sizeof(ifid_latch));
	memset(&idex[0], 0, sizeof(idex_latch));
	memset(&idex[1], 0, sizeof(idex_latch));
	memset(&exmm[0], 0, sizeof(exmm_latch));
	memset(&exmm[1], 0, sizeof(exmm_latch));
	memset(&mmwb[0], 0, sizeof(mmwb_latch));
	memset(&mmwb[1], 0, sizeof(mmwb_latch));
}

int flip(int data){
        int h1 = ((data & 0xff) << 24);
        int h2 = ((data & 0xff00) << 8);
        int h3 = ((data & 0xff0000) >> 8) & 0xff00;
        int h4 = ((data & 0xff000000) >> 24) & 0xff;
        int flip_data = h1 | h2 | h3 | h4;
        return flip_data;
}

void fetch()
{
	ifid[0].inst = M[PC / 4];
	ifid[0].PC = PC;
	PC = PC + 4;
}

void decode()
{	
	int inst = ifid[1].inst;
	int PC_D = ifid[1].PC;
	int opcode, rs, rt, rd, shamt, func, addr, imm;
	char type;
	int v1, v2;
	opcode = (inst & 0xFC000000) >> 26;
	type = find_type(opcode);
	if(type == 'R'){
		rs = (inst & 0x3E00000) >> 21;
		rt = (inst & 0x1F0000) >> 16;
		rd = (inst & 0xF800) >> 11;
		shamt = (inst & 0x7C0) >> 6;
		func = (inst & 0x3F);
	}									
	else if(type == 'J') addr = (inst & 0x3FFFFFF);	
	else{
		rs = (inst & 0x3E00000) >> 21;
		rt = (inst & 0x1F0000) >> 16;
		imm = (inst & 0x0000FFFF);
	}
	int jAddr = PC & 0xF0000000 | addr << 2;
	if(opcode == J){
                PC = jAddr;
		printf("j : PC = %X\n", PC);
	}
	if(opcode == JAL){
		Reg[31] = PC_D + 8;
                PC = jAddr;
                printf("jal : Reg[31] + PC +8, PC = %X",jAddr);
	}
	v1 = Reg[rs];
        v2 = Reg[rt];
	idex[0].PC = PC_D;
	idex[0].opcode = opcode;
        idex[0].rs = rs;
	idex[0].rt = rt;
        idex[0].rd = rd;
	idex[0].v1 = v1;
        idex[0].v2 = v2;
        idex[0].shamt = shamt;
        idex[0].func = func;
	idex[0].inst = inst;
        idex[0].simm = (imm >> 15)? (0xffff0000 | imm):imm;
        idex[0].type = type;
	idex[0].imm = imm;
	control(opcode, v1, v2, imm, func);
}

void execute()
{	
	int result, input2;
	int PC_E = idex[1].PC;
	int inst = idex[1].inst;
	int input1 = idex[1].v1;
	int opcode = idex[1].opcode;
	int rs = idex[1].rs;
	int rt = idex[1].rt;
	int rd = idex[1].rd;
	int func = idex[1].func;
	int shamt = idex[1].shamt;
	int simm = idex[1].simm;
	int bAddr = idex[1].simm << 2;
	int zimm = idex[1].imm;
	char type = idex[1].type;
	
	int ALUSrc = idex[1].cs.ALUSrc;
	int RegDst = idex[1].cs.RegDst;
	int MemRead = idex[1].cs.MemRead;
	int MemWrite = idex[1].cs.MemWrite;
	int MemtoReg = idex[1].cs.MemtoReg;
	int RegWrite = idex[1].cs.RegWrite;

	int temp_input1=idex[1].v1;
	int temp_input2=idex[1].v2;	



	if((rs != 0) && (rs == mmwb[1].WriteReg) && mmwb[1].cs.RegWrite) 
		{
		if(mmwb[1].opcode == LW)
			{
			printf("data dependency\n");
			temp_input1 = mmwb[1].read_data;}
		else
			{
			printf("data dependency\n");
			temp_input1 = mmwb[1].v3;}
		}
	if((rs != 0) && (rs == exmm[1].WriteReg) && exmm[1].cs.RegWrite)
		{printf("data dependency\n");

		temp_input1 = exmm[1].v3;}
	

	if((rt != 0) && (rt == mmwb[1].WriteReg) && mmwb[1].cs.RegWrite)
		{
		if(mmwb[1].opcode == LW)
			{printf("data dependency\n");
			temp_input2 = mmwb[1].read_data;}
		else
			{printf("data dependency\n");
			temp_input2 = mmwb[1].v3;}
		}
	if((rt != 0) && (rt == exmm[1].WriteReg) && exmm[1].cs.RegWrite)
		{printf("data dependency\n");
		temp_input2 = exmm[1].v3;
		}
	

		input1 = temp_input1;
	if(ALUSrc == 0)
		input2 = temp_input2;
	else
		input2 = idex[1].simm;


	if(type == 'R'){

		switch(func){
		case ADD_FUNC:
			result = input1 + input2;
                        printf("add || R[%d] = R[%d] + R[%d]\n", rd, rs, rt);
			break;
		case ADDU_FUNC:
			result = input1 + input2;
			printf("addu || R[%d] = R[%d] + R[%d]\n", rd,rs,rt);
			break;
		case AND_FUNC:
			result = input1 & input2;
			printf("and || R[%d] = R[%d] & R[%d]\n",rd,rs,rt);
			break;
		case JR_FUNC:
			PC = input1;
			PC_E = input1;
			printf("jr || PC = R[%d]\n",rs);
			break;
		case NOR_FUNC:
			result = ~(input1 | input2);
			printf("nor || R[%d] =~(R[%d] | R[%d])\n",rd,rs,rt);
			break;
		case OR_FUNC:
			result = input1 | input2;
			printf("or || R[%d] = R[%d] | R[%d]\n",rd,rs,rt);
			break;
		case SLT_FUNC:
			result = (input1 < input2) ? 1 : 0;
			printf("slt || R[%d] = (R[%d] < R[%d]) ? 1 : 0\n",rd,rs,rt);
			break;
		case SLTU_FUNC:
                        result = (input1 < input2) ? 1 : 0;
                        printf("sltu || R[%d] = (R[%d] < R[%d]) ? 1 : 0\n",rd,rs,rt);
			break;
		case SLL_FUNC:
			result = input2 << shamt;
			printf("sll || R[%d] = R[%d] << %d\n",rd,rs,shamt);
			break;
		case SRL_FUNC:
			result = input2 >> shamt;
			printf("srl || R[%d] + R[%d] >> %d\n",rd,rs,shamt);
			break;
		case SUB_FUNC:
			result = input1 - input2;
			printf("sub || R[%d] = R[%d] - R[%d]\n",rd,rs,rt);
			break;
		case SUBU_FUNC:
			result = input1 - input2;
			printf("subu || R[%d] = R[%d] - R[%d]\n",rd,rs,rt);	
			break;
		default :
			break;
		}
	}
	else if(type == 'I'){
		switch(opcode){
		case BEQ:
			if(input1 == input2)
				{
				memset(&ifid[0], 0, sizeof(ifid_latch));
				memset(&idex[0], 0, sizeof(idex_latch));
				PC = PC_E + bAddr +4;
				BT_count++;
				printf("control dependency\n");
				}
			else
				{
				BNT_count++;
				}
			break;
		case BNE:
			if(input1 != input2)
				{
				memset(&ifid[0], 0, sizeof(ifid_latch));
				memset(&idex[0], 0, sizeof(idex_latch));
				PC = PC_E + bAddr +4;
				BT_count++;
				printf("control dependency\n");
				}
			else
				{
				BNT_count++;
				}
			break;
		case ADDI:
			result = input1 + input2;
			printf("addi || R[%d] = R[%d] + %d\n",rt,rs,simm);
			break;
		case ADDIU:
			result = input1 + input2;
			printf("addi || R[%d] = R[%d] + %d\n",rt,rs,simm);
			break;
		case ANDI:
			result = input1 & zimm;
			printf("andi || R[%d] = R[%d] & %d\n",rt,rs,zimm);
			break;
		case LL:
			result = input1 + simm;
			printf("ll || R[%d] = M[R[%d] + %d]\n",rt,rs,simm);
			break;
		case ORI:
			result = input1 | zimm;
			printf("ori || R[%d] = R[%d] | %d\n",rt,rs,zimm);
			break;
		case SLTI:
			result = (input1 < simm)? 1: 0;
			printf("slti || R[%d] = (R[%d] < %d)? 1 : 0\n",rt,rs,simm);
			break;
		case SW:
			printf("sw ||\n");
			break;
		case LW:
			printf("lw ||\n");
			break;
		default :
			break;
		}
	}
	
	exmm[0].opcode = opcode;
	exmm[0].v1 = input1;
	exmm[0].rt = rt;
	exmm[0].v3 = result;
	exmm[0].v2 = temp_input2;
	exmm[0].simm = simm;
	
	if(idex[1].cs.RegDst == 1)
		exmm[0].WriteReg = rd;
	else
		exmm[0].WriteReg = rt;
	
	exmm[0].cs.MemRead = idex[1].cs.MemRead;
	exmm[0].cs.MemWrite = idex[1].cs.MemWrite;
	exmm[0].cs.MemtoReg = idex[1].cs.MemtoReg;
	exmm[0].cs.RegWrite = idex[1].cs.RegWrite;
	exmm[0].PC = PC_E;
}

char find_type(int opcode){
	char type;
        if(opcode == 0x0){
		printf("R type\n");
                type = 'R';
                R_count++;
        }
        else{
                if((opcode == J)||(opcode == JAL)){
		printf("J type\n");
                        type = 'J';
                        J_count++;
                }
                else{
		printf("I type\n");
                        type = 'I';
                        I_count++;
                }
        }
	return type;
}

void control(int opcode, int v1, int v2, int imm, int func)
{
	int RegDst, ALUSrc, MemtoReg, RegWrite, MemRead, MemWrite; 

	if(opcode == 0) RegDst = 1;
	else RegDst = 0;
	if((opcode != 0) && (opcode != BEQ) && (opcode != BNE)) ALUSrc = 1;
	else ALUSrc = 0;
	if(opcode == LW) MemtoReg = 1;
	else MemtoReg = 0;
	if((opcode == SW) || (opcode == BEQ) || (opcode == BNE) || 						(opcode == J) || (opcode == JAL)) RegWrite = 0;
	else RegWrite = 1;
	if(opcode == LW){MemRead = 1; M_count++;}
	else MemRead = 0;
	if(opcode == SW){MemWrite = 1; M_count++;}
	else MemWrite = 0;

	idex[0].cs.ALUSrc = ALUSrc;
        idex[0].cs.RegDst = RegDst;
        idex[0].cs.MemRead = MemRead;
	idex[0].cs.MemWrite = MemWrite;
	idex[0].cs.MemtoReg = MemtoReg;
	idex[0].cs.RegWrite = RegWrite;
}

void memory_access()
{
	int read_data;
	int PC_M = exmm[1].PC;
	int opcode = exmm[1].opcode;
	int rt = exmm[1].rt;
	int simm = exmm[1].simm;
	int func = exmm[1].func;
	int input1 = exmm[1].v1;
	int result = exmm[1].v3;
	int write_data = exmm[1].v2;
	
	int MemRead = exmm[1].cs.MemRead;
	int MemWrite = exmm[1].cs.MemWrite;
	int MemtoReg = exmm[1].cs.MemtoReg;
	int RegWrite = exmm[1].cs.RegWrite;

	
	switch(opcode){
	case SW:
		M[input1 + simm] = write_data;
		break;
	case LW:
                read_data = M[input1 + simm];
		break;
	default :
		break;
	}
	
	mmwb[0].PC = PC_M;
	mmwb[0].func = func;
	mmwb[0].opcode = opcode;
	mmwb[0].v3 = result;
	mmwb[0].WriteReg = exmm[1].WriteReg;
	mmwb[0].read_data = read_data;
	mmwb[0].cs.MemtoReg = exmm[1].cs.MemtoReg;	
	mmwb[0].cs.RegWrite = exmm[1].cs.RegWrite;
	mmwb[0].type = exmm[1].type;
}

void write_back()
{
	int opcode = mmwb[1].opcode;
	int rd, rt;
	int func = mmwb[1].func;
	int PC_W = mmwb[1].PC;
	int WriteReg = mmwb[1].WriteReg;
	char type = mmwb[1].type;
	int MemtoReg = mmwb[1].cs.MemtoReg;
	int result = mmwb[1].v3;
	int read_data = mmwb[1].read_data;
	int RegWrite = mmwb[1].cs.RegWrite;
	int write_data;

	if(RegWrite == 0)
		return;

	else
	{
	if(MemtoReg == 0)
		write_data = result;
	else
		write_data = read_data;
	
	
	if((type == 'R')&&(func != JR_FUNC))
		Reg[WriteReg] =write_data;
	else if((RegWrite == 1)&&(opcode!=JAL)&&(opcode!=J))
		Reg[WriteReg] = write_data;
	else
		return;
	}
	
	fin_check = mmwb[1].PC;
	
}

void print_result(){
        printf("-----------------------------[Result]----------------------------\n");
        printf("Reg[2] : %d, Cycle : %d\n", Reg[2],cycle-1);
	printf("Rtype : %d\n",R_count);
	printf("Itype : %d\n",I_count);
	printf("Jtype : %d\n",J_count);
	printf("Memory Access : %d\n",M_count);
	printf("Branch Taken : %d\n", BT_count);
	printf("Branch Not Taken : %d\n",BNT_count);
        printf("-----------------------------------------------------------------\n");
}


