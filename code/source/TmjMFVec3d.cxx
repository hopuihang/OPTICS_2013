
#include "TmjMFVec3d.h"
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////
//
// SoMFVec3d class
//
//////////////////////////////////////////////////////////////////////////////

// Use standard definitions of all basic methods
SO_MFIELD_SOURCE_MALLOC(SoMFVec3d, SbVec3d, const SbVec3d &);

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Sets values from array of arrays of 3 doubles. This can be useful
//    in some applications that have vectors stored in this manner and
//    want to keep them that way for efficiency.
//
// Use: public

void
SoMFVec3d::setValues(int start,			// Starting index
		     int num,			// Number of values to set
		     const double xyz[][3])	// Array of vector values
//
////////////////////////////////////////////////////////////////////////
{
    int	newNum = start + num;
    int	i;

    if (newNum > getNum())
	makeRoom(newNum);

    for (i = 0; i < num; i++)
	values[start + i].setValue(xyz[i]);

    valueChanged();
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Sets one vector value from 3 separate doubles. (Convenience function)
//
// Use: public

void
SoMFVec3d::set1Value(int index, double x, double y, double z)
//
////////////////////////////////////////////////////////////////////////
{
    set1Value(index, SbVec3d(x, y, z));
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Sets one vector value from an array of 3 doubles. (Convenience function)
//
// Use: public

void
SoMFVec3d::set1Value(int index, const double xyz[3])
//
////////////////////////////////////////////////////////////////////////
{
    set1Value(index, SbVec3d(xyz));
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Sets to one vector value from 3 separate doubles. (Convenience function)
//
// Use: public

void
SoMFVec3d::setValue(double x, double y, double z)
//
////////////////////////////////////////////////////////////////////////
{
    setValue(SbVec3d(x, y, z));
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Sets to one vector value from array of 3 doubles. (Convenience function)
//
// Use: public

void
SoMFVec3d::setValue(const double xyz[3])
//
////////////////////////////////////////////////////////////////////////
{
    setValue(SbVec3d(xyz));
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Reads one (indexed) value from file. Returns FALSE on error.
//
// Use: private

SbBool
SoMFVec3d::read1Value(SoInput *in, int index)
//
////////////////////////////////////////////////////////////////////////
{
    return (in->read(values[index][0]) &&
	    in->read(values[index][1]) &&
	    in->read(values[index][2]));
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Writes one (indexed) value to file.
//
// Use: private

void
SoMFVec3d::write1Value(SoOutput *out, int index) const
//
////////////////////////////////////////////////////////////////////////
{
    out->write(values[index][0]);

    if (! out->isBinary())
	out->write(' ');

    out->write(values[index][1]);

    if (! out->isBinary())
	out->write(' ');

    out->write(values[index][2]);
}

////////////////////////////////////////////////////////////////////////
//
// Description:
//    Writes array of binary values to file as one chunk.
//
// Use: private

void
SoMFVec3d::writeBinaryValues(SoOutput *out) const // Defines writing action

//
////////////////////////////////////////////////////////////////////////
{
    out->writeBinaryArray((double *)values, 3*num);
}


////////////////////////////////////////////////////////////////////////
//
// Description:
//    Reads array of binary values from file as one chunk.
//
// Use: private

SbBool
SoMFVec3d::readBinaryValues(SoInput *in,    // Reading specification
                      	    int numToRead)  // Number of values to read
//
////////////////////////////////////////////////////////////////////////
{
    return (in->readBinaryArray((double *)values, 3*numToRead));
}



