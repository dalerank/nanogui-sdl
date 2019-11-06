/*
    src/dropdownbox.cpp -- simple dropdown box widget based on a popup button

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/dropdownbox.h>
#include <sdlgui/layout.h>
#include <algorithm>
#include <cassert>
#include <array>

#include "nanovg.h"
#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "nanovg_rt.h"

NAMESPACE_BEGIN(sdlgui)

class DropdownListItem : public Button
{
public:
  bool mInlist = true;

  DropdownListItem(Widget* parent, const std::string& str, bool inlist=true)
    : Button(parent, str), mInlist(inlist) {}

  void renderBodyTexture(NVGcontext* &ctx, int &realw, int &realh) override
  {
    int ww = width();
    int hh = height();
    ctx = nvgCreateRT(NVG_DEBUG, ww + 2, hh + 2, 0);

    float pxRatio = 1.0f;
    realw = ww + 2;
    realh = hh + 2;
    nvgBeginFrame(ctx, realw, realh, pxRatio);

    if (!mInlist)
    {
      Color gradTop = mTheme->mButtonGradientTopPushed;
      Color gradBot = mTheme->mButtonGradientBotPushed;

      nvgBeginPath(ctx);

      nvgRoundedRect(ctx, 1, 1, ww - 2,  hh - 2, mTheme->mButtonCornerRadius - 1);

      if (mBackgroundColor.a() != 0) 
      {
        Color rgb = mBackgroundColor.rgb();
        rgb.setAlpha(1.f);
        nvgFillColor(ctx, rgb.toNvgColor());
        nvgFill(ctx);
        gradTop.a() = gradBot.a() = 0.8f;
      }

      NVGpaint bg = nvgLinearGradient(ctx, 0, 0, 0, hh, gradTop.toNvgColor(), gradBot.toNvgColor());

      nvgFillPaint(ctx, bg);
      nvgFill(ctx);

      nvgBeginPath(ctx);
      nvgStrokeWidth(ctx, 1.0f);
      nvgRoundedRect(ctx, 0.5f, 0.5f, ww- 1, hh, mTheme->mButtonCornerRadius);
      nvgStrokeColor(ctx, mTheme->mBorderLight.toNvgColor());
      nvgStroke(ctx);

      nvgBeginPath(ctx);
      nvgRoundedRect(ctx, 0.5f, 0.5f, ww - 1, hh, mTheme->mButtonCornerRadius);
      nvgStrokeColor(ctx, mTheme->mBorderDark.toNvgColor());
      nvgStroke(ctx);
    }
    else
    {
      if (mMouseFocus && mEnabled)
      {
        Color gradTop = mTheme->mButtonGradientTopFocused;
        Color gradBot = mTheme->mButtonGradientBotFocused;

        nvgBeginPath(ctx);

        nvgRoundedRect(ctx, 1, 1, ww - 2, hh - 2, mTheme->mButtonCornerRadius - 1);

        if (mBackgroundColor.a() != 0) 
        {
          Color rgb = mBackgroundColor.rgb();
          rgb.setAlpha(1.f);
          nvgFillColor(ctx, rgb.toNvgColor());
          nvgFill(ctx);
          if (mPushed)
            gradTop.a() = gradBot.a() = 0.8f;
          else 
          {
            double v = 1 - mBackgroundColor.a();
            gradTop.a() = gradBot.a() = mEnabled ? v : v * .5f + .5f;
          }
        }

        NVGpaint bg = nvgLinearGradient(ctx, 0, 0, 0, hh, gradTop.toNvgColor(), gradBot.toNvgColor());

        nvgFillPaint(ctx, bg);
        nvgFill(ctx); 
      }
    }

    if (mPushed && mInlist)
    {
      Color textColor = mTextColor.a() == 0 ? mTheme->mTextColor : mTextColor;
      Vector2f center = mSize.cast<float>() * 0.5f;

      nvgBeginPath(ctx);
      nvgCircle(ctx, width() * 0.05f, center.y, 2);
      nvgFillColor(ctx, textColor.toNvgColor());
      nvgFill(ctx);
    }
    
    nvgEndFrame(ctx);
  }

  Vector2i getTextOffset() const override { return Vector2i(0, 0); }
};

class DropdownPopup : public Popup
{
public:
  int preferredWidth = 0;

  DropdownPopup(Widget *parent, Window *parentWindow)
    : Popup(parent, parentWindow)
  {
    _anchorDx = 0;
  }

  float targetPath = 0;
  void hide() { targetPath = 0; }

  Vector2i preferredSize(SDL_Renderer *ctx) const override
  {
    Vector2i result = Popup::preferredSize(ctx);
    result.x = preferredWidth;
    return result;
  }

  void refreshRelativePlacement() override
  {
    Popup::refreshRelativePlacement();
    mVisible &= mParentWindow->visibleRecursive();
    _pos = mParentWindow->position() + mAnchorPos;
  }

  void updateCaption(const std::string& caption)
  {
    if (mChildren.size() > 0)
    {
      auto* btn = dynamic_cast<Button*>(mChildren[0]);
      btn->setCaption(caption);
    }
  }

  void updateVisible(bool visible)
  {
    if (!visible)
    {
      if (path > 0) path -= 0.15f;
      if (path <= 0) path = 0.f;
    }
    else
    {
      if (path < 1.f) path += 0.15f;
      if (path > 1.f) path = 1.f;
    }

    mVisible = path > 0;
  }

  float path = 0.f;
  int clamp(int val, int min, int max) { return val < min ? min : (val > max ? max : val); }

  void rendereBodyTexture(NVGcontext* &ctx, int& realw, int& realh, int dx) override
  {
    int ds = 1, cr = mTheme->mWindowCornerRadius;
    int ww = mFixedSize.x > 0 ? mFixedSize.x : mSize.x;
    int hh = height();
    int dy = 0;
    int xadd = 1;

    int headerH = mChildren[0]->height();
    int realH = clamp(mSize.y * path, headerH, mSize.y);

    Vector2i offset(dx + ds, dy + ds);

    realw = ww + 2 * ds + dx + xadd; //with + 2*shadow + 2*boder + offset
    realh = hh + 2 * ds + dy + xadd;

    ctx = nvgCreateRT(NVG_DEBUG, realw, realh, 0);

    float pxRatio = 1.0f;
    nvgBeginFrame(ctx, realw, realh, pxRatio);

    // Draw a drop shadow 
    NVGpaint shadowPaint = nvgBoxGradient(ctx, 0, 0, realw, realh, cr * 2, ds * 2,
                                          mTheme->mDropShadow.toNvgColor(), mTheme->mTransparent.toNvgColor());

    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, ww + 2 * ds, hh + 2 * ds);
    //nvgRoundedRect(ctx, 0, 0, ww + 2 * ds, hh + 2 * ds, cr);
    //nvgPathWinding(ctx, NVG_HOLE);
    nvgFillPaint(ctx, shadowPaint);
    nvgFill(ctx);

    // Draw window
    nvgBeginPath(ctx);
    nvgRect(ctx, offset.x, offset.y, ww, hh);

    nvgFillColor(ctx, mTheme->mWindowPopup.toNvgColor());
    nvgFill(ctx);

    nvgEndFrame(ctx);
  }

  Vector2i getOverrideBodyPos() override
  {
    Vector2i ap = absolutePosition();
    int ds = 2;// mTheme->mWindowDropShadowSize;
    return ap - Vector2i(ds, ds);
  }

  void draw(SDL_Renderer* renderer) override
  {
    refreshRelativePlacement();

    if (!mVisible || mChildren.empty())
      return;

    drawBody(renderer);

    int ds = 1, cr = mTheme->mWindowCornerRadius;
    int ww = mFixedSize.x > 0 ? mFixedSize.x : mSize.x;

    int headerH = mChildren[0]->height();
    int realH = clamp(mSize.y * path, headerH, mSize.y);

    /*if (mChildren.size() > 1)
    {
      nvgBeginPath(ctx);

      Vector2i fp = mPos + mChildren[1]->position();
      NVGpaint bg = nvgLinearGradient(ctx, fp.x(), fp.y(), fp.x(), fp.y() + 12 ,
                                      mTheme->mBorderMedium, mTheme->mTransparent);
      nvgRect(ctx, fp.x(), fp.y(), ww, 12);
      nvgFillPaint(ctx, bg);
      nvgFill(ctx);
    }*/

    Widget::draw(renderer);
  }
};

