/*
 * Description:	charemap.h, header file for charemap.c
 */

#define N	256

/* structs */
typedef struct {
	unsigned char orig;
	int occ;
	unsigned char new;
} Relation;

/* function declarations */
void die(const char *error);
void usage(void);
char substitute(char c);
void sort_by_occ(void);
int load_lang(FILE *l, char *map);
int initialize_relation(FILE *fi);
void associate(void);
void print_char_occ(void);
void remap_file_to_file(FILE *fi, FILE *fo);
void remap_to_video(FILE *fi);

/* variables */
Relation r[N];
char map[N] = {'\0'};
int rl, mapl;
int decrypt_flag = 0;
int show_occ = 0;
int show_bigrams = 0;
int show_trigrams = 0;
int show_words = 0;
int case_sensitive = 0;
int alpha_only = 0;
int print_substituted = 0;
