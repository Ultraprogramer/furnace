set(TARGET_PREFIX ${CMAKE_SYSTEM_PROCESSOR}-w64-mingw32)

set(CMAKE_C_COMPILER ${TARGET_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TARGET_PREFIX}-g++)
set(PKG_CONFIG_EXECUTABLE ${TARGET_PREFIX}-pkg-config)

set(CMAKE_FIND_ROOT_PATH /usr/${TARGET_PREFIX})

# Search host system for programs
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Search target system for libraries, headers & CMake packages
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