DropdownBox::DropdownBox(Widget *parent) 
  : PopupButton(parent)
{
  mSelectedIndex = 0;
  Window *parentWindow = window();
  parentWindow->parent()->removeChild(mPopup);

  mPopup = new DropdownPopup(parentWindow->parent(), window());
  mPopup->setSize(Vector2i(320, 250));
  mPopup->setVisible(false);
  mPopup->setAnchorPos(Vector2i(0, 0));
}

DropdownBox::DropdownBox(Widget *parent, const std::vector<std::string> &items)
    : DropdownBox(parent) 
{
  setItems(items);
}

DropdownBox::DropdownBox(Widget *parent, const std::vector<std::string> &items, const std::vector<std::string> &itemsShort)
    : DropdownBox(parent) {
  setItems(items, itemsShort);
}

void DropdownBox::performLayout(SDL_Renderer *renderer) 
{
  PopupButton::performLayout(renderer);

  auto* dpopup = dynamic_cast<DropdownPopup*>(mPopup);
  if (dpopup)
  {
    dpopup->setAnchorPos(position());
    dpopup->preferredWidth = width();
  }
}

void DropdownBox::setSelectedIndex(int idx)
{
    if (mItemsShort.empty())
        return;

    const std::vector<Widget *> &children = popup().children();
    ((Button *) children[mSelectedIndex + 1])->setPushed(false);
    ((Button *) children[idx + 1])->setPushed(true);
    mSelectedIndex = idx;
    setCaption(mItemsShort[idx]);
    ((DropdownPopup*)mPopup)->updateCaption(mItemsShort[idx]);
}

