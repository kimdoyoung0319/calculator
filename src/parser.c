#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include "parser.h"

/* Maximum number of tokens. */
const int MAX_TOKEN_NUM = 100;

/* Precedences of tokens. This must be updated when updating enum token_type. */
const int TOKEN_PRECEDENCES[] = {0, 0, 1, 1, 2, 2, 0, 0};

/* Associativity of tokens. ASSOC_NONE means that the token is not an operator,
   hence there's no associativity. */
const enum associativity TOKEN_ASSOCIATIVITY[] = {
  ASSOC_NONE,
  ASSOC_NONE,
  ASSOC_LEFT,
  ASSOC_LEFT,
  ASSOC_LEFT,
  ASSOC_LEFT,
  ASSOC_NONE,
  ASSOC_NONE,
};

/* Array and its size of special characters used in the calculator. */
const int SPECIAL_CHARS_NUM = 6;
const char SPECIAL_CHARS[SPECIAL_CHARS_NUM] = {'+', '-', '*', '/', '(', ')'};

enum parse_state {
  STATE_NUMBER,
  STATE_NORMAL,
};

static int consume (struct token *, char, int);
static bool is_special_char (char);
static bool is_digit (char);
static struct token char_to_token (char);
static struct token int_to_token (int);
static int append_digit (char, int);

/* Parses 'str' whose length is 'len' into tokens, and returns the array of 
   parsed tokens. */
struct token *parser (const char *str, int len) {
  int i, pos;
  struct token null_token = {.type = TOKEN_NULL};
  struct token *parsed = (struct token *) calloc (MAX_TOKEN_NUM, 
                                                  sizeof (struct token));

  for (i = 0, pos = 0; i < len && str[i] != '\0'; i++) 
    pos = consume (parsed, str[i], pos);

  return parsed;
}

/* Returns true if 't' is null token. */
bool is_token_null (struct token *t) {
  return t->type == TOKEN_NULL;
}

/* Returns true if 't' is an operator token, such as +, -, *, /. */
bool is_operator_token (struct token *t) {
  return t->type == TOKEN_PLUS || t->type == TOKEN_MINUS 
         || t->type == TOKEN_MULTIPLY || t->type == TOKEN_DIVIDE;
}

/* Consumes a character 'ch', writes the result to 'r' at 'pos', and returns
   the next position to be written. Has inner state 'st', which may be modified 
   at every call of this function. Does nothing but returns original 'pos' if 
   'ch' is not acceptable character. */
static int consume (struct token *r, char ch, int pos) {
  static enum parse_state st = STATE_NORMAL;

  if (st == STATE_NORMAL && is_special_char (ch)) {
    r[pos++] = char_to_token (ch);
    return pos;
  }

  if (st == STATE_NORMAL && is_digit (ch)) {
    r[pos] = int_to_token (ch - '0');
    st = STATE_NUMBER;
    return pos;
  }

  if (st == STATE_NUMBER && is_special_char (ch)) {
    r[++pos] = char_to_token (ch);
    st = STATE_NORMAL;
    return ++pos;
  }

  if (st == STATE_NUMBER && is_digit (ch)) {
    r[pos].number = append_digit (ch, r[pos].number);
    return pos;
  }

  return pos;
}

/* Returns true if 'ch' is a special character specified at the top of this 
   file. */
static bool is_special_char (char ch) {
  for (int i = 0; i < SPECIAL_CHARS_NUM; i++)
    if (SPECIAL_CHARS[i] == ch)
      return true;

  return false;
}

/* Returns true if 'ch' is a digit of ASCII character. */
static bool is_digit (char ch) {
  return '0' <= ch && ch <= '9';
}

/* Converts single character 'ch' to token. It is NOT safe to pass any 
   arbitrary character to argument since it will return uninitialized struct
   when 'ch' is not a valid convertible character. */
static struct token char_to_token (char ch) {
  struct token t;

  assert (is_special_char (ch));

  switch (ch) {
    case '+': t.type = TOKEN_PLUS;
              break;
    case '-': t.type = TOKEN_MINUS;
              break;
    case '*': t.type = TOKEN_MULTIPLY;
              break;
    case '/': t.type = TOKEN_DIVIDE;
              break;
    case '(': t.type = TOKEN_LEFT_PAREN;
              break;
    case ')': t.type = TOKEN_RIGHT_PAREN;
              break;
    default: ;
  }

  return t;
}

/* Converts integer 'n' into token. */
static struct token int_to_token (int n) {
  struct token t = {.type = TOKEN_NUMBER, .number = n};
  return t;
}

/* Appends digit represented by 'ch' into 'n', and returns the appended 
   number. i.e. append_digit ('6', 34) will return 346. */
static int append_digit (char ch, int n) {
  return 10 * n + (ch - '0');
}