/*
    nanogui/nanogui.h -- Pull in *everything* from NanoGUI

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#ifndef PICOGUI_HEADER_INCLUDE
#define PICOGUI_HEADER_INCLUDE

#include <include/window.h>

NAMESPACE_BEGIN(nanogui)

/**
 * \brief Popup window for combo boxes, popup buttons, nested dialogs etc.
 *
 * Usually the Popup instance is constructed by another widget (e.g. \ref PopupButton)
 * and does not need to be created by hand.
 */
class NANOGUI_EXPORT Popup : public Window {
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
    virtual void performLayout(NVGcontext *ctx);

    /// Draw the popup window
    virtual void draw(NVGcontext* ctx);
protected:
    /// Internal helper function to maintain nested window position values
    virtual void refreshRelativePlacement();

protected:
    Window *mParentWindow;
    Vector2i mAnchorPos;
    int mAnchorHeight;
};

class NANOGUI_EXPORT Slider : public Widget {
public:
    Slider(Widget *parent);

    float value() const { return mValue; }
    void setValue(float value) { mValue = value; }

    const Color &highlightColor() const { return mHighlightColor; }
    void setHighlightColor(const Color &highlightColor) { mHighlightColor = highlightColor; }

    std::pair<float, float> highlightedRange() const { return mHighlightedRange; }
    void setHighlightedRange(std::pair<float, float> highlightedRange) { mHighlightedRange = highlightedRange; }

    std::function<void(float)> callback() const { return mCallback; }
    void setCallback(const std::function<void(float)> &callback) { mCallback = callback; }

    std::function<void(float)> finalCallback() const { return mFinalCallback; }
    void setFinalCallback(const std::function<void(float)> &callback) { mFinalCallback = callback; }

    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
    virtual void draw(NVGcontext* ctx);

protected:
    float mValue;
    std::function<void(float)> mCallback;
    std::function<void(float)> mFinalCallback;
    std::pair<float, float> mHighlightedRange;
    Color mHighlightColor;
};

NAMESPACE_END(nanogui)

#endif
