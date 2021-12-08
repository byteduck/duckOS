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

#include <utility>
#include <vector>
#include <string>
#include <functional>
#include <sys/socketfs.h>
#include "packet.h"
#include <cstring>
#include "BusConnection.h"

#pragma once

namespace River {

	class IFunction {
	public:
		virtual void remote_call(const RiverPacket& packet) = 0;
		virtual const std::string& path() = 0;
	};

	template<typename RetT, typename... ParamTs>
	class Function: public IFunction {
		static_assert(std::is_pod<RetT>() || std::is_void<RetT>(), "Function return type must be a plain-old datatype or void!");
		static_assert((std::is_pod<ParamTs>() && ...), "Function arguments must be plain-old datatypes!");

	public:
		Function(const std::string& path): _path(path), _endpoint(nullptr), _callback(nullptr) {}

		Function(const std::string& path, std::shared_ptr<Endpoint> endpoint, std::function<RetT(sockid_t, ParamTs...)> callback = nullptr):
				_path(stringname_of(path)),
				_endpoint(std::move(endpoint)),
				_callback(callback) {}

		static std::string stringname_of(const std::string& path) {
			std::string ret = path + "<" + typeid(RetT).name() + "[";
			if constexpr(std::is_void<RetT>())
				ret += "0]";
			else
				ret += std::to_string(sizeof(RetT)) + "]";
			((ret += std::string(",") + typeid(ParamTs).name() + "[" + std::to_string(sizeof(ParamTs)) + "]"), ...);
			return ret + ">";
		}

		RetT operator()(ParamTs... args) const {
			if(!_endpoint) {
				Duck::Log::err("[River] Tried calling uninitialized function ", _path);
				return RetT();
			}

			if(_endpoint->type() == Endpoint::PROXY) {
				RiverPacket packet = {
						FUNCTION_CALL,
						_endpoint->name(),
						_path
				};

				//Serialize function call data (tuple {arg1, arg2, arg3...})
				std::tuple<ParamTs...> data_tuple = {args...};
				packet.data.resize(sizeof(data_tuple));
				memcpy(packet.data.data(), &data_tuple, sizeof(data_tuple));

				//Send the function call packet and await a reply (if the function has a non-void return type)
				_endpoint->bus()->send_packet(packet);
				if constexpr(!std::is_void<RetT>()) {
					auto pkt = _endpoint->bus()->await_packet(FUNCTION_RETURN, _endpoint->name(), _path);
					if(pkt.error) {
						Duck::Log::err("[River] Remote function call ", _endpoint, ":", _path, " failed: ", error_str(pkt.error));
						if constexpr(!std::is_void<RetT>())
							return RetT();
					}

					//Deserialize and return the return value
					RetT ret;
					if(pkt.data.size() == sizeof(RetT))
						memcpy(&ret, pkt.data.data(), sizeof(RetT));
					return ret;
				}
			} else {
				return _callback(0, args...);
			}
		}

		const std::string& path() override {
			return _path;
		}

		void remote_call(const RiverPacket& packet) override {
			//Make sure the data is the correct size
			if(packet.data.size() != sizeof(std::tuple<ParamTs...>)) {
				_endpoint->bus()->send_packet({
					FUNCTION_RETURN,
					packet.endpoint,
					packet.path,
					MALFORMED_DATA,
					packet.sender
				});
			}

			//Deserialize the parameters
			std::tuple<ParamTs...> data_tuple;
			memcpy(&data_tuple, packet.data.data(), sizeof(data_tuple));

			//Call the function
			if constexpr(!std::is_void<RetT>()) {
				//Serialize the return value and send the response
				RiverPacket resp {
						FUNCTION_RETURN,
						packet.endpoint,
						packet.path
				};
				resp.recipient = packet.sender;
				RetT ret = _callback(packet.sender, std::get<ParamTs>(data_tuple)...);
				resp.data.resize(sizeof(RetT));
				memcpy(resp.data.data(), &ret, sizeof(RetT));
				_endpoint->bus()->send_packet(resp);
			} else {
				_callback(packet.sender, std::get<ParamTs>(data_tuple)...);
			}
		}

	private:
		std::string _path;
		std::shared_ptr<Endpoint> _endpoint;
		std::function<RetT(sockid_t, ParamTs...)> _callback;
	};
}

