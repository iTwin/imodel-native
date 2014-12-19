//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSGraphicToolAttributes.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSGraphicToolAttributes
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSGraphicToolAttributes.h>

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSGraphicToolAttributes::HGSGraphicToolAttributes()
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HGSGraphicToolAttributes::HGSGraphicToolAttributes(const HGSGraphicToolAttributes& pi_rObj)
    {
    // copy the list of attributes
    Attributes::const_iterator Itr;
    for (Itr = pi_rObj.m_Attributes.begin(); Itr != pi_rObj.m_Attributes.end(); Itr++)
        Add(**Itr);
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGSGraphicToolAttributes::HGSGraphicToolAttributes(const HGSGraphicToolAttribute& pi_rAttribute)
    {
    // add the attribute in the Attributes
    Add(pi_rAttribute);
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSGraphicToolAttributes::~HGSGraphicToolAttributes()
    {
    }
