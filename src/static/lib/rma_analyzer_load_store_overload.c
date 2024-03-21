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



#include <stdlib.h>
#include <stdio.h>
#include "rma_analyzer.h"
#include <limits.h>
#include <assert.h>

void STORE(void *addr, uint64_t size, int line, const char *filename)
{
  uint64_t address = (uint64_t)addr;
  LOG(stderr,"Store address %" PRIu64 "\n", address);
  LOG(stderr,"size : %" PRIu64 "\n", (size / 8));

  /* We save this interval in all active windows, since load/store
   * instructions have an impact on all active windows */
  rma_analyzer_save_interval_all_wins(address, (size / 8) - 1,
                                      LOCAL_WRITE, line, filename);
}

void LOAD(void *addr, uint64_t size, int line, const char *filename)
{
  uint64_t address = (uint64_t)addr;
  LOG(stderr,"Load address %" PRIu64 "\n", address);
  LOG(stderr,"size : %" PRIu64 "\n", (size / 8));

  /* We save this interval in all active windows, since load/store
   * instructions have an impact on all active windows */
  rma_analyzer_save_interval_all_wins(address, (size / 8) - 1,
                                      LOCAL_READ, line, filename);
}
