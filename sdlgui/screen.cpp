/*
    sdlgui/screen.cpp -- Top-level widget and interface between sdlgui and SDL

    A significant redesign of this code was contributed by Christian Schueller.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/screen.h>
#include <sdlgui/theme.h>
#include <sdlgui/window.h>
#include <sdlgui/popup.h>
#include <iostream>
#include <map>

#if defined(_WIN32)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

NAMESPACE_BEGIN(sdlgui)

std::map<SDL_Window *, Screen *> __sdlgui_screens;

Screen::Screen( SDL_Window* window, const Vector2i &size, const std::string &caption,
               bool resizable, bool fullscreen)
    : Widget(nullptr), _window(nullptr), mSDL_Renderer(nullptr), mCaption(caption)
{
    SDL_SetWindowTitle( window, caption.c_str() );
    initialize( window );
}

bool Screen::onEvent(SDL_Event& event)
{
    auto it = __sdlgui_screens.find(_window);
    if (it == __sdlgui_screens.end())
       return false;

    switch( event.type )
    {
    case SDL_MOUSEWHEEL:
    {
        if (!mProcessEvents)
            return false;
        return scrollCallbackEvent(event.wheel.x, event.wheel.y);
    }
    break;

    case SDL_MOUSEMOTION:
    {
      if (!mProcessEvents)
         return false;
      return cursorPosCallbackEvent(event.motion.x, event.motion.y);
    }
    break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    {
      if (!mProcessEvents)
        return false;

      SDL_Keymod mods = SDL_GetModState();
      return mouseButtonCallbackEvent(event.button.button, event.button.type, mods);
    }
    break;

    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
      if (!mProcessEvents)
        return false;

      SDL_Keymod mods = SDL_GetModState();
      return keyCallbackEvent(event.key.keysym.sym, event.key.keysym.scancode, event.key.state, mods);
    }
    break;

    case SDL_TEXTINPUT:
    {
      if (!mProcessEvents)
        return false;
      return charCallbackEvent(event.text.text[0]);
    }
    break;
    }
    return false;
}

void Screen::initialize(SDL_Window* window)
{
    _window = window;    
    SDL_GetWindowSize( window, &mSize[0], &mSize[1]);
    SDL_GetWindowSize( window, &mFBSize[0], &mFBSize[1]);
    mSDL_Renderer = SDL_GetRenderer(window);
    
    if (mSDL_Renderer == nullptr)
        throw std::runtime_error("Could not initialize NanoVG!");

    mVisible = true;
    mTheme = new Theme(mSDL_Renderer);
    mMousePos = { 0, 0 };
    mMouseState = mModifiers = 0;
    mDragActive = false;
    mLastInteraction = SDL_GetTicks();
    mProcessEvents = true;
    mBackground = Color(0.3f, 0.3f, 0.32f, 1.0f);
    __sdlgui_screens[_window] = this;
}

Screen::~Screen()
{
    __sdlgui_screens.erase(_window);
}

void Screen::setVisible(bool visible)
{
    if (mVisible != visible)
     {
        mVisible = visible;

        if (visible)
            SDL_ShowWindow(_window);
        else
            SDL_HideWindow(_window);
    }
}

void Screen::setCaption(const std::string &caption)
{
    if (caption != mCaption)
    {
        SDL_SetWindowTitle( _window, caption.c_str());
        mCaption = caption;
    }
}

void Screen::setSize(const Vector2i &size)
{
    Widget::setSize(size);
    SDL_SetWindowSize(_window, size.x, size.y);
}

void Screen::drawAll()
{
  drawContents();
  drawWidgets();
}

void Screen::drawWidgets()
{
    if (!mVisible)
        return;

    /* Calculate pixel ratio for hi-dpi devices. */
    mPixelRatio = (float) mFBSize[0] / (float) mSize[0];
    
    SDL_Renderer* renderer = SDL_GetRenderer(_window);
    draw(renderer);

    double elapsed = SDL_GetTicks() - mLastInteraction;
    if (elapsed > 0.5f) 
    {
        /* Draw tooltips */
        const Widget *widget = findWidget(mMousePos);
        if (widget && !widget->tooltip().empty()) 
        {
            int tooltipWidth = 150;

            if (_lastTooltip != widget->tooltip())
            {
              _lastTooltip = widget->tooltip();
              mTheme->getTexAndRectUtf8(renderer, _tooltipTex, 0, 0, _lastTooltip.c_str(), "sans", 15, Color(1.f, 1.f));
            }

            if (_tooltipTex.tex)
            {
              Vector2i pos = widget->absolutePosition() + Vector2i(widget->width() / 2, widget->height() + 10);

              float alpha = (std::min(1.0, 2 * (elapsed - 0.5f)) * 0.8) * 255;
              SDL_SetTextureAlphaMod(_tooltipTex.tex, alpha);

              SDL_Rect bgrect{ pos.x - 2, pos.y - 2 - _tooltipTex.h(), _tooltipTex.w() + 4, _tooltipTex.h() + 4 };

              SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
              SDL_RenderFillRect(renderer, &bgrect);
              SDL_RenderCopy(renderer, _tooltipTex, Vector2i(pos.x, pos.y - _tooltipTex.h()));
              SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
              SDL_RenderDrawLine(renderer, bgrect.x, bgrect.y, bgrect.x + bgrect.w, bgrect.y);
              SDL_RenderDrawLine(renderer, bgrect.x + bgrect.w, bgrect.y, bgrect.x + bgrect.w, bgrect.y + bgrect.h);
              SDL_RenderDrawLine(renderer, bgrect.x, bgrect.y + bgrect.h, bgrect.x + bgrect.w, bgrect.y + bgrect.h);
              SDL_RenderDrawLine(renderer, bgrect.x, bgrect.y, bgrect.x, bgrect.y + bgrect.h);
            }
        }
    }
}

