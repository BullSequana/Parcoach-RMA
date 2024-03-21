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



#ifndef __notif_tree__H__
#define __notif_tree__H__

#include "interval.h"

/* Node of a notification tree */
typedef struct Notif_tree
{
  int notif_id;
  Interval *itv;
  struct Notif_tree *left;
  struct Notif_tree *right;
  int height;
  unsigned long tree_size;
} Notif_tree;

/* Inserts a new node with the notification ID associated to the interval
 * passed in parameter. */
Notif_tree *insert_notif_tree(Notif_tree *root, Interval *itv);

/* Print the tree in order. */
void in_order_print_notif_tree(Notif_tree *root);

/* Free the whole tree from memory. Used for full cleanup when synchronizing
 * MPI-RMA epochs. The expected return is NULL. */
Notif_tree *free_notif_tree(Notif_tree *root);

/* Remove a specific notification ID from the tree. */
Notif_tree *delete_notif_tree(Notif_tree *root, int notif_id);

/* Find an interval from its notification ID. Used to find what interval should
 * be purged from the interval tree when a notified synchronization routine is
 * called. */
Interval *find_interval_from_notif(Notif_tree *root, int notif_id);

/* Prints notif tree statistics:
 * - Number of nodes in the tree
 * - Memory size of the tree
 * - Depth of the longest branch in the tree
 */
void print_notif_tree_stats(const Notif_tree *root);

#endif // __notif_tree_H__
