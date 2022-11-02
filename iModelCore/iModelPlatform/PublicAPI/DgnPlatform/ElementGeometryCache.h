/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Element geometry cache for edit tools.
//=======================================================================================
namespace ElementGeometryCache
{
    struct Response {
        BeJsValue m_value;
        Response(BeJsValue value) : m_value(value) {}
        BE_JSON_NAME(status)
        BE_JSON_NAME(numGeom)
        BE_JSON_NAME(numPart)
        BE_JSON_NAME(numSolid)
        BE_JSON_NAME(numSurface)
        BE_JSON_NAME(numCurve)
        BE_JSON_NAME(numOther)
        void SetStatus(BentleyStatus val) { m_value[json_status()] = (uint32_t) val;}
        void SetSummary(uint32_t numGeom, uint32_t numPart, uint32_t numSolid, uint32_t numSurface, uint32_t numCurve, uint32_t numOther) {
            m_value[json_numGeom()] = numGeom;
            m_value[json_numPart()] = numPart;
            m_value[json_numSolid()] = numSolid;
            m_value[json_numSurface()] = numSurface;
            m_value[json_numCurve()] = numCurve;
            m_value[json_numOther()] = numOther;
        }
    };

    //! Populate geometry cache.
    DGNPLATFORM_EXPORT void Populate(DgnDbR, BeJsValue out, BeJsConst input, ICancellableR);

    //! Query and modify cached geometry.
    DGNPLATFORM_EXPORT BentleyStatus Operation(DgnDbR, Napi::Object const& input, Napi::Env env);

} // namespace ElementGeometryCache

END_BENTLEY_DGN_NAMESPACE
