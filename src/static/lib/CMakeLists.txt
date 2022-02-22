add_library(rma_analyzer
	 STATIC
	 ../../core/interval.c
	 ../../core/interval_list.c
	 ../../core/interval_tree.c
	 ../../core/rma_analyzer.c
	 rma_analyzer_mpi_c_overload.c
	 rma_analyzer_mpi_fort_overload.c
	 rma_analyzer_load_store_overload.c
)


include_directories(${MPI_C_HEADER_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/core)

add_compile_options(-Wno-int-to-pointer-cast)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DEBUG")

