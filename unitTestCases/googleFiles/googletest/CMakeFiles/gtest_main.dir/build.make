# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest

# Include any dependencies generated for this target.
include googleFiles/googletest/CMakeFiles/gtest_main.dir/depend.make

# Include the progress variables for this target.
include googleFiles/googletest/CMakeFiles/gtest_main.dir/progress.make

# Include the compile flags for this target's objects.
include googleFiles/googletest/CMakeFiles/gtest_main.dir/flags.make

googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o: googleFiles/googletest/CMakeFiles/gtest_main.dir/flags.make
googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o: googleFiles/googletest/src/gtest_main.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o"
	cd /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/gtest_main.dir/src/gtest_main.cc.o -c /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest/src/gtest_main.cc

googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gtest_main.dir/src/gtest_main.cc.i"
	cd /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest/src/gtest_main.cc > CMakeFiles/gtest_main.dir/src/gtest_main.cc.i

googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gtest_main.dir/src/gtest_main.cc.s"
	cd /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest/src/gtest_main.cc -o CMakeFiles/gtest_main.dir/src/gtest_main.cc.s

googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o.requires:

.PHONY : googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o.requires

googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o.provides: googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o.requires
	$(MAKE) -f googleFiles/googletest/CMakeFiles/gtest_main.dir/build.make googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o.provides.build
.PHONY : googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o.provides

googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o.provides.build: googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o


# Object files for target gtest_main
gtest_main_OBJECTS = \
"CMakeFiles/gtest_main.dir/src/gtest_main.cc.o"

# External object files for target gtest_main
gtest_main_EXTERNAL_OBJECTS =

googleFiles/googletest/libgtest_main.a: googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o
googleFiles/googletest/libgtest_main.a: googleFiles/googletest/CMakeFiles/gtest_main.dir/build.make
googleFiles/googletest/libgtest_main.a: googleFiles/googletest/CMakeFiles/gtest_main.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libgtest_main.a"
	cd /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest && $(CMAKE_COMMAND) -P CMakeFiles/gtest_main.dir/cmake_clean_target.cmake
	cd /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gtest_main.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
googleFiles/googletest/CMakeFiles/gtest_main.dir/build: googleFiles/googletest/libgtest_main.a

.PHONY : googleFiles/googletest/CMakeFiles/gtest_main.dir/build

googleFiles/googletest/CMakeFiles/gtest_main.dir/requires: googleFiles/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o.requires

.PHONY : googleFiles/googletest/CMakeFiles/gtest_main.dir/requires

googleFiles/googletest/CMakeFiles/gtest_main.dir/clean:
	cd /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest && $(CMAKE_COMMAND) -P CMakeFiles/gtest_main.dir/cmake_clean.cmake
.PHONY : googleFiles/googletest/CMakeFiles/gtest_main.dir/clean

googleFiles/googletest/CMakeFiles/gtest_main.dir/depend:
	cd /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest /home/einfochips/Desktop/HawkEdge/softwareAgent/src/Gtest/googleFiles/googletest/CMakeFiles/gtest_main.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : googleFiles/googletest/CMakeFiles/gtest_main.dir/depend

