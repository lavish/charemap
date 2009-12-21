/*
 * Author:	Marco Squarcina <lavish@gmail.com>
 * Date:	20/12/2009
 * Version:	0.3
 * License:	MIT, see LICENSE for details
 * Description:	Tiny program to play with substitution ciphers (works also with
 * 		non-standard characters).
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/queue.h>
#include <glib.h>

#define N	256

/* structs */
typedef struct {
	unsigned char orig;
	int occ;
	unsigned char new;
} Relation;

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
static void die(const char *error);
static void usage(void);
static char substitute(char c);
static void sort_by_occ(void);
static int load_lang(char *l, char *map);
static int initialize(FILE *fi);
static void associate(void);
static void print_char_occ(void);
static void print_file(FILE *fi);
static void print_video(FILE *fi);
static void bubble_up(GList *a, GList *b, GList *x, GList *c);
static void count_bigrams(FILE *fi);
static void count_trigrams(FILE *fi);
static void count_words(FILE *fi);
static void print_bigrams(void);
static void print_trigrams(void);
static void print_words(void);
static void free_list(GList *l);

/* variables */
static int show_occ = 0;
static int show_bigrams = 0;
static int show_trigrams = 0;
static int show_words = 0;
static int case_sensitive = 0;
static int alpha_only = 0;
static int print_substituted = 0;
static char in[N];
static char out[N];
static char lang[N];
static Relation r[N];
static char map[N];
static int rl, mapl;
static GList *bigram_list = NULL;
static GList *trigram_list = NULL;
static GList *word_list = NULL;

/* function implementations */
void
die(const char *errstr) {
        fprintf(stderr, "%s\n", errstr);
        exit(EXIT_FAILURE);
}

void
usage() {
	printf("Usage: charemap [options]...\nOptions:\n");
	printf("  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n",
		"-h",		"This help",
		"-v",		"Print version",
		"-s",		"Show only character occurrences and mapping",
		"-c",		"Case sensitive",
		"-a",		"Remap alpha characters only (preserves dots, blanks and so on...)",
		"-p",		"Print substituted text",
		"-b",		"Show bigrams",
		"-t",		"Show trigrams",
		"-w",		"Show words",
		"-i <file>",	"Input file to parse",
		"-o <file>",	"Output file with remapped characters",
		"-l <language>","Try to decrypt using the selected language (default: en)");
	exit(EXIT_FAILURE);
}

char
substitute(char c) {
        int i;

        if(!case_sensitive)
                c = tolower(c);
	for(i=0; i<rl; i++)
        	if(r[i].orig == c)
                	return r[i].new;
        return '?';
}

void
sort_by_occ() {
	Relation tmp;
	int i, j;

	for (i = 1; i < rl; i++)
		for ( j = 0; j < rl - 1; j++ ) 
			if(r[j].occ < r[j+1].occ ) {
				tmp = r[j];
				r[j] = r[j+1];
				r[j+1] = tmp;
			}
}

int
load_lang(char *l, char *map) {
	FILE *f;
	int c, i = 0;
	char fname[N];

	strcpy(fname, "languages/");
	strcat(fname, l);
	strcat(fname, ".txt");
	if((f = fopen(fname, "r")) == NULL) 
		die("Language file not found.");
	c = fgetc(f);
	while(c != EOF) {
		map[i++] = c;
		c = fgetc(f);
	}
	fclose(f);

	return i-1;
}

int
initialize(FILE *fi) {
	int c, i, j;

	/* reset the relation vector */
	for(i = 0; i < N; i++) {
		r[i].occ = 0;
		r[i].new = '?';
	}
	c = fgetc(fi);
	/* by default everything to lowercase */
	if(!case_sensitive)
		c = tolower(c);
	/* count occurrences */
	for(i=0; c != EOF;) {
		for(j = 0; j < i; j++) {
			if(r[j].orig == c) {
				/* char already in r */
				r[j].occ += 1;
				break;
			}
		}
		if(j >= i) {
			/* this is a new entry */
			r[i].orig = c;
			r[i].occ = 1;
			i++;
		}
		c = fgetc(fi);
		if(!case_sensitive)
			c = tolower(c);
	}
	/* return r length */
	return i;
}

void
associate() {
	int i, j;

	for(i=0, j=0; i<rl;) {
		if(alpha_only && !isalpha(r[i].orig)) {
			r[i].new = r[i].orig;
                        i++;
		}
                else if(j < mapl)
                        r[i++].new = map[j++];
		else
			r[i++].new = '?';
	}
}

void
print_char_occ() {
	int i;

	printf("%15s | %15s | %15s |\n%s\n",
		"Original Char", "Occurrences", "Mapped char",
		"-----------------------------------------------------"
	);
	/* print array r */
	for(i = 0; i < rl; i++)
		if(alpha_only) {
			if(r[i].orig == ' ')
				printf("%15s | %15d | %15s |\n", "' '", r[i].occ, "' '");
			else if(r[i].orig == '\n')
				printf("%15s | %15d | %15s |\n", "\\n", r[i].occ, "\\n");
			else
				printf("%15c | %15d | %15c |\n", r[i].orig, r[i].occ, r[i].new);
		} else {
			if(r[i].orig == ' ')
				printf("%15s | %15d | %15c |\n", "' '", r[i].occ, r[i].new);
			else if(r[i].orig == '\n')
				printf("%15s | %15d | %15c |\n", "\\n", r[i].occ, r[i].new);
			else
				printf("%15c | %15d | %15c |\n", r[i].orig, r[i].occ, r[i].new);
		}
}

