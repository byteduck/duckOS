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

#ifndef DUCKOS_LIBDUCK_LOG_H
#define DUCKOS_LIBDUCK_LOG_H

#include <fstream>
#include <iostream>

class Log {
public:
	template<typename... ParamTs>
	static inline void dbg(const ParamTs&... params) {
		(kstream << "\06" << ... << params) << std::endl;
		if(std::cerr.rdbuf())
			(std::cerr << "\033[90m[DEBUG] " << ... << params) << "\033[39;49m" << std::endl;
	}

	template<typename... ParamTs>
	static inline void info(const ParamTs&... params) {
		(kstream << "\05" << ... << params) << std::endl;
		if(std::cerr.rdbuf())
			(std::cerr << "\033[94m[INFO] " << ... << params) << "\033[39;49m" << std::endl;
	}
	
	template<typename... ParamTs>
	static inline void success(const ParamTs&... params) {
		(kstream << "\04" << ... << params) << std::endl;
		if(std::cerr.rdbuf())
			(std::cerr << "\033[92m[SUCCESS] " << ... << params) << "\033[39;49m" << std::endl;
	}
	
	template<typename... ParamTs>
	static inline void warn(const ParamTs&... params) {
		(kstream << "\03" << ... << params) << std::endl;
		if(std::cerr.rdbuf())
			(std::cerr << "\033[93m[WARN] " << ... << params) << "\033[39;49m" << std::endl;
	}
	
	template<typename... ParamTs>
	static inline void err(const ParamTs&... params) {
		(kstream << "\02" << ... << params) << std::endl;
		if(std::cerr.rdbuf())
			(std::cerr << "\033[91m[ERROR] " << ... << params) << "\033[39;49m" << std::endl;
	}

	template<typename... ParamTs>
	static inline void crit(const ParamTs&... params) {
		(kstream << "\01" << ... << params) << std::endl;
		if(std::cerr.rdbuf())
			(std::cerr << "\033[97;41m[CRITICAL] " << ... << params) << "\033[39;49m" << std::endl;
	}
	
private:
	static std::ofstream kstream;
};

#endif //DUCKOS_LIBDUCK_LOG_H
