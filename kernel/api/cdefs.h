/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#ifndef DUCKOS_CDEFS_H
#define DUCKOS_CDEFS_H

#ifdef __cplusplus
#ifndef __DECL_BEGIN
#define __DECL_BEGIN extern "C" {
#define __DECL_END }
#endif
#else
#ifndef __DECL_BEGIN
		#define __DECL_BEGIN
		#define __DECL_END
	#endif
#endif

#endif //DUCKOS_CDEFSL_H