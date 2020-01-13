
#ifndef SDLGUI_HEADER_INCLUDE
#define SDLGUI_HEADER_INCLUDE

#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <functional>
#include <atomic>
#include <vector>
#include <math.h>
#include <assert.h>
#include <istream>
#if defined(_WIN32)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

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

struct NVGcolor;
struct NVGcontext;

#ifdef _MSC_VER
#define SDLGUI_SNPRINTF _snprintf
#else
/// Platform dependent snprintf (``_snprintf`` for MSVC, ``snprintf`` otherwise).
#define SDLGUI_SNPRINTF snprintf
#endif


#if !defined(NAMESPACE_BEGIN)
#define NAMESPACE_BEGIN(name) namespace name {
#endif

#if !defined(NAMESPACE_END)
#define NAMESPACE_END(name) }
#endif

#if defined(_WIN32)
    #if defined(NNGUI_BUILD)
    /* Quench a few warnings on when compiling NNGUI on Windows */
    #pragma warning(disable : 4127) // warning C4127: conditional expression is constant
    #pragma warning(disable : 4244) // warning C4244: conversion from X to Y, possible loss of data
    #endif
//#pragma warning(disable : 4251) // warning C4251: class X needs to have dll-interface to be used by clients of class Y
//#pragma warning(disable : 4714) // warning C4714: funtion X marked as __forceinline not inlined
#endif

NAMESPACE_BEGIN(sdlgui)

struct ImageInfo
{
  SDL_Texture* tex = nullptr;
  int w, h;
  std::string path;
};
typedef std::vector<ImageInfo> ListImages;

/// Load a directory of PNG images and upload them to the GPU (suitable for use with ImagePanel)
ListImages loadImageDirectory(SDL_Renderer* renderer, const std::string &path);

/**
* \class Object object.h sdlgui/object.h
*
* \brief Reference counted object base class.
*/
class Object
{
public:
  /// Default constructor
  Object() { }

  /// Copy constructor
  Object(const Object &) : m_refCount(0) {}

  /// Return the current reference count
  int getRefCount() const { return m_refCount; };

  /// Increase the object's reference count by one
  void incRef() const { ++m_refCount; }

  /** \brief Decrease the reference count of
  * the object and possibly deallocate it.
  *
  * The object will automatically be deallocated once
  * the reference count reaches zero.
  */
  void decRef(bool dealloc = true) const noexcept;
protected:
  /** \brief Virtual protected deconstructor.
  * (Will only be called by \ref ref)
  */
  virtual ~Object();
private:
  mutable std::atomic<int> m_refCount{ 0 };
};

