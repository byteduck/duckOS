/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Client.h"

int main() {
	auto client = Client::make();
	if (client.has_value())
		client.value()->loop();
}