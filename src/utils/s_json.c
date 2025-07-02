/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_json.c
 */

#include "s_json.h"

#define TRACE(j,m)				\
	do {					\
		(j)->curr = "";			\
		(j)->token.op = OP_END;		\
		s__log("info: json:%u:%s",	\
		       (j)->lineno,		\
		       (m));			\
		S__TRACE(S__ERR_SYNTAX);	\
	}					\
	while (0)

#define MKN(j,n,t)				\
	do {					\
		if (!((n) = allocate((j)))) {	\
			S__TRACE(0);		\
			return NULL;		\
		}				\
		(n)->op = (t);			\
	}					\
	while (0)

struct s__json {
	char *curr;
	char *content;
	unsigned lineno;
	struct s__json_node *root;
	struct {
		enum {
			OP_END,
			OP_NULL,
			OP_BOOL,
			OP_COMMA,
			OP_COLON,
			OP_STRING,
			OP_NUMBER,
			OP_OPEN_BRACE,
			OP_CLOSE_BRACE,
			OP_OPEN_BRACKET,
			OP_CLOSE_BRACKET
		} op;
		union {
			int bool;
			double number;
			const char *string;
		} u;
	} token;
	void *chunk;
	uint64_t size;
};

static struct s__json_node *
allocate(struct s__json *json)
{
	const uint64_t CHUNK_SIZE = 4096;
	struct s__json_node *node;
	void *chunk;
	uint64_t size;

	size = sizeof (struct s__json_node);
	if (!json->chunk || (CHUNK_SIZE < (json->size + size))) {
		if (!(chunk = s__malloc(CHUNK_SIZE))) {
			S__TRACE(0);
			return NULL;
		}
		(*((void **)chunk)) = json->chunk;
		json->size = sizeof (struct s__json_node *);
		json->chunk = chunk;
	}
	node = (struct s__json_node *)((char *)json->chunk + json->size);
	json->size += size;
	memset(node, 0, size);
	return node;
}

static int
ishex(char c)
{
	c = tolower(c);
	if (isdigit((unsigned char)c)) {
		return 1;
	}
	if (('a' <= c) && ('f' >= c)) {
		return 1;
	}
	return 0;
}

static char *
eat_string(char *s)
{
	int i;

	while (*++s) {
		if ('\"' == (*s)) {
			return s;
		}
		else if ('\\' == (*s)) {
			++s;
			if (('"' == (*s)) ||
			    ('\\' == (*s)) ||
			    ('/' == (*s)) ||
			    ('b' == (*s)) ||
			    ('f' == (*s)) ||
			    ('n' == (*s)) ||
			    ('r' == (*s)) ||
			    ('t' == (*s))) {
				++s;
				continue;
			}
			else if ('u' == (*s)) {
				for (i=0; i<4; ++i) {
					if (!ishex(*++s)) {
						return NULL;
					}
				}
				continue;
			}
			else {
				return NULL;
			}
		}
		else if (('\r' == (*s)) || ('\n' == (*s))) {
			break;
		}
	}
	return NULL;
}

