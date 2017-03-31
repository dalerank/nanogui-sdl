/*
    src/screen.cpp -- Top-level widget and interface between NanoGUI and GLFW

    A significant redesign of this code was contributed by Christian Schueller.

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/screen.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/window.h>
#include <nanogui/popup.h>
#include <iostream>
#include <map>

#ifndef GL_GLEXT_PROTOTYPES
#ifdef WIN32
  PFNGLACTIVETEXTUREPROC glActiveTexture;
#else
  typedef void (GLAPIENTRY *PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint* arrays);
  typedef void (GLAPIENTRY *PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint* arrays);
  typedef void (GLAPIENTRY * PFNGLBINDVERTEXARRAYPROC) (GLuint array);
  typedef GLuint (GLAPIENTRY * PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar* uniformBlockName);
  typedef void (GLAPIENTRY * PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
  typedef void (GLAPIENTRY * PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
  typedef void (GLAPIENTRY * PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
  typedef void (GLAPIENTRY * PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
  typedef void (GLAPIENTRY * PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint* arrays);
  typedef void (GLAPIENTRY * PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint* renderbuffers);
  typedef void (GLAPIENTRY * PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
  typedef void (GLAPIENTRY * PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint* framebuffers);
  typedef void (GLAPIENTRY * PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
  typedef GLenum (GLAPIENTRY * PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
  typedef void (GLAPIENTRY * PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint* renderbuffers);
  typedef void (GLAPIENTRY * PFNGLBLITFRAMEBUFFERPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
  typedef void (GLAPIENTRY * PFNGLGENERATEMIPMAPPROC) (GLenum target);
  typedef void (GLAPIENTRY * PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
#endif
  #ifndef GL_UNIFORM_BUFFER
  #define GL_UNIFORM_BUFFER 0x8A11
  #endif

  #ifndef GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
  #define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
  #endif

#if defined(NANOVG_GLES3_IMPLEMENTATION)
  PFNGLDRAWBUFFERSPROC glDrawBuffers;
#endif
  PFNGLCREATESHADERPROC glCreateShader;
  PFNGLSHADERSOURCEPROC glShaderSource ;
  PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv ;
  PFNGLCOMPILESHADERPROC glCompileShader ;
  PFNGLGETSHADERIVPROC glGetShaderiv ;
  PFNGLUSEPROGRAMPROC glUseProgram ;
  PFNGLUNIFORM1IPROC glUniform1i ;
  PFNGLUNIFORM1FPROC glUniform1f ;
  PFNGLUNIFORM2IPROC glUniform2i ;
  PFNGLUNIFORM2FPROC glUniform2f ;
  PFNGLUNIFORM3FPROC glUniform3f ;
  PFNGLUNIFORM4FPROC glUniform4f ;
  PFNGLUNIFORM4FVPROC glUniform4fv ;
  PFNGLCREATEPROGRAMPROC glCreateProgram ;
  PFNGLATTACHSHADERPROC glAttachShader ;
  PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog ;
  PFNGLLINKPROGRAMPROC glLinkProgram ;
  PFNGLGETPROGRAMIVPROC glGetProgramiv ;
  PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog ;
  PFNGLGENVERTEXARRAYSPROC glGenVertexArrays ;
  PFNGLBINDVERTEXARRAYPROC glBindVertexArray ;
  PFNGLBINDBUFFERPROC glBindBuffer ;
  PFNGLGETATTRIBLOCATIONPROC  glGetAttribLocation ;
  PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation ;
  PFNGLGENBUFFERSPROC  glGenBuffers ;
  PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
  PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
  PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
  PFNGLBUFFERDATAPROC  glBufferData ;
  PFNGLDISABLEVERTEXATTRIBARRAYPROC  glDisableVertexAttribArray ;
  PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray ;
  PFNGLGETBUFFERSUBDATAPROC  glGetBufferSubData ;
  PFNGLVERTEXATTRIBPOINTERPROC  glVertexAttribPointer ;
  PFNGLDELETEBUFFERSPROC  glDeleteBuffers ;
  PFNGLBINDFRAMEBUFFERPROC  glBindFramebuffer ;
  PFNGLBINDRENDERBUFFERPROC  glBindRenderbuffer ;
  PFNGLRENDERBUFFERSTORAGEPROC  glRenderbufferStorage ;
  PFNGLDELETEVERTEXARRAYSPROC  glDeleteVertexArrays ;
  PFNGLDELETEPROGRAMPROC  glDeleteProgram ;
  PFNGLDELETESHADERPROC  glDeleteShader ;
  PFNGLGENRENDERBUFFERSPROC  glGenRenderbuffers ;
  PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC  glRenderbufferStorageMultisample ;
  PFNGLGENFRAMEBUFFERSPROC  glGenFramebuffers ;
  PFNGLFRAMEBUFFERRENDERBUFFERPROC  glFramebufferRenderbuffer ;
  PFNGLCHECKFRAMEBUFFERSTATUSPROC  glCheckFramebufferStatus ;
  PFNGLDELETERENDERBUFFERSPROC  glDeleteRenderbuffers ;
  PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer ;

  PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
  PFNGLBINDBUFFERRANGEPROC glBindBufferRange;
  PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate;
  PFNGLUNIFORM2FVPROC glUniform2fv;
  PFNGLBINDBUFFERBASEPROC glBindBufferBase;
#endif

#include <SDL2/SDL.h>
#include <nanovg/nanovg_gl.h>

NAMESPACE_BEGIN(nanogui)
static bool __glInit = false;

std::map<SDL_Window *, Screen *> __nanogui_screens;

static void __initGl()
{
   if( !__glInit )
   {
    __glInit = true;
 #ifndef GL_GLEXT_PROTOTYPES
    #define ASSIGNGLFUNCTION(type,name) name = (type)SDL_GL_GetProcAddress( #name );
#ifdef WIN32
    ASSIGNGLFUNCTION(PFNGLACTIVETEXTUREPROC,glActiveTexture)
#endif
#if defined(NANOVG_GLES3_IMPLEMENTATION)
    ASSIGNGLFUNCTION(PFNGLDRAWBUFFERSPROC,glDrawBuffers)
#endif
    ASSIGNGLFUNCTION(PFNGLCREATESHADERPROC,glCreateShader)
    ASSIGNGLFUNCTION(PFNGLSHADERSOURCEPROC,glShaderSource)
    ASSIGNGLFUNCTION(PFNGLUNIFORMMATRIX4FVPROC,glUniformMatrix4fv)
    ASSIGNGLFUNCTION(PFNGLCOMPILESHADERPROC,glCompileShader)
    ASSIGNGLFUNCTION(PFNGLGETSHADERIVPROC,glGetShaderiv)
    ASSIGNGLFUNCTION(PFNGLUSEPROGRAMPROC,glUseProgram)
    ASSIGNGLFUNCTION(PFNGLUNIFORM1IPROC,glUniform1i)
    ASSIGNGLFUNCTION(PFNGLUNIFORM1FPROC,glUniform1f)
    ASSIGNGLFUNCTION(PFNGLUNIFORM2IPROC,glUniform2i)
    ASSIGNGLFUNCTION(PFNGLUNIFORM2FPROC,glUniform2f)
    ASSIGNGLFUNCTION(PFNGLUNIFORM3FPROC,glUniform3f)
    ASSIGNGLFUNCTION(PFNGLUNIFORM4FPROC,glUniform4f)
    ASSIGNGLFUNCTION(PFNGLUNIFORM4FVPROC,glUniform4fv)
    ASSIGNGLFUNCTION(PFNGLCREATEPROGRAMPROC,glCreateProgram)
    ASSIGNGLFUNCTION(PFNGLATTACHSHADERPROC,glAttachShader)
    ASSIGNGLFUNCTION(PFNGLGETSHADERINFOLOGPROC,glGetShaderInfoLog)
    ASSIGNGLFUNCTION(PFNGLGETUNIFORMBLOCKINDEXPROC,glGetUniformBlockIndex)
    ASSIGNGLFUNCTION(PFNGLUNIFORMBLOCKBINDINGPROC,glUniformBlockBinding)
    ASSIGNGLFUNCTION(PFNGLBINDATTRIBLOCATIONPROC,glBindAttribLocation)
    ASSIGNGLFUNCTION(PFNGLLINKPROGRAMPROC,glLinkProgram)
    ASSIGNGLFUNCTION(PFNGLGETPROGRAMIVPROC,glGetProgramiv)
    ASSIGNGLFUNCTION(PFNGLGENVERTEXARRAYSPROC,glGenVertexArrays)
    ASSIGNGLFUNCTION(PFNGLGETPROGRAMINFOLOGPROC,glGetProgramInfoLog)
    ASSIGNGLFUNCTION(PFNGLBINDVERTEXARRAYPROC,glBindVertexArray)
    ASSIGNGLFUNCTION(PFNGLBINDBUFFERPROC,glBindBuffer)
    ASSIGNGLFUNCTION(PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation)
    ASSIGNGLFUNCTION(PFNGLGETUNIFORMLOCATIONPROC,glGetUniformLocation)
    ASSIGNGLFUNCTION(PFNGLGENBUFFERSPROC, glGenBuffers)
    ASSIGNGLFUNCTION(PFNGLBUFFERDATAPROC, glBufferData)
    ASSIGNGLFUNCTION(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray)
    ASSIGNGLFUNCTION(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray)
    ASSIGNGLFUNCTION(PFNGLGETBUFFERSUBDATAPROC, glGetBufferSubData)
    ASSIGNGLFUNCTION(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer)
    ASSIGNGLFUNCTION(PFNGLDELETEBUFFERSPROC, glDeleteBuffers)
    ASSIGNGLFUNCTION(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer)
    ASSIGNGLFUNCTION(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer)
    ASSIGNGLFUNCTION(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage)
    ASSIGNGLFUNCTION(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays)
    ASSIGNGLFUNCTION(PFNGLDELETEPROGRAMPROC, glDeleteProgram)
    ASSIGNGLFUNCTION(PFNGLDELETESHADERPROC, glDeleteShader)
    ASSIGNGLFUNCTION(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers)
    ASSIGNGLFUNCTION(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, glRenderbufferStorageMultisample)
    ASSIGNGLFUNCTION(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers)
    ASSIGNGLFUNCTION(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer)
    ASSIGNGLFUNCTION(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus)
    ASSIGNGLFUNCTION(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers)
    ASSIGNGLFUNCTION(PFNGLBLITFRAMEBUFFERPROC, glBlitFramebuffer)

    ASSIGNGLFUNCTION(PFNGLGENERATEMIPMAPPROC,glGenerateMipmap)
    ASSIGNGLFUNCTION(PFNGLBINDBUFFERRANGEPROC,glBindBufferRange)
    ASSIGNGLFUNCTION(PFNGLSTENCILOPSEPARATEPROC,glStencilOpSeparate)
    ASSIGNGLFUNCTION(PFNGLUNIFORM2FVPROC,glUniform2fv)
    ASSIGNGLFUNCTION(PFNGLBINDBUFFERBASEPROC,glBindBufferBase)
 #endif
   }
}

Screen::Screen( SDL_Window* window, const Vector2i &size, const std::string &caption,
               bool resizable, bool fullscreen)
    : Widget(nullptr), _window(nullptr), mNVGContext(nullptr), mCaption(caption)
{
    __initGl();
    SDL_SetWindowTitle( window, caption.c_str() );
    initialize( window );
}

void Screen::onEvent(SDL_Event& event)
{
    auto it = __nanogui_screens.find(_window);
    if (it == __nanogui_screens.end())
       return;

    switch( event.type )
    {
    case SDL_MOUSEMOTION:
    {
      if (!mProcessEvents)
         return;
      cursorPosCallbackEvent(event.motion.x, event.motion.y);
    }
    break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    {
      if (!mProcessEvents)
        return;

      SDL_Keymod mods = SDL_GetModState();
      mouseButtonCallbackEvent(event.button.button, event.button.type, mods);
    }
    break;

    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
      if (!mProcessEvents)
        return;

      SDL_Keymod mods = SDL_GetModState();
      keyCallbackEvent(event.key.keysym.sym, event.key.keysym.scancode, event.key.state, mods);
    }
    break;

    case SDL_TEXTINPUT:
    {
      if (!mProcessEvents)
        return;
      charCallbackEvent(event.text.text[0]);
    }
    break;
    }
}

void Screen::initialize(SDL_Window* window)
{
    _window = window;    
    SDL_GetWindowSize( window, &mSize[0], &mSize[1]);
    SDL_GetWindowSize( window, &mFBSize[0], &mFBSize[1]);
    
    int flags = NVG_STENCIL_STROKES | NVG_ANTIALIAS;
#ifdef NDEBUG
    flags |= NVG_DEBUG;
#endif

#ifdef NANOVG_GL2_IMPLEMENTATION
    mNVGContext = nvgCreateGL2(flags);
#elif defined(NANOVG_GL3_IMPLEMENTATION)
    mNVGContext = nvgCreateGL3(flags);
#elif defined(NANOVG_GLES2_IMPLEMENTATION)
    mNVGContext = nvgCreateGLES2(flags);
#elif defined(NANOVG_GLES3_IMPLEMENTATION)
    mNVGContext = nvgCreateGLES3(flags);
#endif
    if (mNVGContext == nullptr)
        throw std::runtime_error("Could not initialize NanoVG!");

    mVisible = true;
    mTheme = new Theme(mNVGContext);
    mMousePos = Vector2i::Zero();
    mMouseState = mModifiers = 0;
    mDragActive = false;
    mLastInteraction = SDL_GetTicks();
    mProcessEvents = true;
    mBackground = Vector3f(0.3f, 0.3f, 0.32f);
    __nanogui_screens[_window] = this;
}

Screen::~Screen()
{
    __nanogui_screens.erase(_window);
#ifdef NANOVG_GL2_IMPLEMENTATION
    nvgDeleteGL2(mNVGContext);
#elif DNANOVG_GL3_IMPLEMENTATION
    nvgDeleteGL3(mNVGContext);
#elif DNANOVG_GLES2_IMPLEMENTATION
    nvgDeleteGLES2(mNVGContext);
#elif DNANOVG_GLES3_IMPLEMENTATION
    nvgDeleteGLES3(mNVGContext);
#endif
}

void Screen::setVisible(bool visible)
{
    if (mVisible != visible)
     {
        mVisible = visible;

        if (visible)
            SDL_ShowWindow(_window);
        else
            SDL_HideWindow(_window);
    }
}

void Screen::setCaption(const std::string &caption)
{
    if (caption != mCaption)
    {
        SDL_SetWindowTitle( _window, caption.c_str());
        mCaption = caption;
    }
}

void Screen::setSize(const Vector2i &size)
{
    Widget::setSize(size);
    SDL_SetWindowSize(_window, size.x(), size.y());
}

void Screen::drawAll()
{
    drawContents();
    drawWidgets();
}

void Screen::drawWidgets()
{
    if (!mVisible)
        return;

    //SDL_GL_MakeCurrent( _window, _glcontext );
    SDL_GL_GetDrawableSize(_window, &mFBSize[0], &mFBSize[1]);
    SDL_GetWindowSize( _window, &mSize[0], &mSize[1]);
    glViewport(0, 0, mFBSize[0], mFBSize[1]);

    /* Calculate pixel ratio for hi-dpi devices. */
    mPixelRatio = (float) mFBSize[0] / (float) mSize[0];
    
    nvgBeginFrame(mNVGContext, mSize[0], mSize[1], mPixelRatio);

    draw(mNVGContext);

    double elapsed = SDL_GetTicks() - mLastInteraction;

    if (elapsed > 0.5f) {
        /* Draw tooltips */
        const Widget *widget = findWidget(mMousePos);
        if (widget && !widget->tooltip().empty()) {
            int tooltipWidth = 150;

            float bounds[4];
            nvgFontFace(mNVGContext, "sans");
            nvgFontSize(mNVGContext, 15.0f);
            nvgTextAlign(mNVGContext, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgTextLineHeight(mNVGContext, 1.1f);
            Vector2i pos = widget->absolutePosition() +
                           Vector2i(widget->width() / 2, widget->height() + 10);

            nvgTextBoxBounds(mNVGContext, pos.x(), pos.y(), tooltipWidth,
                             widget->tooltip().c_str(), nullptr, bounds);

            nvgGlobalAlpha(mNVGContext,
                           std::min(1.0, 2 * (elapsed - 0.5f)) * 0.8);

            nvgBeginPath(mNVGContext);
            nvgFillColor(mNVGContext, Color(0, 255));
            int h = (bounds[2] - bounds[0]) / 2;
            nvgRoundedRect(mNVGContext, bounds[0] - 4 - h, bounds[1] - 4,
                           (int) (bounds[2] - bounds[0]) + 8,
                           (int) (bounds[3] - bounds[1]) + 8, 3);

            int px = (int) ((bounds[2] + bounds[0]) / 2) - h;
            nvgMoveTo(mNVGContext, px, bounds[1] - 10);
            nvgLineTo(mNVGContext, px + 7, bounds[1] + 1);
            nvgLineTo(mNVGContext, px - 7, bounds[1] + 1);
            nvgFill(mNVGContext);

            nvgFillColor(mNVGContext, Color(255, 255));
            nvgFontBlur(mNVGContext, 0.0f);
            nvgTextBox(mNVGContext, pos.x() - h, pos.y(), tooltipWidth,
                       widget->tooltip().c_str(), nullptr);
        }
    }

    nvgEndFrame(mNVGContext);
}

