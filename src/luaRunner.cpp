/*
* Copyright 2017, Christophe Calmejane

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

/*
luaRunner -p lcomLua.dll lcomTest.lua
luaRunner -p avdeccLua.dll -p avdeccControllerLua.dll avdeccTest.lua
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
	std::cout << "  -p <plugin to load> -> Load specified plugin before executing the lua script. Multiple '-p' options can be specified to load multiple plugins." << std::endl;
}

int main(int argc, char const* argv[])
{
	std::vector<std::string> pluginsToLoad{};
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
				exit(1);
			}
			else if (arg == "-v")
			{
				std::cout << "LuaRunner version v" << luaRunner::getVersion() << std::endl;
				exit(1);
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
					exit(1);
				}
				pluginsToLoad.push_back(argv[currentPos + 1]);
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

	auto& executor{ luaRunner::execute::Executor::getInstance() };

	// Load plugin(s) if any
	for (auto const& pluginPath : pluginsToLoad)
	{
		std::cout << "Loading plugin '" << pluginPath << "'" << std::endl;
		auto const loadResult = executor.loadPlugin(pluginPath);
		auto const result = std::get<0>(loadResult);
		auto const errorString = std::get<1>(loadResult);
		if (!result)
		{
			std::cout << "Failed to load plugin: " << luaRunner::execute::Executor::resultToString(result) << ": " << errorString << std::endl;
		}
	}

	// Execute lua file
	std::cout << "Executing lua script '" << scriptToExecute << "'" << std::endl;

	auto const executeResult = executor.executeLuaFileWithParameters(scriptToExecute, scriptsParameters);
	auto const result = std::get<0>(executeResult);
	auto const errorString = std::get<1>(executeResult);

	if (!result)
	{
		std::cout << "Failed to execute script: " << luaRunner::execute::Executor::resultToString(result) << ": " << errorString << std::endl;
	}

	return 0;
}
