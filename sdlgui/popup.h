/*
    sdl_gui/popup.h -- Simple popup widget which is attached to another given
    window (can be nested)

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <sdlgui/window.h>
#include <vector>

NAMESPACE_BEGIN(sdlgui)

/**
 * \class Popup popup.h sdl_gui/popup.h
 *
 * \brief Popup window for combo boxes, popup buttons, nested dialogs etc.
 *
 * Usually the Popup instance is constructed by another widget (e.g. \ref PopupButton)
 * and does not need to be created by hand.
 */
class  Popup : public Window 
{
public:
    /// Create a new popup parented to a screen (first argument) and a parent window
    Popup(Widget *parent, Window *parentWindow);

    /// Return the anchor position in the parent window; the placement of the popup is relative to it
    void setAnchorPos(const Vector2i &anchorPos) { mAnchorPos = anchorPos; }
    /// Set the anchor position in the parent window; the placement of the popup is relative to it
    const Vector2i &anchorPos() const { return mAnchorPos; }

    /// Set the anchor height; this determines the vertical shift relative to the anchor position
    void setAnchorHeight(int anchorHeight) { mAnchorHeight = anchorHeight; }
    /// Return the anchor height; this determines the vertical shift relative to the anchor position
    int anchorHeight() const { return mAnchorHeight; }

    /// Return the parent window of the popup
    Window *parentWindow() { return mParentWindow; }
    /// Return the parent window of the popup
    const Window *parentWindow() const { return mParentWindow; }

    /// Invoke the associated layout generator to properly place child widgets, if any
    void performLayout(SDL_Renderer *ctx) override;

    /// Draw the popup window
    void draw(SDL_Renderer* renderer) override;
    virtual void drawBody(SDL_Renderer* renderer) override;
    virtual void drawBodyTemp(SDL_Renderer* renderer);

protected:
    /// Internal helper function to maintain nested window position values
    virtual void refreshRelativePlacement();
    virtual void rendereBodyTexture(NVGcontext* &ctx, int& ctxw, int& ctxh, int dx);
    virtual Vector2i getOverrideBodyPos();

    Window *mParentWindow;
    Vector2i mAnchorPos;
    int mAnchorHeight;
    int _anchorDx = 15;

    struct AsyncTexture;
    typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
    std::vector<AsyncTexturePtr> _txs;
};

NAMESPACE_END(sdlgui)