/**
* \class ref object.h sdlgui/object.h
*
* \brief Reference counting helper.
*
* The \a ref template is a simple wrapper to store a pointer to an object. It
* takes care of increasing and decreasing the object's reference count as
* needed. When the last reference goes out of scope, the associated object
* will be deallocated.
*
* The advantage over C++ solutions such as ``std::shared_ptr`` is that
* the reference count is very compactly integrated into the base object
* itself.
*/
template <typename T> class ref {
public:
  /// Create a ``nullptr``-valued reference
  ref() { }

  /// Construct a reference from a pointer
  ref(T *ptr) : m_ptr(ptr) {
    if (m_ptr)
      ((Object *)m_ptr)->incRef();
  }

  /// Copy constructor
  ref(const ref &r) : m_ptr(r.m_ptr) {
    if (m_ptr)
      ((Object *)m_ptr)->incRef();
  }

  /// Move constructor
  ref(ref &&r) noexcept : m_ptr(r.m_ptr) {
    r.m_ptr = nullptr;
  }

  /// Destroy this reference
  ~ref() {
    if (m_ptr)
      ((Object *)m_ptr)->decRef();
  }

  /// Move another reference into the current one
  ref& operator=(ref&& r) noexcept {
    if (&r != this) {
      if (m_ptr)
        ((Object *)m_ptr)->decRef();
      m_ptr = r.m_ptr;
      r.m_ptr = nullptr;
    }
    return *this;
  }

  /// Overwrite this reference with another reference
  ref& operator=(const ref& r) noexcept {
    if (m_ptr != r.m_ptr) {
      if (r.m_ptr)
        ((Object *)r.m_ptr)->incRef();
      if (m_ptr)
        ((Object *)m_ptr)->decRef();
      m_ptr = r.m_ptr;
    }
    return *this;
  }

  /// Overwrite this reference with a pointer to another object
  ref& operator=(T *ptr) noexcept {
    if (m_ptr != ptr) {
      if (ptr)
        ((Object *)ptr)->incRef();
      if (m_ptr)
        ((Object *)m_ptr)->decRef();
      m_ptr = ptr;
    }
    return *this;
  }

  /// Compare this reference with another reference
  bool operator==(const ref &r) const { return m_ptr == r.m_ptr; }

  /// Compare this reference with another reference
  bool operator!=(const ref &r) const { return m_ptr != r.m_ptr; }

  /// Compare this reference with a pointer
  bool operator==(const T* ptr) const { return m_ptr == ptr; }

  /// Compare this reference with a pointer
  bool operator!=(const T* ptr) const { return m_ptr != ptr; }

  /// Access the object referenced by this reference
  T* operator->() { return m_ptr; }

  /// Access the object referenced by this reference
  const T* operator->() const { return m_ptr; }

  /// Return a C++ reference to the referenced object
  T& operator*() { return *m_ptr; }

  /// Return a const C++ reference to the referenced object
  const T& operator*() const { return *m_ptr; }

  /// Return a pointer to the referenced object
  operator T* () { return m_ptr; }

  /// Return a const pointer to the referenced object
  T* get() { return m_ptr; }

  /// Return a pointer to the referenced object
  const T* get() const { return m_ptr; }

  /// Check if the object is defined
  operator bool() const { return m_ptr != nullptr; }
private:
  T *m_ptr = nullptr;
};

class  Color 
{
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

    Color rgb() const { return Color(_d.r, _d.g, _d.b, 0.f); }

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

    SDL_Color toSdlColor() const;
    NVGcolor toNvgColor() const;

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

struct PntRect { int x1, y1, x2, y2; };
struct PntFRect { float x1, y1, x2, y2; };

SDL_Rect clip_rects(SDL_Rect af, const SDL_Rect& bf);
PntRect clip_rects(PntRect a, const PntRect& b);
PntRect srect2pntrect(const SDL_Rect& srect);
SDL_Rect pntrect2srect(const PntRect& frect);

std::string  file_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes, bool save);


namespace math
{
    //! Constant for PI.
    const double PI64       = 3.1415926535897932384626433832795028841971693993751;

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
};

template <class T>
class Vector2
{
public:
  //! Default constructor (null vector)
  Vector2() : x(0), y(0) {}
  //! Constructor with two different values
  Vector2(T nx, T ny) : x(nx), y(ny) {}
  //! Constructor with the same value for both members
  explicit Vector2(T n) : x(n), y(n) {}
  //! Copy constructor
  Vector2(const Vector2<T>& o) : x(o.x), y(o.y) {}

  static Vector2 Constant(T v) { return Vector2(v,v); }
  static Vector2 Zero() { return Vector2(0,0); }

  // operators
  Vector2<T> operator-() const { return Vector2<T>(-x, -y); }

  Vector2<T>& operator=(const Vector2<T>& o) { x = o.x; y = o.y; return *this; }

  Vector2<T> operator+(const Vector2<T>& o) const { return Vector2<T>(x + o.x, y + o.y); }
  Vector2<T>& operator+=(const Vector2<T>& o) { x+=o.x; y+=o.y; return *this; }
  Vector2<T> operator+(const T v) const { return Vector2<T>(x + v, y + v); }
  Vector2<T>& operator+=(const T v) { x+=v; y+=v; return *this; }

  Vector2<T> operator-(const Vector2<T>& o) const { return Vector2<T>(x - o.x, y - o.y); }
  Vector2<T>& operator-=(const Vector2<T>& o) { x-=o.x; y-=o.y; return *this; }
  Vector2<T> operator-(const T v) const { return Vector2<T>(x - v, y - v); }
  Vector2<T>& operator-=(const T v) { x-=v; y-=v; return *this; }

