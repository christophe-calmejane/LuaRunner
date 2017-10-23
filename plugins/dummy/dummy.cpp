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

#include <luaRunnerPlugin.hpp>
#include <lua.hpp>
#include <chrono>
#include <thread>
#include <string>
#include <iostream>
#include <stdlib.h>

std::string valueToString(lua_State* luaState, int index)
{
	auto const type = lua_type(luaState, index);
	std::string value;
	switch(type)
	{
		case LUA_TNIL:
			value = "nil";
			break;
		case LUA_TBOOLEAN:
			value = lua_toboolean(luaState, index) ? "true" : "false";
			break;
		case LUA_TLIGHTUSERDATA:
			value = "LightUserData";
			break;
		case LUA_TNUMBER:
			if(lua_isinteger(luaState, index))
				value = std::to_string(lua_tointeger(luaState, index)) + " (int) ";
			else
				value = std::to_string(lua_tonumber(luaState, index)) + " (double)";
			break;
		case LUA_TSTRING:
			value = lua_tolstring(luaState, index, nullptr);
			break;
		case LUA_TTABLE:
			value = "Table";
			break;
		case LUA_TFUNCTION:
			value = "Function";
			break;
		case LUA_TUSERDATA:
			value = "UserData";
			break;
		case LUA_TTHREAD:
			value = "Thread";
			break;
		default:
			value = "Unknown";
			break;
	}

	return value;
}

/** Sample method not receiving any parameter and not returning any either. */
int dummy_helloWorld(lua_State* /*luaState*/)
{
	std::cout << "dummyLib.helloWorld" << std::endl;
	std::cout << "  Hello World from Dummy plugin" << std::endl;

	return 0; // Return 0 variable
}

/** Sample method receiving a key-value table as parameter. */
int dummy_printTable(lua_State* luaState)
{
	std::cout << "dummyLib.printTable" << std::endl;

	lua_pushnil(luaState); // First key to enumerate
	while(lua_next(luaState, 1)) // lua_next pushes 'key' @-2 and 'value' @-1
	{
		std::cout << "  Key: " << valueToString(luaState, -2) << " Value: " << valueToString(luaState, -1) << std::endl;
		lua_pop(luaState, 1); // Remove 'value' from the stack but keep 'key' for next iteration
	}

	return 0; // Return 0 variable
}

/** Sample method returning a key-value table. */
int dummy_getTable(lua_State* luaState)
{
	std::cout << "dummyLib.getTable" << std::endl;

	lua_newtable(luaState); // Push a new table on the stack

	lua_pushstring(luaState, "name"); // Push key 'name'
	lua_pushstring(luaState, "This is name string"); // Push value for key 'name'
	lua_rawset(luaState, -3); // Store the key-value pair in the table

	lua_pushstring(luaState, "num"); // Push key 'num'
	lua_pushinteger(luaState, -5); // Push value for key 'num'
	lua_rawset(luaState, -3); // Store the key-value pair in the table

	return 1; // Return 1 variable
}

/** Sample method receiving one mandatory and one optional parameter. */
int dummy_optParams(lua_State* luaState)
{
	std::cout << "dummyLib.optParams" << std::endl;

	auto const* const stringParam = luaL_checklstring(luaState, 1, NULL);
	auto const intParam = luaL_optinteger(luaState, 2, -1);

	std::cout << "  StringParam: " << stringParam << " intParam: " << intParam << std::endl;

	return 0; // Return 0 variable
}

/** Sample method receiving variable parameters. */
int dummy_varParams(lua_State* luaState)
{
	std::cout << "dummyLib.varParams" << std::endl;

	auto const argsCount = lua_gettop(luaState);
	std::cout << "  Got " << argsCount << " parameters:" << std::endl;

	for(auto argNum = 1; argNum <= argsCount; ++argNum)
	{
		auto const value = valueToString(luaState, argNum);
		auto const type = lua_type(luaState, argNum);
		auto const typeName = luaL_typename(luaState, argNum);
		std::cout << "  Param " << argNum << ", type " << type << " (" << typeName << "), value " << value << std::endl;
	}

	return 0; // Return 0 variable
}

constexpr luaL_Reg dummyLib[] = {
	// Dummy methods
	{"helloWorld", dummy_helloWorld},
	{"printTable", dummy_printTable},
	{"getTable", dummy_getTable},
	{"optParams", dummy_optParams},
	{"varParams", dummy_varParams},
	{NULL, NULL}
};

int luaopen_dummyLib(lua_State* luaState)
{
	luaL_newlib(luaState, dummyLib);
	return 1;
}

LUARUNNER_API bool LUARUNNER_CALL_CONVENTION InitPlugin(lua_State* luaState)
{
	luaL_requiref(luaState, "dummyLib", luaopen_dummyLib, 1);
	lua_pop(luaState, 1);  /* remove lib from the stack (luaL_requiref left it on the stack) */

	return true;
}

LUARUNNER_API void LUARUNNER_CALL_CONVENTION UninitPlugin(lua_State* luaState)
{
}
