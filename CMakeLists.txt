project(htree)
cmake_minimum_required(VERSION 3.0)

enable_testing()

set(HTREE_VERSION_MAJOR "1")
set(HTREE_VERSION_MINOR "0")


add_definitions( -DHTREE_VERSION_MAJOR=\"${HTREE_VERSION_MAJOR}\" -DHTREE_VERSION_MINOR=\"${HTREE_VERSION_MINOR}\")


list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
      ${PROJECT_SOURCE_DIR}/CMake/portability
      ${PROJECT_SOURCE_DIR}/CMake/packages
      ${PROJECT_SOURCE_DIR}/CMake/config)


include(BlueGenePortability)
include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)
include(DetectCXX11Support)


## Dependencies
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()


set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_VECTORIZE}")

file(GLOB HTREE_HEADERS_1 "${CMAKE_CURRENT_SOURCE_DIR}/include/htree/*.hpp")
file(GLOB HTREE_HEADERS_2 "${CMAKE_CURRENT_SOURCE_DIR}/include/htree/*/*.hpp")


set(htree_version_src "${CMAKE_CURRENT_SOURCE_DIR}/src/htree.cpp")
add_executable(htree ${htree_version_src} ${HTREE_HEADERS_1} ${HTREE_HEADERS_2})



include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include/  )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/deps/digestpp/  )
include_directories(SYSTEM ${BOOST_INCLUDE_DIR} )



