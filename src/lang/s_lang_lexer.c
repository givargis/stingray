/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_lang_lexer.c
 */

#include "s_lang_map.h"
#include "s_lang_lexer.h"

#define TRACE(l,m)					\
	do {						\
		s__log("error: %s:%lu:%lu: %s",		\
		       (l)->pathname,			\
		       (unsigned long)(l)->lineno,	\
		       (unsigned long)(l)->column,	\
		       (m));				\
		S__TRACE(S__ERR_SYNTAX);		\
	}						\
	while (0)

struct s__lang_lexer {
	char *buf;
	uint64_t lineno;
	uint64_t column;
	s__lang_map_t map;
	const char *pathname;
	struct {
		uint64_t size;
		uint64_t capacity;
		struct s__lang_lexer_token *tokens;
	} tokens;
};

static char *
strdupl(const char *b, const char *e)
{
	uint64_t n;
	char *s;

	n = e - b;
	if (!(s = s__malloc(n + 1))) {
		S__TRACE(0);
		return NULL;
	}
	memcpy(s, b, n);
	s[n] = '\0';
	return s;
}

static int
is_identifier(const char *b, const char *e)
{
	if (('_' == (*b)) || isalpha((unsigned char)(*b))) {
		while (++b < e) {
			if (('_' != (*b)) && !isalnum((unsigned char)(*b))) {
				return 0;
			}
		}
		return 1;
	}
	return 0;
}

static struct s__lang_lexer_token *
mktoken(struct s__lang_lexer *lexer, int op, uint64_t width)
{
	struct s__lang_lexer_token *token, *tokens;
	uint64_t n, capacity;

	if (lexer->tokens.size >= lexer->tokens.capacity) {
		capacity = lexer->tokens.size + 1024;
		n = capacity * sizeof (tokens[0]);
		if (!(tokens = s__realloc(lexer->tokens.tokens, n))) {
			S__TRACE(0);
			return NULL;
		}
		lexer->tokens.tokens = tokens;
		lexer->tokens.capacity = capacity;
	}
	token = &lexer->tokens.tokens[lexer->tokens.size++];
	memset(token, 0, sizeof (tokens[0]));
	token->op = op;
	token->lineno = lexer->lineno;
	token->column = lexer->column - width;
	return token;
}

static char *
eat_eol(char *s)
{
	while (*s) {
		if ('\n' == (*s)) {
			++s;
			break;
		}
		(*s) = ' ';
		++s;
	}
	return s;
}

static char *
eat_comment(struct s__lang_lexer *lexer, char *s)
{
	s[0] = s[1] = ' ';
	while (*s) {
		if ('\n' == (*s)) {
			lexer->lineno += 1;
			lexer->column  = 0;
		}
		else if (('/' == s[0]) && ('*' == s[1])) {
			TRACE(lexer, "'/*' within block comment");
			return NULL;
		}
		else if (('*' == s[0]) && ('/' == s[1])) {
			s[0] = s[1] = ' ';
			lexer->column += 2;
			return s + 2;
		}
		else {
			(*s) = ' ';
		}
		++lexer->column;
		++s;
	}
	TRACE(lexer, "unterminated comment");
	return NULL;
}

static int
eat_comments(struct s__lang_lexer *lexer)
{
	int skip;
	char *s;

	skip = 0;
	s = lexer->buf;
	lexer->lineno = 1;
	lexer->column = 1;
	while (*s) {
		if (!skip) {
			if ('"' == (*s)) {
				skip = 1;
			}
			if ('\'' == (*s)) {
				skip = 2;
			}
		}
		else if ((1 == skip) && ('"' == (*s))) {
			skip = 0;
		}
		else if ((2 == skip) && ('\'' == (*s))) {
			skip = 0;
		}
		/*-*/
		if (!skip && ('/' == s[0]) && ('/' == s[1])) {
			s = eat_eol(s);
			lexer->lineno += 1;
			lexer->column  = 1;
		}
		else if (!skip && ('/' == s[0]) && ('*' == s[1])) {
			if (!(s = eat_comment(lexer, s))) {
				S__TRACE(0);
				return -1;
			}
		}
		else if ('\n' == (*s)) {
			lexer->lineno += 1;
			lexer->column  = 1;
			++s;
		}
		else {
			++lexer->column;
			++s;
		}
	}
	return 0;
}

