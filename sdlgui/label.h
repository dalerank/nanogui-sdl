/*
    sdl_gui/label.h -- Text label with an arbitrary font, color, and size

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
 * \class Label label.h sdl_gui/label.h
 *
 * \brief Text label widget.
 *
 * The font and color can be customized. When \ref Widget::setFixedWidth()
 * is used, the text is wrapped when it surpasses the specified width.
 */
class  Label : public Widget 
{
public:
    Label(Widget *parent, const std::string &caption,
          const std::string &font = "sans", int fontSize = -1);

    /// Get the label's text caption
    const std::string &caption() const { return mCaption; }
    /// Set the label's text caption
    void setCaption(const std::string &caption) { mCaption = caption; }

    /// Set the currently active font (2 are available by default: 'sans' and 'sans-bold')
    void setFont(const std::string &font) { mFont = font; }
    /// Get the currently active font
    const std::string &font() const { return mFont; }

    /// Get the label color
    Color color() const { return mColor; }
    /// Set the label color
    void setColor(const Color& color) { mColor = color; }

    /// Set the \ref Theme used to draw this widget
    virtual void setTheme(Theme *theme) override;

    /// Compute the size needed to fully display the label
    virtual Vector2i preferredSize(SDL_Renderer *ctx) const override;

    /// Draw the label
    void draw(SDL_Renderer *renderer) override;
    void setFontSize(int fontSize) override;

protected:
    std::string mCaption;
    std::string mFont;
    Color mColor;
    Texture _texture;
};

NAMESPACE_END(sdlgui)
