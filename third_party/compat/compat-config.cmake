if(NOT TARGET compat)
  add_library(compat INTERFACE IMPORTED)
  set_target_properties(compat PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include"
      INTERFACE_LINK_LIBRARIES "$<$<CXX_COMPILER_ID:Clang>:c++experimental>")
endif()