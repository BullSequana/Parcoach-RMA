#ifndef __INTERVAL_TREE__H__
#define __INTERVAL_TREE__H__

#include "interval.h"

/* Node of an interval tree */
typedef struct Interval_tree
{
  Interval *itv;
  struct Interval_tree *left;
  struct Interval_tree *right;
  int height;
  unsigned long tree_size;
} Interval_tree;

/* Inserts a new node with the interval asked in the interval tree. The low
 * value of interval is used to do comparisons */
Interval_tree *insert_interval_tree(Interval_tree *root, Interval *i);

/* Checks if the given interval overlaps with another interval in the interval
 * tree. If yes, returns the guilty interval. */
Interval *overlap_search(Interval_tree *root, Interval *i);

/* Print the tree in order. */
void in_order_print_interval_tree(Interval_tree *root);

/* Free the whole tree from memory. Used for full cleanup when synchronizing
 * MPI-RMA epochs. The expected return is NULL. */
Interval_tree *free_interval_tree(Interval_tree *root);

/* Remove a specific interval from the tree. */
Interval_tree *delete_interval_tree(Interval_tree *root, Interval *i);

/* Prints interval tree statistics:
 * - Number of nodes in the tree
 * - Memory size of the tree
 * - Depth of the longest branch in the tree
 */
void print_interval_tree_stats(const Interval_tree *root);

#endif // __INTERVAL_TREE_H__
