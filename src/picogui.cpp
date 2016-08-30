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
#include <include/opengl.h>
#include <include/label.h>
#include <include/screen.h>
#include <include/nanogui_resources.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include <sstream>
#include <regex>
#include <numeric>

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

/****************************** Button ***********************************************/

Button::Button(Widget *parent, const std::string &caption, int icon)
    : Widget(parent), mCaption(caption), mIcon(icon),
      mIconPosition(IconPosition::LeftCentered), mPushed(false),
      mFlags(NormalButton), mBackgroundColor(Color(0, 0)),
      mTextColor(Color(0, 0)) { }

Vector2i Button::preferredSize(NVGcontext *ctx) const {
    int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
    nvgFontSize(ctx, fontSize);
    nvgFontFace(ctx, "sans-bold");
    float tw = nvgTextBounds(ctx, 0,0, mCaption.c_str(), nullptr, nullptr);
    float iw = 0.0f, ih = fontSize;

    if (mIcon) {
        if (nvgIsFontIcon(mIcon)) {
            ih *= 1.5f;
            nvgFontFace(ctx, "icons");
            nvgFontSize(ctx, ih);
            iw = nvgTextBounds(ctx, 0, 0, utf8(mIcon).data(), nullptr, nullptr)
                + mSize.y() * 0.15f;
        } else {
            int w, h;
            ih *= 0.9f;
            nvgImageSize(ctx, mIcon, &w, &h);
            iw = w * ih / h;
        }
    }
    return Vector2i((int)(tw + iw) + 20, fontSize + 10);
}

bool Button::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers)
{
    Widget::mouseButtonEvent(p, button, down, modifiers);
    /* Temporarily increase the reference count of the button in case the
       button causes the parent window to be destructed */
    ref<Button> self = this;

    if (button == SDL_BUTTON_LEFT && mEnabled) {
        bool pushedBackup = mPushed;
        if (down) {
            if (mFlags & RadioButton) {
                if (mButtonGroup.empty()) {
                    for (auto widget : parent()->children()) {
                        Button *b = dynamic_cast<Button *>(widget);
                        if (b != this && b && (b->flags() & RadioButton) && b->mPushed) {
                            b->mPushed = false;
                            if (b->mChangeCallback)
                                b->mChangeCallback(false);
                        }
                    }
                } else {
                    for (auto b : mButtonGroup) {
                        if (b != this && (b->flags() & RadioButton) && b->mPushed) {
                            b->mPushed = false;
                            if (b->mChangeCallback)
                                b->mChangeCallback(false);
                        }
                    }
                }
            }
            if (mFlags & PopupButton) {
                for (auto widget : parent()->children()) {
                    Button *b = dynamic_cast<Button *>(widget);
                    if (b != this && b && (b->flags() & PopupButton) && b->mPushed) {
                        b->mPushed = false;
                        if(b->mChangeCallback)
                            b->mChangeCallback(false);
                    }
                }
            }
            if (mFlags & ToggleButton)
                mPushed = !mPushed;
            else
                mPushed = true;
        } else if (mPushed) {
            if (contains(p) && mCallback)
                mCallback();
            if (mFlags & NormalButton)
                mPushed = false;
        }
        if (pushedBackup != mPushed && mChangeCallback)
            mChangeCallback(mPushed);

        return true;
    }
    return false;
}

void Button::draw(NVGcontext *ctx) {
    Widget::draw(ctx);

    NVGcolor gradTop = mTheme->mButtonGradientTopUnfocused;
    NVGcolor gradBot = mTheme->mButtonGradientBotUnfocused;

    if (mPushed) {
        gradTop = mTheme->mButtonGradientTopPushed;
        gradBot = mTheme->mButtonGradientBotPushed;
    } else if (mMouseFocus && mEnabled) {
        gradTop = mTheme->mButtonGradientTopFocused;
        gradBot = mTheme->mButtonGradientBotFocused;
    }

    nvgBeginPath(ctx);

    nvgRoundedRect(ctx, mPos.x() + 1, mPos.y() + 1.0f, mSize.x() - 2,
                   mSize.y() - 2, mTheme->mButtonCornerRadius - 1);

    if (mBackgroundColor.a() != 0) {
        nvgFillColor(ctx, mBackgroundColor.withAlpha(1.f));
        nvgFill(ctx);
        if (mPushed) {
            gradTop.a = gradBot.a = 0.8f;
        } else {
            double v = 1 - mBackgroundColor.a();
            gradTop.a = gradBot.a = mEnabled ? v : v * .5f + .5f;
        }
    }

    NVGpaint bg = nvgLinearGradient(ctx, mPos.x(), mPos.y(), mPos.x(),
                                    mPos.y() + mSize.y(), gradTop, gradBot);

    nvgFillPaint(ctx, bg);
    nvgFill(ctx);

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x() + 0.5f, mPos.y() + (mPushed ? 0.5f : 1.5f), mSize.x() - 1,
                   mSize.y() - 1 - (mPushed ? 0.0f : 1.0f), mTheme->mButtonCornerRadius);
    nvgStrokeColor(ctx, mTheme->mBorderLight);
    nvgStroke(ctx);

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x() + 0.5f, mPos.y() + 0.5f, mSize.x() - 1,
                   mSize.y() - 2, mTheme->mButtonCornerRadius);
    nvgStrokeColor(ctx, mTheme->mBorderDark);
    nvgStroke(ctx);

    int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
    nvgFontSize(ctx, fontSize);
    nvgFontFace(ctx, "sans-bold");
    float tw = nvgTextBounds(ctx, 0,0, mCaption.c_str(), nullptr, nullptr);

    Vector2f center = mPos.cast<float>() + mSize.cast<float>() * 0.5f;
    Vector2f textPos(center.x() - tw * 0.5f, center.y() - 1);
    NVGcolor textColor =
        mTextColor.a() == 0 ? mTheme->mTextColor : mTextColor;

    if (!mEnabled)
        textColor = mTheme->mDisabledTextColor;

    if (mIcon) {
        auto icon = utf8(mIcon);

        float iw, ih = fontSize;
        if (nvgIsFontIcon(mIcon)) {
            ih *= 1.5f;
            nvgFontSize(ctx, ih);
            nvgFontFace(ctx, "icons");
            iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
        } else {
            int w, h;
            ih *= 0.9f;
            nvgImageSize(ctx, mIcon, &w, &h);
            iw = w * ih / h;
        }
        if (mCaption != "")
            iw += mSize.y() * 0.15f;
        nvgFillColor(ctx, textColor);
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        Vector2f iconPos = center;
        iconPos.ry() -= 1;

        if (mIconPosition == IconPosition::LeftCentered) {
            iconPos.rx() -= (tw + iw) * 0.5f;
            textPos.rx() += iw * 0.5f;
        } else if (mIconPosition == IconPosition::RightCentered) {
            textPos.rx() -= iw * 0.5f;
            iconPos.rx() += tw * 0.5f;
        } else if (mIconPosition == IconPosition::Left) {
            iconPos.rx() = mPos.x() + 8;
        } else if (mIconPosition == IconPosition::Right) {
            iconPos.rx() = mPos.x() + mSize.x() - iw - 8;
        }

        if (nvgIsFontIcon(mIcon)) {
            nvgText(ctx, iconPos.x(), iconPos.y()+1, icon.data(), nullptr);
        } else {
            NVGpaint imgPaint = nvgImagePattern(ctx,
                    iconPos.x(), iconPos.y() - ih/2, iw, ih, 0, mIcon, mEnabled ? 0.5f : 0.25f);

            nvgFillPaint(ctx, imgPaint);
            nvgFill(ctx);
        }
    }

    nvgFontSize(ctx, fontSize);
    nvgFontFace(ctx, "sans-bold");
    nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgFillColor(ctx, mTheme->mTextColorShadow);
    nvgText(ctx, textPos.x(), textPos.y(), mCaption.c_str(), nullptr);
    nvgFillColor(ctx, textColor);
    nvgText(ctx, textPos.x(), textPos.y() + 1, mCaption.c_str(), nullptr);
}

Button::Button(Widget* parent, const std::string& caption,
               const std::function<void ()>& callback, int icon)
  : Button( parent, caption, icon )
{
  setCallback( callback );
}

/******************************** PopupButton **********************************************/

PopupButton::PopupButton(Widget *parent, const std::string &caption,
                         int buttonIcon, int chevronIcon)
    : Button(parent, caption, buttonIcon), mChevronIcon(chevronIcon)
{
    setFlags(Flags::ToggleButton | Flags::PopupButton);

    Window *parentWindow = window();
    mPopup = &parentWindow->parent()->add<Popup>(window());
    mPopup->setSize(Vector2i(320, 250));
}

Vector2i PopupButton::preferredSize(NVGcontext *ctx) const {
    return Button::preferredSize(ctx) + Vector2i(15, 0);
}

void PopupButton::draw(NVGcontext* ctx) {
    if (!mEnabled && mPushed)
        mPushed = false;

    mPopup->setVisible(mPushed);
    Button::draw(ctx);

    if (mChevronIcon) {
        auto icon = utf8(mChevronIcon);
        NVGcolor textColor =
            mTextColor.a() == 0 ? mTheme->mTextColor : mTextColor;

        nvgFontSize(ctx, (mFontSize < 0 ? mTheme->mButtonFontSize : mFontSize) * 1.5f);
        nvgFontFace(ctx, "icons");
        nvgFillColor(ctx, mEnabled ? textColor : mTheme->mDisabledTextColor);
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        float iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
        Vector2f iconPos(mPos.x() + mSize.x() - iw - 8,
                         mPos.y() + mSize.y() * 0.5f - 1);

        nvgText(ctx, iconPos.x(), iconPos.y(), icon.data(), nullptr);
    }
}

void PopupButton::performLayout(NVGcontext *ctx) {
    Widget::performLayout(ctx);

    const Window *parentWindow = window();

    mPopup->setAnchorPos(Vector2i(parentWindow->width() + 15,
        absolutePosition().y() - parentWindow->position().y() + mSize.y() /2));
}

/**************************************** Checkbox ***********************************/

CheckBox::CheckBox(Widget *parent, const std::string &caption,
                   const std::function<void(bool) > &callback)
    : Widget(parent), mCaption(caption), mPushed(false), mChecked(false),
      mCallback(callback) { }

bool CheckBox::mouseButtonEvent(const Vector2i &p, int button, bool down,
                                int modifiers)
{
    Widget::mouseButtonEvent(p, button, down, modifiers);
    if (!mEnabled)
        return false;

    if (button == SDL_BUTTON_LEFT )
    {
        if (down)
        {
            mPushed = true;
        }
        else if (mPushed)
        {
            if (contains(p))
            {
                mChecked = !mChecked;
                if (mCallback)
                    mCallback(mChecked);
            }
            mPushed = false;
        }
        return true;
    }
    return false;
}

