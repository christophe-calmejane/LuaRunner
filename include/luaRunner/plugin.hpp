/*
* Copyright 2017-2019, Christophe Calmejane

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

#include <lua.hpp>

#ifdef _WIN32

# define LUARUNNER_CALL_CONVENTION __cdecl
# define LUARUNNER_API extern "C" __declspec(dllexport)

#else // !_WIN32

#define LUARUNNER_CALL_CONVENTION
#define LUARUNNER_API extern "C" __attribute__((visibility("default")))

#endif // _WIN32

/** True if success. Plugin must export a function named 'InitPlugin' with that prototype. */
typedef bool (LUARUNNER_CALL_CONVENTION *InitPluginFunc)(lua_State* luaState);

/** True if success. Plugin must export a function named 'UninitPlugin' with that prototype. */
typedef void (LUARUNNER_CALL_CONVENTION *UninitPluginFunc)(lua_State* luaState);
