# CMakeLists.txt
# Used for building the example code on any CMake supported system.
# This software is licensed under the MIT license. Please refer to
# LICENSE.txt for more information.

IF(BIICODE)
   INCLUDE("biicode.cmake")
   RETURN()
ENDIF()

CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (EasyDelegate)

# If we're on GCC, set the c++11 compilation flag.
IF (CMAKE_COMPILER_IS_GNUCXX)
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

SET (EX_BUILDLOCATION "example")

INCLUDE_DIRECTORIES ("include/")
ADD_EXECUTABLE (${EX_BUILDLOCATION} "example.cpp"
"include/easydelegate/deferredcallers.hpp"
"include/easydelegate/delegateset.hpp"
"include/easydelegate/easydelegate.hpp"
"include/easydelegate/exceptions.hpp"
"include/easydelegate/mainpage.h"
"include/easydelegate/delegates.hpp"
"include/easydelegate/types.hpp"

"include/easydelegate/delegatesCompat.hpp"
"include/easydelegate/delegatesetCompat.hpp"
)