Vector2i CheckBox::preferredSize(NVGcontext *ctx) const {
    if (mFixedSize != Vector2i::Zero())
        return mFixedSize;
    nvgFontSize(ctx, fontSize());
    nvgFontFace(ctx, "sans");
    return Vector2i(
        nvgTextBounds(ctx, 0, 0, mCaption.c_str(), nullptr, nullptr) +
            1.7f * fontSize(),
        fontSize() * 1.3f);
}

void CheckBox::draw(NVGcontext *ctx) {
    Widget::draw(ctx);

    nvgFontSize(ctx, fontSize());
    nvgFontFace(ctx, "sans");
    nvgFillColor(ctx,
                 mEnabled ? mTheme->mTextColor : mTheme->mDisabledTextColor);
    nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgText(ctx, mPos.x() + 1.2f * mSize.y() + 5, mPos.y() + mSize.y() * 0.5f,
            mCaption.c_str(), nullptr);

    NVGpaint bg = nvgBoxGradient(ctx, mPos.x() + 1.5f, mPos.y() + 1.5f,
                                 mSize.y() - 2.0f, mSize.y() - 2.0f, 3, 3,
                                 mPushed ? Color(0, 100) : Color(0, 32),
                                 Color(0, 0, 0, 180));

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x() + 1.0f, mPos.y() + 1.0f, mSize.y() - 2.0f,
                   mSize.y() - 2.0f, 3);
    nvgFillPaint(ctx, bg);
    nvgFill(ctx);

    if (mChecked) {
        nvgFontSize(ctx, 1.8 * mSize.y());
        nvgFontFace(ctx, "icons");
        nvgFillColor(ctx, mEnabled ? mTheme->mIconColor
                                   : mTheme->mDisabledTextColor);
        nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(ctx, mPos.x() + mSize.y() * 0.5f + 1,
                mPos.y() + mSize.y() * 0.5f, utf8(ENTYPO_ICON_CHECK).data(),
                nullptr);
    }
}

/********************************* MessageDialog **********************************/

MessageDialog::MessageDialog(Widget *parent, Type type, const std::string &title,
              const std::string &message,
              const std::string &buttonText,
              const std::string &altButtonText, bool altButton) : Window(parent, title)
{
    setLayout(new BoxLayout(Orientation::Vertical,
                            Alignment::Middle, 10, 10));
    setModal(true);

    Widget *panel1 = new Widget(this);
    panel1->setLayout(new BoxLayout(Orientation::Horizontal,
                                    Alignment::Middle, 10, 15));
    int icon = 0;
    switch (type) {
        case Type::Information: icon = ENTYPO_ICON_CIRCLED_INFO; break;
        case Type::Question: icon = ENTYPO_ICON_CIRCLED_HELP; break;
        case Type::Warning: icon = ENTYPO_ICON_WARNING; break;
    }
    Label *iconLabel = new Label(panel1, std::string(utf8(icon).data()), "icons");
    iconLabel->setFontSize(50);
    mMessageLabel = new Label(panel1, message);
    mMessageLabel->setFixedWidth(200);
    Widget *panel2 = new Widget(this);
    panel2->setLayout(new BoxLayout(Orientation::Horizontal,
                                    Alignment::Middle, 0, 15));

    if (altButton) {
        Button *button = new Button(panel2, altButtonText, ENTYPO_ICON_CIRCLED_CROSS);
        button->setCallback([&] { if (mCallback) mCallback(1); dispose(); });
    }
    Button *button = new Button(panel2, buttonText, ENTYPO_ICON_CHECK);
    button->setCallback([&] { if (mCallback) mCallback(0); dispose(); });
    center();
    requestFocus();
}

/**************************************** Window *****************************************/

Window::Window(Widget *parent, const std::string &title)
    : Widget(parent), mTitle(title), mButtonPanel(nullptr), mModal(false), mDrag(false) { }

Vector2i Window::preferredSize(NVGcontext *ctx) const
{
    if (mButtonPanel)
        mButtonPanel->setVisible(false);
    Vector2i result = Widget::preferredSize(ctx);
    if (mButtonPanel)
        mButtonPanel->setVisible(true);

    nvgFontSize(ctx, 18.0f);
    nvgFontFace(ctx, "sans-bold");
    float bounds[4];
    nvgTextBounds(ctx, 0, 0, mTitle.c_str(), nullptr, bounds);

    return result.cwiseMax(bounds[2]-bounds[0] + 20, bounds[3]-bounds[1]);
}

Widget *Window::buttonPanel() {
    if (!mButtonPanel) {
        mButtonPanel = new Widget(this);
        mButtonPanel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 4));
    }
    return mButtonPanel;
}

void Window::performLayout(NVGcontext *ctx)
{
    if (!mButtonPanel) {
        Widget::performLayout(ctx);
    } else {
        mButtonPanel->setVisible(false);
        Widget::performLayout(ctx);
        for (auto w : mButtonPanel->children()) {
            w->setFixedSize(Vector2i(22, 22));
            w->setFontSize(15);
        }
        mButtonPanel->setVisible(true);
        mButtonPanel->setSize(Vector2i(width(), 22));
        mButtonPanel->setPosition(Vector2i(width() - (mButtonPanel->preferredSize(ctx).x() + 5), 3));
        mButtonPanel->performLayout(ctx);
    }
}

void Window::draw(NVGcontext *ctx) {
    int ds = mTheme->mWindowDropShadowSize, cr = mTheme->mWindowCornerRadius;
    int hh = mTheme->mWindowHeaderHeight;

    /* Draw window */
    nvgSave(ctx);
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);

    nvgFillColor(ctx, mMouseFocus ? mTheme->mWindowFillFocused
                                  : mTheme->mWindowFillUnfocused);
    nvgFill(ctx);

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

    if (!mTitle.empty()) {
        /* Draw header */
        NVGpaint headerPaint = nvgLinearGradient(
            ctx, mPos.x(), mPos.y(), mPos.x(),
            mPos.y() + hh,
            mTheme->mWindowHeaderGradientTop,
            mTheme->mWindowHeaderGradientBot);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), hh, cr);

        nvgFillPaint(ctx, headerPaint);
        nvgFill(ctx);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), hh, cr);
        nvgStrokeColor(ctx, mTheme->mWindowHeaderSepTop);
        nvgScissor(ctx, mPos.x(), mPos.y(), mSize.x(), 0.5f);
        nvgStroke(ctx);
        nvgResetScissor(ctx);

        nvgBeginPath(ctx);
        nvgMoveTo(ctx, mPos.x() + 0.5f, mPos.y() + hh - 1.5f);
        nvgLineTo(ctx, mPos.x() + mSize.x() - 0.5f, mPos.y() + hh - 1.5);
        nvgStrokeColor(ctx, mTheme->mWindowHeaderSepBot);
        nvgStroke(ctx);

        nvgFontSize(ctx, 18.0f);
        nvgFontFace(ctx, "sans-bold");
        nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

        nvgFontBlur(ctx, 2);
        nvgFillColor(ctx, mTheme->mDropShadow);
        nvgText(ctx, mPos.x() + mSize.x() / 2,
                mPos.y() + hh / 2, mTitle.c_str(), nullptr);

        nvgFontBlur(ctx, 0);
        nvgFillColor(ctx, mFocused ? mTheme->mWindowTitleFocused
                                   : mTheme->mWindowTitleUnfocused);
        nvgText(ctx, mPos.x() + mSize.x() / 2, mPos.y() + hh / 2 - 1,
                mTitle.c_str(), nullptr);
    }

    nvgRestore(ctx);
    Widget::draw(ctx);
}

void Window::dispose() {
    Widget *widget = this;
    while (widget->parent())
        widget = widget->parent();
    ((Screen *) widget)->disposeWindow(this);
}

void Window::center() {
    Widget *widget = this;
    while (widget->parent())
        widget = widget->parent();
    ((Screen *) widget)->centerWindow(this);
}

bool Window::mouseDragEvent(const Vector2i &, const Vector2i &rel,
                            int button, int /* modifiers */) {
    if (mDrag && (button & (1 << SDL_BUTTON_LEFT)) != 0) {
        mPos += rel;
        mPos = mPos.cwiseMax(Vector2i::Zero());
        mPos = mPos.cwiseMin(parent()->size() - mSize);
        return true;
    }
    return false;
}

bool Window::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
    if (Widget::mouseButtonEvent(p, button, down, modifiers))
        return true;
    if (button == SDL_BUTTON_LEFT)
    {
        mDrag = down && (p.y() - mPos.y()) < mTheme->mWindowHeaderHeight;
        return true;
    }
    return false;
}

bool Window::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    Widget::scrollEvent(p, rel);
    return true;
}

void Window::refreshRelativePlacement() {
    /* Overridden in \ref Popup */
}

/********************************** Widget *****************************************/

Widget::Widget(Widget *parent)
    : mParent(nullptr), mTheme(nullptr), mLayout(nullptr),
      mPos(Vector2i::Zero()), mSize(Vector2i::Zero()),
      mFixedSize(Vector2i::Zero()), mVisible(true), mEnabled(true),
      mFocused(false), mMouseFocus(false), mTooltip(""), mFontSize(-1.0f),
      mCursor(Cursor::Arrow) {
    if (parent) {
        parent->addChild(this);
        mTheme = parent->mTheme;
    }
}

Widget::~Widget() {
    for (auto child : mChildren) {
        if (child)
            child->decRef();
    }
}

int Widget::fontSize() const {
    return mFontSize < 0 ? mTheme->mStandardFontSize : mFontSize;
}

Vector2i Widget::preferredSize(NVGcontext *ctx) const {
    if (mLayout)
        return mLayout->preferredSize(ctx, this);
    else
        return mSize;
}

void Widget::performLayout(NVGcontext *ctx) {
    if (mLayout) {
        mLayout->performLayout(ctx, this);
    } else {
        for (auto c : mChildren) {
            Vector2i pref = c->preferredSize(ctx), fix = c->fixedSize();
            c->setSize(Vector2i(
                fix.x() ? fix.x() : pref.x(),
                fix.y() ? fix.y() : pref.y()
            ));
            c->performLayout(ctx);
        }
    }
}

Widget *Widget::findWidget(const Vector2i &p) {
    for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
        Widget *child = *it;
        if (child->visible() && child->contains(p - mPos))
            return child->findWidget(p - mPos);
    }
    return contains(p) ? this : nullptr;
}

bool Widget::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
    for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
        Widget *child = *it;
        if (child->visible() && child->contains(p - mPos) &&
            child->mouseButtonEvent(p - mPos, button, down, modifiers))
            return true;
    }
    if (button == SDL_BUTTON_LEFT && down && !mFocused)
        requestFocus();
    return false;
}