bool Screen::keyboardEvent(int key, int scancode, int action, int modifiers) {
    if (mFocusPath.size() > 0) {
        for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
            if ((*it)->focused() && (*it)->keyboardEvent(key, scancode, action, modifiers))
                return true;
    }

    return false;
}

bool Screen::keyboardCharacterEvent(unsigned int codepoint) {
    if (mFocusPath.size() > 0) {
        for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
            if ((*it)->focused() && (*it)->keyboardCharacterEvent(codepoint))
                return true;
    }
    return false;
}

bool Screen::cursorPosCallbackEvent(double x, double y) {
    Vector2i p((int) x, (int) y);
    bool ret = false;
    mLastInteraction = SDL_GetTicks();
    try {
        p -= Vector2i(1, 2);

        if (!mDragActive) {
            Widget *widget = findWidget(p);
            /*if (widget != nullptr && widget->cursor() != mCursor) {
                mCursor = widget->cursor();
                glfwSetCursor(mGLFWWindow, mCursors[(int) mCursor]);
            }*/
        } else {
            ret = mDragWidget->mouseDragEvent(
                p - mDragWidget->parent()->absolutePosition(), p - mMousePos,
                mMouseState, mModifiers);
        }

        if (!ret)
            ret = mouseMotionEvent(p, p - mMousePos, mMouseState, mModifiers);

        mMousePos = p;

        return ret;
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        abort();
    }

    return false;
}

