/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index_queue.c
 */

#include "s_index_queue.h"

struct s__index_queue {
	void **vals;
	uint64_t size;
	uint64_t head;
	uint64_t tail;
	uint64_t capacity;
};

s__index_queue_t
s__index_queue_open(uint64_t capacity)
{
	struct s__index_queue *queue;
	uint64_t n;

	assert( capacity );

	if (!(queue = s__malloc(sizeof (struct s__index_queue)))) {
		S__TRACE(0);
		return NULL;
	}
	memset(queue, 0, sizeof (struct s__index_queue));
	queue->capacity = capacity;
	n = queue->capacity * sizeof (queue->vals[0]);
	if (!(queue->vals = s__malloc(n))) {
		s__index_queue_close(queue);
		S__TRACE(0);
		return NULL;
	}
	return queue;
}

void
s__index_queue_close(s__index_queue_t queue)
{
	if (queue) {
		S__FREE(queue->vals);
		memset(queue, 0, sizeof (struct s__index_queue));
	}
	S__FREE(queue);
}

void
s__index_queue_push(s__index_queue_t queue, void *v)
{
	assert( queue );
	assert( queue->size < queue->capacity );

	queue->vals[queue->head] = v;
	queue->head = (queue->head + 1) % queue->capacity;
	++queue->size;
}

void *
s__index_queue_pop(s__index_queue_t queue)
{
	void *v;

	assert( queue );
	assert( queue->size );

	v = queue->vals[queue->tail];
	queue->tail = (queue->tail + 1) % queue->capacity;
	--queue->size;
	return v;
}

int
s__index_queue_empty(s__index_queue_t queue)
{
	assert( queue );

	return queue->size ? 0 : 1;
}