bool Widget::mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) {
    for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
        Widget *child = *it;
        if (!child->visible())
            continue;
        bool contained = child->contains(p - mPos), prevContained = child->contains(p - mPos - rel);
        if (contained != prevContained)
            child->mouseEnterEvent(p, contained);
        if ((contained || prevContained) &&
            child->mouseMotionEvent(p - mPos, rel, button, modifiers))
            return true;
    }
    return false;
}

bool Widget::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
        Widget *child = *it;
        if (!child->visible())
            continue;
        if (child->contains(p - mPos) && child->scrollEvent(p - mPos, rel))
            return true;
    }
    return false;
}

bool Widget::mouseDragEvent(const Vector2i &, const Vector2i &, int, int) {
    return false;
}

bool Widget::mouseEnterEvent(const Vector2i &, bool enter) {
    mMouseFocus = enter;
    return false;
}

bool Widget::focusEvent(bool focused) {
    mFocused = focused;
    return false;
}

bool Widget::keyboardEvent(int, int, int, int) {
    return false;
}

bool Widget::keyboardCharacterEvent(unsigned int) {
    return false;
}

void Widget::addChild(Widget *widget) {
    mChildren.push_back(widget);
    widget->incRef();
    widget->setParent(this);
}

void Widget::removeChild(const Widget *widget) {
    for(auto it=mChildren.begin(); it != mChildren.end(); )
        if ( *it == widget) it = mChildren.erase(it);
        else ++it;
    widget->decRef();
}

void Widget::removeChild(int index) {
    Widget *widget = mChildren[index];
    mChildren.erase(mChildren.begin() + index);
    widget->decRef();
}

Window *Widget::window() {
    Widget *widget = this;
    while (true) {
        if (!widget)
            throw std::runtime_error(
                "Widget:internal error (could not find parent window)");
        Window *window = dynamic_cast<Window *>(widget);
        if (window)
            return window;
        widget = widget->parent();
    }
}

void Widget::requestFocus() {
    Widget *widget = this;
    while (widget->parent())
        widget = widget->parent();
    ((Screen *) widget)->updateFocus(this);
}

void Widget::draw(NVGcontext *ctx) {
    #if NANOGUI_SHOW_WIDGET_BOUNDS
        nvgStrokeWidth(ctx, 1.0f);
        nvgBeginPath(ctx);
        nvgRect(ctx, mPos.x() - 0.5f, mPos.y() - 0.5f, mSize.x() + 1, mSize.y() + 1);
        nvgStrokeColor(ctx, nvgRGBA(255, 0, 0, 255));
        nvgStroke(ctx);
    #endif

    if (mChildren.empty())
        return;

    nvgTranslate(ctx, mPos.x(), mPos.y());
    for (auto child : mChildren)
        if (child->visible())
            child->draw(ctx);
    nvgTranslate(ctx, -mPos.x(), -mPos.y());
}

/*********************************** Combobox **************************************/

ComboBox::ComboBox(Widget *parent) : PopupButton(parent), mSelectedIndex(0) {
}

ComboBox::ComboBox(Widget *parent, const std::vector<std::string> &items)
    : PopupButton(parent), mSelectedIndex(0) {
    setItems(items);
}

ComboBox::ComboBox(Widget *parent, const std::vector<std::string> &items, const std::vector<std::string> &itemsShort)
    : PopupButton(parent), mSelectedIndex(0) {
    setItems(items, itemsShort);
}

void ComboBox::setSelectedIndex(int idx) {
    if (mItemsShort.empty())
        return;
    const std::vector<Widget *> &children = popup()->children();
    ((Button *) children[mSelectedIndex])->setPushed(false);
    ((Button *) children[idx])->setPushed(true);
    mSelectedIndex = idx;
    setCaption(mItemsShort[idx]);
}

void ComboBox::setItems(const std::vector<std::string> &items, const std::vector<std::string> &itemsShort) {
    assert(items.size() == itemsShort.size());
    mItems = items;
    mItemsShort = itemsShort;
    if (mSelectedIndex < 0 || mSelectedIndex >= (int) items.size())
        mSelectedIndex = 0;
    while (mPopup->childCount() != 0)
        mPopup->removeChild(mPopup->childCount()-1);
    mPopup->setLayout(new GroupLayout(10));
    int index = 0;
    for (const auto &str: items) {
        Button *button = new Button(mPopup, str);
        button->setFlags(Button::RadioButton);
        button->setCallback([&, index] {
            mSelectedIndex = index;
            setCaption(mItemsShort[index]);
            setPushed(false);
            popup()->setVisible(false);
            if (mCallback)
                mCallback(index);
        });
        index++;
    }
    setSelectedIndex(mSelectedIndex);
}

/******************************* ColorWheel ****************************************************/

ColorWheel::ColorWheel(Widget *parent, const Color& rgb)
    : Widget(parent), mDragRegion(None) {
    setColor(rgb);
}

Vector2i ColorWheel::preferredSize(NVGcontext *) const {
    return Vector2i(100, 100.);
}

void ColorWheel::draw(NVGcontext *ctx) {
    Widget::draw(ctx);

    if (!mVisible)
        return;

    float x = mPos.x(),
          y = mPos.y(),
          w = mSize.x(),
          h = mSize.y();

    NVGcontext* vg = ctx;

    int i;
    float r0, r1, ax,ay, bx,by, cx,cy, aeps, r;
    float hue = mHue;
    NVGpaint paint;

    nvgSave(vg);

    cx = x + w*0.5f;
    cy = y + h*0.5f;
    r1 = (w < h ? w : h) * 0.5f - 5.0f;
    r0 = r1 * .75f;

    aeps = 0.5f / r1;   // half a pixel arc length in radians (2pi cancels out).

    for (i = 0; i < 6; i++) {
        float a0 = (float)i / 6.0f * NVG_PI * 2.0f - aeps;
        float a1 = (float)(i+1.0f) / 6.0f * NVG_PI * 2.0f + aeps;
        nvgBeginPath(vg);
        nvgArc(vg, cx,cy, r0, a0, a1, NVG_CW);
        nvgArc(vg, cx,cy, r1, a1, a0, NVG_CCW);
        nvgClosePath(vg);
        ax = cx + cosf(a0) * (r0+r1)*0.5f;
        ay = cy + sinf(a0) * (r0+r1)*0.5f;
        bx = cx + cosf(a1) * (r0+r1)*0.5f;
        by = cy + sinf(a1) * (r0+r1)*0.5f;
        paint = nvgLinearGradient(vg, ax, ay, bx, by,
                                  nvgHSLA(a0 / (NVG_PI * 2), 1.0f, 0.55f, 255),
                                  nvgHSLA(a1 / (NVG_PI * 2), 1.0f, 0.55f, 255));
        nvgFillPaint(vg, paint);
        nvgFill(vg);
    }

    nvgBeginPath(vg);
    nvgCircle(vg, cx,cy, r0-0.5f);
    nvgCircle(vg, cx,cy, r1+0.5f);
    nvgStrokeColor(vg, nvgRGBA(0,0,0,64));
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);

    // Selector
    nvgSave(vg);
    nvgTranslate(vg, cx,cy);
    nvgRotate(vg, hue*NVG_PI*2);

    // Marker on
    float u = std::max(r1/50, 1.5f);
          u = std::min(u, 4.f);
    nvgStrokeWidth(vg, u);
    nvgBeginPath(vg);
    nvgRect(vg, r0-1,-2*u,r1-r0+2,4*u);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,192));
    nvgStroke(vg);

    paint = nvgBoxGradient(vg, r0-3,-5,r1-r0+6,10, 2,4, nvgRGBA(0,0,0,128), nvgRGBA(0,0,0,0));
    nvgBeginPath(vg);
    nvgRect(vg, r0-2-10,-4-10,r1-r0+4+20,8+20);
    nvgRect(vg, r0-2,-4,r1-r0+4,8);
    nvgPathWinding(vg, NVG_HOLE);
    nvgFillPaint(vg, paint);
    nvgFill(vg);

    // Center triangle
    r = r0 - 6;
    ax = cosf(120.0f/180.0f*NVG_PI) * r;
    ay = sinf(120.0f/180.0f*NVG_PI) * r;
    bx = cosf(-120.0f/180.0f*NVG_PI) * r;
    by = sinf(-120.0f/180.0f*NVG_PI) * r;
    nvgBeginPath(vg);
    nvgMoveTo(vg, r,0);
    nvgLineTo(vg, ax, ay);
    nvgLineTo(vg, bx, by);
    nvgClosePath(vg);
    paint = nvgLinearGradient(vg, r, 0, ax, ay, nvgHSLA(hue, 1.0f, 0.5f, 255),
                              nvgRGBA(255, 255, 255, 255));
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    paint = nvgLinearGradient(vg, (r + ax) * 0.5f, (0 + ay) * 0.5f, bx, by,
                              nvgRGBA(0, 0, 0, 0), nvgRGBA(0, 0, 0, 255));
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 64));
    nvgStroke(vg);

    // Select circle on triangle
    float sx = r*(1 - mWhite - mBlack) + ax*mWhite + bx*mBlack;
    float sy =                           ay*mWhite + by*mBlack;

    nvgStrokeWidth(vg, u);
    nvgBeginPath(vg);
    nvgCircle(vg, sx,sy,2*u);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,192));
    nvgStroke(vg);

    nvgRestore(vg);

    nvgRestore(vg);
}

bool ColorWheel::mouseButtonEvent(const Vector2i &p, int button, bool down,
                                  int modifiers) {
    Widget::mouseButtonEvent(p, button, down, modifiers);
    /*if (!mEnabled || button != GLFW_MOUSE_BUTTON_1)
        return false;*/

    if (down) {
        mDragRegion = adjustPosition(p);
        return mDragRegion != None;
    } else {
        mDragRegion = None;
        return true;
    }
}

bool ColorWheel::mouseDragEvent(const Vector2i &p, const Vector2i &,
                                int, int) {
    return adjustPosition(p, mDragRegion) != None;
}

