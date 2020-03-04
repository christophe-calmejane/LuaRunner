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
#include <array>
#include <optional>

#ifndef LUARUNNER_UTILS_CALL_CONVENTION
#	error "LUARUNNER_UTILS_CALL_CONVENTION must be defined before including this file"
#endif // !LUARUNNER_UTILS_CALL_CONVENTION

namespace luaRunner
{
namespace utils
{
/* ************************************************************ */
/* Class setter/getter templates:                               */
/* Directly maps class simple setter and getter                 */
/* ************************************************************ */
#define DO_EXPAND(...) __VA_ARGS__##1
#define EXPAND(VAL) DO_EXPAND(VAL)

#if EXPAND(LUARUNNER_UTILS_CALL_CONVENTION) != 1 /* LUARUNNER_UTILS_CALL_CONVENTION is defined to something */
// Class setter
template<typename ObjectType, typename ValueType, void (LUARUNNER_UTILS_CALL_CONVENTION ObjectType::*Setter)(ValueType) noexcept>
constexpr int classSetter(lua_State* luaState)
{
	ObjectType* const obj = luaW_check<ObjectType>(luaState, 1);
	if (obj != nullptr)
	{
		(obj->*Setter)(getAndValidateType<ValueType>(luaState, 2));
	}
	return 0;
}

// Parent class setter
template<typename ObjectType, typename ValueType, typename ObjectParentType, void (LUARUNNER_UTILS_CALL_CONVENTION ObjectParentType::*Setter)(ValueType) noexcept>
constexpr int parentClassSetter(lua_State* luaState)
{
	auto* const obj = static_cast<ObjectParentType*>(luaW_check<ObjectType>(luaState, 1));
	if (obj != nullptr)
	{
		(obj->*Setter)(getAndValidateType<ValueType>(luaState, 2));
	}
	return 0;
}

// Class getter
template<typename ObjectType, typename ValueType, ValueType (LUARUNNER_UTILS_CALL_CONVENTION ObjectType::*Getter)() const>
constexpr int classGetter(lua_State* luaState)
{
	ObjectType const* const obj = luaW_check<ObjectType>(luaState, 1);
	if (obj != nullptr)
	{
		return pushValueToStack(luaState, (obj->*Getter)());
	}
	return 0;
}

// Parent class getter
template<typename ObjectType, typename ValueType, typename ObjectParentType, ValueType (LUARUNNER_UTILS_CALL_CONVENTION ObjectParentType::*Getter)() const>
constexpr int parentClassGetter(lua_State* luaState)
{
	auto const* const obj = static_cast<ObjectParentType*>(luaW_check<ObjectType>(luaState, 1));
	if (obj != nullptr)
	{
		return pushValueToStack(luaState, (obj->*Getter)());
	}
	return 0;
}
#endif // LUARUNNER_UTILS_CALL_CONVENTION is defined to something

// Class setter
template<typename ObjectType, typename ValueType, void (ObjectType::*Setter)(ValueType) noexcept>
constexpr int classSetter(lua_State* luaState)
{
	ObjectType* const obj = luaW_check<ObjectType>(luaState, 1);
	if (obj != nullptr)
	{
		(obj->*Setter)(getAndValidateType<ValueType>(luaState, 2));
	}
	return 0;
}

// Parent class setter
template<typename ObjectType, typename ValueType, typename ObjectParentType, void (ObjectParentType::*Setter)(ValueType) noexcept>
constexpr int parentClassSetter(lua_State* luaState)
{
	auto* const obj = static_cast<ObjectParentType*>(luaW_check<ObjectType>(luaState, 1));
	if (obj != nullptr)
	{
		(obj->*Setter)(getAndValidateType<ValueType>(luaState, 2));
	}
	return 0;
}

// Class getter
template<typename ObjectType, typename ValueType, ValueType (ObjectType::*Getter)() const>
constexpr int classGetter(lua_State* luaState)
{
	ObjectType const* const obj = luaW_check<ObjectType>(luaState, 1);
	if (obj != nullptr)
	{
		return pushValueToStack(luaState, (obj->*Getter)());
	}
	return 0;
}

// Parent class getter
template<typename ObjectType, typename ValueType, typename ObjectParentType, ValueType (ObjectParentType::*Getter)() const>
constexpr int parentClassGetter(lua_State* luaState)
{
	auto const* const obj = static_cast<ObjectParentType*>(luaW_check<ObjectType>(luaState, 1));
	if (obj != nullptr)
	{
		return pushValueToStack(luaState, (obj->*Getter)());
	}
	return 0;
}

} // namespace utils
} // namespace luaRunner
