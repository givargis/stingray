/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_lang_parser.c
 */

#include "s_lang_parser.h"

#define TRACE(p,m,x)							\
	do {								\
		if (!(p)->error) {					\
			(p)->error = 1;					\
			s__log("error: %s:%lu:%lu: " m,			\
			       s__lang_lexer_pathname((p)->lexer),	\
			       (unsigned long)next((p))->lineno,	\
			       (unsigned long)next((p))->column,	\
			       (x));					\
			S__TRACE(S__ERR_SYNTAX);			\
		}							\
	}								\
	while (0)

#define MKN(p,n,o)							\
	do {								\
		if (!((n) = s__lang_node_allocate((p)->node))) {	\
			TRACE((p), "out of memory", "");		\
			return NULL;					\
		}							\
		(n)->id = ++(p)->id;					\
		(n)->op = (o);						\
		(n)->token = next((p));					\
	}								\
	while (0)

struct s__lang_parser {
	int id;
	int error;
	uint64_t i;
	uint64_t n;
	s__lang_node_t node;
	s__lang_lexer_t lexer; /* ref */
	struct s__lang_node *root; /* ref */
};

const static struct s__lang_lexer_token *
next(const struct s__lang_parser *parser)
{
	if (!parser->error && (parser->i < parser->n)) {
		return s__lang_lexer_lookup(parser->lexer, parser->i);
	}
	return s__lang_lexer_lookup(parser->lexer, parser->n - 1);
}

static int
match(const struct s__lang_parser *parser, int op)
{
	const struct s__lang_lexer_token *token;

	if ((token = next(parser)) && (op == token->op)) {
		return 1;
	}
	return 0;
}

static void
forward(struct s__lang_parser *parser)
{
	if (parser->i < parser->n) {
		++parser->i;
	}
}

/**============================================================================
 * (E0)  expr_arglist
 * (E1)  expr_primary
 * (E2)  expr_postfix
 * (E3)  expr_unary
 * (E4)  expr_cast
 * (E5)  expr_multiplicative
 * (E6)  expr_additive
 * (E7)  expr_shift
 * (E8)  expr_relational
 * (E9)  expr_equality
 * (E10) expr_and
 * (E11) expr_xor
 * (E12) expr_or
 * (E13) expr_logic_and
 * (E14) expr_logic_xor_or
 * (E15) expr
 *===========================================================================*/

static struct s__lang_node *expr(struct s__lang_parser *parser);
static struct s__lang_node *expr_cast(struct s__lang_parser *parser);
static struct s__lang_node *type_name(struct s__lang_parser *parser);

/**
 * (E0)
 *
 * expr_arglist : expr { ',' expr } <<< expr -> expr_asn >>>
 */

static struct s__lang_node *
expr_args(struct s__lang_parser *parser)
{
	struct s__lang_node *node, *node_, *head, *tail;
	int mark;

	mark = 0;
	head = tail = NULL;
	while ((node_ = expr(parser))) {
		MKN(parser, node, S__LANG_NODE_EXPR_LIST);
		node->cond = node_;
		if (tail) {
			tail->right = node;
			tail = node;
		}
		else {
			head = node;
			tail = node;
		}
		if (!(mark = match(parser, S__LANG_LEXER_OPERATOR_COMMA))) {
			break;
		}
		forward(parser);
	}
	if (mark) {
		TRACE(parser, "dangling ','", "");
		return NULL;
	}
	return head;
}

/**
 * (E1)
 *
 * expr_primary : INT
 *              | UINT
 *              | REAL
 *              | CHAR
 *              | STRING
 *              | IDENTIFIER
 *              | '(' expr ')'
 */

static struct s__lang_node *
expr_primary(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	node = NULL;
	if (match(parser, S__LANG_LEXER_INT) ||
	    match(parser, S__LANG_LEXER_UINT) ||
	    match(parser, S__LANG_LEXER_REAL) ||
	    match(parser, S__LANG_LEXER_CHAR) ||
	    match(parser, S__LANG_LEXER_STRING)) {
		MKN(parser, node, S__LANG_NODE_EXPR_LITERAL);
		forward(parser);
	}
	else if (match(parser, S__LANG_LEXER_IDENTIFIER)) {
		MKN(parser, node, S__LANG_NODE_EXPR_IDENTIFIER);
		forward(parser);
	}
	else if (match(parser, S__LANG_LEXER_OPERATOR_OPEN_PARENTH)) {
		forward(parser);
		if (!(node = expr(parser))) {
			TRACE(parser, "missing expression after '('", "");
			return NULL;
		}
		if (!match(parser, S__LANG_LEXER_OPERATOR_CLOSE_PARENTH)) {
			TRACE(parser, "missing ')'", "");
			return NULL;
		}
		forward(parser);
	}
	return node;
}