ColorWheel::Region ColorWheel::adjustPosition(const Vector2i &p, Region consideredRegions) {
    float x = p.x() - mPos.x(),
          y = p.y() - mPos.y(),
          w = mSize.x(),
          h = mSize.y();

    float cx = w*0.5f;
    float cy = h*0.5f;
    float r1 = (w < h ? w : h) * 0.5f - 5.0f;
    float r0 = r1 * .75f;

    x -= cx;
    y -= cy;

    float mr = std::sqrt(x*x + y*y);

    if ((consideredRegions & OuterCircle) &&
        ((mr >= r0 && mr <= r1) || (consideredRegions == OuterCircle))) {
        if (!(consideredRegions & OuterCircle))
            return None;
        mHue = std::atan(y / x);
        if (x < 0)
            mHue += NVG_PI;
        mHue /= 2*NVG_PI;

        if (mCallback)
            mCallback(color());

        return OuterCircle;
    }

    float r = r0 - 6;

    float ax = std::cos( 120.0f/180.0f*NVG_PI) * r;
    float ay = std::sin( 120.0f/180.0f*NVG_PI) * r;
    float bx = std::cos(-120.0f/180.0f*NVG_PI) * r;
    float by = std::sin(-120.0f/180.0f*NVG_PI) * r;

    /* TODO UNIMPLEMENTED
     * typedef Eigen::Matrix<float,2,2>        Matrix2f;

    Eigen::Matrix<float, 2, 3> triangle;
    triangle << ax,bx,r,
                ay,by,0;
    triangle = Eigen::Rotation2D<float>(mHue * 2 * NVG_PI).matrix() * triangle;

    Matrix2f T;
    T << triangle(0,0) - triangle(0,2), triangle(0,1) - triangle(0,2),
         triangle(1,0) - triangle(1,2), triangle(1,1) - triangle(1,2);
    Vector2f pos { x - triangle(0,2), y - triangle(1,2) };

    Vector2f bary = T.colPivHouseholderQr().solve(pos);
    float l0 = bary[0], l1 = bary[1], l2 = 1 - l0 - l1;
    bool triangleTest = l0 >= 0 && l0 <= 1.f && l1 >= 0.f && l1 <= 1.f &&
                        l2 >= 0.f && l2 <= 1.f;

    if ((consideredRegions & InnerTriangle) &&
        (triangleTest || consideredRegions == InnerTriangle)) {
        if (!(consideredRegions & InnerTriangle))
            return None;
        l0 = std::min(std::max(0.f, l0), 1.f);
        l1 = std::min(std::max(0.f, l1), 1.f);
        l2 = std::min(std::max(0.f, l2), 1.f);
        float sum = l0 + l1 + l2;
        l0 /= sum;
        l1 /= sum;
        mWhite = l0;
        mBlack = l1;
        if (mCallback)
            mCallback(color());
        return InnerTriangle;
    } */

    return None;
}

Color ColorWheel::hue2rgb(float h) const {
    float s = 1., v = 1.;

    if (h < 0) h += 1;

    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    float r = 0, g = 0, b = 0;
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    return { r, g, b, 1.f };
}

Color ColorWheel::color() const {
    Color rgb    = hue2rgb(mHue);
    Color black  { 0.f, 0.f, 0.f, 1.f };
    Color white  { 1.f, 1.f, 1.f, 1.f };
    return rgb * (1 - mWhite - mBlack) + black * mBlack + white * mWhite;
}

void ColorWheel::setColor(const Color &rgb) {
    float r = rgb.r(), g = rgb.g(), b = rgb.b();

    float max = std::max(std::max(r, g), b);
    float min = std::min(std::min(r, g), b);
    float l = (max + min) / 2;

    if (max == min) {
        mHue = 0.;
        mBlack = 1. - l;
        mWhite = l;
    } else {
        float d = max - min, h;
        /* float s = l > 0.5 ? d / (2 - max - min) : d / (max + min); */
        if (max == r)
            h = (g - b) / d + (g < b ? 6 : 0);
        else if (max == g)
            h = (b - r) / d + 2;
        else
            h = (r - g) / d + 4;
        h /= 6;

        mHue = h;

       /* Eigen::Matrix<float, 4, 3> M;
        M.topLeftCorner<3, 1>() = hue2rgb(h).head<3>();
        M(3, 0) = 1.;
        M.col(1) = Vector4f{ 0., 0., 0., 1. };
        M.col(2) = Vector4f{ 1., 1., 1., 1. };

        Vector4f rgb4 = rgb.withAlpha(1);
        Vector3f bary = M.colPivHouseholderQr().solve(rgb4);

        mBlack = bary[1];
        mWhite = bary[2];*/
    }
}

/****************************************** ColorPicker ********************************/

ColorPicker::ColorPicker(Widget *parent, const Color& color) : PopupButton(parent, "") {
    setBackgroundColor(color);
    Popup *popup = this->popup();
    popup->setLayout(new GroupLayout());

    mColorWheel = new ColorWheel(popup);
    mPickButton = new Button(popup, "Pick");
    mPickButton->setFixedSize(Vector2i(100, 25));

    PopupButton::setChangeCallback([&](bool) {
        setColor(backgroundColor());
        mCallback(backgroundColor());
    });

    mColorWheel->setCallback([&](const Color &value) {
        mPickButton->setBackgroundColor(value);
        mPickButton->setTextColor(value.contrastingColor());
        mCallback(value);
    });

    mPickButton->setCallback([&]() {
        Color value = mColorWheel->color();
        setPushed(false);
        setColor(value);
        mCallback(value);
    });
}

Color ColorPicker::color() const {
    return backgroundColor();
}

void ColorPicker::setColor(const Color& color) {
    /* Ignore setColor() calls when the user is currently editing */
    if (!mPushed) {
        Color fg = color.contrastingColor();
        setBackgroundColor(color);
        setTextColor(fg);
        mColorWheel->setColor(color);
        mPickButton->setBackgroundColor(color);
        mPickButton->setTextColor(fg);
    }
}

/********************************** ImageView ***************************/

ImageView::ImageView(Widget *parent, int img, SizePolicy policy)
    : Widget(parent), mImage(img), mPolicy(policy) {}

Vector2i ImageView::preferredSize(NVGcontext *ctx) const {
    if (!mImage)
        return Vector2i(0, 0);
    int w,h;
    nvgImageSize(ctx, mImage, &w, &h);
    return Vector2i(w, h);
}

void ImageView::draw(NVGcontext* ctx) {
    if (!mImage)
        return;
    Vector2i p = mPos;
    Vector2i s = Widget::size();

    int w, h;
    nvgImageSize(ctx, mImage, &w, &h);

    if (mPolicy == SizePolicy::Fixed) {
        if (s.x() < w) {
            h = (int) std::round(h * (float) s.x() / w);
            w = s.x();
        }

        if (s.y() < h) {
            w = (int) std::round(w * (float) s.y() / h);
            h = s.y();
        }
    } else {    // mPolicy == Expand
        // expand to width
        h = (int) std::round(h * (float) s.x() / w);
        w = s.x();

        // shrink to height, if necessary
        if (s.y() < h) {
            w = (int) std::round(w * (float) s.y() / h);
            h = s.y();
        }
    }

    NVGpaint imgPaint = nvgImagePattern(ctx, p.x(), p.y(), w, h, 0, mImage, 1.0);

    nvgBeginPath(ctx);
    nvgRect(ctx, p.x(), p.y(), w, h);
    nvgFillPaint(ctx, imgPaint);
    nvgFill(ctx);
}

/**************************************** ImagePanel ********************************/

ImagePanel::ImagePanel(Widget *parent)
    : Widget(parent), mThumbSize(64), mSpacing(10), mMargin(10),
      mMouseIndex(-1) {}

Vector2i ImagePanel::gridSize() const
{
    int nCols = 1 + std::max(0,
        (int) ((mSize.x() - 2 * mMargin - mThumbSize) /
        (float) (mThumbSize + mSpacing)));
    int nRows = ((int) mImages.size() + nCols - 1) / nCols;
    return Vector2i(nCols, nRows);
}

int ImagePanel::indexForPosition(const Vector2i &p) const {
    Vector2f pp = (p.cast<float>() - Vector2f::Constant(mMargin)) /
                  (float)(mThumbSize + mSpacing);
    float iconRegion = mThumbSize / (float)(mThumbSize + mSpacing);
    bool overImage = pp.x() - std::floor(pp.x()) < iconRegion &&
                    pp.y() - std::floor(pp.y()) < iconRegion;
    Vector2i gridPos = pp.cast<int>(), grid = gridSize();
    overImage &= ((gridPos.x() >= 0 && gridPos.y() >= 0) &&
                  (gridPos.x() < grid.x() && gridPos.y() < grid.y()));
    return overImage ? (gridPos.x() + gridPos.y() * grid.x()) : -1;
}

bool ImagePanel::mouseMotionEvent(const Vector2i &p, const Vector2i & /* rel */,
                              int /* button */, int /* modifiers */) {
    mMouseIndex = indexForPosition(p);
    return true;
}

bool ImagePanel::mouseButtonEvent(const Vector2i &p, int /* button */, bool down,
                                  int /* modifiers */) {
    int index = indexForPosition(p);
    if (index >= 0 && mCallback && down)
        mCallback(index);
    return true;
}

Vector2i ImagePanel::preferredSize(NVGcontext *) const {
    Vector2i grid = gridSize();
    return Vector2i(
        grid.x() * mThumbSize + (grid.x() - 1) * mSpacing + 2*mMargin,
        grid.y() * mThumbSize + (grid.y() - 1) * mSpacing + 2*mMargin
    );
}

void ImagePanel::draw(NVGcontext* ctx) {
    Vector2i grid = gridSize();

    for (size_t i=0; i<mImages.size(); ++i) {
        Vector2i p = mPos + Vector2i::Constant(mMargin) +
            Vector2i((int) i % grid.x(), (int) i / grid.x()) * (mThumbSize + mSpacing);
        int imgw, imgh;

        nvgImageSize(ctx, mImages[i].first, &imgw, &imgh);
        float iw, ih, ix, iy;
        if (imgw < imgh) {
            iw = mThumbSize;
            ih = iw * (float)imgh / (float)imgw;
            ix = 0;
            iy = -(ih - mThumbSize) * 0.5f;
        } else {
            ih = mThumbSize;
            iw = ih * (float)imgw / (float)imgh;
            ix = -(iw - mThumbSize) * 0.5f;
            iy = 0;
        }

        NVGpaint imgPaint = nvgImagePattern(
            ctx, p.x() + ix, p.y()+ iy, iw, ih, 0, mImages[i].first,
            mMouseIndex == (int)i ? 1.0 : 0.7);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, p.x(), p.y(), mThumbSize, mThumbSize, 5);
        nvgFillPaint(ctx, imgPaint);
        nvgFill(ctx);

        NVGpaint shadowPaint =
            nvgBoxGradient(ctx, p.x() - 1, p.y(), mThumbSize + 2, mThumbSize + 2, 5, 3,
                           nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
        nvgBeginPath(ctx);
        nvgRect(ctx, p.x()-5,p.y()-5, mThumbSize+10,mThumbSize+10);
        nvgRoundedRect(ctx, p.x(),p.y(), mThumbSize,mThumbSize, 6);
        nvgPathWinding(ctx, NVG_HOLE);
        nvgFillPaint(ctx, shadowPaint);
        nvgFill(ctx);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, p.x()+0.5f,p.y()+0.5f, mThumbSize-1,mThumbSize-1, 4-0.5f);
        nvgStrokeWidth(ctx, 1.0f);
        nvgStrokeColor(ctx, nvgRGBA(255,255,255,80));
        nvgStroke(ctx);
    }
}