static void
forward(struct s__json *json)
{
	double number;
	char *s, *e;

	s = json->curr;
	while (*s) {
		if (isspace((unsigned char)(*s))) {
			if ('\n' == (*s)) {
				++json->lineno;
			}
			++s;
			continue;
		}
		if ('{' == (*s)) {
			json->curr = s + 1;
			json->token.op = OP_OPEN_BRACE;
			return;
		}
		if ('}' == (*s)) {
			json->curr = s + 1;
			json->token.op = OP_CLOSE_BRACE;
			return;
		}
		if ('[' == (*s)) {
			json->curr = s + 1;
			json->token.op = OP_OPEN_BRACKET;
			return;
		}
		if (']' == (*s)) {
			json->curr = s + 1;
			json->token.op = OP_CLOSE_BRACKET;
			return;
		}
		if (':' == (*s)) {
			json->curr = s + 1;
			json->token.op = OP_COLON;
			return;
		}
		if (',' == (*s)) {
			json->curr = s + 1;
			json->token.op = OP_COMMA;
			return;
		}
		if (!strncmp("null", s, 4)) {
			json->curr = s + 4;
			json->token.op = OP_NULL;
			return;
		}
		if (!strncmp("true", s, 4)) {
			json->curr = s + 4;
			json->token.op = OP_BOOL;
			json->token.u.bool = 1;
			return;
		}
		if (!strncmp("false", s, 5)) {
			json->curr = s + 5;
			json->token.op = OP_BOOL;
			json->token.u.bool = 0;
			return;
		}
		if ('\"' == (*s)) {
			if (!(e = eat_string(s))) {
				TRACE(json, "erroneous string");
				return;
			}
			(*e) = '\0';
			json->curr = e + 1;
			json->token.op = OP_STRING;
			json->token.u.string = s + 1;
			return;
		}
		else if (('-' == (*s)) ||
			 ('.' == (*s)) ||
			 isdigit((unsigned char)(*s))) {
			number = strtod(s, &e);
			if (e == s) {
				TRACE(json, "erroneous number");
				return;
			}
			json->curr = e;
			json->token.op = OP_NUMBER;
			json->token.u.number = number;
			return;
		}
		TRACE(json, "erroneous character");
		return;
	}
	json->token.op = OP_END;
}

static struct s__json_node *jarray(struct s__json *json);
static struct s__json_node *jobject(struct s__json *json);

static struct s__json_node *
jvalue(struct s__json *json)
{
	struct s__json_node *node;

	node = NULL;
	if (OP_NULL == json->token.op) {
		MKN(json, node, S__JSON_NODE_OP_NULL);
		forward(json);
	}
	else if (OP_BOOL == json->token.op) {
		MKN(json, node, S__JSON_NODE_OP_BOOL);
		node->u.bool = json->token.u.bool;
		forward(json);
	}
	else if (OP_NUMBER == json->token.op) {
		MKN(json, node, S__JSON_NODE_OP_NUMBER);
		node->u.number = json->token.u.number;
		forward(json);
	}
	else if (OP_STRING == json->token.op) {
		MKN(json, node, S__JSON_NODE_OP_STRING);
		node->u.string = json->token.u.string;
		forward(json);
	}
	else if (OP_OPEN_BRACKET == json->token.op) {
		if (!(node = jarray(json))) {
			TRACE(json, "erroneous array");
			return NULL;
		}
	}
	else if (OP_OPEN_BRACE == json->token.op) {
		if (!(node = jobject(json))) {
			TRACE(json, "erroneous object");
			return NULL;
		}
	}
	return node;
}

static struct s__json_node *
jarray(struct s__json *json)
{
	struct s__json_node *node, *head, *tail;
	int mark;

	mark = 0;
	head = tail = node = NULL;
	if (OP_OPEN_BRACKET == json->token.op) {
		forward(json);
		MKN(json, node, S__JSON_NODE_OP_ARRAY);
		head = tail = node;
		if (OP_CLOSE_BRACKET != json->token.op) {
			for (;;) {
				mark = 0;
				if (!(node->u.array.node = jvalue(json))) {
					TRACE(json, "erroneous value");
					return NULL;
				}
				if (OP_COMMA != json->token.op) {
					break;
				}
				forward(json);
				MKN(json, node, S__JSON_NODE_OP_ARRAY);
				tail->u.array.link = node;
				tail = node;
				mark = 1;
			}
			if (mark) {
				TRACE(json, "dangling ','");
				return NULL;
			}
		}
		if (OP_CLOSE_BRACKET != json->token.op) {
			TRACE(json, "missing ']'");
			return NULL;
		}
		forward(json);
	}
	return head;
}

