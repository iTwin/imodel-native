/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/PointCloudECTypeAdapters.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnHandlers/DgnECTypes.h>


BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudViewFlagsAdapter : IDgnECTypeAdapter
    {
private:
    PointCloudViewFlagsAdapter() { }
    virtual ~PointCloudViewFlagsAdapter() { }

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
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudDensityAdapter : IDgnECTypeAdapter
    {
private:
    PointCloudDensityAdapter() { }
    virtual ~PointCloudDensityAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _CanConvertFromString() const override  { return true; }
    virtual bool            _HasStandardValues() const override     { return false; }
    virtual bool            _IsStruct() const                       { return false; }

public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudNbPointsAdapter : IDgnECTypeAdapter
    {
private:
    PointCloudNbPointsAdapter() { }
    virtual ~PointCloudNbPointsAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) override  { return false; }
    virtual bool            _CanConvertFromString() const override  { return false; }
    virtual bool            _HasStandardValues() const override     { return false; }
    virtual bool            _IsStruct() const                       { return false; }

    void                    FormatThousandsSeparator(WCharP pOutValue, uint64_t inValue) const;

public:
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterPtr   Create();
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