/*********************************** Layouts *********************************************/

BoxLayout::BoxLayout(Orientation orientation, Alignment alignment,
          int margin, int spacing)
    : mOrientation(orientation), mAlignment(alignment), mMargin(margin),
      mSpacing(spacing) {
}

Vector2i BoxLayout::preferredSize(NVGcontext *ctx, const Widget *widget) const {
    Vector2i size = Vector2i::Constant(2*mMargin);

    if (dynamic_cast<const Window *>(widget))
        size.seq(1) += widget->theme()->mWindowHeaderHeight - mMargin/2;

    bool first = true;
    int axis1 = (int) mOrientation, axis2 = ((int) mOrientation + 1)%2;
    for (auto w : widget->children()) {
        if (!w->visible())
            continue;
        if (first)
            first = false;
        else
            size.seq(axis1) += mSpacing;

        Vector2i ps = w->preferredSize(ctx), fs = w->fixedSize();
        Vector2i targetSize(
            fs.x() ? fs.x() : ps.x(),
            fs.y() ? fs.y() : ps.y()
        );

        size.seq(axis1) += targetSize.seq(axis1);
        size.seq(axis2) = std::max(size.seq(axis2), targetSize.seq(axis2) + 2*mMargin);
        first = false;
    }
    return size;
}

void BoxLayout::performLayout(NVGcontext *ctx, Widget *widget) const {
    Vector2i fs_w = widget->fixedSize();
    Vector2i containerSize(
        fs_w.x() ? fs_w.x() : widget->width(),
        fs_w.y() ? fs_w.y() : widget->height()
    );

    int axis1 = (int) mOrientation, axis2 = ((int) mOrientation + 1)%2;
    int position = mMargin;

    if (dynamic_cast<Window *>(widget))
        position += widget->theme()->mWindowHeaderHeight - mMargin/2;

    bool first = true;
    for (auto w : widget->children()) {
        if (!w->visible())
            continue;
        if (first)
            first = false;
        else
            position += mSpacing;

        Vector2i ps = w->preferredSize(ctx), fs = w->fixedSize();
        Vector2i targetSize(
            fs.x() ? fs.x() : ps.x(),
            fs.y() ? fs.y() : ps.y()
        );
        Vector2i pos = Vector2i::Zero();
        pos.seq(axis1) = position;

        switch (mAlignment) {
            case Alignment::Minimum:
                pos.seq(axis2) = mMargin;
                break;
            case Alignment::Middle:
                pos.seq(axis2) = (containerSize.seq(axis2) - targetSize.seq(axis2)) / 2;
                break;
            case Alignment::Maximum:
                pos.seq(axis2) = containerSize.seq(axis2) - targetSize.seq(axis2) - mMargin;
                break;
            case Alignment::Fill:
                pos.seq(axis2) = mMargin;
                targetSize.seq(axis2) = fs.seq(axis2) ? fs.seq(axis2) : containerSize.seq(axis2);
                break;
        }

        w->setPosition(pos);
        w->setSize(targetSize);
        w->performLayout(ctx);
        position += targetSize.seq(axis1);
    }
}

Vector2i GroupLayout::preferredSize(NVGcontext *ctx, const Widget *widget) const {
    int height = mMargin, width = 2*mMargin;

    const Window *window = dynamic_cast<const Window *>(widget);
    if (window && !window->title().empty())
        height += widget->theme()->mWindowHeaderHeight - mMargin/2;

    bool first = true, indent = false;
    for (auto c : widget->children()) {
        if (!c->visible())
            continue;
        const Label *label = dynamic_cast<const Label *>(c);
        if (!first)
            height += (label == nullptr) ? mSpacing : mGroupSpacing;
        first = false;

        Vector2i ps = c->preferredSize(ctx), fs = c->fixedSize();
        Vector2i targetSize(
            fs.x() ? fs.x() : ps.x(),
            fs.y() ? fs.y() : ps.y()
        );

        bool indentCur = indent && label == nullptr;
        height += targetSize.y();
        width = std::max(width, targetSize.x() + 2*mMargin + (indentCur ? mGroupIndent : 0));

        if (label)
            indent = !label->caption().empty();
    }
    height += mMargin;
    return Vector2i(width, height);
}

void GroupLayout::performLayout(NVGcontext *ctx, Widget *widget) const {
    int height = mMargin, availableWidth =
        (widget->fixedWidth() ? widget->fixedWidth() : widget->width()) - 2*mMargin;

    const Window *window = dynamic_cast<const Window *>(widget);
    if (window && !window->title().empty())
        height += widget->theme()->mWindowHeaderHeight - mMargin/2;

    bool first = true, indent = false;
    for (auto c : widget->children()) {
        if (!c->visible())
            continue;
        const Label *label = dynamic_cast<const Label *>(c);
        if (!first)
            height += (label == nullptr) ? mSpacing : mGroupSpacing;
        first = false;

        bool indentCur = indent && label == nullptr;
        Vector2i ps = Vector2i(availableWidth - (indentCur ? mGroupIndent : 0),
                               c->preferredSize(ctx).y());
        Vector2i fs = c->fixedSize();

        Vector2i targetSize(
            fs.x() ? fs.x() : ps.x(),
            fs.y() ? fs.y() : ps.y()
        );

        c->setPosition(Vector2i(mMargin + (indentCur ? mGroupIndent : 0), height));
        c->setSize(targetSize);
        c->performLayout(ctx);

        height += targetSize.y();

        if (label)
            indent = !label->caption().empty();
    }
}

Vector2i GridLayout::preferredSize(NVGcontext *ctx,
                                   const Widget *widget) const {
    /* Compute minimum row / column sizes */
    std::vector<int> grid[2];
    computeLayout(ctx, widget, grid);

    Vector2i size(
        2*mMargin + std::accumulate(grid[0].begin(), grid[0].end(), 0)
         + std::max((int) grid[0].size() - 1, 0) * mSpacing.x(),
        2*mMargin + std::accumulate(grid[1].begin(), grid[1].end(), 0)
         + std::max((int) grid[1].size() - 1, 0) * mSpacing.y()
    );

    if (dynamic_cast<const Window *>(widget))
        size.ry() += widget->theme()->mWindowHeaderHeight - mMargin/2;

    return size;
}

void GridLayout::computeLayout(NVGcontext *ctx, const Widget *widget, std::vector<int> *grid) const {
    int axis1 = (int) mOrientation, axis2 = (axis1 + 1) % 2;
    size_t numChildren = widget->children().size(), visibleChildren = 0;
    for (auto w : widget->children())
        visibleChildren += w->visible() ? 1 : 0;

    Vector2i dim;
    dim.seq(axis1) = mResolution;
    dim.seq(axis2) = (int) ((visibleChildren + mResolution - 1) / mResolution);

    grid[axis1].clear(); grid[axis1].resize(dim.seq(axis1), 0);
    grid[axis2].clear(); grid[axis2].resize(dim.seq(axis2), 0);

    size_t child = 0;
    for (int i2 = 0; i2 < dim.seq(axis2); i2++) {
        for (int i1 = 0; i1 < dim.seq(axis1); i1++) {
            Widget *w = nullptr;
            do {
                if (child >= numChildren)
                    return;
                w = widget->children()[child++];
            } while (!w->visible());

            Vector2i ps = w->preferredSize(ctx);
            Vector2i fs = w->fixedSize();
            Vector2i targetSize(
                fs.x() ? fs.x() : ps.x(),
                fs.y() ? fs.y() : ps.y()
            );

            grid[axis1][i1] = std::max(grid[axis1][i1], targetSize.seq(axis1));
            grid[axis2][i2] = std::max(grid[axis2][i2], targetSize.seq(axis2));
        }
    }
}

void GridLayout::performLayout(NVGcontext *ctx, Widget *widget) const {
    Vector2i fs_w = widget->fixedSize();
    Vector2i containerSize(
        fs_w.x() ? fs_w.x() : widget->width(),
        fs_w.y() ? fs_w.y() : widget->height()
    );

    /* Compute minimum row / column sizes */
    std::vector<int> grid[2];
    computeLayout(ctx, widget, grid);
    int dim[2] = { (int) grid[0].size(), (int) grid[1].size() };

    Vector2i extra = Vector2i::Zero();
    if (dynamic_cast<Window *>(widget))
        extra.ry() += widget->theme()->mWindowHeaderHeight - mMargin / 2;

    /* Strech to size provided by \c widget */
    for (int i = 0; i < 2; i++) {
        int gridSize = 2 * mMargin + extra.seq(i);
        for (int s : grid[i]) {
            gridSize += s;
            if (i+1 < dim[i])
                gridSize += mSpacing.seq(i);
        }

        if (gridSize < containerSize.seq(i)) {
            /* Re-distribute remaining space evenly */
            int gap = containerSize.seq(i) - gridSize;
            int g = gap / dim[i];
            int rest = gap - g * dim[i];
            for (int j = 0; j < dim[i]; ++j)
                grid[i][j] += g;
            for (int j = 0; rest > 0 && j < dim[i]; --rest, ++j)
                grid[i][j] += 1;
        }
    }

    int axis1 = (int) mOrientation, axis2 = (axis1 + 1) % 2;
    Vector2i start = Vector2i::Constant(mMargin) + extra;

    size_t numChildren = widget->children().size();
    size_t child = 0;

    Vector2i pos = start;
    for (int i2 = 0; i2 < dim[axis2]; i2++) {
        pos.seq(axis1) = start.seq(axis1);
        for (int i1 = 0; i1 < dim[axis1]; i1++) {
            Widget *w = nullptr;
            do {
                if (child >= numChildren)
                    return;
                w = widget->children()[child++];
            } while (!w->visible());

            Vector2i ps = w->preferredSize(ctx);
            Vector2i fs = w->fixedSize();
            Vector2i targetSize(
                fs.x() ? fs.x() : ps.x(),
                fs.y() ? fs.y() : ps.y()
            );

            Vector2i itemPos(pos);
            for (int j = 0; j < 2; j++) {
                int axis = (axis1 + j) % 2;
                int item = j == 0 ? i1 : i2;
                Alignment align = alignment(axis, item);

                switch (align) {
                    case Alignment::Minimum:
                        break;
                    case Alignment::Middle:
                        itemPos.seq(axis) += (grid[axis][item] - targetSize.seq(axis)) / 2;
                        break;
                    case Alignment::Maximum:
                        itemPos.seq(axis) += grid[axis][item] - targetSize.seq(axis);
                        break;
                    case Alignment::Fill:
                        targetSize.seq(axis) = fs.seq(axis) ? fs.seq(axis) : grid[axis][item];
                        break;
                }
            }
            w->setPosition(itemPos);
            w->setSize(targetSize);
            w->performLayout(ctx);
            pos.seq(axis1) += grid[axis1][i1] + mSpacing.seq(axis1);
        }
        pos.seq(axis2) += grid[axis2][i2] + mSpacing.seq(axis2);
    }
}

