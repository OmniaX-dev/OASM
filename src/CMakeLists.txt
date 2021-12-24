 cmake_minimum_required(VERSION 3.21.2)
 project(OmniaScript CXX)

 file (STRINGS "build.nr" BUILD_NUMBER)
 set(CMAKE_CXX_STANDARD 17)
 
 set(bin_dir "bin")
 
 file(MAKE_DIRECTORY ${bin_dir})

 include(assembler/CMakeLists.txt)
 include(debugger/CMakeLists.txt)
 include(common/CMakeLists.txt)
 include(frontend/CMakeLists.txt)
 include(interpreter/CMakeLists.txt)

 add_executable(oasm-vm ${frontend_SOURCES} ${vm_SOURCES} ${common_SOURCES})
 add_executable(oasm-as ${frontend_SOURCES} ${as_SOURCES} ${common_SOURCES})
 add_executable(oasm-dbg ${frontend_SOURCES} ${dbg_SOURCES} ${common_SOURCES})
 add_library(oasm-lib SHARED ${vm_SOURCES} ${as_SOURCES} ${dbg_SOURCES} ${common_SOURCES})

 add_compile_options(-m32 -MMD -MP -Wall -ggdb)

 target_compile_definitions(oasm-vm PUBLIC __COMPILE_VM__ __DEBUG__ __BUILD_NUMBER__=${BUILD_NUMBER} __MAJOR_VER__=0 __MINOR_VER__=3)
 target_compile_definitions(oasm-as PUBLIC __COMPILE_AS__ __DEBUG__ __BUILD_NUMBER__=${BUILD_NUMBER} __MAJOR_VER__=0 __MINOR_VER__=3)
 target_compile_definitions(oasm-dbg PUBLIC __COMPILE_DBG__ __DEBUG__ __BUILD_NUMBER__=${BUILD_NUMBER} __MAJOR_VER__=0 __MINOR_VER__=3)
 target_compile_definitions(oasm-lib PUBLIC __DEBUG__ __BUILD_NUMBER__=${BUILD_NUMBER} __MAJOR_VER__=0 __MINOR_VER__=3)

 set(CMAKE_BUILD_TYPE Debug)

 include_directories(assembler/headers debugger/headers common/headers interpreter/headers)
if (WIN32)
	 target_link_libraries(oasm-vm -lncursesw)
	 target_link_libraries(oasm-as -lncursesw)
	 target_link_libraries(oasm-dbg -lncursesw)
	 target_link_libraries(oasm-lib -lncursesw)
endif (WIN32)
if (UNIX)
     target_link_libraries(oasm-vm -lcurses)
	 target_link_libraries(oasm-as -lcurses)
	 target_link_libraries(oasm-dbg -lcurses)
	 target_link_libraries(oasm-lib -lcurses)
endif (UNIX)

 target_link_libraries(oasm-vm oasm-lib)
 target_link_libraries(oasm-as oasm-lib)
 target_link_libraries(oasm-dbg oasm-lib)


add_custom_command(TARGET oasm-lib POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/../extra/ $<TARGET_FILE_DIR:oasm-lib>
VERBATIM
)
add_custom_command(TARGET oasm-as POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/../extra/ $<TARGET_FILE_DIR:oasm-as>
VERBATIM
)
add_custom_command(TARGET oasm-vm POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/../extra/ $<TARGET_FILE_DIR:oasm-vm>
VERBATIM
)
add_custom_command(TARGET oasm-dbg POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/../extra/ $<TARGET_FILE_DIR:oasm-dbg>
VERBATIM
)
if (WIN32)
	add_custom_command ( OUTPUT build.nr
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/inc_bnr.exe
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	)
endif (WIN32)
if (UNIX)
	add_custom_command ( OUTPUT build.nr
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/inc_bnr
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	)
endif (UNIX)

add_custom_target(
  IncBnr ALL
  DEPENDS build.nr
)

install(TARGETS oasm-vm oasm-as oasm-dbg oasm-lib DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../Build/bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../extra/termcolor-LICENSE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../Build/bin)
install(FILES ${as_HEADERS} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../Build/include/Omnia/assembler)
install(FILES ${vm_HEADERS} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../Build/include/Omnia/interpreter)
install(FILES ${dbg_HEADERS} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../Build/include/Omnia/debugger)
install(FILES ${common_HEADERS} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../Build/include/Omnia/common)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../extra/doc DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../Build/)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../extra/ostd DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../Build/)
install(DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/../extra/examples DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../Build
    FILES_MATCHING PATTERN "*.oasm")
