#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

/* Precedences of expressions. */
const int EXPR_PRECEDENCES[] = {0, 0, 1, 1, 2, 2}; 

/* Struct to represent a stack of nodes. 'top' is the position where a new
   object is pushed, or the number of elements in the stack. 'arr' points to 
   the array that represents the stack itself. */
struct expr_stack {
	int top;
	struct expr **arr;
};

static void analyze (struct token *, struct expr_stack *, struct expr_stack *);
static void exhaust (struct expr_stack *, struct expr_stack *);
static void merge (struct expr_stack *, struct expr_stack *);
static struct expr_stack make_stack (void);
static void destroy_stack (struct expr_stack);
static void push (struct expr_stack *, struct expr *);
static struct expr *pop (struct expr_stack *);
static struct expr *peek (const struct expr_stack *);
static struct expr *token_to_expr (struct token, struct expr *, struct expr *);
static bool compare_precedence (enum token_type, enum expr_type);
static bool is_empty (struct expr_stack *);

/* Generates and returns a root of abstract syntax tree by doing lexical 
   analysis of given token sequence 'tokens'. 'tokens' must be terminated with
   TOKEN_NULL. */
struct expr *lexer (struct token *tokens) {
	struct token *t;
	struct expr *result;
	struct expr_stack expressions = make_stack ();
	struct expr_stack operators = make_stack ();

	for (t = tokens; !is_token_null (t); t++) 
    analyze (t, &expressions, &operators);

  exhaust (&expressions, &operators);

  assert (expressions.top == 1);
  assert (is_empty (&operators));

  result = expressions.arr[0];
  destroy_stack (expressions);
  destroy_stack (operators);

  return result;
}

/* Makes expression that has expression type of 't', left branch of 'left', and
   right branch of 'right'. */
struct expr *make_expr (enum expr_type t, struct expr *left, 
                        struct expr *right) {
  struct expr *new = (struct expr *) malloc (sizeof (struct expr));
  new->type = t;
  new->left = left;
  new->right = right;

  return new;
}

/* Destroys 'e', recursively. */
void destroy_expr (struct expr *e) {
  if (e == NULL)
    return;

  destroy_expr (e->left);
  destroy_expr (e->right);
  free (e);
}

/* Analyzes 't' using 'ops' using Shunting yard algorithm, pushes the result 
   into 'exprs'. */
static void analyze (struct token *t, struct expr_stack *exprs, 
                      struct expr_stack *ops) {
	if (t->type == TOKEN_NUMBER) {
    push (exprs, token_to_expr (*t, NULL, NULL));
    return;
  }

  if (is_operator_token (t)) {
    while (!is_empty (ops) && compare_precedence (t->type, peek (ops)->type))
      merge (exprs, ops);

    push (ops, token_to_expr (*t, NULL, NULL));
    return;
  }

  if (t->type == TOKEN_LEFT_PAREN) {
    push (ops, token_to_expr (*t, NULL, NULL));
    return;
  }

  if (t->type == TOKEN_RIGHT_PAREN) {
    assert (!is_empty (ops));

    while (peek (ops)->type != EXPR_PAREN)
      merge (exprs, ops);

    pop (ops);
    return;
  }
}

/* Exhausts remaining operators in 'ops', writing result to 'exprs'. */
static void exhaust (struct expr_stack *exprs, struct expr_stack *ops) {
  while (!is_empty (ops))
    merge (exprs, ops);
}

/* Pops two element from 'exprs', one element from 'ops', merges them into
   a expression, and pushes it back to 'exprs'. */
static void merge (struct expr_stack *exprs, struct expr_stack *ops) {
  struct expr *op, *left, *right;

  if (is_empty (ops))
    return;

  assert (exprs->top >= 2);

  op = pop (ops);
  right = pop (exprs);
  left = pop (exprs);

  op->left = left;
  op->right = right;

  push (exprs, op);
}

/* Constructs empty expression stack, and returns it. */
static struct expr_stack make_stack (void) {
  struct expr_stack e = {
    .arr = (struct expr **) malloc (sizeof (struct expr *) * MAX_TOKEN_NUM),
    .top = 0
  };

  return e;
}

/* Destroys stack 'st'. Note that it does not free expressions in 'st'. */
static void destroy_stack (struct expr_stack st) {
  free(st.arr);
}

/* Pushes 'e' into 'st'. */
static void push (struct expr_stack *st, struct expr *e) {
  st->arr[st->top++] = e;
}

/* Pops one element from 'st', and returns it. 'st' must not be empty. */
static struct expr *pop (struct expr_stack *st) {
  assert (st->top > 0);
  return st->arr[--st->top];
}

/* Retuns the top element of 'st', but does not pop it. 'st' must not be 
   empty. */
static struct expr *peek (const struct expr_stack *st) {
  assert (st->top > 0);
  return st->arr[st->top - 1];
}

/* Converts 't' into expression that has left branch of 'left' and right branch
   of 'right', and returns it. 't' must not be a null token and it does NOT
   initializes 'number' member of returnded expression when it is not an
   expression of EXPR_NUMBER type. Also, right parenthese are converted into 
   expression of EXPR_PAREN type which is just a dummy type for lexing 
   algorithm. */
static struct expr *token_to_expr (struct token t, struct expr *left, 
                                   struct expr *right) {
  struct expr *e = (struct expr *) malloc (sizeof (struct expr));
  assert (t.type != TOKEN_NULL);

  e->left = left;
  e->right = right;

  switch (t.type) {
    case TOKEN_NUMBER: 
      e->number = t.number; 
      e->type = EXPR_NUMBER;
      break;

    case TOKEN_PLUS: 
      e->type = EXPR_PLUS;
      break;

    case TOKEN_MINUS: 
      e->type = EXPR_MINUS;
      break;

    case TOKEN_MULTIPLY: 
      e->type = EXPR_MULTIPLY;
      break;

    case TOKEN_DIVIDE: 
      e->type = EXPR_DIVIDE;
      break;

    case TOKEN_RIGHT_PAREN: 
    case TOKEN_LEFT_PAREN: 
      e->type = EXPR_PAREN;
      break;

    default: ;
  }

  return e;
}

/* Compares token type 't' and expression type 'e', returns true if precedence
   of 't' is less than that of 'e', or precedence of 't' is equal to that of 
   'e' and 't' is left-associative. */
static bool compare_precedence (enum token_type t, enum expr_type e) {
  if (TOKEN_PRECEDENCES[t] < EXPR_PRECEDENCES[e])
    return true;

  if (TOKEN_PRECEDENCES[t] == EXPR_PRECEDENCES[e] 
      && TOKEN_ASSOCIATIVITY[t] == ASSOC_LEFT)
    return true;
  
  return false;
}

/* Returns true if 'st' is empty. */
static bool is_empty (struct expr_stack *st) {
  return st->top == 0;
}