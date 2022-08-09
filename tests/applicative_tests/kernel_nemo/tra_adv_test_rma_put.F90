!!========================================================================================================================

   !! ***  traadv kernel extracted from the NEMO software ( http://www.nemo-ocean.eu ) ***
   !! ***  governed by the CeCILL licence (http://www.cecill.info)  ***
   !!
   !!                                                       IS-ENES2 - CMCC
   !!========================================================================================================================
PROGRAM tra_adv
#include "mpif.h"
   REAL, ALLOCATABLE, SAVE, DIMENSION(:,:,:,:) :: t3sn, t3ns, t3ew, t3we
   REAL, ALLOCATABLE, SAVE, DIMENSION(:,:,:)   :: tsn
   REAL, ALLOCATABLE, SAVE, DIMENSION(:,:,:)   :: pun, pvn, pwn
   REAL, ALLOCATABLE, SAVE, DIMENSION(:,:,:)   :: mydomain, zslpx, zslpy, zwx, zwy, umask, vmask, tmask, zind !, zwx1
   REAL, ALLOCATABLE, SAVE, DIMENSION(:,:)     :: ztfreez, rnfmsk, upsmsk
   REAL, ALLOCATABLE, SAVE, DIMENSION(:)       :: rnfmsk_z
   REAL                                        :: zice, zu, z0u, zzwx, zv, z0v, zzwy, ztra, zbtr, zdt, zalpha
   INTEGER                                     :: nbondi, nbondj, nono, noso, noea, nowe
   INTEGER                                     :: jpi, jpj, jpk
   INTEGER                                     :: myrank, p, ierr, realcomm
   INTEGER                                     :: px, py, cpt
   CHARACTER(len=10)                           :: env
   INTEGER                                     :: jpreci, jprecj, nreci, nrecj, nlci, nlcj
   INTEGER                                     :: i, hpmid, color, a, j, ji, jj, jk, jt

   REAL                                        :: temps_p1,temps_p2,temps_p3,temps_com1,temps_com2,temps_com3
   REAL                                        :: temps_com4
   REAL                                        :: temps_init,temps_tot, temps_win,temps_win_tot,temps_delloc
   REAL                                        :: temps_fini, temps_delloc_tot
   REAL                                        :: temps_test,temps_test_deb
   INTEGER (KIND=MPI_ADDRESS_KIND)             :: disp_int,wint3ns,wint3ew
   INTEGER (KIND=MPI_ADDRESS_KIND)             :: lowerbound, size, realextent,deplacement




   call cpu_time(temps_init)

   CALL get_environment_variable("PROC_X", env)
   READ ( env, '(i10)' ) px
   CALL get_environment_variable("PROC_Y", env)
   READ ( env, '(i10)' ) py
   CALL get_environment_variable("JPI", env)
   READ ( env, '(i10)' ) jpi
   CALL get_environment_variable("JPJ", env)
   READ ( env, '(i10)' ) jpj
   CALL get_environment_variable("JPK", env)
   READ ( env, '(i10)' ) jpk

   CALL get_environment_variable("CPU_PER_TASK", env)
   READ ( env, '(i10)' ) cpt

   call MPI_init_thread( MPI_THREAD_MULTIPLE, provided, ierr )



   CALL mpi_comm_rank( MPI_COMM_WORLD, myrank, ierr )
   CALL mpi_comm_size( MPI_COMM_WORLD, p, ierr )
      WRITE (*,*) "size=",p," PROC_X=",px, "PROC_Y", py
      color = MOD(myrank,cpt)
   CALL mpi_comm_split(MPI_COMM_WORLD, color, myrank, realcomm, ierr)
   IF (color == 0) THEN
   CALL mpi_comm_rank( realcomm, myrank, ierr )
   CALL mpi_comm_size( realcomm, p, ierr )

   IF (myrank == 0) THEN
    WRITE (*,*) "(PROC_X, PROC_Y)(",px,",",py,")"
    WRITE (*,*) "(jpi, jpj, jpk)(",jpi,",",jpj,",",jpk,")"
   END IF

   IF (p .ne. px*py) THEN
      WRITE (*,*) "Error! the total number of processes does not match with the values of PROC_X and PROC_Y env variables"
      WRITE (*,*) "size=",p," PROC_X=",px, "PROC_Y", py
      CALL EXIT()
   ENDIF

   hpmid = myrank + 10
   nowe = myrank - px
   noea = myrank + px
   noso = myrank - 1
   nono = myrank + 1
   IF (py == 1) THEN
      nbondi = 2
   ELSE IF (myrank < px) THEN
      nbondi = -1
   ELSE IF (myrank >= px*(py-1)) THEN
      nbondi = 1
   ELSE
      nbondi = 0
   END IF

   IF (px == 1) THEN
      nbondj = 2
   ELSE IF (MOD(myrank, px) == px-1) THEN
      nbondj = 1
   ELSE IF (MOD(myrank, px) == 0) THEN
      nbondj = -1
   ELSE
      nbondj = 0
   END IF


   jpreci=1
   jprecj=1
   nlci = jpi
   nlcj = jpj
   nreci = 2*jpreci
   nrecj = 2*jprecj
   ALLOCATE( mydomain (jpi,jpj,jpk))
   ALLOCATE( zwx (jpi,jpj,jpk))
!   ALLOCATE( zwx1 (jpi,jpj,jpk))
   ALLOCATE( zwy (jpi,jpj,jpk))
   ALLOCATE( zslpx (jpi,jpj,jpk))
   ALLOCATE( zslpy (jpi,jpj,jpk))
   ALLOCATE( pun (jpi,jpj,jpk))
   ALLOCATE( pvn (jpi,jpj,jpk))
   ALLOCATE( pwn (jpi,jpj,jpk))
   ALLOCATE( umask (jpi,jpj,jpk))
   ALLOCATE( vmask (jpi,jpj,jpk))
   ALLOCATE( tmask (jpi,jpj,jpk))
   ALLOCATE( zind (jpi,jpj,jpk))
   ALLOCATE( ztfreez (jpi,jpj))
   ALLOCATE( rnfmsk (jpi,jpj))
   ALLOCATE( upsmsk (jpi,jpj))
   ALLOCATE( rnfmsk_z (jpk))
   ALLOCATE( tsn(jpi,jpj,jpk))
   ALLOCATE( t3ew(jpj,jpreci,jpk+1,2)) ! adding one element for notification
   ALLOCATE( t3we(jpj,jpreci,jpk+1,2))
   ALLOCATE( t3ns(jpi,jprecj,jpk+1,2))
   ALLOCATE( t3sn(jpi,jprecj,jpk+1,2))


! array initialization

   zwx(:,:,:)   = 0.e0
   zwy(:,:,:)   = 0.e0
   zslpx(:,:,:) = 0.e0
   zslpy(:,:,:) = 0.e0
   zind(:,:,:)  = 0.e0

   DO jk = 1, jpk
      DO jj = 1, jpj
          DO ji = 1, jpi
              umask(ji,jj,jk) = ji*jj*jk
              mydomain(ji,jj,jk) =ji*jj*jk
              pun(ji,jj,jk) =ji*jj*jk
              pvn(ji,jj,jk) =ji*jj*jk
              pwn(ji,jj,jk) =ji*jj*jk
              vmask(ji,jj,jk)= ji*jj*jk
              tsn(ji,jj,jk)= ji*jj*jk
              tmask(ji,jj,jk)= ji*jj*jk
           END DO
      END DO
   END DO

   DO jj=1, jpj
      DO ji=1, jpi
         ztfreez(ji,jj)=ji*jj
         upsmsk(ji,jj)=ji*jj
         rnfmsk(ji,jj) = ji*jj
      END DO
   END DO

   DO jk=1, jpk
      rnfmsk_z(jk)=jk
   END DO




!!!!! Windows !!!!!

   call cpu_time(temps_win)

   call MPI_TYPE_GET_EXTENT(MPI_real,lowerbound,realextent,ierr)
   IF( ierr /= MPI_SUCCESS) THEN 
      WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
   END IF

   disp_int = realextent
   size = jprecj*(jpk+1)*jpi*realextent
   call mpi_win_create(t3ns(1,1,1,2), size, disp_int, MPI_INFO_NULL, realcomm, wint3ns, ierr)
   IF( ierr /= MPI_SUCCESS) THEN 
      WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
   END IF

   call mpi_win_create(t3sn(1,1,1,2), size, disp_int, MPI_INFO_NULL, realcomm, wint3sn, ierr)
   IF( ierr /= MPI_SUCCESS) THEN 
      WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
   END IF

   size= jpreci*jpj*(jpk+1)*realextent
   call mpi_win_create(t3ew(1,1,1,2), size, disp_int, MPI_INFO_NULL, realcomm, wint3ew, ierr)
   IF( ierr /= MPI_SUCCESS) THEN 
      WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
   END IF

   call mpi_win_create(t3we(1,1,1,2), size, disp_int, MPI_INFO_NULL, realcomm, wint3we, ierr)
   IF( ierr /= MPI_SUCCESS) THEN 
      WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
   END IF
   
   t3sn(jpi,jprecj,jpk+1,1)=1
   t3ns(jpi,jprecj,jpk+1,1)=1
   t3we(jpj,jpreci,jpk+1,1)=1
   t3ew(jpj,jpreci,jpk+1,1)=1

   t3sn(jpi,jprecj,jpk+1,2)=0
   t3ns(jpi,jprecj,jpk+1,2)=0
   t3we(jpj,jpreci,jpk+1,2)=0
   t3ew(jpj,jpreci,jpk+1,2)=0



   call mpi_win_lock_all(0, wint3ns, ierr)
   IF( ierr /= MPI_SUCCESS) THEN 
      WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
   END IF
   call mpi_win_lock_all(0, wint3sn, ierr)
   IF( ierr /= MPI_SUCCESS) THEN 
      WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
   END IF

   call mpi_win_lock_all(0, wint3ew, ierr)
   IF( ierr /= MPI_SUCCESS) THEN 
      WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
   END IF

   call mpi_win_lock_all(0, wint3we, ierr)
   IF( ierr /= MPI_SUCCESS) THEN 
      WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
   END IF

   WRITE (*,*) "rank", myrank, "nbondi:", nbondi, " nbondj:", nbondj, "has locked all"
   CALL flush(6)
   CALL mpi_barrier(realcomm, ierr)
   IF( ierr /= MPI_SUCCESS) THEN 
      WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
   END IF

   call cpu_time(temps_win_tot)
   temps_win_tot=temps_win_tot-temps_win

   
!***********************
!* Start of the synphony
!***********************


   DO jk = 1, jpk
      DO jj = 1, jpj
         DO ji = 1, jpi
            IF( tsn(ji,jj,jk) <= ztfreez(ji,jj) + 0.1 ) THEN   ;   zice = 1.e0
            ELSE                                               ;   zice = 0.e0
            ENDIF
            zind(ji,jj,jk) = MAX (   &
               rnfmsk(ji,jj) * rnfmsk_z(jk),      &
               upsmsk(ji,jj)               ,      &
               zice                               &
               &                  ) * tmask(ji,jj,jk)
               zind(ji,jj,jk) = 1 - zind(ji,jj,jk)
         END DO
      END DO
   END DO

   zwx(:,:,jpk) = 0.e0   ;   zwy(:,:,jpk) = 0.e0

   DO jk = 1, jpk-1
      DO jj = 1, jpj-1
         DO ji = 1, jpi-1
             zwx(ji,jj,jk) = umask(ji,jj,jk) * ( mydomain(ji+1,jj,jk) - mydomain(ji,jj,jk) )
             zwy(ji,jj,jk) = vmask(ji,jj,jk) * ( mydomain(ji,jj+1,jk) - mydomain(ji,jj,jk) )
         END DO
      END DO
   END DO


   CALL cpu_time(temps_test_deb)
   temps_p1 = temps_test_deb - temps_init
   CALL mpp_lnk_3d_test(zwx, 1)
   CALL cpu_time(temps_test)
   temps_com1= temps_test - temps_test_deb
   WRITE (*,*) "rank: ", myrank, " reached second data exchange"
   CALL flush(6)

   CALL mpp_lnk_3d_test(zwy, 2)
   CALL cpu_time(temps_test_deb)
   temps_com2 = temps_test_deb - temps_test

   zslpx(:,:,jpk) = 0.e0   ;   zslpy(:,:,jpk) = 0.e0

   DO jk = 1, jpk-1
     DO jj = 2, jpj
        DO ji = 2, jpi
           zslpx(ji,jj,jk) =                    ( zwx(ji,jj,jk) + zwx(ji-1,jj  ,jk) )   &
           &            * ( 0.25 + SIGN( 0.25, zwx(ji,jj,jk) * zwx(ji-1,jj  ,jk) ) )
           zslpy(ji,jj,jk) =                    ( zwy(ji,jj,jk) + zwy(ji  ,jj-1,jk) )   &
           &            * ( 0.25 + SIGN( 0.25, zwy(ji,jj,jk) * zwy(ji  ,jj-1,jk) ) )
        END DO
     END DO
  END DO

  DO jk = 1, jpk-1
     DO jj = 2, jpj
        DO ji = 2, jpi
           zslpx(ji,jj,jk) = SIGN( 1., zslpx(ji,jj,jk) ) * MIN(    ABS( zslpx(ji  ,jj,jk) ),   &
           &                                                 2.*ABS( zwx  (ji-1,jj,jk) ),   &
           &                                                 2.*ABS( zwx  (ji  ,jj,jk) ) )
           zslpy(ji,jj,jk) = SIGN( 1., zslpy(ji,jj,jk) ) * MIN(    ABS( zslpy(ji,jj  ,jk) ),   &
           &                                                 2.*ABS( zwy  (ji,jj-1,jk) ),   &
           &                                                 2.*ABS( zwy  (ji,jj  ,jk) ) )
        END DO
     END DO
  END DO

  DO jk = 1, jpk-1
     zdt  = 1
     DO jj = 2, jpj-1
        DO ji = 2, jpi-1
            z0u = SIGN( 0.5, pun(ji,jj,jk) )
            zalpha = 0.5 - z0u
            zu  = z0u - 0.5 * pun(ji,jj,jk) * zdt

            zzwx = mydomain(ji+1,jj,jk) + zind(ji,jj,jk) * (zu * zslpx(ji+1,jj,jk))
            zzwy = mydomain(ji  ,jj,jk) + zind(ji,jj,jk) * (zu * zslpx(ji  ,jj,jk))

            zwx(ji,jj,jk) = pun(ji,jj,jk) * ( zalpha * zzwx + (1.-zalpha) * zzwy )

            z0v = SIGN( 0.5, pvn(ji,jj,jk) )
            zalpha = 0.5 - z0v
            zv  = z0v - 0.5 * pvn(ji,jj,jk) * zdt

            zzwx = mydomain(ji,jj+1,jk) + zind(ji,jj,jk) * (zv * zslpy(ji,jj+1,jk))
            zzwy = mydomain(ji,jj  ,jk) + zind(ji,jj,jk) * (zv * zslpy(ji,jj  ,jk))

            zwy(ji,jj,jk) = pvn(ji,jj,jk) * ( zalpha * zzwx + (1.-zalpha) * zzwy )
         END DO
      END DO
  END DO



  CALL cpu_time(temps_test)
  temps_p2 = temps_test - temps_test_deb
  CALL mpp_lnk_3d_test(zwx, 3)
  CALL cpu_time(temps_test_deb)
  temps_com3 = temps_test_deb - temps_test
  WRITE (*,*) "rank: ", myrank, " reached fourth data exchange"
  CALL flush(6)

  CALL mpp_lnk_3d_test(zwy, 4)
  CALL cpu_time(temps_test)
  temps_com4 = temps_test - temps_test_deb


  DO jk = 1, jpk-1
     DO jj = 2, jpj-1
        DO ji = 2, jpi-1
           zbtr = 1.
           ztra = - zbtr * ( zwx(ji,jj,jk) - zwx(ji-1,jj  ,jk  )   &
           &               + zwy(ji,jj,jk) - zwy(ji  ,jj-1,jk  ) )
           mydomain(ji,jj,jk) = mydomain(ji,jj,jk) + ztra
        END DO
     END DO
  END DO

  zwx (:,:, 1 ) = 0.e0    ;    zwx (:,:,jpk) = 0.e0

  DO jk = 2, jpk-1
     zwx(:,:,jk) = tmask(:,:,jk) * ( mydomain(:,:,jk-1) - mydomain(:,:,jk) )
  END DO

  zslpx(:,:,1) = 0.e0

  DO jk = 2, jpk-1
     DO jj = 1, jpj
        DO ji = 1, jpi
           zslpx(ji,jj,jk) =                    ( zwx(ji,jj,jk) + zwx(ji,jj,jk+1) )   &
           &            * ( 0.25 + SIGN( 0.25, zwx(ji,jj,jk) * zwx(ji,jj,jk+1) ) )
        END DO
     END DO
  END DO

  DO jk = 2, jpk-1
     DO jj = 1, jpj
        DO ji = 1, jpi
           zslpx(ji,jj,jk) = SIGN( 1., zslpx(ji,jj,jk) ) * MIN(    ABS( zslpx(ji,jj,jk  ) ),   &
           &                                                 2.*ABS( zwx  (ji,jj,jk+1) ),   &
           &                                                 2.*ABS( zwx  (ji,jj,jk  ) )  )
        END DO
     END DO
  END DO

  zwx(:,:, 1 ) = pwn(:,:,1) * mydomain(:,:,1)

  zdt  = 1
  zbtr = 1.
  DO jk = 1, jpk-1
     DO jj = 2, jpj-1
        DO ji = 2, jpi-1
           z0w = SIGN( 0.5, pwn(ji,jj,jk+1) )
           zalpha = 0.5 + z0w
           zw  = z0w - 0.5 * pwn(ji,jj,jk+1) * zdt * zbtr

           zzwx = mydomain(ji,jj,jk+1) + zind(ji,jj,jk) * (zw * zslpx(ji,jj,jk+1))
           zzwy = mydomain(ji,jj,jk  ) + zind(ji,jj,jk) * (zw * zslpx(ji,jj,jk  ))

           zwx(ji,jj,jk+1) = pwn(ji,jj,jk+1) * ( zalpha * zzwx + (1.-zalpha) * zzwy )
        END DO
     END DO
  END DO

  zbtr = 1.
  DO jk = 1, jpk-1
     DO jj = 2, jpj-1
        DO ji = 2, jpi-1
           ztra = - zbtr * ( zwx(ji,jj,jk) - zwx(ji,jj,jk+1) )
           mydomain(ji,jj,jk) =  mydomain(ji,jj,jk) + ztra
        END DO
     END DO
  END DO

  END IF

  WRITE (*,*) "rank: ", myrank, " reached the end"
  CALL flush(6)
  call mpi_barrier(realcomm, ierr)
  IF( ierr /= MPI_SUCCESS) THEN 
     WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
  END IF

  ! call mpi_win_sync(wint3sn,ierr)
  ! call mpi_win_sync(wint3ns,ierr)
  ! call mpi_win_sync(wint3we,ierr)
  ! call mpi_win_sync(wint3ew,ierr)

  call mpi_win_unlock_all(wint3sn, ierr)
  call mpi_win_unlock_all(wint3ns, ierr)
  call mpi_win_unlock_all(wint3we, ierr)
  call mpi_win_unlock_all(wint3ew, ierr)




  call cpu_time(temps_delloc)

  call mpi_win_free(wint3sn,ierr)
  call mpi_win_free(wint3ns,ierr)
  call mpi_win_free(wint3we,ierr)
  call mpi_win_free(wint3ew,ierr)

  
  call cpu_time(temps_delloc_tot)
  temps_delloc_tot=temps_delloc_tot-temps_delloc

  DEALLOCATE( mydomain )
  DEALLOCATE( zwx )
  DEALLOCATE( zwy )
  DEALLOCATE( zslpx )
  DEALLOCATE( zslpy )
  DEALLOCATE( pun )
  DEALLOCATE( pvn )
  DEALLOCATE( pwn )
  DEALLOCATE( umask)
  DEALLOCATE( vmask)
  DEALLOCATE( tmask)
  DEALLOCATE( zind )
  DEALLOCATE( ztfreez )
  DEALLOCATE( rnfmsk)
  DEALLOCATE( upsmsk)
  DEALLOCATE( rnfmsk_z)
  DEALLOCATE( tsn)
  if (allocated(t3ew))DEALLOCATE( t3ew)
  if (allocated(t3we))DEALLOCATE( t3we)
  if (allocated(t3sn))DEALLOCATE( t3sn)
  if (allocated(t3ns))DEALLOCATE( t3ns)

  call cpu_time(temps_fini)
  temps_p3 = temps_fini-temps_test
  temps_tot = temps_fini-temps_init

  if (myrank == 0) then
     write (*,*) "rank, temps_total, temps_p1, temps_p2" , &
       ", temps_p3, temps_com1, temps_com2" , &
       ", temps_com3, temps_com4"


  end if
  call mpi_barrier(realcomm,ierr)
  write (*,*)  myrank, ", ", temps_tot, ", ", temps_p1, ", ",temps_p2, ", ", &
       temps_p3, ", ", temps_com1, ", ",temps_com2, ", ", &
       temps_com3, ", ",temps_com4




  CALL mpi_finalize(ierr)

CONTAINS

  SUBROUTINE mpp_lnk_3d_test( ptab , nbmpp )
      REAL, DIMENSION(jpi,jpj,jpk), INTENT(inout) ::   ptab
      INTEGER, INTENT(in)  :: nbmpp
      !!
      INTEGER  ::   ji, jj, jk, jl
      INTEGER  ::   imigr, iihom, ijhom
      INTEGER  ::   ml_req1, ml_req2, ml_err
      INTEGER, DIMENSION(mpi_status_size) ::   ml_stat
      INTEGER :: iflag
      INTEGER :: istatus(mpi_status_size)
      DOUBLE PRECISION :: t1, t2

      SELECT CASE ( nbondi )
      CASE ( -1, 0, 1 )
         iihom = nlci-nreci
         DO jl = 1, jpreci
            t3ew(:,jl,1:jpk,1) = ptab(jpreci+jl,:,:)
            t3we(:,jl,1:jpk,1) = ptab(iihom +jl,:,:)
         END DO
         t3ew(jpj,jpreci,jpk+1,1) = nbmpp
         t3we(jpj,jpreci,jpk+1,1) = nbmpp
      END SELECT


      imigr = jpreci * jpj * (jpk+1)
      deplacement= 0
      SELECT CASE ( nbondi )
      CASE ( -1 )
         WRITE (*,*) "before put1, rank=" , myrank, ", dest= ", noea
         CALL flush(6)
         CALL mpi_put(t3we(1,1,1,1), imigr, mpi_real, noea, deplacement, imigr, mpi_real, wint3we,ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         call mpi_win_flush(noea,wint3we,ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_barrier(realcomm, ierr)

         WRITE (*,*) "after flush1, rank=" , myrank
         DO WHILE(t3ew(jpj,jpreci,jpk+1,2) /= nbmpp)
            IF (t3ew(jpj,jpreci,jpk+1,2) > nbmpp) THEN 
               WRITE(*,*) "rank", myrank, "received phase", t3ew(jpj,jpreci,jpk+1,2), "to soon !"
               CALL flush(6)
               CALL MPI_Abort(realcomm, ierr)
            END IF
         END DO
         WRITE (*,*) "after notif1, rank=" , myrank


      CASE ( 0 )
         WRITE (*,*) "before put2, rank=" , myrank, ", dest= ", noea, ", dest2=", nowe
         CALL flush(6)
         CALL mpi_put(t3we(1,1,1,1),imigr,mpi_real,noea,deplacement,imigr,mpi_real,wint3we,ierr)       
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_put(t3ew(1,1,1,1),imigr,mpi_real,nowe,deplacement,imigr,mpi_real,wint3ew,ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         call mpi_win_flush(noea,wint3we,ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         call mpi_win_flush(nowe,wint3ew,ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_barrier(realcomm, ierr)

         WRITE (*,*) "after flush2, rank=" , myrank
         DO WHILE(t3ew(jpj,jpreci,jpk+1,2) /= nbmpp .OR. t3we(jpj,jpreci,jpk+1,2) /= nbmpp)
            IF (t3ew(jpj,jpreci,jpk+1,2) > nbmpp .OR. t3we(jpj,jpreci,jpk+1,2) > nbmpp) THEN 
               WRITE(*,*) "rank", myrank, "a received phase to soon !"
               CALL flush(6)
               CALL MPI_Abort(realcomm, ierr)
            END IF
         END DO

         WRITE (*,*) "after notif2, rank=" , myrank
      CASE ( 1 )
         WRITE (*,*) "before put3, rank=" , myrank, ", dest= ", nowe
         CALL flush(6)
         CALL mpi_put(t3ew(1,1,1,1), imigr, mpi_real, nowe, deplacement, imigr, mpi_real, wint3ew, ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         call mpi_win_flush(nowe, wint3ew, ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_barrier(realcomm, ierr)

         WRITE (*,*) "after flush3, rank=" , myrank
         DO WHILE(t3we(jpj,jpreci,jpk+1,2) /= nbmpp)
            IF (t3we(jpj,jpreci,jpk+1,2) > nbmpp) THEN 
               WRITE(*,*) "rank", myrank, "received phase", t3we(jpj,jpreci,jpk+1,2), "to soon !"
               CALL flush(6)
               CALL MPI_Abort(realcomm, ierr)
            END IF
         END DO

         WRITE (*,*) "after notif3, rank=" , myrank
         
      END SELECT


      iihom = nlci-jpreci
      SELECT CASE ( nbondi )
      CASE ( -1 )
         DO jl = 1, jpreci
            ptab(iihom+jl,:,:) = t3ew(:,jl,1:jpk,2)
         END DO
      CASE ( 0 )
         DO jl = 1, jpreci
            ptab(jl      ,:,:) = t3we(:,jl,1:jpk,2)
            ptab(iihom+jl,:,:) = t3ew(:,jl,1:jpk,2)
         END DO
      CASE ( 1 )
         DO jl = 1, jpreci
            ptab(jl      ,:,:) = t3we(:,jl,1:jpk,2)
         END DO
      END SELECT

      IF( nbondj /= 2 ) THEN
         ijhom = nlcj-nrecj
         DO jl = 1, jprecj
            t3sn(:,jl,1:jpk,1) = ptab(:,ijhom +jl,:)
            t3ns(:,jl,1:jpk,1) = ptab(:,jprecj+jl,:)
         END DO
      ENDIF

      t3sn(jpi,jprecj,jpk+1,1) = nbmpp
      t3ns(jpi,jprecj,jpk+1,1) = nbmpp

      imigr = jprecj * jpi * (jpk+1)
      deplacement=0
      SELECT CASE ( nbondj )
      CASE ( -1)
         WRITE (*,*) "before put4, rank=" , myrank, ", dest= ", nono
         CALL flush(6)
         CALL mpi_put(t3sn(1,1,1,1), imigr, mpi_real, nono, deplacement, imigr, mpi_real, wint3sn, ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_win_flush(nono, wint3sn, ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_barrier(realcomm, ierr)

         WRITE (*,*) "after flush4, rank=" , myrank
         DO WHILE(t3ns(jpi, jprecj, jpk+1, 2) /= nbmpp)
            IF ( t3ns(jpi, jprecj, jpk+1, 2) > nbmpp ) THEN 
               WRITE (*,*) "rank", myrank, "phase error"
               CALL flush(6)
               CALL MPI_Abort(realcomm, ierr)
            END IF
         END DO
         WRITE (*,*) "after notif4, rank=" , myrank

      CASE ( 0 )
         WRITE (*,*) "before put5, rank=" , myrank, ", dest= ", nono, ", dest2=", noso
         CALL flush(6)
         CALL mpi_put(t3sn(1,1,1,1), imigr, mpi_real, nono,deplacement, imigr, mpi_real, wint3sn, ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_put(t3ns(1,1,1,1), imigr, mpi_real, noso,deplacement, imigr, mpi_real, wint3ns, ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_win_flush(noso,wint3ns,ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_win_flush(nono,wint3sn,ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_barrier(realcomm, ierr)

         WRITE (*,*) "after flush5, rank=" , myrank
         do while(t3sn(jpi, jprecj, jpk+1, 2) /= nbmpp .OR. t3ns(jpi, jprecj, jpk+1, 2) /= nbmpp)
            IF (t3sn(jpi, jprecj, jpk+1, 2) > nbmpp .OR. t3ns(jpi, jprecj, jpk+1, 2) > nbmpp) THEN 
               WRITE(*,*) "rank", myrank, "phase error"
               CALL flush(6)
               CALL MPI_Abort(realcomm, ierr)
            END IF
            
         end do
         WRITE (*,*) "after notif5, rank=" , myrank
         t3sn(jpi, jprecj, jpk+1, 2) = 0
         t3ns(jpi, jprecj, jpk+1, 2) = 0

      CASE ( 1 )
         WRITE (*,*) "before put6, rank=" , myrank, ", dest=", noso
         CALL flush(6)
         CALL mpi_put(t3ns(1,1,1,1), imigr, mpi_real, noso,deplacement, imigr, mpi_real, wint3ns, ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_win_flush(noso,wint3ns,ierr)
         IF( ierr /= MPI_SUCCESS) THEN 
            WRITE (*,*) "MPI_ERROR line:", __LINE__ , ", rank: ", myrank
         END IF

         CALL mpi_barrier(realcomm, ierr)

         WRITE (*,*) "after flush6, rank=" , myrank
         DO WHILE(t3sn(jpi,jprecj,jpk+1,2) /= nbmpp)
            IF (t3sn(jpi,jprecj,jpk+1,2) > nbmpp) THEN 
               WRITE (*,*) "rank", myrank , "phase error"
               CALL flush(6)
               CALL MPI_Abort(realcomm,ierr)
            END IF
         END DO
         WRITE (*,*) "after notif6, rank=" , myrank
         t3sn(jpi,jprecj,jpk+1,2)=0
      END SELECT

      ijhom = nlcj-jprecj
      SELECT CASE ( nbondj )
      CASE ( -1 )
         DO jl = 1, jprecj
            ptab(:,ijhom+jl,:) = t3ns(:,jl,:,2)
         END DO
      CASE ( 0 )
         DO jl = 1, jprecj
            ptab(:,jl      ,:) = t3sn(:,jl,:,2)
            ptab(:,ijhom+jl,:) = t3ns(:,jl,:,2)
         END DO
      CASE ( 1 )
         DO jl = 1, jprecj
            ptab(:,jl,:) = t3sn(:,jl,:,2)
         END DO
      END SELECT

   END SUBROUTINE mpp_lnk_3d_test



END PROGRAM tra_adv
