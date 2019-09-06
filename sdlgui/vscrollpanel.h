/*
    sdl_gui/vscrollpanel.h -- Adds a vertical scrollbar around a widget
    that is too big to fit into a certain area

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <sdlgui/widget.h>

NAMESPACE_BEGIN(sdlgui)

/**
 * \class VScrollPanel vscrollpanel.h sdl_gui/vscrollpanel.h
 *
 * \brief Adds a vertical scrollbar around a widget that is too big to fit into
 *        a certain area.
 */
class  VScrollPanel : public Widget 
{
public:
    VScrollPanel(Widget *parent);

    void performLayout(SDL_Renderer *ctx) override;
    Vector2i preferredSize(SDL_Renderer *ctx) const override;
    bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    bool scrollEvent(const Vector2i &p, const Vector2f &rel) override;
    bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override;
    bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    void draw(SDL_Renderer *render) override;

    SDL_Point getAbsolutePos() const override;
    PntRect getAbsoluteCliprect() const override;
    int getAbsoluteTop() const override;

protected:
    int mChildPreferredHeight;
    float mScroll;
    int mDOffset = 0;
};

NAMESPACE_END(sdlgui)
