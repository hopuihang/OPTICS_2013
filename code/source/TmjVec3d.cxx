#include "TmjVec3d.h"

/*!
  The default constructor does nothing. The vector coordinates will be
  uninitialized until you do a setValue() call.
*/
SbVec3d::SbVec3d(void)
{
}

/*!
  Constructs an SbVec3d instance with initial values from \a v.
*/
SbVec3d::SbVec3d(const double v[3])
{
  this->vec[0] = v[0];
  this->vec[1] = v[1];
  this->vec[2] = v[2];
}

/*!
  Constructs an SbVec3d instance with the initial vector endpoint set to
  \a <x,y,z>.
*/
SbVec3d::SbVec3d(const double x, const double y, const double z)
{
  this->vec[0] = x;
  this->vec[1] = y;
  this->vec[2] = z;
}

/*!
  Returns the result of taking the cross product of this vector and \a v.
*/
SbVec3d
SbVec3d::cross(const SbVec3d & v) const
{
  return SbVec3d(this->vec[1]*v.vec[2] - this->vec[2]*v.vec[1],
                 this->vec[2]*v.vec[0] - this->vec[0]*v.vec[2],
                 this->vec[0]*v.vec[1] - this->vec[1]*v.vec[0]);
}

/*!
  Calculates and returns the result of taking the dot product of this
  vector and \a v.
*/
double
SbVec3d::dot(const SbVec3d & v) const
{
  return this->vec[0]*v.vec[0] + this->vec[1]*v.vec[1] + this->vec[2]*v.vec[2];
}

/*!
  Compares the vector with \a v and returns \c TRUE if the largest
  distance between the vectors are larger than the square root of the
  given tolerance value.
*/
SbBool
SbVec3d::equals(const SbVec3d & v, const double tolerance) const
{
  double xdist = this->vec[0] - v[0];
  double ydist = this->vec[1] - v[1];
  double zdist = this->vec[2] - v[2];

  return ((xdist*xdist + ydist*ydist + zdist*zdist) <= tolerance);
}

/*!
  Return the vector representing the principal axis closest to this
  vector.
*/
SbVec3d
SbVec3d::getClosestAxis(void) const
{
  SbVec3d closest(0.0, 0.0, 0.0);
  double xabs = (double)fabs(this->vec[0]);
  double yabs = (double)fabs(this->vec[1]);
  double zabs = (double)fabs(this->vec[2]);

  if (xabs>=yabs && xabs>=zabs) closest[0] = (this->vec[0] > 0.0) ? 1.0 : -1.0;
  else if (yabs>=zabs) closest[1] = (this->vec[1] > 0.0) ? 1.0 : -1.0;
  else closest[2] = (this->vec[2] > 0.0) ? 1.0 : -1.0;

  return closest;
}

/*!
  Returns a pointer to an array of three doubles containing the x, y
  and z coordinates of the vector.

  \sa setValue().
*/
const double *
SbVec3d::getValue(void) const
{
  return this->vec;
}

/*!
  Returns the x, y and z coordinates of the vector.

  \sa setValue().
*/
void
SbVec3d::getValue(double & x, double & y, double & z) const
{
  x = this->vec[0];
  y = this->vec[1];
  z = this->vec[2];
}

/*!
  Return length of vector.
*/
double
SbVec3d::length(void) const
{
  return (double)sqrt(this->vec[0]*this->vec[0] +
                     this->vec[1]*this->vec[1] +
                     this->vec[2]*this->vec[2]);
}

/*!
  Returns the squared length of the vector.
*/
double
SbVec3d::sqrLength(void) const
{
  return
    this->vec[0]*this->vec[0] +
    this->vec[1]*this->vec[1] +
    this->vec[2]*this->vec[2];
}

/*!
  Negate the vector (i.e. point it in the opposite direction).
*/
void
SbVec3d::negate(void)
{
  this->vec[0] = -this->vec[0];
  this->vec[1] = -this->vec[1];
  this->vec[2] = -this->vec[2];
}

/*!
  Normalize the vector to unit length. Return value is the original
  length of the vector before normalization.

  If the vector is the null vector, no attempt at normalization will
  be done.
*/
double
SbVec3d::normalize(void)
{
  double len = this->length();
  if (len > 0.0) operator/=(len);
  return len;
}

/*!
  Set new coordinates for the vector from \a v. Returns reference to
  self.

  \sa getValue().
*/
SbVec3d &
SbVec3d::setValue(const double v[3])
{
  this->vec[0] = v[0];
  this->vec[1] = v[1];
  this->vec[2] = v[2];
  return *this;
}

/*!
  Set new coordinates for the vector. Returns reference to self.

  \sa getValue().
*/
SbVec3d &
SbVec3d::setValue(const double x, const double y, const double z)
{
  this->vec[0] = x;
  this->vec[1] = y;
  this->vec[2] = z;
  return *this;
}

