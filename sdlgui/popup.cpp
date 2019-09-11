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
#include <thread>

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
      std::lock_guard<std::mutex> guard(pp->theme()->loadMutex);

      NVGcontext *ctx = nullptr;
      int realw, realh;
      pp->rendereBodyTexture(ctx, realw, realh, dx);
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

void Popup::rendereBodyTexture(NVGcontext*& ctx, int& realw, int& realh, int dx)
{
  int ww = width();
  int hh = height();
  int ds = mTheme->mWindowDropShadowSize;
  int dy = 0;

  Vector2i offset(dx + ds, dy + ds);

  realw = ww + 2 * ds + dx; //with + 2*shadow + offset
  realh = hh + 2 * ds + dy;
  
  ctx = nvgCreateRT(NVG_DEBUG, realw, realh, 0);

  float pxRatio = 1.0f;
  nvgBeginFrame(ctx, realw, realh, pxRatio);

  int cr = mTheme->mWindowCornerRadius;

  /* Draw a drop shadow */
  NVGpaint shadowPaint = nvgBoxGradient(ctx, offset.x, offset.y, ww, hh, cr * 2, ds * 2,
    mTheme->mDropShadow.toNvgColor(),
    mTheme->mTransparent.toNvgColor());

  nvgBeginPath(ctx);
  //nvgRect(ctx, offset.x - ds, offset.y - ds, ww + 2 * ds, hh + 2 * ds);
  nvgRoundedRect(ctx, offset.x - ds, offset.y - ds, ww + 2 * ds, hh + 2 * ds, cr);
  //nvgPathWinding(ctx, NVG_HOLE);
  nvgFillPaint(ctx, shadowPaint);
  nvgFill(ctx);

  /* Draw window */
  nvgBeginPath(ctx);
  nvgRoundedRect(ctx, offset.x, offset.y, ww, hh, cr);

  Vector2i base = Vector2i(offset.x + 0, offset.y + anchorHeight());
  int sign = -1;

  nvgMoveTo(ctx, base.x + 15 * sign, base.y);
  nvgLineTo(ctx, base.x - 1 * sign, base.y - 15);
  nvgLineTo(ctx, base.x - 1 * sign, base.y + 15);

  nvgFillColor(ctx, mTheme->mWindowPopup.toNvgColor());
  nvgFill(ctx);
  nvgEndFrame(ctx);
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

void Popup::drawBodyTemp(SDL_Renderer* renderer)
{
  int ds = mTheme->mWindowDropShadowSize;
  int cr = mTheme->mWindowCornerRadius;

  /* Draw a drop shadow */
  SDL_Color sh = mTheme->mDropShadow.toSdlColor();
  SDL_Rect shRect{ _pos.x - ds, _pos.y - ds, mSize.x + 2 * ds, mSize.y + 2 * ds };
  SDL_SetRenderDrawColor(renderer, sh.r, sh.g, sh.b, 64);
  SDL_RenderFillRect(renderer, &shRect);

  SDL_Color bg = mTheme->mWindowPopup.toSdlColor();
  SDL_Rect bgRect{ _pos.x, _pos.y, mSize.x, mSize.y };

  SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
  SDL_RenderFillRect(renderer, &bgRect);

  SDL_Color br = mTheme->mBorderDark.toSdlColor();
  SDL_SetRenderDrawColor(renderer, br.r, br.g, br.b, br.a);

  SDL_Rect brr{ _pos.x - 1, _pos.y - 1, width() + 2, height() + 2 };
  SDL_RenderDrawLine(renderer, brr.x, brr.y, brr.x + brr.w, brr.y);
  SDL_RenderDrawLine(renderer, brr.x + brr.w, brr.y, brr.x + brr.w, brr.y + brr.h);
  SDL_RenderDrawLine(renderer, brr.x, brr.y + brr.h, brr.x + brr.w, brr.y + brr.h);
  SDL_RenderDrawLine(renderer, brr.x, brr.y, brr.x, brr.y + brr.h);

  // Draw window anchor
  SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
  for (int i = 0; i < 15; i++)
  {
    SDL_RenderDrawLine(renderer, _pos.x - 15 + i, _pos.y + mAnchorHeight - i,
      _pos.x - 15 + i, _pos.y + mAnchorHeight + i);
  }
}

void Popup::drawBody(SDL_Renderer* renderer)
{
  int id = 1;

  auto atx = std::find_if(_txs.begin(), _txs.end(), [id](AsyncTexturePtr p) { return p->id == id; });

  if (atx != _txs.end())
  {  
    (*atx)->perform(renderer);
    
    if ((*atx)->tex.tex)
      SDL_RenderCopy(renderer, (*atx)->tex, getOverrideBodyPos());
    else
      drawBodyTemp(renderer);
  }
  else
  {
    AsyncTexturePtr newtx = std::make_shared<AsyncTexture>(id);
    newtx->load(this, _anchorDx);
    _txs.push_back(newtx);
  }
}

Vector2i Popup::getOverrideBodyPos()
{
  Vector2i ap = absolutePosition();
  int ds = mTheme->mWindowDropShadowSize;
  return ap - Vector2i(_anchorDx + ds, ds);
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
