/*
    sdlgui/label.cpp -- Text label with an arbitrary font, color, and size

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/label.h>
#include <sdlgui/theme.h>

NAMESPACE_BEGIN(sdlgui)

Label::Label(Widget *parent, const std::string &caption, const std::string &font, int fontSize)
    : Widget(parent), mCaption(caption), mFont(font)
{
    if (mTheme) 
    {
        mFontSize = mTheme->mStandardFontSize;
        mColor = mTheme->mTextColor;
    }
    
    if (fontSize >= 0) 
      mFontSize = fontSize;

    _texture.dirty = true;
}

void Label::setTheme(Theme *theme) 
{
    Widget::setTheme(theme);
    if (mTheme) 
    {
        mFontSize = mTheme->mStandardFontSize;
        mColor = mTheme->mTextColor;
    }
}

Vector2i Label::preferredSize(SDL_Renderer *ctx) const
{
    if (mCaption == "")
        return Vector2i::Zero();
    
    if (mFixedSize.x > 0) 
    {
      int w, h;
      const_cast<Label*>(this)->mTheme->getUtf8Bounds(mFont.c_str(), fontSize(), mCaption.c_str(), &w, &h);
      return Vector2i(mFixedSize.x, h);
    } 
    else 
    {
      int w, h;
      const_cast<Label*>(this)->mTheme->getUtf8Bounds(mFont.c_str(), fontSize(), mCaption.c_str(), &w, &h);
      return Vector2i(w, mTheme->mStandardFontSize);
    }
}

void Label::setFontSize(int fontSize)
{
  Widget::setFontSize(fontSize);
  _texture.dirty = true;
}

void Label::draw(SDL_Renderer *renderer)
{
  Widget::draw(renderer);

  if (_texture.dirty)
    mTheme->getTexAndRectUtf8(renderer, _texture, 0, 0, mCaption.c_str(), mFont.c_str(), fontSize(), mColor);

  if (mFixedSize.x > 0) 
    SDL_RenderCopy(renderer, _texture, absolutePosition());
  else 
    SDL_RenderCopy(renderer, _texture, absolutePosition() + Vector2i(0, (mSize.y - _texture.rrect.h) * 0.5f));
}

NAMESPACE_END(sdlgui)
