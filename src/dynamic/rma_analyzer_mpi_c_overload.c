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
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include "rma_analyzer.h"
#include "util.h"
#include <mpi.h>
#include <inttypes.h>
#include <time.h>
#include <limits.h>
#include <assert.h>

float temps;
clock_t t1, t2;

/******************************************************
 *          Beginning of epochs functions             *
 ******************************************************/

/* This function is used to spawn the thread that would excute the
 * communication checking thread function at the win_create stage */
int MPI_Win_create(void *base, MPI_Aint size, int disp_unit,
                   MPI_Info info, MPI_Comm comm, MPI_Win *win)
{
  t1 = clock();

  int ret = PMPI_Win_create(base, size, disp_unit, info, comm, win);

  rma_analyzer_start(base, size,comm, win, C);

  return ret;
}

/* This function is used to spawn the thread that would excute the
 * communication checking thread function at the win_allocate stage */
int MPI_Win_allocate(MPI_Aint size, int disp_unit, MPI_Info info,
                     MPI_Comm comm, void *baseptr, MPI_Win *win)
{
  t1 = clock();

  int ret = PMPI_Win_allocate(size, disp_unit, info, comm, baseptr, win);

  rma_analyzer_start(baseptr, size, comm, win, C);

  return ret;
}

/* Global passive Target synchronization "Lock_all" */
int MPI_Win_lock_all(int assert, MPI_Win win)
{
  rma_analyzer_init_comm_check_thread(win);
  return PMPI_Win_lock_all(assert, win);
}

/* Passive target synchronization "Lock" */
int MPI_Win_lock(int lock_type, int rank, int assert, MPI_Win win)
{
  rma_analyzer_init_comm_check_thread(win);

  /* lock_type shared or exclusive */
  return PMPI_Win_lock(lock_type, rank, assert, win);
}

/* Active target synchronization "Fence" */
int MPI_Win_fence(int assert, MPI_Win win)
{
  rma_analyzer_state *state = rma_analyzer_get_state(win);

  if(state->count_fence > 0)
  {
    /* This fence is both the end of an epoch and the beginning of a
     * new one. The communication checking thread thus must be cleared
     * properly before starting the new epoch. */
    rma_analyzer_clear_comm_check_thread(DO_REDUCE, win);

    /* Ensure everyone has ended the epoch and finished progress
     * threads before reseting resources and creating new thread */
    PMPI_Win_fence(0, win);
  }

  state->count_fence++;
  rma_analyzer_init_comm_check_thread(win);

  return PMPI_Win_fence(assert, win);
}

/* PSCW active target synchronisation "Win_start" called by the origin */
int MPI_Win_start(MPI_Group group, int assert, MPI_Win win)
{
  rma_analyzer_init_comm_check_thread(win);
  return PMPI_Win_start(group, assert, win);
}

/* PSCW active target synchronisation "Win_post" called by the target */
int MPI_Win_post(MPI_Group group, int assert, MPI_Win win)
{
  rma_analyzer_init_comm_check_thread(win);
  return PMPI_Win_post(group, assert, win);
}

/************************************************
 *      One-sided communication functions       *
 ***********************************************/

/* MPI_Put is used to one-sidedly send data to the window of another
 * process it's used here to retrieve the interval
 * [offset, offset+size[ and send it to the target with MPI_Send */
int MPI_Put(const void *origin_addr,
            int origin_count,
            MPI_Datatype origin_datatype,
            int target_rank,
            MPI_Aint target_disp,
            int target_count,
            MPI_Datatype target_datatype,
            MPI_Win win)
{
  LOG(stderr,"A process is accessing a put function !\n\n");
  uint64_t local_address = (uint64_t) origin_addr;
  uint64_t local_size = 0;
  uint64_t target_size = 0;

  MPI_Type_size(origin_datatype, (int *)&local_size);
  local_size *= origin_count;
  MPI_Type_size(target_datatype, (int *)&target_size);
  target_size *= target_count;

  rma_analyzer_update_on_comm_send(local_address, local_size,
                                   target_disp, target_size,
                                   target_rank, RMA_READ, RMA_WRITE,
                                   -1, 0, NULL, win);

  return PMPI_Put(origin_addr, origin_count, origin_datatype,
                  target_rank, target_disp, target_count,
                  target_datatype, win);
}

