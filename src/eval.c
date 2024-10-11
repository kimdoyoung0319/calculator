#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

int evaluate (struct expr *);
int len (char *);

int main (void) {
  int result;
  struct token *parsed;
  struct expr *expression;

  char str[] = "(3 + (5 + 1))";

  parsed = parser (str, len (str));
  expression = lexer (parsed);
  result = evaluate (expression);
  destroy_expr (expression);

  printf("Evaluated result = %d\n", result);

  return 0;
}

/* Evaluates an expression that is represented by an abstract syntax tree named
   'e', returns the evaluated integer. */
int evaluate (struct expr *e) {
  int r, left, right;

  assert (e->type != EXPR_PAREN);

	if (e->type == EXPR_NUMBER)
		return e->number;

  assert (e->left != NULL && e->right != NULL);
  
  left = evaluate (e->left);
  right = evaluate (e->right);

  switch (e->type) {
    case EXPR_PLUS: 
      r = left + right;
      break;
    default: ;
  }

  return r;
}

/* Returns the length of null-terminated string 'str'. */
int len (char *str) {
  int i;
  for (i = 0; str[i] != '\0'; i++);

  return i;
}