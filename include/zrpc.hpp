#pragma once
//#include <iostream>
//#include <vector>
//#include <string>
//#include <tuple>
//#include <cstddef>
//#include <utility>
//#include <string>
//#include <vector>
//#include <cassert>
//#include <algorithm> // std::min
//#include <memory>
//#include <cstdint> // std::uint32_t
//#include <stdint.h>
//#include <cstring> // memset
//#include <map> // std::map
//#include <functional> // std::function
//#include <utility> // std::move std::forward
//#ifndef ASIO_STANDALONE
//#define ASIO_STANDALONE
//#endif // !ASIO_STANDALONE
//#include <asio.hpp>
//#include <asio/coroutine.hpp>
#include "zrpc/zrpc.h"
#include "zrpc/detail.hpp"
#include "zrpc/serialization.hpp"
#include "zrpc/server.hpp"
#include "zrpc/client.hpp"

//#if ZRPC_HAS_CXX_11
//#include <log.hpp>
//#else
//#if !defined(LOG) && !defined(LOG_IF)
//#define LOG(...)
//#define LOG_IF(...)
//#endif
//#endif // ZRPC_HAS_CXX_11
//
//#ifndef ZRPC_ASSERT
//#define ZRPC_ASSERT(x) assert((x))
////#define ZRPC_ASSERT(x) if (!(x)) { return; }
//#endif // !ZRPC_ASSERT
//
//#ifndef ZRPC_SHARED_PTR
//#if ZRPC_HAS_CXX_11
//#define ZRPC_SHARED_PTR std::shared_ptr
//#endif // ZRPC_HAS_CXX_11
//#endif // !ZRPC_SHARED_PTR
