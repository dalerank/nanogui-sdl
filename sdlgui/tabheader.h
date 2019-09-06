/*
    sdlgui/tabheader.h -- Widget used to control tabs.

    The tab header widget was contributed by Stefan Ivanov.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <sdlgui/widget.h>
#include <vector>
#include <string>
#include <functional>
#include <utility>
#include <iterator>

NAMESPACE_BEGIN(sdlgui)

/**
 * \class TabHeader tabheader.h sdl_gui/tabheader.h
 *
 * \brief A Tab navigable widget.
 */
class  TabHeader : public Widget 
{
public:
    TabHeader(Widget *parent, const std::string &font = "sans-bold");

    void setFont(const std::string& font) { mFont = font; }
    const std::string& font() const { return mFont; }
    bool overflowing() const { return mOverflowing; }

    /**
     * Sets the callable objects which is invoked when a tab button is pressed.
     * The argument provided to the callback is the index of the tab.
     */
    void setCallback(const std::function<void(int)>& callback) { mCallback = callback; };
    const std::function<void(int)>& callback() const { return mCallback; }

    void setActiveTab(int tabIndex);
    int activeTab() const;
    bool isTabVisible(int index) const;
    int tabCount() const { return (int) mTabButtons.size();  }

    /// Inserts a tab at the end of the tabs collection.
    void addTab(const std::string& label);

    /// Inserts a tab into the tabs collection at the specified index.
    void addTab(int index, const std::string& label);

    /**
     * Removes the tab with the specified label and returns the index of the label.
     * Returns -1 if there was no such tab
     */
    int removeTab(const std::string& label);

    /// Removes the tab with the specified index.
    void removeTab(int index);

    /// Retrieves the label of the tab at a specific index.
    const std::string& tabLabelAt(int index) const;

    /**
     * Retrieves the index of a specific tab label.
     * Returns the number of tabs (tabsCount) if there is no such tab.
     */
    int tabIndex(const std::string& label);

    /**
     * Recalculate the visible range of tabs so that the tab with the specified
     * index is visible. The tab with the specified index will either be the
     * first or last visible one depending on the position relative to the
     * old visible range.
     */
    void ensureTabVisible(int index);

    /**
     * Returns a pair of Vectors describing the top left (pair.first) and the
     * bottom right (pair.second) positions of the rectangle containing the visible tab buttons.
     */
    std::pair<Vector2i, Vector2i> visibleButtonArea() const;

    /**
     * Returns a pair of Vectors describing the top left (pair.first) and the
     * bottom right (pair.second) positions of the rectangle containing the active tab button.
     * Returns two zero vectors if the active button is not visible.
     */
    std::pair<Vector2i, Vector2i> activeButtonArea() const;

    void performLayout(SDL_Renderer* ctx) override;
    Vector2i preferredSize(SDL_Renderer* ctx) const override;
    bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override;

    void draw(SDL_Renderer* renderer) override;

private:
    /**
     * \class TabButton tabheader.h
     *
     * \brief Implementation class of the actual tab buttons.
     */
    class TabButton {
    public:
        constexpr static const char* dots = "...";

        TabButton(TabHeader& header, const std::string& label);

        void setLabel(const std::string& label) { mLabel = label; }
        const std::string& label() const { return mLabel; }
        void setSize(const Vector2i& size) { mSize = size; }
        const Vector2i& size() const { return mSize; }

        Vector2i preferredSize(SDL_Renderer* ctx) const;
        void calculateVisibleString(SDL_Renderer* renderer);
        void drawAtPosition(SDL_Renderer* renderer, const Vector2i& position, bool active);
        void drawActiveBorderAt(SDL_Renderer * renderer, const Vector2i& position, float offset, const Color& color);
        void drawInactiveBorderAt(SDL_Renderer * renderer, const Vector2i& position, float offset, const Color& color);

    private:
        TabHeader* mHeader;
        std::string mLabel;
        Vector2i mSize;

        /**
         * \struct StringView tabheader.h sdl_gui/tabheader.h
         *
         * \brief Helper struct to represent the TabButton.
         */
        struct StringView {
            const char* first = nullptr;
            const char* last = nullptr;
        };
        StringView mVisibleText;
        int mVisibleWidth = 0;

        Texture _labelTex;
    };

    using TabIterator = std::vector<TabButton>::iterator;
    using ConstTabIterator = std::vector<TabButton>::const_iterator;

    /// The location in which the Widget will be facing.
    enum class ClickLocation {
        LeftControls, RightControls, TabButtons
    };

    TabIterator visibleBegin() { return std::next(mTabButtons.begin(), mVisibleStart); }
    TabIterator visibleEnd() { return std::next(mTabButtons.begin(), mVisibleEnd); }
    TabIterator activeIterator() { return std::next(mTabButtons.begin(), mActiveTab); }
    TabIterator tabIterator(int index) { return std::next(mTabButtons.begin(), index); }

    ConstTabIterator visibleBegin() const { return std::next(mTabButtons.begin(), mVisibleStart); }
    ConstTabIterator visibleEnd() const { return std::next(mTabButtons.begin(), mVisibleEnd); }
    ConstTabIterator activeIterator() const { return std::next(mTabButtons.begin(), mActiveTab); }
    ConstTabIterator tabIterator(int index) const { return std::next(mTabButtons.begin(), index); }

    /// Given the beginning of the visible tabs, calculate the end.
    void calculateVisibleEnd();

    void drawControls(SDL_Renderer* renderer);
    ClickLocation locateClick(const Vector2i& p);
    void onArrowLeft();
    void onArrowRight();

    std::function<void(int)> mCallback;
    std::vector<TabButton> mTabButtons;
    int mVisibleStart = 0;
    int mVisibleEnd = 0;
    int mActiveTab = 0;
    bool mOverflowing = false;

    std::string mFont;
    Texture _leftIcon;
    Texture _rightIcon;
    int _lastLeftActive = -1, _lastRightActive = -1;
};

NAMESPACE_END(sdlgui)
