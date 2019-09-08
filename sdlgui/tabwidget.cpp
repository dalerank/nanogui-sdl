/*
    sdl_gui/tabwidget.cpp -- A wrapper around the widgets TabHeader and StackedWidget
    which hooks the two classes together.

    The tab widget was contributed by Stefan Ivanov.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/tabwidget.h>
#include <sdlgui/tabheader.h>
#include <sdlgui/stackedwidget.h>
#include <sdlgui/theme.h>
#include <sdlgui/window.h>
#include <sdlgui/screen.h>
#include <algorithm>

NAMESPACE_BEGIN(sdlgui)

TabWidget::TabWidget(Widget* parent)
    : Widget(parent), mHeader(new TabHeader(this)), mContent(new StackedWidget(this)) 
{
    mHeader->setCallback([this](int i) 
    {
        mContent->setSelectedIndex(i);
        if (mCallback)
            mCallback(i);
    });
}

void TabWidget::setActiveTab(int tabIndex) 
{
    mHeader->setActiveTab(tabIndex);
    mContent->setSelectedIndex(tabIndex);
}

int TabWidget::activeTab() const 
{
    assert(mHeader->activeTab() == mContent->selectedIndex());
    return mContent->selectedIndex();
}

int TabWidget::tabCount() const 
{
    assert(mContent->childCount() == mHeader->tabCount());
    return mHeader->tabCount();
}

Widget* TabWidget::createTab(int index, const std::string &label) 
{
    Widget* tab = new Widget(nullptr);
    addTab(index, label, tab);
    return tab;
}

Widget* TabWidget::createTab(const std::string &label) 
{
    return createTab(tabCount(), label);
}

void TabWidget::addTab(const std::string &name, Widget *tab) 
{
    addTab(tabCount(), name, tab);
}

void TabWidget::addTab(int index, const std::string &label, Widget *tab) 
{
    assert(index <= tabCount());
    // It is important to add the content first since the callback
    // of the header will automatically fire when a new tab is added.
    mContent->addChild(index, tab);
    mHeader->addTab(index, label);
    assert(mHeader->tabCount() == mContent->childCount());
}

int TabWidget::tabLabelIndex(const std::string &label) 
{
    return mHeader->tabIndex(label);
}

int TabWidget::tabIndex(Widget* tab) 
{
    return mContent->childIndex(tab);
}

void TabWidget::ensureTabVisible(int index) 
{
    if (!mHeader->isTabVisible(index))
        mHeader->ensureTabVisible(index);
}

const Widget *TabWidget::tab(const std::string &tabName) const 
{
    int index = mHeader->tabIndex(tabName);
    if (index == mContent->childCount())
        return nullptr;
    return mContent->children()[index];
}

Widget *TabWidget::tab(const std::string &tabName) 
{
    int index = mHeader->tabIndex(tabName);
    if (index == mContent->childCount())
        return nullptr;
    return mContent->children()[index];
}

bool TabWidget::removeTab(const std::string &tabName) 
{
    int index = mHeader->removeTab(tabName);
    if (index == -1)
        return false;
    mContent->removeChild(index);
    return true;
}

void TabWidget::removeTab(int index) 
{
    assert(mContent->childCount() < index);
    mHeader->removeTab(index);
    mContent->removeChild(index);

    if (activeTab() == index)
        setActiveTab(index == (index - 1) ? index - 1 : 0);
}

const std::string &TabWidget::tabLabelAt(int index) const 
{
    return mHeader->tabLabelAt(index);
}

void TabWidget::performLayout(SDL_Renderer* ctx) 
{
    int headerHeight = mHeader->preferredSize(ctx).y;
    int margin = mTheme->mTabInnerMargin;
    mHeader->setPosition({ 0, 0 });
    mHeader->setSize({ mSize.x, headerHeight });
    mHeader->performLayout(ctx);
    mContent->setPosition({ margin, headerHeight + margin });
    mContent->setSize({ mSize.x - 2 * margin, mSize.y - 2*margin - headerHeight });
    mContent->performLayout(ctx);
}

Vector2i TabWidget::preferredSize(SDL_Renderer* ctx) const
{
    auto contentSize = mContent->preferredSize(ctx);
    auto headerSize = mHeader->preferredSize(ctx);
    int margin = mTheme->mTabInnerMargin;
    auto borderSize = Vector2i{ 2 * margin, 2 * margin };
    Vector2i tabPreferredSize = contentSize + borderSize + Vector2i{ 0, headerSize.y };
    return tabPreferredSize;
}

void TabWidget::draw(SDL_Renderer* renderer) 
{
    int tabHeight = mHeader->preferredSize(nullptr).y;
    auto activeArea = mHeader->activeButtonArea();

    for (int i = 0; i < 3; ++i) 
    {
      int x = getAbsoluteLeft();
      int y = getAbsoluteTop();
      SDL_Color bl = mTheme->mBorderLight.toSdlColor();
      SDL_Rect blr{ x + 1, y + tabHeight + 2, mSize.x - 2,  mSize.y - tabHeight - 2 };

      SDL_SetRenderDrawColor(renderer, bl.r, bl.g, bl.b, bl.a);
      SDL_RenderDrawLine(renderer, blr.x, blr.y, x + activeArea.first.x, blr.y);
      SDL_RenderDrawLine(renderer, x + activeArea.second.x, blr.y, blr.x + blr.w, blr.y);
      SDL_RenderDrawLine(renderer, blr.x + blr.w, blr.y, blr.x + blr.w, blr.y + blr.h);
      SDL_RenderDrawLine(renderer, blr.x, blr.y, blr.x, blr.y + blr.h);
      SDL_RenderDrawLine(renderer, blr.x, blr.y + blr.h, blr.x + blr.w, blr.y + blr.h);

      SDL_Color bd = mTheme->mBorderDark.toSdlColor();
      SDL_Rect bdr{ x + 1, y + tabHeight + 1, mSize.x - 2, mSize.y - tabHeight - 2 };

      SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
      SDL_RenderDrawLine(renderer, bdr.x, bdr.y, x + activeArea.first.x, bdr.y);
      SDL_RenderDrawLine(renderer, x + activeArea.second.x, bdr.y, bdr.x + bdr.w, bdr.y);
      SDL_RenderDrawLine(renderer, bdr.x + bdr.w, bdr.y, bdr.x + bdr.w, bdr.y + bdr.h);
      SDL_RenderDrawLine(renderer, bdr.x, bdr.y, bdr.x, bdr.y + bdr.h);
      SDL_RenderDrawLine(renderer, bdr.x, bdr.y + bdr.h, bdr.x + bdr.w, bdr.y + bdr.h);

    }

    Widget::draw(renderer);
}

NAMESPACE_END(sdlgui)
