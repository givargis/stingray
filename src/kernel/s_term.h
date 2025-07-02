/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_term.h
 */

#ifndef _S_TERM_H_
#define _S_TERM_H_

enum {
	S__TERM_COLOR_BLACK,
	S__TERM_COLOR_RED,
	S__TERM_COLOR_GREEN,
	S__TERM_COLOR_YELLOW,
	S__TERM_COLOR_BLUE,
	S__TERM_COLOR_MAGENTA,
	S__TERM_COLOR_CYAN,
	S__TERM_COLOR_GRAY
};

void s__term_init(int nocolor);

void s__term_color(int color);

void s__term_bold(void);

void s__term_reset(void);

#endif /* _S_TERM_H_ */
