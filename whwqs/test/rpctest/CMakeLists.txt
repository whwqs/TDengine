CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(TDengine)

INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/src/rpc/inc)

LIST(APPEND SRC ./src/testMain.c)
ADD_EXECUTABLE(testrpc ${SRC})
TARGET_LINK_LIBRARIES(testrpc trpc)
IF (TD_LINUX)
	ADD_SUBDIRECTORY(test)
ENDIF ()