/**
 * (E2)
 *
 * expr_postfix_ : '++' expr_postfix_
 *               | '--' expr_postfix_
 *               | '(' expr_args ')' expr_postfix_
 *               | '[' expr ']' expr_postfix_
 *               | '.' IDENTIFIER expr_postfix_
 *               | '->' IDENTIFIER expr_postfix_
 *               | <e>
 *
 * expr_postfix : expr_primary expr_postfix_
 */

static struct s__lang_node *
expr_postfix_(struct s__lang_parser *parser, struct s__lang_node *left)
{
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_INC)) {
			MKN(parser, node, S__LANG_NODE_EXPR_INC);
			node->left = left;
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_DEC)) {
			MKN(parser, node, S__LANG_NODE_EXPR_DEC);
			node->left = left;
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_OPEN_PARENTH)) {
			MKN(parser, node, S__LANG_NODE_EXPR_FUNCTION);
			node->left = left;
			forward(parser);
			node->right = expr_args(parser);
			if (!match(parser,
				   S__LANG_LEXER_OPERATOR_CLOSE_PARENTH)) {
				TRACE(parser, "missing ']'", "");
				return NULL;
			}
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_OPEN_BRACKET)) {
			MKN(parser, node, S__LANG_NODE_EXPR_ARRAY);
			node->left = left;
			forward(parser);
			if (!(node->right = expr(parser))) {
				TRACE(parser, "missing '[]' expression", "");
				return NULL;
			}
			if (!match(parser,
				   S__LANG_LEXER_OPERATOR_CLOSE_BRACKET)) {
				TRACE(parser, "missing ']'", "");
				return NULL;
			}
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_DOT)) {
			MKN(parser, node, S__LANG_NODE_EXPR_REF);
			node->left = left;
			forward(parser);
			if (!match(parser, S__LANG_LEXER_IDENTIFIER)) {
				TRACE(parser, "missing identifier", "");
				return NULL;
			}
			node->token = next(parser);
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_POINTER)) {
			MKN(parser, node, S__LANG_NODE_EXPR_PREF);
			node->left = left;
			forward(parser);
			if (!match(parser, S__LANG_LEXER_IDENTIFIER)) {
				TRACE(parser, "missing identifier", "");
				return NULL;
			}
			node->token = next(parser);
			forward(parser);
		}
		else {
			break;
		}
		if (!(node = expr_postfix_(parser, node))) {
			S__TRACE(0);
			return NULL;
		}
	}
	return node;
}

static struct s__lang_node *
expr_postfix(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_primary(parser))) {
		return NULL;
	}
	return expr_postfix_(parser, node);
}

/**
 * (E3)
 *
 * expr_unary : [ '+' '-' '~' '!' '&' '*' ] expr_cast
 *            | [ '++' '--' ] expr_unary
 *            | SIZEOF expr_unary
 *            | SIZEOF '(' type_name ')' <<< NOT IMPLEMENTED >>>
 *            | exp_postfix
 */

