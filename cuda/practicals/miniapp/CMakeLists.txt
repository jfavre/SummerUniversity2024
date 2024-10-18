cmake_minimum_required(VERSION 3.20)
project (MiniAppCuda CUDA C CXX)

include (GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

# since we use C++11 in this example.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CUDA_HOST_COMPILER g++)
set(CUDA_SRCS
  stats.cu
  data.cu
  operators.cu
  linalg.cu
  main.cu
)
      
find_package(OpenMP)

set(INSITU None CACHE STRING "Enable in-situ support")
set_property(CACHE INSITU PROPERTY STRINGS None Catalyst Ascent)

set(CMAKE_CUDA_FLAGS_RELEASE "-O3 -DNDEBUG -I/usr/local/cuda-12.2/include/")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-L/usr/local/cuda-12.2/targets/x86_64-linux/lib")

include(CTest)
if(INSITU STREQUAL "None")
  add_executable(miniapp ${CUDA_SRCS})
  target_link_libraries(miniapp PRIVATE cublas OpenMP::OpenMP_CXX)
elseif(INSITU STREQUAL "Catalyst")
  add_executable(miniapp ${CUDA_SRCS} CatalystAdaptor.cu)
  find_package(catalyst REQUIRED PATHS "${EBROOTCATALYST}/lib/cmake/catalyst-2.0")
  target_compile_definitions(miniapp PUBLIC USE_CATALYST=1)
  target_link_libraries(miniapp PRIVATE cublas OpenMP::OpenMP_CXX catalyst::catalyst)

  if(BUILD_TESTING)
  set(miniapp_data_dump_directory "${CMAKE_CURRENT_BINARY_DIR}/datasets")
  # remove old contents, if any.
  add_test(NAME test_prepare COMMAND ${CMAKE_COMMAND} -E rm -rf "${miniapp_data_dump_directory}")
  set_tests_properties(test_prepare PROPERTIES FIXTURES_SETUP prepare)

  # Add a test for writing out images and datasets
  add_test(NAME exec_miniapp COMMAND ${mpi_prefix_or_blank} "$<TARGET_FILE:miniapp>" 256 256 100 0.001 --pv ../catalyst_state.py)
  set_tests_properties(exec_miniapp
    PROPERTIES
      ENVIRONMENT CATALYST_IMPLEMENTATION_NAME=paraview
      FIXTURES_SETUP write_out
      FIXTURES_REQUIRED prepare)

  # Add a test for reading out images
  add_test(NAME verify_catalyst_output COMMAND eog ${miniapp_data_dump_directory})
  set_tests_properties(verify_catalyst_output
    PROPERTIES
      FIXTURES_REQUIRED write_out)
  endif(BUILD_TESTING)
elseif(INSITU STREQUAL "Ascent")
  find_package(Ascent REQUIRED PATHS "/local/apps/Ascent-cuda/install/ascent-checkout/lib/cmake/ascent")
  add_executable(miniapp ${CUDA_SRCS} AscentAdaptor.cu)
  set_target_properties(miniapp PROPERTIES CUDA_ARCHITECTURES "86")
  target_compile_definitions(miniapp PRIVATE USE_ASCENT=1)
  target_link_libraries(miniapp PRIVATE cublas OpenMP::OpenMP_CXX ascent::ascent)
  
  if(BUILD_TESTING)
  set(miniapp_data_dump_directory "${CMAKE_CURRENT_BINARY_DIR}/datasets")
  # remove old contents, if any.
  add_test(NAME test_prepare COMMAND mkdir -p "${miniapp_data_dump_directory}")
  set_tests_properties(test_prepare PROPERTIES FIXTURES_SETUP prepare)

  # Add a test for writing out images and datasets
  add_test(NAME exec_miniapp COMMAND "$<TARGET_FILE:miniapp>" 256 256 100 0.001)
  set_tests_properties(exec_miniapp
    PROPERTIES
      FIXTURES_SETUP write_out
      FIXTURES_REQUIRED prepare)

  # Add a test for reading out images
  add_test(NAME verify_ascent_output COMMAND eog ${miniapp_data_dump_directory})
  set_tests_properties(verify_ascent_output
    PROPERTIES
      FIXTURES_REQUIRED write_out)
  endif(BUILD_TESTING)
endif()
