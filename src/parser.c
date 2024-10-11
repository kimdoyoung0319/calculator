#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include "parser.h"

/* Maximum number of tokens. */
const int MAX_TOKEN_NUM = 100;

/* Precedences of tokens. */
const int TOKEN_PRECEDENCES[] = {0, 0, 1, 0, 0};

/* Associativity of tokens. ASSOC_NONE means that the token is not an operator,
   hence there's no associativity. */
const enum associativity TOKEN_ASSOCIATIVITY[] = {
  ASSOC_NONE,
  ASSOC_NONE,
  ASSOC_LEFT,
  ASSOC_NONE,
  ASSOC_NONE,
};

/* Array and its size of special characters used in the calculator. */
const int SPECIAL_CHARS_NUM = 3;
const char SPECIAL_CHARS[SPECIAL_CHARS_NUM] = {'+', '(', ')'};

enum parse_state {
  STATE_NUMBER,
  STATE_NORMAL,
};

static int consume (struct token *, char, int);
static bool is_special_char (char);
static bool is_digit (char);
static struct token char_to_token (char);
static struct token int_to_token (int);

/* Parses 'str' whose length is 'len' into tokens, and returns the array of 
   parsed tokens. */
struct token *parser (const char *str, int len) {
  int i, pos;
  struct token null_token = {.type = TOKEN_NULL};
  struct token *parsed = (struct token *) malloc (sizeof (struct token) 
                                                  * MAX_TOKEN_NUM);

  for (i = 0, pos = 0; i < len && str[i] != '\0'; i++) 
    pos = consume (parsed, str[i], pos);

  parsed[pos] = null_token;
  return parsed;
}

/* Returns true if 't' is null token. */
bool is_token_null (struct token t) {
  return t.type == TOKEN_NULL;
}

/* Consumes a character 'ch', writes the result to 'r' at 'pos', and returns
   the next position to be written. Has inner state 'st' and static variable 
   'num', which may be modified at every call of this function. Does nothing 
   but returns original 'pos' if 'ch' is not acceptable character. */
static int consume (struct token *r, char ch, int pos) {
  static int num;
  static enum parse_state st = STATE_NORMAL;

  if (st == STATE_NORMAL && is_special_char (ch)) {
    r[pos++] = char_to_token (ch);
    return pos;
  }

  if (st == STATE_NORMAL && is_digit (ch)) {
    num = ch - '0';
    st = STATE_NUMBER;
    return pos;
  }

  if (st == STATE_NUMBER && is_special_char (ch)) {
    r[pos++] = int_to_token (num);
    r[pos++] = char_to_token (ch);
    st = STATE_NORMAL;
    return pos;
  }

  if (st == STATE_NUMBER && is_digit (ch)) {
    num = num * 10 + (ch - '0');
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
   when 'ch' is not a valid convertible token. */
static struct token char_to_token (char ch) {
  struct token t;

  assert (is_special_char (ch));

  switch (ch) {
    case '+': t.type = TOKEN_PLUS;
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