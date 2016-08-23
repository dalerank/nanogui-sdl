//
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#ifndef __PICOGUI_VECTOR2_INCLUDED__
#define __PICOGUI_VECTOR2_INCLUDED__

//#include "math.hpp"
#include <include/common.h>

NAMESPACE_BEGIN(nanogui)

template <class T>
class Vector2
{
public:
  //! Default constructor (null vector)
  Vector2() : _x(0), _y(0) {}
  //! Constructor with two different values
  Vector2(T nx, T ny) : _x(nx), _y(ny) {}
  //! Constructor with the same value for both members
  explicit Vector2(T n) : _x(n), _y(n) {}
  //! Copy constructor
  Vector2(const Vector2<T>& other) : _x(other._x), _y(other._y) {}

  // operators
  Vector2<T> operator-() const { return Vector2<T>(-_x, -_y); }

  Vector2<T>& operator=(const Vector2<T>& other) { _x = other._x; _y = other._y; return *this; }

  Vector2<T> operator+(const Vector2<T>& other) const { return Vector2<T>(_x + other._x, _y + other._y); }
  Vector2<T>& operator+=(const Vector2<T>& other) { _x+=other._x; _y+=other._y; return *this; }
  Vector2<T> operator+(const T v) const { return Vector2<T>(_x + v, _y + v); }
  Vector2<T>& operator+=(const T v) { _x+=v; _y+=v; return *this; }

  Vector2<T> operator-(const Vector2<T>& other) const { return Vector2<T>(_x - other._x, _y - other._y); }
  Vector2<T>& operator-=(const Vector2<T>& other) { _x-=other._x; _y-=other._y; return *this; }
  Vector2<T> operator-(const T v) const { return Vector2<T>(_x - v, _y - v); }
  Vector2<T>& operator-=(const T v) { _x-=v; _y-=v; return *this; }

  Vector2<T> operator*(const Vector2<T>& other) const { return Vector2<T>(_x * other._x, _y * other._y); }
  Vector2<T>& operator*=(const Vector2<T>& other) { _x*=other._x; _y*=other._y; return *this; }
  Vector2<T> operator*(const T v) const { return Vector2<T>(_x * v, _y * v); }
  Vector2<T>& operator*=(const T v) { _x*=v; _y*=v; return *this; }

  Vector2<T> operator/(const Vector2<T>& other) const { return Vector2<T>(_x / other._x, _y / other._y); }
  Vector2<T>& operator/=(const Vector2<T>& other) { _x/=other._x; _y/=other._y; return *this; }
  Vector2<T> operator/(const T v) const { return Vector2<T>(_x / v, _y / v); }
  Vector2<T>& operator/=(const T v) { _x/=v; _y/=v; return *this; }

  //! sort in order X, Y. Equality with rounding tolerance.
  bool operator<=(const Vector2<T>& other) const
  {
    return (_x<other._x || math::isEqual(_x, other._x)) ||
           (math::isEqual(_x, other._x) && (_y<other._y || math::isEqual(_y, other._y)));
  }

  //! sort in order X, Y. Equality with rounding tolerance.
  bool operator>=(const Vector2<T>&other) const
  {
    return (_x>other._x || math::isEqual(_x, other._x)) ||
            (math::isEqual(_x, other._x) && (_y>other.Y || math::isEqual(_y, other._y)));
  }

  //! sort in order X, Y. Difference must be above rounding tolerance.
  bool operator<(const Vector2<T>&other) const
  {
    return (_x<other._x && !math::isEqual(_x, other._x)) ||
           (math::isEqual(_x, other.X) && _y<other.Y && !math::isEqual(_y, other._y));
  }

  //! sort in order X, Y. Difference must be above rounding tolerance.
  bool operator>(const Vector2<T>&other) const
  {
    return (_x>other._x && !math::isEqual(_x, other._x)) ||
           (math::isEqual(_x, other._x) && _y>other._y && !math::isEqual(_y, other._y));
  }

