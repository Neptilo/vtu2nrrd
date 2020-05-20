cmake_minimum_required(VERSION 3.10.2)

project(vtu2nrrd)

find_package(ITK REQUIRED COMPONENTS
  ITKVtkGlue
  ITKIOImageBase
  ITKIONRRD)
include(${ITK_USE_FILE})

find_package(VTK COMPONENTS 
  vtkCommonCore
  vtkCommonDataModel
  vtkIOXML
  vtkIOLegacy
  vtkFiltersCore
  #vtkFiltersParallelDIY2
  vtkIOImage QUIET)
if (NOT VTK_FOUND)
  message("Skipping vtu2nrrd: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
endif()
message (STATUS "VTK_VERSION: ${VTK_VERSION}")
if (VTK_VERSION VERSION_LESS "8.90.0")
  # old system
  include(${VTK_USE_FILE})
  add_executable(vtu2nrrd MACOSX_BUNDLE main.cxx )
  target_link_libraries(vtu2nrrd PRIVATE ${ITK_LIBRARIES} ${VTK_LIBRARIES})
else ()
  # include all components
  add_executable(vtu2nrrd MACOSX_BUNDLE main.cxx )
  target_link_libraries(vtu2nrrd PRIVATE ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  # vtk_module_autoinit is needed
  vtk_module_autoinit(
    TARGETS vtu2nrrd
    MODULES ${VTK_LIBRARIES}
    )
endif () 
