/*
    sdlgui/screen.h -- Top-level widget and interface between sdlgui and SDL

    A significant redesign of this code was contributed by Christian Schueller.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#ifndef __SDLGUI_SCREEN_H__
#define __SDLGUI_SCREEN_H__

#include <sdlgui/window.h>

union SDL_Event;
struct SDL_Window;

NAMESPACE_BEGIN(sdlgui)

/**
 * \brief Represents a display surface (i.e. a full-screen or windowed GLFW window)
 * and forms the root element of a hierarchy of sdlgui widgets
 */
class  Screen : public Widget
{
    friend class Widget;
    friend class Window;
public:
    /// Create a new screen
    Screen( SDL_Window* window, const Vector2i &size, const std::string &caption,
            bool resizable = true, bool fullscreen = false);

    /// Release all resources
    virtual ~Screen();

    /// Get the window titlebar caption
    const std::string &caption() const { return mCaption; }

    /// Set the window titlebar caption
    void setCaption(const std::string &caption);

    /// Return the screen's background color
    const Color &background() const { return mBackground; }

    /// Set the screen's background color
    void setBackground(const Color &background) { mBackground = background; }

    /// Set the top-level window visibility (no effect on full-screen windows)
    void setVisible(bool visible);

    /// Set window size
    void setSize(const Vector2i& size);

    /// Return the ratio between pixel and device coordinates (e.g. >= 2 on Mac Retina displays)
    float pixelRatio() const { return mPixelRatio; }

    virtual bool onEvent(SDL_Event& event);

    /// Draw the window contents -- put your OpenGL draw calls here
    virtual void drawContents() { /* To be overridden */ }

    /// Handle a file drop event
    virtual bool dropEvent(const std::vector<std::string> & /* filenames */) { return false; /* To be overridden */ }

    /// Default keyboard event handler
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);

    /// Text input event handler: codepoint is native endian UTF-32 format
    virtual bool keyboardCharacterEvent(unsigned int codepoint);

    /// Window resize event handler
    virtual bool resizeEvent(const Vector2i &) { return false; }

    virtual void drawAll();

    /// Return the last observed mouse position value
    Vector2i mousePos() const { return mMousePos; }

    /// Return a pointer to the underlying GLFW window data structure
    SDL_Window *window() { return _window; }

    /// Return a pointer to the underlying nanoVG draw context
    SDL_Renderer *sdlRenderer() { return mSDL_Renderer; }

    /// Compute the layout of all widgets
    void performLayout();

    template<typename... Args>Window& window(const Args&... args) { return wdg<Window>(args...); }
public:
    /// Initialize the \ref Screen
    void initialize(SDL_Window *window);

    /* Event handlers */
    bool cursorPosCallbackEvent(double x, double y);
    bool mouseButtonCallbackEvent(int button, int action, int modifiers);
    bool keyCallbackEvent(int key, int scancode, int action, int mods);
    bool charCallbackEvent(unsigned int codepoint);
    bool dropCallbackEvent(int count, const char **filenames);
    bool scrollCallbackEvent(double x, double y);
    bool resizeCallbackEvent(int width, int height);

    /* Internal helper functions */
    void updateFocus(Widget *widget);
    void disposeWindow(Window *window);
    void centerWindow(Window *window);
    void moveWindowToFront(Window *window);
    void drawWidgets();

    void performLayout(SDL_Renderer *renderer);

protected:
    SDL_Window *_window;
    std::vector<Widget *> mFocusPath;
    SDL_Renderer* mSDL_Renderer;
    Vector2i mFBSize;
    float mPixelRatio;
    int mMouseState, mModifiers;
    Vector2i mMousePos;
    bool mDragActive;
    Widget *mDragWidget = nullptr;
    double mLastInteraction;
    bool mProcessEvents;
    Color mBackground;
    std::string mCaption;
    std::string _lastTooltip;
    Texture _tooltipTex;
};

NAMESPACE_END(sdlgui)

#endif //__RRR_SCREEN_H__
