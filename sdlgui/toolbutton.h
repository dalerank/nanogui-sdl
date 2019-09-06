/*
    sdlgui/toolbutton.h -- Simple radio+toggle button with an icon

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <sdlgui/button.h>

NAMESPACE_BEGIN(sdlgui)

class ToolButton : public Button
{
public:
    ToolButton(Widget *parent, int icon,
           const std::string &caption = "")
        : Button(parent, caption, icon)
    {
        setFlags(Flags::RadioButton | Flags::ToggleButton);
        setFixedSize(Vector2i(25, 25));
    }
};

NAMESPACE_END(sdlgui)
