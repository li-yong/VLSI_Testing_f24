%{
#include <stdio.h>
#include <stdlib.h>
%}

%token NUMBER

%%
calculation:
    expression '\n'  { printf("Result = %d\n", $1); }
  ;

expression:
    expression '+' expression { $$ = $1 + $3; }
  | expression '-' expression { $$ = $1 - $3; }
  | NUMBER                    { $$ = $1; }
  ;

%%

int main() {
  return yyparse();
}

void yyerror(const char *s) {
  fprintf(std