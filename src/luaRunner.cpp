/*
* Copyright (C) 2017-2020, Christophe Calmejane

* This file is part of LuaRunner.

* LuaRunner is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* LuaRunner is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.

* You should have received a copy of the GNU Lesser General Public License
* along with LuaRunner.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "luaRunner/execute.hpp"
#include "luaRunner/version.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void printHelp()
{
	std::cout << "LuaRunner v" << luaRunner::getVersion() << " usage:" << std::endl;
	std::cout << "  LuaRunner [Options] <lua script to execute> [lua script parameters]" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h -> Display this help and exit" << std::endl;
	std::cout << "  -v -> Display version and exit" << std::endl;
	std::cout << "  -p <Name of plugin to load[:entry point]> -> Load specified plugin before executing the lua script. Multiple '-p' options can be specified to load multiple plugins. You can optionally provide the entry point, or InitPlugin will be used." << std::endl;
	std::cout << "  -s <Plugins search path> -> Search path for plugins. Multiple '-s' options can be specified to add multiple search paths." << std::endl;
	std::cout << "Returned value:" << std::endl;
	std::cout << "  255: Parameter error" << std::endl;
	std::cout << "  254: Plugin load error" << std::endl;
	std::cout << "  253: Script error" << std::endl;
	std::cout << "  252: Invalid script returned value: must either be nothing or an integer value between 0 and 127 (inclusive)" << std::endl;
	std::cout << "  0-127: Script returned value (0 by default)" << std::endl;
	std::cout << "lua script parameters will be pushed to the index-based table named 'argv' along with the integer value 'argc' which represents the number of arguments. The first element is always the script file (like in a C program)." << std::endl;
	std::cout << "(Note that table indexes start at 1 in lua, not 0)" << std::endl;
}

int main(int argc, char const* argv[])
{
	std::vector<std::string> pluginsToLoad{};
	std::vector<std::string> pluginsSearchPaths{};
	std::string scriptToExecute{};
	luaRunner::execute::Executor::ScriptParameters scriptsParameters{};

	// Parse arguments
	decltype(argc) argPos{ 1 };
	while (argPos < argc)
	{
		auto const arg = std::string(argv[argPos]);

		// This is an option
		if (arg.length() > 1 && arg[0] == '-')
		{
			if (arg == "-h")
			{
				printHelp();
				return 0;
			}
			else if (arg == "-v")
			{
				std::cout << "LuaRunner version v" << luaRunner::getVersion() << std::endl;
				return 0;
			}
			else if (arg == "-p")
			{
				// This option requires an additional argument
				auto const currentPos = argPos;
				++argPos;
				if (argPos >= argc)
				{
					std::cout << "Missing parameter for '-p' option." << std::endl << std::endl;
					printHelp();
					return 255;
				}
				pluginsToLoad.push_back(argv[currentPos + 1]);
			}
			else if (arg == "-s")
			{
				// This option requires an additional argument
				auto const currentPos = argPos;
				++argPos;
				if (argPos >= argc)
				{
					std::cout << "Missing parameter for '-s' option." << std::endl << std::endl;
					printHelp();
					return 255;
				}
				pluginsSearchPaths.push_back(argv[currentPos + 1]);
			}
		}
		// This is the script to execute
		else
		{
			scriptToExecute = arg;
			// Now parse script parameters (all remaining arguments)
			++argPos;
			while (argPos < argc)
			{
				auto const scriptArg = std::string(argv[argPos]);
				scriptsParameters.push_back(scriptArg);
				// Next script argument
				++argPos;
			}
		}

		// Next argument
		++argPos;
	}

	if (scriptToExecute.empty())
	{
		std::cout << "No script specified." << std::endl << std::endl;
		printHelp();
		return 255;
	}

	auto& executor{ luaRunner::execute::Executor::getInstance() };

	// Set plugin search paths
	executor.setPluginSearchPaths(pluginsSearchPaths);

	// Load plugin(s) if any
	for (auto const& pluginName : pluginsToLoad)
	{
		std::cout << "Loading plugin '" << pluginName << "'" << std::endl;
		auto const loadResult = executor.loadPlugin(pluginName);
		auto const result = std::get<0>(loadResult);
		auto const errorString = std::get<1>(loadResult);
		if (!result)
		{
			std::cout << "Failed to load plugin: " << luaRunner::execute::Executor::resultToString(result) << ": " << errorString << std::endl;
			return 254;
		}
	}

	// Execute lua file
	std::cout << "Executing lua script '" << scriptToExecute << "'" << std::endl;

	auto const executeResult = executor.executeLuaFileWithParameters(scriptToExecute, scriptsParameters);
	auto const result = std::get<0>(executeResult);
	auto const scriptReturnValue = std::get<1>(executeResult);
	auto const errorString = std::get<2>(executeResult);

	if (!result)
	{
		std::cout << "Failed to execute script: " << luaRunner::execute::Executor::resultToString(result) << ": " << errorString << std::endl;
	}

	return scriptReturnValue;
}
