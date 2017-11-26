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

#include "luaRunner/execute.hpp"
#include "pluginManager.hpp"
#include "builtin.hpp"
#include <lua.hpp>
#include <cassert>

namespace luaRunner
{
namespace execute
{

class ExecutorImpl final : public Executor
{
public:
	// Constructor
	ExecutorImpl() noexcept;
	// Destructor
	~ExecutorImpl() noexcept;

	// Executor overrides
	virtual void setPluginSearchPaths(PluginSearchPaths const& searchPaths) noexcept override;
	virtual LoadResult loadPlugin(std::string const& pluginName) noexcept override;
	virtual ExecuteResult executeLuaFileWithParameters(std::string const& luaFilePath, ScriptParameters const& parameters) noexcept override;

private:

	// Private methods
	void pushParamsToLua(ScriptParameters const& parameters) noexcept;
	ExecuteResult execute() noexcept;

	// Private members
	lua_State* _state{ nullptr };
	plugin::Manager::UniquePointer _pluginManager{ nullptr, nullptr };
};

// Constructor
ExecutorImpl::ExecutorImpl() noexcept
	: _state(luaL_newstate())
	, _pluginManager(plugin::Manager::create(_state))
{
	// Load lua libs
	luaL_openlibs(_state);
	// Load luaRunner builtins
	builtin::loadBuiltins(_state);
}

// Destructor
ExecutorImpl::~ExecutorImpl() noexcept
{
	_pluginManager->unloadAllPlugins();
	if (_state != nullptr)
		lua_close(_state);
}

// Executor overrides
void ExecutorImpl::setPluginSearchPaths(PluginSearchPaths const& searchPaths) noexcept
{
	// Clear previous search paths
	_pluginManager->clearPluginSearchPaths();

	// Always add current directory path
	_pluginManager->addPluginSearchPaths(".");

	// Add other search paths
	for (auto const& path : searchPaths)
		_pluginManager->addPluginSearchPaths(path);
}

Executor::LoadResult ExecutorImpl::loadPlugin(std::string const& pluginName) noexcept
{
	auto const loadResult = _pluginManager->loadPlugin(pluginName);
	auto const result = std::get<0>(loadResult);
	auto const errorString = std::get<1>(loadResult);
	if (!result)
	{
		return { Result::LoadError, errorString };
	}
	return { Result::Success, "" };
}

Executor::ExecuteResult ExecutorImpl::executeLuaFileWithParameters(std::string const& luaFilePath, ScriptParameters const& parameters) noexcept
{
	pushParamsToLua(parameters);

	if (luaL_loadfile(_state, luaFilePath.c_str()))
	{
		return { Result::ParseError, ScriptReturnValue(253u), lua_tostring(_state, -1) };
	}
	return execute();
}

// Private methods
void ExecutorImpl::pushParamsToLua(ScriptParameters const& parameters) noexcept
{
	lua_newtable(_state);

	auto tableIndex{ 1u }; // Lua table index start at 1 (not 0)

	for (auto const& param : parameters)
	{
		// Push eash argN
		lua_pushinteger(_state, tableIndex);
		lua_pushstring(_state, param.c_str());
		lua_rawset(_state, -3);
		++tableIndex;
	}
	lua_setglobal(_state, "argv");

	lua_pushinteger(_state, parameters.size());
	lua_setglobal(_state, "argc");
}

Executor::ExecuteResult ExecutorImpl::execute() noexcept
{
	if (lua_pcall(
		_state,
		0, //number_of_args,
		1, //number_of_returns,
		0 //errfunc_idx
	))
	{
		return { Result::ExecError, ScriptReturnValue(253u), lua_tostring(_state, -1) };
	}

	// Check if there is a returned value by the script
	auto const type = lua_type(_state, -1);
	if (type != LUA_TNIL && (type != LUA_TNUMBER || lua_isinteger(_state, -1) == 0))
	{
		return { Result::ReturnError, ScriptReturnValue(252u), "Should be an integer (or no value)" };
	}
	
	auto const retValue = lua_tointeger(_state, -1); // If no value was returned, lua_tointeger will return 0
	if (retValue < 0 || retValue >= 128)
	{
		return { Result::ReturnError, ScriptReturnValue(252u), "Should be comprised between 0 and 127 (inclusive)" };
	}

	return { Result::Success, static_cast<ScriptReturnValue>(retValue), "" };
}

// Executor methods
std::string Executor::resultToString(Result const result) noexcept
{
	switch (result)
	{
		case Result::Success:
			return "Success";
		case Result::LoadError:
			return "Load Plugin Error";
		case Result::InitError:
			return "Init Plugin Error";
		case Result::ParseError:
			return "Parse Error";
		case Result::ExecError:
			return "Exec Error";
		case Result::ReturnError:
			return "Invalid return value";
		default:
			assert(false && "luaRunner::execute::Executor::Result value not handled");
			return "Unknown Result";
	}
}

Executor& Executor::getInstance() noexcept
{
	static ExecutorImpl s_Executor;

	return s_Executor;
}

} // namespace execute
} // namespace luaRunner
