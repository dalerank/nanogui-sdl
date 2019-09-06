/*
    sdlgui/vscrollpanel.cpp -- Adds a vertical scrollbar around a widget
    that is too big to fit into a certain area

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/vscrollpanel.h>
#include <sdlgui/theme.h>

NAMESPACE_BEGIN(sdlgui)

VScrollPanel::VScrollPanel(Widget *parent)
    : Widget(parent), mChildPreferredHeight(0), mScroll(0.0f) 
{ }

void VScrollPanel::performLayout(SDL_Renderer *ctx) 
{
    Widget::performLayout(ctx);

    if (mChildren.empty())
        return;
    Widget *child = mChildren[0];
    mChildPreferredHeight = child->preferredSize(ctx).y;
    child->setPosition({ 0, 0 });
    child->setSize({ mSize.x - 12, mChildPreferredHeight });
}

Vector2i VScrollPanel::preferredSize(SDL_Renderer *ctx) const
{
    if (mChildren.empty())
      return{ 0, 0 };
    return mChildren[0]->preferredSize(ctx) + Vector2i(12, 0);
}

bool VScrollPanel::mouseDragEvent(const Vector2i &, const Vector2i &rel,  int, int)
{
    if (mChildren.empty())
        return false;

    float scrollh = height() *
        std::min(1.0f, height() / (float)mChildPreferredHeight);

    mScroll = std::max((float) 0.0f, std::min((float) 1.0f,
                 mScroll + rel.y / (float)(mSize.y - 8 - scrollh)));
    return true;
}

bool VScrollPanel::scrollEvent(const Vector2i &/* p */, const Vector2f &rel)
{
    float scrollAmount = rel.y * (mSize.y / 20.0f);
    float scrollh = height() *
        std::min(1.0f, height() / (float)mChildPreferredHeight);

    mScroll = std::max((float) 0.0f, std::min((float) 1.0f,
            mScroll - scrollAmount / (float)(mSize.y - 8 - scrollh)));
    return true;
}

bool VScrollPanel::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers)
{
    if (mChildren.empty())
        return false;
    int shift = (int) (mScroll*(mChildPreferredHeight - mSize.y));
    return mChildren[0]->mouseButtonEvent(p - _pos + Vector2i{ 0, shift }, button, down, modifiers);
}

bool VScrollPanel::mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers)
{
    if (mChildren.empty())
        return false;
    int shift = (int) (mScroll*(mChildPreferredHeight - mSize.y));
    return mChildren[0]->mouseMotionEvent(p - _pos + Vector2i{ 0, shift }, rel, button, modifiers);
}

void VScrollPanel::draw(SDL_Renderer *renderer) 
{
    if (mChildren.empty())
        return;

    Widget *child = mChildren[0];
    mChildPreferredHeight = child->preferredSize(nullptr).y;
    float scrollh = height() * std::min(1.0f, height() / (float) mChildPreferredHeight);

    SDL_Point ap = getAbsolutePos();
    SDL_Rect brect{ ap.x, ap.y, width(), height() };

    //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    //SDL_RenderDrawRect(renderer, &brect);

    if (child->visible())
    {
      const Vector2i savepos = child->position();
      Vector2i npos = savepos;
      mDOffset = -mScroll*(mChildPreferredHeight - mSize.y);
      npos.y += mDOffset;
      child->setPosition(npos);
      child->draw(renderer);
      child->setPosition(savepos);
    }

    SDL_Color sc = mTheme->mBorderDark.toSdlColor();
    SDL_Rect srect{ ap.x + mSize.x - 12, ap.y + 4, 8, mSize.y - 8 };

    SDL_SetRenderDrawColor(renderer, sc.r, sc.g, sc.b, sc.a);
    SDL_RenderFillRect(renderer, &srect);
      
    SDL_Color ss = mTheme->mBorderLight.toSdlColor();
    SDL_Rect drect{ ap.x + mSize.x - 12 + 1, ap.y + 4 + (mSize.y - 8 - scrollh) * mScroll + 1, 6, scrollh - 1 };

    SDL_SetRenderDrawColor(renderer, ss.r, ss.g, ss.b, ss.a);
    SDL_RenderFillRect(renderer, &drect);
}


SDL_Point VScrollPanel::getAbsolutePos() const
{
  return Widget::getAbsolutePos();
}

PntRect VScrollPanel::getAbsoluteCliprect() const
{
  return Widget::getAbsoluteCliprect();
}

int VScrollPanel::getAbsoluteTop() const
{
  return Widget::getAbsoluteTop();
}

NAMESPACE_END(sdlgui)
