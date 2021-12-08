/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#pragma once

#include <string>
#include <optional>
#include <vector>
#include <queue>
#include <map>

namespace Duck {
	class Args {
	public:
		Args();

		void add_flag(bool& storage, std::optional<std::string> short_flag, std::optional<std::string> long_flag, std::string help_text);

		template<typename T>
		void add_named(T& storage, std::optional<std::string> short_flag, std::optional<std::string> long_flag, std::string help_text) {
			//If any other arguments already share the short or long version of this, delete the corresponding flag on the other
			for(auto& arg : _named_args) {
				if(arg.short_flag.has_value() && arg.short_flag == short_flag) {
					arg.short_flag = std::nullopt;
				}
				if(arg.long_flag.has_value() && arg.long_flag == long_flag) {
					arg.long_flag = std::nullopt;
				}
			}
			_named_args.push_back({std::move(short_flag), std::move(long_flag), std::move(help_text), &storage});
			auto& arg = _named_args.back();
			if constexpr(std::is_same<T, bool>())
				arg.data_type = BOOL;
			else if constexpr(std::is_same<T, int>())
				arg.data_type = INT;
			else if constexpr(std::is_same<T, long>())
				arg.data_type = LONG;
			else if constexpr(std::is_same<T, long long>())
				arg.data_type = LLONG;
			else if constexpr(std::is_same<T, unsigned int>())
				arg.data_type = UINT;
			else if constexpr(std::is_same<T, unsigned long>())
				arg.data_type = ULONG;
			else if constexpr(std::is_same<T, unsigned long long>())
				arg.data_type = ULLONG;
			else if constexpr(std::is_same<T, double>())
				arg.data_type = DOUBLE;
			else if constexpr(std::is_same<T, std::string>())
				arg.data_type = STRING;
			else
				static_invalid_arg();
		}

		template<typename T>
		void add_positional(T& storage, bool required, std::string name, std::string help_text) {
			_positional_args.push_back({required, std::move(name), std::move(help_text), &storage, is_vector<T>()});
			auto& arg = _positional_args.back();
			typedef typename t_or_vec_value_t<T>::type vectype;
			if constexpr(std::is_same<T, int>() || std::is_same<vectype, int>())
				arg.data_type = INT;
			else if constexpr(std::is_same<T, long>() || std::is_same<vectype, long>())
				arg.data_type = LONG;
			else if constexpr(std::is_same<T, long long>() || std::is_same<vectype, long long>())
				arg.data_type = LLONG;
			else if constexpr(std::is_same<T, unsigned int>() || std::is_same<vectype, unsigned int>())
				arg.data_type = UINT;
			else if constexpr(std::is_same<T, unsigned long>() || std::is_same<vectype, unsigned long>())
				arg.data_type = ULONG;
			else if constexpr(std::is_same<T, unsigned long long>() || std::is_same<vectype, unsigned long long>())
				arg.data_type = ULLONG;
			else if constexpr(std::is_same<T, double>() || std::is_same<vectype, double>())
				arg.data_type = DOUBLE;
			else if constexpr(std::is_same<T, std::string>() || std::is_same<vectype, std::string>())
				arg.data_type = STRING;
			else
				static_invalid_arg();
		}

		bool parse(int argc, char** argv, bool exit_on_fail = true, bool show_error_message = true);
	private:
		template<bool flag = false> void static_invalid_arg() { static_assert(flag, "Invalid argument datatype!"); };

		//Determines if the given T is a vector
		template<typename T> struct is_vector : public std::false_type {};
		template<typename T, typename A> struct is_vector<std::vector<T, A>> : public std::true_type {};

		//::type will be the vector value type if T is a vector, or T if not
		template<typename T> struct t_or_vec_value_t { typedef T type; };
		template<typename T, typename A> struct t_or_vec_value_t<std::vector<T, A>> { typedef typename A::value_type type; };

		enum DataType {
			BOOL, INT, LONG, LLONG, UINT, ULONG, ULLONG, DOUBLE, LDOUBLE, STRING
		};

		enum IterationResult {
			NEXT_ARG, CONTINUE, ERROR
		};

		class NamedArg {
		public:
			std::optional<std::string> short_flag;
			std::optional<std::string> long_flag;
			std::string help_text;
			void* storage;
			DataType data_type;
		};

		class PositionalArg {
		public:
			bool required;
			std::string name;
			std::string help_text;
			void* storage;
			bool vararg;
			DataType data_type;
		};

		IterationResult try_named(std::queue<std::string>& input_queue, const std::map<std::string, NamedArg>& args_map, bool show_errors);
		IterationResult try_positional(std::queue<std::string>& input_queue, std::queue<PositionalArg>& positional_queue, bool& got_vararg, bool show_errors);
		bool parse_arg(void* storage, const std::string& arg, DataType data_type, bool vararg);

		std::vector<NamedArg> _named_args;
		std::vector<PositionalArg> _positional_args;
		bool _help_flag = false;
	};
}


