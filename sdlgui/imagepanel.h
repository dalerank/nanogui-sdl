/*
    sdl_gui/imagepanel.h -- Image panel widget which shows a number of
    square-shaped icons

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
 * \class ImagePanel imagepanel.h sdl_gui/imagepanel.h
 *
 * \brief Image panel widget which shows a number of square-shaped icons.
 */
class  ImagePanel : public Widget 
{
public:
    ImagePanel(Widget *parent);
    ImagePanel(Widget *parent, const ListImages &data)
      : ImagePanel(parent) { setImages(data); }

    void setImages(const ListImages &data) { mImages = data; }
    const ListImages& images() const { return mImages; }

    std::function<void(int)> callback() const { return mCallback; }
    void setCallback(const std::function<void(int)> &callback) { mCallback = callback; }

    bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override;
    Vector2i preferredSize(SDL_Renderer *ctx) const override;
    void draw(SDL_Renderer* renderer) override;

    ImagePanel& withImages(const ListImages& data ) { setImages(data); return *this; }
protected:
  Vector2i gridSize() const;
    int indexForPosition(const Vector2i &p) const;
protected:
  ListImages mImages;
    std::function<void(int)> mCallback;
    int mThumbSize;
    int mSpacing;
    int mMargin;
    int mMouseIndex;
};

NAMESPACE_END(sdlgui)
