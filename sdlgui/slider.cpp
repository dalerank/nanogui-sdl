/*
    sdlgui/slider.cpp -- Fractional slider widget with mouse control

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/slider.h>
#include <sdlgui/theme.h>
#include <sdlgui/entypo.h>
#include <array>
#include <thread>

#include "nanovg.h"
#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "nanovg_rt.h"

NAMESPACE_BEGIN(sdlgui)

struct Slider::AsyncTexture
{
  Texture tex;
  NVGcontext* ctx = nullptr;

  void load_body(Slider* ptr, bool enabled)
  {
    Slider* slider = ptr;
    AsyncTexture* self = this;
    std::thread tgr([=]() {
      Theme* mTheme = slider->theme();
      std::lock_guard<std::mutex> guard(mTheme->loadMutex);

      int ww = slider->width();
      int hh = slider->height();
      int rh = hh / 3;
      auto mRange = slider->range();
      auto mHighlightedRange = slider->highlightedRange();
      NVGcontext *ctx = nvgCreateRT(NVG_DEBUG, ww, hh, 0);

      float pxRatio = 1.0f;
      nvgBeginFrame(ctx, ww, hh, pxRatio);

      Vector2f center = slider->size().cast<float>() * 0.5f;
      int rectround = hh / 2;
      float kr = (int)(hh * 0.4f), kshadow = 3;

      float startX = kr + kshadow + 0;
      float widthX = ww - 2 * (kr + kshadow);

      NVGpaint bg = nvgBoxGradient(
        ctx, 0, center.y - rh/2 + 1, ww, rh, 3, 3,
        Color(0, enabled ? 32 : 10).toNvgColor(), Color(0, enabled ? 128 : 210).toNvgColor());

      nvgBeginPath(ctx);
      nvgRoundedRect(ctx, 0, center.y - rh/2 + 1, ww, rh, 2);
      nvgFillPaint(ctx, bg);
      nvgFill(ctx);

      if (mHighlightedRange.second != mHighlightedRange.first) 
      {
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, startX + mHighlightedRange.first * ww,
          center.y - kshadow + 1,
          widthX *  (mHighlightedRange.second - mHighlightedRange.first),
          kshadow * 2, 2);
        nvgFillColor(ctx, slider->highlightColor().toNvgColor());
        nvgFill(ctx);
      }

      nvgEndFrame(ctx);
      self->tex.rrect = { 0, 0, ww, hh };
      self->ctx = ctx;
    });

    tgr.detach();
  }

  void load_knob(Slider* ptr, bool enabled)
  {
    Slider* slider = ptr;
    AsyncTexture* self = this;

    std::thread tgr([=]() {
      Theme* mTheme = slider->theme();
      std::lock_guard<std::mutex> guard(mTheme->loadMutex);

      int hh = slider->height();
      int ww = hh;

      auto mRange = slider->range();
      float mValue = slider->value();

      NVGcontext *ctx = nvgCreateRT(NVG_DEBUG, ww, hh, 0);

      float pxRatio = 1.0f;
      nvgBeginFrame(ctx, ww, hh, pxRatio);

      Vector2f center(hh / 2, hh / 2);
      float kr = (int)(hh * 0.4f), kshadow = 3;

      float startX = kr + kshadow + 0;
      float widthX = ww - 2 * (kr + kshadow);

      Vector2f knobPos(startX, center.y + 0.5f);

      NVGpaint knobShadow =
        nvgRadialGradient(ctx, knobPos.x, knobPos.y, kr - kshadow,
          kr + kshadow, Color(0, 64).toNvgColor(), mTheme->mTransparent.toNvgColor());

      nvgBeginPath(ctx);
      nvgRect(ctx, knobPos.x - kr - 5, knobPos.y - kr - 5, kr * 2 + 10, kr * 2 + 10 + kshadow);
      nvgCircle(ctx, knobPos.x, knobPos.y, kr);
      //nvgPathWinding(ctx, NVG_HOLE);
      nvgFillPaint(ctx, knobShadow);
      nvgFill(ctx);

      NVGpaint knob = nvgLinearGradient(ctx, 0, center.y - kr, 0, center.y + kr,
        mTheme->mBorderLight.toNvgColor(), mTheme->mBorderMedium.toNvgColor());
      NVGpaint knobReverse = nvgLinearGradient(ctx, 0, center.y - kr, 0, center.y + kr,
        mTheme->mBorderMedium.toNvgColor(),
        mTheme->mBorderLight.toNvgColor());

      nvgBeginPath(ctx);
      nvgCircle(ctx, knobPos.x, knobPos.y, kr);
      nvgStrokeColor(ctx, mTheme->mBorderDark.toNvgColor());
      nvgFillPaint(ctx, knob);
      nvgStroke(ctx);
      nvgFill(ctx);
      nvgBeginPath(ctx);
      nvgCircle(ctx, knobPos.x, knobPos.y, kr / 2);
      nvgFillColor(ctx, Color(150, enabled ? 255 : 100).toNvgColor());
      nvgStrokePaint(ctx, knobReverse);
      nvgStroke(ctx);
      nvgFill(ctx);

      nvgEndFrame(ctx);
      self->tex.rrect = { 0, 0, ww, hh };
      self->ctx = ctx;
    });

    tgr.detach();
  }

  void perform(SDL_Renderer* renderer)
  {
    if (!ctx)
      return;

    unsigned char *rgba = nvgReadPixelsRT(ctx);

    if (tex.tex)
    {
      int w, h;
      SDL_QueryTexture(tex.tex, nullptr, nullptr, &w, &h);
      if (w != tex.w() || h != tex.h())
        SDL_DestroyTexture(tex.tex);
    }

    if (!tex.tex)
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


Slider::Slider(Widget *parent, float value)
    : Widget(parent), mValue(value), mRange(0.f, 1.f), mHighlightedRange(0.f, 0.f)
{
    mHighlightColor = Color(255, 80, 80, 70);
}

Vector2i Slider::preferredSize(SDL_Renderer *) const
{
    return Vector2i(70, 20);
}

bool Slider::mouseDragEvent(const Vector2i &p, const Vector2i & /* rel */,
                            int /* button */, int /* modifiers */) 
{
    if (!mEnabled)
        return false;
    mValue = std::min(std::max((p.x - _pos.x) / (float) mSize.x, (float) 0.0f), (float) 1.0f);
    if (mCallback) mCallback(mValue);
    if (mObjCallback) mObjCallback(this, mValue);
    return true;
}

