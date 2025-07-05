/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * main.c
 */

#include "index/s_index.h"
#include "lang/s_lang_parser.h"

#define VERSION 100

static int
stage(void)
{
	return 0;
}

static int
compile(const char *arg)
{
	s__lang_parser_t parser;
	s__lang_lexer_t lexer;

	if (!(lexer = s__lang_lexer_open(arg))) {
		S__TRACE(0);
		return -1;
	}
	s__lang_lexer_print(lexer);
	if (!(parser = s__lang_parser_open(lexer))) {
		s__lang_lexer_close(lexer);
		S__TRACE(0);
		return -1;
	}
	s__lang_lexer_close(lexer);
	s__lang_parser_close(parser);
	return 0;
}

static void
help(void)
{
	printf("\n"
	       "Usage: stingray [options]\n"
	       "Usage: stingray [options] --compile input\n"
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
	enum { NONE, COMPILE } mode = NONE;
	const char *arg = NULL;
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
		else if (!strcmp(argv[i], "--compile") && !mode) {
			mode = COMPILE;
		}
		else if (('-' != argv[i][0]) && !arg) {
			arg = argv[i];
		}
		else {
			fprintf(stderr, "bad argument: '%s'\n", argv[i]);
			return -1;
		}
	}
	if ((NONE == mode) && arg) {
		fprintf(stderr, "bad argument: '%s'\n", arg);
		return -1;
	}
	if ((COMPILE == mode) && !arg) {
		fprintf(stderr, "missing argument\n");
		return -1;
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
	if (COMPILE == mode) {
		if (compile(arg)) {
			s__log("error: failed to compile '%s'", arg);
			S__TRACE(0);
			return -1;
		}
		return 0;
	}
	return stage();
}
