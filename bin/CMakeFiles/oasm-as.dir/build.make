# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/home/sylar/Git Repos/OASM/src"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/sylar/Git Repos/OASM/bin"

# Include any dependencies generated for this target.
include CMakeFiles/oasm-as.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/oasm-as.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/oasm-as.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/oasm-as.dir/flags.make

CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.o: CMakeFiles/oasm-as.dir/flags.make
CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.o: /home/sylar/Git\ Repos/OASM/src/frontend/oasm_frontend.cpp
CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.o: CMakeFiles/oasm-as.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/sylar/Git Repos/OASM/bin/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.o -MF CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.o.d -o CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.o -c "/home/sylar/Git Repos/OASM/src/frontend/oasm_frontend.cpp"

CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/sylar/Git Repos/OASM/src/frontend/oasm_frontend.cpp" > CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.i

CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/sylar/Git Repos/OASM/src/frontend/oasm_frontend.cpp" -o CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.s

CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.o: CMakeFiles/oasm-as.dir/flags.make
CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.o: /home/sylar/Git\ Repos/OASM/src/assembler/Assembler.cpp
CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.o: CMakeFiles/oasm-as.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/sylar/Git Repos/OASM/bin/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.o -MF CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.o.d -o CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.o -c "/home/sylar/Git Repos/OASM/src/assembler/Assembler.cpp"

CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/sylar/Git Repos/OASM/src/assembler/Assembler.cpp" > CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.i

CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/sylar/Git Repos/OASM/src/assembler/Assembler.cpp" -o CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.s

CMakeFiles/oasm-as.dir/common/BitEditor.cpp.o: CMakeFiles/oasm-as.dir/flags.make
CMakeFiles/oasm-as.dir/common/BitEditor.cpp.o: /home/sylar/Git\ Repos/OASM/src/common/BitEditor.cpp
CMakeFiles/oasm-as.dir/common/BitEditor.cpp.o: CMakeFiles/oasm-as.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/sylar/Git Repos/OASM/bin/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/oasm-as.dir/common/BitEditor.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/oasm-as.dir/common/BitEditor.cpp.o -MF CMakeFiles/oasm-as.dir/common/BitEditor.cpp.o.d -o CMakeFiles/oasm-as.dir/common/BitEditor.cpp.o -c "/home/sylar/Git Repos/OASM/src/common/BitEditor.cpp"

CMakeFiles/oasm-as.dir/common/BitEditor.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/oasm-as.dir/common/BitEditor.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/sylar/Git Repos/OASM/src/common/BitEditor.cpp" > CMakeFiles/oasm-as.dir/common/BitEditor.cpp.i

CMakeFiles/oasm-as.dir/common/BitEditor.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/oasm-as.dir/common/BitEditor.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/sylar/Git Repos/OASM/src/common/BitEditor.cpp" -o CMakeFiles/oasm-as.dir/common/BitEditor.cpp.s

CMakeFiles/oasm-as.dir/common/Common.cpp.o: CMakeFiles/oasm-as.dir/flags.make
CMakeFiles/oasm-as.dir/common/Common.cpp.o: /home/sylar/Git\ Repos/OASM/src/common/Common.cpp
CMakeFiles/oasm-as.dir/common/Common.cpp.o: CMakeFiles/oasm-as.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/sylar/Git Repos/OASM/bin/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/oasm-as.dir/common/Common.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/oasm-as.dir/common/Common.cpp.o -MF CMakeFiles/oasm-as.dir/common/Common.cpp.o.d -o CMakeFiles/oasm-as.dir/common/Common.cpp.o -c "/home/sylar/Git Repos/OASM/src/common/Common.cpp"

CMakeFiles/oasm-as.dir/common/Common.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/oasm-as.dir/common/Common.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/sylar/Git Repos/OASM/src/common/Common.cpp" > CMakeFiles/oasm-as.dir/common/Common.cpp.i

CMakeFiles/oasm-as.dir/common/Common.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/oasm-as.dir/common/Common.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/sylar/Git Repos/OASM/src/common/Common.cpp" -o CMakeFiles/oasm-as.dir/common/Common.cpp.s