bool Slider::mouseButtonEvent(const Vector2i &p, int /* button */, bool down, int /* modifiers */)
{
    if (!mEnabled)
        return false;
    mValue = std::min(std::max((p.x - _pos.x) / (float) mSize.x, (float) 0.0f), (float) 1.0f);
    if (mCallback) mCallback(mValue);
    if (mObjCallback) mObjCallback(this, mValue);
    if (mFinalCallback && !down) mFinalCallback(mValue);
    return true;
}

void Slider::drawBody(SDL_Renderer* renderer)
{
  if (!_body)
    _body = std::make_shared<AsyncTexture>();

  if (mEnabled != _lastEnabledState)
    _body->load_body(this, mEnabled);

  if (_body)
  {
    Vector2i ap = absolutePosition();
    _body->perform(renderer);
    SDL_RenderCopy(renderer, _body->tex, ap);
  }
}

void Slider::drawKnob(SDL_Renderer* renderer)
{
  if (!_knob)
    _knob = std::make_shared<AsyncTexture>();

  if (mEnabled != _lastEnabledState)
    _knob->load_knob(this, mEnabled);

  if (_body)
  {
    Vector2i ap = absolutePosition();
    Vector2i knobPos(ap.x + mValue * mSize.x, ap.y + height() * 0.5f);

    _knob->perform(renderer);
    SDL_RenderCopy(renderer, _knob->tex, knobPos - Vector2i(_knob->tex.w()/2, _knob->tex.h()/2));
  }
}

void Slider::draw(SDL_Renderer* renderer) 
{
  drawBody(renderer);
  drawKnob(renderer);

  _lastEnabledState = mEnabled;

  /*if (mHighlightedRange.second != mHighlightedRange.first) 
  {
    SDL_Color hl = mHighlightColor.toSdlColor();
    SDL_FRect hlRect{ ap.x + mHighlightedRange.first * width(), center.y - 3 + 1, 
                      width() * (mHighlightedRange.second - mHighlightedRange.first), 6 };

    SDL_SetRenderDrawColor(renderer, hl.r, hl.g, hl.b, hl.a);
    SDL_RenderFillRectF(renderer, &hlRect);
  }

  SDL_RenderCopy(renderer, _outerKnobTex, (knobPos + Vector2f( - _outerKnobTex.w() / 2.f, - _outerKnobTex.h() / 2.f)).As<int>());
  SDL_RenderCopy(renderer, _innerKnobTex, (knobPos + Vector2f( - _innerKnobTex.w() / 2.f, - _innerKnobTex.h() / 2.f)).As<int>());
  */
}

NAMESPACE_END(sdlgui)
