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



#ifndef __INTERVAL__H__
#define __INTERVAL__H__
#include <stdint.h>
#include "util.h"

#define FILENAME_MAX_LENGTH 128

/* Access types of intervals, similar to access rights of memory.
 * These enums should be kept as is, as the checking of compatibility
 * between accesses is made with a binary OR on values */
typedef enum
{
  LOCAL_READ = 0,
  LOCAL_WRITE = 1,
  RMA_READ = 2,
  RMA_WRITE = 3
} Access_type;

/* An interval is described with an access type, and two bounds
 * that delimits the intervals as such : [low_bound, up_bound[ */
typedef struct Interval
{
  Access_type access_type;
  uint64_t low_bound;
  uint64_t up_bound;
  /* Notification ID if used */
  int notification_id;
  /* Debug infos needed for user feedback */
  int line;
  char filename[FILENAME_MAX_LENGTH];
} Interval;

/* Convert an access type to a string */
char* access_type_to_str(Access_type access);

/* Print the contents of the interval as this :
 * "access_type [low_bound, up_bound[" */
void print_interval(Interval itv);

/* Returns an Interval object with the parameters specified in the
 * routine */
Interval *create_interval(uint64_t low_bound, uint64_t up_bound,
                          Access_type access_type, int notif_id,
                          int line, const char* filename);

void free_interval(Interval *itv);

/* Getter routines for interval specifics */
uint64_t get_low_bound(const Interval *itv);
uint64_t get_up_bound(const Interval *itv);
Access_type get_access_type(const Interval *itv);
int get_notif_id(const Interval *itv);
int get_fileline(const Interval *itv);
char* get_filename(Interval *itv);

/* Returns 0 if the two intervals do not intersect, 1 otherwise. */
int if_intersects(Interval itvA, Interval itvB);

#endif //__INTERVAL_H__
