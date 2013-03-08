#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "yaca.h"
#include "code.h"


int _debug = 0;
extern int error;

static prog mprog;
static lex *lab[50] = {NULL};
static char *insts[] = {
  "cls", 
  "ret", 
  "add", 
  "ld",
  "and",
  "or",
  "xor",
  "sub",
  "subn",
  "jmp",
  "jp",
  "call",
  "scd",
  "scr",
  "scl",
  "low",
  "high",
  "se",
  "sne",
  "shr",
  "rsb",
  "shl",
  "mvi",
  "jmi",
  "rnd",
  "drw",
  /*  "xsprite" ,*/
  "skp",
  "sknp",
  "sys",
  "exit",
  "db",
  "dw",
  NULL
};

static int change_val_rep = 1;

#define STR2LOW(s)				\
  {						\
    char *tmp = s;				\
    while(*tmp != '\0'){			\
      *tmp = tolower(*tmp);			\
      tmp++;					\
    }						\
  }

#define RM_CR(s)					\
  {							\
    char *tmp = s+strlen(s);				\
    while(tmp != s && *tmp != '\15'){			\
      tmp--;						\
    }							\
    if (*tmp == '\15') *tmp = '\0';			\
  }

#define CHAR2VAL(s)  (int)(s == '.'?0:((s >= 'a' && s <= 'f')?s-'a'+10:s-'0'))

void populate_labels(void)
{
  expr *tmp;
  int i = 0;
  for (tmp=mprog.first; tmp != NULL; tmp = tmp->next)
    {
      if (tmp->label != NULL && tmp->label->kind == LAB)
	lab[i++] = tmp->label; 
    }
}

void print_pop_labs(void)
{
  int i;
  
  printf("Labels\n");
  for (i = 0; i<50 && lab[i] != NULL ; i++)
    printf("\t%s, 0x%X\n", lab[i]->name, lab[i]->val);

}

/* Prog and structures management */
void init_prog(void)
{
  mprog.last = NULL;
  mprog.first = mprog.last;
  mprog.lines = 0;
}

lex * create_lex(char *name, int val, enum lex_kind kind)
{
  lex *l = (lex *)malloc(sizeof(lex));

  l->name = name;
  l->val = val;
  l->kind = kind;

  return l;
}

expr * create_expr(int addr, lex *label, lex *inst, lex *src, lex *dest, lex *op, int line)
{
  expr *e = (expr *)malloc(sizeof(expr));
 
  e->addr = addr;
  e->label = label;
  e->inst = inst;
  e->src = src;
  e->dest = dest;
  e->op = op;
  e->line = line; 
  e->next = NULL;

  return e;
}

void add_expr(expr *e)
{
  if (mprog.first == NULL){
    mprog.first = e;
    mprog.last = e;
  }
  else {
    mprog.last->next = e;
    mprog.last = mprog.last->next;
  }
  mprog.lines++;
}

void free_lex(lex *l)
{
  if (l == NULL) return;
  if (l->name != NULL) free(l->name);
  free(l);
}

void free_expr(expr *e)
{
  if (e == NULL) return;
  free_lex(e->label);
  free_lex(e->inst);
  free_lex(e->src);
  free_lex(e->dest);
  free(e);
}

void free_prog()
{
  expr *tmp;
  
  /* CAUTION */
  for (tmp=mprog.first; tmp != NULL; tmp = tmp->next)
    free_expr(tmp);
}

void print_lex(lex *l)
{
  printf("Type=%d, name=%s, val=%d\n", l->kind, l->name, l->val);
}

void print_expr(expr *e)
{
  printf("Addr=0x%x\n", e->addr);
  if (e->label != NULL) print_lex(e->label);
  if (e->inst != NULL) print_lex(e->inst);
  if (e->src != NULL) print_lex(e->src);
  if (e->dest != NULL) print_lex(e->dest);
}

void print_prog()
{
  expr *tmp;
  printf("Lines %d\n",mprog.lines);
  for (tmp=mprog.first; tmp != NULL; tmp = tmp->next)
    {
      print_expr(tmp);
    }  
}


