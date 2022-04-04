#ifndef _BINARY_TREE_H
#define _BINARY_TREE_H

#include <stdint.h>
#include "db.h"

#define MAX_TREE_DEPTH 4
#define MAX_TREE_RESULTS 4
#define MAX_TREE_NODE_VALUES 4

#define TREE_PATH_LEFT -1
#define TREE_PATH_EQUALS 0
#define TREE_PATH_RIGHT 1

typedef uint32_t tree_key_t;
typedef uint32_t tree_value_t;

typedef struct tree {
	hash_t root;
} tree_t;


typedef struct tree_result {
	tree_key_t key;
	tree_value_t value;
} tree_result_t;

typedef struct tree_result_set {
	uint8_t num_results;
	tree_result_t results[MAX_TREE_RESULTS];
} tree_result_set_t;

typedef struct tree_node {
	hash_t left;
	hash_t right;

	tree_key_t key;

	uint8_t num_values;
	tree_value_t values[MAX_TREE_NODE_VALUES];
} tree_node_t;

typedef struct tree_path {
	uint8_t depth;
	tree_node_t nodes[MAX_TREE_DEPTH];
	int8_t branches[MAX_TREE_DEPTH];
} tree_path_t;

#define WITH_PATH_NODE_BEGIN(path) 			    				\
	{															\
		int _wpn_i; 								    		\
		tree_node_t* node;								    	\
		for (_wpn_i = 0; _wpn_i < MAX_TREE_DEPTH; _wpn_i++) {  	\
			if (_wpn_i == path->depth - 1) {             		\
				node = &(path->nodes[_wpn_i]);       			\

#define WITH_PATH_NODE_END \
			}			   \
		}				   \
	}					   \

int _value_equals(tree_value_t* v1, tree_value_t* v2) {
  //return hasheq(v1, v2);
  return (*v1 == *v2);
}

//int value_assign(tree_value_t* v1, tree_value_t* v2) {
  //int i;
  //for (i = 0; i < (DB_HASH_NUM_BITS/64); i++) {
    //v1->bit[i] = v2->bit[i];
  //}
//}

int _copy_value(tree_node_t *node, tree_value_t value) {
	BOOL success = FALSE;
	BOOL found = FALSE;
	int i;

	for (i = 0; i < MAX_TREE_NODE_VALUES; i++) {
		if (i < node->num_values && _value_equals(&(node->values[i]), &value)) {
			found = TRUE;
		} else if (!found && i == node->num_values) {
			node->values[i] = value;
			success = TRUE;
		}
	}
	if (success) {
		node->num_values++;
	}

	return success;
}

void _copy_result(tree_result_set_t* result_set, tree_key_t key, tree_value_t value) {
	int i;
	for (i = 0; i < MAX_TREE_RESULTS; i++) {
		if (i == result_set->num_results) {
			result_set->results[i].key = key;
			result_set->results[i].value = value;
		}
	}
	if (result_set->num_results < MAX_TREE_RESULTS) {
		result_set->num_results++;
	}
}

void _copy_results(tree_result_set_t* result_set, tree_node_t* node) {
	int i;
	for (i = 0; i < MAX_TREE_NODE_VALUES; i++) {
		if (i < node->num_values) {
			_copy_result(result_set, node->key, node->values[i]);
		}
	}
}

// Find the node with matching key if it exists, storing the path along the way
// if a matching key is not found, the path will be some path from the root to
// a leaf node whose key is greater than the searching key or whose in-order
// successor's key is greater than the searching key.
int _find_path(tree_t* tree, tree_key_t key, tree_path_t* path) {
	int i;
	hash_t node_hash = tree->root;
	BOOL found = FALSE;

	for (i = 0; i < MAX_TREE_DEPTH; i++) {
		if (!found && !hasheq(&node_hash, NULL_HASH)) {
			tree_node_t node;
			hashget(&node, &node_hash);

			path->depth = i + 1;
			path->nodes[i] = node;

			if (key == node.key) {
				found = TRUE;
				path->branches[i] = TREE_PATH_EQUALS;
			} else if (key < node.key) {
				node_hash = node.left;
				path->branches[i] = TREE_PATH_LEFT;
			} else {
				node_hash = node.right;
				path->branches[i] = TREE_PATH_RIGHT;
			}
		}
	}

	return found;
}

