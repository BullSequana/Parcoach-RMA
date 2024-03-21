/*
 * Copyright (c) 2020-2024 Bull S.A.S. All rights reserved.
 * Copyright (c) 2020-2024 Inria All rights reserved.
 */

/* This file is part of Parcoach RMA
 * Parcoach RMA is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * Parcoach RMA is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with Parcoach RMA.
 * If not, see <https://www.gnu.org/licenses/>.
 */



/* AVL tree for notifications */

#include <stdio.h>
#include <stdlib.h>
#include "core/interval_tree.h"
#include "core/interval.h"

/* Calculate height of the tree */
static int height(const Interval_tree *N) {
  if (N == NULL)
    return 0;
  return N->height;
}

static inline int max(int A, int B)
{
    return (A > B ? A : B);
}

/* Create a new node in the interval tree */
static Interval_tree *new_interval_tree(Interval *itv) {
  Interval_tree *node = (Interval_tree *)
    malloc(sizeof(Interval_tree));
  node->itv = itv;
  node->left = NULL;
  node->right = NULL;
  node->height = 1;
  node->tree_size = 1;
  return node;
}

/* Right rotation of the tree */
static Interval_tree *interval_tree_rightRotate(Interval_tree *y) {
  if (y == NULL || y->left == NULL)
    return y;

  Interval_tree *x = y->left;
  Interval_tree *T2 = x->right;

  x->right = y;
  y->left = T2;

  y->height = max(height(y->left), height(y->right)) + 1;
  x->height = max(height(x->left), height(x->right)) + 1;

  return x;
}

/* Left rotation of the tree */
static Interval_tree *interval_tree_leftRotate(Interval_tree *x) {
  if (x == NULL || x->right == NULL)
    return x;

  Interval_tree *y = x->right;
  Interval_tree *T2 = y->left;

  y->left = x;
  x->right = T2;

  x->height = max(height(x->left), height(x->right)) + 1;
  y->height = max(height(y->left), height(y->right)) + 1;

  return y;
}

/* Get the balance factor */
static int getBalance(const Interval_tree *N) {
  if (N == NULL)
    return 0;
  return height(N->left) - height(N->right);
}

/* Inserts a new node with the interval asked in the interval tree. The low
 * value of interval is used to do comparisons */
Interval_tree *insert_interval_tree(Interval_tree *node, Interval *itv) {
  if (node == NULL)
    return (new_interval_tree(itv));

  uint64_t low_bound = get_low_bound(itv);
  uint64_t low_bound_node = get_low_bound(node->itv);

  if (low_bound < low_bound_node)
    node->left = insert_interval_tree(node->left, itv);
  else
    node->right = insert_interval_tree(node->right, itv);

  node->tree_size = 1 + node->tree_size;

  /* Update the balance factor of each node and
   * balance the tree */
  node->height = 1 + max(height(node->left),
               height(node->right));

  int balance = getBalance(node);
  uint64_t low_bound_left = low_bound + 1;
  if (NULL != node->left)
      low_bound_left = get_low_bound(node->left->itv);
  uint64_t low_bound_right = low_bound - 1;
  if (NULL != node->right)
      low_bound_right = get_low_bound(node->right->itv);

  if (balance > 1 && low_bound < low_bound_left)
    return interval_tree_rightRotate(node);

  if (balance < -1 && low_bound > low_bound_right)
    return interval_tree_leftRotate(node);

  if (balance > 1 && low_bound > low_bound_left) {
    node->left = interval_tree_leftRotate(node->left);
    return interval_tree_rightRotate(node);
  }

  if (balance < -1 && low_bound < low_bound_right) {
    node->right = interval_tree_rightRotate(node->right);
    return interval_tree_leftRotate(node);
  }

  return node;
}

/* Find the leftmost node of the tree */
static Interval_tree *minValueNode(Interval_tree *node) {
  Interval_tree *current = node;

  while (current->left != NULL)
    current = current->left;

  return current;
}

/* Free the whole tree from memory. Used for full cleanup when synchronizing
 * MPI-RMA epochs. The expected return is NULL. */
Interval_tree *free_interval_tree(Interval_tree *root) {
    Interval_tree *temp = root;
    while (NULL != temp) {
        temp = delete_interval_tree(temp, temp->itv);
    }
    return temp;
}