static struct s__lang_node *
expr_unary(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	node = NULL;
	if (match(parser, S__LANG_LEXER_OPERATOR_ADD)) {
		forward(parser);
		if (!(node = expr_cast(parser))) {
			TRACE(parser, "missing unary '+' operand", "");
			return NULL;
		}
	}
	else if (match(parser, S__LANG_LEXER_OPERATOR_SUB)) {
		MKN(parser, node, S__LANG_NODE_EXPR_NEG);
		forward(parser);
		if (!(node->right = expr_cast(parser))) {
			TRACE(parser, "missing unary '-' operand", "");
			return NULL;
		}
	}
	else if (match(parser, S__LANG_LEXER_OPERATOR_NOT)) {
		MKN(parser, node, S__LANG_NODE_EXPR_NOT);
		forward(parser);
		if (!(node->right = expr_cast(parser))) {
			TRACE(parser, "missing '~' operand", "");
			return NULL;
		}
	}
	else if (match(parser, S__LANG_LEXER_OPERATOR_LOGIC_NOT)) {
		MKN(parser, node, S__LANG_NODE_EXPR_LOGIC_NOT);
		forward(parser);
		if (!(node->right = expr_cast(parser))) {
			TRACE(parser, "missing '!' operand", "");
			return NULL;
		}
	}
	else if (match(parser, S__LANG_LEXER_OPERATOR_AND)) {
		MKN(parser, node, S__LANG_NODE_EXPR_ADDRESS);
		forward(parser);
		if (!(node->right = expr_cast(parser))) {
			TRACE(parser, "missing '&' operand", "");
			return NULL;
		}
	}
	else if (match(parser, S__LANG_LEXER_OPERATOR_MUL)) {
		MKN(parser, node, S__LANG_NODE_EXPR_DEREF);
		forward(parser);
		if (!(node->right = expr_cast(parser))) {
			TRACE(parser, "missing '*' operand", "");
			return NULL;
		}
	}
	else if (match(parser, S__LANG_LEXER_OPERATOR_INC)) {
		MKN(parser, node, S__LANG_NODE_EXPR_INC);
		forward(parser);
		if (!(node->right = expr_unary(parser))) {
			TRACE(parser, "missing '++' operand", "");
			return NULL;
		}
	}
	else if (match(parser, S__LANG_LEXER_OPERATOR_DEC)) {
		MKN(parser, node, S__LANG_NODE_EXPR_DEC);
		forward(parser);
		if (!(node->right = expr_unary(parser))) {
			TRACE(parser, "missing '--' operand", "");
			return NULL;
		}
	}
	else if (match(parser, S__LANG_LEXER_KEYWORD_SIZEOF)) {
		MKN(parser, node, S__LANG_NODE_EXPR_SIZEOF);
		forward(parser);
		if (!(node->right = expr_unary(parser))) {
			TRACE(parser, "missing '*' operand", "");
			return NULL;
		}
	}
	else {
		node = expr_postfix(parser);
	}
	return node;
}

/**
 * (E4)
 *
 * expr_cast : '(' type_name ')' expr_cast
 *           | expr_unary
 */

static struct s__lang_node *
expr_cast(struct s__lang_parser *parser)
{
	struct s__lang_node *node;
	uint64_t checkpoint;

	node = NULL;
	if (match(parser, S__LANG_LEXER_OPERATOR_OPEN_PARENTH)) {
		checkpoint = parser->i;
		MKN(parser, node, S__LANG_NODE_EXPR_CAST);
		forward(parser);
		if (!(node->left = type_name(parser))) {
			parser->i = checkpoint;
			return NULL;
		}
		if (!match(parser, S__LANG_LEXER_OPERATOR_CLOSE_PARENTH)) {
			TRACE(parser, "missing ')'", "");
			return NULL;
		}
		forward(parser);
		if (!(node->right = expr_cast(parser))) {
			TRACE(parser, "missing cast operand", "");
			return NULL;
		}
	}
	else {
		node = expr_unary(parser);
	}
	return node;
}

/**
 * (E5)
 *
 * expr_multiplicative_ : { [ '*' '/' '%' ] expr_cast expr_multiplicative_ }
 *                      | <e>
 *
 * expr_multiplicative : expr_cast expr_multiplicative_
 */

static struct s__lang_node *
expr_multiplicative_(struct s__lang_parser *parser,
		     struct s__lang_node *left)
{
	const char * const TBL[] = { "*", "/", "%" };
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_MUL)) {
			MKN(parser, node, S__LANG_NODE_EXPR_MUL);
			node->left = left;
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_DIV)) {
			MKN(parser, node, S__LANG_NODE_EXPR_DIV);
			node->left = left;
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_MOD)) {
			MKN(parser, node, S__LANG_NODE_EXPR_MOD);
			node->left = left;
			forward(parser);
		}
		else {
			break;
		}
		if (!(node->right = expr_cast(parser))) {
			TRACE(parser,
			      "missing '%s' operand",
			      TBL[node->op - S__LANG_NODE_EXPR_MUL]);
			return NULL;
		}
		if (!(node = expr_multiplicative_(parser, node))) {
			S__TRACE(0);
			return NULL;
		}
	}
	return node;
}

