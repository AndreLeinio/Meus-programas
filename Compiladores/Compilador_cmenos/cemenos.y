
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void);
int yyerror(char *msg);


%}

%token IF ELSE WHILE RETURN VOID
%right INT
%token ASSIGN EQ DIF LT GT GET LET PLUS MINUS TIMES OVER LPAREN RPAREN LCOL RCOL LKEY RKEY COLON SEMI
%token ID NUM
%token ERROR ENDFILE

/*daqui pra baixo tem new coisas*/
%nonassoc RPAREN
%nonassoc ELSE

%% /* Grammar for TINY */

program     : dec-list
                 { savedTree = $1;}
            ;

dec-list : dec-list dec
          {
            YYSTYPE t = $1;
              if (t != NULL){
                while (t->sibling != NULL)
                   t = t->sibling;
                t->sibling = $2;
                $$ = $1;
              }
              else $$ = $2;
          }
	      | dec { $$ = $1; }
	      ;

dec 	    : var-dec { $$ = $1 ;}
	    | fun-dec { $$ = $1; }
	    ;

var-dec : INT identificador
          SEMI
          {
            $$ = newExpNode(TypeK);
            $$->attr.name = "INT";
            $$->size = 1;
            $$->child[0] = $2;
            $2->kind.exp = VarDeclK;
            $2->type = INTTYPE;
          }
	      | INT identificador LCOL numero RCOL SEMI
            {
              $$ = newExpNode(TypeK);
              $$->attr.name = "INT";
              $$->size = $4->attr.val;
              $$->child[0] = $2;
              $2->kind.exp = VarDeclK;
              $2->type = INTTYPE;
            }
	      ;

tipo-espec  : INT
              {
                $$ = newExpNode(TypeK);
                $$->attr.name = "INT";
                $$->type = INTTYPE;
                $$->size = 1;
              }
            | VOID
              {
                $$ = newExpNode(TypeK);
                $$->attr.name = "VOID";
                $$->type = INTTYPE;
                $$->size = 1;
              }
            ;

fun-dec : INT identificador LPAREN params RPAREN composto-dec
            {
              $$ = newExpNode(TypeK);
              $$->attr.name = "INT";
              $$->child[0] = $2;
              $2->kind.exp = FunDeclK;
              $2->lineno = $$->lineno;
              $2->type = INTTYPE;
              $2->child[0] = $4;
              $2->child[1] = $6;
            }
        | VOID identificador LPAREN params RPAREN composto-dec
                    {
                      $$ = newExpNode(TypeK);
                      $$->attr.name = "VOID";
                      $$->child[0] = $2;
                      $2->type = VOIDTYPE;
                      $2->kind.exp = FunDeclK;
                      $2->lineno = $$->lineno;
                      $2->child[0] = $4;
                      $2->child[1] = $6;
                    }
        ;

params : param-list { $$ = $1; }
       | VOID
          {
            $$ = newExpNode(TypeK);
            $$->attr.name = "VOID";
            $$->size = 1;
            $$->child[0] = NULL;
          }
       ;

param-list : param-list COLON param-list
              {
                YYSTYPE t = $1;
                if (t != NULL){
                  while (t->sibling != NULL)
                       t = t->sibling;
                  t->sibling = $3;
                  $$ = $1;
                }
                else $$ = $3;
              }
           | param { $$ = $1; }
           ;

param : tipo-espec identificador
        {
          $$ = $1;
          $$->child[0] = $2;
          $2->kind.exp = ParamK;

        }
      | tipo-espec identificador LCOL RCOL
        {
          $$ = $1;
          $$->size = 0;
          $$->child[0] = $2;
          $2->kind.exp = ParamK;
        }
      ;

composto-dec : LKEY local-dec stmt-list RKEY
              {
                YYSTYPE t = $2;
                  if (t != NULL){
                    while (t->sibling != NULL)
                       t = t->sibling;
                    t->sibling = $3;
                    $$ = $2;
                  }
                  else $$ = $3;
              }
             | LKEY RKEY {}
             | LKEY  local-dec RKEY { $$ = $2; }
             | LKEY stmt-list RKEY { $$ = $2; }
             ;

local-dec : local-dec var-dec
            {
              YYSTYPE t = $1;
                if (t != NULL){
                  while (t->sibling != NULL)
                     t = t->sibling;
                  t->sibling = $2;
                  $$ = $1;
                }
                else $$ = $2;
            }
          | var-dec { $$ = $1; }
          ;

stmt-list : stmt-list stmt
            {
              YYSTYPE t = $1;
              if (t != NULL){
                while (t->sibling != NULL)
                t = t->sibling;
                t->sibling = $2;
                $$ = $1;
              }
              else $$ = $2;
            }
          | stmt { $$ = $1; }
          ;

