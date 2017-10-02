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

#include "execute.hpp"
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
	virtual LoadResult loadPlugin(std::string const& pluginPath) noexcept override;
	virtual ExecuteResult executeLuaFileWithParameters(std::string const& luaFilePath, ScriptParameters const& parameters) noexcept override;

private:

	// Private methods
	void pushParamsToLua(ScriptParameters const& parameters) noexcept;
	ExecuteResult execute() noexcept;

	// Private members
	lua_State* _state{ nullptr };
	plugin::Manager::UniquePointer _pluginManager{ nullptr };
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
Executor::LoadResult ExecutorImpl::loadPlugin(std::string const& pluginPath) noexcept
{
	auto const loadResult = _pluginManager->loadPlugin(pluginPath);
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
		return { Result::ParseError, lua_tostring(_state, -1) };
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
		0, //number_of_returns,
		0 //errfunc_idx
	))
	{
		return { Result::ExecError, lua_tostring(_state, -1) };
	}
	return { Result::Success, "" };
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
