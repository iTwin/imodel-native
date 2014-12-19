//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DTriangleMeshBasedTransfoModel.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DTriangleMeshBasedTransfoModel
//-----------------------------------------------------------------------------

#include "hstdcpp.h"    // must be first for PreCompiledHeader Option

#include "HGF2DTriangleMeshBasedTransfoModel.h"

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGF2DTriangleMeshBasedTransfoModel::HGF2DTriangleMeshBasedTransfoModel()
    : HGF2DMeshBasedTransfoModel()
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DTriangleMeshBasedTransfoModel::HGF2DTriangleMeshBasedTransfoModel(const HGF2DTriangleMeshBasedTransfoModel& i_rObj)
    : HGF2DMeshBasedTransfoModel(i_rObj)
    {
    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DTriangleMeshBasedTransfoModel::~HGF2DTriangleMeshBasedTransfoModel()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DTriangleMeshBasedTransfoModel& HGF2DTriangleMeshBasedTransfoModel::operator=(const HGF2DTriangleMeshBasedTransfoModel& i_rObj)
    {
    HINVARIANTS;

    // Check that object to copy is not self
    if (this != &i_rObj)
        {
        // Call ancestor operator=
        HGF2DMeshBasedTransfoModel::operator=(i_rObj);
        }

    // Return reference to self
    return (*this);
    }