static char *
eat_string(struct s__lang_lexer *lexer, char *s, char delim)
{
	while (*s) {
		if (delim == (*s)) {
			++lexer->column;
			++s;
			break;
		}
		else if ('\\' == (*s)) {
			++lexer->column;
			++s;
		}
		else if ('\n' == (*s)) {
			TRACE(lexer, "missing terminating character");
			return NULL;
		}
		++lexer->column;
		++s;
	}
	return s;
}

static int
process_string(struct s__lang_lexer *lexer, const char *b, const char *e)
{
	struct s__lang_lexer_token *token;

	if (!(token = mktoken(lexer, S__LANG_LEXER_STRING, e - b)) ||
	    !(token->val.s = strdupl(b, e))) {
		S__TRACE(0);
		return -1;
	}
	return 0;
}

static char *
process_numeric(struct s__lang_lexer *lexer, char *s)
{
	struct s__lang_lexer_token *token;
	char *b, *e;
	double r;

	/* int/uint (hex) */

	if (('0' == s[0]) && (('x' == s[1]) || ('X' == s[1]))) {
		b = s;
		s += 2;
		while (isdigit((unsigned char)(*s)) ||
		       (('a' <= tolower((unsigned char)(*s))) &&
			('f' >= tolower((unsigned char)(*s))))) {
			++s;
		}
		e = s;
		if (!(s = strdupl(b, e))) {
			S__TRACE(0);
			return NULL;
		}
		if (('u' == (*e)) || ('U' == (*e))) {
			++e;
			if (!(token = mktoken(lexer, S__LANG_LEXER_UINT, 0))) {
				S__FREE(s);
				S__TRACE(0);
				return NULL;
			}
			if (s__uint256_init(&token->val.u, s)) {
				TRACE(lexer, "invalid integer");
				S__FREE(s);
				return NULL;
			}
		}
		else {
			if (!(token = mktoken(lexer, S__LANG_LEXER_INT, 0))) {
				S__FREE(s);
				S__TRACE(0);
				return NULL;
			}
			if (s__int256_init(&token->val.i, s)) {
				TRACE(lexer, "invalid integer");
				S__FREE(s);
				return NULL;
			}
		}
		S__FREE(s);
		return e;
	}

	/* floating */

	e = b = s;
	while (*s) {
		if (('.' == (*s)) ||
		    ('e' == (*s)) ||
		    ('E' == (*s))) {
			errno = 0;
			r = strtod(b, &e);
			if ((EINVAL == errno) || (ERANGE == errno)) {
				TRACE(lexer, "invalid real value");
				return NULL;
			}
			if (!(token = mktoken(lexer, S__LANG_LEXER_REAL, 0))) {
				S__TRACE(0);
				return NULL;
			}
			token->val.r = r;
			return e;
		}
		if (!isdigit((unsigned char)(*s))) {
			break;
		}
		++s;
	}

	/* int/uint (dec) */

	e = s;
	if (!(s = strdupl(b, e))) {
		S__TRACE(0);
		return NULL;
	}
	if (('u' == (*e)) || ('U' == (*e))) {
		++e;
		if (!(token = mktoken(lexer, S__LANG_LEXER_UINT, 0))) {
			S__FREE(s);
			S__TRACE(0);
			return NULL;
		}
		if (s__uint256_init(&token->val.u, s)) {
			TRACE(lexer, "invalid integer");
			S__FREE(s);
			return NULL;
		}
	}
	else {
		if (!(token = mktoken(lexer, S__LANG_LEXER_INT, 0))) {
			S__FREE(s);
			S__TRACE(0);
			return NULL;
		}
		if (s__int256_init(&token->val.i, s)) {
			TRACE(lexer, "invalid integer");
			S__FREE(s);
			return NULL;
		}
	}
	S__FREE(s);
	return e;
}

static int
process(struct s__lang_lexer *lexer, const char *b, const char *e)
{
	struct s__lang_lexer_token *token;
	int op;

	if (b < e) {
		if ((op = s__lang_map_lookup(lexer->map, b, e))) {
			if (!(token = mktoken(lexer, op, e - b))) {
				S__TRACE(0);
				return -1;
			}
			else if (S__LANG_LEXER_KEYWORD_FALSE == op) {
				token->op = S__LANG_LEXER_BOOL;
				S__UINT256_SET(&token->val.u, 0);
			}
			else if (S__LANG_LEXER_KEYWORD_TRUE == op) {
				token->op = S__LANG_LEXER_BOOL;
				S__UINT256_SET(&token->val.u, 1);
			}
		}
		else if (is_identifier(b, e)) {
			if (!(token = mktoken(lexer,
					      S__LANG_LEXER_IDENTIFIER,
					      e - b)) ||
			    !(token->val.s = strdupl(b, e))) {
				S__TRACE(0);
				return -1;
			}
		}
		else {
			TRACE(lexer, "unrecognized character");
			return -1;
		}
	}
	return 0;
}

