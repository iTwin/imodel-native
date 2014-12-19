/******************************************************************************
 * $Id: ogrfielddefn.cpp 21018 2010-10-30 11:30:51Z rouault $
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  The OGRFieldDefn class implementation.
 * Author:   Frank Warmerdam, warmerda@home.com
 *
 ******************************************************************************
 * Copyright (c) 1999,  Les Technologies SoftMap Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "ogr_feature.h"
#include "ogr_api.h"
#include "ogr_p.h"

CPL_CVSID("$Id: ogrfielddefn.cpp 21018 2010-10-30 11:30:51Z rouault $");

/************************************************************************/
/*                            OGRFieldDefn()                            */
/************************************************************************/

/**
 * \brief Constructor.
 *
 * @param pszNameIn the name of the new field.
 * @param eTypeIn the type of the new field.
 */

OGRFieldDefn::OGRFieldDefn( const char * pszNameIn, OGRFieldType eTypeIn )

{
    Initialize( pszNameIn, eTypeIn );
}

/************************************************************************/
/*                            OGRFieldDefn()                            */
/************************************************************************/

/**
 * \brief Constructor.
 *
 * Create by cloning an existing field definition.
 *
 * @param poPrototype the field definition to clone.
 */

OGRFieldDefn::OGRFieldDefn( OGRFieldDefn *poPrototype )

{
    Initialize( poPrototype->GetNameRef(), poPrototype->GetType() );

    SetJustify( poPrototype->GetJustify() );
    SetWidth( poPrototype->GetWidth() );
    SetPrecision( poPrototype->GetPrecision() );
//    SetDefault( poPrototype->GetDefaultRef() );
    bIgnore = FALSE;
}

/************************************************************************/
/*                           OGR_Fld_Create()                           */
/************************************************************************/
/**
 * \brief Create a new field definition.
 *
 * This function is the same as the CPP method OGRFieldDefn::OGRFieldDefn().
 *
 * @param pszName the name of the new field definition.
 * @param eType the type of the new field definition.
 * @return handle to the new field definition.
 */

OGRFieldDefnH OGR_Fld_Create( const char *pszName, OGRFieldType eType )

{
    return (OGRFieldDefnH) (new OGRFieldDefn(pszName,eType));
}

/************************************************************************/
/*                             Initialize()                             */
/************************************************************************/

void OGRFieldDefn::Initialize( const char * pszNameIn, OGRFieldType eTypeIn )

{
    pszName = CPLStrdup( pszNameIn );
    eType = eTypeIn;
    eJustify = OJUndefined;

    nWidth = 0;         // should these be defined in some particular way
    nPrecision = 0;     // for numbers?

    memset( &uDefault, 0, sizeof(OGRField) );
}

/************************************************************************/
/*                           ~OGRFieldDefn()                            */
/************************************************************************/

OGRFieldDefn::~OGRFieldDefn()

{
    CPLFree( pszName );
}

/************************************************************************/
/*                          OGR_Fld_Destroy()                           */
/************************************************************************/
/**
 * \brief Destroy a field definition.
 *
 * @param hDefn handle to the field definition to destroy.
 */

void OGR_Fld_Destroy( OGRFieldDefnH hDefn )

{
    delete (OGRFieldDefn *) hDefn;
}

/************************************************************************/
/*                              SetName()                               */
/************************************************************************/

/**
 * \brief Reset the name of this field.
 *
 * This method is the same as the C function OGR_Fld_SetName().
 *
 * @param pszNameIn the new name to apply.
 */

void OGRFieldDefn::SetName( const char * pszNameIn )

{
    CPLFree( pszName );
    pszName = CPLStrdup( pszNameIn );
}

/************************************************************************/
/*                          OGR_Fld_SetName()                           */
/************************************************************************/
/**
 * \brief Reset the name of this field.
 *
 * This function is the same as the CPP method OGRFieldDefn::SetName().
 *
 * @param hDefn handle to the field definition to apply the new name to.
 * @param pszName the new name to apply.
 */

void OGR_Fld_SetName( OGRFieldDefnH hDefn, const char *pszName )

{
    ((OGRFieldDefn *) hDefn)->SetName( pszName );
}

/************************************************************************/
/*                             GetNameRef()                             */
/************************************************************************/

/**
 * \fn const char *OGRFieldDefn::GetNameRef();
 *
 * \brief Fetch name of this field.
 *
 * This method is the same as the C function OGR_Fld_GetNameRef().
 *
 * @return pointer to an internal name string that should not be freed or
 * modified.
 */

