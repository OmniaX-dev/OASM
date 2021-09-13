# Install script for directory: /home/sylar/Git Repos/OASM/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-vm" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-vm")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-vm"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-vm")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/bin" TYPE EXECUTABLE FILES "/home/sylar/Git Repos/OASM/bin/oasm-vm")
  if(EXISTS "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-vm" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-vm")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-vm"
         OLD_RPATH "/home/sylar/Git Repos/OASM/bin:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-vm")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-as" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-as")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-as"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-as")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/bin" TYPE EXECUTABLE FILES "/home/sylar/Git Repos/OASM/bin/oasm-as")
  if(EXISTS "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-as" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-as")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-as"
         OLD_RPATH "/home/sylar/Git Repos/OASM/bin:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-as")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-dbg" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-dbg")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-dbg"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-dbg")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/bin" TYPE EXECUTABLE FILES "/home/sylar/Git Repos/OASM/bin/oasm-dbg")
  if(EXISTS "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-dbg" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-dbg")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-dbg"
         OLD_RPATH "/home/sylar/Git Repos/OASM/bin:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/oasm-dbg")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/liboasm-lib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/liboasm-lib.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/liboasm-lib.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build/bin/liboasm-lib.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/bin" TYPE SHARED_LIBRARY FILES "/home/sylar/Git Repos/OASM/bin/liboasm-lib.so")
  if(EXISTS "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/liboasm-lib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/liboasm-lib.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/sylar/Git Repos/OASM/src/../Build/bin/liboasm-lib.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build/bin/termcolor-LICENSE")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/bin" TYPE FILE FILES "/home/sylar/Git Repos/OASM/src/../extra/termcolor-LICENSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/assembler/Assembler.hpp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/assembler" TYPE FILE FILES "/home/sylar/Git Repos/OASM/src/assembler/headers/Assembler.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/interpreter/Interpreter.hpp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/interpreter" TYPE FILE FILES "/home/sylar/Git Repos/OASM/src/interpreter/headers/Interpreter.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/debugger/Debugger.hpp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/debugger" TYPE FILE FILES "/home/sylar/Git Repos/OASM/src/debugger/headers/Debugger.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common/BitEditor.hpp;/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common/Common.hpp;/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common/Defines.hpp;/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common/Enums.hpp;/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common/ErrorDefines.hpp;/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common/Flags.hpp;/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common/IOManager.hpp;/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common/OmniaString.hpp;/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common/TermColor.hpp;/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common/Utils.hpp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/include/Omnia/common" TYPE FILE FILES
    "/home/sylar/Git Repos/OASM/src/common/headers/BitEditor.hpp"
    "/home/sylar/Git Repos/OASM/src/common/headers/Common.hpp"
    "/home/sylar/Git Repos/OASM/src/common/headers/Defines.hpp"
    "/home/sylar/Git Repos/OASM/src/common/headers/Enums.hpp"
    "/home/sylar/Git Repos/OASM/src/common/headers/ErrorDefines.hpp"
    "/home/sylar/Git Repos/OASM/src/common/headers/Flags.hpp"
    "/home/sylar/Git Repos/OASM/src/common/headers/IOManager.hpp"
    "/home/sylar/Git Repos/OASM/src/common/headers/OmniaString.hpp"
    "/home/sylar/Git Repos/OASM/src/common/headers/TermColor.hpp"
    "/home/sylar/Git Repos/OASM/src/common/headers/Utils.hpp"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build//doc")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/" TYPE DIRECTORY FILES "/home/sylar/Git Repos/OASM/src/../extra/doc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build//ostd")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build/" TYPE DIRECTORY FILES "/home/sylar/Git Repos/OASM/src/../extra/ostd")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/sylar/Git Repos/OASM/src/../Build/examples")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/sylar/Git Repos/OASM/src/../Build" TYPE DIRECTORY FILES "/home/sylar/Git Repos/OASM/bin/../extra/examples" FILES_MATCHING REGEX "/[^/]*\\.oasm$")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/sylar/Git Repos/OASM/bin/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
