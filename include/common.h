/*
    nanogui/common.h -- common definitions used by NanoGUI

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/ 

#ifndef __RRR_COMMMON_H__
#define __RRR_COMMMON_H__

#include <stdint.h>
#include <array>
#include <vector>
#include <cmath>

/* Set to 1 to draw boxes around widgets */
//#define NANOGUI_SHOW_WIDGET_BOUNDS 1

#if !defined(NAMESPACE_BEGIN)
#define NAMESPACE_BEGIN(name) namespace name {
#endif
#if !defined(NAMESPACE_END)
#define NAMESPACE_END(name) }
#endif

#if defined(NANOGUI_SHARED)
#  if defined(_WIN32)
#    if defined(NANOGUI_BUILD)
#      define NANOGUI_EXPORT __declspec(dllexport)
#    else
#      define NANOGUI_EXPORT __declspec(dllimport)
#    endif
#  elif defined(NANOGUI_BUILD)
#    define NANOGUI_EXPORT __attribute__ ((visibility("default")))
#  else
#    define NANOGUI_EXPORT
#  endif
#else
#    define NANOGUI_EXPORT
#endif

/* Force usage of discrete GPU on laptops (macro must be invoked in main application) */
#if defined(_WIN32)
#define NANOGUI_FORCE_DISCRETE_GPU() \
    extern "C" { \
        __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; \
        __declspec(dllexport) int NvOptimusEnablement = 1; \
    }
#else
#define NANOGUI_FORCE_DISCRETE_GPU()
#endif

#if defined(_WIN32)
    #if defined(NANOGUI_BUILD)
    /* Quench a few warnings on when compiling NanoGUI on Windows */
    #pragma warning(disable : 4127) // warning C4127: conditional expression is constant
    #pragma warning(disable : 4244) // warning C4244: conversion from X to Y, possible loss of data
    #endif
//#pragma warning(disable : 4251) // warning C4251: class X needs to have dll-interface to be used by clients of class Y
//#pragma warning(disable : 4714) // warning C4714: funtion X marked as __forceinline not inlined
#endif

struct NVGcontext;
struct NVGcolor;
struct NVGglyphPosition;

// Define command key for windows/mac/linux
#ifdef __APPLE__
#define SYSTEM_COMMAND_MOD GLFW_MOD_SUPER
#else
#define SYSTEM_COMMAND_MOD GLFW_MOD_CONTROL
#endif

NAMESPACE_BEGIN(nanogui)

/* Cursor shapes */
enum class Cursor {
    Arrow = 0,
    IBeam,
    Crosshair,
    Hand,
    HResize,
    VResize,
    CursorCount
};


/// Stores an RGBA color value
class Color {
public:
    Color() : Color(0, 0, 0, 0) {}

    //Color(const Eigen::Vector4f &color) : Eigen::Vector4f(color) { }

    //Color(const Eigen::Vector3f &color, float alpha)
      //  : Color(color(0), color(1), color(2), alpha) { }

    //Color(const Eigen::Vector3i &color, int alpha)
      //  : Color(color.cast<float>() / 255.f, alpha / 255.f) { }

    //Color(const Eigen::Vector3f &color) : Color(color, 1.0f) {}

    //Color(const Eigen::Vector3i &color)
      //  : Color((Vector3f)(color.cast<float>() / 255.f)) { }

    //Color(const Eigen::Vector4i &color)
      //  : Color((Vector4f)(color.cast<float>() / 255.f)) { }

    Color(float intensity, float alpha)
    { _d.r = _d.g = _d.b = intensity; _d.a = alpha; }

    Color(int intensity, int alpha)
    { _d.r = _d.g = _d.b = (intensity / 255.f); _d.a = alpha / 255.f; }

    Color(float r, float g, float b, float a)
    {  _d.r = r; _d.g = g; _d.b = b; _d.a = a;  }

    Color(int r, int g, int b, int a)
    {  _d.r = r/255.f; _d.g = g/255.f; _d.b = b/255.f; _d.a = a/255.f;  }

    /// Construct a color vector from MatrixBase (needed to play nice with Eigen)
    //template <typename Derived> Color(const Eigen::MatrixBase<Derived>& p)
      //  : Base(p) { }

    /// Assign a color vector from MatrixBase (needed to play nice with Eigen)
    /*template <typename Derived> Color &operator=(const Eigen::MatrixBase<Derived>& p) {
        this->Base::operator=(p);
        return *this;
    }*/

    /// Return a reference to the red channel
    float &r() { return _d.r; }
    /// Return a reference to the red channel (const version)
    const float &r() const { return _d.r; }
    /// Return a reference to the green channel
    float &g() { return _d.g; }
    /// Return a reference to the green channel (const version)
    const float &g() const { return _d.g; }
    /// Return a reference to the blue channel
    float &b() { return _d.b; }
    /// Return a reference to the blue channel (const version)
    const float &b() const { return _d.b; }

