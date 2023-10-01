/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libduck/Result.h>
#include "SoundSource.h"

namespace Sound {
	extern "C" void* libsound_thread_entry(void* ptr);

	Duck::Result init();
	Duck::Ptr<SoundSource> add_source(Duck::Ptr<WavReader> reader);
	void wait();
}