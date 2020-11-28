if(dbg_macro_ROOT)
    set(dbg_macro_INCLUDE_DIRS ${dbg_macro_ROOT})
    set(dbg_macro_FOUND TRUE)
    if(NOT TARGET dbg_macro::dbg_macro)
      add_library(dbg_macro_header_only INTERFACE)
      add_library(dbg_macro::dbg_macro ALIAS dbg_macro_header_only)
      set_target_properties(dbg_macro_header_only PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${dbg_macro_INCLUDE_DIRS}"
      )
      set(dbg_macro_VERSION "1.0.0")
      include(FindPackageHandleStandardArgs)
      find_package_handle_standard_args(dbg_macro
        FOUND_VAR dbg_macro_FOUND
        REQUIRED_VARS dbg_macro_INCLUDE_DIRS
        VERSION_VAR dbg_macro_VERSION
      )
      set(dbg_macro_VERSION_STRING ${dbg_macro_VERSION})
    endif()
endif()
