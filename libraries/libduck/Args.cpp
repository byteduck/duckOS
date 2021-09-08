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

#include "Args.h"
#include <map>
#include <queue>
#include <cassert>

using namespace Duck;

Args::Args() {
	add_flag(_help_flag, "h", "help", "Shows this help message.");
}

void Args::add_flag(bool& storage, std::optional<std::string> short_flag, std::optional<std::string> long_flag, std::string help_text) {
	//If any other arguments already share the short or long version of this, delete the corresponding flag on the other
	for(auto& arg : _named_args) {
		if(arg.short_flag.has_value() && arg.short_flag == short_flag) {
			arg.short_flag = std::nullopt;
		}
		if(arg.long_flag.has_value() && arg.long_flag == long_flag) {
			arg.long_flag = std::nullopt;
		}
	}
	_named_args.push_back({std::move(short_flag), std::move(long_flag), std::move(help_text), &storage, BOOL});
}

bool Args::parse(int argc, char** argv, bool exit_on_fail, bool show_error_message) {
	//Compile map of named arguments
	std::map<std::string, NamedArg> named_map;
	for(auto& arg : _named_args) {
		if(arg.short_flag)
			named_map[arg.short_flag.value()] = arg;
		if(arg.long_flag)
			named_map[arg.long_flag.value()] = arg;
	}

	//Compile queue of positional_queue arguments
	std::queue<PositionalArg> positional_queue;
	for(auto& arg : _positional_args)
		positional_queue.push(arg);

	//Compile queue of input args
	std::queue<std::string> input_queue;
	for(int i = 1; i < argc; i++)
		input_queue.push(argv[i]);

	bool got_vararg = false;

	//Parse args
	while(!input_queue.empty()) {
		switch(try_named(input_queue, named_map, show_error_message)) {
			case CONTINUE:
				break;
			case NEXT_ARG:
				continue;
			case ERROR:
				if(show_error_message)
					fprintf(stderr, "Try using '%s --help' for more info.\n", argv[0]);
				if(exit_on_fail)
					exit(-1);
				return false;
		}

		switch(try_positional(input_queue, positional_queue, got_vararg, show_error_message)) {
			case CONTINUE:
				assert(false);
				break;
			case NEXT_ARG:
				continue;
			case ERROR:
				if(show_error_message)
					fprintf(stderr, "Try using '%s --help' for more info.\n", argv[0]);
				if(exit_on_fail)
					exit(-1);
				return false;
		}
	}

	//Check if we set the help flag
	if(_help_flag) {
		printf("Usage: %s [OPTIONS...]", argv[0]);
		for(auto& pos : _positional_args) {
			if(pos.required)
				printf(" %s", pos.name.c_str());
			else
				printf(" [%s]", pos.name.c_str());
			if(pos.vararg)
				printf("...");
		}

		if(!_positional_args.empty()) {
			printf("\n\nPositional arguments:\n");
			for(auto& pos: _positional_args) {
				printf("%s%s%s\t%s\n", pos.name.c_str(), pos.vararg ? "..." : "", pos.required ? " (required)" : "",
					   pos.help_text.c_str());
			}
		}

		if(!_named_args.empty()) {
			printf("\n\nNamed arguments:\n");
			for(auto& named: _named_args) {
				if(named.short_flag)
					printf("-%s", named.short_flag->c_str());
				if(named.long_flag && named.short_flag)
					printf(" or ");
				if(named.long_flag)
					printf("--%s", named.long_flag->c_str());
				printf("\t%s\n", named.help_text.c_str());
			}
		}

		exit(0);
	}

	//Check if any required args are left
	while(!positional_queue.empty()) {
		if(positional_queue.front().required && (!got_vararg || !positional_queue.front().vararg)) {
			if(show_error_message)
				fprintf(stderr, "Missing argument %s.\nTry using '%s --help' for more info.\n", positional_queue.front().name.c_str(), argv[0]);
			if(exit_on_fail)
				exit(-1);
			return false;
		}
		positional_queue.pop();
	}

	return true;
}

