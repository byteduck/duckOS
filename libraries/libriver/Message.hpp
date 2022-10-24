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

#include "packet.h"
#include "Endpoint.h"
#include "libduck/serialization_utils.h"
#include <type_traits>

namespace River {
	class IMessage {
	public:
		virtual void handle_message(const RiverPacket& packet) const = 0;
		virtual const std::string& path() const = 0;
	};

	template<typename T>
	class Message: public IMessage {
		static_assert(Duck::Serialization::is_serializable_type<T>(), "Message type must be serializable!");

	public:
		Message(const std::string& path): _path(path), _endpoint(nullptr), _callback(nullptr) {}

		Message(const std::string& path, std::shared_ptr<Endpoint> endpoint):
				_path(stringname_of(path)),
				_endpoint(std::move(endpoint)) {}

		Message(const std::string& path, std::shared_ptr<Endpoint> endpoint, std::function<void(T)> callback):
				_path(stringname_of(path)),
				_endpoint(std::move(endpoint)),
				_callback(callback) {}

		static std::string stringname_of(const std::string& path) {
			return path + "<" + typeid(T).name() + "[" + std::to_string(sizeof(T)) + "]>";
		}

		void send(sockid_t recipient, const T& data) const {
			if(!_endpoint) {
				Duck::Log::err("[River] Tried sending uninitialized message ", _path);
				return;
			}

			if(_endpoint->type() == Endpoint::HOST) {
				RiverPacket packet = {
						SEND_MESSAGE,
						_endpoint->name(),
						_path
				};
				packet.recipient = recipient;

				//Serialize message data
				packet.data.resize(Duck::Serialization::buffer_size(data));
				uint8_t* buf = packet.data.data();
				Duck::Serialization::serialize(buf, data);

				//Send the message packet
				_endpoint->bus()->send_packet(packet);
			} else {
				Duck::Log::err("[River] Tried sending message through proxy endpoint");
			}
		}

		const std::string& path() const override {
			return _path;
		}


		virtual void set_callback(std::function<void(T)> callback) {
			if(!_endpoint) {
				Duck::Log::err("[River] Tried setting callback for uninitialized message!");
				return;
			}

			if(_endpoint->type() == Endpoint::HOST) {
				Duck::Log::err("[River] Tried setting callback for message on host!");
				return;
			}

			_callback = callback;
		}

		void handle_message(const RiverPacket& packet) const override {
			if(!_callback)
				return;
			//if(packet.data.size() != sizeof(T)) TODO Size check
			//	return;
			T ret;
			const uint8_t* data = packet.data.data();
			Duck::Serialization::deserialize(data, ret);
			_callback(ret);
		}

	private:
		std::string _path;
		std::shared_ptr<Endpoint> _endpoint;
		std::function<void(T)> _callback = nullptr;
	};
}

