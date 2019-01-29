//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DMeshBasedTransfoModel.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DMeshBasedTransfoModel
//-----------------------------------------------------------------------------

#include "hstdcpp.h"    // must be first for PreCompiledHeader Option

#include "HGF2DMeshBasedTransfoModel.h"

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGF2DMeshBasedTransfoModel::HGF2DMeshBasedTransfoModel()
    : HGF2DTransfoModel()
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DMeshBasedTransfoModel::HGF2DMeshBasedTransfoModel(const HGF2DMeshBasedTransfoModel& pi_rObj)
    : HGF2DTransfoModel(pi_rObj)
    {
    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DMeshBasedTransfoModel::~HGF2DMeshBasedTransfoModel()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DMeshBasedTransfoModel& HGF2DMeshBasedTransfoModel::operator=(const HGF2DMeshBasedTransfoModel& pi_rObj)
    {
    HINVARIANTS;

    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Call ancestor operator=
        HGF2DTransfoModel::operator=(pi_rObj);
        }

    // Return reference to self
    return (*this);
    }



