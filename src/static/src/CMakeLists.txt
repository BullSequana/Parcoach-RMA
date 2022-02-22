add_llvm_library(parcoachRMA MODULE
	parcoachRMA.cpp

  PLUGIN_TOOL
  opt
  )

if(APPLE)
  # The magic line.
  # We're building a shared lib, tell OSX to resolve the symbol when actually
  # loading the library.
  # If we don't set this and manually add the appropriate LLVM libs, loading
  # the plugin will silently fail (!!!) because of duplicate symbols.
  set_target_properties(parcoachRMA PROPERTIES
    LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

set(PARCOACH_LIB ${CMAKE_CURRENT_BINARY_DIR}/parcoachRMA${CMAKE_SHARED_LIBRARY_SUFFIX})
configure_file(parcoach.in ${CMAKE_BINARY_DIR}/parcoach @ONLY)
configure_file(parcoachBC.in ${CMAKE_BINARY_DIR}/parcoachBC @ONLY)

