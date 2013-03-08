#ifndef __CH8_H__
#define __CH8_H__


/* Some defs and macros*/
#define	BAD_ARGS (-1)
#define BAD_FILE (-2)

enum lex_kind {VAL, LAB, INST, REG, ADDR};
#define INST_NUM 11

/* What a lex/tok is */
typedef struct
{
  enum lex_kind kind;
  char *name;
  int val;
} lex;

/* What a line is */
typedef struct expression
{
  int addr;
  int line;
  lex *label;
  lex *inst;
  lex *src;
  lex *dest;
  lex *op;
  struct expression *next;
} expr;

/* What a prog is */
typedef struct{
  int lines;
  expr *first;
  expr *last;
} prog;


#define DEFAULT_ADDR 0x200

#endif
