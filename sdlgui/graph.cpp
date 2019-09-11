/*
    sdlgui/graph.cpp -- Simple graph widget for showing a function plot

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/graph.h>
#include <sdlgui/theme.h>
#include <thread>

#include "nanovg.h"
#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "nanovg_rt.h"

NAMESPACE_BEGIN(sdlgui)

struct Graph::AsyncTexture
{
  Texture tex;
  NVGcontext* ctx = nullptr;

  void load(Graph* ptr)
  {
    Graph* graph = ptr;
    AsyncTexture* self = this;

    std::thread tgr([=]() {
      Theme* theme = graph->theme();

      int ww = graph->width();
      int hh = graph->height();
      NVGcontext *ctx = nvgCreateRT(NVG_DEBUG, ww, hh, 0);

      float pxRatio = 1.0f;
      nvgBeginFrame(ctx, ww, hh, pxRatio);

      nvgBeginPath(ctx);
      nvgRect(ctx, 0, 0, ww, hh);
      nvgFillColor(ctx, graph->backgroundColor().toNvgColor());
      nvgFill(ctx);

      if (graph->values().size() < 2)
        return;

      nvgBeginPath(ctx);
      nvgMoveTo(ctx, 0, 0 + hh);
      auto& values = graph->values();
      for (size_t i = 0; i < (size_t)values.size(); i++) 
      {
        float value = values[i];
        float vx = 0 + i * ww / (float)(values.size() - 1);
        float vy = 0 + (1 - value) * hh;
        nvgLineTo(ctx, vx, vy);
      }

      nvgLineTo(ctx, 0 + ww, 0 + hh);
      nvgStrokeColor(ctx, Color(100, 255).toNvgColor());
      nvgStroke(ctx);
      nvgFillColor(ctx, graph->foregroundColor().toNvgColor());
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

Graph::Graph(Widget *parent, const std::string &caption)
    : Widget(parent), mCaption(caption) 
{
    mBackgroundColor = Color(20, 128);
    mForegroundColor = Color(255, 192, 0, 128);
    mTextColor = Color(240, 192);
    _captionTex.dirty = true;
    _headerTex.dirty = true;
}

Vector2i Graph::preferredSize(SDL_Renderer *) const
{
    return Vector2i(180, 45);
}

void Graph::draw(SDL_Renderer *renderer) 
{
    Widget::draw(renderer);

    Vector2i ap = absolutePosition();
    
    if (_atx)
    {
      Vector2i ap = absolutePosition();
      _atx->perform(renderer);
      SDL_RenderCopy(renderer, _atx->tex, ap);
    }
    else
    {
      _atx = std::make_shared<AsyncTexture>();
      _atx->load(this);
    }

    if (_captionTex.dirty)
      mTheme->getTexAndRectUtf8(renderer, _captionTex, 0, 0, mCaption.c_str(), "sans", 14, mTextColor);

    if (_headerTex.dirty)
      mTheme->getTexAndRectUtf8(renderer, _headerTex, 0, 0, mHeader.c_str(), "sans", 18, mTextColor);

    if (_footerTex.dirty)
      mTheme->getTexAndRectUtf8(renderer, _footerTex, 0, 0, mFooter.c_str(), "sans", 15, mTextColor);

    SDL_RenderCopy(renderer, _captionTex, ap + Vector2i(3,1) );
    SDL_RenderCopy(renderer, _headerTex, ap + Vector2i(mSize.x - 3 - _headerTex.w(), 1));
    SDL_RenderCopy(renderer, _footerTex, ap + Vector2i(mSize.x - 3 - _footerTex.w(), mSize.y - 1 - _footerTex.h()));
 }

NAMESPACE_END(sdlgui)
