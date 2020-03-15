/*
    sdlgui/window.cpp -- Top-level window widget

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/window.h>
#include <sdlgui/theme.h>
#include <sdlgui/screen.h>
#include <sdlgui/layout.h>
#if defined(_WIN32)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <thread>

#include "nanovg.h"
#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "nanovg_rt.h"

NAMESPACE_BEGIN(sdlgui)

struct Window::AsyncTexture
{
  int id;
  Texture tex;
  NVGcontext* ctx = nullptr;

  bool ready() const { return tex.tex != nullptr; }

  AsyncTexture(int _id) : id(_id) {};

  void load(Window* ptr, int dx, int dy, bool mouseFocus)
  {
    Window* wnd = ptr;
    AsyncTexture* self = this;
    std::thread tgr([=]() {
      Theme* mTheme = wnd->theme();

      int ww = wnd->width();
      int hh = wnd->height();
      int ds = mTheme->mWindowDropShadowSize;

      Vector2i mPos(dx + ds, dy + ds);

      int realw = ww + 2 * ds + dx; //with + 2*shadow + offset
      int realh = hh + 2 * ds + dy;
      NVGcontext *ctx = nvgCreateRT(NVG_DEBUG, realw, realh, 0);

      float pxRatio = 1.0f;
      nvgBeginFrame(ctx, realw, realh, pxRatio);

      int cr = mTheme->mWindowCornerRadius;
      int headerH = mTheme->mWindowHeaderHeight;

      /* Draw window */
      nvgSave(ctx);
      nvgBeginPath(ctx);
      nvgRoundedRect(ctx, mPos.x, mPos.y, ww, hh, cr);

      nvgFillColor(ctx, (mouseFocus ? mTheme->mWindowFillFocused
                                    : mTheme->mWindowFillUnfocused).toNvgColor());
      nvgFill(ctx);


      /* Draw a drop shadow */
      NVGpaint shadowPaint = nvgBoxGradient(
        ctx, mPos.x, mPos.y, ww, hh, cr * 2, ds * 2,
        mTheme->mDropShadow.toNvgColor(), 
        mTheme->mTransparent.toNvgColor());

      nvgSave(ctx);
      nvgResetScissor(ctx);
      nvgBeginPath(ctx);
      nvgRect(ctx, mPos.x - ds, mPos.y - ds, ww + 2 * ds, hh + 2 * ds);
      nvgRoundedRect(ctx, mPos.x, mPos.y, ww, hh, cr);
      nvgPathWinding(ctx, NVG_HOLE);
      nvgFillPaint(ctx, shadowPaint);
      nvgFill(ctx);
      nvgRestore(ctx);

      /* Draw header */
      NVGpaint headerPaint = nvgLinearGradient(
        ctx, mPos.x, mPos.y, mPos.x,
        mPos.y + headerH,
        mTheme->mWindowHeaderGradientTop.toNvgColor(),
        mTheme->mWindowHeaderGradientBot.toNvgColor());

      nvgBeginPath(ctx);
      nvgRoundedRect(ctx, mPos.x, mPos.y, ww, headerH, cr);

      nvgFillPaint(ctx, headerPaint);
      nvgFill(ctx);

      nvgBeginPath(ctx);
      nvgRoundedRect(ctx, mPos.x, mPos.y, ww, headerH, cr);
      nvgStrokeColor(ctx, mTheme->mWindowHeaderSepTop.toNvgColor());

      nvgSave(ctx);
      nvgIntersectScissor(ctx, mPos.x, mPos.y, ww, 0.5f);
      nvgStroke(ctx);
      nvgRestore(ctx);

      nvgBeginPath(ctx);
      nvgMoveTo(ctx, mPos.x + 0.5f, mPos.y + headerH - 1.5f);
      nvgLineTo(ctx, mPos.x + ww - 0.5f, mPos.y + headerH - 1.5);
      nvgStrokeColor(ctx, mTheme->mWindowHeaderSepBot.toNvgColor());
      nvgStroke(ctx);

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

Window::Window(Widget *parent, const std::string &title)
    : Widget(parent), mTitle(title), mButtonPanel(nullptr), mModal(false), mDrag(false) 
{
  _titleTex.dirty = true;
}

Vector2i Window::preferredSize(SDL_Renderer *ctx) const
{
    if (mButtonPanel)
        mButtonPanel->setVisible(false);
    Vector2i result = Widget::preferredSize(ctx);
    if (mButtonPanel)
        mButtonPanel->setVisible(true);

    int w, h;
    const_cast<Window*>(this)->mTheme->getTextBounds("sans-bold", 18.0, mTitle.c_str(), &w, &h);

    return result.cmax(Vector2i(w + 20, h));
}

Widget *Window::buttonPanel() 
{
    if (!mButtonPanel) 
    {
        mButtonPanel = new Widget(this);
        mButtonPanel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 4));
    }
    return mButtonPanel;
}

void Window::performLayout(SDL_Renderer *ctx) 
{
    if (!mButtonPanel) 
    {
        Widget::performLayout(ctx);
    } 
    else 
    {
        mButtonPanel->setVisible(false);
        Widget::performLayout(ctx);
        for (auto w : mButtonPanel->children()) 
        {
          w->setFixedSize({ 22, 22 });
          w->setFontSize(15);
        }
        mButtonPanel->setVisible(true);
        mButtonPanel->setSize({ width(), 22 });
        mButtonPanel->setPosition({ width() - (mButtonPanel->preferredSize(ctx).x + 5), 3 });
        mButtonPanel->performLayout(ctx);
    }
}