/************************************************************************/
/*                         OGR_Fld_GetNameRef()                         */
/************************************************************************/
/**
 * \brief Fetch name of this field.
 *
 * This function is the same as the CPP method OGRFieldDefn::GetNameRef().
 *
 * @param hDefn handle to the field definition.
 * @return the name of the field definition.
 * 
 */

const char *OGR_Fld_GetNameRef( OGRFieldDefnH hDefn )

{
    return ((OGRFieldDefn *) hDefn)->GetNameRef();
}

/************************************************************************/
/*                              GetType()                               */
/************************************************************************/

/**
 * \fn OGRFieldType OGRFieldDefn::GetType();
 *
 * \brief Fetch type of this field.
 *
 * This method is the same as the C function OGR_Fld_GetType().
 *
 * @return field type.
 */

/************************************************************************/
/*                          OGR_Fld_GetType()                           */
/************************************************************************/
/**
 * \brief Fetch type of this field.
 *
 * This function is the same as the CPP method OGRFieldDefn::GetType().
 *
 * @param hDefn handle to the field definition to get type from.
 * @return field type.
 */

OGRFieldType OGR_Fld_GetType( OGRFieldDefnH hDefn )

{
    return ((OGRFieldDefn *) hDefn)->GetType();
}

/************************************************************************/
/*                              SetType()                               */
/************************************************************************/

/**
 * \fn void OGRFieldDefn::SetType( OGRFieldType eType );
 *
 * \brief Set the type of this field.
 * This should never be done to an OGRFieldDefn
 * that is already part of an OGRFeatureDefn.
 *
 * This method is the same as the C function OGR_Fld_SetType().
 *
 * @param eType the new field type.
 */

/************************************************************************/
/*                          OGR_Fld_SetType()                           */
/************************************************************************/
/**
 * \brief Set the type of this field.
 * This should never be done to an OGRFieldDefn
 * that is already part of an OGRFeatureDefn.
 *
 * This function is the same as the CPP method OGRFieldDefn::SetType().
 *
 * @param hDefn handle to the field definition to set type to.
 * @param eType the new field type.
 */

void OGR_Fld_SetType( OGRFieldDefnH hDefn, OGRFieldType eType )

{
    ((OGRFieldDefn *) hDefn)->SetType( eType );
}

/************************************************************************/
/*                             SetDefault()                             */
/************************************************************************/

/**
 * \brief Set default field value.
 *
 * Currently use of OGRFieldDefn "defaults" is discouraged.  This feature
 * may be fleshed out in the future.
 *
 */

void OGRFieldDefn::SetDefault( const OGRField * puDefaultIn )

{
    switch( eType )
    {
      case OFTInteger:
      case OFTReal:
        uDefault = *puDefaultIn;
        break;

      case OFTString:
//        CPLFree( uDefault.String );
//        uDefault.String = CPLStrdup( puDefaultIn->String );
        break;

      default:
        // add handling for other complex types.
        CPLAssert( FALSE );
        break;
    }
}

/************************************************************************/
/*                          GetFieldTypeName()                          */
/************************************************************************/

/**
 * \brief Fetch human readable name for a field type.
 *
 * This static method is the same as the C function OGR_GetFieldTypeName().
 *
 * @param eType the field type to get name for.
 *
 * @return pointer to an internal static name string. It should not be
 * modified or freed.
 */

const char * OGRFieldDefn::GetFieldTypeName( OGRFieldType eType )

{
    switch( eType )
    {
      case OFTInteger:
        return "Integer";

      case OFTReal:
        return "Real";

      case OFTString:
        return "String";

      case OFTIntegerList:
        return "IntegerList";

      case OFTRealList:
        return "RealList";

      case OFTStringList:
        return "StringList";

      case OFTBinary:
        return "Binary";

      case OFTDate:
        return "Date";

      case OFTTime:
        return "Time";

      case OFTDateTime:
        return "DateTime";

      default:
        return "(unknown)";
    }
}

/************************************************************************/
/*                        OGR_GetFieldTypeName()                        */
/************************************************************************/
/**
 * \brief Fetch human readable name for a field type.
 *
 * This function is the same as the CPP method 
 * OGRFieldDefn::GetFieldTypeName().
 *
 * @param eType the field type to get name for.
 * @return the name.
 */

const char *OGR_GetFieldTypeName( OGRFieldType eType )

{
    return OGRFieldDefn::GetFieldTypeName( eType );
}

/************************************************************************/
/*                             GetJustify()                             */
/************************************************************************/