AdvancedGridLayout::AdvancedGridLayout(const std::vector<int> &cols, const std::vector<int> &rows)
 : mCols(cols), mRows(rows) {
    mColStretch.resize(mCols.size(), 0);
    mRowStretch.resize(mRows.size(), 0);
}

Vector2i AdvancedGridLayout::preferredSize(NVGcontext *ctx, const Widget *widget) const {
    /* Compute minimum row / column sizes */
    std::vector<int> grid[2];
    computeLayout(ctx, widget, grid);

    Vector2i size(
        std::accumulate(grid[0].begin(), grid[0].end(), 0),
        std::accumulate(grid[1].begin(), grid[1].end(), 0));

    Vector2i extra = Vector2i::Constant(2 * mMargin);
    if (dynamic_cast<const Window *>(widget))
        extra.ry() += widget->theme()->mWindowHeaderHeight - mMargin/2;

    return size+extra;
}

void AdvancedGridLayout::performLayout(NVGcontext *ctx, Widget *widget) const {
    std::vector<int> grid[2];
    computeLayout(ctx, widget, grid);

    grid[0].insert(grid[0].begin(), mMargin);
    if (dynamic_cast<const Window *>(widget))
        grid[1].insert(grid[1].begin(), widget->theme()->mWindowHeaderHeight + mMargin/2);
    else
        grid[1].insert(grid[1].begin(), mMargin);

    for (int axis=0; axis<2; ++axis) {
        for (size_t i=1; i<grid[axis].size(); ++i)
            grid[axis][i] += grid[axis][i-1];

        for (Widget *w : widget->children()) {
            Anchor anchor = this->anchor(w);
            if (!w->visible())
                continue;

            int itemPos = grid[axis][anchor.pos[axis]];
            int cellSize  = grid[axis][anchor.pos[axis] + anchor.size[axis]] - itemPos;
            int ps = w->preferredSize(ctx).seq(axis), fs = w->fixedSize().seq(axis);
            int targetSize = fs ? fs : ps;

            switch (anchor.align[axis]) {
                case Alignment::Minimum:
                    break;
                case Alignment::Middle:
                    itemPos += (cellSize - targetSize) / 2;
                    break;
                case Alignment::Maximum:
                    itemPos += cellSize - targetSize;
                    break;
                case Alignment::Fill:
                    targetSize = fs ? fs : cellSize;
                    break;
            }

            Vector2i pos = w->position(), size = w->size();
            pos.seq(axis) = itemPos;
            size.seq(axis) = targetSize;
            w->setPosition(pos);
            w->setSize(size);
            w->performLayout(ctx);
        }
    }
}

void AdvancedGridLayout::computeLayout(NVGcontext *ctx, const Widget *widget,
                                       std::vector<int> *_grid) const {
    Vector2i fs_w = widget->fixedSize();
    Vector2i containerSize(
        fs_w.x() ? fs_w.x() : widget->width(),
        fs_w.y() ? fs_w.y() : widget->height()
    );

    Vector2i extra = Vector2i::Constant(2 * mMargin);
    if (dynamic_cast<const Window *>(widget))
        extra.ry() += widget->theme()->mWindowHeaderHeight - mMargin/2;

    containerSize -= extra;

    for (int axis=0; axis<2; ++axis) {
        std::vector<int> &grid = _grid[axis];
        const std::vector<int> &sizes = axis == 0 ? mCols : mRows;
        const std::vector<float> &stretch = axis == 0 ? mColStretch : mRowStretch;
        grid = sizes;

        for (int phase = 0; phase < 2; ++phase) {
            for (auto pair : mAnchor) {
                const Widget *w = pair.first;
                if (!w->visible())
                    continue;
                const Anchor &anchor = pair.second;
                if ((anchor.size[axis] == 1) != (phase == 0))
                    continue;
                int ps = w->preferredSize(ctx).seq(axis), fs = w->fixedSize().seq(axis);
                int targetSize = fs ? fs : ps;

                if (anchor.pos[axis] + anchor.size[axis] > (int) grid.size())
                    throw std::runtime_error(
                        "Advanced grid layout: widget is out of bounds: " +
                        (std::string) anchor);

                int currentSize = 0;
                float totalStretch = 0;
                for (int i = anchor.pos[axis];
                     i < anchor.pos[axis] + anchor.size[axis]; ++i) {
                    if (sizes[i] == 0 && anchor.size[axis] == 1)
                        grid[i] = std::max(grid[i], targetSize);
                    currentSize += grid[i];
                    totalStretch += stretch[i];
                }
                if (targetSize <= currentSize)
                    continue;
                if (totalStretch == 0)
                    throw std::runtime_error(
                        "Advanced grid layout: no space to place widget: " +
                        (std::string) anchor);
                float amt = (targetSize - currentSize) / totalStretch;
                for (int i = anchor.pos[axis];
                     i < anchor.pos[axis] + anchor.size[axis]; ++i) {
                    grid[i] += (int) std::round(amt * stretch[i]);
                }
            }
        }

        int currentSize = std::accumulate(grid.begin(), grid.end(), 0);
        float totalStretch = std::accumulate(stretch.begin(), stretch.end(), 0.0f);
        if (currentSize >= containerSize.seq(axis) || totalStretch == 0)
            continue;
        float amt = (containerSize.seq(axis) - currentSize) / totalStretch;
        for (size_t i = 0; i<grid.size(); ++i)
            grid[i] += (int) std::round(amt * stretch[i]);
    }
}


/*************************************** Graph **************************************/

Graph::Graph(Widget *parent, const std::string &caption)
    : Widget(parent), mCaption(caption) {
    mBackgroundColor = Color(20, 128);
    mForegroundColor = Color(255, 192, 0, 128);
    mTextColor = Color(240, 192);
}

Vector2i Graph::preferredSize(NVGcontext *) const {
    return Vector2i(180, 45);
}

void Graph::draw(NVGcontext *ctx) {
    Widget::draw(ctx);

    nvgBeginPath(ctx);
    nvgRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y());
    nvgFillColor(ctx, mBackgroundColor);
    nvgFill(ctx);

    if (mValues.size() < 2)
        return;

    nvgBeginPath(ctx);
    nvgMoveTo(ctx, mPos.x(), mPos.y()+mSize.y());
    for (size_t i = 0; i < (size_t) mValues.size(); i++) {
        float value = mValues[i];
        float vx = mPos.x() + i * mSize.x() / (float) (mValues.size() - 1);
        float vy = mPos.y() + (1-value) * mSize.y();
        nvgLineTo(ctx, vx, vy);
    }

    nvgLineTo(ctx, mPos.x() + mSize.x(), mPos.y() + mSize.y());
    nvgStrokeColor(ctx, Color(100, 255));
    nvgStroke(ctx);
    nvgFillColor(ctx, mForegroundColor);
    nvgFill(ctx);

    nvgFontFace(ctx, "sans");

    if (!mCaption.empty()) {
        nvgFontSize(ctx, 14.0f);
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFillColor(ctx, mTextColor);
        nvgText(ctx, mPos.x() + 3, mPos.y() + 1, mCaption.c_str(), NULL);
    }

    if (!mHeader.empty()) {
        nvgFontSize(ctx, 18.0f);
        nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
        nvgFillColor(ctx, mTextColor);
        nvgText(ctx, mPos.x() + mSize.x() - 3, mPos.y() + 1, mHeader.c_str(), NULL);
    }

    if (!mFooter.empty()) {
        nvgFontSize(ctx, 15.0f);
        nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
        nvgFillColor(ctx, mTextColor);
        nvgText(ctx, mPos.x() + mSize.x() - 3, mPos.y() + mSize.y() - 1, mFooter.c_str(), NULL);
    }

    nvgBeginPath(ctx);
    nvgRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y());
    nvgStrokeColor(ctx, Color(100, 255));
    nvgStroke(ctx);
}

/**************************************** Theme *****************************************/

Theme::Theme(NVGcontext *ctx) {
    mStandardFontSize                 = 16;
    mButtonFontSize                   = 20;
    mTextBoxFontSize                  = 20;
    mWindowCornerRadius               = 2;
    mWindowHeaderHeight               = 30;
    mWindowDropShadowSize             = 10;
    mButtonCornerRadius               = 2;

    mDropShadow                       = Color(0, 128);
    mTransparent                      = Color(0, 0);
    mBorderDark                       = Color(29, 255);
    mBorderLight                      = Color(92, 255);
    mBorderMedium                     = Color(35, 255);
    mTextColor                        = Color(255, 160);
    mDisabledTextColor                = Color(255, 80);
    mTextColorShadow                  = Color(0, 160);
    mIconColor                        = mTextColor;

    mButtonGradientTopFocused         = Color(64, 255);
    mButtonGradientBotFocused         = Color(48, 255);
    mButtonGradientTopUnfocused       = Color(74, 255);
    mButtonGradientBotUnfocused       = Color(58, 255);
    mButtonGradientTopPushed          = Color(41, 255);
    mButtonGradientBotPushed          = Color(29, 255);

    /* Window-related */
    mWindowFillUnfocused              = Color(43, 230);
    mWindowFillFocused                = Color(45, 230);
    mWindowTitleUnfocused             = Color(220, 160);
    mWindowTitleFocused               = Color(255, 190);

    mWindowHeaderGradientTop          = mButtonGradientTopUnfocused;
    mWindowHeaderGradientBot          = mButtonGradientBotUnfocused;
    mWindowHeaderSepTop               = mBorderLight;
    mWindowHeaderSepBot               = mBorderDark;

    mWindowPopup                      = Color(50, 255);
    mWindowPopupTransparent           = Color(50, 0);

    mFontNormal = nvgCreateFontMem(ctx, "sans", roboto_regular_ttf,
                                   roboto_regular_ttf_size, 0);
    mFontBold = nvgCreateFontMem(ctx, "sans-bold", roboto_bold_ttf,
                                 roboto_bold_ttf_size, 0);
    mFontIcons = nvgCreateFontMem(ctx, "icons", entypo_ttf,
                                  entypo_ttf_size, 0);
    if (mFontNormal == -1 || mFontBold == -1 || mFontIcons == -1)
        throw std::runtime_error("Could not load fonts!");
}

/**************************** Vscrollpanel *******************************************/

VScrollPanel::VScrollPanel(Widget *parent)
    : Widget(parent), mChildPreferredHeight(0), mScroll(0.0f) { }

