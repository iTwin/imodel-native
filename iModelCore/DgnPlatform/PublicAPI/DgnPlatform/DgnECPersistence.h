/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnECPersistence.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECDbApi.h>

DGNPLATFORM_REF_COUNTED_PTR (DgnECPropertyFormatter);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* Utility to work with "standard" ec categories. 
* @see JsonECSqlSelectAdapter
* @remarks
  A lot of this logic has been carried forward from ECPropertyPane.cs for the Graphite property 
  panes to be backward compatible with 8.11.9. However, Graphite does NOT support multiple 
  categories that 8.11.9 seems to allow. The category system will be revamped in the future - 
  (TFS 63407)
* @bsiclass                                                 Ramanujam.Raman      08/2012
+===============+===============+===============+===============+===============+======*/
struct ECStandardCategoryHelper : NonCopyableClass
{
private:
    typedef bmap<int, ECN::IECInstancePtr> CategoriesByStandard;
    CategoriesByStandard m_categoriesByStandard;
    ECN::ECClassCP m_categoryClass;

public:

    enum StandardCategory
        {
        Miscellaneous       =   0,
        General             =   1,
        Extended            =   1 << 1,
        RawData             =   1 << 2,
        Geometry            =   1 << 3,
        Groups              =   1 << 4,
        Material            =   1 << 5,
        Relationships       =   1 << 6
        };

// TODO: Move this to DgnECPersistence.cpp as soon as DgnECJsonAdapter is deprecated
// TODO: Eventually deprecate this by moving the functionality to a lower level. 
private:
    static const int CategorySortPriorityVeryHigh  = 400000;
    static const int CategorySortPriorityHigh      = 300000;
    static const int CategorySortPriorityMedium    = 200000;
    static const int CategorySortPriorityLow       = 100000;
    static const int CategorySortPriorityVeryLow   = 0;

    static Utf8String GetDisplayLabel (StandardCategory standard);
    static int GetPriority (StandardCategory standard);
    static bool GetDefaultExpand (StandardCategory standard);
    static Utf8CP GetName (StandardCategory standard);
    static void SetValue (ECN::IECInstanceR instance, Utf8CP name, ECN::ECValueCR ecValue);
    static void SetStringValue (ECN::IECInstanceR instance, Utf8CP name, Utf8CP val);
    static void SetBooleanValue (ECN::IECInstanceR instance, Utf8CP name, bool val);
    static void SetIntegerValue (ECN::IECInstanceR instance, Utf8CP name, int val);

public:
    ECStandardCategoryHelper () : m_categoryClass (NULL) {}

    void Initialize (BeSQLite::EC::ECDbCR ecDb);
    ECN::IECInstancePtr GetCategory (StandardCategory standard);
    
};

/*=================================================================================**//**
* Used to format the property values returned by the ECDb API. 
* @see JsonECSqlSelectAdapter
* @bsiclass                                                 Ramanujam.Raman      10/2013
+===============+===============+===============+===============+===============+======*/
struct DgnECPropertyFormatter : BeSQLite::EC::ECPropertyFormatter
{
private:
    DgnModelP m_dgnModel;
    ECStandardCategoryHelper m_standardCategoryHelper;
    DgnECPropertyFormatter (DgnModelP dgnModel);
    virtual ~DgnECPropertyFormatter() {}

private:
    virtual bool _FormattedStringFromECValue 
        (
        Utf8StringR strVal, 
        ECN::ECValueCR ecValue, 
        ECN::ECPropertyCR ecProperty, 
        bool isArrayMember
        ) const override;

     virtual ECN::IECInstancePtr _GetPropertyCategory (ECN::ECPropertyCR ecProperty) override;

public:
    DGNPLATFORM_EXPORT static DgnECPropertyFormatterPtr Create (DgnModelP dgnModel);
};

/*=================================================================================**//**
* Used to read information on elements
* @nosubgrouping
* @bsiclass                                                 Ramanujam.Raman      05/2013
+===============+===============+===============+===============+===============+======*/
struct DgnECPersistence : NonCopyableClass
{
public:
    //! Helper utility to get all the EC information associated with an element
    //! @param [out] jsonInstances  Information on the element in the JSON format.
    //!              The properties are formatted strings that include unit labels 
    //!              if specified. 
    //! @param [out] jsonDisplayInfo Additional presentation meta-data to properly 
    //!              format the properties in the instances. 
    //! @param [in] elementId Id of Element to get information on. 
    //! @param [in] dgndb DgnDb
    //! @see JsonReader::Read, JsonECSqlSelectAdapter::ReadInstance for more
    //! information on the format of the JSON. 
    DGNPLATFORM_EXPORT static BentleyStatus GetElementInfo (JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, DgnElementId elementId, DgnDbCR dgndb);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
