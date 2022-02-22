#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>


static MPI_Win window;

void my_rand(int my_rank)
{
  int my_value = 1235; 
  int my_sd_value = 1835; 
  int my_td_value = 25648;

  srand(time(NULL) + my_rank);
  int value = rand()%3 + 1;

  if(my_rank == 0)
  {
    if(value == 1) 
    {
      MPI_Put(&my_value, 1, MPI_INT, 1, 0, 1, MPI_INT, window);
    }
    if(value == 2)
    {
      MPI_Get(&my_sd_value, 1, MPI_INT, 1, 0, 1, MPI_INT, window);
    }
    if(value == 3) 
    {
      MPI_Accumulate(&my_td_value, 1, MPI_INT, 1, 0, 1, MPI_INT,MPI_SUM, window);
    }
  }
  if(my_rank == 1)
  {
    if(value == 1) 
    {
      MPI_Put(&my_value, 1, MPI_INT, 0, 0, 1, MPI_INT, window);
    }
    if(value == 2)
    {
      MPI_Get(&my_sd_value, 1, MPI_INT, 0, 0, 1, MPI_INT, window);
    }
    if(value == 3) 
    {
      MPI_Accumulate(&my_td_value, 1, MPI_INT, 0, 0, 1, MPI_INT,MPI_SUM, window);
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
  int window_buffer[100] = {0};
  MPI_Win_create(&window_buffer, 100*sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &window);


  MPI_Win_lock_all(0, window);
  my_rand(my_rank);

  MPI_Win_unlock_all(window);


  // Destroy the window
  MPI_Win_free(&window);
  printf("I passed the win_free  !\n");

  MPI_Finalize();

  return EXIT_SUCCESS;
}

