#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "mpi-ext.h"

int main(int argc, char* argv[])
{
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  //Check that only three processes are spawn
  int comm_size;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  if(comm_size != 2)
  {
    printf("This application is meant to be run with 3 MPI processes, not %d.\n", comm_size);
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }

  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  int window_buffer[100] = {0};
  MPI_Win window;

#ifdef OMPI_HAVE_MPI_EXT_NOTIFIED_RMA
  MPIX_Win_create_notify(&window_buffer, 100*sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &window);

  int my_value = 12345;

  MPI_Win_lock_all(0, window);

  if(my_rank == 0)
  {
    // Put from my_value to target 1
    MPIX_Put_notify(&my_value, 1, MPI_INT, 1, 80, 1, MPI_INT, window, 1);
    MPIX_Win_wait_notify(window, 2);
    MPIX_Put_notify(&my_value, 1, MPI_INT, 1, 80, 1, MPI_INT, window, 3);
  }

  if(my_rank == 1)
  {
    // Put from my_value to target 0
    MPIX_Win_wait_notify(window, 1);
    MPIX_Put_notify(&my_value, 1, MPI_INT, 0, 80, 1, MPI_INT, window, 2);
    MPIX_Win_wait_notify(window, 3);
  }

  MPI_Win_unlock_all(window);

  // Destroy the window
  MPI_Win_free(&window);
  printf("I passed the win_free  !\n");

#else // OMPI_HAVE_MPI_EXT_NOTIFIED_RMA
  printf("This Open MPI implementation does not have support for notified RMA.");
#endif // OMPI_HAVE_MPI_EXT_NOTIFIED_RMA

  MPI_Finalize();

  return EXIT_SUCCESS;
}
