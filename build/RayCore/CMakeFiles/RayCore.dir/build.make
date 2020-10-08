# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_COMMAND = "C:/Program Files/CMake/bin/cmake.exe"

# The command to remove a file.
RM = "C:/Program Files/CMake/bin/cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:/Users/Work/Documents/Semesterprojekt/RAYReworked

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build

# Include any dependencies generated for this target.
include RayCore/CMakeFiles/RayCore.dir/depend.make

# Include the progress variables for this target.
include RayCore/CMakeFiles/RayCore.dir/progress.make

# Include the compile flags for this target's objects.
include RayCore/CMakeFiles/RayCore.dir/flags.make

RayCore/CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.obj: RayCore/CMakeFiles/RayCore.dir/flags.make
RayCore/CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.obj: ../RayCore/src/RayCore/Application.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object RayCore/CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.obj"
	cd C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/RayCore && C:/msys64/mingw32/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.obj -c C:/Users/Work/Documents/Semesterprojekt/RAYReworked/RayCore/src/RayCore/Application.cpp

RayCore/CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.i"
	cd C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/RayCore && C:/msys64/mingw32/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:/Users/Work/Documents/Semesterprojekt/RAYReworked/RayCore/src/RayCore/Application.cpp > CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.i

RayCore/CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.s"
	cd C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/RayCore && C:/msys64/mingw32/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:/Users/Work/Documents/Semesterprojekt/RAYReworked/RayCore/src/RayCore/Application.cpp -o CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.s

# Object files for target RayCore
RayCore_OBJECTS = \
"CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.obj"

# External object files for target RayCore
RayCore_EXTERNAL_OBJECTS =

RayCore/RayCore.dll: RayCore/CMakeFiles/RayCore.dir/src/RayCore/Application.cpp.obj
RayCore/RayCore.dll: RayCore/CMakeFiles/RayCore.dir/build.make
RayCore/RayCore.dll: RayCore/CMakeFiles/RayCore.dir/linklibs.rsp
RayCore/RayCore.dll: RayCore/CMakeFiles/RayCore.dir/objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library RayCore.dll"
	cd C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/RayCore && "C:/Program Files/CMake/bin/cmake.exe" -E rm -f CMakeFiles/RayCore.dir/objects.a
	cd C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/RayCore && C:/msys64/mingw32/bin/ar.exe cr CMakeFiles/RayCore.dir/objects.a @CMakeFiles/RayCore.dir/objects1.rsp
	cd C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/RayCore && C:/msys64/mingw32/bin/g++.exe -g -shared -o RayCore.dll -Wl,--out-implib,libRayCore.dll.a -Wl,--major-image-version,0,--minor-image-version,0 -Wl,--whole-archive CMakeFiles/RayCore.dir/objects.a -Wl,--no-whole-archive @CMakeFiles/RayCore.dir/linklibs.rsp

# Rule to build all files generated by this target.
RayCore/CMakeFiles/RayCore.dir/build: RayCore/RayCore.dll

.PHONY : RayCore/CMakeFiles/RayCore.dir/build

RayCore/CMakeFiles/RayCore.dir/clean:
	cd C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/RayCore && $(CMAKE_COMMAND) -P CMakeFiles/RayCore.dir/cmake_clean.cmake
.PHONY : RayCore/CMakeFiles/RayCore.dir/clean

RayCore/CMakeFiles/RayCore.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" C:/Users/Work/Documents/Semesterprojekt/RAYReworked C:/Users/Work/Documents/Semesterprojekt/RAYReworked/RayCore C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/RayCore C:/Users/Work/Documents/Semesterprojekt/RAYReworked/build/RayCore/CMakeFiles/RayCore.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : RayCore/CMakeFiles/RayCore.dir/depend

