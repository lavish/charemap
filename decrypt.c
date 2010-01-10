/*
 * Author:      Marco Squarcina <lavish@gmail.com>
 * Date:        10/01/2010
 * Version:     0.5
 * License:     MIT, see LICENSE for details
 * Description: decrypt.c, implementation of an improved jackobsen algorithm
 * 		to break substitution ciphers.
 */

#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "decrypt.h"
#include "utils.h"

/* function implementations */
void
guess_key(FILE *f, char k[KEYSIZE]) {
	int occ[KEYSIZE], i, j, c, max, tmp = 0;

	/* reset occurrences vector values to 0 */
	for(i=0; i<KEYSIZE; i++)
		occ[i] = 0;
	/* count characters occurrences */
	i = 0;
	rewind(f);
	c = fgetc(f);
	while(1) {
		while(c != EOF && !isalpha(c))
			c = fgetc(f);
		if(c == EOF)
			break;
		c = tolower(c);
		occ[c-OFFSET] += 1;
		c = fgetc(f);
	}
	for(i=0; i<KEYSIZE; i++) {
		max = -1;
		/* find out most frequent char */
		for(j=0; j<KEYSIZE; j++) 
			if(occ[j] >= max) {
				max = occ[j];
				tmp = j;
			}
		occ[tmp] = -1;
		k[i] = tmp+OFFSET;
	}
}

void
print_key(char k[KEYSIZE]) {
	int i;

	for(i=0; i<KEYSIZE; i++)
		printf("%c", k[i]);
	printf("\n");
}

void
populate_bigram_matrix(FILE *f, float m[KEYSIZE][KEYSIZE]) {
	int c0, c1, i, j, n;

	/* reset matrix values to 0 */
	for(i = 0; i<KEYSIZE; i++)
		for(j=0; j<KEYSIZE; j++)
			m[i][j] = 0;
	/* count bigrams occurrences */
	i = 0;
	rewind(f);
	c0 = fgetc(f);
	c1 = fgetc(f);
	while(1) {
		/* skip until a valid bigram is found */
		while(c1 != EOF && (!isalpha(c0) || !isalpha(c1))) {
			c0 = c1;
			c1 = fgetc(f);
		}
		if(c1 == EOF)
			break;
		c0 = tolower(c0);
		c1 = tolower(c1);
		m[c0-OFFSET][c1-OFFSET] += 1;
		i++;
		c0 = c1;
		c1 = fgetc(f);
	}
	/* get bigram number */
	n = i;
	/* generate statistics */
	for(i=0; i<KEYSIZE; i++)
		for(j=0; j<KEYSIZE; j++)
			m[i][j] = m[i][j]/n;
}

void
populate_trigram_matrix(FILE *f, float m[KEYSIZE][KEYSIZE][KEYSIZE]) {
	int c0, c1, c2, i, j, k, n;

	/* reset matrix values to 0 */
	for(i = 0; i<KEYSIZE; i++)
		for(j=0; j<KEYSIZE; j++)
			for(k=0; k<KEYSIZE; k++)
				m[i][j][k] = 0;
	/* count trigrams occurrences */
	i = 0;
	rewind(f);
	c0 = fgetc(f);
	c1 = fgetc(f);
	c2 = fgetc(f);
	while(1) {
		/* skip until a valid trigram is found */
		while(c2 != EOF && (!isalpha(c0) || !isalpha(c1) || !isalpha(c2))) {
			c0 = c1;
			c1 = c2;
			c2 = fgetc(f);
		}
		if(c2 == EOF)
			break;
		c0 = tolower(c0);
		c1 = tolower(c1);
		c2 = tolower(c2);
		m[c0-OFFSET][c1-OFFSET][c2-OFFSET] += 1;
		i++;
		c0 = c1;
		c1 = c2;
		c2 = fgetc(f);
	}
	/* get trigram number */
	n = i;
	/* generate statistics */
	for(i=0; i<KEYSIZE; i++)
		for(j=0; j<KEYSIZE; j++)
			for(k=0; k<KEYSIZE; k++)
				m[i][j][k] = m[i][j][k]/n;
}

void
swap_in_bigram_matrix(float m[KEYSIZE][KEYSIZE], int a, int b) {
	float t;
	int i;

	/* swap columns */
	for(i=0; i<KEYSIZE; i++) {
		t = m[i][a];
		m[i][a] = m[i][b];
		m[i][b] = t;
	}
	/* swap rows */
	for(i=0; i<KEYSIZE; i++) {
		t = m[a][i];
		m[a][i] = m[b][i];
		m[b][i] = t;
	}

}

