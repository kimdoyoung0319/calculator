#include "parser.h"
#include "lexer.h"
#include "utils.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Precedences of expressions. */
const int EXPR_PRECEDENCES[] = {0, 0, 1, 1, 2, 2};

/* Struct to represent a stack of nodes. `top` is the position where a new
 * object is pushed, or the number of elements in the stack. `arr` points to
 * the array that represents the stack itself. */
struct expr_stack {
    int top;
    struct expr **arr;
};

static int analyze (struct token *, struct expr_stack *, struct expr_stack *);
static int exhaust (struct expr_stack *, struct expr_stack *);
static int merge (struct expr_stack *, struct expr_stack *);
static struct expr_stack make_stack (void);
static void destroy_stack (struct expr_stack);
static void push (struct expr_stack *, struct expr *);
static struct expr *pop (struct expr_stack *);
static struct expr *peek (const struct expr_stack *);
static struct expr *token_to_expr (struct token, struct expr *, struct expr *);
static bool compare_precedence (enum token_type, enum expr_type);
static bool is_empty (struct expr_stack *);

/* Generates and writes the pointer to it to `expr` a root of abstract syntax
 * tree by parsing given token sequence `tokens`. `tokens` must be terminated
 * with TOKEN_NULL. Returns zero if parsing succeeded. Otherwise, returns -1 and
 * writes nothing in `*expr`. */
int parser (struct token *tokens, struct expr **expr) {
    int result = 0;

    struct expr *e;
    struct token *t;
    struct expr_stack expressions = make_stack ();
    struct expr_stack operators = make_stack ();

    for (t = tokens; !is_token_null (t); t++) {
        if (analyze (t, &expressions, &operators) < 0) {
            result = -1;
            goto error;
        }
    }

    if (exhaust (&expressions, &operators) < 0 || expressions.top != 1 ||
        !is_empty (&operators)) {
        result = -1;
        goto error;
    };

    *expr = expressions.arr[0];
    goto end;

error:
    while (!is_empty (&expressions)) {
        e = pop (&expressions);
        destroy_expr (e);
    }

    while (!is_empty (&operators)) {
        e = pop (&operators);
        destroy_expr (e);
    }

end:
    destroy_stack (expressions);
    destroy_stack (operators);
    return result;
}

/* Makes expression that has expression type of `t`, number of `n`, left branch
 * of `left`, and right branch of `right`. */
struct expr *make_expr (enum expr_type t, int n, struct expr *left,
                        struct expr *right) {
    struct expr *new = malloc (sizeof (struct expr));
    assert_nonnull (new, "ERROR: out of memory\n");

    new->type = t;
    new->number = n;
    new->left = left;
    new->right = right;

    return new;
}

/* Destroys `e`, recursively. */
void destroy_expr (struct expr *e) {
    if (e == NULL)
        return;

    destroy_expr (e->left);
    destroy_expr (e->right);
    free (e);
}

/* Analyzes `t` using `ops` using Shunting yard algorithm, pushes the result
 * into `exprs`. Returns zero if the analysis was successful, -1 otherwise. */
static int analyze (struct token *t, struct expr_stack *exprs,
                    struct expr_stack *ops) {
    if (t->type == TOKEN_NUMBER) {
        push (exprs, token_to_expr (*t, NULL, NULL));
        return 0;
    }

    if (is_operator_token (t)) {
        while (!is_empty (ops) &&
               compare_precedence (t->type, peek (ops)->type))
            if (merge (exprs, ops) < 0)
                return -1;

        push (ops, token_to_expr (*t, NULL, NULL));
        return 0;
    }

    if (t->type == TOKEN_LEFT_PAREN) {
        push (ops, token_to_expr (*t, NULL, NULL));
        return 0;
    }

    if (t->type == TOKEN_RIGHT_PAREN) {
        if (is_empty (ops))
            return -1;

        while (!is_empty (ops) && peek (ops)->type != EXPR_PAREN)
            if (merge (exprs, ops) < 0)
                return -1;

        /* We need to check if there's a left parentheses at the top of the
         * stack. */
        if (is_empty (ops) || peek (ops)->type != EXPR_PAREN)
            return -1;

        destroy_expr (pop (ops));
        return 0;
    }

    /* Above cases should be exhaustive and `TOKEN_NULL` should not be supplied
     * to `analyze`. */
    assert (false);
    return -1;
}

