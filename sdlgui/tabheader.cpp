/*
    sdlgui/tabheader.cpp -- Widget used to control tabs.

    The tab header widget was contributed by Stefan Ivanov.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/tabheader.h>
#include <sdlgui/theme.h>
#include <sdlgui/entypo.h>
#if defined(_WIN32)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <numeric>
#include <array>

NAMESPACE_BEGIN(sdlgui)

TabHeader::TabButton::TabButton(TabHeader &header, const std::string &label)
    : 
  mHeader(&header), mLabel(label) 
{
  _labelTex.dirty = true;
}

Vector2i TabHeader::TabButton::preferredSize(SDL_Renderer *ctx) const
{
    // No need to call nvg font related functions since this is done by the tab header implementation
    int w, h;
    auto theme = const_cast<TabButton*>(this)->mHeader->theme();
    theme->getUtf8Bounds("sans", mHeader->fontSize(), mLabel.c_str(), &w, &h);
   
    int buttonWidth = w + 2 * mHeader->theme()->mTabButtonHorizontalPadding;
    int buttonHeight = h + 2 * mHeader->theme()->mTabButtonVerticalPadding;
    return Vector2i(buttonWidth, buttonHeight);
}

void TabHeader::TabButton::calculateVisibleString(SDL_Renderer *renderer) 
{
    // The size must have been set in by the enclosing tab header.
    std::string displayedText =  mHeader->theme()->breakText(renderer, mLabel.c_str(), 
                                                             "sans", mHeader->fontSize(), mSize.x-10);

    mVisibleText.first = mLabel.c_str();

    // Check to see if the text need to be truncated.
    if (displayedText.size() != mLabel.size()) 
    {
      int truncatedWidth = mHeader->theme()->getTextWidth("sans", mHeader->fontSize(), displayedText.c_str());
      int dotsWidth = mHeader->theme()->getTextWidth("sans", mHeader->fontSize(), dots);
      
      // Remember the truncated width to know where to display the dots.
      mVisibleWidth = truncatedWidth;
      mVisibleText.last = mLabel.c_str() + displayedText.size();
    } 
    else 
    {
        mVisibleText.last = nullptr;
        mVisibleWidth = 0;
    }
}

void TabHeader::TabButton::drawAtPosition(SDL_Renderer *renderer, const Vector2i& position, bool active)
{
    int xPos = position.x;
    int yPos = position.y;
    int width = mSize.x;
    int height = mSize.y;
    auto theme = mHeader->theme();

    int lx = mHeader->getAbsoluteLeft();
    int ly = mHeader->getAbsoluteTop();

    //nvgSave(ctx);
    //nvgIntersectScissor(ctx, xPos, yPos, width+1, height);
    if (!active) 
    {
        // Background gradients
        Color gradTop = theme->mButtonGradientTopPushed;
        Color gradBot = theme->mButtonGradientBotPushed;

        // Draw the background.
        //nvgBeginPath(ctx);
        SDL_Rect trect{ lx + xPos + 1, ly + yPos + 1, width - 1, height - 1 };
        SDL_Color b = gradTop.toSdlColor();
        SDL_Color bt = gradBot.toSdlColor();

        SDL_SetRenderDrawColor(renderer, b.r, b.g, b.b, b.a);
        SDL_RenderFillRect(renderer, &trect);
    }

    if (active) 
    {
      SDL_Color bl = theme->mBorderLight.toSdlColor();
      SDL_Rect blr{ lx + xPos + 1, ly + yPos + 2, width, height };

      SDL_SetRenderDrawColor(renderer, bl.r, bl.g, bl.b, bl.a);
      SDL_RenderDrawLine(renderer, blr.x, blr.y, blr.x, blr.y + blr.h);
      SDL_RenderDrawLine(renderer, blr.x, blr.y, blr.x + blr.w, blr.y);
      SDL_RenderDrawLine(renderer, blr.x+blr.w, blr.y, blr.x + blr.w, blr.y + blr.h);

      SDL_Color bd = theme->mBorderDark.toSdlColor();
      SDL_Rect bdr{ lx + xPos + 1, ly + yPos + 1, width, height };
      
      SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
      SDL_RenderDrawLine(renderer, bdr.x, bdr.y, bdr.x, bdr.y + bdr.h);
      SDL_RenderDrawLine(renderer, bdr.x, bdr.y, bdr.x + bdr.w, bdr.y);
      SDL_RenderDrawLine(renderer, bdr.x + bdr.w, bdr.y, bdr.x + bdr.w, bdr.y + bdr.h);
    }
    else 
    {
      SDL_Color bd = theme->mBorderDark.toSdlColor();
      SDL_Rect bdr{ lx + xPos + 1, ly + yPos + 2, width, height - 1 };

      SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
      SDL_RenderDrawLine(renderer, bdr.x, bdr.y, bdr.x, bdr.y + bdr.h);
      SDL_RenderDrawLine(renderer, bdr.x, bdr.y, bdr.x + bdr.w, bdr.y);
      SDL_RenderDrawLine(renderer, bdr.x + bdr.w, bdr.y, bdr.x + bdr.w, bdr.y + bdr.h);
    }

    // Draw the text with some padding
    if (_labelTex.dirty)
    {
      std::string lb(mVisibleText.first, mVisibleText.last ? mVisibleText.last - mVisibleText.first : 0xff );

      if (mVisibleText.last != nullptr)
        lb += dots;
      mHeader->theme()->getTexAndRectUtf8(renderer, _labelTex, 0, 0, lb.c_str(), "sans", mHeader->fontSize(), mHeader->theme()->mTextColor);
    }

    if (_labelTex.tex)
    {
      int textX = mHeader->getAbsoluteLeft() + xPos + mHeader->theme()->mTabButtonHorizontalPadding;
      int textY = mHeader->getAbsoluteTop() + yPos  + mHeader->theme()->mTabButtonVerticalPadding + (active ? 1 : -2);

      SDL_RenderCopy(renderer, _labelTex, Vector2i(textX, textY));
    }    
}

void TabHeader::TabButton::drawActiveBorderAt(SDL_Renderer *renderer, const Vector2i &position,
                                              float offset, const Color &color) 
{
    int xPos = position.x;
    int yPos = position.y;
    int width = mSize.x;
    int height = mSize.y;

    SDL_Color c = color.toSdlColor();
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawLine(renderer, xPos + offset, yPos + height + offset, xPos + offset, yPos + offset);
    SDL_RenderDrawLine(renderer, xPos + offset, yPos + offset, xPos + width - offset, yPos + offset);
    SDL_RenderDrawLine(renderer, xPos + width - offset, yPos + offset, xPos + width - offset, yPos + height + offset);
}

void TabHeader::TabButton::drawInactiveBorderAt(SDL_Renderer *renderer, const Vector2i &position,
                                                float offset, const Color& color) 
{
    int xPos = position.x;
    int yPos = position.y;
    int width = mSize.x;
    int height = mSize.y;

    SDL_Color c = color.toSdlColor();
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_Rect r{ xPos + offset, yPos + offset, width - offset, height - offset };
    SDL_RenderDrawRect(renderer, &r);
}


TabHeader::TabHeader(Widget* parent, const std::string& font)
    : Widget(parent), mFont(font) 
{
}

void TabHeader::setActiveTab(int tabIndex) 
{
    assert(tabIndex < tabCount());
    mActiveTab = tabIndex;
    if (mCallback)
        mCallback(tabIndex);
}

int TabHeader::activeTab() const 
{
    return mActiveTab;
}

bool TabHeader::isTabVisible(int index) const 
{
    return index >= mVisibleStart && index < mVisibleEnd;
}

void TabHeader::addTab(const std::string & label) 
{
    addTab(tabCount(), label);
}

void TabHeader::addTab(int index, const std::string &label) 
{
    assert(index <= tabCount());
    mTabButtons.insert(std::next(mTabButtons.begin(), index), TabButton(*this, label));
    setActiveTab(index);
}

int TabHeader::removeTab(const std::string &label) 
{
    auto element = std::find_if(mTabButtons.begin(), mTabButtons.end(),
                                [&](const TabButton& tb) { return label == tb.label(); });
    int index = std::distance(mTabButtons.begin(), element);
    if (element == mTabButtons.end())
        return -1;
    mTabButtons.erase(element);
    if (index == mActiveTab && index != 0)
        setActiveTab(index - 1);
    return index;
}

void TabHeader::removeTab(int index) 
{
    assert(index < tabCount());
    mTabButtons.erase(std::next(mTabButtons.begin(), index));
    if (index == mActiveTab && index != 0)
        setActiveTab(index - 1);
}

const std::string& TabHeader::tabLabelAt(int index) const 
{
    assert(index < tabCount());
    return mTabButtons[index].label();
}

int TabHeader::tabIndex(const std::string &label) 
{
    auto it = std::find_if(mTabButtons.begin(), mTabButtons.end(),
                           [&](const TabButton& tb) { return label == tb.label(); });
    if (it == mTabButtons.end())
        return -1;
    return it - mTabButtons.begin();
}

void TabHeader::ensureTabVisible(int index) 
{
    auto visibleArea = visibleButtonArea();
    auto visibleWidth = visibleArea.second.x - visibleArea.first.x;
    int allowedVisibleWidth = mSize.x - 2 * theme()->mTabControlWidth;
    assert(allowedVisibleWidth >= visibleWidth);
    assert(index >= 0 && index < (int) mTabButtons.size());

    auto first = visibleBegin();
    auto last = visibleEnd();
    auto goal = tabIterator(index);

    // Reach the goal tab with the visible range.
    if (goal < first) 
    {
        do 
        {
            --first;
            visibleWidth += first->size().x;
        } 
        while (goal < first);

        while (allowedVisibleWidth < visibleWidth) 
        {
            --last;
            visibleWidth -= last->size().x;
        }
    }
    else if (goal >= last) 
    {
        do 
        {
            visibleWidth += last->size().x;
            ++last;
        } 
        while (goal >= last);
        
        while (allowedVisibleWidth < visibleWidth) 
        {
            visibleWidth -= first->size().x;
            ++first;
        }
    }

    // Check if it is possible to expand the visible range on either side.
    while (first != mTabButtons.begin()
           && std::next(first, -1)->size().x < allowedVisibleWidth - visibleWidth) 
    {
        --first;
        visibleWidth += first->size().x;
    }
    
    while (last != mTabButtons.end()
           && last->size().x < allowedVisibleWidth - visibleWidth) 
    {
        visibleWidth += last->size().x;
        ++last;
    }

    mVisibleStart = std::distance(mTabButtons.begin(), first);
    mVisibleEnd = std::distance(mTabButtons.begin(), last);
}

std::pair<Vector2i, Vector2i> TabHeader::visibleButtonArea() const
{
    if (mVisibleStart == mVisibleEnd)
      return{ {0,0}, {0,0} };
    auto topLeft = _pos + Vector2i(theme()->mTabControlWidth, 0);
    auto width = std::accumulate(visibleBegin(), visibleEnd(), theme()->mTabControlWidth,
                                 [](int acc, const TabButton& tb) 
    {
        return acc + tb.size().x;
    });
    auto bottomRight = _pos + Vector2i{ width, mSize.y };
    return { topLeft, bottomRight };
}

std::pair<Vector2i, Vector2i> TabHeader::activeButtonArea() const
{
    if (mVisibleStart == mVisibleEnd || mActiveTab < mVisibleStart || mActiveTab >= mVisibleEnd)
      return{ {0,0}, {0,0} };

    auto width = std::accumulate(visibleBegin(), activeIterator(), theme()->mTabControlWidth,
                                 [](int acc, const TabButton& tb) 
    {
        return acc + tb.size().x;
    });
    auto topLeft = _pos + Vector2i{ width, 0 };
    auto bottomRight = _pos + Vector2i{ width + activeIterator()->size().x, mSize.y };
    return { topLeft, bottomRight };
}

void TabHeader::performLayout(SDL_Renderer* ctx) 
{
    Widget::performLayout(ctx);

    Vector2i currentPosition(0, 0);
    // Place the tab buttons relative to the beginning of the tab header.
    for (auto& tab : mTabButtons) 
    {
        auto tabPreferred = tab.preferredSize(ctx);
        if (tabPreferred.x < theme()->mTabMinButtonWidth)
            tabPreferred.x = theme()->mTabMinButtonWidth;
        else if (tabPreferred.x > theme()->mTabMaxButtonWidth)
            tabPreferred.x = theme()->mTabMaxButtonWidth;
        tab.setSize(tabPreferred);
        tab.calculateVisibleString(nullptr);
        currentPosition.x += tabPreferred.x;
    }
    calculateVisibleEnd();
    if (mVisibleStart != 0 || mVisibleEnd != tabCount())
        mOverflowing = true;
}

Vector2i TabHeader::preferredSize(SDL_Renderer* ctx) const 
{
    // Set up the nvg context for measuring the text inside the tab buttons.
    //nvgFontFace(ctx, mFont.c_str());
    //nvgFontSize(ctx, fontSize());
    //nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
  Vector2i size = Vector2i(2*theme()->mTabControlWidth, 0);
    for (auto& tab : mTabButtons) 
    {
        auto tabPreferred = tab.preferredSize(ctx);
        if (tabPreferred.x < theme()->mTabMinButtonWidth)
            tabPreferred.x = theme()->mTabMinButtonWidth;
        else if (tabPreferred.x > theme()->mTabMaxButtonWidth)
            tabPreferred.x = theme()->mTabMaxButtonWidth;
        size.x += tabPreferred.x;
        size.y = std::max(size.y, tabPreferred.y);
    }
    return size;
}

bool TabHeader::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers)
{
    Widget::mouseButtonEvent(p, button, down, modifiers);
    if (button == SDL_BUTTON_LEFT && down) 
    {
        switch (locateClick(p)) 
        {
        case ClickLocation::LeftControls:
            onArrowLeft();
            return true;
        case ClickLocation::RightControls:
            onArrowRight();
            return true;
        case ClickLocation::TabButtons:
            auto first = visibleBegin();
            auto last = visibleEnd();
            int currentPosition = theme()->mTabControlWidth;
            int endPosition = p.x;
            auto firstInvisible = std::find_if(first, last,
                                               [&currentPosition, endPosition](const TabButton& tb) 
            {
                currentPosition += tb.size().x;
                return currentPosition > endPosition;
            });

            // Did not click on any of the tab buttons
            if (firstInvisible == last)
                return true;

            // Update the active tab and invoke the callback.
            setActiveTab(std::distance(mTabButtons.begin(), firstInvisible));
            return true;
        }
    }
    return false;
}

void TabHeader::draw(SDL_Renderer* renderer) 
{
    // Draw controls.
    Widget::draw(renderer);
    if (mOverflowing)
        drawControls(renderer);

    auto current = visibleBegin();
    auto last = visibleEnd();
    auto active = std::next(mTabButtons.begin(), mActiveTab);
    Vector2i currentPosition = _pos + Vector2i(theme()->mTabControlWidth, 0);

    // Flag to draw the active tab last. Looks a little bit better.
    bool drawActive = false;
    Vector2i activePosition{ 0, 0 };

    // Draw inactive visible buttons.
    for (; current != last; ++current)
    {
        if (current == active) 
        {
            drawActive = true;
            activePosition = currentPosition;
        }
        else
        {
            current->drawAtPosition(renderer, currentPosition, false);
        }
        currentPosition.x += current->size().x;
    }

    // Draw active visible button.
    if (drawActive)
        active->drawAtPosition(renderer, activePosition, true);
}

void TabHeader::calculateVisibleEnd() 
{
    auto first = visibleBegin();
    auto last = mTabButtons.end();
    int currentPosition = theme()->mTabControlWidth;
    int lastPosition = mSize.x - theme()->mTabControlWidth;
    auto firstInvisible = std::find_if(first, last,
                                       [&currentPosition, lastPosition](const TabButton& tb) 
    {
        currentPosition += tb.size().x;
        return currentPosition > lastPosition;
    });
    mVisibleEnd = std::distance(mTabButtons.begin(), firstInvisible);
}

void TabHeader::drawControls(SDL_Renderer *renderer) 
{
    // Left button.
    int lactive = mVisibleStart != 0 ? 1 : 0;
    int ractive = (mVisibleEnd != tabCount()) ? 1 : 0;

    // Draw the arrow.
    if (_lastLeftActive != lactive || _lastRightActive != ractive)
    {
      int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
      float ih = fontSize;
      ih *= 1.5f;
      if (_lastLeftActive != lactive)
      {
        auto iconLeft = utf8(ENTYPO_ICON_LEFT_BOLD);
        mTheme->getTexAndRectUtf8(renderer, _leftIcon, 0, 0, iconLeft.data(), "icons", ih, 
                                  lactive ? mTheme->mTextColor : mTheme->mButtonGradientBotPushed);
      }

      if (_lastRightActive != ractive)
      {
        auto iconRight = utf8(ENTYPO_ICON_RIGHT_BOLD);
        mTheme->getTexAndRectUtf8(renderer, _rightIcon, 0, 0, iconRight.data(), "icons", ih,
                                  ractive ? mTheme->mTextColor : mTheme->mButtonGradientBotPushed);
      }

      _lastLeftActive = lactive;
      _lastRightActive = ractive;
    }

    float yScaleLeft = 0.5f;
    float xScaleLeft = 0.2f;
    if (_leftIcon.tex)
    {
      Vector2f leftIconPos = absolutePosition().tofloat();
      leftIconPos += _pos.tofloat() + Vector2f{ xScaleLeft*theme()->mTabControlWidth, yScaleLeft*mSize.y };
      SDL_RenderCopy(renderer, _leftIcon, Vector2i(leftIconPos.x - _leftIcon.w() / 2, leftIconPos.y - _leftIcon.h() / 2));
    }

    // Draw the arrow.
    if (_rightIcon.tex)
    {
      float yScaleRight = 0.5f;
      float xScaleRight = 1.0f - xScaleLeft - _rightIcon.w() / theme()->mTabControlWidth;
      Vector2f leftControlsPos = absolutePosition().tofloat();
      leftControlsPos += _pos.tofloat() + Vector2f( mSize.x - theme()->mTabControlWidth, 0 );
      Vector2f rightIconPos = leftControlsPos + Vector2f(xScaleRight*theme()->mTabControlWidth, yScaleRight*mSize.tofloat().y);
      SDL_RenderCopy(renderer, _rightIcon, Vector2i(rightIconPos.x - _rightIcon.w() / 2, rightIconPos.y - _rightIcon.h() / 2 + 1));
    }
}

TabHeader::ClickLocation TabHeader::locateClick(const Vector2i& p)
{
  Vector2i leftDistance = p - _pos;
  bool hitLeft = leftDistance.positive() && leftDistance.lessOrEq({ theme()->mTabControlWidth, mSize.y });
  if (hitLeft)
    return ClickLocation::LeftControls;
  auto rightDistance = p - (_pos + Vector2i{ mSize.x - theme()->mTabControlWidth, 0 });
  bool hitRight = rightDistance.positive() && rightDistance.lessOrEq({theme()->mTabControlWidth, mSize.y});
    if (hitRight)
        return ClickLocation::RightControls;
    return ClickLocation::TabButtons;
}

void TabHeader::onArrowLeft() 
{
    if (mVisibleStart == 0)
        return;
    --mVisibleStart;
    calculateVisibleEnd();
}

void TabHeader::onArrowRight() 
{
    if (mVisibleEnd == tabCount())
        return;
    ++mVisibleStart;
    calculateVisibleEnd();
}

NAMESPACE_END(sdlgui)