static struct s__json_node *
jobject(struct s__json *json)
{
	struct s__json_node *node, *head, *tail;
	int mark;

	mark = 0;
	head = tail = node = NULL;
	if (OP_OPEN_BRACE == json->token.op) {
		forward(json);
		MKN(json, node, S__JSON_NODE_OP_OBJECT);
		head = tail = node;
		for (;;) {
			if (OP_STRING != json->token.op) {
				break;
			}
			mark = 0;
			node->u.object.key = json->token.u.string;
			forward(json);
			if (OP_COLON != json->token.op) {
				TRACE(json, "missing ':'");
				return NULL;
			}
			forward(json);
			if (!(node->u.object.node = jvalue(json))) {
				TRACE(json, "erroneous value");
				return NULL;
			}
			if (OP_COMMA != json->token.op) {
				break;
			}
			forward(json);
			MKN(json, node, S__JSON_NODE_OP_OBJECT);
			tail->u.object.link = node;
			tail = node;
			mark = 1;
		}
		if (mark) {
			TRACE(json, "dangling ','");
			return NULL;
		}
		if (OP_CLOSE_BRACE != json->token.op) {
			TRACE(json, "missing '}'");
			return NULL;
		}
		forward(json);
	}
	return head;
}

static struct s__json_node *
jtop(struct s__json *json)
{
	struct s__json_node *node;

	node = NULL;
	if (OP_OPEN_BRACKET == json->token.op) {
		if (!(node = jarray(json))) {
			TRACE(json, "erroneous object");
			return NULL;
		}
	}
	else if (OP_OPEN_BRACE == json->token.op) {
		if (!(node = jobject(json))) {
			TRACE(json, "erroneous object");
			return NULL;
		}
	}
	return node;
}

