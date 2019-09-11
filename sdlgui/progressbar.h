/*
    sdl_gui/progressbar.h -- Standard widget for visualizing progress

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <sdlgui/widget.h>
#include <memory>

NAMESPACE_BEGIN(sdlgui)

/**
 * \class ProgressBar progressbar.h sdl_gui/progressbar.h
 *
 * \brief Standard widget for visualizing progress.
 */
class  ProgressBar : public Widget 
{
public:
    ProgressBar(Widget *parent);

    float value() { return mValue; }
    void setValue(float value);

    Vector2i preferredSize(SDL_Renderer *ctx) const override;
    void draw(SDL_Renderer* renderer) override;
    void drawBody(SDL_Renderer* renderer);
    void drawBar(SDL_Renderer* renderer);

protected:
  float mValue;

    struct AsyncTexture;
    typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
    AsyncTexturePtr _body;
    AsyncTexturePtr _bar;
};

NAMESPACE_END(sdlgui)