static char *
operator(struct s__lang_lexer *lexer, char *p, char *s)
{
	int op;

	if ((op = s__lang_map_lookup(lexer->map, s, s + 3))) {
		if (S__LANG_LEXER_OPERATOR_ < op) {
			if (process(lexer, p, s) || !mktoken(lexer, op, 0)) {
				S__TRACE(0);
				return NULL;
			}
			lexer->column += 3;
			return s + 3;
		}
	}
	if ((op = s__lang_map_lookup(lexer->map, s, s + 2))) {
		if (S__LANG_LEXER_OPERATOR_ < op) {
			if (process(lexer, p, s) || !mktoken(lexer, op, 0)) {
				S__TRACE(0);
				return NULL;
			}
			lexer->column += 2;
			return s + 2;
		}
	}
	if ((op = s__lang_map_lookup(lexer->map, s, s + 1))) {
		if (S__LANG_LEXER_OPERATOR_ < op) {
			if (process(lexer, p, s) || !mktoken(lexer, op, 0)) {
				S__TRACE(0);
				return NULL;
			}
			lexer->column += 1;
			return s + 1;
		}
	}
	return s;
}

static int
tokenize(struct s__lang_lexer *lexer)
{
	char *s, *p, *r;

	s = lexer->buf;
	p = lexer->buf;
	lexer->lineno = 1;
	lexer->column = 1;
	while (*s) {
		if (('"' == s[0]) || ('\'' == s[0])) {
			if (process(lexer, p, s)) {
				S__TRACE(0);
				return -1;
			}
			p = s;
			++lexer->column;
			if (!(s = eat_string(lexer, s + 1, (*s)))) {
				S__TRACE(0);
				return -1;
			}
			if (process_string(lexer, p, s)) {
				S__TRACE(0);
				return -1;
			}
			++lexer->column;
			p = s;
		}
		else if ((p == s) &&
			 ((('.' == s[0]) && isdigit((unsigned char)s[1])) ||
			  isdigit((unsigned char)(*s)))) {
			p = s;
			if (!(s = process_numeric(lexer, p))) {
				S__TRACE(0);
				return -1;
			}
			lexer->column += s - p;
			p = s;
		}
		else if (isspace((unsigned char)(*s))) {
			if (process(lexer, p, s)) {
				S__TRACE(0);
				return -1;
			}
			if ('\n' == (*s)) {
				lexer->lineno += 1;
				lexer->column  = 0;
			}
			++lexer->column;
			++s;
			p = s;
		}
		else {
			if (!(r = operator(lexer, p, s))) {
				S__TRACE(0);
				return -1;
			}
			if (s == r) {
				++lexer->column;
				++s;
			}
			else {
				s = r;
				p = s;
			}
		}
	}
	if (process(lexer, p, s) || !mktoken(lexer, S__LANG_LEXER_EOF, 0)) {
		S__TRACE(0);
		return -1;
	}
	return 0;
}

#define I_ s__lang_map_insert