Args::IterationResult Args::try_named(std::queue<std::string>& input_queue, const std::map<std::string, NamedArg>& named_map, bool show_errors) {
	//Get the next argument string from the queue
	auto next_arg_raw = input_queue.front();

	//Check if it starts with '-'
	if(next_arg_raw[0] != '-')
		return CONTINUE;

	//Make sure the string contains more than just one/two dashes, otherwise treat it as positional
	auto name_idx = next_arg_raw.find_first_not_of('-');
	if(name_idx > 2)
		return CONTINUE;

	//If there's one dash, it is a short named argument
	bool is_short = name_idx == 1;

	//The argument name will be the character after the dash if short, or the rest of the argument if long
	std::string arg_name;
	if(is_short)
		arg_name = next_arg_raw[1];
	else
		arg_name = next_arg_raw.substr(2);

	//Find the argument in question
	auto arg_it = named_map.find(arg_name);
	if(arg_it == named_map.end()) {
		if(show_errors)
			fprintf(stderr, "Unknown argument '%s'.\n", arg_name.c_str());
		return ERROR;
	}
	auto& arg = arg_it->second;

	//If the argument is a bool, it won't take an operand, just set it to true and move on.
	if(arg.data_type == BOOL) {
		*((bool*) arg.storage) = true;
		if(is_short && next_arg_raw.length() > 2)
			input_queue.front() = "-" + next_arg_raw.substr(2);
		else
			input_queue.pop();
		return NEXT_ARG;
	}

	//Pop the argument off of the queue
	input_queue.pop();

	//If the flag is short, the rest of the argument is the operand. If long, the next argument is the operand.
	std::string operand;
	if(is_short) {
		if(next_arg_raw.length() < 3) {
			if(show_errors)
				fprintf(stderr, "Missing operand for argument '%s'.\n", arg_name.c_str());
			return ERROR;
		}
		operand = next_arg_raw.substr(2);
	} else {
		if(input_queue.empty()) {
			if(show_errors)
				fprintf(stderr, "Missing operand for argument '%s'.\n", arg_name.c_str());
			return ERROR;
		}
		operand = input_queue.front();
		input_queue.pop();
	}

	//Parse the argument value and put it in the correct storage.
	if(!parse_arg(arg.storage, operand, arg.data_type, false)) {
		if(show_errors)
			fprintf(stderr, "Invalid value '%s' for argument '%s'.\n", operand.c_str(), arg_name.c_str());
		return ERROR;
	}

	return NEXT_ARG;
}

Args::IterationResult Args::try_positional(std::queue<std::string>& input_queue, std::queue<PositionalArg>& positional_queue, bool& got_vararg, bool show_errors) {
	//If there's no more positional arguments, we provided too many arguments.
	if(_positional_args.empty()) {
		if(show_errors)
			fprintf(stderr, "Too many arguments.\n");
		return ERROR;
	}

	//Get the next argument and pop it off the queue.
	auto next_arg = input_queue.front();
	input_queue.pop();

	//Get the next positional argument, and pop it if it isn't variadic
	auto arg = positional_queue.front();
	if(!arg.vararg)
		positional_queue.pop();

	//Parse the argument value, and store it in storage.
	if(!parse_arg(arg.storage, next_arg, arg.data_type, arg.vararg)) {
		if(show_errors)
			fprintf(stderr, "Invalid value '%s' for argument '%s'.\n", next_arg.c_str(), arg.name.c_str());
		return ERROR;
	}

	if(arg.vararg)
		got_vararg = true;

	return NEXT_ARG;
}

bool Args::parse_arg(void* storage, const std::string& arg, Args::DataType data_type, bool vararg) {
	errno = 0;
	char* endptr;
	if(vararg) {
		switch(data_type) {
			case BOOL:
				return false; //Shouldn't be an argument, should be a flag
			case INT:
			case LONG:
				((std::vector<int>*) storage)->push_back(strtol(arg.c_str(), &endptr, 0));
				break;;
			case LLONG:
				((std::vector<long long>*) storage)->push_back(strtoll(arg.c_str(), &endptr, 0));
				break;;
			case UINT:
			case ULONG:
				((std::vector<unsigned int>*) storage)->push_back(strtoul(arg.c_str(), &endptr, 0));
				break;;
			case ULLONG:
				((std::vector<unsigned long long>*) storage)->push_back(strtoull(arg.c_str(), &endptr, 0));
				break;;
			case DOUBLE:
				((std::vector<double>*) storage)->push_back(strtod(arg.c_str(), &endptr));
				break;;
			case LDOUBLE:
				((std::vector<long double>*) storage)->push_back(strtold(arg.c_str(), &endptr));
				break;;
			case STRING:
				((std::vector<std::string>*) storage)->push_back(arg);
				break;;
		}
	} else {
		switch(data_type) {
			case BOOL:
				return false; //Shouldn't be an argument, should be a flag
			case INT:
			case LONG:
				*((int*) storage) = strtol(arg.c_str(), &endptr, 0);
				break;;
			case LLONG:
				*((long long*) storage) = strtoll(arg.c_str(), &endptr, 0);
				break;;
			case UINT:
			case ULONG:
				*((unsigned int*) storage) = strtoul(arg.c_str(), &endptr, 0);
				break;;
			case ULLONG:
				*((unsigned long long*) storage) = strtoull(arg.c_str(), &endptr, 0);
				break;;
			case DOUBLE:
				*((double*) storage) = strtod(arg.c_str(), &endptr);
				break;;
			case LDOUBLE:
				*((long double*) storage) = strtold(arg.c_str(), &endptr);
				break;;
			case STRING:
				*((std::string*) storage) = arg;
				break;;
		}
	}

	return endptr == (&arg[arg.length()]) && !errno;
}
