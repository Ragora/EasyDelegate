# If we're on GCC, set the c++11 compilation flag.
IF (CMAKE_COMPILER_IS_GNUCXX)
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

ADD_BII_TARGETS() 
