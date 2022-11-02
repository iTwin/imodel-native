/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/bvector.h>
#include "ExportMacros.h"
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {

//=======================================================================================
//! Package definition
//=======================================================================================
struct Package
    {
    private:
        Utf8String m_version;
        Utf8String m_description;
        Utf8String m_date;
        Utf8String m_userId;
    public:
        //! Get version
        Utf8StringCR GetVersion() const { return m_version; }

        //! Get description
        Utf8StringCR GetDescription() const { return m_description; }

        BASEGEOCOORD_EXPORTED void ToJson(JsonValueR) const;
        BASEGEOCOORD_EXPORTED BentleyStatus FromJson(JsonValueCR);
    };

//=======================================================================================
//! Assets definition
//=======================================================================================
typedef std::shared_ptr<struct Asset> AssetPtr;
struct Asset
    {
    private:
        Utf8String m_id;
        Utf8String m_description;
        Utf8String m_userId;
        Utf8String m_ultimateRefId;
        bvector<Package> m_packages;

    public:
        //! Get version
        BASEGEOCOORD_EXPORTED static AssetPtr Create(Utf8String jsonData);

        //! Get version
        Utf8StringCR GetId() const { return m_id; }

        //! Get description
        Utf8StringCR GetDescription() const { return m_description; }

        //! Get packages
        bvector<Package> const& GetPackages() const { return m_packages; }

        BASEGEOCOORD_EXPORTED void ToJson(JsonValueR) const;
        BASEGEOCOORD_EXPORTED BentleyStatus FromJson(JsonValueCR);
    };

} // ends GeoCoordinates namespace
END_BENTLEY_NAMESPACE