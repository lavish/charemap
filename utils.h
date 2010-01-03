/*
 * Description: utils.h, header file for utils.c
 */

#include <glib.h>

#define	N	256

/* structs */
typedef struct {
        int bigram[2];
        int occ;
} Bigram;

typedef struct {
        int trigram[3];
        int occ;
} Trigram;

typedef struct {
        char word[N];
        int occ;
} Word;

/* function declarations */
void bubble_up(GList *a, GList *b, GList *x, GList *c);
void print_bigrams(GList *l);
void print_trigrams(GList *l);
void print_words(GList *l);
void free_list(GList *l);
GList *count_bigrams(FILE *fi, GList *l, int case_sensitive, int alpha_only);
GList *count_trigrams(FILE *fi, GList *l, int case_sensitive, int alpha_only);
GList *count_words(FILE *fi, GList *l, int case_sensitive);
