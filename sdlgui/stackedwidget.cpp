/*
    sdlgui/stackedwidget.cpp -- Widget used to stack widgets on top
    of each other. Only the active widget is visible.

    The stacked widget was contributed by Stefan Ivanov.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/stackedwidget.h>

NAMESPACE_BEGIN(sdlgui)

StackedWidget::StackedWidget(Widget *parent)
    : Widget(parent) { }

void StackedWidget::setSelectedIndex(int index) 
{
    assert(index < childCount());
    if (mSelectedIndex >= 0)
        mChildren[mSelectedIndex]->setVisible(false);
    mSelectedIndex = index;
    mChildren[mSelectedIndex]->setVisible(true);
}

int StackedWidget::selectedIndex() const {
    return mSelectedIndex;
}

void StackedWidget::performLayout(SDL_Renderer *ctx) {
    for (auto child : mChildren) {
      child->setPosition({ 0, 0 });
        child->setSize(mSize);
        child->performLayout(ctx);
    }
}

Vector2i StackedWidget::preferredSize(SDL_Renderer *ctx) const 
{
  Vector2i size{ 0, 0 };
    for (auto child : mChildren)
        size = size.cmax(child->preferredSize(ctx));
    return size;
}

void StackedWidget::addChild(int index, Widget *widget) {
    if (mSelectedIndex >= 0)
        mChildren[mSelectedIndex]->setVisible(false);
    Widget::addChild(index, widget);
    widget->setVisible(true);
    setSelectedIndex(index);
}

NAMESPACE_END(sdlgui)
