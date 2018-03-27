project(htree)
cmake_minimum_required(VERSION 2.8)

enable_testing()

set(HTREE_VERSION_MAJOR "1")
set(HTREE_VERSION_MINOR "2")


option(HTREE_STATIC_BUILD "Enable or disable htree static binary" OFF)

add_definitions( -DHTREE_VERSION_MAJOR=\"${HTREE_VERSION_MAJOR}\" -DHTREE_VERSION_MINOR=\"${HTREE_VERSION_MINOR}\")


list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
      ${PROJECT_SOURCE_DIR}/CMake/portability
      ${PROJECT_SOURCE_DIR}/CMake/packages
      ${PROJECT_SOURCE_DIR}/CMake/config)


include(BlueGenePortability)
include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)
include(DetectCXX11Support)

find_package(Threads)

if(HTREE_STATIC_BUILD)
        set(HTREE_STATIC_PREFIX "STATIC_")
        set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS )
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS " -static -static-libgcc -static-libstdc++ -Wl,--whole-archive -lpthread -Wl,--no-whole-archive")
endif()


set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_VECTORIZE}")

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined -fsanitize=object-size")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include/  )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/deps/digestpp/  )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/deps/hadoken/include  )

include_directories(SYSTEM ${BOOST_INCLUDE_DIR} )

set(htree_version_src "${CMAKE_CURRENT_SOURCE_DIR}/src/htree.cpp")
add_executable(htree ${htree_version_src} ${HTREE_HEADERS_1} ${HTREE_HEADERS_2})
target_link_libraries(htree ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS htree DESTINATION ${BIN_INSTALL_DIR}/)