static struct s__lang_node *
expr_multiplicative(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_cast(parser))) {
		return NULL;
	}
	return expr_multiplicative_(parser, node);
}

/**
 * (E6)
 *
 * expr_additive_ : { [ '+' '-' ] expr_multiplicative expr_additive_ }
 *                | <e>
 *
 * expr_additive : expr_multiplicative expr_additive_
 */

static struct s__lang_node *
expr_additive_(struct s__lang_parser *parser, struct s__lang_node *left)
{
	const char * const TBL[] = { "+", "-" };
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_ADD)) {
			MKN(parser, node, S__LANG_NODE_EXPR_ADD);
			node->left = left;
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_SUB)) {
			MKN(parser, node, S__LANG_NODE_EXPR_SUB);
			node->left = left;
			forward(parser);
		}
		else {
			break;
		}
		if (!(node->right = expr_multiplicative(parser))) {
			TRACE(parser,
			      "missing '%s' operand",
			      TBL[node->op - S__LANG_NODE_EXPR_ADD]);
			return NULL;
		}
		if (!(node = expr_additive_(parser, node))) {
			S__TRACE(0);
			return NULL;
		}
	}
	return node;
}

static struct s__lang_node *
expr_additive(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_multiplicative(parser))) {
		return NULL;
	}
	return expr_additive_(parser, node);
}

/**
 * (E7)
 *
 * expr_shift_ : { [ '<<' '>>' ] expr_additive expr_shift_ }
 *             | <e>
 *
 * expr_shift : expr_additive expr_shift_
 */

static struct s__lang_node *
expr_shift_(struct s__lang_parser *parser, struct s__lang_node *left)
{
	const char * const TBL[] = { "<<", ">>" };
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_SHL)) {
			MKN(parser, node, S__LANG_NODE_EXPR_SHL);
			node->left = left;
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_SHR)) {
			MKN(parser, node, S__LANG_NODE_EXPR_SHR);
			node->left = left;
			forward(parser);
		}
		else {
			break;
		}
		if (!(node->right = expr_additive(parser))) {
			TRACE(parser,
			      "missing '%s' operand",
			      TBL[node->op - S__LANG_NODE_EXPR_SHL]);
			return NULL;
		}
		if (!(node = expr_shift_(parser, node))) {
			S__TRACE(0);
			return NULL;
		}
	}
	return node;
}

static struct s__lang_node *
expr_shift(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_additive(parser))) {
		return NULL;
	}
	return expr_shift_(parser, node);
}

/**
 * (E8)
 *
 * expr_relational_ : { [ '<' '>' '<=' '>=' ] expr_shift expr_relational_ }
 *                  | <e>
 *
 * expr_relational : expr_shift expr_relational_
 */

static struct s__lang_node *
expr_relational_(struct s__lang_parser *parser,
		 struct s__lang_node *left)
{
	const char * const TBL[] = { "<", ">", "<=", ">=" };
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_LT)) {
			MKN(parser, node, S__LANG_NODE_EXPR_LT);
			node->left = left;
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_GT)) {
			MKN(parser, node, S__LANG_NODE_EXPR_GT);
			node->left = left;
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_LE)) {
			MKN(parser, node, S__LANG_NODE_EXPR_LE);
			node->left = left;
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_GE)) {
			MKN(parser, node, S__LANG_NODE_EXPR_GE);
			node->left = left;
			forward(parser);
		}
		else {
			break;
		}
		if (!(node->right = expr_shift(parser))) {
			TRACE(parser,
			      "missing '%s' operand",
			      TBL[node->op - S__LANG_NODE_EXPR_LT]);
			return NULL;
		}
		if (!(node = expr_relational_(parser, node))) {
			S__TRACE(0);
			return NULL;
		}
	}
	return node;
}

static struct s__lang_node *
expr_relational(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_shift(parser))) {
		return NULL;
	}
	return expr_relational_(parser, node);
}

/**
 * (E9)
 *
 * expr_equality_ : { [ '==' '!=' ] expr_relational expr_equality_ }
 *                | <e>
 *
 * expr_equality : expr_relational expr_equality_
 */

