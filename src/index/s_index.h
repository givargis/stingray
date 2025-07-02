/**
 * Copyright (c) Tony Givargis, 2020-2025
 *
 * s_index.h
 */

#ifndef _S_INDEX_H_
#define _S_INDEX_H_

#include "../utils/s_utils.h"

#define S__INDEX_MAX_KEY_LEN 32767 /* including '\0' */

typedef struct s__index *s__index_t;

/**
 * Opens an empty index and returns an s__index_t handle for subsequent use.
 *
 * @return  An s__index_t handle or NULL on error
 */

s__index_t s__index_open(void);

/**
 * Closes the index and frees resources associated with it.
 *
 * @index  A valid index handle or NULL
 */

void s__index_close(s__index_t index);

/**
 * Removes all indexed items and resets the index to initial state.
 *
 * @index  A valid index handle
 */

void s__index_truncate(s__index_t index);

/**
 * Compresses the index, reducing its memory footprint.
 *
 * @index   A valid index handle
 * @return  0 on success or -1 on error
 *
 * NOTES: A compressed index is no longer able to accept new keys,
 *        effectively turning into a read-only dictionary. However, records
 *        associated with existing keys can still be modified.
 */

int s__index_compress(s__index_t index);

/**
 * Updates the index by adding a new key or returning the record associated
 * with an existing key.
 *
 * @index   A valid index handle
 * @key     A non-empty key
 * @return  A pointer to a record, which can be modified by the caller,
 *          or NULL in case of an error
 */

uint64_t *s__index_update(s__index_t index, const char *key);

/**
 * Finds and returns the record associated with the key.
 *
 * @index   A valid index handle
 * @key     A non-empty key
 * @return  A pointer to a record, which can be modified by the caller,
 *          or NULL if the key does not exist
 */

uint64_t *s__index_find(s__index_t index, const char *key);

/**
 * Finds and returns the record associated with the lexicographical
 * successor of key.
 *
 * @index   A valid index handle
 * @key     A non-empty key, or NULL
 * @return  A pointer to a record, which can be modified by the caller,
 *          or NULL if the key does not exist
 *
 * NOTES: If the key is NULL, the function returns the record associated
 *        with the smallest key. The key doesn't need to be present in the
 *        index, enabling a neighborhood search feature.
 */

uint64_t *s__index_next(s__index_t succinct, const char *key, char *okey);

/**
 * Finds and returns the record associated with the lexicographical
 * predecessor of the key.
 *
 * @index   A valid index handle
 * @key     A non-empty key, or NULL
 * @return  A pointer to a record, which can be modified by the caller,
 *          or NULL if the key does not exist
 *
 * NOTES: If the key is NULL, the function returns the record associated
 *        with the smallest key. The key doesn't need to be present in the
 *        index, enabling a neighborhood search feature.
 */

uint64_t *s__index_prev(s__index_t succinct, const char *key, char *okey);

/**
 * Returns the number of indexed items.
 *
 * @index   A valid index handle
 * @return  The number of indexed items
 */

uint64_t s__index_items(s__index_t index);

/**
 * Runs the built-in self test.
 *
 * @return  0 on success or -1 on error
 */

int s__index_bist(void);

#endif /* _S_INDEX_H_ */
