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

#pragma once

#include <LuaWrapper/luawrapper.hpp>
#include <lua.hpp>

#include <type_traits>
#include <limits>
#include <string>
#include <functional>
#include <vector>
#include <set>
#include <map>
#include <optional>

#define LUA_ENUM(L, name, val) \
	lua_pushlstring(L, #name, sizeof(#name) - 1); \
	lua_pushinteger(L, static_cast<lua_Integer>(val)); \
	lua_rawset(L, -3);

/** Converts a relative stack index (negative) to an absolute stack index. No change if index is already absolute. */
inline int getStackAbsoluteIndex(lua_State* luaState, int const index)
{
	return index < 0 ? (lua_gettop(luaState) + index + 1) : index;
}

/** Returns true if the element is found, leaving it on the stack. */
inline bool hasTableElement(lua_State* luaState, int const tableIndex, char const* const elementName)
{
	lua_getfield(luaState, tableIndex, elementName);
	if (!lua_isnil(luaState, -1))
	{
		return true;
	}

	lua_pop(luaState, 1);

	return false;
}

/** Invokes a lua callback. */
template<typename... Args>
void invokeMethod(lua_State* luaState, int const tableIndex, char const* const name, Args&&... args)
{
	lua_rawgeti(luaState, LUA_REGISTRYINDEX, tableIndex); // Push the registry stored table to the stack
	lua_getfield(luaState, -1, name);
	if (lua_isfunction(luaState, -1))
	{
		if constexpr (sizeof...(args) != 0)
		{
			[[maybe_unused]] int processArgs[]{ pushValueToStack(luaState, args)... };
		}
		if (lua_pcall(luaState, sizeof...(args), 0, 0))
		{
			//lua_pop(luaState, 1); // Pop the error string from the stack
			lua_error(luaState); // Let's throw the error instead of silently ignoring it
		}
	}
	lua_pop(luaState, 1); // Pop the table from the stack
}

/* ************************************************************ */
/* getAndValidateType templates:                                */
/* Get value and validate type                                  */
/* ************************************************************ */

template<typename ValueType> // TODO: Specialization for uint64 and int64 (or the min/max comparison will not work properly)
constexpr std::enable_if_t<std::is_integral_v<ValueType>, ValueType> getAndValidateType(lua_State* luaState, int const index)
{
	static_assert(!std::is_same<bool, ValueType>::value, "Bool type should use specialization");
	auto const isInteger = lua_isinteger(luaState, index);
	if (!isInteger)
	{
		char const* const msg = lua_pushfstring(luaState, "integer expected, got %s", luaL_typename(luaState, index));
		luaL_argerror(luaState, index, msg);
	}
	auto const value = static_cast<ValueType>(lua_tointeger(luaState, index));
	auto constexpr minValue = std::numeric_limits<ValueType>::min();
	auto constexpr maxValue = std::numeric_limits<ValueType>::max();
	if (value < minValue)
	{
		char const* const msg = lua_pushfstring(luaState, std::string("passed integer value inferior to min value of " + std::to_string(minValue)).c_str());
		luaL_argerror(luaState, index, msg);
	}
	if (value > maxValue)
	{
		char const* const msg = lua_pushfstring(luaState, std::string("passed integer value superior to max value of " + std::to_string(maxValue)).c_str());
		luaL_argerror(luaState, index, msg);
	}
	return static_cast<ValueType>(value);
}

// bool specialization
template<>
inline bool getAndValidateType<bool>(lua_State* luaState, int const index)
{
	auto const isBoolean = lua_isboolean(luaState, index);
	if (!isBoolean)
	{
		char const* const msg = lua_pushfstring(luaState, "boolean expected, got %s", luaL_typename(luaState, index));
		luaL_argerror(luaState, index, msg);
	}
	return static_cast<bool>(lua_toboolean(luaState, index));
}

// String overload
template<typename ValueType>
constexpr std::enable_if_t<std::is_base_of_v<std::string, std::decay_t<ValueType>>, std::decay_t<ValueType>> getAndValidateType(lua_State* luaState, int const index)
{
	auto const isString = lua_isstring(luaState, index);
	if (!isString)
	{
		char const* const msg = lua_pushfstring(luaState, "string expected, got %s", luaL_typename(luaState, index));
		luaL_argerror(luaState, index, msg);
	}
	auto const* const value = lua_tostring(luaState, index);

	return value;
}

// Enum overload
template<typename ValueType>
constexpr std::enable_if_t<std::is_enum<ValueType>::value, ValueType> getAndValidateType(lua_State* luaState, int const index)
{
	return static_cast<ValueType>(getAndValidateType<std::underlying_type_t<ValueType>>(luaState, index));
}

// std::set<value> overload
template<typename ValueType>
constexpr std::enable_if_t<std::is_base_of_v<std::set<typename ValueType::value_type>, std::decay_t<ValueType>>, std::decay_t<ValueType>> getAndValidateType(lua_State* luaState, int const index)
{
	auto result = std::decay_t<ValueType>{};

	auto tableIndex = getStackAbsoluteIndex(luaState, index);
	auto const isTable = lua_istable(luaState, tableIndex);
	if (!isTable)
	{
		char const* const msg = lua_pushfstring(luaState, "table expected, got %s", luaL_typename(luaState, tableIndex));
		luaL_argerror(luaState, tableIndex, msg);
		return result;
	}

	lua_pushnil(luaState); // First key to enumerate
	while (lua_next(luaState, tableIndex)) // lua_next pushes 'key' @-2 and 'value' @-1
	{
		constexpr auto valueIndex = -1;
		result.insert(getAndValidateType<ValueType::value_type>(luaState, valueIndex));
		lua_pop(luaState, 1); // Remove 'value' from the stack but keep 'key' for next iteration
	}

	return result;
}

// std::map<integral, value> overload
template<typename ValueType, typename = std::enable_if_t<std::is_integral_v<typename ValueType::key_type>>>
constexpr std::enable_if_t<std::is_base_of_v<std::map<typename ValueType::key_type, typename ValueType::mapped_type>, std::decay_t<ValueType>>, std::decay_t<ValueType>> getAndValidateType(lua_State* luaState, int const index)
{
	auto result = std::decay_t<ValueType>{};

	auto tableIndex = getStackAbsoluteIndex(luaState, index);
	auto const isTable = lua_istable(luaState, tableIndex);
	if (!isTable)
	{
		char const* const msg = lua_pushfstring(luaState, "table expected, got %s", luaL_typename(luaState, tableIndex));
		luaL_argerror(luaState, tableIndex, msg);
		return result;
	}

	lua_pushnil(luaState); // First key to enumerate
	while (lua_next(luaState, tableIndex)) // lua_next pushes 'key' @-2 and 'value' @-1
	{
		constexpr auto keyIndex = -2;
		constexpr auto valueIndex = -1;
		if (!lua_isinteger(luaState, keyIndex))
		{
			char const* const msg = lua_pushfstring(luaState, "table integer index expected, got %s type index", luaL_typename(luaState, keyIndex));
			luaL_argerror(luaState, keyIndex, msg);
			return result;
		}
		auto const key = static_cast<ValueType::key_type>(lua_tointeger(luaState, keyIndex));
		result[key] = getAndValidateType<ValueType::mapped_type>(luaState, valueIndex);
		lua_pop(luaState, 1); // Remove 'value' from the stack but keep 'key' for next iteration
	}

	return result;
}


/* ************************************************************ */
/* getAndValidateTableElement template:                         */
/* Get value from a table and validate type                     */
/* ************************************************************ */

template<typename ValueType>
inline std::decay_t<ValueType> getAndValidateTableElement(lua_State* luaState, int const tableIndex, char const* const elementName)
{
	auto result = std::decay_t<ValueType>{};

	lua_getfield(luaState, tableIndex, elementName);
	if (lua_isnil(luaState, -1))
	{
		lua_pop(luaState, 1);

		char const* const msg = lua_pushfstring(luaState, "required table element '%s' not found", elementName);
		luaL_argerror(luaState, tableIndex, msg);
		return result;
	}

	result = getAndValidateType<std::decay_t<ValueType>>(luaState, -1);
	lua_pop(luaState, 1);

	return result;
}


/* ************************************************************ */
/* getAndValidateOptionalTableElement template:                 */
/* Get value from a table if it exists, and validate type       */
/* ************************************************************ */

template<typename ValueType>
inline std::optional<ValueType> getAndValidateOptionalTableElement(lua_State* luaState, int const tableIndex, char const* const elementName)
{
	auto result = std::optional<ValueType>{};

	lua_getfield(luaState, tableIndex, elementName);
	if (!lua_isnil(luaState, -1))
	{
		result = getAndValidateType<ValueType>(luaState, -1);
	}

	lua_pop(luaState, 1);

	return result;
}


/* ************************************************************ */
/* pushValueToStack templates:                                  */
/* Push a value to the lua stack                                */
/* ************************************************************ */

template<typename ValueType>
constexpr std::enable_if_t<std::is_integral_v<ValueType>, int> pushValueToStack(lua_State* luaState, ValueType const value)
{
	static_assert(!std::is_same<bool, ValueType>::value, "Bool type should use specialization");
	lua_pushinteger(luaState, value);
	return 1;
}

// std::set<value> overload
template<typename ValueType>
constexpr std::enable_if_t<std::is_base_of_v<std::set<typename ValueType::value_type>, ValueType>, int> pushValueToStack(lua_State* luaState, ValueType const& values)
{
	lua_newtable(luaState); // Push a new table on the stack

	auto idx = 1u;
	for (auto const& value : values)
	{
		lua_pushinteger(luaState, idx); // Push key index
		pushValueToStack(luaState, value); // Push value
		lua_rawset(luaState, -3); // Store the key-value pair in the table
		++idx;
	}

	return 1; // Return one pushed variable on the lua stack
}

// std::map<integral, value> overload
template<typename ValueType, typename = std::enable_if_t<std::is_integral_v<typename ValueType::key_type>>>
constexpr std::enable_if_t<std::is_base_of_v<std::map<typename ValueType::key_type, typename ValueType::mapped_type>, ValueType>, int> pushValueToStack(lua_State* luaState, ValueType const& values)
{
	lua_newtable(luaState); // Push a new table on the stack

	for (auto const& [key, value] : values)
	{
		lua_pushinteger(luaState, key); // Push key index
		pushValueToStack(luaState, value); // Push value
		lua_rawset(luaState, -3); // Store the key-value pair in the table
	}

	return 1; // Return one pushed variable on the lua stack
}

// bool specialization
template<>
inline int pushValueToStack<bool>(lua_State* luaState, bool const value)
{
	lua_pushboolean(luaState, value);
	return 1;
}

// String overload
template<typename ValueType>
constexpr std::enable_if_t<std::is_base_of_v<std::string, ValueType>, int> pushValueToStack(lua_State* luaState, ValueType const value)
{
	lua_pushstring(luaState, value.c_str());
	return 1;
}

// Enum overload
template<typename ValueType>
constexpr std::enable_if_t<std::is_enum<ValueType>::value, int> pushValueToStack(lua_State* luaState, ValueType const value)
{
	lua_pushinteger(luaState, static_cast<std::underlying_type_t<ValueType>>(value));
	return 1;
}

/* ************************************************************ */
/* pushTableElement template:                                   */
/* Push value to a table (must be on the stack already)         */
/* ************************************************************ */

template<typename ValueType>
inline void pushTableElement(lua_State* luaState, char const* const elementName, ValueType value)
{
	lua_pushstring(luaState, elementName); // Push key
	pushValueToStack<ValueType>(luaState, value); // Push value
	lua_rawset(luaState, -3); // Store the key-value pair in the table
}