bool Screen::mouseButtonCallbackEvent(int button, int action, int modifiers) {
    mModifiers = modifiers;
    mLastInteraction = SDL_GetTicks();
    try {
        if (mFocusPath.size() > 1) {
            const Window *window =
                dynamic_cast<Window *>(mFocusPath[mFocusPath.size() - 2]);
            if (window && window->modal()) {
                if (!window->contains(mMousePos))
                    return false;
            }
        }

        if (action == SDL_MOUSEBUTTONDOWN)
            mMouseState |= 1 << button;
        else
            mMouseState &= ~(1 << button);

        auto dropWidget = findWidget(mMousePos);
        if (mDragActive && action == SDL_MOUSEBUTTONUP &&
            dropWidget != mDragWidget)
            mDragWidget->mouseButtonEvent(
                mMousePos - mDragWidget->parent()->absolutePosition(), button,
                false, mModifiers);

        /*if (dropWidget != nullptr && dropWidget->cursor() != mCursor) {
            mCursor = dropWidget->cursor();
            glfwSetCursor(mGLFWWindow, mCursors[(int) mCursor]);
        }*/

        if (action == SDL_MOUSEBUTTONDOWN && button == SDL_BUTTON_LEFT) {
            mDragWidget = findWidget(mMousePos);
            if (mDragWidget == this)
                mDragWidget = nullptr;
            mDragActive = mDragWidget != nullptr;
            if (!mDragActive)
                updateFocus(nullptr);
        } else {
            mDragActive = false;
            mDragWidget = nullptr;
        }

        return mouseButtonEvent(mMousePos, button, action == SDL_MOUSEBUTTONDOWN,
                                mModifiers);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        abort();
    }

    return false;
}