bool Window::focusEvent(bool focused)
{
  _titleTex.dirty = focused != mFocused;
  return Widget::focusEvent(focused);
}

void Window::drawBodyTemp(SDL_Renderer* renderer)
{
  int ds = mTheme->mWindowDropShadowSize;
  int cr = mTheme->mWindowCornerRadius;
  int hh = mTheme->mWindowHeaderHeight;

  Vector2i ap = absolutePosition();
  SDL_Rect rect{ ap.x, ap.y, mSize.x, mSize.y };

  /* Draw a drop shadow */
  SDL_Rect shadowRect{ ap.x - ds, ap.y - ds, mSize.x + 2 * ds, mSize.y + 2 * ds };
  SDL_Color shadowColor = mTheme->mDropShadow.toSdlColor();

  SDL_SetRenderDrawColor(renderer, shadowColor.r, shadowColor.g, shadowColor.b, 32);
  SDL_RenderFillRect(renderer, &shadowRect);

  /* Draw window */
  SDL_Color color = (mMouseFocus ? mTheme->mWindowFillFocused : mTheme->mWindowFillUnfocused).toSdlColor();
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(renderer, &rect);

  SDL_Rect wndBdRect{ ap.x - 2, ap.y - 2, width() + 4, height() + 4 };
  SDL_Color bd = mTheme->mBorderDark.toSdlColor();
  SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
  SDL_RenderDrawRect(renderer, &wndBdRect);

  SDL_Color headerColor = mTheme->mWindowHeaderGradientTop.toSdlColor();
  SDL_Rect headerRect{ ap.x, ap.y, mSize.x, hh };

  SDL_SetRenderDrawColor(renderer, headerColor.r, headerColor.g, headerColor.b, headerColor.a);
  SDL_RenderFillRect(renderer, &headerRect);

  SDL_Color headerBotColor = mTheme->mWindowHeaderSepBot.toSdlColor();
  SDL_SetRenderDrawColor(renderer, headerBotColor.r, headerBotColor.g, headerBotColor.b, headerBotColor.a);
  SDL_RenderDrawLine(renderer, ap.x + 0.5f, ap.y + hh - 1.5f, ap.x + width() - 0.5f, ap.y + hh - 1.5);
}

void Window::drawBody(SDL_Renderer* renderer)
{
  int id = (mMouseFocus ? 0x1 : 0);

  auto atx = std::find_if(_txs.begin(), _txs.end(), [id](AsyncTexturePtr p) { return p->id == id; });

  if (atx != _txs.end())
  {
    Vector2i ap = absolutePosition();
    (*atx)->perform(renderer);
    if ((*atx)->ready())
    {
      int ds = mTheme->mWindowDropShadowSize;
      SDL_RenderCopy(renderer, (*atx)->tex, ap - Vector2i(ds, ds));
    }
    else
    {
      drawBodyTemp(renderer);
    }
  }
  else
  {
    AsyncTexturePtr newtx = std::make_shared<AsyncTexture>(id);
    newtx->load(this, 0, 0, mMouseFocus);
    _txs.push_back(newtx);

    drawBodyTemp(renderer);
  }
}

void Window::draw(SDL_Renderer* renderer)
{
  drawBody(renderer);

  if (_titleTex.dirty)
  {
    Color titleTextColor = (mFocused ? mTheme->mWindowTitleFocused : mTheme->mWindowTitleUnfocused);
    mTheme->getTexAndRectUtf8(renderer, _titleTex, 0, 0, mTitle.c_str(), "sans-bold", 18, titleTextColor);
  }

  if (!mTitle.empty() && _titleTex.tex) 
  {
    int headerH = mTheme->mWindowHeaderHeight;
    SDL_RenderCopy(renderer, _titleTex, _pos + Vector2i( (mSize.x - _titleTex.w())/2, (headerH - _titleTex.h()) / 2));
  }

  Widget::draw(renderer);
}

void Window::dispose() 
{
    Widget *widget = this;
    while (widget->parent())
        widget = widget->parent();
    ((Screen *) widget)->disposeWindow(this);
}

void Window::center() 
{
    Widget *widget = this;
    while (widget->parent())
        widget = widget->parent();
    ((Screen *) widget)->centerWindow(this);
}

bool Window::mouseDragEvent(const Vector2i &, const Vector2i &rel,
                            int button, int /* modifiers */) 
{
    if (mDrag && (button & (1 << SDL_BUTTON_LEFT)) != 0)
    {
        _pos += rel;
        _pos = _pos.cmax({ 0, 0 });
        _pos = _pos.cmin(parent()->size() - mSize);
        return true;
    }
    return false;
}

bool Window::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers)
{
    if (Widget::mouseButtonEvent(p, button, down, modifiers))
        return true;
    if (button == SDL_BUTTON_LEFT) {
        mDrag = down && (p.y - _pos.y) < mTheme->mWindowHeaderHeight;
        return true;
    }
    return false;
}

bool Window::scrollEvent(const Vector2i &p, const Vector2f &rel)
{
    Widget::scrollEvent(p, rel);
    return true;
}

void Window::refreshRelativePlacement() 
{
    /* Overridden in \ref Popup */
}

NAMESPACE_END(sdlgui)
