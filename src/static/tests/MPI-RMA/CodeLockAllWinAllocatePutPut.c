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
    printf("We need at least two processes for this test \n");

    MPI_Abort(MPI_COMM_WORLD,0);
  }
  MPI_Win_allocate(1000*sizeof(int), sizeof(int),MPI_INFO_NULL, MPI_COMM_WORLD,localbuf, &win);

  printf("I am rank %d,localbuf=%d \n", rank, localbuf[0]);
  MPI_Win_lock_all(0, win);
  MPI_Put(sharedbuf,1,MPI_INT,0, 0, 1, MPI_INT, win);
 // if(*localbuf%2==0)
    sharedbuf[0]++;

  MPI_Put(localbuf,1,MPI_INT,0, 0, 1, MPI_INT, win);
  localbuf[0]++;
  MPI_Win_unlock_all(win);
  printf("I am rank %d,localbuf=%d \n", rank, localbuf[0]);
  MPI_Win_free(&win);
  MPI_Free_mem(sharedbuf);
  MPI_Finalize(); 

  return 0; 

}
