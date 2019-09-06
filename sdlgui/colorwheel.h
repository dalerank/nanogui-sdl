/*
    sdl_gui/colorwheel.h -- fancy analog widget to select a color value

    This widget was contributed by Dmitriy Morozov.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <sdlgui/widget.h>

NAMESPACE_BEGIN(sdlgui)

class  ColorWheel : public Widget
{
public:
    ColorWheel(Widget *parent, const Color& color = { 1.f, 0.f, 0.f, 1.f });

    /// Set the change callback
    std::function<void(const Color &)> callback() const                  { return mCallback;     }
    void setCallback(const std::function<void(const Color &)> &callback) { mCallback = callback; }

    /// Get the current color
    Color color() const;
    /// Set the current color
    void setColor(const Color& color);

    Vector2i preferredSize(SDL_Renderer *ctx) const override;
    void draw(SDL_Renderer *renderer) override;
    bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
    bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);

private:
    enum Region {
        None = 0,
        InnerTriangle = 1,
        OuterCircle = 2,
        Both = 3
    };

    Color hue2rgb(float h) const;
    Region adjustPosition(const Vector2i &p, Region consideredRegions = Both);

protected:
    float mHue;
    float mWhite;
    float mBlack;
    Region mDragRegion;
    std::function<void(const Color &)> mCallback;
};

NAMESPACE_END(sdlgui)
