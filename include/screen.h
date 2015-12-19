/*
    nanogui/screen.h -- Top-level widget and interface between NanoGUI and GLFW

    A significant redesign of this code was contributed by Christian Schueller.

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#ifndef __RRR_SCREEN_H__
#define __RRR_SCREEN_H__

#include <include/widget.h>

union SDL_Event;
struct SDL_Window;

NAMESPACE_BEGIN(nanogui)

/**
 * \brief Represents a display surface (i.e. a full-screen or windowed GLFW window)
 * and forms the root element of a hierarchy of nanogui widgets
 */
class NANOGUI_EXPORT Screen : public Widget
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
    const Vector3f &background() const { return mBackground; }

    /// Set the screen's background color
    void setBackground(const Vector3f &background) { mBackground = background; }

    /// Set the top-level window visibility (no effect on full-screen windows)
    void setVisible(bool visible);

    /// Set window size
    void setSize(const Vector2i& size);

    /// Draw the Screen contents
    virtual void drawAll();

    virtual void onEvent(SDL_Event& event);

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

    /// Return the last observed mouse position value
    Vector2i mousePos() const { return mMousePos; }

    /// Return a pointer to the underlying GLFW window data structure
    SDL_Window *window() { return _window; }

    /// Return a pointer to the underlying nanoVG draw context
    NVGcontext *nvgContext() { return mNVGContext; }

    /// Compute the layout of all widgets
    void performLayout();
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

    void performLayout(NVGcontext *ctx);

protected:
    SDL_Window *_window;
    NVGcontext *mNVGContext;
    std::vector<Widget *> mFocusPath;
    Vector2i mFBSize;
    float mPixelRatio;
    int mMouseState, mModifiers;
    Vector2i mMousePos;
    bool mDragActive;
    Widget *mDragWidget = nullptr;
    double mLastInteraction;
    bool mProcessEvents;
    Vector3f mBackground;
    std::string mCaption;
};

NAMESPACE_END(nanogui)

#endif //__RRR_SCREEN_H__
