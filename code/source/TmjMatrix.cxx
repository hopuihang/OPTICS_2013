#include "TmjMatrix.h"
//#include <string.h>

/*!
  The default constructor does nothing. The matrix will be uninitialized.
 */
SdMatrix::SdMatrix(void)
{
}


/*!
  Constructs a matrix instance with the given initial elements.
 */
SdMatrix::SdMatrix(const double a11, const double a12,
                   const double a13, const double a14,
                   const double a21, const double a22,
                   const double a23, const double a24,
                   const double a31, const double a32,
                   const double a33, const double a34,
                   const double a41, const double a42,
                   const double a43, const double a44)
{
  const SdMat m = { { a11, a12, a13, a14 },
                    { a21, a22, a23, a24 },
                    { a31, a32, a33, a34 },
                    { a41, a42, a43, a44 } };
  this->setValue(m);
}

/*!
  Constructs a matrix instance with the initial elements from the/a matrix argument.
 */
SdMatrix::SdMatrix(const SdMat & matrix)
{
  this->setValue(matrix);
}

/*!
  This constructor is courtesy of the Microsoft Visual C++ compiler.
*/
SdMatrix::SdMatrix(const SdMat * matrix)
{
  this->setValue(*matrix);
}

/*!
  Default destructor does nothing.
 */
SdMatrix::~SdMatrix(void)
{
}

/*!
  Returns a pointer to the 2 dimensional double array with the matrix
  elements.

  \sa setValue().
 */
const SdMat &
SdMatrix::getValue(void) const
{
  return this->matrix;
}

/*!
  Copies the elements from \a m into the matrix.

  \sa getValue().
 */
void
SdMatrix::setValue(const SdMat & m)
{
  (void)memmove(this->matrix, m, sizeof(double)*4*4);
}

/*!
  Assignment operator. Copies the elements from \a m to the matrix.
 */
SdMatrix &
SdMatrix::operator=(const SdMat & m)
{
  this->setValue(m);
  return *this;
}

/*!
  Assignment operator. Copies the elements from \a m to the matrix.
 */
SdMatrix &
SdMatrix::operator=(const SdMatrix & m)
{
  this->setValue(m.matrix);
  return *this;
}

/*!
  Set the matrix to be the identity matrix.

  \sa identity().
 */
void
SdMatrix::makeIdentity(void)
{
  this->matrix[0][0]=this->matrix[1][1]=
  this->matrix[2][2]=this->matrix[3][3] = 1.0f;

  this->matrix[0][1]=this->matrix[0][2]=this->matrix[0][3]=
  this->matrix[1][0]=this->matrix[1][2]=this->matrix[1][3]=
  this->matrix[2][0]=this->matrix[2][1]=this->matrix[2][3]=
  this->matrix[3][0]=this->matrix[3][1]=this->matrix[3][2] = 0.0f;
}

/*!
  Multiply all element values in the matrix with \a v.
 */
void
SdMatrix::operator*=(const double v)
{
  for (int i=0; i < 4; i++) {
    for (int j=0; j < 4; j++) {
      this->matrix[i][j] *= v;
    }
  }
}

/*!
  Divide all element values in the matrix on \a v.
 */
void
SdMatrix::operator/=(const double v)
{
  this->operator*=(1.0f/v);
}

/*!
  Returns the determinant of the 3x3 submatrix specified by the row and
  column indices.
 */
//r1,r2,r3,c1,c2,c3;get a matrix,get determinant
double SdMatrix::det3(int r1, int r2, int r3,
					  int c1, int c2, int c3) const
{
  // More or less directly from "Advanced Engineering Mathematics"
  // (E. Kreyszig), 6th edition.

  double a11 = this->matrix[r1][c1];
  double a12 = this->matrix[r1][c2];
  double a13 = this->matrix[r1][c3];
  double a21 = this->matrix[r2][c1];
  double a22 = this->matrix[r2][c2];
  double a23 = this->matrix[r2][c3];
  double a31 = this->matrix[r3][c1];
  double a32 = this->matrix[r3][c2];
  double a33 = this->matrix[r3][c3];

  double M11 = a22 * a33 - a32 * a23;
  double M21 = -(a12 * a33 - a32 * a13);
  double M31 = a12 * a23 - a22 * a13;

  return (a11 * M11 + a21 * M21 + a31 * M31);
}

/*!
  Returns the determinant of the upper left 3x3 submatrix.
 */
double
SdMatrix::det3(void) const
{
  return this->det3(0, 1, 2, 0, 1, 2);
}

/*!
  Returns the determinant of the matrix.
 */
