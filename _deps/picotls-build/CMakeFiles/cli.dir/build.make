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
include _deps/picotls-build/CMakeFiles/cli.dir/depend.make

# Include the progress variables for this target.
include _deps/picotls-build/CMakeFiles/cli.dir/progress.make

# Include the compile flags for this target's objects.
include _deps/picotls-build/CMakeFiles/cli.dir/flags.make

_deps/picotls-build/CMakeFiles/cli.dir/t/cli.c.o: _deps/picotls-build/CMakeFiles/cli.dir/flags.make
_deps/picotls-build/CMakeFiles/cli.dir/t/cli.c.o: _deps/picotls-src/t/cli.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mininet/picoquic/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object _deps/picotls-build/CMakeFiles/cli.dir/t/cli.c.o"
	cd /home/mininet/picoquic/_deps/picotls-build && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/cli.dir/t/cli.c.o   -c /home/mininet/picoquic/_deps/picotls-src/t/cli.c

_deps/picotls-build/CMakeFiles/cli.dir/t/cli.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cli.dir/t/cli.c.i"
	cd /home/mininet/picoquic/_deps/picotls-build && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mininet/picoquic/_deps/picotls-src/t/cli.c > CMakeFiles/cli.dir/t/cli.c.i

_deps/picotls-build/CMakeFiles/cli.dir/t/cli.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cli.dir/t/cli.c.s"
	cd /home/mininet/picoquic/_deps/picotls-build && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mininet/picoquic/_deps/picotls-src/t/cli.c -o CMakeFiles/cli.dir/t/cli.c.s

_deps/picotls-build/CMakeFiles/cli.dir/lib/pembase64.c.o: _deps/picotls-build/CMakeFiles/cli.dir/flags.make
_deps/picotls-build/CMakeFiles/cli.dir/lib/pembase64.c.o: _deps/picotls-src/lib/pembase64.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mininet/picoquic/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object _deps/picotls-build/CMakeFiles/cli.dir/lib/pembase64.c.o"
	cd /home/mininet/picoquic/_deps/picotls-build && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/cli.dir/lib/pembase64.c.o   -c /home/mininet/picoquic/_deps/picotls-src/lib/pembase64.c

_deps/picotls-build/CMakeFiles/cli.dir/lib/pembase64.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cli.dir/lib/pembase64.c.i"
	cd /home/mininet/picoquic/_deps/picotls-build && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mininet/picoquic/_deps/picotls-src/lib/pembase64.c > CMakeFiles/cli.dir/lib/pembase64.c.i

_deps/picotls-build/CMakeFiles/cli.dir/lib/pembase64.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cli.dir/lib/pembase64.c.s"
	cd /home/mininet/picoquic/_deps/picotls-build && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mininet/picoquic/_deps/picotls-src/lib/pembase64.c -o CMakeFiles/cli.dir/lib/pembase64.c.s

# Object files for target cli
cli_OBJECTS = \
"CMakeFiles/cli.dir/t/cli.c.o" \
"CMakeFiles/cli.dir/lib/pembase64.c.o"

# External object files for target cli
cli_EXTERNAL_OBJECTS =

_deps/picotls-build/cli: _deps/picotls-build/CMakeFiles/cli.dir/t/cli.c.o
_deps/picotls-build/cli: _deps/picotls-build/CMakeFiles/cli.dir/lib/pembase64.c.o
_deps/picotls-build/cli: _deps/picotls-build/CMakeFiles/cli.dir/build.make
_deps/picotls-build/cli: _deps/picotls-build/libpicotls-openssl.a
_deps/picotls-build/cli: _deps/picotls-build/libpicotls-core.a
_deps/picotls-build/cli: /usr/lib/x86_64-linux-gnu/libresolv.so
_deps/picotls-build/cli: /usr/lib/x86_64-linux-gnu/libcrypto.so
_deps/picotls-build/cli: _deps/picotls-build/CMakeFiles/cli.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mininet/picoquic/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable cli"
	cd /home/mininet/picoquic/_deps/picotls-build && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cli.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
_deps/picotls-build/CMakeFiles/cli.dir/build: _deps/picotls-build/cli

.PHONY : _deps/picotls-build/CMakeFiles/cli.dir/build

_deps/picotls-build/CMakeFiles/cli.dir/clean:
	cd /home/mininet/picoquic/_deps/picotls-build && $(CMAKE_COMMAND) -P CMakeFiles/cli.dir/cmake_clean.cmake
.PHONY : _deps/picotls-build/CMakeFiles/cli.dir/clean

_deps/picotls-build/CMakeFiles/cli.dir/depend:
	cd /home/mininet/picoquic && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mininet/picoquic /home/mininet/picoquic/_deps/picotls-src /home/mininet/picoquic /home/mininet/picoquic/_deps/picotls-build /home/mininet/picoquic/_deps/picotls-build/CMakeFiles/cli.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : _deps/picotls-build/CMakeFiles/cli.dir/depend
