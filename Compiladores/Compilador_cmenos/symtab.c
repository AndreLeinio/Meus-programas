
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"


/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* the list of line numbers of the source
 * code in which a variable is referenced
 */
typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

/* The record in the bucket lists for
 * each variable, including name,
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
   { char * name;
     dataTypes Dtype;
     IDTypes IType;
     char* escopo;
     LineList lines;
     int memloc ; /* memory location for variable */
     struct BucketListRec * next;
   } * BucketList;

/* the hash table */
static BucketList hashTable[SIZE];

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int op, char* escopo, dataTypes DType, IDTypes IType )
{
  //printf("st_insert\n");
  int h = hash(name);
  BucketList l =  hashTable[h];


  // Procura a ultima declaração com o mesmo nome
  while ((l != NULL) && ((strcmp(name,l->name) != 0))){
    l = l->next;
  }

  //Para inserir: não achou outra declaração, se achou verificar se o escopo é diferente e não é uma função
  if ( l == NULL || (op != 0 && l->escopo != escopo && l->IType != FUN)) /* variable not yet in table */
  {
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = op;
    l->IType = IType;
    l->Dtype = DType;
    l->escopo = escopo;
    l->lines->next = NULL;
    l->next = hashTable[h];
    hashTable[h] = l;
  }

  else if( l->IType == FUN  && IType == VAR){
    fprintf(listing,"Erro: Nome da variavel %s já utilizada como nome de função.[%d]\n",name, lineno);
    Error = TRUE;
  }
  else if( l->escopo == escopo && op != 0)
  {
    fprintf(listing,"Erro: Variavel %s já declarada neste escopo.[%d]\n",name, lineno);
    Error = TRUE;
  }
  else if(l->escopo != escopo && (strcmp(l->escopo,"global") != 0) ){
    //procura por variavel global entes de supor que não existe
    while ((l != NULL)){
      if((strcmp(l->escopo, "global")==0)&& ((strcmp( name,l->name) == 0))){
        LineList t = l->lines;
        while (t->next != NULL) t = t->next;
        t->next = (LineList) malloc(sizeof(struct LineListRec));
        t->next->lineno = lineno;
        t->next->next = NULL;
        break;
      }
      l = l->next;
    }
    if(l == NULL){
      fprintf(listing,"Erro: Variavel %s não declarada neste escopo.[%d]\n",name, lineno);
      Error = TRUE;
    }
  }
  else if(op == 0)
  {
    LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name )
{
  //printf("st_lookup\n");
  int h = hash(name);
  BucketList l =  hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return -1;
  else return l->memloc;
}
void busca_main ()
{
  //printf("st_lookup\n");
  int h = hash("main");
  BucketList l =  hashTable[h];
  while ((l != NULL) && ((strcmp("main",l->name) != 0 || l->IType == VAR)))
    l = l->next;
  if (l == NULL) {
    fprintf(listing,"Erro: Função main não declarada\n");
    Error = TRUE;
  }

}
dataTypes getFunType(char* nome){
  int h = hash(nome);
  BucketList l =  hashTable[h];
  while ((l != NULL) && (strcmp(nome,l->name) != 0))
    l = l->next;

  if (l == NULL) return -1;
  else return l->Dtype;
}

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE * listing)
{
  //printf("printSymtab\n");
  int i;
  fprintf(listing,"Variable Name  Escopo  Tipo ID  Tipo dado  Line Numbers\n");
  fprintf(listing,"-------------  ------  -------  ---------  ------------\n");
  for (i=0;i<SIZE;++i)
  { if (hashTable[i] != NULL)
    { BucketList l = hashTable[i];
      while (l != NULL)
      { LineList t = l->lines;
        fprintf(listing,"%-14s ",l->name);
        fprintf(listing,"%-6s  ",l->escopo);
        char* id, *data;
        if(l->IType == VAR){
          id = "VAR";
        }
        else if(l->IType == FUN){
          id = "FUN";
        }
        if(l->Dtype == INTTYPE){
          data = "INT";
        }
        else if(l->Dtype == VOIDTYPE){
          data = "VOID";
        }
        fprintf(listing,"%-7s  ",id);
        fprintf(listing,"%-8s  ",data);

        while (t != NULL)
        { fprintf(listing,"%3d; ",t->lineno);
          t = t->next;
        }
        fprintf(listing,"\n");
        l = l->next;
      }
    }
  }
} /* printSymTab */
