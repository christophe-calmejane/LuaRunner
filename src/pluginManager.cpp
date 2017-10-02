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

#include "pluginManager.hpp"
#include "luaRunnerPlugin.hpp"
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

class ManagerImpl final : public Manager
{
public:
	// Constructor
	ManagerImpl(lua_State* luaState) noexcept;

	// Manager overrides
	virtual LoadResult loadPlugin(std::string const& pluginPath) noexcept override;
	virtual void unloadAllPlugins() noexcept override;

	/** Destroy method for COM-like interface */
	virtual void destroy() noexcept override;

private:
	// Destructor
	~ManagerImpl() noexcept;

	// Private methods

	// Private members
	using LoadedPlugins = std::vector<DL_HANDLE>;

	lua_State* _state{ nullptr };
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
Manager::LoadResult ManagerImpl::loadPlugin(std::string const& pluginPath) noexcept
{
	auto const handle = DL_OPEN(pluginPath.c_str());
	if (handle == nullptr)
	{
		return { false, "TODO: LoadError Error Message" };
	}

	InitPluginFunc initFunc = reinterpret_cast<InitPluginFunc>(DL_SYM(handle, "?InitPlugin@@YG_NPAUlua_State@@@Z"));
	if (initFunc != nullptr)
	{
		initFunc(_state);
	}
	return { true, "" };
}

void ManagerImpl::unloadAllPlugins() noexcept
{
	for (auto const handle : _loadedPlugins)
	{
		UninitPluginFunc uninitFunc = reinterpret_cast<UninitPluginFunc>(DL_SYM(handle, "?UninitPlugin@@YGXPAUlua_State@@@Z"));
		if (uninitFunc != nullptr)
		{
			uninitFunc(_state);
		}
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
