/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "Processor.h"
#include "arch/i386/CPUID.h"
#include "kstd/cstring.h"
#include "kstd/KLog.h"

char Processor::s_vendor[sizeof(uint32_t) * 3 + 1];
CPUFeatures Processor::s_features = {};

void Processor::init() {
	// Get vendor string
	CPUID id = cpuid(CPUIDOp::Vendor);
	memcpy(&s_vendor[sizeof(uint32_t) * 0], &id.ebx, sizeof(uint32_t));
	memcpy(&s_vendor[sizeof(uint32_t) * 1], &id.edx, sizeof(uint32_t));
	memcpy(&s_vendor[sizeof(uint32_t) * 2], &id.ecx, sizeof(uint32_t));
	s_vendor[sizeof(uint32_t) * 3] = '\0';

	KLog::dbg("Processor", "CPU Detected: {}", s_vendor);

	// Get features
	id = cpuid(CPUIDOp::Features);
	s_features.ecx_value = id.ecx;
	s_features.edx_value = id.edx;
}

Processor::CPUID Processor::cpuid(uint32_t op) {
	CPUID ret {};
	asm volatile(
		"cpuid" :
		"=a" (ret.eax), "=b" (ret.ebx), "=c" (ret.ecx), "=d" (ret.edx) :
		"a" (op)
	);
	return ret;
}

void Processor::save_fpu_state(void*& fpu_state) {
	if (s_features.FXSR)
		asm volatile("fxsave %0" : "=m"(fpu_state));
	else
		asm volatile("fnsave %0" : "=m"(fpu_state));
}

void Processor::load_fpu_state(void*& fpu_state) {
	if (s_features.FXSR)
		asm volatile("fxrstor %0" :: "m"(fpu_state));
	else
		asm volatile("frstor %0" :: "m"(fpu_state));
}