  Vector2<T> operator*(const Vector2<T>& o) const { return Vector2<T>(x * o.x, y * o.y); }
  Vector2<T>& operator*=(const Vector2<T>& o) { x*=o.x; y*=o.y; return *this; }
  Vector2<T> operator*(const T v) const { return Vector2<T>(x * v, y * v); }
  Vector2<T>& operator*=(const T v) { x*=v; y*=v; return *this; }

  Vector2<T> operator/(const Vector2<T>& o) const { return Vector2<T>(x / o.x, y / o.y); }
  Vector2<T>& operator/=(const Vector2<T>& o) { x/=o.x; y/=o.y; return *this; }
  Vector2<T> operator/(const T v) const { return Vector2<T>(x / v, y / v); }
  Vector2<T>& operator/=(const T v) { x/=v; y/=v; return *this; }

  //! sort in order X, Y. Equality with rounding tolerance.
  bool operator<=(const Vector2<T>& o) const
  {
    return (x<o.x || math::isEqual(x, o.x)) ||
           (math::isEqual(x, o.x) && (y<o.y || math::isEqual(y, o.y)));
  }

  bool positive() const { return x >= 0 && y >= 0; }

  bool lessOrEq(const Vector2<T>& o) const
  {
    return (x<o.x || math::isEqual(x, o.x)) 
           && (y<o.y || math::isEqual(y, o.y));
  }

  //! sort in order X, Y. Equality with rounding tolerance.
  bool operator>=(const Vector2<T>&o) const
  {
    return (x>o.x || math::isEqual(x, o.x)) ||
            (math::isEqual(x, o.x) && (y>o.Y || math::isEqual(y, o.y)));
  }

  //! sort in order X, Y. Difference must be above rounding tolerance.
  bool operator<(const Vector2<T>&o) const
  {
    return (x<o.x && !math::isEqual(x, o.x)) ||
           (math::isEqual(x, o.x) && y<o.y && !math::isEqual(y, o.y));
  }

  //! sort in order X, Y. Difference must be above rounding tolerance.
  bool operator>(const Vector2<T>&o) const
  {
    return (x>o.x && !math::isEqual(x, o.x)) ||
           (math::isEqual(x, o.x) && y>o.y && !math::isEqual(y, o.y));
  }

  bool operator==(const Vector2<T>& o) const { return IsEqual(o, math::ROUNDING_ERROR_f32); }
  bool operator!=(const Vector2<T>& o) const { return !IsEqual(o, math::ROUNDING_ERROR_f32); }

  Vector2<T> cquotient(const Vector2<T>& o) const { return Vector2<T>(x/(float)o.x, y/(float)o.y); }

  T minCoeff() const { return x > y ? y : x; }

  // functions

  //! Checks if this vector equals the o one.
  /** Takes floating point rounding errors into account.
  \param o Vector to compare with.
  \return True if the two vector are (almost) equal, else false. */
  bool IsEqual(const Vector2<T>& o, float tolerance) const
  {
    return math::isEqual<T>(x, o.x, tolerance) && math::isEqual<T>(y, o.y, tolerance);
  }

  Vector2<T>& set(T nx, T ny) {x=nx; y=ny; return *this; }
  Vector2<T>& set(const Vector2<T>& p) { x=p.x; y=p.y; return *this; }

  //! Gets the length of the vector.
  /** \return The length of the vector. */
  float getLength() const { return sqrt( x*x + y*y ); }

  //! Get the squared length of this vector
  /** This is useful because it is much faster than getLength().
  \return The squared length of the vector. */
  T getLengthSQ() const { return x*x + y*y; }

  //! Get the dot product of this vector with ano.
  /** \param o o vector to take dot product with.
  \return The dot product of the two vectors. */
  T dotProduct(const Vector2<T>& o) const
  {
    return x*o.x + y*o.y;
  }

  template< class A >
  Vector2<A> As()
  {
    return Vector2<A>( (A)x, (A)y );
  }

  template< class A >
  Vector2<A> As() const
  {
    return Vector2<A>( (A)x, (A)y );
  }