void
swap_in_trigram_matrix(float m[KEYSIZE][KEYSIZE][KEYSIZE], int a, int b) {
	float t;
	int i;

	for(i=0; i<KEYSIZE; i++) {
		t = m[i][a][b];
		m[i][a][b] = m[i][b][a];
		m[i][b][a] = t;
	}
	for(i=0; i<KEYSIZE; i++) {
		t = m[a][i][b];
		m[a][i][b] = m[b][i][a];
		m[b][i][a] = t;
	}
        for(i=0; i<KEYSIZE; i++) {
		t = m[a][b][i];
		m[a][b][i] = m[b][a][i];
		m[b][a][i] = t;
	}
}

void
copy_bigram_matrix(float m1[KEYSIZE][KEYSIZE], float m2[KEYSIZE][KEYSIZE]) {
	int i, j;

	for(i=0; i<KEYSIZE; i++)
		for(j=0; j<KEYSIZE; j++)
			m1[i][j] = m2[i][j];
}

void
copy_trigram_matrix(float m1[KEYSIZE][KEYSIZE][KEYSIZE], float m2[KEYSIZE][KEYSIZE][KEYSIZE]) {
	int i, j, k;

	for(i=0; i<KEYSIZE; i++)
		for(j=0; j<KEYSIZE; j++)
			for(k=0; k<KEYSIZE; k++)
				m1[i][j][k] = m2[i][j][k];
}

float
bigram_goodness(float m1[KEYSIZE][KEYSIZE], float m2[KEYSIZE][KEYSIZE]) {
	int i, j;
	float t = 0;

	for(i=0; i<KEYSIZE; i++)
		for(j=0; j<KEYSIZE; j++)
			t += fabsf(m1[i][j] - m2[i][j]);

	return t;
}

float
trigram_goodness(float m1[KEYSIZE][KEYSIZE][KEYSIZE], float m2[KEYSIZE][KEYSIZE][KEYSIZE]) {
	int i, j, k;
	float t = 0;

	for(i=0; i<KEYSIZE; i++)
		for(j=0; j<KEYSIZE; j++) 
			for(k=0; k<KEYSIZE; k++) 
				t += fabsf(m1[i][j][k] - m2[i][j][k]);
	return t;
}

int
word_goodness(GList *l1, GList *l2) {
        GList *iter1;
        GList *iter2;
        int t = 0;

        iter1 = g_list_first(l1);
        while(iter1 != NULL) {
                iter2 = g_list_first(l2);
                while(iter2 != NULL && ((Word *)iter2->data)->occ > 1) {
                        if(strcmp(((Word *)iter1->data)->word, ((Word *)iter2->data)->word) == 0) {
				t += 1;
                                break;
                        }
                        iter2 = iter2->next;
                }
                iter1 = iter1->next;
        }

        return t;
}

char *
decrypt_to_file(FILE *f, char *k1, char *k2) {
	int c, i;
	char *outfle = "/tmp/.charemap.txt";
	FILE *fo;

	fo = fopen(outfle, "w");
	rewind(f);
	c = fgetc(f);
	while(c != EOF) {
		if(!isalpha(c)) 
			putc(c, fo);
		c = tolower(c);
		for(i=0; i<KEYSIZE; i++)
			if(k1[i] == c) {
				putc(k2[i], fo);
				break;
			}
		c = fgetc(f);
	}
	fclose(fo);

	return outfle;
}

void
swap_in_key(char *k, int a, int b) {
	char c;

	c = k[a];
	k[a] = k[b];
	k[b] = c;
}

void
copy_key(char key1[KEYSIZE], char key2[KEYSIZE]) {
	int i;

	for(i=0; i<KEYSIZE; i++)
		key1[i] = key2[i];
}

void
echo_file(FILE *f) {
	int c;

	rewind(f);
	while((c = fgetc(f)) != EOF) 
		putchar(c);
}