void VScrollPanel::performLayout(NVGcontext *ctx) {
    Widget::performLayout(ctx);

    if (mChildren.empty())
        return;
    Widget *child = mChildren[0];
    mChildPreferredHeight = child->preferredSize(ctx).y();
    child->setPosition(Vector2i(0, 0));
    child->setSize(Vector2i(mSize.x()-12, mChildPreferredHeight));
}

Vector2i VScrollPanel::preferredSize(NVGcontext *ctx) const {
    if (mChildren.empty())
        return Vector2i::Zero();
    return mChildren[0]->preferredSize(ctx) + Vector2i(12, 0);
}

bool VScrollPanel::mouseDragEvent(const Vector2i &, const Vector2i &rel,
                            int, int) {
    if (mChildren.empty())
        return false;

    float scrollh = height() *
        std::min(1.0f, height() / (float)mChildPreferredHeight);

    mScroll = std::max((float) 0.0f, std::min((float) 1.0f,
                 mScroll + rel.y() / (float)(mSize.y() - 8 - scrollh)));
    return true;
}

bool VScrollPanel::scrollEvent(const Vector2i &/* p */, const Vector2f &rel) {
    float scrollAmount = rel.y() * (mSize.y() / 20.0f);
    float scrollh = height() *
        std::min(1.0f, height() / (float)mChildPreferredHeight);

    mScroll = std::max((float) 0.0f, std::min((float) 1.0f,
            mScroll - scrollAmount / (float)(mSize.y() - 8 - scrollh)));
    return true;
}

bool VScrollPanel::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
    if (mChildren.empty())
        return false;
    int shift = (int) (mScroll*(mChildPreferredHeight - mSize.y()));
    return mChildren[0]->mouseButtonEvent(p + Vector2i(0, shift), button, down, modifiers);
}

bool VScrollPanel::mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) {
    if (mChildren.empty())
        return false;
    int shift = (int) (mScroll*(mChildPreferredHeight - mSize.y()));
    return mChildren[0]->mouseMotionEvent(p + Vector2i(0, shift), rel, button, modifiers);
}

void VScrollPanel::draw(NVGcontext *ctx) {
    if (mChildren.empty())
        return;
    Widget *child = mChildren[0];
    mChildPreferredHeight = child->preferredSize(ctx).y();
    float scrollh = height() *
        std::min(1.0f, height() / (float) mChildPreferredHeight);

    nvgSave(ctx);
    nvgTranslate(ctx, mPos.x(), mPos.y());
    nvgScissor(ctx, 0, 0, mSize.x(), mSize.y());
    nvgTranslate(ctx, 0, -mScroll*(mChildPreferredHeight - mSize.y()));
    if (child->visible())
        child->draw(ctx);
    nvgRestore(ctx);

    NVGpaint paint = nvgBoxGradient(
        ctx, mPos.x() + mSize.x() - 12 + 1, mPos.y() + 4 + 1, 8,
        mSize.y() - 8, 3, 4, Color(0, 32), Color(0, 92));
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x() + mSize.x() - 12, mPos.y() + 4, 8,
                   mSize.y() - 8, 3);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);

    paint = nvgBoxGradient(
        ctx, mPos.x() + mSize.x() - 12 - 1,
        mPos.y() + 4 + (mSize.y() - 8 - scrollh) * mScroll - 1, 8, scrollh,
        3, 4, Color(220, 100), Color(128, 100));

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x() + mSize.x() - 12 + 1,
                   mPos.x() + 4 + 1 + (mSize.y() - 8 - scrollh) * mScroll, 8 - 2,
                   scrollh - 2, 2);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);
}

/*********************************** Progressbar **************************************/

ProgressBar::ProgressBar(Widget *parent)
    : Widget(parent), mValue(0.0f) {}

Vector2i ProgressBar::preferredSize(NVGcontext *) const
{
    return Vector2i(70, 12);
}

void ProgressBar::draw(NVGcontext* ctx)
{
    Widget::draw(ctx);

    NVGpaint paint = nvgBoxGradient(
        ctx, mPos.x() + 1, mPos.y() + 1,
        mSize.x()-2, mSize.y(), 3, 4, Color(0, 32), Color(0, 92));
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), 3);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);

    float value = std::min(std::max(0.0f, mValue), 1.0f);
    int barPos = (int) std::round((mSize.x() - 2) * value);

    paint = nvgBoxGradient(
        ctx, mPos.x(), mPos.y(),
        barPos+1.5f, mSize.y()-1, 3, 4,
        Color(220, 100), Color(128, 100));

    nvgBeginPath(ctx);
    nvgRoundedRect(
        ctx, mPos.x()+1, mPos.y()+1,
        barPos, mSize.y()-2, 3);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);
}

/******************************** Textbox **************************************************/

TextBox::TextBox(Widget *parent,const std::string &value)
    : Widget(parent),
      mEditable(false),
      mCommitted(true),
      mValue(value),
      mDefaultValue(""),
      mAlignment(Alignment::Center),
      mUnits(""),
      mFormat(""),
      mUnitsImage(-1),
      mValidFormat(true),
      mValueTemp(value),
      mCursorPos(-1),
      mSelectionPos(-1),
      mMousePos(Vector2i(-1,-1)),
      mMouseDownPos(Vector2i(-1,-1)),
      mMouseDragPos(Vector2i(-1,-1)),
      mMouseDownModifier(0),
      mTextOffset(0),
      mLastClick(0) {
    mFontSize = mTheme->mTextBoxFontSize;
}

void TextBox::setEditable(bool editable) {
    mEditable = editable;
    setCursor(editable ? Cursor::IBeam : Cursor::Arrow);
}

Vector2i TextBox::preferredSize(NVGcontext *ctx) const {
    Vector2i size(0, fontSize() * 1.4f);

    float uw = 0;
    if (mUnitsImage > 0) {
        int w, h;
        nvgImageSize(ctx, mUnitsImage, &w, &h);
        float uh = size.y() * 0.4f;
        uw = w * uh / h;
    } else if (!mUnits.empty()) {
        uw = nvgTextBounds(ctx, 0, 0, mUnits.c_str(), nullptr, nullptr);
    }

    float ts = nvgTextBounds(ctx, 0, 0, mValue.c_str(), nullptr, nullptr);
    size.rx() = size.y() + ts + uw;
    return size;
}

void TextBox::draw(NVGcontext* ctx) {
    Widget::draw(ctx);

    NVGpaint bg = nvgBoxGradient(ctx,
        mPos.x() + 1, mPos.y() + 1 + 1.0f, mSize.x() - 2, mSize.y() - 2,
        3, 4, Color(255, 32), Color(32, 32));
    NVGpaint fg1 = nvgBoxGradient(ctx,
        mPos.x() + 1, mPos.y() + 1 + 1.0f, mSize.x() - 2, mSize.y() - 2,
        3, 4, Color(150, 32), Color(32, 32));
    NVGpaint fg2 = nvgBoxGradient(ctx,
        mPos.x() + 1, mPos.y() + 1 + 1.0f, mSize.x() - 2, mSize.y() - 2,
        3, 4, nvgRGBA(255, 0, 0, 100), nvgRGBA(255, 0, 0, 50));

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x() + 1, mPos.y() + 1 + 1.0f, mSize.x() - 2,
                   mSize.y() - 2, 3);

    if(mEditable && focused())
        mValidFormat ? nvgFillPaint(ctx, fg1) : nvgFillPaint(ctx, fg2);
    else
        nvgFillPaint(ctx, bg);

    nvgFill(ctx);

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x() + 0.5f, mPos.y() + 0.5f, mSize.x() - 1,
                   mSize.y() - 1, 2.5f);
    nvgStrokeColor(ctx, Color(0, 48));
    nvgStroke(ctx);

    nvgFontSize(ctx, fontSize());
    nvgFontFace(ctx, "sans");
    Vector2i drawPos(mPos.x(), mPos.y() + mSize.y() * 0.5f + 1);

    float xSpacing = mSize.y() * 0.3f;

    float unitWidth = 0;

    if (mUnitsImage > 0) {
        int w, h;
        nvgImageSize(ctx, mUnitsImage, &w, &h);
        float unitHeight = mSize.y() * 0.4f;
        unitWidth = w * unitHeight / h;
        NVGpaint imgPaint = nvgImagePattern(
            ctx, mPos.x() + mSize.x() - xSpacing - unitWidth,
            drawPos.y() - unitHeight * 0.5f, unitWidth, unitHeight, 0,
            mUnitsImage, mEnabled ? 0.7f : 0.35f);
        nvgBeginPath(ctx);
        nvgRect(ctx, mPos.x() + mSize.x() - xSpacing - unitWidth,
                drawPos.y() - unitHeight * 0.5f, unitWidth, unitHeight);
        nvgFillPaint(ctx, imgPaint);
        nvgFill(ctx);
        unitWidth += 2;
    } else if (!mUnits.empty()) {
        unitWidth = nvgTextBounds(ctx, 0, 0, mUnits.c_str(), nullptr, nullptr);
        nvgFillColor(ctx, Color(255, mEnabled ? 64 : 32));
        nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        nvgText(ctx, mPos.x() + mSize.x() - xSpacing, drawPos.y(),
                mUnits.c_str(), nullptr);
        unitWidth += 2;
    }

    switch (mAlignment) {
        case Alignment::Left:
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            drawPos.rx() += xSpacing;
            break;
        case Alignment::Right:
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
            drawPos.rx() += mSize.x() - unitWidth - xSpacing;
            break;
        case Alignment::Center:
            nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            drawPos.rx() += mSize.x() * 0.5f;
            break;
    }

    nvgFontSize(ctx, fontSize());
    nvgFillColor(ctx,
                 mEnabled ? mTheme->mTextColor : mTheme->mDisabledTextColor);

    // clip visible text area
    float clipX = mPos.x() + xSpacing - 1.0f;
    float clipY = mPos.y() + 1.0f;
    float clipWidth = mSize.x() - unitWidth - 2 * xSpacing + 2.0f;
    float clipHeight = mSize.y() - 3.0f;
    nvgScissor(ctx, clipX, clipY, clipWidth, clipHeight);

    Vector2i oldDrawPos(drawPos);
    drawPos.rx() += mTextOffset;

    if (mCommitted) {
        nvgText(ctx, drawPos.x(), drawPos.y(), mValue.c_str(), nullptr);
    } else {
        const int maxGlyphs = 1024;
        NVGglyphPosition glyphs[maxGlyphs];
        float textBound[4];
        nvgTextBounds(ctx, drawPos.x(), drawPos.y(), mValueTemp.c_str(),
                      nullptr, textBound);
        float lineh = textBound[3] - textBound[1];

        // find cursor positions
        int nglyphs =
            nvgTextGlyphPositions(ctx, drawPos.x(), drawPos.y(),
                                  mValueTemp.c_str(), nullptr, glyphs, maxGlyphs);
        updateCursor(ctx, textBound[2], glyphs, nglyphs);

        // compute text offset
        int prevCPos = mCursorPos > 0 ? mCursorPos - 1 : 0;
        int nextCPos = mCursorPos < nglyphs ? mCursorPos + 1 : nglyphs;
        float prevCX = cursorIndex2Position(prevCPos, textBound[2], glyphs, nglyphs);
        float nextCX = cursorIndex2Position(nextCPos, textBound[2], glyphs, nglyphs);

        if (nextCX > clipX + clipWidth)
            mTextOffset -= nextCX - (clipX + clipWidth) + 1;
        if (prevCX < clipX)
            mTextOffset += clipX - prevCX + 1;

        drawPos.rx() = oldDrawPos.x() + mTextOffset;

        // draw text with offset
        nvgText(ctx, drawPos.x(), drawPos.y(), mValueTemp.c_str(), nullptr);
        nvgTextBounds(ctx, drawPos.x(), drawPos.y(), mValueTemp.c_str(),
                      nullptr, textBound);

        // recompute cursor positions
        nglyphs = nvgTextGlyphPositions(ctx, drawPos.x(), drawPos.y(),
                mValueTemp.c_str(), nullptr, glyphs, maxGlyphs);

        if (mCursorPos > -1) {
            if (mSelectionPos > -1) {
                float caretx = cursorIndex2Position(mCursorPos, textBound[2],
                                                    glyphs, nglyphs);
                float selx = cursorIndex2Position(mSelectionPos, textBound[2],
                                                  glyphs, nglyphs);

                if (caretx > selx)
                    std::swap(caretx, selx);

                // draw selection
                nvgBeginPath(ctx);
                nvgFillColor(ctx, nvgRGBA(255, 255, 255, 80));
                nvgRect(ctx, caretx, drawPos.y() - lineh * 0.5f, selx - caretx,
                        lineh);
                nvgFill(ctx);
            }

            float caretx = cursorIndex2Position(mCursorPos, textBound[2], glyphs, nglyphs);

            // draw cursor
            nvgBeginPath(ctx);
            nvgMoveTo(ctx, caretx, drawPos.y() - lineh * 0.5f);
            nvgLineTo(ctx, caretx, drawPos.y() + lineh * 0.5f);
            nvgStrokeColor(ctx, nvgRGBA(255, 192, 0, 255));
            nvgStrokeWidth(ctx, 1.0f);
            nvgStroke(ctx);
        }
    }

    nvgResetScissor(ctx);
}

