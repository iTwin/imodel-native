/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/RasterECTypeAdapters.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnHandlers/DgnECTypes.h>


BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PageNumberTypeAdapter : IDgnECTypeAdapter
    {
private:
    PageNumberTypeAdapter() { }
    virtual ~PageNumberTypeAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _HasStandardValues() const override     { return false; }

public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ColorModeTypeAdapter : IDgnECTypeAdapter
    {
private:
    ColorModeTypeAdapter() { }
    virtual ~ColorModeTypeAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool            _CanConvertFromString() const override  { return false; }
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override {return false;}
    virtual bool            _HasStandardValues() const override     { return false; }

public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RasterViewFlagsAdapter : IDgnECTypeAdapter
    {
private:
    RasterViewFlagsAdapter() { }
    virtual ~RasterViewFlagsAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _CanConvertFromString() const override  { return false; }
    virtual bool            _HasStandardValues() const override     { return false; }
    virtual bool            _IsStruct() const                       { return false; }

public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RangeToPercentageAdapter : IDgnECTypeAdapter
    {
private:
    RangeToPercentageAdapter() { }
    virtual ~RangeToPercentageAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _HasStandardValues() const override     { return false; }
public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };

#ifdef DGNV10FORMAT_CHANGES_WIP

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RasterGeopriorityTypeAdapter : IDgnECTypeAdapter
    {
private:
    RasterGeopriorityTypeAdapter() { }
    virtual ~RasterGeopriorityTypeAdapter() { }

    virtual bool     _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool     _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool     _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override;
    virtual bool     _HasStandardValues() const override;
    virtual bool     _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) override;

public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };

#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RasterDisplayPriorityPlaneTypeAdapter : IDgnECTypeAdapter
    {
private:
    RasterDisplayPriorityPlaneTypeAdapter() { }
    virtual ~RasterDisplayPriorityPlaneTypeAdapter() { }

    virtual bool     _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool     _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool     _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override;
    virtual bool     _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) override;

public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RasterDisplayOrderTypeAdapter : IDgnECTypeAdapter
    {
private:
    RasterDisplayOrderTypeAdapter() { }
    virtual ~RasterDisplayOrderTypeAdapter() { }

    virtual bool     _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool     _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool     _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override;
    virtual bool     _HasStandardValues() const override     { return false; }

public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct GeotiffUnitTypeAdapter : IDgnECTypeAdapter
    {
private:
    GeotiffUnitTypeAdapter() { }
    virtual ~GeotiffUnitTypeAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _HasStandardValues() const override     { return true; }
    virtual bool            _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) override;
public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct WorldFileUnitTypeAdapter : IDgnECTypeAdapter
    {
private:
    WorldFileUnitTypeAdapter() { }
    virtual ~WorldFileUnitTypeAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _HasStandardValues() const override     { return true; }
    virtual bool            _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) override;
public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };



END_BENTLEY_DGNPLATFORM_NAMESPACE