int _find_first(tree_t* tree, tree_path_t* path) {
	int i;
	hash_t node_hash = tree->root;
	BOOL found = FALSE;

	for (i = 0; i < MAX_TREE_DEPTH; i++) {
		if (!hasheq(&node_hash, NULL_HASH)) {
			tree_node_t node;
			hashget(&node, &node_hash);

			found = TRUE;

			path->depth = i + 1;
			path->nodes[i] = node;
			path->branches[i] = TREE_PATH_LEFT;

			node_hash = node.left;
		}
	}

	return found;
}

int _find_next_below(tree_path_t* path) {
	int i;
	hash_t node_hash = *NULL_HASH;
	BOOL found = FALSE;
	int old_index = path->depth - 1;

	for (i = 0; i < MAX_TREE_DEPTH; i++) {
		if (i == old_index) {
			node_hash = path->nodes[i].right;
      path->branches[i] = TREE_PATH_RIGHT;
		} else if (i > old_index && !hasheq(&node_hash, NULL_HASH)) {
			tree_node_t node;
			hashget(&node, &node_hash);

			found = TRUE;

			path->depth = i + 1;
			path->nodes[i] = node;
			path->branches[i] = TREE_PATH_LEFT;

			node_hash = node.left;

			//if (i == old_index + 1) {
				//path->branches[i - 1] = TREE_PATH_RIGHT;
			//}
		}
	}

	return found;
}

int _find_next_above(tree_path_t* path) {
	int i;
	BOOL found = FALSE;

	for (i = MAX_TREE_DEPTH - 1; i >= 0; i--) {
		if (!found && i < path->depth - 1 && path->branches[i] == TREE_PATH_LEFT) {
			found = TRUE;
			path->depth = i + 1;
		}
	}

	return found;
}

int _find_next(tree_path_t* path) {
	int found = _find_next_below(path);
	if (!found) {
		found = _find_next_above(path);
	}

	return found;
}

// Recompute the hashes along the path back up to the root
//
// BUG: Doesn't free old unlinked nodes
void _recompute_path(tree_t* tree, tree_path_t* path) {
	int i;
	hash_t node_hash;

	for (i = MAX_TREE_DEPTH - 1; i >= 0; i--) {
		if (i < path->depth) {
			tree_node_t* node = &(path->nodes[i]);

			if (i < path->depth - 1) {
				if (path->branches[i] == TREE_PATH_LEFT) {
					node->left = node_hash;
				} else if (path->branches[i] == TREE_PATH_RIGHT) {
					node->right = node_hash;
				}
			}

			hashput(&node_hash, node);
		}
	}

	tree->root = node_hash;
}

void tree_init(tree_t* tree) {
	tree->root = *NULL_HASH;
}

void tree_find_eq(tree_t* tree, tree_key_t key, tree_result_set_t* result_set) {
	int i;
	hash_t node_hash = tree->root;
	BOOL found = FALSE;

	result_set->num_results = 0;

	for (i = 0; i < MAX_TREE_DEPTH; i++) {
		if (!found && !hasheq(&node_hash, NULL_HASH)) {
			tree_node_t node;
			hashget(&node, &node_hash);

			if (key == node.key) {
				found = TRUE;
				_copy_results(result_set, &node);
			} else if (key < node.key) {
				node_hash = node.left;
			} else {
				node_hash = node.right;
			}
		}
	}
}

void tree_find_lt(tree_t* tree, tree_key_t key, BOOL equal_to, tree_result_set_t* result_set) {
	int i;
	tree_path_t path;
	BOOL found = FALSE;
	path.depth = 0;

	found = _find_first(tree, &path);
	if (found) {
		WITH_PATH_NODE_BEGIN((&path))
			_copy_results(result_set, node);
		WITH_PATH_NODE_END

		for (i = 0; i < MAX_TREE_RESULTS; i++) {
			found = _find_next(&path);
			if (found) {
				WITH_PATH_NODE_BEGIN((&path))
					if (node->key < key || (equal_to && node->key == key)) {
						_copy_results(result_set, node);
					}
				WITH_PATH_NODE_END
			}
		}
	}
}

