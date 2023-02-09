/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "DataSize.h"

using namespace Duck;

double precise_human_size(unsigned long bytes) {
	if(bytes > DataSize::GiB) {
		return bytes / (double) DataSize::GiB;
	} else if(bytes > DataSize::MiB) {
		return bytes / (double) DataSize::MiB;
	} else if(bytes > DataSize::KiB) {
		return bytes / (double) DataSize::KiB;
	}
	return bytes;
}

size_t round_human_size(unsigned long bytes) {
	if(bytes > DataSize::GiB) {
		return bytes / DataSize::GiB;
	} else if(bytes > DataSize::MiB) {
		return bytes / DataSize::MiB;
	} else if(bytes > DataSize::KiB) {
		return bytes / DataSize::KiB;
	}
	return bytes;
}

const char* long_human_suffix(unsigned long bytes) {
	if(bytes > DataSize::GiB) {
		return "GiB";
	} else if(bytes > DataSize::MiB) {
		return "MiB";
	} else if(bytes > DataSize::KiB) {
		return "KiB";
	} else return "bytes";
}

const char* short_human_suffix(unsigned long bytes) {
	if(bytes > DataSize::GiB) {
		return "G";
	} else if(bytes > DataSize::MiB) {
		return "M";
	} else if(bytes > DataSize::KiB) {
		return "K";
	}
	return "";
}

std::string DataSize::readable(DataSize::Precision precision, DataSize::Suffix suffix) const {
	char buf[64];
	const char* suffix_str = suffix == Suffix::Short ? short_human_suffix(bytes) : long_human_suffix(bytes);
	if(precision == Precision::Precise)
		snprintf(buf, 512, "%.2f%s", precise_human_size(bytes), suffix_str);
	else
		snprintf(buf, 512, "%ld%s", round_human_size(bytes), suffix_str);
	return buf;
}