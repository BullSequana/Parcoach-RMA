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
#include <stdlib.h>
#include "mpi.h"
#include <time.h>

static MPI_Win window;

static int window_buffer[100] = {0};

void my_rand(int my_rank)
{
  int fs_value = 10; 
  srand(time(NULL) + my_rank);
  int value = rand()%3 + 1;
  if(my_rank = 0)
  {
    if(value == 1)
    {
      MPI_Put(&fs_value, 1, MPI_INT, 1, 0, 1, MPI_INT, window);
      printf("[MPI process 0] I put data %d in MPI process 1 window via MPI_Put.\n", fs_value);
    }
    if(value == 2)
    {
      MPI_Get(&fs_value, 1, MPI_INT, 1, 0, 1, MPI_INT, window);
      printf("[MPI process 0] I Get data %d in MPI process 1 window via MPI_Put.\n", fs_value);
    }
    if(value == 3)
    {
      MPI_Accumulate(&fs_value, 1, MPI_INT, 1, 0, 1, MPI_INT,MPI_SUM, window);
    }
  }
  if(my_rank = 1)
  {
    if(value == 1)
    {
      MPI_Put(&window_buffer[0], 1, MPI_INT, 0, 0, 1, MPI_INT, window);
      printf("[MPI process 1] I put data %p in MPI process 0 window via MPI_Put.\n",window_buffer);
    }
    if(value == 2)
    {
      MPI_Get(&window_buffer[0], 1, MPI_INT, 0, 0, 1, MPI_INT, window);
    }
    if(value == 3)
    {
      MPI_Accumulate(&window_buffer[0], 1, MPI_INT, 0, 0, 1, MPI_INT,MPI_SUM, window);
    }
  }
}
int main(int argc, char* argv[])
{

  MPI_Init(&argc, &argv);


//Check that only two processes are spawn
  int comm_size;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);


  if(comm_size != 2)
  {
    printf("This application is meant to be run with 2 MPI processes, not %d.\n", comm_size);
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }



  //get my rank 
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


  //create the window 

  MPI_Win_create(window_buffer, 100 * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &window);




  MPI_Win_lock_all(0, window);
  my_rand(my_rank);
  //my_rand(my_rank);


  MPI_Win_unlock_all(window);



  // Destroy the window
  MPI_Win_free(&window);

  MPI_Finalize();

  return EXIT_SUCCESS;
}

