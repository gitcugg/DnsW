# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_SOURCE_DIR = /root/dns_t3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/dns_t3

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /root/dns_t3/CMakeFiles /root/dns_t3/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /root/dns_t3/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named dns_TEST

# Build rule for target.
dns_TEST: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 dns_TEST
.PHONY : dns_TEST

# fast build rule for target.
dns_TEST/fast:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/build
.PHONY : dns_TEST/fast

cache.o: cache.cpp.o

.PHONY : cache.o

# target to build an object file
cache.cpp.o:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/cache.cpp.o
.PHONY : cache.cpp.o

cache.i: cache.cpp.i

.PHONY : cache.i

# target to preprocess a source file
cache.cpp.i:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/cache.cpp.i
.PHONY : cache.cpp.i

cache.s: cache.cpp.s

.PHONY : cache.s

# target to generate assembly for a file
cache.cpp.s:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/cache.cpp.s
.PHONY : cache.cpp.s

dns_main.o: dns_main.cpp.o

.PHONY : dns_main.o

# target to build an object file
dns_main.cpp.o:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/dns_main.cpp.o
.PHONY : dns_main.cpp.o

dns_main.i: dns_main.cpp.i

.PHONY : dns_main.i

# target to preprocess a source file
dns_main.cpp.i:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/dns_main.cpp.i
.PHONY : dns_main.cpp.i

dns_main.s: dns_main.cpp.s

.PHONY : dns_main.s

# target to generate assembly for a file
dns_main.cpp.s:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/dns_main.cpp.s
.PHONY : dns_main.cpp.s

epoll.o: epoll.cpp.o

.PHONY : epoll.o

# target to build an object file
epoll.cpp.o:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/epoll.cpp.o
.PHONY : epoll.cpp.o

epoll.i: epoll.cpp.i

.PHONY : epoll.i

# target to preprocess a source file
epoll.cpp.i:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/epoll.cpp.i
.PHONY : epoll.cpp.i

epoll.s: epoll.cpp.s

.PHONY : epoll.s

# target to generate assembly for a file
epoll.cpp.s:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/epoll.cpp.s
.PHONY : epoll.cpp.s

message.o: message.cpp.o

.PHONY : message.o

# target to build an object file
message.cpp.o:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/message.cpp.o
.PHONY : message.cpp.o

message.i: message.cpp.i

.PHONY : message.i

# target to preprocess a source file
message.cpp.i:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/message.cpp.i
.PHONY : message.cpp.i

message.s: message.cpp.s

.PHONY : message.s

# target to generate assembly for a file
message.cpp.s:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/message.cpp.s
.PHONY : message.cpp.s

net.o: net.cpp.o

.PHONY : net.o

# target to build an object file
net.cpp.o:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/net.cpp.o
.PHONY : net.cpp.o

net.i: net.cpp.i

.PHONY : net.i

# target to preprocess a source file
net.cpp.i:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/net.cpp.i
.PHONY : net.cpp.i

net.s: net.cpp.s

.PHONY : net.s

# target to generate assembly for a file
net.cpp.s:
	$(MAKE) -f CMakeFiles/dns_TEST.dir/build.make CMakeFiles/dns_TEST.dir/net.cpp.s
.PHONY : net.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... dns_TEST"
	@echo "... edit_cache"
	@echo "... cache.o"
	@echo "... cache.i"
	@echo "... cache.s"
	@echo "... dns_main.o"
	@echo "... dns_main.i"
	@echo "... dns_main.s"
	@echo "... epoll.o"
	@echo "... epoll.i"
	@echo "... epoll.s"
	@echo "... message.o"
	@echo "... message.i"
	@echo "... message.s"
	@echo "... net.o"
	@echo "... net.i"
	@echo "... net.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

