/*
    sdl_gui/graph.h -- Simple graph widget for showing a function plot

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <sdlgui/widget.h>
#include <memory>

NAMESPACE_BEGIN(sdlgui)

class  Graph : public Widget 
{
public:
    Graph(Widget *parent, const std::string &caption = "Untitled");

    const std::string &caption() const { return mCaption; }
    void setCaption(const std::string &caption) { mCaption = caption; _captionTex.dirty = true; }

    const std::string &header() const { return mHeader; }
    void setHeader(const std::string &header) { mHeader = header; _headerTex.dirty = true; }

    const std::string &footer() const { return mFooter; }
    void setFooter(const std::string &footer) { mFooter = footer; _footerTex.dirty = true; }

    const Color &backgroundColor() const { return mBackgroundColor; }
    void setBackgroundColor(const Color &backgroundColor) { mBackgroundColor = backgroundColor; }

    const Color &foregroundColor() const { return mForegroundColor; }
    void setForegroundColor(const Color &foregroundColor) { mForegroundColor = foregroundColor; }

    const Color &textColor() const { return mTextColor; }
    void setTextColor(const Color &textColor) { mTextColor = textColor; }

    const  std::vector<float>  &values() const { return mValues; }
    std::vector<float>  &values() { return mValues; }
    void setValues(const  std::vector<float>  &values) { mValues = values; }

    Vector2i preferredSize(SDL_Renderer *ctx) const override;
    void draw(SDL_Renderer *ctx) override;

protected:
    std::string mCaption, mHeader, mFooter;
    Color mBackgroundColor, mForegroundColor, mTextColor;
    std::vector<float>  mValues;

    Texture _captionTex;
    Texture _headerTex;
    Texture _footerTex;

    struct AsyncTexture;
    typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
    AsyncTexturePtr _atx;
};

NAMESPACE_END(sdlgui)
