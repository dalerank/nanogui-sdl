/*
    sdlgui/colorpicker.cpp -- push button with a popup to tweak a color value

    This widget was contributed by Christian Schueller.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/colorpicker.h>
#include <sdlgui/layout.h>
#include <sdlgui/colorwheel.h>

NAMESPACE_BEGIN(sdlgui)

ColorPicker::ColorPicker(Widget *parent, const Color& color) 
  : PopupButton(parent, "") 
{
    setBackgroundColor(color);
    Popup& p = this->popup();
    p.setLayout(new GroupLayout());

    mColorWheel = new ColorWheel(&p);
    mPickButton = new Button(&p, "Pick");
    mPickButton->setFixedSize(Vector2i(100, 25));

    PopupButton::setChangeCallback([&](bool) {
        setColor(backgroundColor());
        mCallback(backgroundColor());
    });

    mColorWheel->setCallback([&](const Color &value) {
        mPickButton->setBackgroundColor(value);
        mPickButton->setTextColor(value.contrastingColor());
        mCallback(value);
    });

    mPickButton->setCallback([&]() {
        Color value = mColorWheel->color();
        setPushed(false);
        setColor(value);
        mCallback(value);
    });
}

Color ColorPicker::color() const 
{
    return backgroundColor();
}

void ColorPicker::setColor(const Color& color) {
    /* Ignore setColor() calls when the user is currently editing */
    if (!mPushed) {
        Color fg = color.contrastingColor();
        setBackgroundColor(color);
        setTextColor(fg);
        mColorWheel->setColor(color);
        mPickButton->setBackgroundColor(color);
        mPickButton->setTextColor(fg);
    }
}

NAMESPACE_END(sdlgui)
