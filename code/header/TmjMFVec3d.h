#ifndef  TMJ_VEC3D
#define  TMJ_VEC3D

#include <Inventor/fields/SoSubField.h>
#include "TmjLinear.h"

//////////////////////////////////////////////////////////////////////////////
//
//  SoMFVec3d subclass of SoMField.
//
//////////////////////////////////////////////////////////////////////////////

// C-api: prefix=SoMV3f
class SoMFVec3d : 
	public SoMField {
    // Use standard field stuff
    SO_MFIELD_HEADER(SoMFVec3d, SbVec3d, const SbVec3d &);

  public:

    //
    // Some additional convenience functions:
    //

    // Set values from array of arrays of 3 doubles
    // C-api: name=SetXYZs
    void	setValues(int start, int num, const double xyz[][3]);//n rows and 3 columns

    // Set one value from 3 doubles
	// C-api: name=Set1X_Y_Z
    void	set1Value(int index, double x, double y, double z);

    // Set one value from 3 doubles in array
	// C-api: name=Set1XYZ
    void	set1Value(int index, const double xyz[3]);

    // Set to one value from 3 doubles
	// C-api: name=SetX_Y_Z
    void	setValue(double x, double y, double z);

    // Set to one value from 3 doubles in array
	// C-api: name=SetXYZ
    void	setValue(const double xyz[3]);

  SoINTERNAL public:
    static void		initClass();

  private:

    // Write the values out as a block of data
    virtual void	writeBinaryValues(SoOutput *out) const;
    virtual SbBool	readBinaryValues(SoInput *in, int numToRead);
};

#endif /* TMJ_VEC3D */