/* Remove a specific interval from the tree. */
Interval_tree *delete_interval_tree(Interval_tree *root, Interval *itv) {
  if (root == NULL)
    return root;

  uint64_t low_bound = get_low_bound(itv);
  uint64_t low_bound_root = get_low_bound(root->itv);

  if (low_bound < low_bound_root) {
    root->left = delete_interval_tree(root->left, itv);
  } else if (low_bound > low_bound_root) {
    root->right = delete_interval_tree(root->right, itv);
  } else {
    /* If part of the subtree of the node to delete is empty, just bring back
     * the other subtree up, or remove the root if it is a leaf */
    if ((root->left == NULL) || (root->right == NULL)) {
      /* Try finding the side of the tree that is not to bring up to replace root */
      Interval_tree *temp = root->left ? root->left : root->right;

      /* If the value is NULL, it means that root is a leaf; just delete it. */
      if (temp == NULL) {
        free(root);
        root = NULL;
      } else {
        /* Else, put the subtree that is not empty as new root */
        Interval_tree *to_free = root;
        root = temp;
        free(to_free);
      }
    } else {
      /* If both of the subtrees are populated, try finding the leftmost value
       * of the right tree, and substitute it to the root. To do so, we will
       * fill the current root with the values of this leftmost node of the
       * right tree, and remove this leaf instead.*/
      Interval_tree *temp = minValueNode(root->right);
      root->itv = temp->itv;
      root->right = delete_interval_tree(root->right, temp->itv);
    }
  }

  if (root == NULL)
    return root;

  /* Update the balance factor of each node and
   * balance the tree */
  root->height = 1 + max(height(root->left),
               height(root->right));

  int balance = getBalance(root);
  if (balance > 1 && getBalance(root->left) >= 0)
    return interval_tree_rightRotate(root);

  if (balance > 1 && getBalance(root->left) < 0) {
    root->left = interval_tree_leftRotate(root->left);
    return interval_tree_rightRotate(root);
  }

  if (balance < -1 && getBalance(root->right) <= 0)
    return interval_tree_leftRotate(root);

  if (balance < -1 && getBalance(root->right) > 0) {
    root->right = interval_tree_rightRotate(root->right);
    return interval_tree_leftRotate(root);
  }

  return root;
}

/* Checks if the given interval overlaps with another interval in the interval
 * tree. If yes, returns the guilty interval. */
Interval *overlap_search(Interval_tree *root, Interval *i)
{
  if(root == NULL)
    return NULL;
  uint64_t low_bound = get_low_bound(i);
  uint64_t low_bound_root = get_low_bound(root->itv);

  /* Check if the given interval overlaps with root*/
  if(if_intersects(*root->itv,*i))
    return root->itv;

  /* Check if the left subtree exists, and check overlap conditions */
  if(low_bound < low_bound_root)
    return overlap_search(root->left, i);
  /* Check if the right subtree exists and check overlap conditions */

  if(low_bound > low_bound_root)
    return overlap_search(root->right, i);

  return NULL;
}

/* Print the tree in order. */
void in_order_print_interval_tree(Interval_tree *root) {
  if (root != NULL) {
    print_interval(*(root->itv));
    in_order_print_interval_tree(root->left);
    in_order_print_interval_tree(root->right);
  }
}

/* Prints interval tree statistics:
 * - Number of nodes in the tree
 * - Memory size of the tree
 * - Depth of the longest branch in the tree
 */
void print_interval_tree_stats(const Interval_tree *root)
{
  if (root != NULL)
  {
    printf("Tree statistics:\n");
    printf("Number of nodes in the tree: %lu \n", root->tree_size);
    printf("Size of the tree: %lu \n", root->tree_size * sizeof(Interval_tree));
    printf("Depth of the longest branch of the tree: %d \n", root->height);
  }
}

/* Kept for testing purposes */
//int main() {
//  Interval_tree *root = NULL;
//  Interval *itv1 = create_interval(0, 2, RMA_READ, -1, 0, NULL);
//  Interval *itv2 = create_interval(2, 4, RMA_WRITE, -1, 0, NULL);
//  Interval *itv3 = create_interval(4, 6, RMA_WRITE, -1, 0, NULL);
//  Interval *itv4 = create_interval(6, 8, RMA_WRITE, -1, 0, NULL);
//
//  root = insert_interval_tree(root, itv1);
//  root = insert_interval_tree(root, itv2);
//  root = insert_interval_tree(root, itv3);
//  root = insert_interval_tree(root, itv4);
//
//  in_order_print_interval_tree(root);
//
//  root = delete_interval_tree(root, itv3);
//
//  printf("\nAfter deletion: ");
//  in_order_print_interval_tree(root);
//
//  root = free_interval_tree(root);
//
//  printf("\nAfter full deletion: ");
//  in_order_print_interval_tree(root);
//
//  return 0;
//}
