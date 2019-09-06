/*
    sdl_gui/popupbutton.h -- Button which launches a popup widget

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <sdlgui/button.h>
#include <sdlgui/popup.h>
#include <sdlgui/entypo.h>

NAMESPACE_BEGIN(sdlgui)

/**
 * \class PopupButton popupbutton.h sdl_gui/popupbutton.h
 *
 * \brief Button which launches a popup widget.
 */
class  PopupButton : public Button 
{
public:
    PopupButton(Widget *parent, const std::string &caption = "Untitled",
                int buttonIcon = 0,
                int chevronIcon = ENTYPO_ICON_CHEVRON_SMALL_RIGHT);

    void setChevronIcon(int icon) { mChevronIcon = icon; }
    int chevronIcon() const { return mChevronIcon; }

    Popup& popup(const Vector2i& size) { mPopup->setFixedSize(size); return *mPopup; }
    Popup& popup() { return *mPopup; }
    Popup* popupptr() { return mPopup; }

    void draw(SDL_Renderer* renderer) override;
    Vector2i preferredSize(SDL_Renderer *ctx) const override;
    void performLayout(SDL_Renderer *ctx) override;

    PopupButton& withChevron(int icon) { setChevronIcon(icon); return *this; }
protected:
    
    Popup *mPopup = nullptr;
    int mChevronIcon;

    Texture _chevronTex;
};

NAMESPACE_END(sdlgui)