    float &a() { return _d.a; }
    const float &a() const { return _d.a; }

    void setAlpha(float a) { _d.a = a; }

    Color withAlpha(float a) const {
        Color c = *this;
        c._d.a = a;
        return c;
    }

    bool operator!=(const Color& c)
    {
        return !(c.a() == a() && c.r() == r() && c.g() == g() && c.b() == b());
    }

    Color contrastingColor() const {
        float luminance = r() * 0.299f + g() * 0.587f + b() * 0.144f;
        return Color(luminance < 0.5f ? 1.f : 0.f, 1.f);
    }

    Color operator*(float m) const
    {
        return Color(r()*m, g()*m, b()*m, a()*m);
    }

    Color operator+(const Color& c) const
    {
        return Color(r() + c.r(), g() + c.g(), b() + c.b(), a() + c.a());
    }

    inline operator const NVGcolor &() const;
private:
    struct _Data {
        union {
            float rgba[4];
            struct {
                float r,g,b,a;
            };
        };
    };
    _Data _d;
};


inline Color::operator const NVGcolor &() const {
    return reinterpret_cast<const NVGcolor &>(this->_d);
}

/// Determine whether an icon ID is a texture loaded via nvgImageIcon
inline bool nvgIsImageIcon(int value) { return value < 1024; }

/// Determine whether an icon ID is a font-based icon (e.g. from the entypo.ttf font)
inline bool nvgIsFontIcon(int value) { return value >= 1024; }


/* Forward declarations */
template <typename T> class ref;
class AdvancedGridLayout;
class BoxLayout;
class Button;
class CheckBox;
class ColorWheel;
class ColorPicker;
class ComboBox;
class GLFramebuffer;
class GLShader;
class GridLayout;
class GroupLayout;
class ImagePanel;
class Label;
class Layout;
class MessageDialog;
class Object;
class Popup;
class PopupButton;
class ProgressBar;
class Screen;
class Slider;
class TextBox;
class Theme;
class ToolButton;
class VScrollPanel;
class Widget;
class Window;

/// Static initialization; should be called once before invoking any NanoGUI functions
extern NANOGUI_EXPORT void init();

/// Static shutdown; should be called before the application terminates
extern NANOGUI_EXPORT void shutdown();

/// Enter the application main loop
extern NANOGUI_EXPORT void mainloop();

/// Request the application main loop to terminate
extern NANOGUI_EXPORT void leave();

/**
 * \brief Open a native file open/save dialog.
 *
 * \param filetypes
 *     Pairs of permissible formats with descriptions like
 *     <tt>("png", "Portable Network Graphics")</tt>
 */
extern NANOGUI_EXPORT std::string file_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes, bool save);

#if defined(__APPLE__)
/**
 * \brief Move to the application bundle's parent directory
 *
 * This is function is convenient when deploying .app bundles on OSX. It
 * adjusts the file path to the parent directory containing the bundle.
 */
extern NANOGUI_EXPORT void chdir_to_bundle_parent();
#endif

/**
 * \brief Convert a single UTF32 character code to UTF8
 *
 * NanoGUI uses this to convert the icon character codes
 * defined in entypo.h
 */
extern NANOGUI_EXPORT std::array<char, 8> utf8(int c);

/// Load a directory of PNG images and upload them to the GPU (suitable for use with ImagePanel)
extern NANOGUI_EXPORT std::vector<std::pair<int, std::string>>
    loadImageDirectory(NVGcontext *ctx, const std::string &path);

/// Convenience function for instanting a PNG icon from the application's data segment (via bin2c)
#define nvgImageIcon(ctx, name) nanogui::__nanogui_get_image(ctx, #name, name##_png, name##_png_size)
/// Helper function used by nvgImageIcon
extern NANOGUI_EXPORT int __nanogui_get_image(NVGcontext *ctx, const std::string &name, uint8_t *data, uint32_t size);

namespace math
{
    //! Constant for PI.
    const double PI64		= 3.1415926535897932384626433832795028841971693993751;

    const float ROUNDING_ERROR_f32 = 0.000001f;
    //! 64bit constant for converting from degrees to radians (formally known as GRAD_PI2)
    const double DEGTORAD64 = PI64 / 180.0;
    const double ROUNDING_ERROR_f64 = 0.00000001;
    const double RADTODEG64 = 180.0 / PI64;

    template<class T>
    inline bool isEqual(const T a, const T b)
    {
        return (a + ROUNDING_ERROR_f32 >= b) && (a - ROUNDING_ERROR_f32 <= b);
    }

    template<class T>
    inline bool isEqual(const T a, const T b, const T tolerance)
    {
        return (a + tolerance >= b) && (a - tolerance <= b);
    }
}

NAMESPACE_END(nanogui)

#endif