CMakeFiles/oasm-as.dir/common/Flags.cpp.o: CMakeFiles/oasm-as.dir/flags.make
CMakeFiles/oasm-as.dir/common/Flags.cpp.o: /home/sylar/Git\ Repos/OASM/src/common/Flags.cpp
CMakeFiles/oasm-as.dir/common/Flags.cpp.o: CMakeFiles/oasm-as.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/sylar/Git Repos/OASM/bin/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/oasm-as.dir/common/Flags.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/oasm-as.dir/common/Flags.cpp.o -MF CMakeFiles/oasm-as.dir/common/Flags.cpp.o.d -o CMakeFiles/oasm-as.dir/common/Flags.cpp.o -c "/home/sylar/Git Repos/OASM/src/common/Flags.cpp"

CMakeFiles/oasm-as.dir/common/Flags.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/oasm-as.dir/common/Flags.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/sylar/Git Repos/OASM/src/common/Flags.cpp" > CMakeFiles/oasm-as.dir/common/Flags.cpp.i

CMakeFiles/oasm-as.dir/common/Flags.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/oasm-as.dir/common/Flags.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/sylar/Git Repos/OASM/src/common/Flags.cpp" -o CMakeFiles/oasm-as.dir/common/Flags.cpp.s

CMakeFiles/oasm-as.dir/common/OmniaString.cpp.o: CMakeFiles/oasm-as.dir/flags.make
CMakeFiles/oasm-as.dir/common/OmniaString.cpp.o: /home/sylar/Git\ Repos/OASM/src/common/OmniaString.cpp
CMakeFiles/oasm-as.dir/common/OmniaString.cpp.o: CMakeFiles/oasm-as.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/sylar/Git Repos/OASM/bin/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/oasm-as.dir/common/OmniaString.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/oasm-as.dir/common/OmniaString.cpp.o -MF CMakeFiles/oasm-as.dir/common/OmniaString.cpp.o.d -o CMakeFiles/oasm-as.dir/common/OmniaString.cpp.o -c "/home/sylar/Git Repos/OASM/src/common/OmniaString.cpp"

CMakeFiles/oasm-as.dir/common/OmniaString.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/oasm-as.dir/common/OmniaString.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/sylar/Git Repos/OASM/src/common/OmniaString.cpp" > CMakeFiles/oasm-as.dir/common/OmniaString.cpp.i

CMakeFiles/oasm-as.dir/common/OmniaString.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/oasm-as.dir/common/OmniaString.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/sylar/Git Repos/OASM/src/common/OmniaString.cpp" -o CMakeFiles/oasm-as.dir/common/OmniaString.cpp.s

CMakeFiles/oasm-as.dir/common/Utils.cpp.o: CMakeFiles/oasm-as.dir/flags.make
CMakeFiles/oasm-as.dir/common/Utils.cpp.o: /home/sylar/Git\ Repos/OASM/src/common/Utils.cpp
CMakeFiles/oasm-as.dir/common/Utils.cpp.o: CMakeFiles/oasm-as.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/sylar/Git Repos/OASM/bin/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/oasm-as.dir/common/Utils.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/oasm-as.dir/common/Utils.cpp.o -MF CMakeFiles/oasm-as.dir/common/Utils.cpp.o.d -o CMakeFiles/oasm-as.dir/common/Utils.cpp.o -c "/home/sylar/Git Repos/OASM/src/common/Utils.cpp"

CMakeFiles/oasm-as.dir/common/Utils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/oasm-as.dir/common/Utils.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/sylar/Git Repos/OASM/src/common/Utils.cpp" > CMakeFiles/oasm-as.dir/common/Utils.cpp.i

CMakeFiles/oasm-as.dir/common/Utils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/oasm-as.dir/common/Utils.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/sylar/Git Repos/OASM/src/common/Utils.cpp" -o CMakeFiles/oasm-as.dir/common/Utils.cpp.s

