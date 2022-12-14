/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSPolyfaceVisitor_H_
#define _JSPolyfaceVisitor_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct JsPolyfaceVisitor: JsGeomWrapperBase<PolyfaceVisitorPtr>
{
    JsPolyfaceVisitor (PolyfaceVisitorPtr &data) {m_data = data;}
public:
    JsPolyfaceVisitor (){}  // umm.. really should not happen -- visitor without client is disaster
    static JsPolyfaceVisitorP CreateVisitor (JsPolyfaceMeshP mesh, double aNumWrap)
        {
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*mesh->GetPolyfaceHeaderPtr (), true);
        visitor->SetNumWrap ((uint32_t)aNumWrap);
        return new JsPolyfaceVisitor (visitor);
        }

    void Reset (){m_data->Reset ();}

    bool AdvanceToNextFacet (){return m_data->AdvanceToNextFace ();}

    double GetEdgeCount (){return m_data->NumEdgesThisFace ();}
    JsDPoint3dP GetPoint (double aIndex)
        {
        DPoint3d data;
        if (m_data->Point ().TryGetAt ((size_t)aIndex, DPoint3d::FromZero (), data))
            return new JsDPoint3d (data);
        return nullptr;
        }

    JsDPoint2dP GetParam (double aIndex)
        {
        DPoint2d data;
        if (m_data->Param ().TryGetAt ((size_t)aIndex, DPoint2d::FromZero (), data))
            return new JsDPoint2d (data);
        return nullptr;
        }

    JsDVector3dP GetNormal (double aIndex)
        {
        DVec3d data;
        data.Zero ();
        if (m_data->Normal ().TryGetAt ((size_t)aIndex, data, data))
            return new JsDVector3d (data);
        return nullptr;
        }


};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSPolyfaceVisitor_H_

