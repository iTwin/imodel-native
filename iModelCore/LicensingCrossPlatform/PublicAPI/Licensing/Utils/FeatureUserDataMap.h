/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Utils/FeatureUserDataMap.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>

#include <Bentley/bstdmap.h>
#include <Bentley/WString.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
typedef bstdmap<Utf8String, Utf8String> Utf8StringMap;
typedef bvector<Utf8String> Utf8StringVector;

struct FeatureUserDataMap
    {
    private:
         Utf8StringMap m_featureUserDataMap;

    public:
        bool operator==(const FeatureUserDataMap &other) const;
        bool operator!=(const FeatureUserDataMap &other) const { return !(*this == other); }
        void Dump() const;

        LICENSING_EXPORT BentleyStatus  AddAttribute(Utf8String key, Utf8String value);
        LICENSING_EXPORT uint64_t       GetKeys(Utf8StringVector &vector);
        LICENSING_EXPORT uint64_t       GetMapEntries(Utf8StringVector &vector);
        LICENSING_EXPORT BentleyStatus  GetValue(Utf8CP key, Utf8StringR value);
        LICENSING_EXPORT uint32_t       GetCount() { return (uint32_t) m_featureUserDataMap.size(); }
        LICENSING_EXPORT void           Clear() { m_featureUserDataMap.clear(); }
        LICENSING_EXPORT bool           Empty() { return m_featureUserDataMap.empty(); }
        LICENSING_EXPORT void           CopyMap(const FeatureUserDataMap* other);
    };

END_BENTLEY_LICENSING_NAMESPACE
