# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /home/mininet/picoquic

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mininet/picoquic

# Include any dependencies generated for this target.
include _deps/picotls-build/CMakeFiles/ptlsbench.dir/depend.make

# Include the progress variables for this target.
include _deps/picotls-build/CMakeFiles/ptlsbench.dir/progress.make

# Include the compile flags for this target's objects.
include _deps/picotls-build/CMakeFiles/ptlsbench.dir/flags.make

_deps/picotls-build/CMakeFiles/ptlsbench.dir/t/ptlsbench.c.o: _deps/picotls-build/CMakeFiles/ptlsbench.dir/flags.make
_deps/picotls-build/CMakeFiles/ptlsbench.dir/t/ptlsbench.c.o: _deps/picotls-src/t/ptlsbench.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mininet/picoquic/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object _deps/picotls-build/CMakeFiles/ptlsbench.dir/t/ptlsbench.c.o"
	cd /home/mininet/picoquic/_deps/picotls-build && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/ptlsbench.dir/t/ptlsbench.c.o   -c /home/mininet/picoquic/_deps/picotls-src/t/ptlsbench.c

_deps/picotls-build/CMakeFiles/ptlsbench.dir/t/ptlsbench.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ptlsbench.dir/t/ptlsbench.c.i"
	cd /home/mininet/picoquic/_deps/picotls-build && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mininet/picoquic/_deps/picotls-src/t/ptlsbench.c > CMakeFiles/ptlsbench.dir/t/ptlsbench.c.i

_deps/picotls-build/CMakeFiles/ptlsbench.dir/t/ptlsbench.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ptlsbench.dir/t/ptlsbench.c.s"
	cd /home/mininet/picoquic/_deps/picotls-build && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mininet/picoquic/_deps/picotls-src/t/ptlsbench.c -o CMakeFiles/ptlsbench.dir/t/ptlsbench.c.s

# Object files for target ptlsbench
ptlsbench_OBJECTS = \
"CMakeFiles/ptlsbench.dir/t/ptlsbench.c.o"

# External object files for target ptlsbench
ptlsbench_EXTERNAL_OBJECTS =

_deps/picotls-build/ptlsbench: _deps/picotls-build/CMakeFiles/ptlsbench.dir/t/ptlsbench.c.o
_deps/picotls-build/ptlsbench: _deps/picotls-build/CMakeFiles/ptlsbench.dir/build.make
_deps/picotls-build/ptlsbench: _deps/picotls-build/libpicotls-minicrypto.a
_deps/picotls-build/ptlsbench: _deps/picotls-build/libpicotls-core.a
_deps/picotls-build/ptlsbench: _deps/picotls-build/libpicotls-openssl.a
_deps/picotls-build/ptlsbench: /usr/lib/x86_64-linux-gnu/libcrypto.so
_deps/picotls-build/ptlsbench: _deps/picotls-build/libpicotls-fusion.a
_deps/picotls-build/ptlsbench: _deps/picotls-build/libpicotls-core.a
_deps/picotls-build/ptlsbench: _deps/picotls-build/CMakeFiles/ptlsbench.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mininet/picoquic/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ptlsbench"
	cd /home/mininet/picoquic/_deps/picotls-build && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ptlsbench.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
_deps/picotls-build/CMakeFiles/ptlsbench.dir/build: _deps/picotls-build/ptlsbench

.PHONY : _deps/picotls-build/CMakeFiles/ptlsbench.dir/build

_deps/picotls-build/CMakeFiles/ptlsbench.dir/clean:
	cd /home/mininet/picoquic/_deps/picotls-build && $(CMAKE_COMMAND) -P CMakeFiles/ptlsbench.dir/cmake_clean.cmake
.PHONY : _deps/picotls-build/CMakeFiles/ptlsbench.dir/clean

_deps/picotls-build/CMakeFiles/ptlsbench.dir/depend:
	cd /home/mininet/picoquic && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mininet/picoquic /home/mininet/picoquic/_deps/picotls-src /home/mininet/picoquic /home/mininet/picoquic/_deps/picotls-build /home/mininet/picoquic/_deps/picotls-build/CMakeFiles/ptlsbench.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : _deps/picotls-build/CMakeFiles/ptlsbench.dir/depend
