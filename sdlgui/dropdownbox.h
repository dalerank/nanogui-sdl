/*
    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/**
 * \file nanogui/dropdownbox.h
 *
 * \brief Simple dropdown box widget based on a popup button.
 */

#pragma once

#include <sdlgui/popupbutton.h>

NAMESPACE_BEGIN(sdlgui)

/**
 * \class DropdownBox dropdownbox.h nanogui/dropdownbox.h
 *
 * \brief Simple dropdownbox box widget based on a popup button.
 */
class DropdownBox : public PopupButton
{
public:
    /// Create an empty combo box
    DropdownBox(Widget *parent);

    /// Create a new combo box with the given items
    DropdownBox(Widget *parent, const std::vector<std::string> &items);

    /**
     * \brief Create a new dropdownbox with the given items, providing both short and
     * long descriptive labels for each item
     */
    DropdownBox(Widget *parent, const std::vector<std::string> &items,
             const std::vector<std::string> &itemsShort);

    /// The callback to execute for this widget.
    std::function<void(int)> callback() const { return mCallback; }

    /// Sets the callback to execute for this widget.
    void setCallback(const std::function<void(int)> &callback) { mCallback = callback; }

    /// The current index this dropdownbox has selected.
    int selectedIndex() const { return mSelectedIndex; }

    void performLayout(SDL_Renderer *renderer) override;

    /// Sets the current index this dropdownbox has selected.
    void setSelectedIndex(int idx);

    /// Sets the items for this dropdownbox, providing both short and long descriptive lables for each item.
    void setItems(const std::vector<std::string> &items, const std::vector<std::string> &itemsShort);

    /// Sets the items for this dropdownbox.
    void setItems(const std::vector<std::string> &items) { setItems(items, items); }

    /// The items associated with this dropdownbox.
    const std::vector<std::string> &items() const { return mItems; }

    /// The short descriptions associated with this dropdownbox.
    const std::vector<std::string> &itemsShort() const { return mItemsShort; }

    /// Handles mouse scrolling events for this dropdownbox.
    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel) override;

    virtual void draw(SDL_Renderer* renderer) override;
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override; 

protected:
    /// The items associated with this dropdownbox.
    std::vector<std::string> mItems;

    /// The short descriptions of items associated with this dropdownbox.
    std::vector<std::string> mItemsShort;

    /// The callback for this dropdownbox.
    std::function<void(int)> mCallback;

    /// The current index this dropdownbox has selected.
    int mSelectedIndex;
};

NAMESPACE_END(sdlgui)