double
SdMatrix::det4(void) const
{
  double det = 0.0f;
  det += this->matrix[0][0] * det3(1, 2, 3, 1, 2, 3);
  det -= this->matrix[1][0] * det3(0, 2, 3, 1, 2, 3);
  det += this->matrix[2][0] * det3(0, 1, 3, 1, 2, 3);
  det -= this->matrix[3][0] * det3(0, 1, 2, 1, 2, 3);
  return det;
}

/*!
  Return a new matrix which is the inverse matrix of this.

  The user is responsible for checking that this is a valid operation
  to execute, by first making sure that the result of SdMatrix::det4()
  is not equal to zero.
 */
SdMatrix
SdMatrix::inverse(void) const//need to be improved
{
  double det = this->det4();
  SdMatrix result;

  // FIXME: we should be using an optimized way of calculating the
  // inverse matrix. 20010114 mortene.
  result.matrix[0][0] = this->det3(1, 2, 3, 1, 2, 3);
  result.matrix[1][0] = -this->det3(1, 2, 3, 0, 2, 3);
  result.matrix[2][0] = this->det3(1, 2, 3, 0, 1, 3);
  result.matrix[3][0] = -this->det3(1, 2, 3, 0, 1, 2);
  result.matrix[0][1] = -this->det3(0, 2, 3, 1, 2, 3);
  result.matrix[1][1] = this->det3(0, 2, 3, 0, 2, 3);
  result.matrix[2][1] = -this->det3(0, 2, 3, 0, 1, 3);
  result.matrix[3][1] = this->det3(0, 2, 3, 0, 1, 2);
  result.matrix[0][2] = this->det3(0, 1, 3, 1, 2, 3);
  result.matrix[1][2] = -this->det3(0, 1, 3, 0, 2, 3);
  result.matrix[2][2] = this->det3(0, 1, 3, 0, 1, 3);
  result.matrix[3][2] = -this->det3(0, 1, 3, 0, 1, 2);
  result.matrix[0][3] = -this->det3(0, 1, 2, 1, 2, 3);
  result.matrix[1][3] = this->det3(0, 1, 2, 0, 2, 3);
  result.matrix[2][3] = -this->det3(0, 1, 2, 0, 1, 3);
  result.matrix[3][3] = this->det3(0, 1, 2, 0, 1, 2);

  result /= det;
  return result;
}

/*!
  Check if the \a m matrix is equal to this one, within the given tolerance
  value. The tolerance value is applied in the comparison on a component by
  component basis.
 */
SbBool
SdMatrix::equals(const SdMatrix & m, double tolerance) const
{
  for (int i=0; i < 4; i++) {
    for (int j=0;  j< 4; j++) {
      if (fabs(this->matrix[i][j] - m.matrix[i][j]) > tolerance) return FALSE;
    }
  }

  return TRUE;
}


/*!
  Return pointer to the matrix' 4x4 double array.
 */
SdMatrix::operator double*(void)
{
  return &(this->matrix[0][0]);
}


/*!
  Return pointer to the matrix' 4x4 double array.
 */
SdMatrix::operator SdMat&(void)
{
  return this->matrix;
}

/*!
  Returns pointer to the 4 element array representing a matrix row.
  \a i should be within [0, 3].

  \sa getValue(), setValue().
 */
double *
SdMatrix::operator [](int i)
{
   return this->matrix[i];
}

/*!
  Returns pointer to the 4 element array representing a matrix row.
  \a i should be within [0, 3].

  \sa getValue(), setValue().
 */
const double *
SdMatrix::operator [](int i) const
{
   return this->matrix[i];
}

/*!
  Right-multiply with the \a m matrix.

  \sa multRight().
 */
SdMatrix&
SdMatrix::operator *=(const SdMatrix & m)
{
  return this->multRight(m);
}

/*!
  \relates SdMatrix

  Multiplies matrix \a m1 with matrix \a m2 and returns the resultant
  matrix.
*/
SdMatrix
operator *(const SdMatrix & m1, const SdMatrix & m2)
{
  SdMatrix result = m1;
  result *= m2;
  return result;
}

/*!
  \relates SdMatrix

  Compare matrices to see if they are equal. For two matrices to be equal,
  all their individual elements must be equal.

  \sa equals().
*/
int
operator ==(const SdMatrix & m1, const SdMatrix & m2)
{
  for (int i=0; i < 4; i++) {
    for (int j=0; j < 4; j++) {
      if (m1.matrix[i][j] != m2.matrix[i][j]) return FALSE;
    }
  }

  return TRUE;
}

