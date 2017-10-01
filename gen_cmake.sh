#!/bin/bash
# Usefull script to generate project files using cmake

cmake_opt=""

############################ DO NOT MODIFY AFTER THAT LINE #############

# Get absolute folder for this script
selfFolderPath="`cd "${BASH_SOURCE[0]%/*}"; pwd -P`/" # Command to get the absolute path

# Include util functions
. "${selfFolderPath}scripts/utils.sh"

# Default values
default_VisualGenerator="Visual Studio 15 2017"
default_VisualToolset="v141"
default_VisualToolchain="x64"
default_VisualArch="x86"
default_VisualSdk="8.1"

# 
cmake_generator=""
if isMac; then
	cmake_path="/Applications/CMake.app/Contents/bin/cmake"
	# CMake.app not found, use cmake from the path
	if [ ! -f "${cmake_path}" ]; then
		cmake_path="cmake"
	fi
	generator="Xcode"
	getCcArch arch
else
	# Use cmake from the path
	cmake_path="cmake"
	if isWindows; then
		generator="$default_VisualGenerator"
		toolset="$default_VisualToolset"
		toolchain="$default_VisualToolchain"
		platformSdk="$default_VisualSdk"
		arch="$default_VisualArch"
	else
		generator="Unix Makefiles"
		getCcArch arch
	fi
fi

which "${cmake_path}" &> /dev/null
if [ $? -ne 0 ]; then
	echo "CMake not found. Please add CMake binary folder in your PATH environment variable."
	exit 1
fi

outputFolder="./_build"
cmake_config=""
add_cmake_opt=""
outputFolderForced=0
useVSclang=0

while [ $# -gt 0 ]
do
	case "$1" in
		-h)
			echo "Usage: gen_cmake.sh [options]"
			echo " -h -> Display this help"
			echo " -o <folder> -> Output folder (Default: ${outputFolder}_${arch})"
			echo " -f <flags> -> Force all cmake flags (Default: $cmake_opt)"
			echo " -a <flags> -> Add cmake flags to default ones (or to forced ones with -f option)"
			echo " -b <cmake path> -> Force cmake binary path (Default: $cmake_path)"
			echo " -c <cmake generator> -> Force cmake generator (Default: $generator)"
			if isWindows; then
				echo " -t <visual toolset> -> Force visual toolset (Default: $toolset)"
				echo " -tc <visual toolchain> -> Force visual toolchain (Default: $toolchain)"
				echo " -64 -> Generate the 64 bits version of the project (Default: 32)"
				echo " -clang -> Compile using clang for VisualStudio (if predefined toolset do not work, override with -t option INSTEAD of -clang)"
			fi
			if isLinux; then
				echo " -debug -> Force debug configuration (Linux only)"
				echo " -release -> Force release configuration (Linux only)"
			fi
			exit 3
			;;
		-o)
			shift
			if [ $# -lt 1 ]; then
				echo "ERROR: Missing parameter for -o option, see help (-h)"
				exit 4
			fi
			outputFolder="$1"
			outputFolderForced=1
			;;
		-f)
			shift
			if [ $# -lt 1 ]; then
				echo "ERROR: Missing parameter for -f option, see help (-h)"
				exit 4
			fi
			cmake_opt="$1"
			;;
		-a)
			shift
			if [ $# -lt 1 ]; then
				echo "ERROR: Missing parameter for -a option, see help (-h)"
				exit 4
			fi
			add_cmake_opt="$add_cmake_opt $1"
			;;
		-b)
			shift
			if [ $# -lt 1 ]; then
				echo "ERROR: Missing parameter for -b option, see help (-h)"
				exit 4
			fi
			if [ ! -x "$1" ]; then
				echo "ERROR: Specified cmake binary is not valid (not found or not executable): $1"
				exit 4
			fi
			cmake_path="$1"
			;;
		-c)
			shift
			if [ $# -lt 1 ]; then
				echo "ERROR: Missing parameter for -c option, see help (-h)"
				exit 4
			fi
			cmake_generator="$1"
			;;
		-t)
			if isWindows; then
				shift
				if [ $# -lt 1 ]; then
					echo "ERROR: Missing parameter for -t option, see help (-h)"
					exit 4
				fi
				toolset="$1"
			else
				echo "ERROR: -t option is only supported on Windows platform"
				exit 4
			fi
			;;
		-tc)
			if isWindows; then
				shift
				if [ $# -lt 1 ]; then
					echo "ERROR: Missing parameter for -tc option, see help (-h)"
					exit 4
				fi
				toolchain="$1"
			else
				echo "ERROR: -tc option is only supported on Windows platform"
				exit 4
			fi
			;;
		-64)
			if isWindows; then
				generator="Visual Studio 14 2015 Win64"
				arch="x64"
			else
				echo "ERROR: -64 option is only supported on Windows platform"
				exit 4
			fi
			;;
		-clang)
			if isWindows; then
				useVSclang=1
			else
				echo "ERROR: -clang option is only supported on Windows platform"
				exit 4
			fi
			;;
		-debug)
			if isLinux; then
				cmake_config="-DCMAKE_BUILD_TYPE=Debug"
			else
				echo "ERROR: -debug option is only supported on Linux platform"
				exit 4
			fi
			;;
		-release)
			if isLinux; then
				cmake_config="-DCMAKE_BUILD_TYPE=Release"
			else
				echo "ERROR: -release option is only supported on Linux platform"
				exit 4
			fi
			;;
		*)
			echo "ERROR: Unknown option '$1' (use -h for help)"
			exit 4
			;;
	esac
	shift
done

if [ $outputFolderForced -eq 0 ]; then
	outputFolder="${outputFolder}_${arch}"
fi

if [ ! -z "$cmake_generator" ]; then
	echo "Overriding default cmake generator ($generator) with: $cmake_generator"
	generator="$cmake_generator"
fi

# Check if at least a -debug or -release option has been passed on linux
if isLinux; then
	if [ -z $cmake_config ]; then
		echo "ERROR: Linux requires either -debug or -release option to be specified"
		exit 4
	fi
fi

# Using -clang option (shortcut to auto-define the toolset)
if [ $useVSclang -eq 1 ]; then
	toolset="v141_clang_c2"
fi

# Clang on windows does not properly compile using Sdk8.1, we have to force Sdk10.0
shopt -s nocasematch
if [[ isWindows && $toolset =~ clang ]]; then
	platformSdk="10.0"
fi

echo "Generating cmake project..."
"$cmake_path" -H. -B"${outputFolder}" "-G${generator}" $toolset_option $sdk_option $cmake_opt $add_cmake_opt $cmake_config

echo ""
echo "All done, generated project lies in ${outputFolder}"
