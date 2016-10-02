/*
    nanogui/opengl.h -- Pulls in OpenGL, GLEW (if needed), GLFW, and
    NanoVG header files

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#ifndef __NGSDL_OPENGL_INCLUDE_H__
#define __NGSDL_OPENGL_INCLUDE_H__

#ifdef NANOGUI_LINUX
    #include <SDL2/SDL_opengl.h>
    #include <SDL2/SDL_opengl_glext.h>
#else
    #include <SDL/SDL_opengl.h>
#endif

#include <nanogui/common.h>
#include <nanovg/nanovg.h>

NAMESPACE_BEGIN(nanogui)

inline Color::operator const NVGcolor &() const {
    return reinterpret_cast<const NVGcolor &>(*this->data());
}

/// Determine whether an icon ID is a texture loaded via nvgImageIcon
inline bool nvgIsImageIcon(int value) { return value < 1024; }

/// Determine whether an icon ID is a font-based icon (e.g. from the entypo.ttf font)
inline bool nvgIsFontIcon(int value) { return value >= 1024; }

NAMESPACE_END(nanogui)

#endif
