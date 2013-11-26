#ifndef TMJ_MATRIX_H
#define TMJ_MATRIX_H

#include <stdio.h>
#include <Inventor/SbBasic.h>
#include <Inventor/SbLinear.h>
#include "TmjVec3d.h"

typedef double SdMat[4][4]; //double[4][4] = SdMat

class SdMatrix //4*4 matrix
{
public:
  SdMatrix(void);
  SdMatrix(const double a11, const double a12, const double a13, const double a14,
           const double a21, const double a22, const double a23, const double a24,
           const double a31, const double a32, const double a33, const double a34,
           const double a41, const double a42, const double a43, const double a44);
  SdMatrix(const SdMat & matrix);
  SdMatrix(const SdMat * matrix);
  ~SdMatrix(void);

  SdMatrix & operator =(const SdMat & m);

  operator double*(void);
  SdMatrix & operator =(const SdMatrix & m);
  void setValue(const SdMat & m);
  const SdMat & getValue(void) const;

  void makeIdentity(void);
  SdMatrix inverse(void) const;
  double det3(int r1, int r2, int r3,
             int c1, int c2, int c3) const;
  double det3(void) const;
  double det4(void) const;

  SbBool equals(const SdMatrix & m, double tolerance) const;


  operator SdMat&(void);
  double * operator [](int i);
  const double * operator [](int i) const;
   SdMatrix & operator *=(const SdMatrix & m);
  friend SdMatrix operator *(const SdMatrix & m1, const SdMatrix & m2);
  friend int operator ==(const SdMatrix & m1, const SdMatrix & m2);
  friend int operator !=(const SdMatrix & m1, const SdMatrix & m2);
  void getValue(SdMat & m) const;
  static SdMatrix identity(void);
  void setScale(const double s);
  void setScale(const SbVec3d & s);
  void setTranslate(const SbVec3d & t);
  SbBool factor(SdMatrix & r, SbVec3d & s, SdMatrix & u, SbVec3d & t,
                SdMatrix & proj);
  SdMatrix transpose(void) const;
  SdMatrix & multRight(const SdMatrix & m);
  SdMatrix & multLeft(const SdMatrix & m);
  void multMatrixVec(const SbVec3d & src, SbVec3d & dst) const;
  void multVecMatrix(const SbVec3d & src, SbVec3d & dst) const;
  void multDirMatrix(const SbVec3d & src, SbVec3d & dst) const;
//  void multVecMatrix(const SbVec4f & src, SbVec4f & dst) const;

  void print(FILE * fp) const;
  void setSbMatrix(SbMatrix & dst);

private:
  double matrix[4][4];

  void operator /=(const double v);
  void operator *=(const double v);
};

SdMatrix operator *(const SdMatrix & m1, const SdMatrix & m2);
int operator ==(const SdMatrix & m1, const SdMatrix & m2);
int operator !=(const SdMatrix & m1, const SdMatrix & m2);

#endif // TMJ_MATRIX_H