/**
 * \fn OGRJustification OGRFieldDefn::GetJustify();
 *
 * \brief Get the justification for this field.
 *
 * This method is the same as the C function OGR_Fld_GetJustify().
 *
 * @return the justification.
 */

/************************************************************************/
/*                         OGR_Fld_GetJustify()                         */
/************************************************************************/
/**
 * \brief Get the justification for this field.
 *
 * This function is the same as the CPP method OGRFieldDefn::GetJustify().
 *
 * @param hDefn handle to the field definition to get justification from.
 * @return the justification.
 */

OGRJustification OGR_Fld_GetJustify( OGRFieldDefnH hDefn )

{
    return ((OGRFieldDefn *) hDefn)->GetJustify();
}

/************************************************************************/
/*                             SetJustify()                             */
/************************************************************************/

/**
 * \fn void OGRFieldDefn::SetJustify( OGRJustification eJustify );
 *
 * \brief Set the justification for this field.
 *
 * This method is the same as the C function OGR_Fld_SetJustify().
 *
 * @param eJustify the new justification.
 */

/************************************************************************/
/*                         OGR_Fld_SetJustify()                         */
/************************************************************************/
/**
 * \brief Set the justification for this field.
 *
 * This function is the same as the CPP method OGRFieldDefn::SetJustify().
 *
 * @param hDefn handle to the field definition to set justification to.
 * @param eJustify the new justification.
 */

void OGR_Fld_SetJustify( OGRFieldDefnH hDefn, OGRJustification eJustify )

{
    ((OGRFieldDefn *) hDefn)->SetJustify( eJustify );
}

/************************************************************************/
/*                              GetWidth()                              */
/************************************************************************/

/**
 * \fn int OGRFieldDefn::GetWidth();
 *
 * \brief Get the formatting width for this field.
 *
 * This method is the same as the C function OGR_Fld_GetWidth().
 *
 * @return the width, zero means no specified width. 
 */

/************************************************************************/
/*                          OGR_Fld_GetWidth()                          */
/************************************************************************/
/**
 * \brief Get the formatting width for this field.
 *
 * This function is the same as the CPP method OGRFieldDefn::GetWidth().
 *
 * @param hDefn handle to the field definition to get width from.
 * @return the width, zero means no specified width. 
 */

int OGR_Fld_GetWidth( OGRFieldDefnH hDefn )

{
    return ((OGRFieldDefn *) hDefn)->GetWidth();
}

/************************************************************************/
/*                              SetWidth()                              */
/************************************************************************/

/**
 * \fn void OGRFieldDefn::SetWidth( int nWidth );
 *
 * \brief Set the formatting width for this field in characters.
 *
 * This method is the same as the C function OGR_Fld_SetWidth().
 *
 * @param nWidth the new width.
 */

/************************************************************************/
/*                          OGR_Fld_SetWidth()                          */
/************************************************************************/
/**
 * \brief Set the formatting width for this field in characters.
 *
 * This function is the same as the CPP method OGRFieldDefn::SetWidth().
 *
 * @param hDefn handle to the field definition to set width to.
 * @param nNewWidth the new width.
 */

void OGR_Fld_SetWidth( OGRFieldDefnH hDefn, int nNewWidth )

{
    ((OGRFieldDefn *) hDefn)->SetWidth( nNewWidth );
}

/************************************************************************/
/*                            GetPrecision()                            */
/************************************************************************/

/**
 * \fn int OGRFieldDefn::GetPrecision();
 *
 * \brief Get the formatting precision for this field.
 * This should normally be
 * zero for fields of types other than OFTReal.
 *
 * This method is the same as the C function OGR_Fld_GetPrecision().
 *
 * @return the precision.
 */

/************************************************************************/
/*                        OGR_Fld_GetPrecision()                        */
/************************************************************************/
/**
 * \brief Get the formatting precision for this field.
 * This should normally be
 * zero for fields of types other than OFTReal.
 *
 * This function is the same as the CPP method OGRFieldDefn::GetPrecision().
 *
 * @param hDefn handle to the field definition to get precision from.
 * @return the precision.
 */

int OGR_Fld_GetPrecision( OGRFieldDefnH hDefn )

{
    return ((OGRFieldDefn *) hDefn)->GetPrecision();
}

/************************************************************************/
/*                            SetPrecision()                            */
/************************************************************************/

/**
 * \fn void OGRFieldDefn::SetPrecision( int nPrecision );
 *
 * \brief Set the formatting precision for this field in characters.
 * 
 * This should normally be zero for fields of types other than OFTReal. 
 *
 * This method is the same as the C function OGR_Fld_SetPrecision().
 *
 * @param nPrecision the new precision. 
 */