void tree_find_gt(tree_t* tree, tree_key_t key, BOOL equal_to, tree_result_set_t* result_set) {
	int i;
	tree_path_t path;
	BOOL found = FALSE;
	path.depth = 0;

	found = _find_path(tree, key, &path);
	if (path.depth > 0) {
		WITH_PATH_NODE_BEGIN((&path))
			if ((found && equal_to) || node->key > key) {
				_copy_results(result_set, node);
			}
		WITH_PATH_NODE_END

    for (i = 0; i < MAX_TREE_RESULTS; i++) {
      found = _find_next(&path);
      if (found) {
        WITH_PATH_NODE_BEGIN((&path))
          //if (node->key > key) {
          _copy_results(result_set, node);
          //}
        WITH_PATH_NODE_END
      }
    }
	}
}

void tree_find_range(tree_t* tree, tree_key_t low_key, BOOL low_equal_to,
					 tree_key_t high_key, BOOL high_equal_to, tree_result_set_t* result_set) {
	int i;
	tree_path_t path;
	BOOL found = FALSE;
	path.depth = 0;

	found = _find_path(tree, low_key, &path);
	if (path.depth > 0) {
		WITH_PATH_NODE_BEGIN((&path))
			if ((found && low_equal_to) || (node->key > low_key && node->key < high_key) ||
				(high_equal_to && node->key == high_key)) {
				_copy_results(result_set, node);
			}
		WITH_PATH_NODE_END

    for (i = 0; i < MAX_TREE_RESULTS; i++) {
      found = _find_next(&path);
      if (found) {
        WITH_PATH_NODE_BEGIN((&path))
          //if ((node->key > low_key && node->key < high_key) ||
            //(high_equal_to && node->key == high_key)) {
          if ((node->key < high_key) || (high_equal_to && node->key == high_key)) {
            _copy_results(result_set, node);
          }
        WITH_PATH_NODE_END
      }
    }
	}
}

int tree_insert(tree_t* tree, tree_key_t key, tree_value_t value) {
	BOOL found = FALSE;
	tree_path_t path;
	int i;
	BOOL success = FALSE;

	path.depth = 0;
	found = _find_path(tree, key, &path);

	if (found) {
		WITH_PATH_NODE_BEGIN((&path))
			success = _copy_value(node, value);
		WITH_PATH_NODE_END
	} else if (!found && path.depth < MAX_TREE_DEPTH) {
		path.depth++;

		WITH_PATH_NODE_BEGIN((&path))
			node->left = *NULL_HASH;
			node->right = *NULL_HASH;
			node->key = key;
			node->num_values = 0;
			success = _copy_value(node, value);
		WITH_PATH_NODE_END
	}

	if (success) {
		_recompute_path(tree, &path);
	}

	return success;
}

// BUG: For now, removing a key just erases the values. It doesn't actually
// remove the node itself. Lame...
int tree_remove(tree_t* tree, tree_key_t key) {
	BOOL found = FALSE;
	tree_path_t path;
	path.depth = 0;

	found = _find_path(tree, key, &path);
	if (found) {
		WITH_PATH_NODE_BEGIN((&path))
			node->num_values = 0;
		WITH_PATH_NODE_END
		_recompute_path(tree, &path);
	}

	return found;
}

int tree_remove_value(tree_t* tree, tree_key_t key, tree_value_t value) {
	BOOL found = FALSE;
	tree_path_t path;
	int i;
	BOOL removed = FALSE;

	path.depth = 0;

	found = _find_path(tree, key, &path);
	if (found) {
		WITH_PATH_NODE_BEGIN((&path))
			for (i = 0; i < MAX_TREE_NODE_VALUES; i++) {
				if (i < node->num_values) {
					if (_value_equals(&(node->values[i]), &value)) {
						removed = TRUE;
					} else if (removed && i > 0){
						node->values[i - 1] = node->values[i];
					}
				}
			}

			if (removed) {
				node->num_values--;
			}
		WITH_PATH_NODE_END

		if (removed) {
			_recompute_path(tree, &path);
		}
	}

	return found;
}
#endif