/* Manage tokens */
lex * create_lab(char *s, int addr)
{
  char *tmp;
  tmp = malloc(sizeof(char)*(strlen(s)-1));
  s[strlen(s)-1]='\0';
  STR2LOW(s);
  strcpy(tmp,s);

  return create_lex(tmp,addr,LAB);
}

lex * create_mem(char *s)
{
  char *tmp;
  tmp = malloc(sizeof(char)*(strlen(s)));
  strcpy(tmp,s);

  return create_lex(tmp,0,ADDR);
}

lex * create_inst(char *s)
{
  char *tmp;
  tmp = malloc(sizeof(char)*strlen(s));
  strcpy(tmp,s);
  return create_lex(tmp,0,INST);
}

lex * create_reg(char *s)
{
  char *tmp;
  int num = (int)((s[1] >= 'a' && s[1] <= 'f')?s[1]-'a'+10:s[1]-'0');
  tmp = malloc(sizeof(char)*sizeof(s));
  strcpy(tmp,s);
  return create_lex(tmp,num,REG);
}

lex * create_val(char *s)
{
  char *tmp;
  int val = 0;
  int dec = 0;

  tmp = malloc(sizeof(char)*strlen(s));
  if (change_val_rep) {
    strcpy(tmp,s);
    if (!strncmp(s,"$",1)) dec = 1, s++;
    if (!strncmp(s,"#",1)) dec = 4, s++;
  }
  else {
    strcpy(tmp,++s);
    STR2LOW(s);
    if (!strncmp(s,"0b",2)) dec = 1, s+=2;
    if (!strncmp(s,"0x",2)) dec = 4, s+=2;
  }

  if (dec != 0) {
    val =  CHAR2VAL(*s);
    for (s++;*s!='\0'; s++)
      val = (val<<dec) | CHAR2VAL(*s); 
  } else {
    val = CHAR2VAL(*s);
    for (s++;*s!='\0'; s++)
      val = val*10+CHAR2VAL(*s);
  } 
  return create_lex(tmp,val,VAL);
}

int is_inst(char *s)
{
  int i = 0;

  STR2LOW(s);  
  while (insts[i] != NULL && strcmp(insts[i], s))
    i++;
  
  return (insts[i] != NULL);
}

int is_label(char *s)
{
  return (s[strlen(s)-1] == ':');
}

int is_reg(char *s)
{  
  STR2LOW(s);
  
  return (strlen(s) == 2 
	  && s[0] == 'v' 
	  && isxdigit(s[1]));
}

int is_new_val(char *s)
{
  if (*s != '#') return (0==1);
  s++;
  
  
  while (*s != '\0'){
    if (*s != 'X' && *s != 'x' 
	&& !isxdigit(*s)) return (0==1); 
    s++;
  }
  
  return (0==0);
}


int is_old_val(char *s)
{
  
  while (*s != '\0'){
    if (*s != '#' && *s != '$' && *s != '.' 
	&& !isxdigit(*s)) return (0==1); 
    s++;
  }
  
  return (0==0);
}

int is_val(char *s)
{
  if (change_val_rep) return is_old_val(s);
  else return is_new_val(s);    
}

