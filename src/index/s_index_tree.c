/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index_tree.c
 */

#include "s_index_queue.h"
#include "s_index_tree.h"

#pragma pack(push, 1)
struct node {
	int depth;
	uint64_t record;
	struct node *left;
	struct node *right;
};
#pragma pack(pop)

struct s__index_tree {
	void *chunk;
	uint64_t size;
	/*-*/
	void *root;
	uint64_t items;
};

static int
check(struct s__index_tree *tree, uint64_t n)
{
	const uint64_t CHUNK_SIZE = 1048576;
	void *chunk;

	if (!tree->chunk || (CHUNK_SIZE < (tree->size + n))) {
		if (!(chunk = s__malloc(CHUNK_SIZE))) {
			S__TRACE(0);
			return -1;
		}
		(*((void **)chunk)) = tree->chunk; /* link */
		tree->size = sizeof (void *);
		tree->chunk = chunk;
	}
	return 0;
}

static struct node *
get_node(struct s__index_tree *tree, uint64_t i)
{
	return (struct node *)((char *)tree->chunk + i);
}

static const char *
get_key(const struct node *node)
{
	return (const char *)(node + 1);
}

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
update(struct s__index_tree *tree,
       struct node *root,
       const char *key,
       uint64_t **record)
{
	int d;

	if (!root) {
		root = get_node(tree, tree->size);
		memset(root, 0, sizeof (struct node));
		memcpy(root + 1, key, s__strlen(key) + 1);
		tree->size += sizeof (struct node) + s__strlen(key) + 1;
		tree->items += 1;
		(*record) = &root->record;
		return root;
	}
	if (!(d = strcmp(key, get_key(root)))) {
		(*record) = &root->record;
	}
	else if (0 > d) {
		root->left = update(tree, root->left, key, record);
		if (1 < abs(balance(root))) {
			if (0 > strcmp(key, get_key(root->left))) {
				root = rotate_right(root);
			}
			else {
				root = rotate_left_right(root);
			}
		}
	}
	else if (0 < d) {
		root->right = update(tree, root->right, key, record);
		if (1 < abs(balance(root))) {
			if (0 < strcmp(key, get_key(root->right))) {
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

static struct node *
min(struct node *root)
{
	while (root->left) {
		root = root->left;
	}
	return root;
}

static struct node *
max(struct node *root)
{
	while (root->right) {
		root = root->right;
	}
	return root;
}

static struct node *
next(struct node *root, const char *key)
{
	struct node *node;
	int d;

	node = NULL;
	while (root) {
		if (!(d = strcmp(key, get_key(root)))) {
			if (root->right) {
				return min(root->right);
			}
			break;
		}
		else if (0 > d) {
			node = root;
			root = root->left;
		}
		else {
			root = root->right;
		}
	}
	return node;
}

static struct node *
prev(struct node *root, const void *key)
{
	struct node *node;
	int d;

	node = NULL;
	while (root) {
		if (!(d = strcmp(key, get_key(root)))) {
			if (root->left) {
				return max(root->left);
			}
			break;
		}
		else if (0 > d) {
			root = root->left;
		}
		else {
			node = root;
			root = root->right;
		}
	}
	return node;
}

int
s__index_tree_iterate(s__index_tree_t tree, s__index_tree_fnc_t fnc, void *ctx)
{
	struct node *node;
	s__index_queue_t queue;

	assert( tree );
	assert( fnc );

	if (tree->root) {
		if (!(queue = s__index_queue_open(tree->items))) {
			S__TRACE(0);
			return -1;
		}
		s__index_queue_push(queue, tree->root);
		while (!s__index_queue_empty(queue)) {
			node = s__index_queue_pop(queue);
			if (fnc(ctx, get_key(node), node->record)) {
				s__index_queue_close(queue);
				S__TRACE(0);
				return -1;
			}
			if (node->left) {
				s__index_queue_push(queue, node->left);
			}
			if (node->right) {
				s__index_queue_push(queue, node->right);
			}
		}
		s__index_queue_close(queue);
	}
	return 0;
}

s__index_tree_t
s__index_tree_open(void)
{
	struct s__index_tree *tree;

	if (!(tree = s__malloc(sizeof (struct s__index_tree)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(tree, 0, sizeof (struct s__index_tree));
	return tree;
}

void
s__index_tree_close(s__index_tree_t tree)
{
	void *chunk;

	if (tree) {
		while ((chunk = tree->chunk)) {
			tree->chunk = (*((void **)chunk));
			S__FREE(chunk);
		}
		memset(tree, 0, sizeof (struct s__index_tree));
	}
	S__FREE(tree);
}

void
s__index_tree_truncate(s__index_tree_t tree)
{
	void *chunk;

	if (tree) {
		while ((chunk = tree->chunk)) {
			tree->chunk = (*((void **)chunk));
			S__FREE(chunk);
		}
		memset(tree, 0, sizeof (struct s__index_tree));
	}
}

uint64_t *
s__index_tree_update(s__index_tree_t tree, const char *key)
{
	uint64_t *record;

	assert( tree );
	assert( s__strlen(key) );
	assert( S__INDEX_TREE_MAX_KEY_LEN > s__strlen(key) );

	if (check(tree, sizeof (struct node) + s__strlen(key) + 1)) {
		S__TRACE(0);
		return NULL;
	}
	tree->root = update(tree, tree->root, key, &record);
	return record;
}

uint64_t *
s__index_tree_find(s__index_tree_t tree, const char *key)
{
	struct node *node;
	int d;

	assert( tree );
	assert( s__strlen(key) );

	node = tree->root;
	while (node) {
		if (!(d = strcmp(key, get_key(node)))) {
			return &node->record;
		}
		node = (0 > d) ? node->left : node->right;
	}
	return NULL;
}

uint64_t *
s__index_tree_next(s__index_tree_t tree, const char *key, char *okey)
{
	struct node *node;

	assert( tree );
	assert( okey );

	if (s__strlen(key)) {
		if ((node = next(tree->root, key))) {
			memcpy(okey,
			       get_key(node),
			       s__strlen(get_key(node)) + 1);
			return &node->record;
		}
	}
	else if (tree->root) {
		if ((node = min(tree->root))) {
			memcpy(okey,
			       get_key(node),
			       s__strlen(get_key(node)) + 1);
			return &node->record;
		}
	}
	return NULL;
}

uint64_t *
s__index_tree_prev(s__index_tree_t tree, const char *key, char *okey)
{
	struct node *node;

	assert( tree );
	assert( okey );

	if (s__strlen(key)) {
		if ((node = prev(tree->root, key))) {
			memcpy(okey,
			       get_key(node),
			       s__strlen(get_key(node)) + 1);
			return &node->record;
		}
	}
	else if (tree->root) {
		if ((node = max(tree->root))) {
			memcpy(okey,
			       get_key(node),
			       s__strlen(get_key(node)) + 1);
			return &node->record;
		}
	}
	return NULL;
}

uint64_t
s__index_tree_items(s__index_tree_t tree)
{
	assert( tree );

	return tree->items;
}