  bool operator==(const Vector2<T>& other) const { return IsEqual(other, math::ROUNDING_ERROR_f32); }
  bool operator!=(const Vector2<T>& other) const { return !IsEqual(other, math::ROUNDING_ERROR_f32); }

  // functions

  //! Checks if this vector equals the other one.
  /** Takes floating point rounding errors into account.
  \param other Vector to compare with.
  \return True if the two vector are (almost) equal, else false. */
  bool IsEqual(const Vector2<T>& other, float tolerance) const
  {
    return math::isEqual<T>(_x, other._x, tolerance) && math::isEqual<T>(_y, other._y, tolerance);
  }

  Vector2<T>& set(T nx, T ny) {_x=nx; _y=ny; return *this; }
  Vector2<T>& set(const Vector2<T>& p) { _x=p._x; _y=p._y; return *this; }

  //! Gets the length of the vector.
  /** \return The length of the vector. */
  float getLength() const { return sqrt( (float)_x*(float)_x + (float)_y*(float)_y ); }

  //! Get the squared length of this vector
  /** This is useful because it is much faster than getLength().
  \return The squared length of the vector. */
  T getLengthSQ() const { return _x*_x + _y*_y; }

  //! Get the dot product of this vector with another.
  /** \param other Other vector to take dot product with.
  \return The dot product of the two vectors. */
  T dotProduct(const Vector2<T>& other) const
  {
    return _x*other._x + _y*other._y;
  }

  template< class A >
  Vector2<A> As()
  {
    return Vector2<A>( (A)_x, (A)_y );
  }

  template< class A >
  Vector2<A> As() const
  {
    return Vector2<A>( (A)_x, (A)_y );
  }

  //! Gets distance from another point.
  /** Here, the vector is interpreted as a point in 2-dimensional space.
  \param other Other vector to measure from.
  \return Distance from other point. */
  float getDistanceFrom(const Vector2<T>& other) const
  {
          return Vector2<T>(_x - other._x, _y - other._y).getLength();
  }

  //! Returns squared distance from another point.
  /** Here, the vector is interpreted as a point in 2-dimensional space.
  \param other Other vector to measure from.
  \return Squared distance from other point. */
  T getDistanceFromSQ(const Vector2<T>& other) const
  {
          return Vector2<T>(_x - other._x, _y - other._y).getLengthSQ();
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

            _x -= center._x;
            _y -= center._y;

            set((T)(_x*cs - _y*sn), (T)(_x*sn + _y*cs));

            _x += center._x;
            _y += center._y;
            return *this;
  }

  //! Normalize the vector.
  /** The null vector is left untouched.
  \return Reference to this vector, after normalization. */
  Vector2<T>& normalize()
  {
    float length = (float)(_x*_x + _y*_y);

    if (math::isEqual(length, 0.f))
                    return *this;
    length = 1.f / sqrt( length );
    _x = (T)(_x * length);
    _y = (T)(_y * length);
		return *this;
  }

  //! Calculates the angle of this vector in degrees in the trigonometric sense.
  /** 0 is to the right (3 o'clock), values increase counter-clockwise.
  This method has been suggested by Pr3t3nd3r.
  \return Returns a value between 0 and 360. */
  float getAngleTrig() const
  {
    if (_y == 0)
      return _x < 0 ? 180 : 0;
    else if (_x == 0)
      return _y < 0 ? 270 : 90;

    if ( _y > 0)
    {
      if (_x > 0)
        return atanf((float)_y/(float)_x) * math::RADTODEG64;
      else
        return 180.0-atanf((float)_y/-(float)_x) * math::RADTODEG64;
    }
    else
    {
      if (_x > 0)
        return 360.0-atanf(-(float)_y/(float)_x) * math::RADTODEG64;
      else
        return 180.0+atanf(-(float)_y/-(float)_x) * math::RADTODEG64;
    }
  }