  //! Gets distance from ano point.
  /** Here, the vector is interpreted as a point in 2-dimensional space.
  \param o o vector to measure from.
  \return Distance from o point. */
  float getDistanceFrom(const Vector2<T>& o) const
  {
          return Vector2<T>(x - o.x, y - o.y).getLength();
  }

  //! Returns squared distance from ano point.
  /** Here, the vector is interpreted as a point in 2-dimensional space.
  \param o o vector to measure from.
  \return Squared distance from o point. */
  T getDistanceFromSQ(const Vector2<T>& o) const
  {
          return Vector2<T>(x - o.x, y - o.y).getLengthSQ();
  }

  //! rotates the point anticlockwise around a center by an amount of degrees.
  /** \param degrees Amount of degrees to rotate by, anticlockwise.
  \param center Rotation center.
  \return This vector after transformation. */
  Vector2<T>& rotateBy(float degrees, const Vector2<T>& center=Vector2<T>())
  {
    degrees *= math::DEGTORAD64;
            const float cs = cos(degrees);
            const float sn = sin(degrees);

            x -= center.x;
            y -= center.y;

            set((T)(x*cs - y*sn), (T)(x*sn + y*cs));

            x += center.x;
            y += center.y;
            return *this;
  }

  //! Normalize the vector.
  /** The null vector is left untouched.
  \return Reference to this vector, after normalization. */
  Vector2<T>& normalize()
  {
    float length = (float)(x*x + y*y);

    if (math::isEqual(length, 0.f))
                    return *this;
    length = 1.f / sqrt( length );
    x = (T)(x * length);
    y = (T)(y * length);
        return *this;
  }

  //! Calculates the angle of this vector in degrees in the trigonometric sense.
  /** 0 is to the right (3 o'clock), values increase counter-clockwise.
  This method has been suggested by Pr3t3nd3r.
  \return Returns a value between 0 and 360. */
  float getAngleTrig() const
  {
    if (y == 0)
      return x < 0 ? 180 : 0;
    else if (x == 0)
      return y < 0 ? 270 : 90;

    if ( y > 0)
    {
      if (x > 0)
        return atanf((float)y/(float)x) * math::RADTODEG64;
      else
        return 180.0-atanf((float)y/-(float)x) * math::RADTODEG64;
    }
    else
    {
      if (x > 0)
        return 360.0-atanf(-(float)y/(float)x) * math::RADTODEG64;
      else
        return 180.0+atanf(-(float)y/-(float)x) * math::RADTODEG64;
    }
  }

  //! Calculates the angle of this vector in degrees in the counter trigonometric sense.
  /** 0 is to the right (3 o'clock), values increase clockwise.
  \return Returns a value between 0 and 360. */
  inline float getAngle() const
  {
    if (y == 0) // corrected thanks to a suggestion by Jox
            return x < 0 ? 180 : 0;
    else if (x == 0)
            return y < 0 ? 90 : 270;

    // don't use getLength here to avoid precision loss with s32 vectors
    float tmp = y / sqrt((float)(x*x + y*y));
    tmp = atanf( sqrt(1.f - tmp*tmp) / tmp) * math::RADTODEG64;

    if (x>0 && y>0)
      return tmp + 270;
    else if (x>0 && y<0)
      return tmp + 90;
    else if (x<0 && y<0)
      return 90 - tmp;
    else if (x<0 && y>0)
      return 270 - tmp;

    return tmp;
  }

  //! Calculates the angle between this vector and ano one in degree.
  /** \param b o vector to test with.
  \return Returns a value between 0 and 90. */
  inline float getAngleWith(const Vector2<T>& b) const
  {
    double tmp = x*b.x + y*b.y;

    if (tmp == 0.0)
      return 90.0;

    tmp = tmp / sqrtf((float)((x*x + y*y) * (b.x*b.x + b.y*b.y)));
    if (tmp < 0.0)
      tmp = -tmp;

    return atanf(sqrtf(1 - tmp*tmp) / tmp) * math::RADTODEG64;
  }