/*!
  \relates SdMatrix

  Compare matrices to see if they are not equal. For two matrices to not be
  equal, it is enough that at least one of their elements are unequal.

  \sa equals().
*/
int
operator !=(const SdMatrix & m1, const SdMatrix & m2)
{
  return !(m1 == m2);
}

/*!
  Return matrix components in the SdMat structure.

  \sa setValue().
 */
void
SdMatrix::getValue(SdMat & m) const
{
  (void)memmove(&m[0][0], &(this->matrix[0][0]), sizeof(double)*4*4);
}

/*!
  Return the identity matrix.

  \sa makeIdentity().
 */
SdMatrix
SdMatrix::identity(void)
{
  return SdMatrix(1.0f, 0.0f, 0.0f, 0.0f,
                  0.0f, 1.0f, 0.0f, 0.0f,
                  0.0f, 0.0f, 1.0f, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f);
}

/*!
  Set matrix to be a pure scaling matrix. Scale factors are specified
  by \a s.

  \sa setRotate(), setTranslate().
 */
void
SdMatrix::setScale(const double s)
{
  this->makeIdentity();
  this->matrix[0][0] = s;
  this->matrix[1][1] = s;
  this->matrix[2][2] = s;
}

/*!
  Set matrix to be a pure scaling matrix. Scale factors in x, y and z
  is specified by the \a s vector.

  \sa setRotate(), setTranslate().
 */
void
SdMatrix::setScale(const SbVec3d & s)
{
  this->makeIdentity();
  this->matrix[0][0] = s[0];
  this->matrix[1][1] = s[1];
  this->matrix[2][2] = s[2];
}

/*!
  Make this matrix into a pure translation matrix (no scale or rotation
  components) with the given vector \t as the translation.

  \sa setRotate(), setScale().
 */
void
SdMatrix::setTranslate(const SbVec3d & t)
{
  this->makeIdentity();
  this->matrix[3][0] = t[0];
  this->matrix[3][1] = t[1];
  this->matrix[3][2] = t[2];
}


/*!
  Returns the transpose of this matrix.
*/

SdMatrix
SdMatrix::transpose(void) const
{
  SdMatrix trans = (*this);
  double tmptrans;

  for (int i=0; i < 3; i++) {
    for (int j=i+1; j < 4; j++) {
		tmptrans = trans[i][j];
		trans[i][j] = trans[j][i];
		trans[j][i] = tmptrans;
    }
  }

  return trans;
}

/*!
  Let this matrix be right-multiplied by \a m. Returns reference to
  self.

  \sa multLeft()
*/
SdMatrix &
SdMatrix::multRight(const SdMatrix & m)
{
  // FIXME: should check if one or the other matrix is the
  // identity-matrix first. (Because it's major optimization if one of
  // them _is_, and the check should be very quick in the common case
  // where none of them are.)  20010919 mortene.

  SdMatrix tmp(*this);
  for (int i=0; i < 4; i++) {
    for (int j=0; j < 4; j++) {
      this->matrix[i][j] =
        tmp.matrix[i][0] * m.matrix[0][j] +
        tmp.matrix[i][1] * m.matrix[1][j] +
        tmp.matrix[i][2] * m.matrix[2][j] +
        tmp.matrix[i][3] * m.matrix[3][j];
    }
  }
  return *this;
}

/*!
  Let this matrix be left-multiplied by \a m. Returns reference to
  self.

  \sa multRight()
*/
SdMatrix&
SdMatrix::multLeft(const SdMatrix & m)
{
  // FIXME: should check if one or the other matrix is the
  // identity-matrix first. (Because it's major optimization if one of
  // them _is_, and the check should be very quick in the common case
  // where none of them are.)  20010919 mortene.

  SdMatrix tmp(*this);
  for (int i=0; i < 4; i++) {
    for (int j=0; j < 4; j++) {
      this->matrix[i][j] =
        tmp.matrix[0][j] * m.matrix[i][0] +
        tmp.matrix[1][j] * m.matrix[i][1] +
        tmp.matrix[2][j] * m.matrix[i][2] +
        tmp.matrix[3][j] * m.matrix[i][3];
    }
  }
  return *this;
}

