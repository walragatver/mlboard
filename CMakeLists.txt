cmake_minimum_required(VERSION 3.3.2)
project(mlboard 
        LANGUAGES C CXX)

# Set required C++ standard to C++11.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include modules in the CMake directory.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")

option(BUILD_TESTS "Build tests." ON)
option(KEEP_TEST_LOGS "Keep the files generated during test." OFF)
option(USE_OPENMP "If available, use OpenMP for parallelization." OFF)

include(FindProtobuf)
# Armadillo 

find_package(Armadillo 8.400.0 REQUIRED)

# If Armadillo was compiled without ARMA_64BIT_WORD and we are on a 64-bit
# system (where size_t will be 64 bits), suggest to the user that they should
# compile Armadillo with 64-bit words.  Note that with Armadillo 5.000.0 and
# newer, ARMA_64BIT_WORD is enabled by default.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  # Check the version, to see if ARMA_64BIT_WORD is enabled by default.
  set(ARMA_HAS_64BIT_WORD 0)
  if(NOT (${ARMADILLO_VERSION_MAJOR} LESS 5))
    set(ARMA_HAS_64BIT_WORD 1)
  else()
    # Can we open the configuration file?  If not, issue a warning.
    if(NOT EXISTS "${ARMADILLO_INCLUDE_DIR}/armadillo_bits/config.hpp")
      message(WARNING "Armadillo configuration file "
          "(${ARMADILLO_INCLUDE_DIR}/armadillo_bits/config.hpp) does not exist!")
    else()
      # We are on a 64-bit system.  Does Armadillo have ARMA_64BIT_WORD enabled?
      file(READ "${ARMADILLO_INCLUDE_DIR}/armadillo_bits/config.hpp" ARMA_CONFIG)
      string(REGEX MATCH
          "[\r\n][ ]*#define ARMA_64BIT_WORD"
          ARMA_HAS_64BIT_WORD_PRE
          "${ARMA_CONFIG}")

      string(LENGTH "${ARMA_HAS_64BIT_WORD_PRE}" ARMA_HAS_64BIT_WORD)
    endif()
  endif()

  if(ARMA_HAS_64BIT_WORD EQUAL 0)
    message(WARNING "This is a 64-bit system, but Armadillo was compiled "
        "without 64-bit index support.  Consider recompiling Armadillo with "
        "ARMA_64BIT_WORD to enable 64-bit indices (large matrix support). "
        "mlpack will still work without ARMA_64BIT_WORD defined, but will not "
        "scale to matrices with more than 4 billion elements.")
  endif()
else()
  # If we are on a 32-bit system, we must manually specify the size of the word
  # to be 32 bits, since otherwise Armadillo will produce a warning that it is
  # disabling 64-bit support.
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-DARMA_32BIT_WORD)
  endif ()
endif()

# Remove this when serialization is removed from mlpack.
set(Boost_ADDITIONAL_VERSIONS
  "1.49.0" "1.50.0" "1.51.0" "1.52.0" "1.53.0" "1.54.0" "1.55.0")
find_package(Boost 1.49
    COMPONENTS
      serialization
    REQUIRED
)

# In Visual Studio, automatic linking is performed, so we don't need to worry
# about it.  Clear the list of libraries to link against and let Visual Studio
# handle it.
if (MSVC)
  link_directories(${Boost_LIBRARY_DIRS})
  set(Boost_LIBRARIES "")
endif ()

link_directories(${Boost_LIBRARY_DIRS})

if (WIN32)
  find_package(LAPACK REQUIRED)
  find_package(BLAS REQUIRED)

  # Piggyback LAPACK and BLAS linking into Armadillo link.
  set(ARMADILLO_LIBRARIES
      ${ARMADILLO_LIBRARIES} ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
endif ()

find_package(MLPACK REQUIRED)

find_package(Protobuf REQUIRED)

add_subdirectory(include/proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(MLBOARD_INCLUDE_DIRS ${MLPACK_INCLUDE_DIR}
                        ${ARMADILLO_INCLUDE_DIR}
                        ${PROTOBUF_INCLUDE_DIR}
                        )

if (KEEP_TEST_LOGS)
        add_definitions(-DKEEP_TEST_LOGS)
        message("keep logs is ${KEEP_TEST_LOGS}")
endif()

if (USE_OPENMP)
  message(WARNING "Only use OpenMP if mlpack has been compiled with OpenMP.")
  find_package(OpenMP)
endif ()

if (OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

include_directories(${MLBOARD_INCLUDE_DIRS})

file(GLOB headerfiles "${CMAKE_CURRENT_SOURCE_DIR}/include/filewriter/*.hpp")

file(GLOB datafiles "${CMAKE_CURRENT_SOURCE_DIR}/examples/assets/*.*")

file(COPY ${datafiles}
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data/)

file(COPY ${headerfiles}
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/mlboard/filewriter/)

file(GLOB headerfiles "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

file(COPY ${headerfiles}
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/mlboard/)

add_library(mlboard INTERFACE)
include_directories(${CMAKE_INCLUDE_PATH})
target_include_directories(mlboard INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mlboard)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/proto
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/proto/libproto.a
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

# Enable testing and build tests.
enable_testing()
if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