/*!
  Set this vector to be the average of \a v0, \a v1 and \a v2.
  The vector components are weighted by the \a barycentric vector.

  \sa getValue().
*/
SbVec3d &
SbVec3d::setValue(const SbVec3d & barycentric,
                  const SbVec3d & v0, const SbVec3d & v1, const SbVec3d & v2)
{
  this->vec[0] = barycentric[0]*v0[0]+barycentric[1]*v1[0]+barycentric[2]*v2[0];
  this->vec[1] = barycentric[0]*v0[1]+barycentric[1]*v1[1]+barycentric[2]*v2[1];
  this->vec[2] = barycentric[0]*v0[2]+barycentric[1]*v1[2]+barycentric[2]*v2[2];
  return *this;
}

/*!
  Multiply components of vector with value \a d. Returns reference to self.
*/
SbVec3d &
SbVec3d::operator *=(const double d)
{
  this->vec[0] *= d;
  this->vec[1] *= d;
  this->vec[2] *= d;
  return *this;
}

/*!
  Divides components of vector with value \a d. Returns reference to self.
*/
SbVec3d &
SbVec3d::operator /=(const double d)
{
  // Assumes 1 div and 3 muls is quicker than 3 divs.
  double inv = 1.0/d;
  this->vec[0] *= inv;
  this->vec[1] *= inv;
  this->vec[2] *= inv;
  return *this;
}

/*!
  Adds this vector and vector \a u. Returns reference to self.
*/
SbVec3d &
SbVec3d::operator +=(const SbVec3d & u)
{
  this->vec[0] += u.vec[0];
  this->vec[1] += u.vec[1];
  this->vec[2] += u.vec[2];
  return *this;
}

/*!
  Subtracts vector \a u from this vector. Returns reference to self.
*/
SbVec3d &
SbVec3d::operator -=(const SbVec3d & u)
{
  this->vec[0] -= u.vec[0];
  this->vec[1] -= u.vec[1];
  this->vec[2] -= u.vec[2];
  return *this;
}

/*!
  Non-destructive negation operator. Returns a new SbVec3d instance
  which points in the opposite direction of this vector.

  \sa negate().
*/
SbVec3d
SbVec3d::operator -(void) const
{
  return SbVec3d(-this->vec[0], -this->vec[1], -this->vec[2]);
}

/*!
  \relates SbVec3d

  Returns an SbVec3d instance which is the components of vector \a v
  multiplied with \a d.
*/
SbVec3d
operator *(const SbVec3d & v, const double d)
{
  return SbVec3d(v.vec[0] * d, v.vec[1] * d, v.vec[2] * d);
}

/*!
  \relates SbVec3d

  Returns an SbVec3d instance which is the components of vector \a v
  multiplied with \a d.
*/
SbVec3d
operator *(const double d, const SbVec3d & v)
{
  return v * d;
}

/*!
  \relates SbVec3d

  Returns an SbVec3d instance which is the components of vector \a v
  divided on the scalar factor \a d.
*/
SbVec3d
operator /(const SbVec3d & v, const double d)
{
  return SbVec3d(v.vec[0] / d, v.vec[1] / d, v.vec[2] / d);
}

/*!
  \relates SbVec3d

  Returns an SbVec3d instance which is the sum of vectors \a v1 and \a v2.
*/
SbVec3d
operator +(const SbVec3d & v1, const SbVec3d & v2)
{
  return SbVec3d(v1.vec[0] + v2.vec[0],
                 v1.vec[1] + v2.vec[1],
                 v1.vec[2] + v2.vec[2]);
}

/*!
  \relates SbVec3d

  Returns an SbVec3d instance which is vector \a v2 subtracted from
  vector \a v1.
*/
SbVec3d
operator -(const SbVec3d & v1, const SbVec3d & v2)
{
  return SbVec3d(v1.vec[0] - v2.vec[0],
                 v1.vec[1] - v2.vec[1],
                 v1.vec[2] - v2.vec[2]);
}

/*!
  \relates SbVec3d

  Returns \a 1 if \a v1 and \a v2 are \e exactly equal, \a 0 otherwise.

  \sa equals().
*/
int
operator ==(const SbVec3d & v1, const SbVec3d & v2)
{
  if (v1.vec[0] == v2.vec[0] &&
     v1.vec[1] == v2.vec[1] &&
     v1.vec[2] == v2.vec[2]) return TRUE;
  return FALSE;
}

/*!
  \relates SbVec3d

  Returns \a 1 if \a v1 and \a v2 are not equal, \a 0 if they are equal.

  \sa equals().
*/
int
operator !=(const SbVec3d & v1, const SbVec3d & v2)
{
  return !(v1 == v2);
}

/*!
  Dump the state of this object to the \a file stream. Only works in
  debug version of library, method does nothing in an optimized
  compile.
*/
void
SbVec3d::print(FILE * fp) const
{
  (void)fprintf(fp, "<%f, %f, %f>", this->vec[0], this->vec[1], this->vec[2]);
}