bool Screen::keyCallbackEvent(int key, int scancode, int action, int mods)
{
    mLastInteraction = SDL_GetTicks();
    try {
        return keyboardEvent(key, scancode, action, mods);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        abort();
    }
}

bool Screen::charCallbackEvent(unsigned int codepoint)
 {
    mLastInteraction = SDL_GetTicks();
    try {
        return keyboardCharacterEvent(codepoint);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        abort();
    }
}

bool Screen::dropCallbackEvent(int count, const char **filenames) {
    std::vector<std::string> arg(count);
    for (int i = 0; i < count; ++i)
        arg[i] = filenames[i];
    return dropEvent(arg);
}

bool Screen::scrollCallbackEvent(double x, double y)
{
    mLastInteraction = SDL_GetTicks();
    try {
        if (mFocusPath.size() > 1) {
            const Window *window =
                dynamic_cast<Window *>(mFocusPath[mFocusPath.size() - 2]);
            if (window && window->modal()) {
                if (!window->contains(mMousePos))
                    return false;
            }
        }
        return scrollEvent(mMousePos, Vector2f(x, y));
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        abort();
    }

    return false;
}

bool Screen::resizeCallbackEvent(int, int)
{
    Vector2i fbSize, size;
    //glfwGetFramebufferSize(mGLFWWindow, &fbSize[0], &fbSize[1]);
    SDL_GetWindowSize(_window, &size[0], &size[1]);

    if (mFBSize == Vector2i(0, 0) || size == Vector2i(0, 0))
        return false;

    mFBSize = fbSize;
    mSize = size;
    mLastInteraction = SDL_GetTicks();

    try {
        return resizeEvent(mSize);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        abort();
    }
}