/************************************************************************/
/*                        OGR_Fld_SetPrecision()                        */
/************************************************************************/
/**
 * \brief Set the formatting precision for this field in characters.
 * 
 * This should normally be zero for fields of types other than OFTReal. 
 *
 * This function is the same as the CPP method OGRFieldDefn::SetPrecision().
 *
 * @param hDefn handle to the field definition to set precision to.
 * @param nPrecision the new precision. 
 */

void OGR_Fld_SetPrecision( OGRFieldDefnH hDefn, int nPrecision )

{
    ((OGRFieldDefn *) hDefn)->SetPrecision( nPrecision );
}

/************************************************************************/
/*                                Set()                                 */
/************************************************************************/

/**
 * \brief Set defining parameters for a field in one call.
 *
 * This method is the same as the C function OGR_Fld_Set().
 *
 * @param pszNameIn the new name to assign.
 * @param eTypeIn the new type (one of the OFT values like OFTInteger). 
 * @param nWidthIn the preferred formatting width.  Defaults to zero indicating
 * undefined.
 * @param nPrecisionIn number of decimals places for formatting, defaults to
 * zero indicating undefined.
 * @param eJustifyIn the formatting justification (OJLeft or OJRight), defaults
 * to OJUndefined.
 */

void OGRFieldDefn::Set( const char *pszNameIn,
                        OGRFieldType eTypeIn,
                        int nWidthIn, int nPrecisionIn,
                        OGRJustification eJustifyIn )
{
    SetName( pszNameIn );
    SetType( eTypeIn );
    SetWidth( nWidthIn );
    SetPrecision( nPrecisionIn );
    SetJustify( eJustifyIn );
}

/************************************************************************/
/*                            OGR_Fld_Set()                             */
/************************************************************************/
/**
 * \brief Set defining parameters for a field in one call.
 *
 * This function is the same as the CPP method OGRFieldDefn::Set().
 *
 * @param hDefn handle to the field definition to set to.
 * @param pszNameIn the new name to assign.
 * @param eTypeIn the new type (one of the OFT values like OFTInteger). 
 * @param nWidthIn the preferred formatting width.  Defaults to zero indicating
 * undefined.
 * @param nPrecisionIn number of decimals places for formatting, defaults to
 * zero indicating undefined.
 * @param eJustifyIn the formatting justification (OJLeft or OJRight), defaults
 * to OJUndefined.
 */

void OGR_Fld_Set( OGRFieldDefnH hDefn, const char *pszNameIn, 
                        OGRFieldType eTypeIn,
                        int nWidthIn, int nPrecisionIn,
                        OGRJustification eJustifyIn )

{
    ((OGRFieldDefn *) hDefn)->Set( pszNameIn, eTypeIn, nWidthIn, 
                                   nPrecisionIn, eJustifyIn );
}

/************************************************************************/
/*                             IsIgnored()                              */
/************************************************************************/

/**
 * \fn int OGRFieldDefn::IsIgnored();
 *
 * \brief Return whether this field should be omitted when fetching features
 *
 * This method is the same as the C function OGR_Fld_IsIgnored().
 *
 * @return ignore state
 */

/************************************************************************/
/*                         OGR_Fld_IsIgnored()                          */
/************************************************************************/

/**
 * \brief Return whether this field should be omitted when fetching features
 *
 * This method is the same as the C++ method OGRFieldDefn::IsIgnored().
 *
 * @param hDefn handle to the field definition
 * @return ignore state
 */

int OGR_Fld_IsIgnored( OGRFieldDefnH hDefn )
{
    return ((OGRFieldDefn *) hDefn)->IsIgnored();
}

/************************************************************************/
/*                            SetIgnored()                              */
/************************************************************************/

/**
 * \fn void OGRFieldDefn::SetIgnored( int ignore );
 *
 * \brief Set whether this field should be omitted when fetching features
 *
 * This method is the same as the C function OGR_Fld_SetIgnored().
 *
 * @param ignore ignore state
 */

/************************************************************************/
/*                        OGR_Fld_SetIgnored()                          */
/************************************************************************/

/**
 * \brief Set whether this field should be omitted when fetching features
 *
 * This method is the same as the C function OGRFieldDefn::SetIgnored().
 *
 * @param hDefn handle to the field definition
 * @param ignore ignore state
 */

void OGR_Fld_SetIgnored( OGRFieldDefnH hDefn, int ignore )
{
    ((OGRFieldDefn *) hDefn)->SetIgnored( ignore );
}