/* Exhausts remaining operators in `ops`, writing result to `exprs`. Returns 0
 * if successful, -1 otherwise. */
static int exhaust (struct expr_stack *exprs, struct expr_stack *ops) {
    while (!is_empty (ops))
        if (merge (exprs, ops) < 0)
            return -1;

    return 0;
}

/* Pops two element from `exprs`, one element from `ops`, merges them into
 * a expression, and pushes it back to `exprs`. */
static int merge (struct expr_stack *exprs, struct expr_stack *ops) {
    struct expr *op, *left, *right;

    if (is_empty (ops) || exprs->top < 2)
        return -1;

    op = pop (ops);
    right = pop (exprs);
    left = pop (exprs);

    op->left = left;
    op->right = right;

    push (exprs, op);
    return 0;
}

/* Constructs empty expression stack, and returns it. */
static struct expr_stack make_stack (void) {
    struct expr_stack e = {
        .arr = malloc (sizeof (struct expr *) * MAX_TOKEN_NUM), .top = 0};

    assert_nonnull (e.arr, "ERROR: out of memory\n");
    return e;
}

/* Destroys stack `st`. Note that it does not free expressions in `st`. */
static void destroy_stack (struct expr_stack st) { free (st.arr); }

/* Pushes `e` into `st`. */
static void push (struct expr_stack *st, struct expr *e) {
    st->arr[st->top++] = e;
}

/* Pops one element from `st`, and returns it. `st` must not be empty. */
static struct expr *pop (struct expr_stack *st) {
    assert (st->top > 0);
    return st->arr[--st->top];
}

/* Retuns the top element of `st`, but does not pop it. `st` must not be
 * empty. */
static struct expr *peek (const struct expr_stack *st) {
    assert (st->top > 0);
    return st->arr[st->top - 1];
}

/* Converts `t` into expression that has left branch of `left` and right branch
 * of `right`, and returns it. `t` must not be a null token and it initializes
 * `number` member of returned expression to zero when it is not an expression
 * of EXPR_NUMBER type. Also, right parentheses are converted into expression of
 * EXPR_PAREN type which is just a dummy type for lexing algorithm. */
static struct expr *token_to_expr (struct token t, struct expr *left,
                                   struct expr *right) {
    assert (t.type != TOKEN_NULL);

    switch (t.type) {
    case TOKEN_NUMBER:
        return make_expr (EXPR_NUMBER, t.number, left, right);

    case TOKEN_PLUS:
        return make_expr (EXPR_PLUS, 0, left, right);

    case TOKEN_MINUS:
        return make_expr (EXPR_MINUS, 0, left, right);

    case TOKEN_MULTIPLY:
        return make_expr (EXPR_MULTIPLY, 0, left, right);

    case TOKEN_DIVIDE:
        return make_expr (EXPR_DIVIDE, 0, left, right);

    case TOKEN_RIGHT_PAREN:
    case TOKEN_LEFT_PAREN:
        return make_expr (EXPR_PAREN, 0, left, right);

    default:
        /* This switch statement should be exhaustive, and `TOKEN_NULL` must not
         * be passed to this function. */
        assert (false);
    }
}

/* Compares token type `t` and expression type `e`, returns true if precedence
 * of `t` is less than that of `e`, or precedence of `t` is equal to that of
 * `e` and `t` is left-associative. */
static bool compare_precedence (enum token_type t, enum expr_type e) {
    if (TOKEN_PRECEDENCES[t] < EXPR_PRECEDENCES[e])
        return true;

    if (TOKEN_PRECEDENCES[t] == EXPR_PRECEDENCES[e] &&
        TOKEN_ASSOCIATIVITY[t] == ASSOC_LEFT)
        return true;

    return false;
}

/* Returns true if `st` is empty. */
static bool is_empty (struct expr_stack *st) { return st->top == 0; }
