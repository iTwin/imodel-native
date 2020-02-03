/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ICriteria.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ICriteria.h,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : ICriteria
//-----------------------------------------------------------------------------
// Every criterias must inherit this class to work with the Hunter class.
// Each criteria has a Criteria class and a Builder class. The Builder class
// knows how to build the Criteria when requested by the Hunter.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Determines the kind of controls required by the criteria in the UI.
//-----------------------------------------------------------------------------
enum class FilterType : int
{
    Text=1, 
    Number, 
    Enum, 
    XY, 
    KeyValue, 
    Size,
    Boolean,
    EnumWithFields
};

//----------------------------------------------------------------------------------
// List of all operators that can be used in a search if supported by the criteria
//----------------------------------------------------------------------------------
enum class CriteriaOperator : int
{
    Equal=1, 
    NotEqual, 
    LessThan, 
    LessThanOrEqual, 
    GreaterThan, 
    GreaterThanOrEqual
};

//-----------------------------------------------------------------------------
// List of all capabilities supported by the Hunter.
//-----------------------------------------------------------------------------
enum class SupportedCapability : int
{
    FileFormat=1,
    PixelType,
    BlockType,
    Codec,
    Histogram,
    ImageSize,
    ScanlineOrientation,
    TransfoModel,
    MultiPages,
    MultiResolutions,
    Tag,
    Geocoding
};

//-----------------------------------------------------------------------------
// ICriteria class
//-----------------------------------------------------------------------------
interface class ICriteria
{
public:
    bool                isRespected(HFCPtr<HRFRasterFile> pRasterFile);
    bool                isValidWithAtLeastOne();
    CriteriaOperator    GetCriteriaOperator();
};

//-----------------------------------------------------------------------------
// ICriteriaBuilder class
//-----------------------------------------------------------------------------
interface class ICriteriaBuilder
{
public:
	FilterType                          GetFilterType();
    SupportedCapability                 GetCapabilityType();
	System::String^                     GetName();
	System::Collections::ArrayList^     GetOptions();
	ICriteria^                          BuildCriteria(CriteriaOperator criteriaOperator, array<Object^>^ args);
};