static struct s__lang_node *
expr_equality_(struct s__lang_parser *parser, struct s__lang_node *left)
{
	const char * const TBL[] = { "==", "!=" };
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_EQ)) {
			MKN(parser, node, S__LANG_NODE_EXPR_EQ);
			node->left = left;
			forward(parser);
		}
		else if (match(parser, S__LANG_LEXER_OPERATOR_NE)) {
			MKN(parser, node, S__LANG_NODE_EXPR_NE);
			node->left = left;
			forward(parser);
		}
		else {
			break;
		}
		if (!(node->right = expr_relational(parser))) {
			TRACE(parser,
			      "missing '%s' operand",
			      TBL[node->op - S__LANG_NODE_EXPR_EQ]);
			return NULL;
		}
		if (!(node = expr_equality_(parser, node))) {
			S__TRACE(0);
			return NULL;
		}
	}
	return node;
}

static struct s__lang_node *
expr_equality(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_relational(parser))) {
		return NULL;
	}
	return expr_equality_(parser, node);
}

/**
 * (E10)
 *
 * expr_and_ : { '&' expr_equality expr_and_ }
 *           | <e>
 *
 * expr_and : expr_equality expr_and_
 */

static struct s__lang_node *
expr_and_(struct s__lang_parser *parser, struct s__lang_node *left)
{
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_AND)) {
			MKN(parser, node, S__LANG_NODE_EXPR_AND);
			node->left = left;
			forward(parser);
			if (!(node->right = expr_equality(parser))) {
				TRACE(parser, "missing '&' operand", "");
				return NULL;
			}
			if (!(node = expr_and_(parser, node))) {
				S__TRACE(0);
				return NULL;
			}
		}
		else {
			break;
		}
	}
	return node;
}

static struct s__lang_node *
expr_and(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_equality(parser))) {
		return NULL;
	}
	return expr_and_(parser, node);
}

/**
 * (E11)
 *
 * expr_xor_ : { '^' expr_and expr_xor_ }
 *           | <e>
 *
 * expr_xor : expr_and expr_xor_
 */

static struct s__lang_node *
expr_xor_(struct s__lang_parser *parser, struct s__lang_node *left)
{
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_XOR)) {
			MKN(parser, node, S__LANG_NODE_EXPR_XOR);
			node->left = left;
			forward(parser);
			if (!(node->right = expr_and(parser))) {
				TRACE(parser, "missing '^' operand", "");
				return NULL;
			}
			if (!(node = expr_xor_(parser, node))) {
				S__TRACE(0);
				return NULL;
			}
		}
		else {
			break;
		}
	}
	return node;
}

static struct s__lang_node *
expr_xor(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_and(parser))) {
		return NULL;
	}
	return expr_xor_(parser, node);
}

/**
 * (E12)
 *
 * expr_or_ : { '|' expr_xor expr_or_ }
 *          | <e>
 *
 * expr_or : expr_xor expr_or_
 */

static struct s__lang_node *
expr_or_(struct s__lang_parser *parser, struct s__lang_node *left)
{
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_OR)) {
			MKN(parser, node, S__LANG_NODE_EXPR_OR);
			node->left = left;
			forward(parser);
			if (!(node->right = expr_xor(parser))) {
				TRACE(parser, "missing '|' operand", "");
				return NULL;
			}
			if (!(node = expr_or_(parser, node))) {
				S__TRACE(0);
				return NULL;
			}
		}
		else {
			break;
		}
	}
	return node;
}

static struct s__lang_node *
expr_or(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_xor(parser))) {
		return NULL;
	}
	return expr_or_(parser, node);
}

/**
 * (E13)
 *
 * expr_logic_and_ : { '&&' expr_or expr_logic_and_ }
 *                 | <e>
 *
 * expr_logic_and : expr_or expr_logic_and_
 */

static struct s__lang_node *
expr_logic_and_(struct s__lang_parser *parser, struct s__lang_node *left)
{
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_LOGIC_AND)) {
			MKN(parser, node, S__LANG_NODE_EXPR_LOGIC_AND);
			node->left = left;
			forward(parser);
			if (!(node->right = expr_or(parser))) {
				TRACE(parser, "missing '&&' operand", "");
				return NULL;
			}
			if (!(node = expr_logic_and_(parser, node))) {
				S__TRACE(0);
				return NULL;
			}
		}
		else {
			break;
		}
	}
	return node;
}

