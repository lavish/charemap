/*
 * Author:	Marco Squarcina <lavish@gmail.com>
 * Date:	16/12/2009
 * Version:	0.2
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

#define N	512

/* structs */
typedef struct {
	char orig;
	int occ;
	char new;
} Relation;

/* function declarations */
static int load_lang(char *l, char *map);
static int initialize(Relation r[], FILE *fi);
static int isletter(char c);
static char substitute(Relation r[], char c, int rl);
static void usage(void);
static void die(const char *error);
static void sort_by_occ(Relation r[], int l);
static void associate(Relation r[], int rl, char map[], int mapl);
static void show_occ(Relation r[], int rl);
static void print_file(Relation r[], int rl, FILE *fi);
static void print_video(Relation r[], int rl, FILE *fi);

/* variables */
static int show = 0;
static int case_sensitive = 0;
static int alphabetic_only = 0;
static char in[N];
static char out[N];
static char lang[N];

/* function implementations */
void
die(const char *errstr) {
        fprintf(stderr, "%s\n", errstr);
        exit(EXIT_FAILURE);
}

void
usage() {
	printf("Usage: charemap [options]...\nOptions:\n");
	printf("  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n",
		"-h",		"This help",
		"-v",		"Print version",
		"-s",		"Show only character occurrences and mapping",
		"-c",		"Case sensitive",
		"-a",		"Remap alphabetic characters only (preserves dots, blanks and so on...)",
		"-i <file>",	"Input file to parse",
		"-o <file>",	"Output file with remapped characters",
		"-l <language>","Try to decrypt using the selected language (default: en)");
	exit(EXIT_FAILURE);
}

char
substitute(Relation r[], char c, int rl) {
        int i;

        if(!case_sensitive)
                c = tolower(c);
	for(i=0; i<rl; i++)
        	if(r[i].orig == c)
                	return r[i].new;
        return '?';
}

void
sort_by_occ(Relation *r, int l) {
	Relation tmp;
	int i, j;

	for (i = 1; i < l; i++)
		for ( j = 0; j < l - 1; j++ ) 
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
	f = fopen(fname, "r");
	c = fgetc(f);
	while(c != EOF) {
		map[i++] = c;
		c = fgetc(f);
	}
	fclose(f);

	return i-1;
}

int
isletter(char c) {
	if(c < 'A' || c > 'z' || (c > 'Z' && c < 'a'))
		return 0;
	return 1;
}

int
initialize(Relation *r, FILE *fi) {
	int c, i, j;

	/* set initial state */
	for(i = 0; i < N; i++) {
		r[i].occ = 0;
		r[i].new = '?';
	}
	c = fgetc(fi);
	/* by default everything to lowercase */
	if(!case_sensitive)
		c = tolower(c);
	/* count occurrences */
	for(i = 0; c != EOF;) {
		for(j = 0; j <= i; j++)
			if(r[j].orig == c) {
				/* char already in r */
				r[j].occ += 1;
				break;
			}
		if(i == j-1) {
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
associate(Relation r[], int rl, char map[], int mapl) {
	int i, j;

	for(i=0, j=0; i<rl;) {
		if(alphabetic_only && !isletter(r[i].orig)) {
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
show_occ(Relation r[], int rl) {
	int i;

	printf("%15s | %15s | %15s |\n%s\n",
		"Original Char", "Occurrences", "Mapped char",
		"-----------------------------------------------------"
	);
	/* print array r */
	for(i = 0; i < rl; i++)
		if(alphabetic_only) {
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
print_file(Relation r[], int rl, FILE *fi) {
	FILE *fo;
	int c;

	rewind(fi);
	fo = fopen(out, "w");
	c = fgetc(fi);
	while(c != EOF) {
		putc(substitute(r, c, rl), fo);
		c = fgetc(fi);
	}
	fclose(fo);
}

void
print_video(Relation r[], int rl, FILE *fi) {
	int c;

	rewind(fi);
	printf("Substitution output:\n");
	c = fgetc(fi);
	while(c != EOF) {
		putc(substitute(r, c, rl), stdout);
		c = fgetc(fi);
	}
}

int
main(int argc, char *argv[]) {
	FILE *fi;
	Relation r[N];
	char map[N];
	int i, c, rl, mapl;
	extern char *optarg;
	extern int optind, opterr, optopt;

	/* handle command line options */
	opterr = 0;
	while((c = getopt(argc, argv, "vscahi:o:l:")) != -1)
		switch(c) {
			case 'v':
				die("charemap-"VERSION", © 2009 Marco Squarcina, see LICENSE for details");
				break;
			case 's':
				show = 1;
				break;
			case 'c':
				case_sensitive = 1;
				break;
			case 'a':
				alphabetic_only = 1;
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
	if(strlen(in) == 0) {
		die("You need at least an input file!\nTry `-h' for more information.");
	}
	/* set default language */
	if(strlen(lang) == 0)
		strcpy(lang, "en");
	fi = fopen(in, "r");
	/* load language file into map */
	mapl = load_lang(lang, map);
	/* create relation */
	rl = initialize(r, fi);
	/* sort array */
	sort_by_occ(r, rl);
	/* associate each character to a new one */
	associate(r, rl, map, mapl);
	/* show char set */
	if(show)
		show_occ(r, rl);
	else {
		/* print translated text file to stdout or a file */
		if(strlen(out) > 0)
			print_file(r, rl, fi);
		else
			print_video(r, rl, fi);
	}
	/* close file streams */
        fclose(fi);
	
	return 0;
}
