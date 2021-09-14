function(target_install target ver)
  include(GNUInstallDirs)

  # install the target and create export-set
  install(TARGETS ${target}
    EXPORT ${target}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  # install header file
  # install(DIRECTORY include/ztest DESTINATION include)

  # generate and install export file
  install(EXPORT ${target}Targets
    FILE ${target}Targets.cmake
    NAMESPACE ${target}::
    DESTINATION lib/cmake/${target}
  )

  # include CMakePackageConfigHelpers macro
  include(CMakePackageConfigHelpers)

  string(REGEX MATCH "^([0-9]+)\..*" v1 ver)

  # set version
  set(version ${ver})
  set(v1 CMAKE_MATCH_1)

  set_property(TARGET ${target} PROPERTY VERSION ${version})
  set_property(TARGET ${target} PROPERTY SOVERSION ${v1})
  set_property(TARGET ${target} PROPERTY
    INTERFACE_${target}_MAJOR_VERSION ${v1})
  set_property(TARGET ${target} APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING ${target}_MAJOR_VERSION
  )

  # generate the version file for the config file
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${target}ConfigVersion.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
  )

  # create config file
  configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${target}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${target}Config.cmake"
    INSTALL_DESTINATION lib/cmake/${target}
  )

  # install config files
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${target}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${target}ConfigVersion.cmake"
    DESTINATION lib/cmake/${target}
  )

  # generate the export targets for the build tree
  export(EXPORT ${target}Targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}Targets.cmake"
    NAMESPACE ${target}::
  )
endfunction()