static struct s__lang_node *
expr_logic_and(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_or(parser))) {
		return NULL;
	}
	return expr_logic_and_(parser, node);
}

/**
 * (E14)
 *
 * expr_logic_or_ : { '||' expr_logic_and expr_logic_or_ }
 *                | <e>
 *
 * expr_logic_or : expr_logic_and expr_logic_or_
 */

static struct s__lang_node *
expr_logic_or_(struct s__lang_parser *parser, struct s__lang_node *left)
{
	struct s__lang_node *node;

	node = left;
	for (;;) {
		if (match(parser, S__LANG_LEXER_OPERATOR_LOGIC_OR)) {
			MKN(parser, node, S__LANG_NODE_EXPR_LOGIC_OR);
			node->left = left;
			forward(parser);
		}
		else {
			break;
		}
		if (!(node->right = expr_logic_and(parser))) {
			TRACE(parser, "missing '||' operand", "");
			return NULL;
		}
		if (!(node = expr_logic_or_(parser, node))) {
			S__TRACE(0);
			return NULL;
		}
	}
	return node;
}

static struct s__lang_node *
expr_logic_or(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr_logic_and(parser))) {
		return NULL;
	}
	return expr_logic_or_(parser, node);
}

/**
 * (E15)
 *
 * expr : expr_logic_or { '?' expr ':' expr }?
 */

static struct s__lang_node *
expr(struct s__lang_parser *parser)
{
	struct s__lang_node *node, *node_;

	assert( parser );

	if (!(node = expr_logic_or(parser))) {
		return NULL;
	}
	if (match(parser, S__LANG_LEXER_OPERATOR_QUESTION)) {
		MKN(parser, node_, S__LANG_NODE_EXPR_COND);
		node_->cond = node;
		forward(parser);
		if (!(node_->left = expr(parser))) {
			TRACE(parser, "missing '?' operand", "");
			return NULL;
		}
		if (!match(parser, S__LANG_LEXER_OPERATOR_COLON)) {
			TRACE(parser, "missing ':'", "");
			return NULL;
		}
		forward(parser);
		if (!(node_->right = expr(parser))) {
			TRACE(parser, "missing ':' operand", "");
			return NULL;
		}
		node = node_;
	}
	return node;
}

static struct s__lang_node *
type_name(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	node = NULL;
	if (match(parser, S__LANG_LEXER_KEYWORD_INT)) {
		MKN(parser, node, S__LANG_NODE_);
		forward(parser);
	}
	return node; /* FIX : IMPLEMENT */
}

/**============================================================================
 * (--)  top
 *===========================================================================*/

static struct s__lang_node *
top(struct s__lang_parser *parser)
{
	struct s__lang_node *node;

	if (!(node = expr(parser))) {
		TRACE(parser, "syntax error", "");
		return NULL;
	}
	if (!match(parser, S__LANG_LEXER_EOF)) {
		TRACE(parser, "syntax error", "");
		return NULL;
	}
	return node;
}

s__lang_parser_t
s__lang_parser_open(s__lang_lexer_t lexer)
{
	struct s__lang_parser *parser;

	assert( lexer );

	if (!(parser = s__malloc(sizeof (struct s__lang_parser)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(parser, 0, sizeof (struct s__lang_parser));
	parser->lexer = lexer;
	if (!(parser->node = s__lang_node_open()) ||
	    !(parser->n = s__lang_lexer_size(parser->lexer)) ||
	    !(parser->root = top(parser))) {
		s__lang_parser_close(parser);
		S__TRACE(0);
		return NULL;
	}
	return parser;
}

void
s__lang_parser_close(s__lang_parser_t parser)
{
	if (parser) {
		s__lang_node_close(parser->node);
		memset(parser, 0, sizeof (struct s__lang_parser));
	}
	S__FREE(parser);
}

struct s__lang_node *
s__lang_parser_root(s__lang_parser_t parser)
{
	assert( parser );

	return parser->root;
}

const char *
s__lang_parser_pathname(s__lang_parser_t parser)
{
	assert( parser );

	return s__lang_lexer_pathname(parser->lexer);
}
