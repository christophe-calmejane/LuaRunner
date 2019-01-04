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

#include <string>
#include <vector>
#include <memory>
#include <tuple>

namespace luaRunner
{
namespace execute
{
class Executor
{
public:
	enum class Result
	{
		Success = 0, /**< Success */
		LoadError = 1, /**< Error loading the plugin */
		InitError = 2, /**< Error initialiazing the plugin */
		ParseError = 3, /**< Error parsing lua script (file or buffer) */
		ExecError = 4, /**< Error executing lua script (file or buffer) */
		ReturnError = 5, /**< Successfully executed the script but invalid return value */
	};

	using LuaBuffer = std::string;
	using PluginSearchPaths = std::vector<std::string>;
	using ScriptParameters = std::vector<std::string>;
	using LoadResult = std::tuple<Result, std::string>;
	using ScriptReturnValue = std::uint8_t; // Clamped to [0-127]
	using ExecuteResult = std::tuple<Result, ScriptReturnValue, std::string>;

	static Executor& getInstance() noexcept;

	virtual void setPluginSearchPaths(PluginSearchPaths const& searchPaths) noexcept = 0;
	virtual LoadResult loadPlugin(std::string const& pluginName) noexcept = 0;

	/** Result, ErrorString (if Result != Success) */
	virtual ExecuteResult executeLuaFileWithParameters(std::string const& luaFilePath, ScriptParameters const& parameters) noexcept = 0;

	static std::string resultToString(Result const result) noexcept;

	// Deleted compiler auto-generated methods
	Executor(Executor&&) = delete;
	Executor(Executor const&) = delete;
	Executor& operator=(Executor const&) = delete;
	Executor& operator=(Executor&&) = delete;

protected:
	/** Constructor */
	Executor() noexcept = default;

	/** Destructor */
	virtual ~Executor() noexcept = default;
};

/* Operator overloads */
constexpr bool operator!(Executor::Result const result)
{
	return result != Executor::Result::Success;
}

} // namespace execute
} // namespace luaRunner
