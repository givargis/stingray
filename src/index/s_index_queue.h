/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index_queue.h
 */

#ifndef _S_INDEX_QUEUE_H_
#define _S_INDEX_QUEUE_H_

#include "../utils/s_utils.h"

typedef struct s__index_queue *s__index_queue_t;

s__index_queue_t s__index_queue_open(uint64_t capacity);

void s__index_queue_close(s__index_queue_t queue);

void s__index_queue_push(s__index_queue_t queue, void *v);

void *s__index_queue_pop(s__index_queue_t queue);

int s__index_queue_empty(s__index_queue_t queue);

#endif /* _S_INDEX_QUEUE_H_ */