int is_white_line(char *s)
{
  int l = strlen(s), res = 0;
  
  for (;*s!='\0'; s++)
    if (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\15')
      res++;
  
  return (res==l);
}

void print_help()
{
  printf("YACA: Yet Another (nasty) Chip8 Compiler\n");
  printf("Usage: yaca {-a -o} [asm_file] [output_name]\n");
  printf("  --other, -o  \t manage other int representation\n  \t\t  ");
  printf("(with a '#' before each int and '0x' and '0b' \n\t\t  for hex and bin values)\n");
  printf("  --addr, -a n\t change base adress to n \n\t\t (in hex format 0xYYY, n could not be higher than 0xFFF)\n ");
}

int main(int argc, char *argv[])
{
  FILE *fin, *fout;
  char delims[] = "\t, ";
  char *result = NULL;
  char *str = (char *)malloc(1024 * sizeof(char));
  int base_addr = DEFAULT_ADDR;
  expr *first;
  int sd;
  int next = 1;
  int line = 1;

  /* Testing command line arguments */
  if (!strcmp(argv[1],"-h") 
      || !strcmp(argv[1],"--help")) print_help(), exit(0);

  if (argc < 3)
    {
      fprintf(stderr,"Usage: yaca {-o} [asm_file] [output_name]\n");
      exit(BAD_ARGS);
    }
  
  for (;next<argc-2;next++)
    if (!strcmp(argv[next],"-v")) _debug = 1;
    else if (!strcmp(argv[next],"--other") 
	     || !strcmp(argv[next], "-o"))  change_val_rep = 0;
    else if (!strcmp(argv[next],"--addr") 
	     || !strcmp(argv[next], "-a")){
      STR2LOW(argv[++next]);
      if (strncmp(argv[next],"0x",2)) 
	fprintf(stderr, "Bad addr format\n"), exit(BAD_ARGS);
      argv[next]+=2;
      base_addr =  CHAR2VAL(*argv[next]);
      for (*argv[next]++;*argv[next]!='\0'; argv[next]++)
	base_addr = (base_addr<<4) | CHAR2VAL(*argv[next]); 
    }
    
  
  /* Opening the input asm file */
      if ((fin=fopen(argv[next],"r")) == NULL)
	{
	  fprintf(stderr,"There is no file: %s\n", argv[next]);
	  exit(BAD_FILE);
	}

      /* Manage parsing and tokens */
      while (!feof(fin)){
	lex *lab = NULL, *inst = NULL, *srcdest[3];
	char copy[1024];
	int ops, i;
         
	fgets(str, 1023, fin); /* Get a line */
	RM_CR(str);
	while (str[0] == '\0' && !strcmp(str,"\n") && !feof(fin)){
	  fgets(str, 1023, fin); /* Get anoter line */
	  RM_CR(str);
	  line++;
	  if (_debug) printf("We skip blank lines\n");
	}
	if (feof(fin) && strlen(str) == 0) break; /* Ugly hack */

	for (i=0; str[i]!='\n' && str[i] != '\0' ; i++) ;
	str[i] = '\0';
    
	if (_debug) strcpy(copy,str); /* copy the line for display purposes */

	result = strtok( str, delims ); /* Parse into tokens */
	ops = 0;
	sd = 0; /* Another ugly hack */
	srcdest[0] = NULL; /* Again */
	srcdest[1] = NULL; /* Again */
	srcdest[2] = NULL; /* Again */
	while( result != NULL ) {
	  if (ops > 5){
	    fprintf(stderr,"Warning - Line %d: I do not manage multiple mem zone yet\n", line); 
	    ops = 0;
	    break;
	  }
	  if (*result == ';') break;
	  if (is_white_line(result)) break;
	  if (is_label(result)) lab = create_lab(result,base_addr);
	  else if (is_inst(result)) inst = create_inst(result);
	  else if (is_reg(result)) srcdest[sd++] = create_reg(result);
	  else if (is_val(result)) srcdest[sd++] = create_val(result);
	  else srcdest[sd++] = create_mem(result);
	  result = strtok( NULL, delims );
	  ops++;
	}
        
	if (ops > 0)
	  {
	    if (_debug) printf("LINE %d\n", line);
	    if (_debug) printf("On line %s\n there is %d ops\n", copy, ops);
	    if (ops >= 3 && lab == NULL)
	      first = create_expr(base_addr,lab,inst,srcdest[1],srcdest[0],srcdest[2],line);
	    else 
	      first = create_expr(base_addr,lab,inst,srcdest[0],srcdest[1],srcdest[2],line);
	    add_expr(first);
	    if (inst != NULL){
	      if (strcmp(inst->name,"db")) base_addr += 2;
	      else base_addr++;
	    }
	  }
	line++;
      }

      if (_debug) print_prog();
 
      populate_labels();
 
      if (_debug) print_pop_labs();
  
      /* Output file opening */
      if ((fout=fopen(argv[next+1], "w+")) == NULL){
	fprintf(stderr,"Error while creating %s\n", argv[next+1]);
	exit(BAD_FILE);
      }

      code_prog(mprog,lab,fout);

      free_prog();
      free(str);
      fclose(fin);
      fclose(fout);

      if (error){
	fprintf(stderr,"%d errors found!\n", error);
	remove(argv[next+1]);
      }

      return 0;
    }