void
decrypt(FILE *fi, FILE *fs) {
        FILE *fptr;
        char *fname;
	float v = 0, v1 = 0, vt = 0, vt1 = 0;
	int a = 0, b = 1, i = 0;

	float Db[KEYSIZE][KEYSIZE];
	float Eb[KEYSIZE][KEYSIZE];
	float Db1[KEYSIZE][KEYSIZE];
	float Dt[KEYSIZE][KEYSIZE][KEYSIZE];
	float Et[KEYSIZE][KEYSIZE][KEYSIZE];
	float Dt1[KEYSIZE][KEYSIZE][KEYSIZE];
	char ks[KEYSIZE];
	char key[KEYSIZE];
	char k1[KEYSIZE];

	GList *input_wlist = NULL;
	GList *input_slist = NULL;
	char loader[] = "|/-\\|";
	int case_sensitive = 0; /* no case sensitive */

	guess_key(fs, ks);
	guess_key(fi, key);

	fname = decrypt_to_file(fi, ks, key);
	fptr = fopen(fname, "r");
	populate_bigram_matrix(fptr, Db);
	populate_trigram_matrix(fptr, Dt);
	fclose(fptr);

	populate_bigram_matrix(fs, Eb);
	populate_trigram_matrix(fs, Et);
	
	v = bigram_goodness(Db, Eb);
	vt = trigram_goodness(Dt, Et);
	copy_bigram_matrix(Db1, Db);
	copy_trigram_matrix(Dt1, Dt);
	copy_key(k1, key);
        printf("Decripting using bigram and trigram detection...\n");
	while(1) {
                printf("\r%c", loader[i++ % 5]);

		swap_in_key(k1, a, a+b);
		swap_in_bigram_matrix(Db1, k1[a]-OFFSET, k1[a+b]-OFFSET);
		swap_in_trigram_matrix(Dt1, k1[a]-OFFSET, k1[a+b]-OFFSET);

		a = a+1;
		if(a+b > KEYSIZE-1) {
			a = 0;
			b = b+1;
			if(b == KEYSIZE-1) {
				printf("\rdone!\n\nThe mapping found is:\n\n\t<- ");
				print_key(ks);
				printf("\t   ||||||||||||||||||||||||||\n");
				printf("\t-> ");
				print_key(key);
				printf("\nDecryption result:\n\n");
				fname = decrypt_to_file(fi, ks, key);
				fptr = fopen(fname, "r");
				echo_file(fptr);
				fclose(fptr);
				remove(fname);
				putchar('\n');
				break;
			}
		}
		
		v1 = bigram_goodness(Db1, Eb);
		vt1 = trigram_goodness(Dt1, Et);
		if(v1+vt1 < v+vt) {
			a = 0;
			b = 1;
			v = v1;
			vt = vt1;
			copy_key(key, k1);
			copy_bigram_matrix(Db, Db1);
			copy_trigram_matrix(Dt, Dt1);
		}
		else {
			copy_key(k1, key);
			copy_bigram_matrix(Db1, Db);
			copy_trigram_matrix(Dt1, Dt);
		}
	}
	fname = decrypt_to_file(fi, ks, key);
	fptr = fopen(fname, "r");
	input_wlist = count_words(fptr, input_wlist, case_sensitive);
	fclose(fptr);
	input_slist = count_words(fs, input_slist, case_sensitive);

	int w = 0, w1 = 0;

	w = word_goodness(input_wlist, input_slist);
	free_list(input_wlist);
	input_wlist = NULL;

	copy_key(k1, key);
	
	a = 0;
	b = 1;

	printf("Affining result with dictionary-based decryption...\n");
	while(1) {
		printf("\r%c", loader[i++ % 5]);

                swap_in_key(k1, a, a+b);

                a = a+1;
                if(a+b > KEYSIZE-1) {
                        a = 0;
                        b = b+1;
                        if(b == KEYSIZE-1) {
                                printf("\rdone!\n\nThe mapping found is:\n\n\t<- ");
                                print_key(ks);
                                printf("\t   ||||||||||||||||||||||||||\n");
                                printf("\t-> ");
                                print_key(key);
                                printf("\nDecryption result:\n\n");
                                decrypt_to_file(fi, ks, key);
                                fptr = fopen(fname, "r");
                                echo_file(fptr);
                                fclose(fptr);
                                putchar('\n');
                                break;
                        }
                }

		fname = decrypt_to_file(fi, ks, k1);
		fptr = fopen(fname, "r");
		input_wlist = count_words(fptr, input_wlist, case_sensitive);
		fclose(fptr);
		
		w1 = word_goodness(input_wlist, input_slist);
		free_list(input_wlist);
		input_wlist = NULL;

                if(w1 > w) {
                        a = 0;
                        b = 1;
                        w = w1;
                        copy_key(key, k1);
                }
                else {
                        copy_key(k1, key);
                }
	}

	remove(fname);
	free_list(input_wlist);
	free_list(input_slist);
}
