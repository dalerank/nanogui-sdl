/*
    sdlgui/popupbutton.cpp -- Button which launches a popup widget

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/popupbutton.h>
#include <sdlgui/entypo.h>
#include <sdlgui/theme.h>
#include <array>

NAMESPACE_BEGIN(sdlgui)

PopupButton::PopupButton(Widget *parent, const std::string &caption,
                         int buttonIcon, int chevronIcon)
    : Button(parent, caption, buttonIcon), mChevronIcon(chevronIcon) 
{
    setFlags(Flags::ToggleButton | Flags::PopupButton);

    Window *parentWindow = window();
    mPopup = new Popup(parentWindow->parent(), window());
    mPopup->setSize(Vector2i(320, 250));
    mPopup->setVisible(false);
}

Vector2i PopupButton::preferredSize(SDL_Renderer *ctx) const
{
    return Button::preferredSize(ctx) + Vector2i(15, 0);
}

void PopupButton::draw(SDL_Renderer* renderer)
{
  if (!mEnabled && mPushed)
    mPushed = false;

  mPopup->setVisible(mPushed);
  Button::draw(renderer);

  if (mChevronIcon) 
  {
    if (_chevronTex.dirty)
    {
      auto icon = utf8(mChevronIcon);
      Color textColor = (mTextColor.a() == 0 ? mTheme->mTextColor : mTextColor);
      if (!mEnabled)
        textColor = mTheme->mDisabledTextColor;
      int fntsize = (mFontSize < 0 ? mTheme->mButtonFontSize : mFontSize) * 1.5f;
      mTheme->getTexAndRectUtf8(renderer, _chevronTex, 0, 0, icon.data(), "icons", fntsize, textColor);
    }

    Vector2i ap = absolutePosition();
    SDL_RenderCopy(renderer, _chevronTex, ap + Vector2i(mSize.x - _chevronTex.w() - 8, mSize.y * 0.5f - 1) );
  }
}

void PopupButton::performLayout(SDL_Renderer *ctx) 
{
    Widget::performLayout(ctx);

    const Window *parentWindow = window();

    mPopup->setAnchorPos(Vector2i(parentWindow->width() + 15,
                         absolutePosition().y - parentWindow->position().y + mSize.y /2));
}

NAMESPACE_END(sdlgui)
