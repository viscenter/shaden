CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(confidenceConnected)

# Find ITK.
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

# Edit this line to change which example program to compile.
ADD_EXECUTABLE(confidenceConnected confidenceConnected.cxx )

TARGET_LINK_LIBRARIES(confidenceConnected ITKCommon ITKIO )
