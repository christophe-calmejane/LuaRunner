/*
* Copyright 2017, Christophe Calmejane

* This file is part of LuaRunner.

* LuaRunner is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* LuaRunner is distributed in the hope that it will be usefu_state,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.

* You should have received a copy of the GNU Lesser General Public License
* along with LuaRunner.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <lua.hpp>
#include <cassert>
#include <chrono>
#include <thread>
#include "execute.hpp"

namespace luaRunner
{
namespace builtin
{

/*
* Pauses current thread for the specified milliseconds
* [in] msec The number of milliseconds to pause the current thread for.
*/
int utils_sleep(lua_State* luaState)
{
	auto const msec = luaL_checkinteger(luaState, 1);

	std::this_thread::sleep_for(std::chrono::milliseconds(msec));

	return 0; // Return 0 variable
}

/*
* Loads the specified luaRunner plugin into the lua VM. Plugin must be found and valid or an error will be thrown.
* [in] pluginPath The path of the luaRunner plugin to load.
*/
int utils_require(lua_State* luaState)
{
	auto const* const pluginPath = luaL_checklstring(luaState, 1, NULL);

	auto& executor {execute::Executor::getInstance()};
	auto const loadResult = executor.loadPlugin(pluginPath);
	auto const result = std::get<0>(loadResult);
	auto const errorString = std::get<1>(loadResult);
	if(!result)
	{
		luaL_error(luaState, (std::string("Failed to load plugin: ") + luaRunner::execute::Executor::resultToString(result) + ": " + errorString).c_str());
	}
	
	return 0; // Return 0 variable
}

constexpr luaL_Reg builtins[] = {
	// Utils methods
	{"sleep", utils_sleep},
	{"require", utils_require},
	{NULL, NULL}
};

int luaopen_builtins(lua_State* luaState)
{
	luaL_newlib(luaState, builtins);
	return 1;
}

void loadBuiltins(lua_State* luaState) noexcept
{
	luaL_requiref(luaState, "lrbi", luaopen_builtins, 1);
	lua_pop(luaState, 1);  /* remove lib from the stack (luaL_requiref left it on the stack) */
}

} // namespace builtin
} // namespace luaRunner