bool Screen::keyboardEvent(int key, int scancode, int action, int modifiers) 
{
    if (mFocusPath.size() > 0) 
    {
        for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
            if ((*it)->focused() && (*it)->keyboardEvent(key, scancode, action, modifiers))
                return true;
    }

    return false;
}

bool Screen::keyboardCharacterEvent(unsigned int codepoint) {
    if (mFocusPath.size() > 0) {
        for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
            if ((*it)->focused() && (*it)->keyboardCharacterEvent(codepoint))
                return true;
    }
    return false;
}

bool Screen::cursorPosCallbackEvent(double x, double y) 
{
  Vector2i p((int) x, (int) y);
    bool ret = false;
    mLastInteraction = SDL_GetTicks();
    try 
    {
        p -= Vector2i(1, 2);

        if (!mDragActive) 
        {
            Widget *widget = findWidget(p);
            /*if (widget != nullptr && widget->cursor() != mCursor) {
                mCursor = widget->cursor();
                glfwSetCursor(mGLFWWindow, mCursors[(int) mCursor]);
            }*/
        } 
        else 
        {
            ret = mDragWidget->mouseDragEvent(
                p - mDragWidget->parent()->absolutePosition(), p - mMousePos,
                mMouseState, mModifiers);
        }

        if (!ret)
            ret = mouseMotionEvent(p, p - mMousePos, mMouseState, mModifiers);

        mMousePos = p;

        return ret;
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        abort();
    }

    return false;
}

bool Screen::mouseButtonCallbackEvent(int button, int action, int modifiers) {
    mModifiers = modifiers;
    mLastInteraction = SDL_GetTicks();
    try {
        if (mFocusPath.size() > 1) {
            const Window *window =
                dynamic_cast<Window *>(mFocusPath[mFocusPath.size() - 2]);
            if (window && window->modal()) {
                if (!window->contains(mMousePos))
                    return false;
            }
        }

        if (action == SDL_MOUSEBUTTONDOWN)
            mMouseState |= 1 << button;
        else
            mMouseState &= ~(1 << button);

        auto dropWidget = findWidget(mMousePos);
        if (mDragActive && action == SDL_MOUSEBUTTONUP &&
            dropWidget != mDragWidget)
            mDragWidget->mouseButtonEvent(
                mMousePos - mDragWidget->parent()->absolutePosition(), button,
                false, mModifiers);

        /*if (dropWidget != nullptr && dropWidget->cursor() != mCursor) {
            mCursor = dropWidget->cursor();
            glfwSetCursor(mGLFWWindow, mCursors[(int) mCursor]);
        }*/

        if (action == SDL_MOUSEBUTTONDOWN && button == SDL_BUTTON_LEFT) {
            mDragWidget = findWidget(mMousePos);
            if (mDragWidget == this)
                mDragWidget = nullptr;
            mDragActive = mDragWidget != nullptr;
            if (!mDragActive)
                updateFocus(nullptr);
        } else {
            mDragActive = false;
            mDragWidget = nullptr;
        }

        return mouseButtonEvent(mMousePos, button, action == SDL_MOUSEBUTTONDOWN,
                                mModifiers);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        abort();
    }

    return false;
}

