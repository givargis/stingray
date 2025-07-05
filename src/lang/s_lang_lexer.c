/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_lang_lexer.c
 */

#include "s_lang_map.h"
#include "s_lang_lexer.h"

#define TRACE(l,m,x)					\
	do {						\
		s__log("error: %s:%lu:%lu: " m,		\
		       (l)->pathname,			\
		       (unsigned long)(l)->lineno,	\
		       (unsigned long)(l)->column,	\
		       (x));				\
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

static int
hexval(int c)
{
	c = tolower(c);
	if (isdigit(c)) {
		return c - '0';
	}
	if (('a' <= c) && ('f' >= c)) {
		return c - 'a' + 10;
	}
	return -1;
}

static int
charval(const char *s, int *c)
{
	int i, v;

	(*c) = -1;
	if ('\\' == (*s)) {
		switch (*(++s)) {
		case 'a' : (*c) = '\a'; ++s; break;
		case 'b' : (*c) = '\b'; ++s; break;
		case 'f' : (*c) = '\f'; ++s; break;
		case 'n' : (*c) = '\n'; ++s; break;
		case 'r' : (*c) = '\r'; ++s; break;
		case 't' : (*c) = '\t'; ++s; break;
		case 'v' : (*c) = '\v'; ++s; break;
		case '\\': (*c) = '\\'; ++s; break;
		case '\'': (*c) = '\''; ++s; break;
		case '\"': (*c) = '\"'; ++s; break;
		case '\?': (*c) = '\?'; ++s; break;
		case 'x':
			++s;
			(*c) = 0;
			for (i=0; i<8; ++i,++s) {
				if (0 > (v = hexval((unsigned char)*s))) {
					break;
				}
				(*c) = (*c) * 16 + v;
			}
			break;
		default:
			(*c) = 0;
			for (i=0; i<3; ++i,++s) {
				if (('0' > (*s)) || ('7' < (*s))) {
					break;
				}
				(*c) = (*c) * 8 + (int)(*s) - '0';
			}
		}
	}
	else if ('\0' != (*s)) {
		(*c) = (int)(*(s++));
	}
	return ((0 > (*c)) || ('\0' != (*s))) ? -1 : 0;
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
			TRACE(lexer, "'/*' within block comment", "");
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
	TRACE(lexer, "unterminated comment", "");
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
			TRACE(lexer, "missing terminating character", "");
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
	const char *s;

	if (!(s = strdupl(b + 1, e - 1))) {
		S__TRACE(0);
		return -1;
	}
	if ('\'' == (*b)) {
		if (!(token = mktoken(lexer, S__LANG_LEXER_CHAR, e - b))) {
			S__TRACE(0);
			return -1;
		}
		if (charval(s, &token->u.c)) {
			TRACE(lexer, "invalid character literal '%s'", s);
			S__FREE(s);
			return -1;
		}
		S__FREE(s);
	}
	else {
		if (!(token = mktoken(lexer, S__LANG_LEXER_STRING, e - b))) {
			S__TRACE(0);
			return -1;
		}
		token->u.s = s;
	}
	return 0;
}

static char *
process_numeric(struct s__lang_lexer *lexer, char *s)
{
	struct s__lang_lexer_token *token;
	char *b, *e;
	s__real r;

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
			if (s__uint256_init(&token->u.u, s)) {
				TRACE(lexer, "invalid integer", "");
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
			if (s__int256_init(&token->u.i, s)) {
				TRACE(lexer, "invalid integer", "");
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
			r = (s__real)strtod(b, &e);
			if ((EINVAL == errno) || (ERANGE == errno)) {
				TRACE(lexer, "invalid real value", "");
				return NULL;
			}
			if (!(token = mktoken(lexer, S__LANG_LEXER_REAL, 0))) {
				S__TRACE(0);
				return NULL;
			}
			token->u.r = r;
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
		if (s__uint256_init(&token->u.u, s)) {
			TRACE(lexer, "invalid integer", "");
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
		if (s__int256_init(&token->u.i, s)) {
			TRACE(lexer, "invalid integer", "");
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
		}
		else if (is_identifier(b, e)) {
			if (!(token = mktoken(lexer,
					      S__LANG_LEXER_IDENTIFIER,
					      e - b)) ||
			    !(token->u.s = strdupl(b, e))) {
				S__TRACE(0);
				return -1;
			}
		}
		else {
			TRACE(lexer, "unrecognized character", "");
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
	I_(map, "auto",     S__LANG_LEXER_KEYWORD_AUTO);
	I_(map, "break",    S__LANG_LEXER_KEYWORD_BREAK);
	I_(map, "case",     S__LANG_LEXER_KEYWORD_CASE);
	I_(map, "char",     S__LANG_LEXER_KEYWORD_CHAR);
	I_(map, "const",    S__LANG_LEXER_KEYWORD_CONST);
	I_(map, "continue", S__LANG_LEXER_KEYWORD_CONTINUE);
	I_(map, "default",  S__LANG_LEXER_KEYWORD_DEFAULT);
	I_(map, "do",       S__LANG_LEXER_KEYWORD_DO);
	I_(map, "double",   S__LANG_LEXER_KEYWORD_DOUBLE);
	I_(map, "else",     S__LANG_LEXER_KEYWORD_ELSE);
	I_(map, "enum",     S__LANG_LEXER_KEYWORD_ENUM);
	I_(map, "extern",   S__LANG_LEXER_KEYWORD_EXTERN);
	I_(map, "float",    S__LANG_LEXER_KEYWORD_FLOAT);
	I_(map, "for",      S__LANG_LEXER_KEYWORD_FOR);
	I_(map, "goto",     S__LANG_LEXER_KEYWORD_GOTO);
	I_(map, "if",       S__LANG_LEXER_KEYWORD_IF);
	I_(map, "int",      S__LANG_LEXER_KEYWORD_INT);
	I_(map, "long",     S__LANG_LEXER_KEYWORD_LONG);
	I_(map, "register", S__LANG_LEXER_KEYWORD_REGISTER);
	I_(map, "return",   S__LANG_LEXER_KEYWORD_RETURN);
	I_(map, "short",    S__LANG_LEXER_KEYWORD_SHORT);
	I_(map, "signed",   S__LANG_LEXER_KEYWORD_SIGNED);
	I_(map, "sizeof",   S__LANG_LEXER_KEYWORD_SIZEOF);
	I_(map, "static",   S__LANG_LEXER_KEYWORD_STATIC);
	I_(map, "struct",   S__LANG_LEXER_KEYWORD_STRUCT);
	I_(map, "switch",   S__LANG_LEXER_KEYWORD_SWITCH);
	I_(map, "typedef",  S__LANG_LEXER_KEYWORD_TYPEDEF);
	I_(map, "union",    S__LANG_LEXER_KEYWORD_UNION);
	I_(map, "unsigned", S__LANG_LEXER_KEYWORD_UNSIGNED);
	I_(map, "void",     S__LANG_LEXER_KEYWORD_VOID);
	I_(map, "volatile", S__LANG_LEXER_KEYWORD_VOLATILE);
	I_(map, "while",    S__LANG_LEXER_KEYWORD_WHILE);
	/*-*/
	I_(map, "+",        S__LANG_LEXER_OPERATOR_ADD);
	I_(map, "-",        S__LANG_LEXER_OPERATOR_SUB);
	I_(map, "*",        S__LANG_LEXER_OPERATOR_MUL);
	I_(map, "/",        S__LANG_LEXER_OPERATOR_DIV);
	I_(map, "%",        S__LANG_LEXER_OPERATOR_MOD);
	I_(map, "<<",       S__LANG_LEXER_OPERATOR_SHL);
	I_(map, ">>",       S__LANG_LEXER_OPERATOR_SHR);
	I_(map, "|",        S__LANG_LEXER_OPERATOR_OR);
	I_(map, "^",        S__LANG_LEXER_OPERATOR_XOR);
	I_(map, "&",        S__LANG_LEXER_OPERATOR_AND);
	I_(map, "~",        S__LANG_LEXER_OPERATOR_NOT);
	I_(map, "||",       S__LANG_LEXER_OPERATOR_LOGIC_OR);
	I_(map, "&&",       S__LANG_LEXER_OPERATOR_LOGIC_AND);
	I_(map, "!",        S__LANG_LEXER_OPERATOR_LOGIC_NOT);
	I_(map, "++",       S__LANG_LEXER_OPERATOR_INC);
	I_(map, "--",       S__LANG_LEXER_OPERATOR_DEC);
	I_(map, "<",        S__LANG_LEXER_OPERATOR_LT);
	I_(map, ">",        S__LANG_LEXER_OPERATOR_GT);
	I_(map, "<=",       S__LANG_LEXER_OPERATOR_LE);
	I_(map, ">=",       S__LANG_LEXER_OPERATOR_GE);
	I_(map, "==",       S__LANG_LEXER_OPERATOR_EQ);
	I_(map, "!=",       S__LANG_LEXER_OPERATOR_NE);
	I_(map, "+=",       S__LANG_LEXER_OPERATOR_ADDASN);
	I_(map, "-=",       S__LANG_LEXER_OPERATOR_SUBASN);
	I_(map, "*=",       S__LANG_LEXER_OPERATOR_MULASN);
	I_(map, "/=",       S__LANG_LEXER_OPERATOR_DIVASN);
	I_(map, "%=",       S__LANG_LEXER_OPERATOR_MODASN);
	I_(map, "<<=",      S__LANG_LEXER_OPERATOR_SHLASN);
	I_(map, ">>=",      S__LANG_LEXER_OPERATOR_SHRASN);
	I_(map, "|=",       S__LANG_LEXER_OPERATOR_ORASN);
	I_(map, "^=",       S__LANG_LEXER_OPERATOR_XORASN);
	I_(map, "&=",       S__LANG_LEXER_OPERATOR_ANDASN);
	I_(map, "=",        S__LANG_LEXER_OPERATOR_ASN);
	I_(map, "{",        S__LANG_LEXER_OPERATOR_OPEN_BRACE);
	I_(map, "}",        S__LANG_LEXER_OPERATOR_CLOSE_BRACE);
	I_(map, "<%",       S__LANG_LEXER_OPERATOR_OPEN_BRACE);
	I_(map, "%>",       S__LANG_LEXER_OPERATOR_CLOSE_BRACE);
	I_(map, "(",        S__LANG_LEXER_OPERATOR_OPEN_PARENTH);
	I_(map, ")",        S__LANG_LEXER_OPERATOR_CLOSE_PARENTH);
	I_(map, "[",        S__LANG_LEXER_OPERATOR_OPEN_BRACKET);
	I_(map, "]",        S__LANG_LEXER_OPERATOR_CLOSE_BRACKET);
	I_(map, "<:",       S__LANG_LEXER_OPERATOR_OPEN_BRACKET);
	I_(map, ":>",       S__LANG_LEXER_OPERATOR_CLOSE_BRACKET);
	I_(map, ".",        S__LANG_LEXER_OPERATOR_DOT);
	I_(map, ",",        S__LANG_LEXER_OPERATOR_COMMA);
	I_(map, ":",        S__LANG_LEXER_OPERATOR_COLON);
	I_(map, "->",       S__LANG_LEXER_OPERATOR_POINTER);
	I_(map, "?",        S__LANG_LEXER_OPERATOR_QUESTION);
	I_(map, ";",        S__LANG_LEXER_OPERATOR_SEMICOLON);
	I_(map, "...",      S__LANG_LEXER_OPERATOR_DOTDOTDOT);
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
				if ((S__LANG_LEXER_STRING == token->op) ||
				    (S__LANG_LEXER_IDENTIFIER == token->op)) {
					S__FREE(token->u.s);
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

const struct s__lang_lexer_token *
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

int
s__lang_lexer_print(s__lang_lexer_t lexer)
{
	const struct s__lang_lexer_token *token;
	s__table_t table;
	char buf[256];
	uint64_t i;

	assert( lexer );

	if (!(table = s__table_open(s__lang_lexer_size(lexer) + 1, 4))) {
		S__TRACE(0);
		return -1;
	}
	s__table_insert(table, 0, 0, "lineno");
	s__table_insert(table, 0, 1, "column");
	s__table_insert(table, 0, 2, "op");
	s__table_insert(table, 0, 3, "value");
	for (i=0; i<s__lang_lexer_size(lexer); ++i) {
		token = s__lang_lexer_lookup(lexer, i);
		s__sprintf(buf,
			   sizeof (buf),
			   "%lu",
			   (unsigned long)token->lineno);
		s__table_insert(table, i + 1, 0, buf);
		s__sprintf(buf,
			   sizeof (buf),
			   "%lu",
			   (unsigned long)token->column);
		s__table_insert(table, i + 1, 1, buf);
		s__table_insert(table, i + 1, 2, S__LANG_LEXER_STR[token->op]);
		if (S__LANG_LEXER_INT == token->op) {
			s__int256_dec(&token->u.i, buf);
			s__table_insert(table, i + 1, 3, buf);
		}
		else if (S__LANG_LEXER_UINT == token->op) {
			s__uint256_dec(&token->u.u, buf);
			s__table_insert(table, i + 1, 3, buf);
		}
		else if (S__LANG_LEXER_CHAR == token->op) {
			s__sprintf(buf,
				   sizeof (buf),
				   "%d",
				   token->u.c);
			s__table_insert(table, i + 1, 3, buf);
		}
		else if (S__LANG_LEXER_REAL == token->op) {
			s__sprintf(buf,
				   sizeof (buf),
				   "%a",
				   token->u.r);
			s__table_insert(table, i + 1, 3, buf);
		}
		else if ((S__LANG_LEXER_STRING == token->op) ||
			 (S__LANG_LEXER_IDENTIFIER == token->op)) {
			s__table_insert(table, i + 1, 3, token->u.s);
		}
	}
	s__table_print(table);
	s__table_close(table);
	return 0;
}

const char * const S__LANG_LEXER_STR[] = {
	"",
	"EOF",
	"INT",
	"UINT",
	"REAL",
	"CHAR",
	"STRING",
	"IDENTIFIER",
	/*-*/
	"KEYWORD_",
	"KEYWORD_AUTO",
	"KEYWORD_BREAK",
	"KEYWORD_CASE",
	"KEYWORD_CHAR",
	"KEYWORD_CONST",
	"KEYWORD_CONTINUE",
	"KEYWORD_DEFAULT",
	"KEYWORD_DO",
	"KEYWORD_DOUBLE",
	"KEYWORD_ELSE",
	"KEYWORD_ENUM",
	"KEYWORD_EXTERN",
	"KEYWORD_FLOAT",
	"KEYWORD_FOR",
	"KEYWORD_GOTO",
	"KEYWORD_IF",
	"KEYWORD_INT",
	"KEYWORD_LONG",
	"KEYWORD_REGISTER",
	"KEYWORD_RETURN",
	"KEYWORD_SHORT",
	"KEYWORD_SIGNED",
	"KEYWORD_SIZEOF",
	"KEYWORD_STATIC",
	"KEYWORD_STRUCT",
	"KEYWORD_SWITCH",
	"KEYWORD_TYPEDEF",
	"KEYWORD_UNION",
	"KEYWORD_UNSIGNED",
	"KEYWORD_VOID",
	"KEYWORD_VOLATILE",
	"KEYWORD_WHILE",
	/*-*/
	"OPERATOR_",
	"OPERATOR_ADD",
	"OPERATOR_SUB",
	"OPERATOR_MUL",
	"OPERATOR_DIV",
	"OPERATOR_MOD",
	"OPERATOR_SHL",
	"OPERATOR_SHR",
	"OPERATOR_OR",
	"OPERATOR_XOR",
	"OPERATOR_AND",
	"OPERATOR_NOT",
	"OPERATOR_LOGIC_OR",
	"OPERATOR_LOGIC_AND",
	"OPERATOR_LOGIC_NOT",
	"OPERATOR_INC",
	"OPERATOR_DEC",
	"OPERATOR_LT",
	"OPERATOR_GT",
	"OPERATOR_LE",
	"OPERATOR_GE",
	"OPERATOR_EQ",
	"OPERATOR_NE",
	"OPERATOR_ADDASN",
	"OPERATOR_SUBASN",
	"OPERATOR_MULASN",
	"OPERATOR_DIVASN",
	"OPERATOR_MODASN",
	"OPERATOR_SHLASN",
	"OPERATOR_SHRASN",
	"OPERATOR_ORASN",
	"OPERATOR_XORASN",
	"OPERATOR_ANDASN",
	"OPERATOR_ASN",
	"OPERATOR_OPEN_BRACE",
	"OPERATOR_CLOSE_BRACE",
	"OPERATOR_OPEN_PARENTH",
	"OPERATOR_CLOSE_PARENTH",
	"OPERATOR_OPEN_BRACKET",
	"OPERATOR_CLOSE_BRACKET",
	"OPERATOR_DOT",
	"OPERATOR_COMMA",
	"OPERATOR_COLON",
	"OPERATOR_POINTER",
	"OPERATOR_QUESTION",
	"OPERATOR_SEMICOLON",
	"OPERATOR_DOTDOTDOT"
};
