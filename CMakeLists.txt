cmake_minimum_required(VERSION 2.8.12)

project(VTKtest C CXX)

find_package(MPI REQUIRED)
find_package(VTK REQUIRED)

add_executable(single single.cpp)
set_target_properties(single PROPERTIES
  INCLUDE_DIRECTORIES "${VTK_INCLUDE_DIRS}")

target_link_libraries(single vtkIOXML)

add_executable(parallel parallel.cpp)

set_target_properties(parallel PROPERTIES
  INCLUDE_DIRECTORIES "${VTK_INCLUDE_DIRS};${MPI_CXX_INCLUDE_PATH}")

target_link_libraries(parallel
  vtkFiltersProgrammable vtkIOParallelXML vtkParallelMPI
  ${MPI_CXX_LIBRARIES})

add_executable(testparallel testparallel.cpp)

set_target_properties(testparallel PROPERTIES
  INCLUDE_DIRECTORIES "${VTK_INCLUDE_DIRS};${MPI_CXX_INCLUDE_PATH}")

target_link_libraries(testparallel
  vtkImagingCore vtkFiltersCore vtkFiltersProgrammable
  vtkIOParallelXML vtkParallelMPI
  ${MPI_CXX_LIBRARIES})
