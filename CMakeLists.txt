# CMakeLists.txt
# Used for building the example code on any CMake supported system.
# This software is licensed under the MIT license. Please refer to
# LICENSE.txt for more information.

CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (EasyDelegate)

SET (EX_BUILDLOCATION "ex_delegateset")

# If we're on windows, make the output name have the exe extension
IF (WIN32)
SET (EX_BUILDLOCATION "${EX_BUILDLOCATION}.exe")
ENDIF (WIN32)

# If we're on GCC, set the c++11 compilation flag.
IF (CMAKE_COMPILER_IS_GNUCXX)
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

INCLUDE_DIRECTORIES ("include/")
ADD_EXECUTABLE (${EX_BUILDLOCATION} "ex_delegateset.cpp")
