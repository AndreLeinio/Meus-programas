
%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
static int yylex(void);
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t\r]+

%%

"void"		{return VOID;}
"if"            {return IF;}
"int"            {return INT;}
"return"            {return RETURN;}
"while"            {return WHILE;}
"else"          {return ELSE;}

"="            {return ASSIGN;}
"=="             {return EQ;}
"!="            { return DIF;}
"<"             {return LT;}
">"	          	{return GT;}
">="	          	{return GET;}
"<="	          	{return LET;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"["             {return LCOL;}
"]"             {return RCOL;}
"{"             {return LKEY;}
"}"             {return RKEY;}
";"             {return SEMI;}
","             {return COLON;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
"/*"             { char c;
                    int f = 1;
                  do
                  { c = input();
                    if (c == EOF) break;
                    if (c == '\n') lineno++;
                    while (c == '*'){
                      if((c = input()) == '/') {
                      f = 0;
                      break;
                      }
                    }

                  } while (f);
                }
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}
