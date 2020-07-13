#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//int Reg[10];
char* cut_str(char* op, char* s1, char* line);
//void operand(char* opr, int n);
void mov(int a, int b);
void add(int a, int b);
void sub(int a, int b);
void mul(int a, int b);
void divide(int a, int b);
void GCD(int a, int b);
void cmpr(int a, int b);
int jump(int n1, int index, char* argv[]);
int branch(int n1, int index, char* argv[]);

int Reg[10];
FILE *fp;
char *line=0;
size_t n;

struct Operation_Variables{
	char *op;
        int index;
        int index1;
        int index2;
        int n1;
        int n2;
};

int main(int argc, char* argv[]){
        
	struct Operation_Variables OV;
	OV.index = 0;
	OV.index1 = 0;
	OV.index2 = 0;	
        ssize_t len;//signed signed int (반환값으로 해당 IO 함수의 실패여부를 알려준다 )
	char *opr1;
	char *opr2;
	char *err_str;

	int ret;
	int result;
	char inequality;
	int chng_line = 0;

        printf("argc: %d, argv[0]: %s, argv[1]: %s\n", argc, argv[0], argv[1]);

        if (argc == 2)
                fp=fopen(argv[1], "r");//input file 입력시 받아들임
        else
                fp=fopen("input.txt", "r");//input file 미입력시 자동으로 input.txt 읽음
        while (1){

                len = getline(&line,&n, fp);
		if (len == -1) break;
		printf("[%d] %s", OV.index + 1, line);
		OV.index++;

		if (*(OV.op) == 'H'){
                	printf("termination\n");
                        break;
                }

		//operator
		OV.op = cut_str(OV.op, "opcode", line);	
			
		//operand1
		opr1 = cut_str(opr1, "opr1", NULL);
		//if it is a number
		if (*opr1 >= '0' && *opr1 <= '9'){ 
			//포인터변수이므로 첫번째자리 값만 받아옴| 0<=opr1<=9이면
			OV.n1 = atoi(opr1); //char to int
			printf("n1: %d ", OV.n1);
		}
		else{
			// this should be register index
			// e.g.) R1 or R2 ...
			if (*opr1 == 'R' || *opr1 == 'r'){
				OV.index1 = atoi(opr1 + 1);//다음자리 값으로 이동
				OV.n1 = Reg[OV.index1];
				printf("R_opr1: %d (val:%d) ", OV.index1, OV.n1);
			}	
			else{
				//error mal-format number
				OV.n1 = 0;
			}
		}

		//operand2
		//if(*(OV.op) != 'J' && *(OV.op) != 'B'){
		
		opr2 = cut_str(opr2, "opr2", NULL);
		if (opr2 != NULL){
		//if it is a number
			if (*opr2 >= '0' && *opr2 <= '9'){
                       	//포인터변수이므로 첫번째자리 값만 받아옴| 0<=opr1<=9이면
                       	
				OV.n2 = atoi(opr2); //char to int
                         	
				printf("n2: %d\n", OV.n2);
                	
			}
                	else{
                        // this should be register index
                        // e.g.) R1 or R2 ...
                        	if (*opr2 == 'R' || *opr2 == 'r'){
                                 
					OV.index2 = atoi(opr2 + 1);//다음자리 값으로 이동
                                
					OV.n2 = Reg[OV.index2];
                                	printf("R_opr2: %d (val:%d)\n ", OV.index2, OV.n2);
                          	
				}
                        	else{
                               		//error mal-format number
                                	OV.n2 = 0;
                        	}
                	}
		}
		//}
		
		//execution          
		if (*(OV.op) == 'M'){
			mov(OV.index1, OV.index2);
		}
		if (*(OV.op) == '+'){
			add(OV.n1, OV.n2);
		}
		if (*(OV.op) == '-'){
                        sub(OV.n1, OV.n2);
		}
		if (*(OV.op) == '*'){
                        mul(OV.n1, OV.n2);
		}
		if (*(OV.op) == '/'){
			divide(OV.n1, OV.n2);
		}
		if (*(OV.op) == 'C'){
                        cmpr(OV.n1, OV.n2);
			}
		if (*(OV.op) == 'J'){
			OV.index = jump(OV.n1, OV.index, argv);
		}
		if (*(OV.op) == 'B'){
			OV.index =  branch(OV.n1, OV.index, argv);
		}
		if (*(OV.op) == 'G'){//GCD
			GCD(OV.n1, OV.n2);
			printf(" GCD( %d, %d)\n", OV.n1, OV.n2);
		}		
	}
	
	free(line);
	fclose(fp);

	return 0;
}

char* cut_str(char* op, char* s1, char* line){
	op = strtok(line, " \t\n");//' ', \t, \n  앞에서 문자열 자르기
        printf("%s: %s ",s1, op);
	return op;
}
/*void operand(char* opr, int n){

	int k;
	if (*opr >= '0' && *opr1 <= '9'){
		k = atoi(opr1);
		printf("n%d: %d", n, k);
		
	}
	else{
		if(*opr == 'R' || *opr == 'r'){
			n1 = atoi(opr + 1);
			k = Reg[k];
			printf("R_opr%d: %d (val:%d) ", n, n1, k);
		}
		else{
			k = 0;
	} 
}*/
void mov(int a, int b){
	Reg[a] = Reg[b];
        printf("Result: R%d (%d) <= R%d\n", a, Reg[a], b);
}
void add(int a, int b){
	Reg[0] = a + b;
	printf("Result: %d := %d + %d\n", Reg[0], a, b);
}
void sub(int a, int b){
	Reg[0] = a - b;
	printf("Result: %d := %d - %d\n", Reg[0], a, b);
}
void mul(int a, int b){
	Reg[0] = a * b;
	printf("Result: %d := %d * %d\n", Reg[0], a, b);
}
void divide(int a, int b){
	Reg[0] = a / b;
	printf("Result: %d := %d / %d\n", Reg[0], a, b);
}
void GCD(int a, int b){
	while (a != b){
	if (a > b)
		a -= b;
	else
		b -= a;
	}
	Reg[0] = a;
	printf("Result: %d =", Reg[0]);
}
void cmpr(int a, int b){
	char inequality;
	if (a < b){
		inequality = '<';
		Reg[0] = 1;
	}
	else{
		inequality = '>';
		Reg[0] = 0;
	}
	printf("Result: %d : %d %c %d\n", Reg[0], a, inequality, b);
}
int jump(int n1, int index, char* argv[]){
	printf(" *jump to line %d\n", n1);

	if (n1 > index){
		int chng_line = n1 - index;
		for (int i = 1; i < chng_line; i++){
			getline(&line, &n, fp);
			index++;
		}
	}
	else{
		index = n1 - 1;
		free(line);
		line = NULL;
		fclose(fp);
		fp = fopen(argv[1], "r");
		for(int i = 0; i < index; i++){
			getline(&line, &n, fp);
		}
	}
	printf(" \r");
	return index;
}

int branch(int n1, int index, char* argv[]){
	if (Reg[0] == 1)
		index = jump(n1, index, argv);
	else printf(" \n");
	return index;
}
