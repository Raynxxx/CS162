#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "parse.h"

#define TOKseparator " \n:"

/**
 * Tokenizes the line of characters.
 * Returns a new array of pointers to token locations within the line.
 * Please note that token separators in the line are replaced with NULL,
 * so the line is therefore mutated.
 */
tok_t *get_toks(char *line) {
  int i;
  char *c;

  tok_t *toks = malloc(MAXTOKS * sizeof(tok_t));

  /** Intializes an empty token array */
  for (i = 0; i < MAXTOKS; i++) toks[i] = NULL;

  /* Start tokenizer on line */
  c = strtok(line, TOKseparator);
  for (i = 0; c && i < MAXTOKS; i++) {
    toks[i] = c;
    /* scan for next token */
    c = strtok(NULL, TOKseparator);
  }
  return toks;
}

/**
 * Frees the memory of the token array toks.
 * Please call if and only if toks will not be used in the future.
 */
void free_toks(tok_t *toks) {
  free(toks);
}

/**
 * Prints the token stream t to the output file ofile.
 */
void fprint_tok(FILE *ofile, tok_t *t) {
  for (int i = 0; i < MAXTOKS && t[i]; i++) {
    fprintf(ofile, "%s ", t[i]);
  }
  fprintf(ofile, "\n");
}

/**
 * Locates the character R in the token array t.
 * Please note that R must be of length 1.
 * Returns the token that is an exact match of R.
 */
int is_direct_tok(tok_t *t, char *R) {
  for (int i = 0; i < MAXTOKS - 1 && t[i]; i++) {
    if (strncmp(t[i], R, 1) == 0)
      return i;
  }
  return 0;
}
