/*
 * Description: decrypt.h, header file for decrypt.c
 */

#include "glib.h"

#define KEYSIZE 26
#define OFFSET  97

/* function declarations */
void guess_key(FILE *f, char k[KEYSIZE]);
void swap_in_key(char *k, int a, int b);
void copy_key(char key1[KEYSIZE], char key2[KEYSIZE]);
void print_key(char k[KEYSIZE]);
void populate_bigram_matrix(FILE *f, float m[KEYSIZE][KEYSIZE]);
void populate_trigram_matrix(FILE *f, float m[KEYSIZE][KEYSIZE][KEYSIZE]);
void swap_in_bigram_matrix(float m[KEYSIZE][KEYSIZE], int a, int b);
void swap_in_trigram_matrix(float m[KEYSIZE][KEYSIZE][KEYSIZE], int a, int b);
void copy_bigram_matrix(float m1[KEYSIZE][KEYSIZE], float m2[KEYSIZE][KEYSIZE]);
void copy_trigram_matrix(float m1[KEYSIZE][KEYSIZE][KEYSIZE], float m2[KEYSIZE][KEYSIZE][KEYSIZE]);
void echo_file(FILE *f);
void decrypt(FILE *fi, FILE *fs);
float bigram_goodness(float m1[KEYSIZE][KEYSIZE], float m2[KEYSIZE][KEYSIZE]);
float trigram_goodness(float m1[KEYSIZE][KEYSIZE][KEYSIZE], float m2[KEYSIZE][KEYSIZE][KEYSIZE]);
int word_goodness(GList *l1, GList *l2);
char *decrypt_to_file(FILE *fi, char *k1, char *k2);