void
print_file(FILE *fi) {
	FILE *fo;
	int c;

	rewind(fi);
	fo = fopen(out, "w");
	c = fgetc(fi);
	while(c != EOF) {
		putc(substitute(c), fo);
		c = fgetc(fi);
	}
	fclose(fo);
}

void
print_video(FILE *fi) {
	int c;

	rewind(fi);
	printf("Substitution output:\n");
	c = fgetc(fi);
	while(c != EOF) {
		putc(substitute(c), stdout);
		c = fgetc(fi);
	}
}

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

void
count_bigrams(FILE *fi) {
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
		iter = g_list_first(bigram_list);
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
			bigram_list = g_list_append(bigram_list, tmp);
		}
		/* go on */
		a0 = a1;
		a1 = fgetc(fi);
	}
}

void
count_trigrams(FILE *fi) {
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
		iter = g_list_first(trigram_list);
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
			trigram_list = g_list_append(trigram_list, tmp);
		}
		/* go on */
		a0 = a1;
		a1 = a2;
		a2 = fgetc(fi);
	}
}

void
count_words(FILE *fi) {
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
			if(!case_sensitive)
				c = tolower(c);
			buf[i++] = c;
			c = fgetc(fi);
		}
		if(i) {
			buf[i] = '\0';
			iter = g_list_first(word_list);
			while(iter != NULL) {
				if(strcmp(((Word *)iter->data)->word, buf) == 0) {
					/* word already in word_list */
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
                                word_list = g_list_append(word_list, tmp);
			}
		}
	}
}

void
print_bigrams() {
	GList *iter = g_list_first(bigram_list);

	while(iter != NULL) {
		printf("%c%c%14d\n",
			((Bigram *)iter->data)->bigram[0],
			((Bigram *)iter->data)->bigram[1],
			((Bigram *)iter->data)->occ);
		iter = iter->next;
        }
}

void
print_trigrams() {
	GList *iter = g_list_first(trigram_list);

	while(iter != NULL) {
		printf("%c%c%c%15d\n",
			((Trigram *)iter->data)->trigram[0],
			((Trigram *)iter->data)->trigram[1],
			((Trigram *)iter->data)->trigram[2],
			((Trigram *)iter->data)->occ);
		iter = iter->next;
        }
}

void
print_words() {
	GList *iter = g_list_first(word_list);

	while(iter != NULL) {
		printf("%-20s%10d\n",
			((Word *)iter->data)->word,
			((Word *)iter->data)->occ);
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

int
main(int argc, char *argv[]) {
	FILE *fi;
	int i, c;
	extern char *optarg;
	extern int optind, opterr, optopt;

	/* handle command line options */
	opterr = 0;
	while((c = getopt(argc, argv, "vscabptwhi:o:l:")) != -1)
		switch(c) {
			case 'v':
				die("charemap-"VERSION", © 2009 Marco Squarcina, see LICENSE for details");
				break;
			case 's':
				show_occ = 1;
				break;
			case 'c':
				case_sensitive = 1;
				break;
			case 'a':
				alpha_only = 1;
				break;
			case 'b':
				show_bigrams = 1;
				break;
			case 't':
				show_trigrams = 1;
				break;
			case 'w':
				show_words = 1;
				break;
			case 'p':
				print_substituted = 1;
				break;
			case 'h':
				usage();
				break;
			case 'i':
				strcpy(in, optarg);
				break;
			case 'o':
				strcpy(out, optarg);
				break;
			case 'l':
				strcpy(lang, optarg);
				break;
			case '?':
				if(optopt == 'i' || optopt == 'o' || optopt == 'l')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if(isprint(optopt))
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				die("Try `-h' for more information.");
			default:
				die("Try `-h' for more information.");
		}
	for(i = optind; i < argc; i++) {
		printf("Non-option argument %s\n", argv[i]);
		die("Try `-h' for more information.");
	}
	/* set default language */
	if(strlen(lang) == 0)
		strcpy(lang, "en");
	/* load language file into map */
	mapl = load_lang(lang, map);
	/* check for an input file */
	if(strlen(in) == 0) {
		die("You need at least an input file!\nTry `-h' for more information.");
	}
        if((fi = fopen(in, "r")) == NULL)
	                die("Input file not found.");
	/* create relation */
	rl = initialize(fi);
	/* sort array */
	sort_by_occ();
	/* associate each character to a new one */
	associate();
	count_bigrams(fi);
        count_trigrams(fi);
	/* show char set */
	if(show_occ)
		print_char_occ();
	if(show_bigrams)
		print_bigrams();
	if(show_trigrams)
		print_trigrams();
	if(show_words) {
		count_words(fi);
		print_words();
	}
	/* print translated text file to stdout or a file */
	if(strlen(out) > 0)
		print_file(fi);
	if(print_substituted)
		print_video(fi);
	/* close file streams and clear memory */
        fclose(fi);
	free_list(bigram_list);
	free_list(trigram_list);
        if(show_words)
		free_list(word_list);

	return 0;
}
