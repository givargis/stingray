/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * main.c
 */

#include "index/s_index.h"
#include "lang/s_lang_parser.h"

#define VERSION 100

static void
print(const struct s__lang_node *node)
{
	if (node) {
		print(node->cond);
		print(node->left);
		print(node->right);
		printf("%2d: %15s", node->id, S__LANG_NODE_STR[node->op]);
		if (node->cond) {
			printf(" %2d", node->cond->id);
		}
		else {
			printf(" --");
		}
		if (node->left) {
			printf(" %2d", node->left->id);
		}
		else {
			printf(" --");
		}
		if (node->right) {
			printf(" %2d", node->right->id);
		}
		else {
			printf(" --");
		}
		printf("\n");
	}
}

static int
stage(void)
{
	const struct s__lang_node *node;
	s__lang_parser_t parser;
	s__lang_lexer_t lexer;

	if (!(lexer = s__lang_lexer_open("test"))) {
		S__TRACE(0);
		return -1;
	}
	s__lang_lexer_print(lexer);
	if (!(parser = s__lang_parser_open(lexer))) {
		s__lang_lexer_close(lexer);
		S__TRACE(0);
		return -1;
	}
	node = s__lang_parser_root(parser);
	print(node);
	s__lang_lexer_close(lexer);
	s__lang_parser_close(parser);
	return 0;
}

static void
help(void)
{
	printf("\n"
	       "Usage: stingray [options]\n"
	       "\n"
	       "Options:\n"
	       "\t --help    Print the help menu and exit\n"
	       "\t --version Print the version string and exit\n"
	       "\t --bist    Run the built-in-test mode and exit\n"
	       "\t --notrace Do not print error traces\n"
	       "\t --nocolor Do not use terminal colors\n"
	       "\n");
}

int
main(int argc, char *argv[])
{
	int notrace = 0;
	int nocolor = 0;
	int bist = 0;
	int i;

	for (i=1; i<argc; ++i) {
		if (!strcmp(argv[i], "--help") && (2 == argc)) {
			help();
			return 0;
		}
		else if (!strcmp(argv[i], "--version")) {
			printf("%d.%d.%d\n",
			       VERSION / 100,
			       VERSION / 10 % 10,
			       VERSION % 10);
			return 0;
		}
		else if (!strcmp(argv[i], "--notrace")) {
			notrace = 1;
		}
		else if (!strcmp(argv[i], "--nocolor")) {
			nocolor = 1;
		}
		else if (!strcmp(argv[i], "--bist")) {
			bist = 1;
		}
		else {
			fprintf(stderr, "bad argument: '%s'\n", argv[i]);
			return -1;
		}
	}
	s__kernel_init(notrace, nocolor);
	s__utils_init();
	if (bist) {
		if (s__utils_bist() || s__index_bist()) {
			S__TRACE(0);
			return -1;
		}
		return 0;
	}
	return stage();
}
