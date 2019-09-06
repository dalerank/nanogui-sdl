/*
    sdlgui/popup.cpp -- Simple popup widget which is attached to another given
    window (can be nested)

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/popup.h>
#include <sdlgui/theme.h>

#include "nanovg.h"
#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "nanovg_rt.h"

NAMESPACE_BEGIN(sdlgui)

struct Popup::AsyncTexture
{
  int id;
  Texture tex;
  NVGcontext* ctx = nullptr;

  AsyncTexture(int _id) : id(_id) {};

  void load(Popup* ptr, int dx)
  {
    Popup* pp = ptr;
    AsyncTexture* self = this;
    std::thread tgr([=]() {
      Theme* mTheme = pp->theme();

      int ww = pp->width();
      int hh = pp->height();
      int ds = mTheme->mWindowDropShadowSize;
      int dy = 0;

      Vector2i offset(dx + ds, dy + ds);

      int realw = ww + 2 * ds + dx; //with + 2*shadow + offset
      int realh = hh + 2 * ds + dy;
      NVGcontext *ctx = nvgCreateRT(NVG_DEBUG, realw, realh);

      float pxRatio = 1.0f;
      nvgClearBackgroundRT(ctx, 0, 0, 0, 0.0f);
      nvgBeginFrame(ctx, realw, realh, pxRatio);

      int cr = mTheme->mWindowCornerRadius;

      /* Draw a drop shadow */
      NVGpaint shadowPaint = nvgBoxGradient(ctx, offset.x, offset.y, ww, hh, cr * 2, ds * 2,
                                            mTheme->mDropShadow.toNvgColor(),
                                            mTheme->mTransparent.toNvgColor());

      nvgBeginPath(ctx);
      //nvgRect(ctx, offset.x - ds, offset.y - ds, ww + 2 * ds, hh + 2 * ds);
      nvgRoundedRect(ctx, offset.x - ds, offset.y - ds, ww + 2*ds, hh+2*ds, cr);
      //nvgPathWinding(ctx, NVG_HOLE);
      nvgFillPaint(ctx, shadowPaint);
      nvgFill(ctx);

      /* Draw window */
      nvgBeginPath(ctx);
      nvgRoundedRect(ctx, offset.x, offset.y, ww, hh, cr);

      Vector2i base = Vector2i(offset.x + 0, offset.y + pp->anchorHeight());
      int sign = -1;

      nvgMoveTo(ctx, base.x + 15 * sign, base.y);
      nvgLineTo(ctx, base.x - 1 * sign, base.y - 15);
      nvgLineTo(ctx, base.x - 1 * sign, base.y + 15);

      nvgFillColor(ctx, mTheme->mWindowPopup.toNvgColor());
      nvgFill(ctx);
      nvgEndFrame(ctx);

      self->tex.rrect = { 0, 0, realw, realh };
      self->ctx = ctx;
    });

    tgr.detach();
  }

  void perform(SDL_Renderer* renderer)
  {
    if (!ctx)
      return;

    unsigned char *rgba = nvgReadPixelsRT(ctx);

    tex.tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, tex.w(), tex.h());

    int pitch;
    uint8_t *pixels;
    int ok = SDL_LockTexture(tex.tex, nullptr, (void **)&pixels, &pitch);
    memcpy(pixels, rgba, sizeof(uint32_t) * tex.w() * tex.h());
    SDL_SetTextureBlendMode(tex.tex, SDL_BLENDMODE_BLEND);
    SDL_UnlockTexture(tex.tex);

    nvgDeleteRT(ctx);
    ctx = nullptr;
  }

};


Popup::Popup(Widget *parent, Window *parentWindow)
    : Window(parent, ""), mParentWindow(parentWindow),
      mAnchorPos(Vector2i::Zero()), mAnchorHeight(30)
{
}

void Popup::performLayout(SDL_Renderer *ctx) 
{
    if (mLayout || mChildren.size() != 1) 
    {
        Widget::performLayout(ctx);
    } 
    else 
    {
        mChildren[0]->setPosition(Vector2i::Zero());
        mChildren[0]->setSize(mSize);
        mChildren[0]->performLayout(ctx);
    }
}

void Popup::refreshRelativePlacement() 
{
    mParentWindow->refreshRelativePlacement();
    mVisible &= mParentWindow->visibleRecursive();
    _pos = mParentWindow->position() + mAnchorPos - Vector2i(0, mAnchorHeight);
}

void Popup::drawBody(SDL_Renderer* renderer)
{
  int id = 1;

  AsyncTexturePtr atx;
  for (auto& txid : _txs)
  {
    if (txid->id == id)
    {
      atx = txid;
      break;
    }
  }

  if (atx)
  {
    Vector2i ap = absolutePosition();
    atx->perform(renderer);
    int ds = mTheme->mWindowDropShadowSize;
    SDL_RenderCopy(renderer, atx->tex, ap - Vector2i(_anchorDx + ds, ds));
  }
  else
  {
    AsyncTexturePtr newtx = std::make_shared<AsyncTexture>(id);
    newtx->load(this, _anchorDx);
    _txs.push_back(newtx);
  }
}

void Popup::draw(SDL_Renderer* renderer)
{
  refreshRelativePlacement();

  if (!mVisible)
    return;

  drawBody(renderer);
  
  Widget::draw(renderer);
}

NAMESPACE_END(sdlgui)
