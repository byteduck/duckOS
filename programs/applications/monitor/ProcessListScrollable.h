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

#ifndef DUCKOS_PROCESSLISTSCROLLABLE_H
#define DUCKOS_PROCESSLISTSCROLLABLE_H

#include <libui/widget/ListScrollable.h>
#include <libsys/Process.h>

class ProcessListScrollable: public UI::ListScrollable {
public:
    WIDGET_DEF(ProcessListScrollable);
    void update();

protected:
    Widget::Ptr create_entry(int index) override;
    Dimensions preferred_item_dimensions() override;
    int num_items() override;

private:
    ProcessListScrollable();
    std::vector<Sys::Process> _processes;
};

#endif //DUCKOS_PROCESSLISTSCROLLABLE_H