  //! Returns if this vector interpreted as a point is on a line between two o points.
  /** It is assumed that the point is on the line.
  \param begin Beginning vector to compare between.
  \param end Ending vector to compare between.
  \return True if this vector is between begin and end, false if not. */
  bool isBetweenPoints(const Vector2<T>& begin, const Vector2<T>& end) const
  {
    if (begin.x != end.x)
    {
      return ((begin.x <= x && x <= end.x) ||
              (begin.x >= x && x >= end.x));
    }
    else
    {
      return ((begin.y <= y && y <= end.y) ||
              (begin.y >= y && y >= end.y));
    }
  }

  T& operator[](int index)
  {
    return index == 0 ? x : y;
  }

  const T& operator[](int index) const
  {
    return index == 0 ? x : y;
  }

  //! Creates an interpolated vector between this vector and ano vector.
  /** \param o The o vector to interpolate with.
  \param d Interpolation value between 0.0f (all the o vector) and 1.0f (all this vector).
  Note that this is the opposite direction of interpolation to getInterpolated_quadratic()
  \return An interpolated vector.  This vector is not modified. */
  Vector2<T> getInterpolated(const Vector2<T>& o, T d) const
  {
    float inv = 1.0f - (float)d;
    return Vector2<T>((T)(o.x*inv + x*d), (T)(o.y*inv + y*d));
  }

  //! Creates a quadratically interpolated vector between this and two o vectors.
  /** \param v2 Second vector to interpolate with.
  \param v3 Third vector to interpolate with (maximum at 1.0f)
  \param d Interpolation value between 0.0f (all this vector) and 1.0f (all the 3rd vector).
  Note that this is the opposite direction of interpolation to getInterpolated() and interpolate()
  \return An interpolated vector. This vector is not modified. */
  Vector2<T> getInterpolated_quadratic(const Vector2<T>& v2, const Vector2<T>& v3, const T d) const
  {
    // this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
    const float inv = 1.0f - d;
    const float mul0 = inv * inv;
    const float mul1 = 2.0f * d * inv;
    const float mul2 = d * d;

    return Vector2<T> ( (T)(x * mul0 + v2.x * mul1 + v3.x * mul2),
                        (T)(y * mul0 + v2.y * mul1 + v3.y * mul2));
  }

  Vector2<T> cmax(T rx, T ry) const
  {
      return Vector2<T>(std::max(x, rx), std::max(y, ry));
  }

  Vector2<T> cmax(const Vector2<T>& o) const
  {
      return Vector2<T>(std::max(x, o.x), std::max(y, o.y));
  }

  Vector2<T> cmin(const Vector2<T>& o) const
  {
      return Vector2<T>(std::min(x, o.x), std::min(y, o.y));
  }

  //! Sets this vector to the linearly interpolated vector between a and b.
  /** \param a first vector to interpolate with, maximum at 1.0f
  \param b second vector to interpolate with, maximum at 0.0f
  \param d Interpolation value between 0.0f (all vector b) and 1.0f (all vector a)
  Note that this is the opposite direction of interpolation to getInterpolated_quadratic()
  */
  Vector2<T>& interpolate(const Vector2<T>& a, const Vector2<T>& b, const T d)
  {
    x = (T)((float)b.x + ( ( a.x - b.x ) * d ));
    y = (T)((float)b.y + ( ( a.y - b.y ) * d ));
    return *this;
  }

  template <class B>
  inline Vector2<B> cast() const { return Vector2<B>(static_cast<B>(x), static_cast<B>(y)); }

  inline Vector2<float> tofloat() const { return cast<float>(); }
  inline Vector2<int> toint() const { return cast<int>(); }

  inline Vector2<float> floor() const { return Vector2<float>(std::floor(x), std::floor(y)); }
  inline Vector2<float> ceil() const { return Vector2<float>(std::ceil(x), std::ceil(y)); }


  T x = 0, y = 0;
};

typedef Vector2<int> Vector2i;
typedef Vector2<float> Vector2f;

std::array<char, 8> utf8(int c);
/// Determine whether an icon ID is a texture loaded via nvgImageIcon
inline bool nvgIsImageIcon(int value) { return value < 1024; }

/// Determine whether an icon ID is a font-based icon (e.g. from the entypo.ttf font)
inline bool nvgIsFontIcon(int value) { return value >= 1024; }

NAMESPACE_END(sdlgui)

#endif
