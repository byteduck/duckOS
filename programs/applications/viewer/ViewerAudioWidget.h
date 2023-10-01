/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <libui/widget/layout/BoxLayout.h>
#include <libsound/SoundSource.h>
#include <libui/widget/Button.h>
#include <libui/widget/ProgressBar.h>
#include <libui/Timer.h>

class ViewerAudioWidget: public UI::BoxLayout {
public:
	WIDGET_DEF(ViewerAudioWidget)

private:
	ViewerAudioWidget(Duck::Ptr<Sound::WavReader> reader);

	void update();

	Duck::Ptr<Sound::SoundSource> m_source;
	Duck::Ptr<UI::Button> m_play_button;
	Duck::Ptr<UI::Button> m_stop_button;
	Duck::Ptr<UI::Button> m_ff_button;
	Duck::Ptr<UI::Button> m_rev_button;
	Duck::Ptr<UI::ProgressBar> m_progress_bar;
	Duck::Ptr<UI::Timer> m_timer;
	Duck::Ptr<UI::Label> m_time_label;
};