bool TextBox::mouseButtonEvent(const Vector2i &p, int button, bool down,
                               int modifiers)
{
    Widget::mouseButtonEvent(p, button, down, modifiers);

    if (mEditable && focused() && button == SDL_BUTTON_LEFT)
    {
        if (down)
        {
            mMouseDownPos = p;
            mMouseDownModifier = modifiers;

            double time = SDL_GetTicks();
            if (time - mLastClick < 0.25) {
                /* Double-click: select all text */
                mSelectionPos = 0;
                mCursorPos = (int) mValueTemp.size();
                mMouseDownPos = Vector2i(-1, 1);
            }
            mLastClick = time;
        } else {
            mMouseDownPos = Vector2i(-1, -1);
            mMouseDragPos = Vector2i(-1, -1);
        }
        return true;
    }

    return false;
}

bool TextBox::mouseMotionEvent(const Vector2i &p, const Vector2i & /* rel */,
                               int /* button */, int /* modifiers */) {
    if (mEditable && focused()) {
        mMousePos = p;
        return true;
    }
    return false;
}

bool TextBox::mouseDragEvent(const Vector2i &p, const Vector2i &/* rel */,
                             int /* button */, int /* modifiers */) {
    if (mEditable && focused()) {
        mMouseDragPos = p;
        return true;
    }
    return false;
}

bool TextBox::mouseEnterEvent(const Vector2i &p, bool enter) {
    Widget::mouseEnterEvent(p, enter);
    return false;
}

bool TextBox::focusEvent(bool focused) {
    Widget::focusEvent(focused);

    std::string backup = mValue;

    if (mEditable) {
        if (focused) {
            mValueTemp = mValue;
            mCommitted = false;
            mCursorPos = 0;
        } else {
            if (mValidFormat) {
                if (mValueTemp == "")
                    mValue = mDefaultValue;
                else
                    mValue = mValueTemp;
            }

            if (mCallback && !mCallback(mValue))
                mValue = backup;

            mValidFormat = true;
            mCommitted = true;
            mCursorPos = -1;
            mSelectionPos = -1;
            mTextOffset = 0;
        }

        mValidFormat = (mValueTemp == "") || checkFormat(mValueTemp, mFormat);
    }

    return true;
}

bool TextBox::keyboardEvent(int key, int /* scancode */, int action, int modifiers)
{
    if (mEditable && focused()) {
        if (action == SDL_KEYDOWN /*|| action == GLFW_REPEAT*/)
        {
            if (key == SDLK_LEFT )
            {
                if (modifiers == SDLK_LSHIFT)
                {
                    if (mSelectionPos == -1)
                        mSelectionPos = mCursorPos;
                } else {
                    mSelectionPos = -1;
                }

                if (mCursorPos > 0)
                    mCursorPos--;
            } else if (key == SDLK_RIGHT) {
                if (modifiers == SDLK_LSHIFT) {
                    if (mSelectionPos == -1)
                        mSelectionPos = mCursorPos;
                } else {
                    mSelectionPos = -1;
                }

                if (mCursorPos < (int) mValueTemp.length())
                    mCursorPos++;
            } else if (key == SDLK_HOME) {
                if (modifiers == SDLK_LSHIFT) {
                    if (mSelectionPos == -1)
                        mSelectionPos = mCursorPos;
                } else {
                    mSelectionPos = -1;
                }

                mCursorPos = 0;
            } else if (key == SDLK_END) {
                if (modifiers == SDLK_LSHIFT) {
                    if (mSelectionPos == -1)
                        mSelectionPos = mCursorPos;
                } else {
                    mSelectionPos = -1;
                }

                mCursorPos = (int) mValueTemp.size();
            } else if (key == SDLK_BACKSPACE) {
                if (!deleteSelection()) {
                    if (mCursorPos > 0) {
                        mValueTemp.erase(mValueTemp.begin() + mCursorPos - 1);
                        mCursorPos--;
                    }
                }
            } else if (key == SDLK_DELETE) {
                if (!deleteSelection()) {
                    if (mCursorPos < (int) mValueTemp.length())
                        mValueTemp.erase(mValueTemp.begin() + mCursorPos);
                }
            }
            else if (key == SDLK_RETURN)
            {
                if (!mCommitted)
                    focusEvent(false);
            }
            else if (key == SDLK_a && modifiers == SDLK_RCTRL)
            {
                mCursorPos = (int) mValueTemp.length();
                mSelectionPos = 0;
            }
            else if (key == SDLK_x && modifiers == SDLK_RCTRL)
            {
                copySelection();
                deleteSelection();
            }
            else if (key == SDLK_c && modifiers == SDLK_RCTRL)
            {
                copySelection();
            }
            else if (key == SDLK_v && modifiers == SDLK_RCTRL)
            {
                deleteSelection();
                pasteFromClipboard();
            }

            mValidFormat =
                (mValueTemp == "") || checkFormat(mValueTemp, mFormat);
        }

        return true;
    }

    return false;
}

bool TextBox::keyboardCharacterEvent(unsigned int codepoint) {
    if (mEditable && focused()) {
        std::ostringstream convert;
        convert << (char) codepoint;

        deleteSelection();
        mValueTemp.insert(mCursorPos, convert.str());
        mCursorPos++;

        mValidFormat = (mValueTemp == "") || checkFormat(mValueTemp, mFormat);

        return true;
    }

    return false;
}

bool TextBox::checkFormat(const std::string &input, const std::string &format) {
    if (format.empty())
        return true;
    std::regex regex(format);
    return regex_match(input, regex);
}

bool TextBox::copySelection()
{
    if (mSelectionPos > -1) {
        Screen *sc = dynamic_cast<Screen *>(this->window()->parent());

        int begin = mCursorPos;
        int end = mSelectionPos;

        if (begin > end)
            std::swap(begin, end);

        SDL_SetClipboardText( mValueTemp.substr(begin, end).c_str());
        return true;
    }

    return false;
}

void TextBox::pasteFromClipboard()
{
    Screen *sc = dynamic_cast<Screen *>(this->window()->parent());
    std::string str( SDL_GetClipboardText() );
    mValueTemp.insert(mCursorPos, str);
}

bool TextBox::deleteSelection()
{
    if (mSelectionPos > -1) {
        int begin = mCursorPos;
        int end = mSelectionPos;

        if (begin > end)
            std::swap(begin, end);

        if (begin == end - 1)
            mValueTemp.erase(mValueTemp.begin() + begin);
        else
            mValueTemp.erase(mValueTemp.begin() + begin,
                             mValueTemp.begin() + end);

        mCursorPos = begin;
        mSelectionPos = -1;
        return true;
    }

    return false;
}

void TextBox::updateCursor(NVGcontext *, float lastx,
                           const NVGglyphPosition *glyphs, int size)
                           {
    // handle mouse cursor events
    if (mMouseDownPos.x() != -1) {
        if (mMouseDownModifier == SDLK_LSHIFT) {
            if (mSelectionPos == -1)
                mSelectionPos = mCursorPos;
        } else
            mSelectionPos = -1;

        mCursorPos =
            position2CursorIndex(mMouseDownPos.x(), lastx, glyphs, size);

        mMouseDownPos = Vector2i(-1, -1);
    } else if (mMouseDragPos.x() != -1) {
        if (mSelectionPos == -1)
            mSelectionPos = mCursorPos;

        mCursorPos =
            position2CursorIndex(mMouseDragPos.x(), lastx, glyphs, size);
    } else {
        // set cursor to last character
        if (mCursorPos == -2)
            mCursorPos = size;
    }

    if (mCursorPos == mSelectionPos)
        mSelectionPos = -1;
}

float TextBox::cursorIndex2Position(int index, float lastx,
                                    const NVGglyphPosition *glyphs, int size) {
    float pos = 0;
    if (index == size)
        pos = lastx; // last character
    else
        pos = glyphs[index].x;

    return pos;
}

int TextBox::position2CursorIndex(float posx, float lastx,
                                  const NVGglyphPosition *glyphs, int size) {
    int mCursorId = 0;
    float caretx = glyphs[mCursorId].x;
    for (int j = 1; j < size; j++) {
        if (std::abs(caretx - posx) > std::abs(glyphs[j].x - posx)) {
            mCursorId = j;
            caretx = glyphs[mCursorId].x;
        }
    }
    if (std::abs(caretx - posx) > std::abs(lastx - posx))
        mCursorId = size;

    return mCursorId;
}


NAMESPACE_END(nanogui)
