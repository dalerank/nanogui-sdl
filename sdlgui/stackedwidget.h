/*
    sdl_gui/stackedwidget.h -- Widget used to stack widgets on top
    of each other. Only the active widget is visible.

    The stacked widget was contributed by Stefan Ivanov.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>


    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <sdlgui/widget.h>

NAMESPACE_BEGIN(sdlgui)

/**
 * \class StackedWidget stackedwidget.h sdl_gui/stackedwidget.h
 *
 * \brief A stack widget.
 */
class  StackedWidget : public Widget 
{
public:
    StackedWidget(Widget* parent);

    void setSelectedIndex(int index);
    int selectedIndex() const;

    void performLayout(SDL_Renderer* ctx) override;
    Vector2i preferredSize(SDL_Renderer* ctx) const override;
    void addChild(int index, Widget* widget) override;

private:
    int mSelectedIndex = -1;
};

NAMESPACE_END(sdlgui)
