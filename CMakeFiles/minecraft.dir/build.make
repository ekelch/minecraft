# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

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
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.30.3/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.30.3/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/evankelch/Programming/minecraft

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/evankelch/Programming/minecraft

# Include any dependencies generated for this target.
include CMakeFiles/minecraft.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/minecraft.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/minecraft.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/minecraft.dir/flags.make

CMakeFiles/minecraft.dir/src/main.cpp.o: CMakeFiles/minecraft.dir/flags.make
CMakeFiles/minecraft.dir/src/main.cpp.o: src/main.cpp
CMakeFiles/minecraft.dir/src/main.cpp.o: CMakeFiles/minecraft.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/evankelch/Programming/minecraft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/minecraft.dir/src/main.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/minecraft.dir/src/main.cpp.o -MF CMakeFiles/minecraft.dir/src/main.cpp.o.d -o CMakeFiles/minecraft.dir/src/main.cpp.o -c /Users/evankelch/Programming/minecraft/src/main.cpp

CMakeFiles/minecraft.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/minecraft.dir/src/main.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/evankelch/Programming/minecraft/src/main.cpp > CMakeFiles/minecraft.dir/src/main.cpp.i

CMakeFiles/minecraft.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/minecraft.dir/src/main.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/evankelch/Programming/minecraft/src/main.cpp -o CMakeFiles/minecraft.dir/src/main.cpp.s

# Object files for target minecraft
minecraft_OBJECTS = \
"CMakeFiles/minecraft.dir/src/main.cpp.o"

# External object files for target minecraft
minecraft_EXTERNAL_OBJECTS =

bin/minecraft: CMakeFiles/minecraft.dir/src/main.cpp.o
bin/minecraft: CMakeFiles/minecraft.dir/build.make
bin/minecraft: /opt/homebrew/lib/libSDL2main.a
bin/minecraft: /opt/homebrew/lib/libSDL2.dylib
bin/minecraft: /opt/homebrew/lib/libSDL2_ttf.dylib
bin/minecraft: /opt/homebrew/lib/libSDL2_ttf.dylib
bin/minecraft: /opt/homebrew/lib/libSDL2_image.dylib
bin/minecraft: /opt/homebrew/lib/libSDL2_mixer.dylib
bin/minecraft: CMakeFiles/minecraft.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/evankelch/Programming/minecraft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable bin/minecraft"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/minecraft.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/minecraft.dir/build: bin/minecraft
.PHONY : CMakeFiles/minecraft.dir/build

CMakeFiles/minecraft.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/minecraft.dir/cmake_clean.cmake
.PHONY : CMakeFiles/minecraft.dir/clean

CMakeFiles/minecraft.dir/depend:
	cd /Users/evankelch/Programming/minecraft && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/evankelch/Programming/minecraft /Users/evankelch/Programming/minecraft /Users/evankelch/Programming/minecraft /Users/evankelch/Programming/minecraft /Users/evankelch/Programming/minecraft/CMakeFiles/minecraft.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/minecraft.dir/depend

