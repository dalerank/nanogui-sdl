/*
    sdlgui/colorpicker.h -- push button with a popup to tweak a color value

    This widget was contributed by Christian Schueller.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <sdlgui/popupbutton.h>

NAMESPACE_BEGIN(sdlgui)

class ColorWheel;

class  ColorPicker : public PopupButton 
{
public:
    ColorPicker(Widget *parent, const Color& color = { 1.f, 0.f, 0.f, 1.f });

    /// Set the change callback
    std::function<void(const Color &)> callback() const                  { return mCallback; }
    void setCallback(const std::function<void(const Color &)> &callback) { mCallback = callback; }

    /// Get the current color
    Color color() const;
    /// Set the current color
    void setColor(const Color& color);

protected:
    std::function<void(const Color &)> mCallback;
    ColorWheel *mColorWheel;
    Button *mPickButton;
};

NAMESPACE_END(sdlgui)
