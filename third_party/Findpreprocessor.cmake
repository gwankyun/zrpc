if(preprocessor_ROOT)
    set(preprocessor_INCLUDE_DIRS
      ${preprocessor_ROOT}/include)
    find_package(Boost REQUIRED)
    set(preprocessor_FOUND TRUE)
      if(NOT TARGET preprocessor::preprocessor)
      add_library(preprocessor_header_only INTERFACE)
      add_library(preprocessor::preprocessor ALIAS preprocessor_header_only)
      set(preprocessor_LIBRARIES preprocessor::preprocessor Boost::headers)
      set_target_properties(preprocessor_header_only PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${preprocessor_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${preprocessor_LIBRARIES}"
      )
    endif()
endif()