void DropdownBox::setItems(const std::vector<std::string> &items, const std::vector<std::string> &itemsShort)
{
    assert(items.size() == itemsShort.size());
    mItems = items;
    mItemsShort = itemsShort;
    if (mSelectedIndex < 0 || mSelectedIndex >= (int) items.size())
        mSelectedIndex = 0;
    
    while (mPopup->childCount() != 0)
      mPopup->removeChild(mPopup->childCount() - 1);
   
    mPopup->setLayout(new GroupLayout(0,0,0,0));
    if (!items.empty())
    {
      DropdownListItem *button = new DropdownListItem(mPopup, items[mSelectedIndex], false);
      button->setPushed(false);
      button->setCallback([&] { setPushed(false); popup().setVisible(false); });
    }

    int index = 0;
    for (const auto &str: items) {
        DropdownListItem *button = new DropdownListItem(mPopup, str);
        button->setFlags(Button::RadioButton);
        button->setCallback([&, index] {
            setSelectedIndex(index);
            setPushed(false);
            if (mCallback)
                mCallback(index);
        });
        index++;
    }
    setSelectedIndex(mSelectedIndex);
}

bool DropdownBox::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) 
{
  if (button == SDL_BUTTON_LEFT && mEnabled) {
    if (!mItems.empty())
    {
      auto* item = dynamic_cast<DropdownListItem*>(mPopup->childAt(0));
      if (item)
        item->setCaption(mItems[mSelectedIndex]);
    }
  }

  return PopupButton::mouseButtonEvent(p, button, down, modifiers);
}

bool DropdownBox::scrollEvent(const Vector2i &p, const Vector2f &rel) 
{
    if (rel.y < 0) 
    {
        setSelectedIndex(std::min(mSelectedIndex+1, (int)(items().size()-1)));
        if (mCallback)
            mCallback(mSelectedIndex);
        return true;
    } 
    else if (rel.y > 0) 
    {
        setSelectedIndex(std::max(mSelectedIndex-1, 0));
        if (mCallback)
            mCallback(mSelectedIndex);
        return true;
    }
    return PopupButton::scrollEvent(p, rel);
}

void DropdownBox::draw(SDL_Renderer* renderer) 
{
  if (!mEnabled && mPushed)
    mPushed = false;

  if (auto pp = dynamic_cast<DropdownPopup*>(mPopup))
    pp->updateVisible(mPushed);
  
  Button::draw(renderer);

  if (mChevronIcon)
  {
    auto icon = utf8(mChevronIcon);
    Color textColor = mTextColor.a() == 0 ? mTheme->mTextColor : mTextColor;

  /*  nvgFontSize(ctx, (mFontSize < 0 ? mTheme->mButtonFontSize : mFontSize) * icon_scale());
    nvgFontFace(ctx, "icons");
    nvgFillColor(ctx, mEnabled ? textColor : mTheme->mDisabledTextColor);
    nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

    float iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
    Vector2f iconPos(0, mPos.y() + mSize.y() * 0.5f - 1);

    if (mPopup->side() == Popup::Right)
      iconPos[0] = mPos.x() + mSize.x() - iw - 8;
    else
      iconPos[0] = mPos.x() + 8;

    nvgText(ctx, iconPos.x(), iconPos.y(), icon.data(), nullptr); */
  }
}

NAMESPACE_END(sdlgui)
