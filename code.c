#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "yaca.h"
#include "code.h"

#define WRITE16(v) if (!error) fwrite(big2little16(v),sizeof(char)*2,1,myf)
#define WRITE8(v) if (!error) fwrite(v,sizeof(char)*1,1,myf)

#define ERROR_LINE(e) {fprintf(stderr,"Error - Line %d: ", e.line); error++;}

extern int _debug;

int error = 0;
static lex **mylab;
static FILE *myf;
static char *types[] = {"const value", "adress", "instruction", "register", "address"};


int *big2little16(int *opcode)
{
  int tmp = *opcode & 0xFF;
  *opcode>>=8;
  *opcode &= 0xFF;
  *opcode |= (tmp<<8);
}

int get_nb_ops(expr l)
{
  int num = 0;
  if (l.src != NULL) num++;
  if (l.dest != NULL) num++;
  if (l.op != NULL) num++;

  return num;
}

void code_scd(expr e)
{
  int opcode = 0x00C0;

  if (e.src->val > 0xF){
    ERROR_LINE(e);
    fprintf(stderr,"Constant value break 4bits limit\n");
    return;
  }
  opcode |= e.src->val;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_scr(expr e)
{
  int opcode = 0x00FB;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_low(expr e)
{
  int opcode = 0x00FE;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_high(expr e)
{
  int opcode = 0x00FF;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_scl(expr e)
{
  int opcode = 0x00FC;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

int search_in_lab(expr e)
{
  int i;

  for (i=0; mylab[i]!=NULL && i<50 && strcmp(mylab[i]->name,e.src->name); i++);
  
  if (i == 50 || mylab[i] == NULL){
    if (_debug)  printf("\n");
    ERROR_LINE(e);
    fprintf(stderr,"Label %s does not exist\n", e.src->name); 
    return ;//exit(BAD_LAB);
  }

  return mylab[i]->val;
}

void code_jmp(expr e)
{
  int opcode = (e.dest != NULL && e.dest->kind == REG)?0xB000:0x1000;
  
  
  opcode |= search_in_lab(e);
  if (_debug) printf("code: %x\n", opcode);
  WRITE16(&opcode);
}



void code_cls(expr e)
{
  int opcode = 0x00E0;
  if (_debug) printf("code: 00E0\n");
  WRITE16(&opcode);
}

void code_exit(expr e)
{
  int opcode = 0x00FD;
  if (_debug) printf("code: 00FD\n");
  WRITE16(&opcode);
}

void code_ld_r_c(expr e)
{
  int opcode = 0x6000;
  if (e.src->val > 0xFF){
    ERROR_LINE(e);
    fprintf(stderr,"Constant value break 1byte limit\n");
    return ;//exit(BAD_RANGE);
  }
  opcode |= (e.dest->val<<8);
  opcode |= (e.src->val);
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_ld_r_r(expr e)
{
  int opcode = 0x8000;
 
  opcode |= (e.dest->val<<8);
  opcode |= (e.src->val<<4);
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}


//TODO: gestion adresse directe
void code_ld_i_m(expr e)
{
  int opcode = 0xA000;
 
  opcode |=  search_in_lab(e);
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}


void code_ld_r_dt(expr e)
{
  int opcode = 0xF007;
 
  opcode |=  e.dest->val<<8;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_ld_r_k(expr e)
{
  int opcode = 0xF00A;
 
  opcode |=  e.dest->val<<8;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_ld_dt_r(expr e)
{
  int opcode = 0xF015;

  opcode |=  e.src->val<<8;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_ld_st_r(expr e)
{
  int opcode = 0xF018;
 
  opcode |=  e.src->val<<8;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_ld_f_r(expr e)
{
  int opcode = 0xF029;
 
  opcode |=  e.src->val<<8;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_ld_b_r(expr e)
{
  int opcode = 0xF033;
 
  opcode |=  e.src->val<<8;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_ld_i_r(expr e)
{
  int opcode = 0xF055;
 
  opcode |=  e.src->val<<8;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_ld_r_i(expr e)
{
  int opcode = 0xF065;
 
  opcode |=  e.src->val<<8;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_ld(expr e)
{
  if (e.src->kind == VAL) code_ld_r_c(e);
  else if (e.src->kind == REG && e.dest->kind == REG) code_ld_r_r(e);
  else if (!strcmp(e.dest->name,"i")) code_ld_i_m(e);
  else if (!strcmp(e.src->name,"dt")) code_ld_r_dt(e);
  else if (!strcmp(e.src->name,"k")) code_ld_r_k(e);
  else if (!strcmp(e.dest->name,"dt")) code_ld_dt_r(e);
  else if (!strcmp(e.dest->name,"st")) code_ld_st_r(e);
  else if (!strcmp(e.dest->name,"f")) code_ld_f_r(e);
  else if (!strcmp(e.dest->name,"b")) code_ld_b_r(e);
  else if (!strcmp(e.dest->name,"[i]")) code_ld_i_r(e);
  else if (!strcmp(e.src->name,"[i]")) code_ld_r_i(e);
  else if (e.src->kind == ADDR){
    ERROR_LINE(e);
    fprintf(stderr,"%s %s,%s: ",e.inst->name, e.dest->name, e.src->name);
    fprintf(stderr,"src operand has address type\n");
  }
  else {
    ERROR_LINE(e);
    fprintf(stderr,"%s %s,%s is not recognize\n",e.inst->name, e.dest->name, e.src->name);
  }
}

void code_or(expr e)
{
  int opcode = 0x8001;
 
  opcode |= (e.dest->val<<8);
  opcode |= (e.src->val<<4);
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_and(expr e)
{
  int opcode = 0x8002;
 
  opcode |= (e.dest->val<<8);
  opcode |= (e.src->val<<4);
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_xor(expr e)
{
  int opcode = 0x8003;
 
  opcode |= (e.dest->val<<8);
  opcode |= (e.src->val<<4);
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_sub(expr e)
{
  int opcode = 0x8005;
 
  opcode |= (e.dest->val<<8);
  opcode |= (e.src->val<<4);
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_shr(expr e)
{
  int opcode = 0x8006;
  
  opcode |= (e.dest != NULL)?e.dest->val<<8:e.src->val<<8;
  opcode |= (e.dest != NULL)?e.src->val<<4:0;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_subn(expr e)
{
  int opcode = 0x8007;
  
  opcode |= (e.dest->val<<8);
  opcode |= (e.src->val<<4);
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_shl(expr e)
{
  int opcode = 0x800E;
  
  opcode |= (e.dest->val<<8);
  opcode |= (e.src !=NULL)?(e.src->val<<4):0;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_call(expr e)
{
  int opcode = 0x2000;
  
  opcode |= search_in_lab(e);
  if (_debug) printf("code: %x\n", opcode);
  WRITE16(&opcode);
}

void code_rnd(expr e)
{
  int opcode = 0xC000;
  if (e.src->val > 0xFF){
    ERROR_LINE(e);
    fprintf(stderr,"Constant value break 1byte limit\n");
    return; //exit(BAD_RANGE);
  }
  opcode |= (e.dest->val<<8);
  opcode |= (e.src->val);
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_ret(expr e)
{
  int opcode = 0x00EE;
  if (_debug) printf("code: 00EE\n");
  WRITE16(&opcode);
}

void code_se_sne_c(expr e)
{
  int opcode = (!strcmp(e.inst->name,"se"))?0x3000:0x4000;
  
  if (e.src->val > 0xFF){
    ERROR_LINE(e);
    fprintf(stderr,"Constant value break 1byte limit\n");
    return ;//exit(BAD_RANGE);
  }
  
  opcode |= (e.dest->val<<8);
  opcode |= e.src->val;

  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_se(expr e)
{
  int opcode = 0x5000;
  
  opcode |= (e.dest->val<<8);
  opcode |= (e.src->val<<4);

  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_sne(expr e)
{
  int opcode = 0x9000;
  
  opcode |= (e.dest->val<<8);
  opcode |= (e.src->val<<4);

  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_se_sne(expr e)
{
  if (e.src->kind == VAL) code_se_sne_c(e);
  else if (e.src->kind == REG && !strcmp(e.inst->name, "se")) code_se(e);
  else if (e.src->kind == REG && !strcmp(e.inst->name, "sne")) code_sne(e);
  else if (e.dest->kind != REG){
    ERROR_LINE(e);
    fprintf(stderr,"dest operand has %s type and is not a register\n", types[e.dest->kind]);
    return ;//exit(BAD_OPS);
  }
  else{
    ERROR_LINE(e);
    fprintf(stderr,"src operand has %s type and is not a constant value\n", types[e.src->kind]);
    return ;
  }
}

void code_add_r_c(expr e)
{
   int opcode = 0x7000;
   
   if (e.src->val > 0xFF){
     ERROR_LINE(e);
     fprintf(stderr,"Constant value break 1byte limit\n");
     return ;//exit(BAD_RANGE);
   }
   
   opcode |= e.dest->val<<8;
   opcode |= e.src->val;
   if (_debug) printf("code: %X\n", opcode);
   WRITE16(&opcode);
}

void code_add_r_r(expr e)
{
   int opcode = 0x8004;
   
   opcode |= e.dest->val<<8;
   opcode |= e.src->val<<4;
   if (_debug) printf("code: %X\n", opcode);
   WRITE16(&opcode);
}

void code_add_i_r(expr e)
{
   int opcode = 0xF01E;
   
   opcode |= e.src->val<<8;
   if (_debug) printf("code: %X\n", opcode);
   WRITE16(&opcode);
}

void code_add(expr e)
{
  if (e.src->kind == VAL) code_add_r_c(e);
  else if (e.src->kind == REG && e.dest->kind == REG) code_add_r_r(e);
  else if (!strcmp(e.dest->name, "i")) code_add_i_r(e);
  else {
    ERROR_LINE(e);
    fprintf(stderr,"Unrecognize operand for %s\n", e.inst->name);
    return ;//exit(BAD_OPS);
  }
}

void code_drw(expr e)
{
  int opcode = 0xD000;

  if (e.op->kind != VAL){
    ERROR_LINE(e);
    fprintf(stderr,"Error: drw %s,%s,[%s] must have type const value but has type %s\n",e.src->name, e.dest->name, e.op->name, types[e.op->kind]);
    return ;//exit(BAD_VAL);
  }

  if (e.op->val > 0xF){
    ERROR_LINE(e);
    fprintf(stderr,"Constant value break 4bits limit\n");
    return ;//exit(BAD_RANGE);
  }
  
  opcode |= e.dest->val<<8;
  opcode |= e.src->val<<4;
  opcode |= e.op->val;
  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_skp_sknp(expr e)
{
  int opcode = (!strcmp(e.inst->name,"skp"))?0xE09E:0xE0A1;
  
  opcode |= (e.src->val<<8);

  if (_debug) printf("code: %X\n", opcode);
  WRITE16(&opcode);
}

void code_db_dw(expr e)
{
  int opcode = 0x0000;
  int is_db = !strcmp(e.inst->name,"db");
  if (e.src == NULL) return;
  opcode |= e.src->val;
  if (_debug) printf("code: %X\n", opcode);

  if (is_db && e.src->val > 0xFF){
    ERROR_LINE(e);
    fprintf(stderr,"Constant value break 1byte limit\n");
    return;
  } else if (!is_db && e.src->val > 0xFFFF){
    ERROR_LINE(e);
    fprintf(stderr,"Constant value break 2byte limit\n");
    return;
  }
  
  if (is_db) WRITE8(&opcode);
  else WRITE16(&opcode);
}



void debug_print_inst(expr l)
{
  int n = get_nb_ops(l);
  printf("%d: ",l.line);
  switch(n){
  case 0:
    printf("%s\t\t\t-> ", l.inst->name);
    break;
  case 1:
    printf("%s\t%s\t\t\t-> ", l.inst->name, l.src->name);
    break;
  case 2:
    printf("%s\t%s,%s\t-> ", l.inst->name, l.dest->name, l.src->name);
    break;
  case 3:
    printf("%s\t%s,%s,%s\t-> ", l.inst->name, l.dest->name, l.src->name, l.op->name);
  }
}

void code_prog(prog mprog, lex *lab[50], FILE *f)
{
  expr *tmp;
  mylab=lab;
  myf = f;
  
  for (tmp=mprog.first; tmp != NULL; tmp = tmp->next)
    {
      // NEED IMPROVEMENT FUNCTION POINTER TAB SHOULD DO THE WORK
      if (tmp->inst != NULL){
	if (_debug) debug_print_inst(*tmp);
	if (!strcmp(tmp->inst->name,"cls")) code_cls(*tmp);
	else if (!strcmp(tmp->inst->name,"ld")) code_ld(*tmp);
	else if (!strcmp(tmp->inst->name,"call")) code_call(*tmp);
	else if (!strcmp(tmp->inst->name,"ret")) code_ret(*tmp);
	else if (!strcmp(tmp->inst->name,"scd")) code_scd(*tmp);
	else if (!strcmp(tmp->inst->name,"scr")) code_scr(*tmp);
	else if (!strcmp(tmp->inst->name,"scl")) code_scl(*tmp);
	else if (!strcmp(tmp->inst->name,"low")) code_low(*tmp);
	else if (!strcmp(tmp->inst->name,"high")) code_high(*tmp);
	else if (!strcmp(tmp->inst->name,"jmp")) code_jmp(*tmp);
	else if (!strcmp(tmp->inst->name,"jp")) code_jmp(*tmp);
	else if (!strcmp(tmp->inst->name,"se")) code_se_sne(*tmp);
	else if (!strcmp(tmp->inst->name,"sne")) code_se_sne(*tmp);
	else if (!strncmp(tmp->inst->name,"add",3)) code_add(*tmp); // Ugly hack again
	else if (!strcmp(tmp->inst->name,"exit")) code_exit(*tmp);
	else if (!strcmp(tmp->inst->name,"or")) code_or(*tmp);
	else if (!strcmp(tmp->inst->name,"and")) code_and(*tmp);
	else if (!strcmp(tmp->inst->name,"xor")) code_xor(*tmp);
	else if (!strcmp(tmp->inst->name,"sub")) code_sub(*tmp);
	else if (!strcmp(tmp->inst->name,"shr")) code_shr(*tmp);
	else if (!strcmp(tmp->inst->name,"subn")) code_subn(*tmp);
	else if (!strcmp(tmp->inst->name,"shl")) code_shl(*tmp);
	else if (!strcmp(tmp->inst->name,"rnd")) code_rnd(*tmp);
	else if (!strcmp(tmp->inst->name,"drw")) code_drw(*tmp);
	else if (!strcmp(tmp->inst->name,"skp")) code_skp_sknp(*tmp);
	else if (!strcmp(tmp->inst->name,"sknp")) code_skp_sknp(*tmp);
	else if (!strcmp(tmp->inst->name,"db")) code_db_dw(*tmp);
	else if (!strcmp(tmp->inst->name,"dw")) code_db_dw(*tmp);
	else {
	  ERROR_LINE((*tmp));
	  fprintf(stderr,"Unrecognized %s instruction\n",tmp->inst->name);
	  //exit(BAD_OPS);
	}
      } 
    } 
}
