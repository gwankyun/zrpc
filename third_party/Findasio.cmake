if(asio_ROOT)
    set(asio_INCLUDE_DIRS ${asio_ROOT}/include)
    set(asio_FOUND TRUE)
    if(NOT TARGET asio::asio)
      add_library(aiso_header_only INTERFACE)
      add_library(asio::asio ALIAS aiso_header_only)
      set_target_properties(aiso_header_only PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${asio_INCLUDE_DIRS}"
      )
      set(asio_VERSION "1.8.1")
      include(FindPackageHandleStandardArgs)
      find_package_handle_standard_args(asio
        FOUND_VAR asio_FOUND
        REQUIRED_VARS asio_INCLUDE_DIRS
        VERSION_VAR asio_VERSION
      )
      set(asio_VERSION_STRING ${asio_VERSION})
    endif()
endif()
