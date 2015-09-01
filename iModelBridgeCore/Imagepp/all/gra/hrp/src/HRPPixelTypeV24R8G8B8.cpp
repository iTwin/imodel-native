//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV24R8G8B8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPPixelTypeV24R8G8B8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

#include "v24rgb.h"

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>

HPM_REGISTER_CLASS(HRPPixelTypeV24R8G8B8, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;


static ConverterV24R8G8B8_V24R8G8B8<RGB_RED,RGB_BLUE,RGB_RED,RGB_BLUE> s_V24R8G8B8_V24R8G8B8;
static ConverterV24R8G8B8_V8Gray8<RGB_RED,RGB_BLUE>                    s_V24R8G8B8_V8Gray8;
static ConverterV24R8G8B8_V1Gray1<RGB_RED,RGB_BLUE>                    s_V24R8G8B8_V1Gray1;
static ConverterV24R8G8B8_V8GrayWhite8<RGB_RED,RGB_BLUE>               s_V24R8G8B8_V8GrayWhite8;
static ConverterV24R8G8B8_V1GrayWhite1<RGB_RED,RGB_BLUE>               s_V24R8G8B8_V1GrayWhite1;
struct ConverterV8Gray8_V24R8G8B8                                      s_V8Gray8_V24R8G8B8;
struct ConverterV1Gray1_V24R8G8B8                                      s_V1Gray1_V24R8G8B8;
struct ConverterV8GrayWhite8_V24R8G8B8                                 s_V8GrayWhite8_V24R8G8B8;
struct ConverterV1GrayWhite1_V24R8G8B8                                 s_V1GrayWhite1_V24R8G8B8;
static ConverterV24R8G8B8_I8R8G8B8<RGB_RED,RGB_BLUE>                   s_V24R8G8B8_I8R8G8B8;
static ConverterI8R8G8B8_V24R8G8B8<RGB_RED,RGB_BLUE>                   s_I8R8G8B8_V24R8G8B8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V24R8G8B8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V24R8G8B8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID,   &s_V8Gray8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1Gray1::CLASS_ID,   &s_V1Gray1_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8GrayWhite8::CLASS_ID,   &s_V8GrayWhite8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1GrayWhite1::CLASS_ID,   &s_V1GrayWhite1_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID,  &s_I8R8G8B8_V24R8G8B8));
        };
    };



//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V24R8G8B8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V24R8G8B8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID,   &s_V24R8G8B8_V8Gray8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1Gray1::CLASS_ID,   &s_V24R8G8B8_V1Gray1));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8GrayWhite8::CLASS_ID,   &s_V24R8G8B8_V8GrayWhite8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1GrayWhite1::CLASS_ID,   &s_V24R8G8B8_V1GrayWhite1));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID,  &s_V24R8G8B8_I8R8G8B8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeV24R8G8B8::HRPPixelTypeV24R8G8B8()
    : HRPPixelTypeRGB(8,8,8,0,0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV24R8G8B8::HRPPixelTypeV24R8G8B8(const HRPPixelTypeV24R8G8B8& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV24R8G8B8::~HRPPixelTypeV24R8G8B8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV24R8G8B8::Clone() const
    {
    return new HRPPixelTypeV24R8G8B8(*this);
    }

/** -----------------------------------------------------------------------------
    This function is used to know the number of "value" bits contain in a pixel
    of this pixel type.

    @b{Example:} @list{HRPPixelTypeV32R8G8B8A8 should return 32.}
                 @list{HRPPixelTypeI8R8G8B8A8 should return 0.}
                 @list{HRPPixelTypeI8VA8R8G8B8 should return 8.}

    @return The number of "value" bits contain in a pixel of this pixel type.
    @end

    @see HRPPixelType::CountIndexBits()
    @see HRPPixelType::CountPixelRawData()
    @end
    -----------------------------------------------------------------------------
 */
unsigned short HRPPixelTypeV24R8G8B8::CountValueBits() const
    {
    return 24;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV24R8G8B8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V24R8G8B8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV24R8G8B8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V24R8G8B8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

