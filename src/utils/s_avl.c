/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_avl.c
 */

#include "s_avl.h"

struct s__avl {
	uint64_t size;
	struct node {
		int depth;
		const char *key;
		const void *val;
		struct node *left;
		struct node *right;
	} *root;
};

static int
delta(const struct node *node)
{
	return node ? node->depth : -1;
}

static int
balance(const struct node *node)
{
	return delta(node->left) - delta(node->right);
}

static int
depth(const struct node *a, const struct node *b)
{
	return (delta(a) > delta(b)) ? (delta(a) + 1) : (delta(b) + 1);
}

static struct node *
rotate_right(struct node *node)
{
	struct node *root;

	root = node->left;
	node->left = root->right;
	root->right = node;
	node->depth = depth(node->left, node->right);
	root->depth = depth(root->left, node);
	return root;
}

static struct node *
rotate_left(struct node *node)
{
	struct node *root;

	root = node->right;
	node->right = root->left;
	root->left = node;
	node->depth = depth(node->left, node->right);
	root->depth = depth(root->right, node);
	return root;
}

static struct node *
rotate_left_right(struct node *node)
{
	node->left = rotate_left(node->left);
	return rotate_right(node);
}

static struct node *
rotate_right_left(struct node *node)
{
	node->right = rotate_right(node->right);
	return rotate_left(node);
}

static struct node *
min(struct node *node)
{
	struct node *node_;

	node_ = node;
	while (node) {
		node_ = node;
		node = node->left;
	}
	return node_;
}

static void
destroy_(struct node *root)
{
	if (root) {
		destroy_(root->left);
		destroy_(root->right);
		S__FREE(root->key);
		memset(root, 0, sizeof (struct node));
	}
	S__FREE(root);
}

static struct node *
remove_(struct node *root, const char *key, int *found)
{
	struct node *node, temp;
	int d;

	if (root) {
		if (!(d = strcmp(key, root->key))) {
			if (!root->left || !root->right) {
				node = root->left ? root->left : root->right;
				if (!node) {
					destroy_(root);
					root = NULL;
				}
				else {
					temp = *root;
					(*root) = (*node);
					(*node) = temp;
					node->left = NULL;
					node->right = NULL;
					destroy_(node);
				}
			}
			else {
				node = min(root->right);
				S__FREE(root->key);
				if (!(root->key = s__strdup(node->key))) {
					S__HALT(0);
					return NULL;
				}
				root->val = node->val;
				node->val = NULL;
				root->right = remove_(root->right,
						      node->key,
						      found);
			}
			(*found) = 1;
		}
		else if (0 > d) {
			root->left = remove_(root->left, key, found);
		}
		else if (0 < d) {
			root->right = remove_(root->right, key, found);
		}
	}
	if (root) {
		root->depth = depth(root->left, root->right);
		if ((1 < balance(root)) && (0 <= balance(root->left))) {
			root = rotate_right(root);
		}
		else if ((1 < balance(root)) && (0 > balance(root->left))) {
			root = rotate_left_right(root);
		}
		else if ((-1 > balance(root)) && (0 >= balance(root->right))) {
			root = rotate_left(root);
		}
		else if ((-1 > balance(root)) && (0 < balance(root->right))) {
			root = rotate_right_left(root);
		}
	}
	return root;
}

static struct node *
update_(struct s__avl *avl,
	struct node *root,
	const char *key,
	const void *val)
{
	int d;

	if (!root) {
		if (!(root = s__malloc(sizeof (struct node)))) {
			S__TRACE(0);
			return NULL;
		}
		memset(root, 0, sizeof (struct node));
		if (!(root->key = s__strdup(key))) {
			S__TRACE(0);
			return NULL;
		}
		root->val = val;
		++avl->size;
		return root;
	}
	if (!(d = strcmp(key, root->key))) {
		root->val = val;
	}
	else if (0 > d) {
		root->left = update_(avl, root->left, key, val);
		if (1 < abs(balance(root))) {
			if (0 > strcmp(key, root->left->key)) {
				root = rotate_right(root);
			}
			else {
				root = rotate_left_right(root);
			}
		}
	}
	else if (0 < d) {
		root->right = update_(avl, root->right, key, val);
		if (1 < abs(balance(root))) {
			if (0 < strcmp(key, root->right->key)) {
				root = rotate_left(root);
			}
			else {
				root = rotate_right_left(root);
			}
		}
	}
	root->depth = depth(root->left, root->right);
	return root;
}

static int
iterate_(struct node *root, s__avl_fnc_t fnc, void *ctx)
{
	int e;

	if (root) {
		if ((e = iterate_(root->left, fnc, ctx)) ||
		    (e = fnc(ctx, root->key, (void *)root->val)) ||
		    (e = iterate_(root->right, fnc, ctx))) {
			return e;
		}
	}
	return 0;
}

