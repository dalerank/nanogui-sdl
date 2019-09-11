/*
    sdl_gui/window.h -- Top-level window widget

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <sdlgui/widget.h>
#include <memory>

NAMESPACE_BEGIN(sdlgui)

/**
 * \class Window window.h sdl_gui/window.h
 *
 * \brief Top-level window widget.
 */
class  Window : public Widget 
{
    friend class Popup;
public:
    Window(Widget *parent, const std::string &title = "Untitled");
    Window(Widget *parent, const std::string &title, const Vector2i& pos)
      : Window(parent, title) { setPosition(pos); }

    /// Return the window title
    const std::string &title() const { return mTitle; }
    /// Set the window title
    void setTitle(const std::string &title) { mTitle = title; }

    /// Is this a model dialog?
    bool modal() const { return mModal; }
    /// Set whether or not this is a modal dialog
    void setModal(bool modal) { mModal = modal; }

    /// Return the panel used to house window buttons
    Widget *buttonPanel();

    /// Dispose the window
    void dispose();

    /// Center the window in the current \ref Screen
    void center();

    /// Draw the window
    void draw(SDL_Renderer* surface) override;
    virtual void drawBody(SDL_Renderer* renderer);
    virtual void drawBodyTemp(SDL_Renderer* renderer);

    /// Handle window drag events
    bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    /// Handle mouse events recursively and bring the current window to the top
    bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override;
    /// Accept scroll events and propagate them to the widget under the mouse cursor
    bool scrollEvent(const Vector2i &p, const Vector2f &rel) override;
    /// Compute the preferred size of the widget
    Vector2i preferredSize(SDL_Renderer *ctx) const override;
    /// Invoke the associated layout generator to properly place child widgets, if any
    void performLayout(SDL_Renderer *ctx) override;

    /// Handle a focus change event (default implementation: record the focus status, but do nothing)
    bool focusEvent(bool focused);

protected:
    /// Internal helper function to maintain nested window position values; overridden in \ref Popup
    void refreshRelativePlacement();
protected:

    std::string mTitle;
    Widget *mButtonPanel;

    Texture _titleTex;

    bool mModal;
    bool mDrag;

    struct AsyncTexture;
    typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
    std::vector<AsyncTexturePtr> _txs;
};

NAMESPACE_END(sdlgui)