void Screen::updateFocus(Widget *widget) {
    for (auto w: mFocusPath) {
        if (!w->focused())
            continue;
        w->focusEvent(false);
    }
    mFocusPath.clear();
    Widget *window = nullptr;
    while (widget) {
        mFocusPath.push_back(widget);
        if (dynamic_cast<Window *>(widget))
            window = widget;
        widget = widget->parent();
    }
    for (auto it = mFocusPath.rbegin(); it != mFocusPath.rend(); ++it)
        (*it)->focusEvent(true);

    if (window)
        moveWindowToFront((Window *) window);
}

void Screen::disposeWindow(Window *window) {
    if (std::find(mFocusPath.begin(), mFocusPath.end(), window) != mFocusPath.end())
        mFocusPath.clear();
    if (mDragWidget == window)
        mDragWidget = nullptr;
    removeChild(window);
}

void Screen::centerWindow(Window *window) {
    if (window->size() == Vector2i::Zero()) {
        window->setSize(window->preferredSize(mNVGContext));
        window->performLayout(mNVGContext);
    }
    window->setPosition((mSize - window->size()) / 2);
}

void Screen::moveWindowToFront(Window *window) {
    mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), window), mChildren.end());
    mChildren.push_back(window);
    /* Brute force topological sort (no problem for a few windows..) */
    bool changed = false;
    do {
        size_t baseIndex = 0;
        for (size_t index = 0; index < mChildren.size(); ++index)
            if (mChildren[index] == window)
                baseIndex = index;
        changed = false;
        for (size_t index = 0; index < mChildren.size(); ++index) {
            Popup *pw = dynamic_cast<Popup *>(mChildren[index]);
            if (pw && pw->parentWindow() == window && index < baseIndex) {
                moveWindowToFront(pw);
                changed = true;
                break;
            }
        }
    } while (changed);
}

void Screen::performLayout(NVGcontext* ctx)
{
  Widget::performLayout(ctx);
}

void Screen::performLayout()
{
  Widget::performLayout(mNVGContext);
}

NAMESPACE_END(nanogui)