  //! Calculates the angle of this vector in degrees in the counter trigonometric sense.
  /** 0 is to the right (3 o'clock), values increase clockwise.
  \return Returns a value between 0 and 360. */
  inline float getAngle() const
  {
    if (_y == 0) // corrected thanks to a suggestion by Jox
            return _x < 0 ? 180 : 0;
    else if (_x == 0)
            return _y < 0 ? 90 : 270;

    // don't use getLength here to avoid precision loss with s32 vectors
    float tmp = _y / sqrt((float)(_x*_x + _y*_y));
    tmp = atanf( sqrt(1.f - tmp*tmp) / tmp) * math::RADTODEG64;

    if (_x>0 && _y>0)
      return tmp + 270;
    else if (_x>0 && _y<0)
      return tmp + 90;
    else if (_x<0 && _y<0)
      return 90 - tmp;
    else if (_x<0 && _y>0)
      return 270 - tmp;

    return tmp;
  }

  //! Calculates the angle between this vector and another one in degree.
  /** \param b Other vector to test with.
  \return Returns a value between 0 and 90. */
  inline float getAngleWith(const Vector2<T>& b) const
  {
    double tmp = _x*b._x + _y*b._y;

    if (tmp == 0.0)
      return 90.0;

    tmp = tmp / sqrtf((float)((_x*_x + _y*_y) * (b._x*b._x + b._y*b._y)));
    if (tmp < 0.0)
      tmp = -tmp;

    return atanf(sqrtf(1 - tmp*tmp) / tmp) * math::RADTODEG64;
  }

  //! Returns if this vector interpreted as a point is on a line between two other points.
  /** It is assumed that the point is on the line.
  \param begin Beginning vector to compare between.
  \param end Ending vector to compare between.
  \return True if this vector is between begin and end, false if not. */
  bool isBetweenPoints(const Vector2<T>& begin, const Vector2<T>& end) const
  {
    if (begin._x != end._x)
    {
      return ((begin._x <= _x && _x <= end._x) ||
              (begin._x >= _x && _x >= end._x));
    }
    else
    {
      return ((begin._y <= _y && _y <= end._y) ||
              (begin._y >= _y && _y >= end._y));
    }
  }

  //! Creates an interpolated vector between this vector and another vector.
  /** \param other The other vector to interpolate with.
  \param d Interpolation value between 0.0f (all the other vector) and 1.0f (all this vector).
  Note that this is the opposite direction of interpolation to getInterpolated_quadratic()
  \return An interpolated vector.  This vector is not modified. */
  Vector2<T> getInterpolated(const Vector2<T>& other, T d) const
  {
    float inv = 1.0f - (float)d;
    return Vector2<T>((T)(other._x*inv + _x*d), (T)(other._y*inv + _y*d));
  }

  //! Creates a quadratically interpolated vector between this and two other vectors.
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

    return Vector2<T> ( (T)(_x * mul0 + v2.X * mul1 + v3.X * mul2),
                        (T)(_y * mul0 + v2.Y * mul1 + v3.Y * mul2));
  }

  //! Sets this vector to the linearly interpolated vector between a and b.
  /** \param a first vector to interpolate with, maximum at 1.0f
  \param b second vector to interpolate with, maximum at 0.0f
  \param d Interpolation value between 0.0f (all vector b) and 1.0f (all vector a)
  Note that this is the opposite direction of interpolation to getInterpolated_quadratic()
  */
  Vector2<T>& interpolate(const Vector2<T>& a, const Vector2<T>& b, const T d)
  {
    _x = (T)((float)b._x + ( ( a._x - b._x ) * d ));
    _y = (T)((float)b._y + ( ( a._y - b._y ) * d ));
    return *this;
  }

  inline T x() const { return _x; }
  inline T y() const { return _y; }

  inline T& rx() { return _x; }
  inline T& ry() { return _y; }

  inline void setX( T xv ) { _x = xv; }
  inline void setY( T yv ) { _y = yv; }

protected:
  //! X coordinate of vector.
  T _x;

  //! Y coordinate of vector.
  T _y;
};


typedef Vector2<int> Vector2i;
typedef Vector2<float> Vector2f;

NAMESPACE_END(nanogui)

#endif //__PICOGUI_VECTOR2_INCLUDED__