s__json_t
s__json_open(const char *s)
{
	struct s__json *json;

	assert( s );

	/* initialize */

	if (!(json = s__malloc(sizeof (struct s__json)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(json, 0, sizeof (struct s__json));
	if (!(json->content = s__strdup(s))) {
		s__json_close(json);
		S__TRACE(0);
		return NULL;
	}

	/* initialize */

	json->curr = json->content;
	json->lineno = 1;
	forward(json);

	/* parse */

	if (!(json->root = jtop(json))) {
		TRACE(json, "empty input");
		s__json_close(json);
		return NULL;
	}
	if (OP_END != json->token.op) {
		TRACE(json, "invalid trailing content");
		s__json_close(json);
		return NULL;
	}
	return json;
}

void
s__json_close(s__json_t json)
{
	void *chunk;

	if (json) {
		while ((chunk = json->chunk)) {
			json->chunk = (*((void **)chunk));
			S__FREE(chunk);
		}
		S__FREE(json->content);
		memset(json, 0, sizeof (struct s__json));
	}
	S__FREE(json);
}

const struct s__json_node *
s__json_root(s__json_t json)
{
	assert( json );

	return json->root;
}

int
s__json_bist(void)
{
	const struct s__json_node *node;
	s__json_t json;

	/* empty array */

	if (!(json = s__json_open("[]"))) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	if (!(node = s__json_root(json)) ||
	    (S__JSON_NODE_OP_ARRAY != node->op) ||
	    node->u.array.node ||
	    (node = node->u.array.link)) {
		s__json_close(json);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__json_close(json);

	/* empty object */

	if (!(json = s__json_open("{}"))) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	if (!(node = s__json_root(json)) ||
	    (S__JSON_NODE_OP_OBJECT != node->op) ||
	    node->u.object.key ||
	    node->u.object.node ||
	    (node = node->u.object.link)) {
		s__json_close(json);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__json_close(json);

	/* single-element array */

	if (!(json = s__json_open("[true]"))) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	if (!(node = s__json_root(json)) ||
	    (S__JSON_NODE_OP_ARRAY != node->op) ||
	    !node->u.array.node ||
	    (S__JSON_NODE_OP_BOOL != node->u.array.node->op) ||
	    (1 != node->u.array.node->u.bool) ||
	    (node = node->u.array.link)) {
		s__json_close(json);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__json_close(json);

	/* single-element object */

	if (!(json = s__json_open("{\"key\":false}"))) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	if (!(node = s__json_root(json)) ||
	    (S__JSON_NODE_OP_OBJECT != node->op) ||
	    !node->u.object.key ||
	    strcmp("key", node->u.object.key) ||
	    !node->u.object.node ||
	    (S__JSON_NODE_OP_BOOL != node->u.object.node->op) ||
	    (0 != node->u.object.node->u.bool) ||
	    (node = node->u.object.link)) {
		s__json_close(json);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__json_close(json);

	/* two-element array */

	if (!(json = s__json_open("[\"hello\",\"world\"]"))) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	if (!(node = s__json_root(json)) ||
	    (S__JSON_NODE_OP_ARRAY != node->op) ||
	    !node->u.array.node ||
	    (S__JSON_NODE_OP_STRING != node->u.array.node->op) ||
	    !node->u.array.node->u.string ||
	    strcmp("hello", node->u.array.node->u.string) ||
	    !(node = node->u.array.link) ||
	    (S__JSON_NODE_OP_ARRAY != node->op) ||
	    !node->u.array.node ||
	    (S__JSON_NODE_OP_STRING != node->u.array.node->op) ||
	    !node->u.array.node->u.string ||
	    strcmp("world", node->u.array.node->u.string) ||
	    (node = node->u.array.link)) {
		s__json_close(json);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__json_close(json);

	/* two-element object */

	if (!(json = s__json_open("{\"key1\":1,\"key2\":2}"))) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	if (!(node = s__json_root(json)) ||
	    (S__JSON_NODE_OP_OBJECT != node->op) ||
	    !node->u.object.node ||
	    !node->u.object.key ||
	    strcmp("key1", node->u.object.key) ||
	    (S__JSON_NODE_OP_NUMBER != node->u.object.node->op) ||
	    (1 != node->u.object.node->u.number) ||
	    !(node = node->u.object.link) ||
	    (S__JSON_NODE_OP_OBJECT != node->op) ||
	    !node->u.object.node ||
	    !node->u.object.key ||
	    strcmp("key2", node->u.object.key) ||
	    (S__JSON_NODE_OP_NUMBER != node->u.object.node->op) ||
	    (2 != node->u.object.node->u.number) ||
	    (node = node->u.object.link)) {
		s__json_close(json);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__json_close(json);

	/* nested */

	if (!(json = s__json_open("[[true,false],{\"key\":\"val\"}]"))) {
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	if (!(node = s__json_root(json)) ||
	    (S__JSON_NODE_OP_ARRAY != node->op) ||
	    !(node = node->u.array.node) ||
	    (S__JSON_NODE_OP_ARRAY != node->op) ||
	    (S__JSON_NODE_OP_BOOL != node->u.array.node->op) ||
	    (1 != node->u.array.node->u.bool) ||
	    !(node = node->u.array.link) ||
	    (S__JSON_NODE_OP_BOOL != node->u.array.node->op) ||
	    (0 != node->u.array.node->u.bool)) {
		s__json_close(json);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	if (!(node = s__json_root(json)) ||
	    (S__JSON_NODE_OP_ARRAY != node->op) ||
	    !(node = node->u.array.link) ||
	    (S__JSON_NODE_OP_ARRAY != node->op) ||
	    !(node = node->u.array.node) ||
	    (S__JSON_NODE_OP_OBJECT != node->op) ||
	    !node->u.object.key ||
	    strcmp("key", node->u.object.key) ||
	    !(node = node->u.object.node) ||
	    (S__JSON_NODE_OP_STRING != node->op) ||
	    strcmp("val", node->u.string)) {
		s__json_close(json);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__json_close(json);
	return 0;
}