static void
configure(struct s__lang_lexer *lexer)
{
	s__lang_map_t map;

	map = lexer->map;
	I_(map, "true",      S__LANG_LEXER_KEYWORD_TRUE);
	I_(map, "false",     S__LANG_LEXER_KEYWORD_FALSE);
	I_(map, "int",       S__LANG_LEXER_KEYWORD_INT);
	I_(map, "uint",      S__LANG_LEXER_KEYWORD_UINT);
	I_(map, "real",      S__LANG_LEXER_KEYWORD_REAL);
	I_(map, "bool",      S__LANG_LEXER_KEYWORD_BOOL);
	/*-*/
	I_(map, "+",         S__LANG_LEXER_OPERATOR_ADD);
	I_(map, "-",         S__LANG_LEXER_OPERATOR_SUB);
	I_(map, "/",         S__LANG_LEXER_OPERATOR_DIV);
	I_(map, "%",         S__LANG_LEXER_OPERATOR_MOD);
	I_(map, "*",         S__LANG_LEXER_OPERATOR_MUL);
	I_(map, "<<",        S__LANG_LEXER_OPERATOR_SHL);
	I_(map, ">>",        S__LANG_LEXER_OPERATOR_SHR);
	I_(map, "|",         S__LANG_LEXER_OPERATOR_OR);
	I_(map, "^",         S__LANG_LEXER_OPERATOR_XOR);
	I_(map, "&",         S__LANG_LEXER_OPERATOR_AND);
	I_(map, "~",         S__LANG_LEXER_OPERATOR_NOT);
	I_(map, "||",        S__LANG_LEXER_OPERATOR_LOGIC_OR);
	I_(map, "^^",        S__LANG_LEXER_OPERATOR_LOGIC_XOR);
	I_(map, "&&",        S__LANG_LEXER_OPERATOR_LOGIC_AND);
	I_(map, "!",         S__LANG_LEXER_OPERATOR_LOGIC_NOT);
	I_(map, "<",         S__LANG_LEXER_OPERATOR_LT);
	I_(map, ">",         S__LANG_LEXER_OPERATOR_GT);
	I_(map, "<=",        S__LANG_LEXER_OPERATOR_LE);
	I_(map, ">=",        S__LANG_LEXER_OPERATOR_GE);
	I_(map, "==",        S__LANG_LEXER_OPERATOR_EQ);
	I_(map, "!=",        S__LANG_LEXER_OPERATOR_NE);
	I_(map, "=",         S__LANG_LEXER_OPERATOR_ASSIGN);
	I_(map, "{",         S__LANG_LEXER_OPERATOR_OPEN_BRACE);
	I_(map, "}",         S__LANG_LEXER_OPERATOR_CLOSE_BRACE);
	I_(map, "(",         S__LANG_LEXER_OPERATOR_OPEN_PARENTH);
	I_(map, ")",         S__LANG_LEXER_OPERATOR_CLOSE_PARENTH);
	I_(map, "[",         S__LANG_LEXER_OPERATOR_OPEN_BRACKET);
	I_(map, "]",         S__LANG_LEXER_OPERATOR_CLOSE_BRACKET);
	I_(map, "@",         S__LANG_LEXER_OPERATOR_AT);
	I_(map, "#",         S__LANG_LEXER_OPERATOR_HASH);
	I_(map, ".",         S__LANG_LEXER_OPERATOR_DOT);
	I_(map, ",",         S__LANG_LEXER_OPERATOR_COMMA);
	I_(map, ":",         S__LANG_LEXER_OPERATOR_COLON);
	I_(map, "$",         S__LANG_LEXER_OPERATOR_DOLLAR);
	I_(map, "?",         S__LANG_LEXER_OPERATOR_QUESTION);
	I_(map, ";",         S__LANG_LEXER_OPERATOR_SEMICOLON);
}

s__lang_lexer_t
s__lang_lexer_open(const char *pathname)
{
	struct s__lang_lexer *lexer;

	assert( s__strlen(pathname) );

	if (!(lexer = s__malloc(sizeof (struct s__lang_lexer)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(lexer, 0, sizeof (struct s__lang_lexer));
	lexer->pathname = pathname;
	if (!(lexer->buf = (char *)s__file_read(lexer->pathname)) ||
	    !(lexer->map = s__lang_map_open())) {
		s__lang_lexer_close(lexer);
		S__TRACE(0);
		return NULL;
	}
	configure(lexer);
	if (eat_comments(lexer) || tokenize(lexer)) {
		s__lang_lexer_close(lexer);
		S__TRACE(0);
		return NULL;
	}
	return lexer;
}

void
s__lang_lexer_close(s__lang_lexer_t lexer)
{
	struct s__lang_lexer_token *token;
	uint64_t i;

	if (lexer) {
		if (lexer->tokens.tokens) {
			for (i=0; i<lexer->tokens.size; ++i) {
				token = &lexer->tokens.tokens[i];
				if (S__LANG_LEXER_IDENTIFIER == token->op) {
					S__FREE(token->val.s);
				}
				if (S__LANG_LEXER_STRING == token->op) {
					S__FREE(token->val.s);
				}
			}
		}
		s__lang_map_close(lexer->map);
		S__FREE(lexer->tokens.tokens);
		S__FREE(lexer->buf);
		memset(lexer, 0, sizeof (struct s__lang_lexer));
	}
	S__FREE(lexer);
}

struct s__lang_lexer_token *
s__lang_lexer_lookup(s__lang_lexer_t lexer, uint64_t i)
{
	assert( lexer );
	assert( i < lexer->tokens.size );

	return &lexer->tokens.tokens[i];
}

uint64_t
s__lang_lexer_size(s__lang_lexer_t lexer)
{
	assert( lexer );

	return lexer->tokens.size;
}

const char *
s__lang_lexer_pathname(s__lang_lexer_t lexer)
{
	assert( lexer );

	return lexer->pathname;
}