s__avl_t
s__avl_open(void)
{
	struct s__avl *avl;

	if (!(avl = s__malloc(sizeof (struct s__avl)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(avl, 0, sizeof (struct s__avl));
	return avl;
}

void
s__avl_close(s__avl_t avl)
{
	if (avl) {
		destroy_(avl->root);
		memset(avl, 0, sizeof (struct s__avl));
	}
	S__FREE(avl);
}

void
s__avl_empty(s__avl_t avl)
{
	assert( avl );

	destroy_(avl->root);
	memset(avl, 0, sizeof (struct s__avl));
}

void
s__avl_remove(s__avl_t avl, const char *key)
{
	int found;

	assert( avl );
	assert( s__strlen(key) );

	found = 0;
	avl->root = remove_(avl->root, key, &found);
	avl->size -= found ? 1 : 0;
}

int
s__avl_update(s__avl_t avl, const char *key, const void *val)
{
	struct node *root;

	assert( avl );
	assert( s__strlen(key) );

	if (!(root = update_(avl, avl->root, key, val))) {
		S__TRACE(0);
		return -1;
	}
	avl->root = root;
	return 0;
}

void *
s__avl_lookup(s__avl_t avl, const char *key)
{
	const struct node *node;
	int d;

	assert( avl );
	assert( s__strlen(key) );

	node = avl->root;
	while (node) {
		if (!(d = strcmp(key, node->key))) {
			return (void *)node->val;
		}
		node = (0 > d) ? node->left : node->right;
	}
	return NULL;
}

int
s__avl_iterate(s__avl_t avl, s__avl_fnc_t fnc, void *ctx)
{
	int e;

	assert( avl );
	assert( fnc );

	if ((e = iterate_(avl->root, fnc, ctx))) {
		return e;
	}
	return 0;
}

uint64_t
s__avl_size(s__avl_t avl)
{
	assert( avl );

	return avl->size;
}

int
s__avl_bist(void)
{
	char key[32], val[32];
	const void *val_;
	s__avl_t avl;
	int i, n;

	n = 123456;
	if (!(avl = s__avl_open())) {
		S__TRACE(0);
		return -1;
	}
	if (0 != s__avl_size(avl)) {
		s__avl_close(avl);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}

	/* single item */

	if (s__avl_update(avl, "key", "val") ||
	    (1 != s__avl_size(avl)) ||
	    strcmp("val",
		   s__avl_lookup(avl, "key") ?
		   s__avl_lookup(avl, "key") : "") ||
	    s__avl_update(avl, "key", "lav") ||
	    (1 != s__avl_size(avl)) ||
	    strcmp("lav",
		   s__avl_lookup(avl, "key") ?
		   s__avl_lookup(avl, "key") : "")) {
		s__avl_close(avl);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}
	s__avl_remove(avl, "key");
	if (0 != s__avl_size(avl)) {
		s__avl_close(avl);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}

	/* random update */

	srand(10);
	for (i=0; i<n; ++i) {
		s__sprintf(key, sizeof (key), "%d-%d", rand(), i);
		s__sprintf(val, sizeof (val), "%d-%d", rand(), i);
		if (s__avl_update(avl, key, val)) {
			s__avl_close(avl);
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
		val_ = s__avl_lookup(avl, key);
		if (!val_ || strcmp(val_, val)) {
			s__avl_close(avl);
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}
	if (i != (int)s__avl_size(avl)) {
		s__avl_close(avl);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}

	/* random lookup */

	srand(10);
	for (i=0; i<n; ++i) {
		s__sprintf(key, sizeof (key), "%d-%d", rand(), i);
		s__sprintf(val, sizeof (val), "%d-%d", rand(), i);
		val_ = s__avl_lookup(avl, key);
		if (!val_ || strcmp(val_, val)) {
			s__avl_close(avl);
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}

	/* random remove */

	srand(10);
	for (i=0; i<(n/2); ++i) {
		s__sprintf(key, sizeof (key), "%d-%d", rand(), i);
		s__sprintf(val, sizeof (val), "%d-%d", rand(), i);
		s__avl_remove(avl, key);
		if (s__avl_lookup(avl, key)) {
			s__avl_close(avl);
			S__TRACE(S__ERR_SOFTWARE);
			return -1;
		}
	}
	if (i != (int)s__avl_size(avl)) {
		s__avl_close(avl);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}

	/* random lookup */

	srand(10);
	for (i=0; i<n; ++i) {
		s__sprintf(key, sizeof (key), "%d-%d", rand(), i);
		s__sprintf(val, sizeof (val), "%d-%d", rand(), i);
		val_ = s__avl_lookup(avl, key);
		if (i < (n / 2)) {
			if (val_) {
				s__avl_close(avl);
				S__TRACE(S__ERR_SOFTWARE);
				return -1;
			}
		}
		else {
			if (!val_ || strcmp(val_, val)) {
				s__avl_close(avl);
				S__TRACE(S__ERR_SOFTWARE);
				return -1;
			}
		}
	}

	/* empty */

	s__avl_empty(avl);
	if (0 != s__avl_size(avl)) {
		s__avl_close(avl);
		S__TRACE(S__ERR_SOFTWARE);
		return -1;
	}

	/* done */

	s__avl_close(avl);
	return 0;
}
