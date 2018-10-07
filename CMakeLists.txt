cmake_minimum_required(VERSION 2.8.12)

project(VTKtest C CXX)

find_package(MPI REQUIRED)
find_package(VTK REQUIRED COMPONENTS vtkIOXML vtkIOParallelXML vtkParallelMPI)

add_executable(single single.cpp)
set_target_properties(single PROPERTIES
  INCLUDE_DIRECTORIES "${VTK_INCLUDE_DIRS}")

target_link_libraries(single vtkIOXML)

add_executable(parallel parallel.cpp)

set_target_properties(parallel PROPERTIES
  INCLUDE_DIRECTORIES "${VTK_INCLUDE_DIRS};${MPI_CXX_INCLUDE_PATH}")

target_link_libraries(parallel vtkIOParallelXML vtkParallelMPI
  ${MPI_CXX_LIBRARIES})
