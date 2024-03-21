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



#ifndef __INTERVAL_LIST__H__
#define __INTERVAL_LIST__H__

#include "interval.h"
#include "util.h"

typedef struct Interval_list
{
  Interval itv;
  struct Interval_list *next_itv;
} Interval_list;

/* Utilitary routines to create, free, print and get information of
 * the interval list */
Interval_list *create_interval_list(void);
void free_interval_list(Interval_list **li_inter);
void print_interval_list(Interval_list *li_inter);
int interval_list_size(Interval_list *li_inter);
int is_empty_list(const Interval_list *li_inter);

/* Add and remove elements of the list routines */
Interval_list *insert_interval_head(Interval_list *li_inter, Interval itv);
void remove_interval_tail(Interval_list **li_inter);
void remove_interval_head(Interval_list **li_inter);

/* Intersection checking routines */
int if_intersects_interval_list(Interval itv, Interval_list *li_inter);

#endif // __INTERVAL_LIST_H__
