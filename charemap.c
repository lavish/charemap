/*
 * Author:	Marco Squarcina <lavish@gmail.com>
 * Date:	03/01/2010
 * Version:	0.5
 * License:	MIT, see LICENSE for details
 * Description:	charemap.c, core of charemap, a tiny program to play with
 * 		substitution ciphers.
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <glib.h>
#include "charemap.h"
#include "decrypt.h"
#include "utils.h"

/* function implementations */
void
die(const char *errstr) {
        fprintf(stderr, "%s\n", errstr);
        exit(EXIT_FAILURE);
}

void
usage() {
	printf("Usage: charemap [options]...\nOptions:\n");
	printf("  %-15s %s\n  %-15s %s\n  %-15s %s\n                  %s\n                  %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n  %-15s %s\n",
		"-h",		"This help.",
		"-v",		"Print version.",
		"-d",		"Decrypt the file.", "Warning, this algorithm works ONLY on alphabetic lowercase characters.", "Remap ciphertext with charemap before using this option.",
		"-s",		"Show only character occurrences and mapping.",
		"-c",		"Case sensitive.",
		"-a",		"Remap alpha characters only (preserves dots, blanks and so on...).",
		"-p",		"Print substituted text.",
		"-b",		"Show bigrams.",
		"-t",		"Show trigrams.",
		"-w",		"Show words.",
		"-m <file>",	"Use a sample file to generate digram statistics (default samples/moby.txt).",
		"-i <file>",	"Input file to parse.",
		"-o <file>",	"Output file with remapped characters.",
		"-l <file>",	"Remap using the typical character frequency of selected language (default: languages/en.txt).");
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
load_lang(FILE *l, char *map) {
	int c, i = 0;

	while((c = fgetc(l)) != EOF)
		map[i++] = c;

	return i;
}

int
initialize_relation(FILE *fi) {
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
remap_file_to_file(FILE *fi, FILE *fo) {
	int c;

	rewind(fi);
	while((c = fgetc(fi)) != EOF) {
		putc(substitute(c), fo);
	}
}

void
remap_to_video(FILE *f) {
	int c;

	rewind(f);
	printf("Substitution output:\n");
	while((c = fgetc(f)) != EOF) {
		putc(substitute(c), stdout);
	}
}

int
main(int argc, char *argv[]) {
	FILE *fi, *fs, *ftmp;
	int i, c;
	char in[N] = {'\0'};
	char out[N] = {'\0'};
	char lang[N] = {'\0'};
	char sample[N] = {'\0'};
	GList *word_list = NULL;
	GList *bigram_list = NULL;
	GList *trigram_list = NULL;
        extern char *optarg;
	extern int optind, opterr, optopt;

	/* handle command line options */
	opterr = 0;
	while((c = getopt(argc, argv, "vscdabptwhm:i:o:l:")) != -1)
		switch(c) {
			case 'v':
				die("charemap-"VERSION", © 2009-2010 Marco Squarcina, see LICENSE for details");
				break;
			case 's':
				show_occ = 1;
				break;
			case 'd':
				decrypt_flag = 1;
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
			case 'm':
				strcpy(sample, optarg);
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
				if(optopt == 'i' || optopt == 'o' || optopt == 'l' || optopt == 'm')
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
		strcpy(lang, "languages/en.txt");
        /* load language file into map */
        if((ftmp = fopen(lang, "r")) == NULL)
                die("Language file not found.");
	mapl = load_lang(ftmp, map);
	fclose(ftmp);
	/* check for the sample file */
	if(strlen(sample) == 0)
		strcpy(sample, "samples/moby.txt");
        if((fs = fopen(sample, "r")) == NULL)
		die("Sample file not found.");
	/* check for an input file */
	if(strlen(in) == 0)
		die("You need at least an input file!\nTry `-h' for more information.");
        if((fi = fopen(in, "r")) == NULL)
		die("Input file not found.");
	/* create relation */
	rl = initialize_relation(fi);
	/* sort array */
	sort_by_occ();
	/* associate each character to a new one */
	associate();
	if(decrypt_flag)
		decrypt(fi, fs);
	/* show char set */
	if(show_occ)
		print_char_occ();
	if(show_bigrams) {
		bigram_list = count_bigrams(fi, bigram_list, case_sensitive, alpha_only);
		print_bigrams(bigram_list);
		free_list(bigram_list);
	}
	if(show_trigrams) {
		trigram_list = count_trigrams(fi, trigram_list, case_sensitive, alpha_only);
		print_trigrams(trigram_list);
		free_list(trigram_list);
	}
	if(show_words) {
		word_list = count_words(fi, word_list, case_sensitive);
		print_words(word_list);
		free_list(word_list);
	}
	/* print translated text file to stdout or a file */
	if(strlen(out) > 0) {
		ftmp = fopen(out, "w");
		remap_file_to_file(fi, ftmp);
		fclose(ftmp);
	}
	if(print_substituted)
		remap_to_video(fi);
	/* close file streams */
        fclose(fi);

	return 0;
}
