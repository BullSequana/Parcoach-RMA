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



#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

 

int main(int argc, char** argv){

  int rank, numProc;
  int *sharedbuf;
  int *localbuf;
  MPI_Win win;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Comm_size(MPI_COMM_WORLD, &numProc);
  MPI_Alloc_mem(1000*sizeof(int), MPI_INFO_NULL, &sharedbuf);
  MPI_Alloc_mem(1000*sizeof(int), MPI_INFO_NULL, &localbuf);

  localbuf[0]=0;
  sharedbuf[0]=4;
  if (numProc !=4){
    printf("We need four processes for this test \n");
    MPI_Abort(MPI_COMM_WORLD,0);
  }
  MPI_Win_create(sharedbuf, 1000*sizeof(int), sizeof(int),MPI_INFO_NULL, MPI_COMM_WORLD, &win);

  printf("I am rank %d,localbuf=%d \n", rank, localbuf[0]);
  MPI_Win_fence(0, win);
  MPI_Get(localbuf,1,MPI_INT,0, 0, 1, MPI_INT, win);
  if(*localbuf%2==0) {
    localbuf[0]++;
  }

   // printf("I am rank %d,localbuf=%d \n", rank, localbuf[0]);
  MPI_Win_fence(0, win);
  printf("I am rank %d,localbuf=%d \n", rank, localbuf[0]);
  MPI_Win_free(&win);
  MPI_Free_mem(sharedbuf);
  MPI_Finalize(); 

  return 0; 

}
