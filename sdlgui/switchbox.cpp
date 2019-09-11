/*
    src/checkbox.cpp -- Two-state check box widget

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/switchbox.h>
#include <sdlgui/theme.h>
#include <thread>

#include "nanovg.h"
#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "nanovg_rt.h"

NAMESPACE_BEGIN(sdlgui)

struct SwitchBox::AsyncTexture
{
  int id;
  Texture tex;
  NVGcontext* ctx = nullptr;

  AsyncTexture (int _id) : id(_id) {}

  void load_body(SwitchBox* ptr, bool enabled)
  {
    SwitchBox* sb = ptr;
    AsyncTexture* self = this;
    std::thread tgr([=]() {
      Theme* theme = sb->theme();

      int ww = sb->width();
      int hh = sb->height();
      NVGcontext *ctx = nvgCreateRT(NVG_DEBUG, ww, hh, 0);

      float pxRatio = 1.0f;
      nvgBeginFrame(ctx, ww, hh, pxRatio);

      Vector2f center = sb->size().cast<float>() * 0.5f;
      float kr, startX, startY, widthX, heightY;
      if (sb->mAlign == Alignment::Horizontal)
      {
        kr = hh * 0.4f;
        startX = hh * 0.1f;
        heightY = hh * 0.8;

        startY = ( (hh - heightY) / 2) + 1;
        widthX = ( hh * 1.5);
      }
      else
      {
        kr = hh * 0.2f;
        startX = hh * 0.05f + 1;
        heightY = hh * 0.8;

        startY = ((hh - heightY) / 2);
        widthX = (hh * 0.4f);
      }

      NVGpaint bg = nvgBoxGradient(ctx, startX, startY, widthX, heightY, 3, 3,
        Color(0, enabled ? 32 : 10).toNvgColor(),
        Color(0, enabled ? 128 : 210).toNvgColor());

      nvgBeginPath(ctx);
      nvgRoundedRect(ctx, startX, startY, widthX, heightY, kr);
      nvgFillPaint(ctx, bg);

      nvgBeginPath(ctx);
      nvgStrokeWidth(ctx, 1.0f);
      nvgRoundedRect(ctx, startX + 0.5f, startY + 0.5f, widthX - 1, heightY - 1, kr);
      nvgStrokeColor(ctx, theme->mBorderLight.toNvgColor());
      nvgStroke(ctx);
      nvgFill(ctx);

      nvgBeginPath(ctx);
      nvgRoundedRect(ctx, startX + 0.5f, startY + 0.5f, widthX - 1, heightY - 2, kr);
      nvgStrokeColor(ctx, theme->mBorderDark.toNvgColor());
      nvgStroke(ctx);

      nvgEndFrame(ctx);

      self->tex.rrect = { 0, 0, ww, hh };
      self->ctx = ctx;
    });

    tgr.detach();
  }

  void load_knob(SwitchBox* ptr, bool enabled)
  {
    SwitchBox* sb = ptr;
    AsyncTexture* self = this;
    std::thread tgr([=]() {
      Theme* theme = sb->theme();

      int ww = std::min(sb->width(), sb->height());
      int hh = ww;

      Vector2f center(ww/2, hh/2);
      float kr = hh * 0.4f; 

      NVGcontext *ctx = nvgCreateRT(NVG_DEBUG, ww, ww, 0);

      float pxRatio = 1.0f;
      nvgBeginFrame(ctx, ww, ww, pxRatio);

      NVGpaint knob = nvgLinearGradient(ctx, 0, center.y - kr, 0, center.y + kr,
        theme->mBorderLight.toNvgColor(), theme->mBorderMedium.toNvgColor());
      NVGpaint knobReverse = nvgLinearGradient(ctx, 0, center.y - kr, 0, center.y + kr,
        theme->mBorderMedium.toNvgColor(), theme->mBorderLight.toNvgColor());

      nvgBeginPath(ctx);
      nvgCircle(ctx, center.x, center.y, kr * 0.9);
      nvgStrokeColor(ctx, Color(0, 200).toNvgColor());
      nvgFillPaint(ctx, knob);
      nvgStroke(ctx);
      nvgFill(ctx);
      nvgBeginPath(ctx);
      nvgCircle(ctx, center.x, center.y, kr * 0.7);
      nvgFillColor(ctx, Color(120, enabled ? 255 : 100).toNvgColor());
      nvgStrokePaint(ctx, knobReverse);
      nvgStroke(ctx);
      nvgFill(ctx);

      nvgEndFrame(ctx);

      self->tex.rrect = { 0, 0, ww, ww };
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

SwitchBox::SwitchBox(Widget *parent, Alignment align, const std::string &caption,
                   const std::function<void(bool) > &callback)
    : CheckBox(parent, caption, callback), mAlign(align) 
{
}

Vector2i SwitchBox::preferredSize(SDL_Renderer *renderer) const 
{
    if (mFixedSize != Vector2i::Zero())
        return mFixedSize;

    int w, h;
    const_cast<SwitchBox*>(this)->theme()->getUtf8Bounds("sans", fontSize(), mCaption.c_str(), &w, &h);
    int knobW = 1.8f * fontSize();
    knobW = std::max<int>(knobW / 32, 1) * 32;

    if (mAlign == Alignment::Horizontal)
      return Vector2i(w + knobW, knobW);
    else
      return Vector2i(w + knobW, 2 * knobW);
}

void SwitchBox::drawBody(SDL_Renderer *renderer)
{
  int id = (0x100) + (mEnabled ? 1 : 0);
  auto atx = std::find_if(_txs.begin(), _txs.end(), [id](AsyncTexturePtr p) { return p->id == id; });

  if (atx != _txs.end())
  {
    Vector2i ap = absolutePosition();
    (*atx)->perform(renderer);
    SDL_RenderCopy(renderer, (*atx)->tex, ap);
  }
  else
  {
    AsyncTexturePtr newtx = std::make_shared<AsyncTexture>(id);
    newtx->load_body(this, mEnabled);
    _txs.push_back(newtx);
  }
}

void SwitchBox::drawKnob(SDL_Renderer *renderer)
{
  int id = (0x200) + (mEnabled ? 1 : 0);
  auto atx = std::find_if(_txs.begin(), _txs.end(), [id](AsyncTexturePtr p) { return p->id == id; });
 
  Vector2i ap = absolutePosition();
  Vector2f center = ap.As<float>() + mSize.As<float>() * 0.5f;
  Vector2i knobPos;
  float kr, startX, startY, widthX, heightY, hh;
  hh = height();
  if (mAlign == Alignment::Horizontal)
  {
    kr = (hh * 0.4f);
    startX = ap.x + hh * 0.1f;
    heightY = hh * 0.8;

    startY = (ap.y + (hh - heightY) / 2) + 1;
    widthX = (hh * 1.5);

    knobPos = Vector2i(startX + kr + path * (widthX - 2 * kr), center.y + 0.5f);
  }
  else
  {
    kr = (hh * 0.2f);
    startX = ap.x + hh * 0.05f + 1;
    heightY = hh * 0.8;

    startY = (ap.y + (hh - heightY) / 2);
    widthX = (hh * 0.4f);

    knobPos = Vector2i(startX + kr, startY + path * (heightY - 2 * kr) + kr);
  }

  if (atx != _txs.end())
  {
    (*atx)->perform(renderer);
    SDL_RenderCopy(renderer, (*atx)->tex, knobPos - Vector2i((*atx)->tex.w()/2, (*atx)->tex.h() / 2));
  }
  else
  {
    AsyncTexturePtr newtx = std::make_shared<AsyncTexture>(id);
    newtx->load_knob(this, mEnabled);
    _txs.push_back(newtx);
  }
}

void SwitchBox::draw(SDL_Renderer *renderer)
{
  if (mChecked)
  {
    if (path < 1.0f)
      path += 0.1f;
  }
  else
  {
    if (path > 0) path -= 0.1f;
    if (path < 0) path = 0;
  }

  drawBody(renderer);
  drawKnob(renderer);
/*
  nvgFontSize(ctx, fontSize());
  nvgFontFace(ctx, "sans");
  nvgFillColor(ctx, mEnabled ? mTheme->mTextColor : mTheme->mDisabledTextColor);
  nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
  nvgText(ctx, mPos.x() + 1.6f * fontSize(), mPos.y() + mSize.y() * 0.5f, mCaption.c_str(), nullptr);
*/
  Widget::draw(renderer);
}

NAMESPACE_END(sdlgui)
