/*
 * Author:      Marco Squarcina <lavish@gmail.com>
 * Date:        03/01/2010
 * Version:     0.5
 * License:     MIT, see LICENSE for details
 * Description: utils.c, a collection of useful functions for charemap.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

/* function implementations */
void
bubble_up(GList *a, GList *b, GList *x, GList *c) {
        /* a <--> b <--> x <--> c */
        a = x->prev->prev;
        b = x->prev;
        c = x->next;

        /* a <--> b -> c */
        b->next = c;
        if(c != NULL)
                /* a <--> b <--> c */
                c->prev = b;
        if(a != NULL)
                /* a <- b <--> c */
                a->next = x;
        /* a -> x <- b <--> c */
        b->prev = x;
        /* a <--> x <- b <--> c */
        x->prev = a;
        /* a <--> x <--> b <--> c */
        x->next = b;
}

GList *
count_bigrams(FILE *fi, GList *l, int case_sensitive, int alpha_only) {
        int a0, a1;
        Bigram *tmp = NULL;
        GList *iter = NULL;

        rewind(fi);
        a0 = fgetc(fi);
        a1 = fgetc(fi);
        while(1) {
                while(a1 != EOF && alpha_only && (!isalpha(a0) || !isalpha(a1))) {
                        a0 = a1;
                        a1 = fgetc(fi);
                }
                if(a1 == EOF)
                        break;
                if(!case_sensitive) {
                        a0 = tolower(a0);
                        a1 = tolower(a1);
                }
                iter = g_list_first(l);
                while(iter != NULL) {
                        if(((Bigram *)iter->data)->bigram[0] == a0 &&
                           ((Bigram *)iter->data)->bigram[1] == a1) {
                                /* bigram already in list */
                                ((Bigram *)iter->data)->occ += 1;
                                while(iter->prev != NULL && ((Bigram *)iter->prev->data)->occ < ((Bigram *)iter->data)->occ)
                                        bubble_up(iter->prev->prev, iter->prev, iter, iter->next);
                                break;
                        }
                        iter = iter->next;
                }
                if(iter == NULL) {
                        /* this is a new bigram */
                        tmp = malloc(sizeof(Bigram));
                        tmp->bigram[0] = a0;
                        tmp->bigram[1] = a1;
                        tmp->occ = 1;
                        l = g_list_append(l, tmp);
                }
                /* go on */
                a0 = a1;
                a1 = fgetc(fi);
        }

        return l;
}

GList *
count_trigrams(FILE *fi, GList *l, int case_sensitive, int alpha_only) {
        int a0, a1, a2;
        Trigram *tmp = NULL;
        GList *iter = NULL;

        rewind(fi);
        a0 = fgetc(fi);
        a1 = fgetc(fi);
        a2 = fgetc(fi);
        while(1) {
                while(a2 != EOF && alpha_only && (!isalpha(a0) || !isalpha(a1) || !isalpha(a2))) {
                        a0 = a1;
                        a1 = a2;
                        a2 = fgetc(fi);
                }
                if(a2 == EOF)
                        break;
                if(!case_sensitive) {
                        a0 = tolower(a0);
                        a1 = tolower(a1);
                        a2 = tolower(a2);
                }
                iter = g_list_first(l);
                while(iter != NULL) {
                        if(((Trigram *)iter->data)->trigram[0] == a0 &&
                           ((Trigram *)iter->data)->trigram[1] == a1 &&
                           ((Trigram *)iter->data)->trigram[2] == a2) {
                                /* trigram already in list */
                                ((Trigram *)iter->data)->occ += 1;
                                while(iter->prev != NULL && ((Trigram *)iter->prev->data)->occ < ((Trigram *)iter->data)->occ)
                                        bubble_up(iter->prev->prev, iter->prev, iter, iter->next);
                                break;
                        }
                        iter = iter->next;
                }
                if(iter == NULL) {
                        /* this is a new trigram */
                        tmp = malloc(sizeof(Trigram));
                        tmp->trigram[0] = a0;
                        tmp->trigram[1] = a1;
                        tmp->trigram[2] = a2;
                        tmp->occ = 1;
                        l = g_list_append(l, tmp);
                }
                /* go on */
                a0 = a1;
                a1 = a2;
                a2 = fgetc(fi);
        }

        return l;
}

GList *
count_words(FILE *fi, GList *l, int case_sensitive) {
        char buf[N], c;
        Word *tmp = NULL;
        GList *iter = NULL;
        int i;

        rewind(fi);
        while(1) {
                i = 0;
                c = fgetc(fi);
                if(c == EOF)
                        break;
                while(isalpha(c)) {
                        if(i > N-2)
                                break;
                        if(!case_sensitive)
                                c = tolower(c);
                        buf[i++] = c;
                        c = fgetc(fi);
                }
                if(i) {
                        buf[i] = '\0';
                        iter = g_list_first(l);

                        while(iter != NULL) {
                                if(strcmp(((Word *)iter->data)->word, buf) == 0) {
                                        /* word already in l */
                                        ((Word *)iter->data)->occ += 1;
                                        while(iter->prev != NULL && ((Word *)iter->prev->data)->occ < ((Word *)iter->data)->occ)
                                                bubble_up(iter->prev->prev, iter->prev, iter, iter->next);
                                        break;
                                }
                                iter = iter->next;
                        }
                        if(iter == NULL) {
                                /* this is a new word */
                                tmp = malloc(sizeof(Word));
                                strcpy(tmp->word, buf);
                                tmp->occ = 1;
                                l = g_list_append(l, tmp);
                        }
                }
        }

        return l;
}

void
print_bigrams(GList *l) {
        GList *iter = g_list_first(l);

        while(iter != NULL) {
                printf("%8d : %c%c\n",
                        ((Bigram *)iter->data)->occ,
                        ((Bigram *)iter->data)->bigram[0],
                        ((Bigram *)iter->data)->bigram[1]);
                iter = iter->next;
        }
}

void
print_trigrams(GList *l) {
        GList *iter = g_list_first(l);

        while(iter != NULL) {
                printf("%8d : %c%c%c\n",
                        ((Trigram *)iter->data)->occ,
                        ((Trigram *)iter->data)->trigram[0],
                        ((Trigram *)iter->data)->trigram[1],
                        ((Trigram *)iter->data)->trigram[2]);
                iter = iter->next;
        }
}

void
print_words(GList *l) {
        GList *iter = g_list_first(l);

        while(iter != NULL) {
                printf("%8d : ", ((Word *)iter->data)->occ);
                printf("%s\n", ((Word *)iter->data)->word);
                iter = iter->next;
        }
}

void
free_list(GList *l) {
        GList *iter = g_list_first(l);

        while(iter != NULL) {
                free(iter->data);
                iter = iter->next;
        }
        g_list_free(l);
}