/*!
  Multiply \a src vector with this matrix and return the result in \a dst.
  Multiplication is done with the vector on the right side of the
  expression, i.e. dst = M * src.

  \sa multVecMatrix(), multDirMatrix() and multLineMatrix().
*/
void
SdMatrix::multMatrixVec(const SbVec3d & src, SbVec3d & dst) const
{
  // FIXME: should check if we're the identity matrix? 20010919 mortene.

  const double * t0 = (*this)[0];
  const double * t1 = (*this)[1];
  const double * t2 = (*this)[2];
  const double * t3 = (*this)[3];
  // Copy the src vector, just in case src and dst is the same vector.
  SbVec3d s = src;

  double W = s[0]*t3[0] + s[1]*t3[1] + s[2]*t3[2] + t3[3];

  dst[0] = (s[0]*t0[0] + s[1]*t0[1] + s[2]*t0[2] + t0[3])/W;
  dst[1] = (s[0]*t1[0] + s[1]*t1[1] + s[2]*t1[2] + t1[3])/W;
  dst[2] = (s[0]*t2[0] + s[1]*t2[1] + s[2]*t2[2] + t2[3])/W;
}

/*!
  Multiply \a src vector with this matrix and return the result in \a dst.
  Multiplication is done with the vector on the left side of the
  expression, i.e. dst = src * M.

  It is safe to let \a src and \a dst be the same SbVec3d instance.

  \sa multMatrixVec(), multDirMatrix() and multLineMatrix().
*/
void
SdMatrix::multVecMatrix(const SbVec3d & src, SbVec3d & dst) const
{
  // FIXME: should check if we're the identity matrix? 20010919 mortene.

  const double * t0 = this->matrix[0];
  const double * t1 = this->matrix[1];
  const double * t2 = this->matrix[2];
  const double * t3 = this->matrix[3];
  // Copy the src vector, just in case src and dst is the same vector.
  SbVec3d s = src;

  double W = s[0]*t0[3] + s[1]*t1[3] + s[2]*t2[3] + t3[3];

  dst[0] = (s[0]*t0[0] + s[1]*t1[0] + s[2]*t2[0] + t3[0])/W;
  dst[1] = (s[0]*t0[1] + s[1]*t1[1] + s[2]*t2[1] + t3[1])/W;
  dst[2] = (s[0]*t0[2] + s[1]*t1[2] + s[2]*t2[2] + t3[2])/W;
}

/*!
  \overload
*/
/*void
SdMatrix::multVecMatrix(const SbVec4d & src, SbVec4d & dst) const
{
  // FIXME: should check if we're the identity matrix? 20010919 mortene.

  const double * t0 = (*this)[0];
  const double * t1 = (*this)[1];
  const double * t2 = (*this)[2];
  const double * t3 = (*this)[3];

  SbVec4f s = src;

  dst[0] = (s[0]*t0[0] + s[1]*t0[1] + s[2]*t0[2] + t0[3]);
  dst[1] = (s[0]*t1[0] + s[1]*t1[1] + s[2]*t1[2] + t1[3]);
  dst[2] = (s[0]*t2[0] + s[1]*t2[1] + s[2]*t2[2] + t2[3]);
  dst[3] = (s[0]*t3[0] + s[1]*t3[1] + s[2]*t3[2] + t3[3]);
}
*/
/*!
  Multiplies \a src by the matrix. \a src is assumed to be a direction
  vector, and the translation components of the matrix are therefore
  ignored.

  Multiplication is done with the vector on the left side of the
  expression, i.e. dst = src * M.

  \sa multVecMatrix(), multMatrixVec() and multLineMatrix().
 */
void
SdMatrix::multDirMatrix(const SbVec3d & src, SbVec3d & dst) const
{
  // FIXME: should check if we're the identity matrix? 20010919 mortene.

  const double * t0 = (*this)[0];
  const double * t1 = (*this)[1];
  const double * t2 = (*this)[2];
  // Copy the src vector, just in case src and dst is the same vector.
  SbVec3d s = src;

  dst[0] = s[0]*t0[0] + s[1]*t1[0] + s[2]*t2[0];
  dst[1] = s[0]*t0[1] + s[1]*t1[1] + s[2]*t2[1];
  dst[2] = s[0]*t0[2] + s[1]*t1[2] + s[2]*t2[2];
}


/*!
  Write out the matrix contents to the given file.
 */
void
SdMatrix::print(FILE * fp) const
{
  for (int i=0; i < 4; i++) {
    fprintf(fp, "%10.5g\t%10.5g\t%10.5g\t%10.5g\n",
            this->matrix[i][0], this->matrix[i][1],
            this->matrix[i][2], this->matrix[i][3]);
  }
}


/*
  Convert double Matrix SdMatrix to float Matrix SbMatrix
*/
void
SdMatrix::setSbMatrix(SbMatrix & dst)
{
  dst.makeIdentity();
  for (int i=0; i < 4; i++) {
    for (int j=0; j < 4; j++) {
      dst[i][j] = (float)this->matrix[i][j];
    }
  }
}