stmt : exp-dec { $$ = $1; }
     | composto-dec { $$ = $1; }
     | sel-dec { $$ = $1; }
     | it-dec { $$ = $1; }
     | retorno-dec { $$ = $1; }
     ;

exp-dec : exp SEMI { $$ = $1; }
        |  SEMI {}
        ;

sel-dec : IF LPAREN exp RPAREN stmt
          {
            $$ = newStmtNode(IfK);
            $$->child[0] = $3;
            $$->child[1] = $5;
          }
        | IF LPAREN exp RPAREN stmt ELSE stmt
          {
            $$ = newStmtNode(IfK);
            $$->child[0] = $3;
            $$->child[1] = $5;
            $$->child[2] = $7;
          }
        ;

it-dec : WHILE LPAREN exp RPAREN stmt
        {
          $$ = newStmtNode(WhileK);
          $$->child[0] = $3;
          $$->child[1] = $5;
        }
        ;

retorno-dec : RETURN SEMI { $$ = newStmtNode(ReturnK); }
            | RETURN exp SEMI
              {
                $$ = newStmtNode(ReturnK);
                $$->child[0] = $2;
              }
            ;

exp : var ASSIGN exp
      {
        $$ = newStmtNode(AssignK);
        $$->child[0] = $1;
        // $$->child[0]->kind.exp = AssignElK;
        $$->child[1] = $3;
        // $$->child[1]->kind.exp = AssignElK;
      }
    | simples-exp { $$ = $1; }
    ;

var : identificador { $$ = $1; }
    | identificador LCOL exp RCOL
      {
        $$ = $1;
        $$->type = INTTYPE;
        $$->child[0] = $3;
      }
    ;

simples-exp : soma-exp relacional soma-exp
              {
                  $$ = $2;
                  $$->child[0] = $1;
                  $$->child[1] = $3;
              }
            | soma-exp { $$ = $1; }
            ;

relacional : EQ
              {
                $$ = newExpNode(OpK);
                $$->attr.op = EQ;
              }
           | LT
              {
                $$ = newExpNode(OpK);
                $$->attr.op = LT;
              }
           | GT
              {
                $$ = newExpNode(OpK);
                $$->attr.op = GT;
              }
           | GET
              {
                $$ = newExpNode(OpK);
                $$->attr.op = GET;
              }
           | LET
              {
                $$ = newExpNode(OpK);
                $$->attr.op = LET;
              }
           | DIF
              {
                $$ = newExpNode(OpK);
                $$->attr.op = DIF;
              }
           ;

soma-exp : soma-exp soma termo {
            $$ = $2;
            $$->child[0] = $1;
            $$->child[1] = $3;
         }
         | termo { $$ = $1; }
         ;

soma : PLUS
       {
         $$ = newExpNode(OpK);
         $$->attr.op = PLUS;
       }
     | MINUS
        {
          $$ = newExpNode(OpK);
          $$->attr.op = MINUS;
        }
     ;

termo : termo mult fator
            {
              $$ = $2;
              $$->child[0] = $1;
              $$->child[1] = $3;
            }
      | fator { $$ = $1; }
      ;

mult : TIMES
       {
         $$ = newExpNode(OpK);
         $$->attr.op = TIMES;
       }
     | OVER
        {
          $$ = newExpNode(OpK);
          $$->attr.op = OVER;
        }
     ;

fator : LPAREN exp RPAREN { $$ = $2; }
      | var { $$ = $1; }
      | ativ { $$ = $1; }
      | numero { $$ = $1; }
      ;

ativ : identificador LPAREN arg-list RPAREN
        {
          $$ = newExpNode(AtivK);
          $$->attr.name = $1->attr.name;
          $$->child[0] = $3;

        }
        | identificador LPAREN RPAREN
         {
           $$ = newExpNode(AtivK);
           $$->attr.name = $1->attr.name;
         }
     ;


arg-list : arg-list COLON exp
            {
              YYSTYPE t = $1;
              if (t != NULL){
                while (t->sibling != NULL)
                t = t->sibling;
                t->sibling = $3;
                $$ = $1;
              }
              else $$ = $3;
            }
         | exp { $$ = $1; }
         ;
identificador : ID
                {
                  $$ = newExpNode(IdK);
                  $$->attr.name = copyString(tokenString);
                }
              ;
numero : NUM
          {
            $$ = newExpNode(ConstK);
            $$->type = INTTYPE;
            $$->attr.val = atoi(tokenString);
          }

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}
