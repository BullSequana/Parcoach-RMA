PROGRAM main
include 'mpif.h'
include 'mpif-ext.h'

   INTEGER, ALLOCATABLE, SAVE, DIMENSION(:)    :: window_buffer
   INTEGER                                     :: myrank, comm_size, ierr, my_value

   INTEGER (KIND=MPI_ADDRESS_KIND)             :: disp_int, window
   INTEGER (KIND=MPI_ADDRESS_KIND)             :: lowerbound, size_window, intextent, deplacement

   my_value = 12345
   deplacement = 0

   IF (OMPI_HAVE_MPI_EXT_NOTIFIED_RMA == 1) THEN
     call MPI_init_thread( MPI_THREAD_MULTIPLE, provided, ierr )

     CALL mpi_comm_rank( MPI_COMM_WORLD, myrank, ierr )
     CALL mpi_comm_size( MPI_COMM_WORLD, comm_size, ierr )

     IF (comm_size .ne. 2) THEN
      print *, "This application is meant to be run with 2 MPI processes, not ",comm_size
      call MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE, ierr);
     END IF

     ALLOCATE(window_buffer(100))
     DO i = 1, 100
        window_buffer(i) = 0
     END DO

     call MPI_TYPE_GET_EXTENT(MPI_int, lowerbound, intextent, ierr)
     IF( ierr /= MPI_SUCCESS) THEN 
        print *, "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
     END IF

     disp_int = intextent
     size_window = 100 * intextent

     call MPIX_WIN_CREATE_NOTIFY(window_buffer(1), size_window, disp_int, MPI_INFO_NULL, MPI_COMM_WORLD, window, ierr)
     IF( ierr /= MPI_SUCCESS) THEN 
        print *, "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
     END IF

     call MPI_Win_lock_all(0, window, ierr)
     IF( ierr /= MPI_SUCCESS) THEN 
        print *, "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
     END IF

     if (myrank == 0) then
         CALL mpix_put_notify(my_value, 1, mpi_int, 1, deplacement, 1, mpi_int, window, 1, ierr)
         print *, "Put_notify notif 1 from ",myrank," to 1"
         IF( ierr /= MPI_SUCCESS) THEN 
            print *, "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF
         CALL mpix_win_wait_notify(window, 2, ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            print *, "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF
         CALL mpix_put_notify(my_value, 1, mpi_int, 1, deplacement, 1, mpi_int, window, 3, ierr)
         print *, "Put_notify notif 3 from ",myrank," to 1"
         IF( ierr /= MPI_SUCCESS) THEN 
            print *, "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF
     end if

     if (myrank == 1) then
         CALL mpix_win_wait_notify(window, 1, ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            print *, "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF
         CALL mpix_put_notify(my_value, 1, mpi_int, 0, deplacement, 1, mpi_int, window, 2, ierr)
         print *, "Put_notify notif 0 from ",myrank," to 0"
         IF( ierr /= MPI_SUCCESS) THEN 
            print *, "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF
         CALL mpix_win_wait_notify(window, 3, ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            print *, "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF
     end if

     call mpi_win_unlock_all(window, ierr)

     call mpi_win_free(window, ierr)

     DEALLOCATE(window_buffer)

     if (myrank == 0) then
        print *, "Test finished successfully"
     end if

     CALL mpi_finalize(ierr)
   END IF

END PROGRAM main
