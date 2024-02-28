/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <sys/keyboard.h>

namespace Keyboard {
	enum Key {
		None = 0x00,

		One = 0x02,
		Two = 0x03,
		Three = 0x04,
		Four = 0x05,
		Five = 0x06,
		Six = 0x07,
		Seven = 0x08,
		Eight = 0x09,
		Nine = 0x0a,
		Zero = 0x0b,

		Hyphen = 0x0c,
		Equals = 0x0d,
		Backspace = 0x0e,

		Q = 0x10,
		W = 0x11,
		E = 0x12,
		R = 0x13,
		T = 0x14,
		Y = 0x15,
		U = 0x16,
		I = 0x17,
		O = 0x18,
		P = 0x19,
		A = 0x1e,
		S = 0x1f,
		D = 0x20,
		F = 0x21,
		G = 0x22,
		H = 0x23,
		J = 0x24,
		K = 0x25,
		L = 0x26,
		Z = 0x2c,
		X = 0x2d,
		C = 0x2e,
		V = 0x2f,
		B = 0x30,
		N = 0x31,
		M = 0x32,

		LBracket = 0x1a,
		RBracket = 0x1b,
		Colon = 0x27,
		Quote = 0x28,
		Backtick = 0x29,
		Backslash = 0x2b,
		Comma = 0x33,
		Period = 0x34,
		Slash = 0x35,
		Space = 0x39,

		Tab = 0x0f,
		Enter = 0x1c,

		Esc = 0x01,
		LCtrl = 0x1d,
		LShift = 0x2a,
		RShift = 0x36,
		LAlt = 0x38,
		CapsLock = 0x3a,
		NumLock = 0x45,
		ScrollLock = 0x46,

		F1 = 0x3b,
		F2 = 0x3c,
		F3 = 0x3d,
		F4 = 0x3e,
		F5 = 0x3f,
		F6 = 0x40,
		F7 = 0x41,
		F8 = 0x42,
		F9 = 0x43,
		F10 = 0x44,

		Up = 0x48,
		Left = 0x4b,
		Right = 0x4d,
		Down = 0x50
	};

	enum Modifier {
		Alt = KBD_MOD_ALT,
		Ctrl = KBD_MOD_CTRL,
		Shift = KBD_MOD_SHIFT,
		Super = KBD_MOD_SUPER,
		AltGr = KBD_MOD_ALTGR
	};

	struct Shortcut {
		Key key = (Key) 0;
		Modifier modifiers = (Modifier) 0;
	};

	inline Modifier operator|(Modifier a, Modifier b) {
		return (Modifier) ((int) a | (int) b);
	}

	inline bool operator==(Shortcut a, Shortcut b) {
		return a.key == b.key && a.modifiers == b.modifiers;
	}
}