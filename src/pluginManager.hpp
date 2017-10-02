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

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <lua.hpp>

namespace luaRunner
{
namespace plugin
{

class Manager
{
public:
	using UniquePointer = std::unique_ptr<Manager, void(*)(Manager*)>;
	using LoadResult = std::tuple<bool, std::string>;

	/**
	* @brief Factory to create a new Manager.
	* @details Creates a new Manager as a unique pointer.
	* @param[in] luaState A valid lua_State.
	* @return A new Manager as a Manager::UniquePointer.
	*/
	static UniquePointer create(lua_State* luaState)
	{
		auto deleter = [](Manager* self)
		{
			self->destroy();
		};
		return UniquePointer(createRawManager(luaState), deleter);
	}

	/** Result, ErrorString (if Result != Success) */
	virtual LoadResult loadPlugin(std::string const& pluginPath) noexcept = 0;

	virtual void unloadAllPlugins() noexcept = 0;

	// Deleted compiler auto-generated methods
	Manager(Manager&&) = delete;
	Manager(Manager const&) = delete;
	Manager& operator=(Manager const&) = delete;
	Manager& operator=(Manager&&) = delete;

protected:
	/** Constructor */
	Manager() noexcept = default;

	/** Destructor */
	virtual ~Manager() noexcept = default;

private:
	/** Entry point */
	static Manager* createRawManager(lua_State* luaState);

	/** Destroy method for COM-like interface */
	virtual void destroy() noexcept = 0;
};

} // namespace plugin
} // namespace luaRunner
