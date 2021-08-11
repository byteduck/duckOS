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

#include "Endpoint.h"
#include "Function.hpp"

using namespace River;

Endpoint::Endpoint(BusConnection* bus, const std::string& name, ConnectionType type): _bus(bus), _type(type), _name(name) {

}

std::shared_ptr<IFunction> Endpoint::get_ifunction(const std::string& path) {
	return _functions[path];
}

const std::string& Endpoint::name() {
	return _name;
}

Endpoint::ConnectionType Endpoint::type() const {
	return _type;
}

BusConnection* Endpoint::bus() {
	return _bus;
}
