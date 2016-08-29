/*
    src/popup.cpp -- Simple popup widget which is attached to another given
    window (can be nested)

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <include/picogui.h>
#include <include/theme.h>
#include <include/opengl.h>

NAMESPACE_BEGIN(nanogui)
    
Popup::Popup(Widget *parent, Window *parentWindow)
    : Window(parent, ""), mParentWindow(parentWindow),
      mAnchorPos(Vector2i::Zero()), mAnchorHeight(30) {
}

void Popup::performLayout(NVGcontext *ctx) {
    if (mLayout || mChildren.size() != 1) {
        Widget::performLayout(ctx);
    } else {
        mChildren[0]->setPosition(Vector2i::Zero());
        mChildren[0]->setSize(mSize);
        mChildren[0]->performLayout(ctx);
    }
}

void Popup::refreshRelativePlacement() {
    mParentWindow->refreshRelativePlacement();
    mVisible &= mParentWindow->visibleRecursive();
    mPos = mParentWindow->position() + mAnchorPos - Vector2i(0, mAnchorHeight);
}

void Popup::draw(NVGcontext* ctx) {
    refreshRelativePlacement();

    if (!mVisible)
        return;

    int ds = mTheme->mWindowDropShadowSize, cr = mTheme->mWindowCornerRadius;

    /* Draw a drop shadow */
    NVGpaint shadowPaint = nvgBoxGradient(
        ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr*2, ds*2,
        mTheme->mDropShadow, mTheme->mTransparent);

    nvgBeginPath(ctx);
    nvgRect(ctx, mPos.x()-ds,mPos.y()-ds, mSize.x()+2*ds, mSize.y()+2*ds);
    nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);
    nvgPathWinding(ctx, NVG_HOLE);
    nvgFillPaint(ctx, shadowPaint);
    nvgFill(ctx);

    /* Draw window */
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);

    nvgMoveTo(ctx, mPos.x()-15,mPos.y()+mAnchorHeight);
    nvgLineTo(ctx, mPos.x()+1,mPos.y()+mAnchorHeight-15);
    nvgLineTo(ctx, mPos.x()+1,mPos.y()+mAnchorHeight+15);

    nvgFillColor(ctx, mTheme->mWindowPopup);
    nvgFill(ctx);

    Widget::draw(ctx);
}

/***************************** Slider *********************************************/

Slider::Slider(Widget *parent)
    : Widget(parent), mValue(0.0f), mHighlightedRange(std::make_pair(0.f, 0.f)) {
    mHighlightColor = Color(255, 80, 80, 70);
}

Vector2i Slider::preferredSize(NVGcontext *) const {
    return Vector2i(70, 12);
}

bool Slider::mouseDragEvent(const Vector2i &p, const Vector2i & /* rel */,
                            int /* button */, int /* modifiers */) {
    if (!mEnabled)
        return false;
    mValue = std::min(std::max((p.x() - mPos.x()) / (float) mSize.x(), (float) 0.0f), (float) 1.0f);
    if (mCallback)
        mCallback(mValue);
    return true;
}

bool Slider::mouseButtonEvent(const Vector2i &p, int /* button */, bool down, int /* modifiers */) {
    if (!mEnabled)
        return false;
    mValue = std::min(std::max((p.x() - mPos.x()) / (float) mSize.x(), (float) 0.0f), (float) 1.0f);
    if (mCallback)
        mCallback(mValue);
    if (mFinalCallback && !down)
        mFinalCallback(mValue);
    return true;
}

void Slider::draw(NVGcontext* ctx) {
    Vector2f center = mPos.cast<float>() + mSize.cast<float>() * 0.5f;
    Vector2f knobPos(mPos.x() + mValue * mSize.x(), center.y() + 0.5f);
    float kr = (int)(mSize.y()*0.5f);
    NVGpaint bg = nvgBoxGradient(ctx,
        mPos.x(), center.y() - 3 + 1, mSize.x(), 6, 3, 3, Color(0, mEnabled ? 32 : 10), Color(0, mEnabled ? 128 : 210));

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x(), center.y() - 3 + 1, mSize.x(), 6, 2);
    nvgFillPaint(ctx, bg);
    nvgFill(ctx);

    if (mHighlightedRange.second != mHighlightedRange.first) {
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, mPos.x() + mHighlightedRange.first * mSize.x(), center.y() - 3 + 1, mSize.x() * (mHighlightedRange.second-mHighlightedRange.first), 6, 2);
        nvgFillColor(ctx, mHighlightColor);
        nvgFill(ctx);
    }

    NVGpaint knobShadow = nvgRadialGradient(ctx,
        knobPos.x(), knobPos.y(), kr-3, kr+3, Color(0, 64), mTheme->mTransparent);

    nvgBeginPath(ctx);
    nvgRect(ctx, knobPos.x() - kr - 5, knobPos.y() - kr - 5, kr*2+10, kr*2+10+3);
    nvgCircle(ctx, knobPos.x(), knobPos.y(), kr);
    nvgPathWinding(ctx, NVG_HOLE);
    nvgFillPaint(ctx, knobShadow);
    nvgFill(ctx);

    NVGpaint knob = nvgLinearGradient(ctx,
        mPos.x(), center.y() - kr, mPos.x(), center.y() + kr,
        mTheme->mBorderLight, mTheme->mBorderMedium);
    NVGpaint knobReverse = nvgLinearGradient(ctx,
        mPos.x(), center.y() - kr, mPos.x(), center.y() + kr,
        mTheme->mBorderMedium,
        mTheme->mBorderLight);

    nvgBeginPath(ctx);
    nvgCircle(ctx, knobPos.x(), knobPos.y(), kr);
    nvgStrokeColor(ctx, mTheme->mBorderDark);
    nvgFillPaint(ctx, knob);
    nvgStroke(ctx);
    nvgFill(ctx);
    nvgBeginPath(ctx);
    nvgCircle(ctx, knobPos.x(), knobPos.y(), kr/2);
    nvgFillColor(ctx, Color(150, mEnabled ? 255 : 100));
    nvgStrokePaint(ctx, knobReverse);
    nvgStroke(ctx);
    nvgFill(ctx);
}

NAMESPACE_END(nanogui)
