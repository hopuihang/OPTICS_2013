#ifndef TMJ_VEC3D_H
#define TMJ_VEC3D_H

#include <stdio.h>
#include <Inventor/SbBasic.h>

class SbVec3d { //group of 3 double
public:
  SbVec3d(void);
  SbVec3d(const double v[3]);
  SbVec3d(const double x, const double y, const double z);
  SbVec3d cross(const SbVec3d & v) const;
  double dot(const SbVec3d & v) const;
  SbBool equals(const SbVec3d & v, const double tolerance) const;
  SbVec3d getClosestAxis(void) const;
  const double * getValue(void) const;
  void getValue(double & x, double & y, double & z) const;
  double length(void) const;
  double sqrLength() const;
  void negate(void);
  double normalize(void);
  SbVec3d & setValue(const double v[3]);
  SbVec3d & setValue(const double x, const double y, const double z);
  SbVec3d & setValue(const SbVec3d & barycentric,
                     const SbVec3d & v0,
                     const SbVec3d & v1,
                     const SbVec3d & v2);
  double & operator [](const int i);
  const double & operator [](const int i) const;
  SbVec3d & operator *=(const double d);
  SbVec3d & operator /=(const double d);
  SbVec3d & operator +=(const SbVec3d & u);
  SbVec3d & operator -=(const SbVec3d & u);
  SbVec3d operator -(void) const;
  friend SbVec3d operator *(const SbVec3d & v, const double d);
  friend SbVec3d operator *(const double d, const SbVec3d & v);
  friend SbVec3d operator /(const SbVec3d & v, const double d);
  friend SbVec3d operator +(const SbVec3d & v1, const SbVec3d & v2);
  friend SbVec3d operator -(const SbVec3d & v1, const SbVec3d & v2);
  friend int operator ==(const SbVec3d & v1, const SbVec3d & v2);
  friend int operator !=(const SbVec3d & v1, const SbVec3d & v2);

  void print(FILE * fp) const;

private:
  double vec[3];
};

SbVec3d operator *(const SbVec3d & v, const double d);
SbVec3d operator *(const double d, const SbVec3d & v);
SbVec3d operator /(const SbVec3d & v, const double d);
SbVec3d operator +(const SbVec3d & v1, const SbVec3d & v2);
SbVec3d operator -(const SbVec3d & v1, const SbVec3d & v2);
int operator ==(const SbVec3d & v1, const SbVec3d & v2);
int operator !=(const SbVec3d & v1, const SbVec3d & v2);


/* inlined methods ********************************************************/

inline double &
SbVec3d::operator [](const int i)
{
  return this->vec[i];
}

inline const double &
SbVec3d::operator [](const int i) const
{
  return this->vec[i];
}

#endif // TMJ_VEC3D_H
