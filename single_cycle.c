#include <stdio.h>
#include <errno.h>

//function declaration
int flip(int data);
int fetch();
void decode(int inst);
void control();
void find_type();
void execute();
void memory_access();
void write_back();
void update_pc();
void print_result();
//Declare 1MB memory structure
//memory has code and data
int Memory[0x100000];
//program counter(or instruction pointer)
int PC = 0;
//cpu registers
int Reg[32];
struct seper_inst
{
        int opcode;
        int rs;
        int rt;
        int rd;
        int shamt;
        int func;
        int imm;
        int addr;
        int simm;
        int zimm;
        int bAddr;
        int jAddr;
        char type;

        int read_reg1;
        int read_reg2;
        int write_reg;
        int write_data;

        int read_data1;
        int read_data2;

        int alu_result;
};
struct control_signal
{
        int RegDest;
        int ALUSrc;
        int MemtoReg;
        int RegWrite;
        int MemRead;
        int MemWrite;
        int PCSrc1;
        int PCSrc2;
        int bcond;
};
struct seper_inst s;
struct control_signal cs;
int I_count = 0;
int J_count = 0;
int R_count = 0;
int inst_count=0;
int value = 0;
int B_count = 0;
int M_count = 0;

int main(){
        FILE *fp=NULL;
        size_t ret = 0;
        int data = 0;
        int index = 0;
        int inst=0;
        Reg[31] = 0xffffffff;//ra
        Reg[29] = 0x100000;//sp
        //input file
        fp = fopen("input4.bin", "rb");
        if (fp == NULL){
                perror("no such input file");
        }
        while(1){
                //inst = 0;
                //reading file to the eof
                ret = fread(&data, sizeof(data), 1, fp);
                if (ret == 0) break;
                //print out
                inst = flip(data);
                Memory[index/4] = inst;
                index = index + 4;
        }
        fclose(fp);
        while(1){
                //fetch
                inst = fetch();
                //decode
                decode(inst);
                //execute
                //run the ALU
                execute();
                //load/store result to memory
                memory_access();
                //update register value
                write_back();
                //update PC
                update_pc();
                if (PC == 0xffffffff) break;
        }
        print_result();
        return 0;
}//main
int flip(int data){
        int h1 = ((data & 0xff) << 24);
        int h2 = ((data & 0xff00) << 8);
        int h3 = ((data & 0xff0000) >> 8) & 0xff00;
        int h4 = ((data & 0xff000000) >> 24) & 0xff;
        int flip_data = h1 | h2 | h3 | h4;
        return flip_data;
}
int fetch(){
        return Memory[PC/4];
}
void decode(int inst){
        s.opcode = (inst & 0xfc000000)>>26;
        s.rs     = (inst & 0x03e00000)>>21;
        s.rt     = (inst & 0x001f0000)>>16;
        s.rd     = (inst & 0x0000f800)>>11;
        s.shamt  = (inst & 0x000007c0)>>6;
        s.func   = (inst & 0x0000003f)>>0;
        s.imm    = (inst & 0x0000ffff)>>0;
        s.addr   = (inst & 0x03ffffff)>>0;
        s.simm   = (s.imm >> 15)? (0xffff0000 | s.imm):s.imm;
        s.zimm   = (0x00000000 | s.imm);
        s.bAddr  = s.simm<<2;
        s.jAddr  = ((PC+4 & 0xf0000000)|s.addr)<<2;
        find_type();
        control();


        //control signal setting
        s.read_reg1 = s.rs;
        s.read_reg2 = s.rt;

        if(cs.RegDest==1)
                s.write_reg = s.rd;
        else
                s.write_reg = s.rt;

        s.read_data1 = Reg[s.read_reg1];
        s.read_data2 = Reg[s.read_reg2];

}
void control(){
        if(s.opcode==0) cs.RegDest = 1;
        else cs.RegDest = 0;
        if((s.opcode!=0)&&(s.opcode!=0x4)&&(s.opcode!=0x5)) cs.ALUSrc = 1;
        else cs.ALUSrc = 0;
        if(s.opcode==0x23) cs.MemtoReg = 1;
        else cs.MemtoReg = 0;
        if((s.opcode==0x2b)||(s.opcode==0x4)||(s.opcode==0x5)||(s.opcode==0x2)||((s.opcode==0x0)&&(s.func==0x8)))
                cs.RegWrite = 0;
        else
                cs.RegWrite = 1;
        if(s.opcode==0x23) cs.MemRead = 1;
        else cs.MemRead = 0;
        if(s.opcode==0x2b) cs.MemWrite = 1;
        else cs.MemWrite = 0;
        if((s.opcode==0x2)||(s.opcode==0x3)) cs.PCSrc1 = 1;
        else cs.PCSrc1 = 0;
        if((s.opcode==0x4)&&(s.opcode==0x5)) cs.PCSrc2 = 1;
        else cs.PCSrc2 = 0;
        cs.bcond = 0;
}
void find_type(){
        if(s.opcode == 0x0){
                s.type = 'R';
                R_count++;
        }
        else{
                if((s.opcode == 0x2)||(s.opcode == 0x3)){
                        s.type = 'J';
                        J_count++;
                }
                else{
                        s.type = 'I';
                        I_count++;
                }
        }
}
void execute(){
        int data1 = 0 , data2 = 0;
        data1 = s.read_data1;
        if(cs.ALUSrc) // I type
                data2 = s.simm;
        else // R type
                data2 = s.read_data2;

        switch(s.opcode){
                case 0x0://R-type instruction
                        switch(s.func){
                                case 0x20://add instruction
                                        value = data1 + data2;
                                        break;
                                case 0x21://addu instruction
                                        value = data1 + data2;
                                        break;
                                case 0x22://sub instruction
                                        value = data1 - data2;
                                        break;
                                case 0x23://subu instruction
                                        value = data1 - data2;
                                        break;
                                case 0x24://and instruction
                                        value = data1 & data2;
                                        break;
                                case 0x27://nor instruction
                                        value = !(data1 | data2);
                                        break;
                                case 0x25://or instruction
                                        value = (data1 | data2);
                                        break;
                                case 0x2a://slt instruction
                                        value = (data1 < data2)? 1 : 0;
                                        break;
                                case 0x2b://sltu instruction
                                        value = (data1 < data2)? 1 : 0;
                                        break;
                                case 0x0://sll instruction
                                        value = Reg[s.rt]<<s.shamt;
                                        break;
                                case 0x2://srl instruction
                                        value = Reg[s.rt]>>s.shamt;
                                        break;
                        }
                        break;
                case 0x8://addi instruction(I-type)
                        value = data1 + data2;
                        break;
                case 0x9://addiu instruction(I-type)
                        value = data1 + data2;
                        break;
                case 0xc://andi instruction(I-type)
                        value = data1 & data2;
                        break;
                case 0xd://ori instruction(I-type)
                        value = (data1 | s.zimm);
                        break;
                case 0xf://lui instruction(I-type)
                        value = (data2<<16);
                        break;
                case 0xa://slti instruction(I-type)
                        value = (data1 < data2)? 1: 0;
                        break;
                case 0x23://lw instruction(I-type)
                        value = data1 + data2;
                        break;
                case 0x2b://sw instruction(I-type)
                        value = data1 + data2;
                        break;
                case 0x4://beq instruction(I-type)
                        if (data1 == data2){
                                PC += s.bAddr + 4;
                                cs.bcond=1;
                        }
                        else{
                                cs.bcond=0;
                                PC += 4;
                        }
                        B_count++;
                        break;
                case 0x5://bne instruction(I-type)
                        if (data1 != data2){
                                PC += s.bAddr + 4;
                                cs.bcond=1;
                        }
                        else{
                                cs.bcond=0;
                                PC += 4;
                        }
                        B_count++;
                        break;

        }
        s.alu_result = value;
}
void memory_access(){
        int read_data = 0;
        if(cs.MemWrite){ // sw
                Memory[s.alu_result/4] = Reg[s.rt];
                M_count++;
        }
        if(cs.MemRead){ // lw
                read_data = Memory[s.alu_result/4];
                M_count++;
        }

        if(cs.MemtoReg)//lw
                s.write_data = read_data;
        else
                s.write_data = s.alu_result;
}
void write_back(){

        if(cs.RegWrite){ //write back
                if(s.opcode == 0x3){ //jal
                        Reg[31] = PC + 8;
                        PC = s.jAddr;
                }
                else{
                        Reg[s.write_reg] = s.write_data;
                }
        }
        if(s.opcode==0x0 && s.func == 0x8){ //jr
                PC = Reg[s.rs];
        }
        if(s.opcode==0x2){ //j
                PC = s.jAddr;
        }
}
void update_pc(){
        if((s.opcode == 0x2) || (s.opcode == 0x3) ||
                        (s.opcode == 0x4) || (s.opcode == 0x5)||
                        ((s.opcode == 0x0) && (s.func == 0x8))){
        }
        else{
                PC = PC + 4;
        }
        inst_count++;
}
void print_result(){
        printf("-----------------------------[Result]----------------------------\n");
        printf("|\t\tNumber of executed instruction = %d\t|\n", inst_count);
        printf("|\t\tNumber of R-type instruction = %d\t\t|\n", R_count);
        printf("|\t\tNumber of I-type instruction = %d\t\t|\n", I_count);
        printf("|\t\tNumber of J-type instruction = %d\t\t|\n", J_count);
        printf("|\t\tNumber of memory access instruction = %d\t|\n", M_count);
        printf("|\t\tNumber of taken branches = %d\t\t|\n", B_count);
        printf("|\t\tFinal return value(R[2]) = %d\t\t\t|\n",Reg[2]);
        printf("-----------------------------------------------------------------\n");
}