/* This function is the same as MPI_Put the only difference is that
 * the get permits to one-sidedly fetch data from the window of
 * another MPI process */
int MPI_Get(void *origin_addr,
            int origin_count,
            MPI_Datatype origin_datatype,
            int target_rank,
            MPI_Aint target_disp,
            int target_count,
            MPI_Datatype target_datatype,
            MPI_Win win)
{
  LOG(stderr,"A process is accessing a get function !\n\n");
  uint64_t local_address = (uint64_t) origin_addr;
  uint64_t local_size = 0;
  uint64_t target_size = 0;

  MPI_Type_size(origin_datatype, (int *)&local_size);
  local_size *= origin_count;
  MPI_Type_size(target_datatype, (int *)&target_size);
  target_size *= target_count;

  rma_analyzer_update_on_comm_send(local_address, local_size,
                                   target_disp, target_size,
                                   target_rank, RMA_WRITE, RMA_READ,
                                   -1, 0, NULL, win);

  return PMPI_Get(origin_addr, origin_count, origin_datatype,
                  target_rank, target_disp, target_count,
                  target_datatype, win);
}

/* An atomic put that does the same as MPI_Put here ! */
int MPI_Accumulate(const void *origin_addr,
                   int origin_count,
                   MPI_Datatype origin_datatype,
                   int target_rank,
                   MPI_Aint target_disp,
                   int target_count,
                   MPI_Datatype target_datatype,
                   MPI_Op op,
                   MPI_Win win)
{
  LOG(stderr,"A process is accessing an accumulate function !\n\n");
  uint64_t local_address = (uint64_t) origin_addr;
  uint64_t local_size = 0;
  uint64_t target_size = 0;

  MPI_Type_size(origin_datatype, (int *)&local_size);
  local_size *= origin_count;
  MPI_Type_size(target_datatype, (int *)&target_size);
  target_size *= target_count;

  rma_analyzer_update_on_comm_send(local_address, local_size,
                                   target_disp, target_size,
                                   target_rank, RMA_READ, RMA_WRITE,
                                   -1, 0, NULL, win);

  return PMPI_Accumulate(origin_addr, origin_count, origin_datatype,
                         target_rank, target_disp, target_count,
                         target_datatype, op, win);
}

/********************************************
 *            End of epochs                 *
 ********************************************/

/* Global Passive target synchronisation "Unlock_all" called by all
 * the processus */
int MPI_Win_unlock_all(MPI_Win win)
{
  rma_analyzer_clear_comm_check_thread(DO_REDUCE, win);
  return PMPI_Win_unlock_all(win);
}

/* Passive target Synchronization "Unlock" */
int MPI_Win_unlock(int rank, MPI_Win win)
{
  rma_analyzer_clear_comm_check_thread(DO_REDUCE, win);
  return PMPI_Win_unlock(rank, win);
}

/* PSCW active synchronisation "Win_complete" called by the origin */
int MPI_Win_complete(MPI_Win win)
{
  rma_analyzer_clear_comm_check_thread(DO_REDUCE, win);
  return PMPI_Win_complete(win);
}

/* PSCW active synchronisation "Win_wait" called by the target */
int MPI_Win_wait(MPI_Win win)
{
  rma_analyzer_clear_comm_check_thread(DO_REDUCE, win);
  return PMPI_Win_wait(win);
}

/* Free memory */
int MPI_Win_free(MPI_Win *win)
{
  const rma_analyzer_state *state = rma_analyzer_get_state(*win);

  if(state->count_fence > 0)
  {
     /* We need to free here the last thread created by the Fence
      * call, because there is no way that we can know in the RMA
      * Analyzer which Fence is the last one of the lifetime of the
      * window. We enforce 0 in the clear call to avoid waiting on
      * non-existent communications. */
     rma_analyzer_clear_comm_check_thread(0, *win);
  }

  t2 = clock();
  temps = (float)(t2-t1)/CLOCKS_PER_SEC;
  LOG(stderr,"time = %f\n", temps);
  LOG(stderr,"I passed the win_free\n");
  LOG(stderr,"time = %f\n", temps);

  rma_analyzer_stop(*win);
  return PMPI_Win_free(win);
}
