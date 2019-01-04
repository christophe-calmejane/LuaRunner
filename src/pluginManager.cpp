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

#include "luaRunner/plugin.hpp"
#include "pluginManager.hpp"
#include "config.h"
#include <cassert>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#define DL_HANDLE HMODULE
#define DL_OPEN(library) LoadLibraryA(library)
#define DL_CLOSE(handle) FreeLibrary(handle)
#define DL_SYM(handle,symbol) GetProcAddress(handle,symbol)
#else // !_WIN32
#include <dlfcn.h>
#include <signal.h>
#define DL_HANDLE void*
#define DL_OPEN(library) dlopen(library,RTLD_LAZY)
#define DL_CLOSE(handle) dlclose(handle)
#define DL_SYM(handle,symbol) dlsym(handle,symbol)
#endif // _WIN32

namespace luaRunner
{
namespace plugin
{

constexpr auto InitPluginEntryPointName = "InitPlugin";
constexpr auto UninitPluginEntryPointName = "UninitPlugin";

class ManagerImpl final : public Manager
{
public:
	// Constructor
	ManagerImpl(lua_State* luaState) noexcept;

	// Manager overrides
	virtual void clearPluginSearchPaths() noexcept override;
	virtual void addPluginSearchPaths(std::string const& path) noexcept override;
	virtual LoadResult loadPlugin(std::string const& pluginName) noexcept override;
	virtual void unloadAllPlugins() noexcept override;

	/** Destroy method for COM-like interface */
	virtual void destroy() noexcept override;

private:
	// Destructor
	~ManagerImpl() noexcept;

	// Private methods

	// Private members
	using PluginSearchPaths = std::vector<std::string>;
	using LoadedPlugins = std::vector<DL_HANDLE>;

	lua_State* _state{ nullptr };
	PluginSearchPaths _searchPaths{};
	LoadedPlugins _loadedPlugins{};
};

// Constructor
ManagerImpl::ManagerImpl(lua_State* luaState) noexcept
	: _state(luaState)
{
}

// Destructor
ManagerImpl::~ManagerImpl() noexcept
{
	unloadAllPlugins();
}

// Manager overrides
void ManagerImpl::clearPluginSearchPaths() noexcept
{
	_searchPaths.clear();
}

void ManagerImpl::addPluginSearchPaths(std::string const& path) noexcept
{
	auto searchPath = path;

	if (searchPath.back() != '/' && searchPath.back() != '\\')
		searchPath.push_back('/');

	_searchPaths.push_back(std::move(searchPath));
}

Manager::LoadResult ManagerImpl::loadPlugin(std::string const& pluginName) noexcept
{
	// Validate plugin name

	//  1- Should not contain any '/' or '\\' (use plugin search path instead)
	if (pluginName.find_first_of("/\\") != pluginName.npos)
		return { false, "Plugin's name should not contain any '/' or '\\'. Use plugin search path instead." };

	//  2- Should not contain any '.' (do not specify plugin extension)
	if (pluginName.find('.') != pluginName.npos)
		return { false, "Plugin's name should not contain any '.'. Do not specify plugin file extension, only its name." };

	//  3- Should not start with 'lib' (do not specify plugin prefix)
	if (pluginName.substr(0, 3) == "lib")
		return { false, "Plugin's name should not start with 'lib'. Do not specify plugin file prefix, only its name." };

	// Try to load plugin using all search paths
	auto const name = LUARUNNER_PLUGIN_PREFIX + pluginName + LUARUNNER_PLUGIN_SUFFIX;
	DL_HANDLE handle{ nullptr };
	for (auto const& path : _searchPaths)
	{
		handle = DL_OPEN((path + name).c_str());
		if (handle != nullptr)
		{
			// Check entry points
			InitPluginFunc initFunc = reinterpret_cast<InitPluginFunc>(DL_SYM(handle, InitPluginEntryPointName));
			if (initFunc == nullptr)
			{
				return { false, "InitPlugin entry point not found." };
			}
			UninitPluginFunc uninitFunc = reinterpret_cast<UninitPluginFunc>(DL_SYM(handle, UninitPluginEntryPointName));
			if (uninitFunc == nullptr)
			{
				return { false, "UninitPlugin entry point not found." };
			}

			// Call InitPlugin entry point
			if (!initFunc(_state))
			{
				return { false, "InitPlugin entry point returned an error." };
			}
			return { true, "" };
		}
	}

	return { false, "Plugin '" + pluginName + "' not found in specified search paths (" + name + ")." };
}

void ManagerImpl::unloadAllPlugins() noexcept
{
	for (auto const handle : _loadedPlugins)
	{
		UninitPluginFunc uninitFunc = reinterpret_cast<UninitPluginFunc>(DL_SYM(handle, UninitPluginEntryPointName));
		uninitFunc(_state);
		DL_CLOSE(handle);
	}
	_loadedPlugins.clear();
}

// Private methods

/** Destroy method for COM-like interface */
void ManagerImpl::destroy() noexcept
{
	delete this;
}

/** Manager Entry point */
Manager* Manager::createRawManager(lua_State* luaState)
{
	return new ManagerImpl(luaState);
}

} // namespace plugin
} // namespace luaRunner
