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

#ifndef DUCKOS_LIBRIVER_ENDPOINT_H
#define DUCKOS_LIBRIVER_ENDPOINT_H

#include <string>
#include <functional>
#include <optional>
#include "BusConnection.h"
#include <optional>

namespace River {
	class IFunction;
	template<typename RetT, typename... ArgTs>
	class Function;
	class BusConnection;

	class Endpoint {
	public:
		enum ConnectionType { PROXY, HOST };

		Endpoint(BusConnection* bus, const std::string& name, ConnectionType type);

		template<typename RetT, typename... ParamTs>
		ResultRet<Function<RetT, ParamTs...>> register_function(const std::string& path, RetT callback(sockid_t, ParamTs...)) {
			auto stringname = Function<RetT, ParamTs...>::stringname_of(path);

			if(_functions[stringname])
				return *std::dynamic_pointer_cast<Function<RetT, ParamTs...>>(_functions[path]);

			_bus->send_packet({
				REGISTER_FUNCTION,
				_name,
				stringname
			});

			auto packet = _bus->await_packet(REGISTER_FUNCTION, _name, stringname);
			if(packet.error) {
				fprintf(stderr, "[River] Couldn't register function %s:%s: %s\n", _name.c_str(), path.c_str(), error_str(packet.error));
				return Result(packet.error);
			}

			auto ret = std::make_shared<Function<RetT, ParamTs...>>(path, this, callback);
			_functions[stringname] = ret;
			return *ret;
		}

		template<typename RetT, typename... ParamTs>
		ResultRet<Function<RetT, ParamTs...>> get_function(const std::string& path) {
			auto stringname = Function<RetT, ParamTs...>::stringname_of(path);

			if(_functions[stringname])
				return *std::dynamic_pointer_cast<Function<RetT, ParamTs...>>(_functions[path]);

			_bus->send_packet({
				GET_FUNCTION,
				_name,
				stringname
			});

			auto packet = _bus->await_packet(River::GET_FUNCTION, _name, stringname);
			if(packet.error) {
				fprintf(stderr, "[River] Error getting function %s:%s: %s\n", _name.c_str(), path.c_str(), error_str(packet.error));
				return Result(packet.error);
			}

			auto ret = std::make_shared<Function<RetT, ParamTs...>>(path, this);
			_functions[stringname] = ret;
			return *ret;
		}

		std::shared_ptr<IFunction> get_ifunction(const std::string& path);
		const std::string& name();
		ConnectionType type() const;
		BusConnection* bus();

	private:
		std::map<std::string, std::shared_ptr<IFunction>> _functions;
		std::string _name;
		ConnectionType _type;
		BusConnection* _bus;
	};
}

#endif //DUCKOS_LIBRIVER_ENDPOINT_H