CMakeFiles/oasm-as.dir/common/Keyboard.cpp.o: CMakeFiles/oasm-as.dir/flags.make
CMakeFiles/oasm-as.dir/common/Keyboard.cpp.o: /home/sylar/Git\ Repos/OASM/src/common/Keyboard.cpp
CMakeFiles/oasm-as.dir/common/Keyboard.cpp.o: CMakeFiles/oasm-as.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/sylar/Git Repos/OASM/bin/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/oasm-as.dir/common/Keyboard.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/oasm-as.dir/common/Keyboard.cpp.o -MF CMakeFiles/oasm-as.dir/common/Keyboard.cpp.o.d -o CMakeFiles/oasm-as.dir/common/Keyboard.cpp.o -c "/home/sylar/Git Repos/OASM/src/common/Keyboard.cpp"

CMakeFiles/oasm-as.dir/common/Keyboard.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/oasm-as.dir/common/Keyboard.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/sylar/Git Repos/OASM/src/common/Keyboard.cpp" > CMakeFiles/oasm-as.dir/common/Keyboard.cpp.i

CMakeFiles/oasm-as.dir/common/Keyboard.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/oasm-as.dir/common/Keyboard.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/sylar/Git Repos/OASM/src/common/Keyboard.cpp" -o CMakeFiles/oasm-as.dir/common/Keyboard.cpp.s

# Object files for target oasm-as
oasm__as_OBJECTS = \
"CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.o" \
"CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.o" \
"CMakeFiles/oasm-as.dir/common/BitEditor.cpp.o" \
"CMakeFiles/oasm-as.dir/common/Common.cpp.o" \
"CMakeFiles/oasm-as.dir/common/Flags.cpp.o" \
"CMakeFiles/oasm-as.dir/common/OmniaString.cpp.o" \
"CMakeFiles/oasm-as.dir/common/Utils.cpp.o" \
"CMakeFiles/oasm-as.dir/common/Keyboard.cpp.o"

# External object files for target oasm-as
oasm__as_EXTERNAL_OBJECTS =

oasm-as: CMakeFiles/oasm-as.dir/frontend/oasm_frontend.cpp.o
oasm-as: CMakeFiles/oasm-as.dir/assembler/Assembler.cpp.o
oasm-as: CMakeFiles/oasm-as.dir/common/BitEditor.cpp.o
oasm-as: CMakeFiles/oasm-as.dir/common/Common.cpp.o
oasm-as: CMakeFiles/oasm-as.dir/common/Flags.cpp.o
oasm-as: CMakeFiles/oasm-as.dir/common/OmniaString.cpp.o
oasm-as: CMakeFiles/oasm-as.dir/common/Utils.cpp.o
oasm-as: CMakeFiles/oasm-as.dir/common/Keyboard.cpp.o
oasm-as: CMakeFiles/oasm-as.dir/build.make
oasm-as: liboasm-lib.so
oasm-as: CMakeFiles/oasm-as.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/home/sylar/Git Repos/OASM/bin/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_9) "Linking CXX executable oasm-as"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/oasm-as.dir/link.txt --verbose=$(VERBOSE)
	/usr/bin/cmake -E copy_directory "/home/sylar/Git Repos/OASM/src/../extra/" "/home/sylar/Git Repos/OASM/bin"

# Rule to build all files generated by this target.
CMakeFiles/oasm-as.dir/build: oasm-as
.PHONY : CMakeFiles/oasm-as.dir/build

CMakeFiles/oasm-as.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/oasm-as.dir/cmake_clean.cmake
.PHONY : CMakeFiles/oasm-as.dir/clean

CMakeFiles/oasm-as.dir/depend:
	cd "/home/sylar/Git Repos/OASM/bin" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/sylar/Git Repos/OASM/src" "/home/sylar/Git Repos/OASM/src" "/home/sylar/Git Repos/OASM/bin" "/home/sylar/Git Repos/OASM/bin" "/home/sylar/Git Repos/OASM/bin/CMakeFiles/oasm-as.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/oasm-as.dir/depend