bool Screen::keyCallbackEvent(int key, int scancode, int action, int mods)
{
    mLastInteraction = SDL_GetTicks();
    try {
        return keyboardEvent(key, scancode, action, mods);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        abort();
    }
}

bool Screen::charCallbackEvent(unsigned int codepoint)
 {
    mLastInteraction = SDL_GetTicks();
    try {
        return keyboardCharacterEvent(codepoint);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        abort();
    }
}

bool Screen::dropCallbackEvent(int count, const char **filenames) {
    std::vector<std::string> arg(count);
    for (int i = 0; i < count; ++i)
        arg[i] = filenames[i];
    return dropEvent(arg);
}

bool Screen::scrollCallbackEvent(double x, double y)
{
    mLastInteraction = SDL_GetTicks();
    try {
        if (mFocusPath.size() > 1) {
            const Window *window =
                dynamic_cast<Window *>(mFocusPath[mFocusPath.size() - 2]);
            if (window && window->modal()) {
                if (!window->contains(mMousePos))
                    return false;
            }
        }
        return scrollEvent(mMousePos, Vector2f(x, y));
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        abort();
    }

    return false;
}

bool Screen::resizeCallbackEvent(int, int)
{
  Vector2i fbSize, size;
    //glfwGetFramebufferSize(mGLFWWindow, &fbSize[0], &fbSize[1]);
    SDL_GetWindowSize(_window, &size[0], &size[1]);

    if (mFBSize == Vector2i(0, 0) || size == Vector2i(0, 0))
        return false;

    mFBSize = fbSize;
    mSize = size;
    mLastInteraction = SDL_GetTicks();

    try 
    {
        return resizeEvent(mSize);
    } 
    catch (const std::exception &e) 
    {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        abort();
    }
}

void Screen::updateFocus(Widget *widget) {
    for (auto w: mFocusPath) {
        if (!w->focused())
            continue;
        w->focusEvent(false);
    }
    mFocusPath.clear();
    Widget *window = nullptr;
    while (widget) {
        mFocusPath.push_back(widget);
        if (dynamic_cast<Window *>(widget))
            window = widget;
        widget = widget->parent();
    }
    for (auto it = mFocusPath.rbegin(); it != mFocusPath.rend(); ++it)
        (*it)->focusEvent(true);

    if (window)
        moveWindowToFront((Window *) window);
}

void Screen::disposeWindow(Window *window) {
    if (std::find(mFocusPath.begin(), mFocusPath.end(), window) != mFocusPath.end())
        mFocusPath.clear();
    if (mDragWidget == window)
        mDragWidget = nullptr;
    removeChild(window);
}

void Screen::centerWindow(Window *window) 
{
  if (window->size() == Vector2i{0, 0}) 
  {
     window->setSize(window->preferredSize(mSDL_Renderer));
     window->performLayout(mSDL_Renderer);
  }
  window->setPosition((mSize - window->size()) / 2);
}

void Screen::moveWindowToFront(Window *window) {
    mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), window), mChildren.end());
    mChildren.push_back(window);
    /* Brute force topological sort (no problem for a few windows..) */
    bool changed = false;
    do {
        size_t baseIndex = 0;
        for (size_t index = 0; index < mChildren.size(); ++index)
            if (mChildren[index] == window)
                baseIndex = index;
        changed = false;
        for (size_t index = 0; index < mChildren.size(); ++index) {
            Popup *pw = dynamic_cast<Popup *>(mChildren[index]);
            if (pw && pw->parentWindow() == window && index < baseIndex) {
                moveWindowToFront(pw);
                changed = true;
                break;
            }
        }
    } while (changed);
}

void Screen::performLayout(SDL_Renderer* ctx)
{
  Widget::performLayout(ctx);
}

void Screen::performLayout()
{
  Widget::performLayout(mSDL_Renderer);
}

NAMESPACE_END(sdlgui)
