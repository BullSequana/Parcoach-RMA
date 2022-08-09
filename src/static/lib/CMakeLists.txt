set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(rma_analyzer
	 STATIC
	 ../../core/interval.c
	 ../../core/interval_list.c
	 ../../core/interval_avl_tree.c
	 ../../core/notif_avl_tree.c
	 ../../core/rma_analyzer.c
	 rma_analyzer_mpi_c_overload.c
	 rma_analyzer_mpi_fort_overload.c
	 rma_analyzer_load_store_overload.c
)

target_link_libraries(rma_analyzer PRIVATE Threads::Threads)

include_directories(${MPI_C_HEADER_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/core)

add_compile_options(-Wall -fPIC -pthread -Wno-int-to-pointer-cast)
if(COVERAGE)
    add_compile_options(-g --coverage)
    add_link_options(-lgcov --coverage)
endif(COVERAGE)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DEBUG")

