if(zrpc_ROOT)
    set(zrpc_INCLUDE_DIRS
      ${zrpc_ROOT}/include
      ${zrpc_ROOT}/include/zrpc
      ${asio_ROOT}/include)
      find_package(Boost REQUIRED)
      find_package(msgpack REQUIRED)
      find_package(apply REQUIRED)
      set(zrpc_LIBRARIES msgpackc apply::apply Boost::headers)
	  set(zrpc_FOUND TRUE)
      if(NOT TARGET zrpc::zrpc)
        add_library(zrpc_header_only INTERFACE)
        add_library(zrpc::zrpc ALIAS zrpc_header_only)
        set_target_properties(zrpc_header_only PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${zrpc_INCLUDE_DIRS}"
          INTERFACE_LINK_LIBRARIES "${zrpc_LIBRARIES}"
      )
    endif()
endif()
