/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECSqlStatement.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct QueryJsonAdaptor {
private:
    bool m_abbreviateBlobs;
    bool m_classIdToClassNames;
    bool m_useJsName;
    bool m_jsifyElements;
    ECDbCR m_ecdb;

private:
    BentleyStatus RenderRootProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderPrimitiveProperty(BeJsValue out, IECSqlValue const& in, ECN::PrimitiveType const* prop) const;
    BentleyStatus RenderNavigationProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderPoint2d(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderPoint3d(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderLong(BeJsValue out, IECSqlValue const& in, ECN::PrimitiveECPropertyCP prop) const;
    BentleyStatus RenderGeometryProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderBinaryProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderStructProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderPrimitiveArrayProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderStructArrayProperty(BeJsValue out, IECSqlValue const& in) const;

public:
    QueryJsonAdaptor(ECDbCR ecdb) : m_ecdb(ecdb), m_abbreviateBlobs(true), m_classIdToClassNames(false), m_useJsName(false), m_jsifyElements(false) {}
    QueryJsonAdaptor& SetAbbreviateBlobs(bool v) { m_abbreviateBlobs = v; return *this; }
    QueryJsonAdaptor& SetConvertClassIdsToClassNames(bool v) { m_classIdToClassNames = v; return *this; }
    QueryJsonAdaptor& UseJsNames(bool v) { m_useJsName = v; return *this; }
    QueryJsonAdaptor& JsifyElements(bool v) { m_jsifyElements = v; return *this; }
    BentleyStatus RenderRow(BeJsValue rowJson, IECSqlRow const& stmt, bool asArray = true) const;
    BentleyStatus RenderValue(BeJsValue valJson, IECSqlValue const& val) { return RenderRootProperty(valJson, val); }
    void GetMetaData(QueryProperty::List& list, ECSqlStatement const& stmt) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
