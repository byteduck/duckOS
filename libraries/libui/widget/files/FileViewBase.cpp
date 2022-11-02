/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "FileViewBase.h"
#include <libduck/Log.h>

Duck::Result UI::FileViewBase::set_directory(Duck::Path path) {
	auto dirs = TRY(Duck::Path(path).get_directory_entries());
	m_directory = path;
	m_entries = dirs;
	m_selected.clear();
	did_set_directory(path);
	if(!delegate.expired()) {
		delegate.lock()->fv_did_navigate(path);
		delegate.lock()->fv_did_select_files(m_selected);
	}
	return Duck::Result::SUCCESS;
}
