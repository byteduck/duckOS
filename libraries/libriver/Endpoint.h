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

namespace River {
	class IFunction;
	template<typename RetT, typename... ArgTs>
	class Function;
	class BusConnection;
	class IMessage;
	template<typename T>
	class Message;

	template<typename T>
	struct type_identity {
		using type = T;
	};

	class Endpoint: public std::enable_shared_from_this<Endpoint> {
	public:
		enum ConnectionType { PROXY, HOST };

		Endpoint(std::shared_ptr<BusConnection> bus, const std::string& name, ConnectionType type);

		template<typename RetT, typename... ParamTs>
		ResultRet<Function<RetT, ParamTs...>> register_function(const std::string& path, typename type_identity<std::function<RetT(sockid_t, ParamTs...)>>::type callback) {
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

			auto ret = std::make_shared<Function<RetT, ParamTs...>>(path, shared_from_this(), callback);
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

			auto ret = std::make_shared<Function<RetT, ParamTs...>>(path, shared_from_this());
			_functions[stringname] = ret;
			return *ret;
		}

		template<typename T>
		ResultRet<Message<T>> register_message(const std::string& path) {
			auto stringname = Message<T>::stringname_of(path);

			if(_messages[stringname])
				return *std::dynamic_pointer_cast<Message<T>>(_messages[path]);

			_bus->send_packet({
				REGISTER_MESSAGE,
				_name,
				stringname
			});

			auto packet = _bus->await_packet(River::REGISTER_MESSAGE, _name, stringname);
			if(packet.error) {
				fprintf(stderr, "[River] Error registering message %s:%s: %s\n", _name.c_str(), path.c_str(), error_str(packet.error));
				return Result(packet.error);
			}

			auto ret = std::make_shared<Message<T>>(path, shared_from_this());
			_messages[stringname] = ret;
			return *ret;
		}

		template<typename T>
		Result send_message(const std::string& path, sockid_t recipient, const T& data) {
			auto stringname = Message<T>::stringname_of(path);
			auto msg = _messages[stringname];
			if(!msg) {
				fprintf(stderr, "[River] Tried sending nonexistent message %s:%s!\n", _name.c_str(), path.c_str());
				return MESSAGE_DOES_NOT_EXIST;
			}
			std::dynamic_pointer_cast<Message<T>>(msg)->send(recipient, data);
			return SUCCESS;
		}

		template<typename T>
		Result set_message_handler(const std::string& path, typename type_identity<std::function<void(T)>>::type callback) {
			auto stringname = Message<T>::stringname_of(path);

			if(_messages[stringname])
				return Result(MESSAGE_HANDLER_ALREADY_SET);

			_bus->send_packet({
				GET_MESSAGE,
				_name,
				stringname
			});

			auto packet = _bus->await_packet(River::GET_MESSAGE, _name, stringname);
			if(packet.error) {
				fprintf(stderr, "[River] Error getting message %s:%s: %s\n", _name.c_str(), path.c_str(), error_str(packet.error));
				return Result(packet.error);
			}

			auto ret = std::make_shared<Message<T>>(path, shared_from_this(), callback);
			_messages[stringname] = ret;
			return Result(SUCCESS);
		}

		std::shared_ptr<IFunction> get_ifunction(const std::string& path);
		std::shared_ptr<IMessage> get_imessage(const std::string& path);

		const std::string& name();
		ConnectionType type() const;
		const std::shared_ptr<BusConnection>& bus();

		std::function<void(sockid_t, pid_t)> on_client_connect = nullptr;
		std::function<void(sockid_t, pid_t)> on_client_disconnect = nullptr;
	private:
		std::map<std::string, std::shared_ptr<IFunction>> _functions;
		std::map<std::string, std::shared_ptr<IMessage>> _messages;
		std::string _name;
		ConnectionType _type;
		std::shared_ptr<BusConnection> _bus;
	};
}

#endif //DUCKOS_LIBRIVER_ENDPOINT_H
