/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef JSDPOINT3DDVECTOR3DDVECTOR3D_H_
#define JSDPOINT3DDVECTOR3DDVECTOR3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
// @bsiclass
//=======================================================================================
struct JsDPoint3dDVector3dDVector3d: JsGeomWrapperBase<DPoint3dDVec3dDVec3d>
{
public:
    JsDPoint3dDVector3dDVector3d (){}

    JsDPoint3dDVector3dDVector3d (DPoint3dDVec3dDVec3dCR data) {m_data = data;}
    JsDPoint3dDVector3dDVector3d (DPoint3dCR origin, DVec3dCR vectorU, DVec3dCR vectorV)
        {
        m_data = DPoint3dDVec3dDVec3d (origin, vectorU, vectorV);
        }
    JsDPoint3dDVector3dDVector3d (JsDPoint3dP origin, JsDVector3dP vectorU, JsDVector3dP vectorV)
        {
        m_data = DPoint3dDVec3dDVec3d (origin->Get (), vectorU->Get (), vectorV->Get ());
        }
        
    JsDPoint3dDVector3dDVector3dP Clone () { return new JsDPoint3dDVector3dDVector3d (m_data);}

    JsDPoint3dP Evaluate (double u, double v){return new JsDPoint3d (m_data.origin + m_data.vectorU * u + m_data.vectorV * v);}


    JsDPoint3dP GetOrigin (){return new JsDPoint3d (m_data.origin);}
    JsDVector3dP GetVectorU (){return new JsDVector3d (m_data.vectorU);}
    JsDVector3dP GetVectorV (){return new JsDVector3d (m_data.vectorV);}

    void SetPoint (JsDPoint3dP data){m_data.origin = data->Get ();}
    void SetVectorU (JsDVector3dP data){m_data.vectorU = data->Get ();}
    void SetVectorV (JsDVector3dP data){m_data.vectorV = data->Get ();}

#ifdef ForwardDeclareTransform
    JsTransformP GetLocalToWorldTransform ();
    JsTransformP GetNormalizedLocalToWorldTransform ();
    JsTransformP GetWorldToLocalTransform ();
    JsTransformP GetWorldToNormalizedLocalTransform ();
#else
    JsTransformP GetLocalToWorldTransform (){return JsTransform::Create (m_data.LocalToWorldTransform ());}
    JsTransformP GetNormalizedLocalToWorldTransform (){return JsTransform::Create (m_data.NormalizedLocalToWorldTransform ());}
    JsTransformP GetWorldToLocalTransform (){return JsTransform::Create (m_data.WorldToLocalTransform ());}
    JsTransformP GetWorldToNormalizedLocalTransform (){return JsTransform::Create (m_data.WorldToNormalizedLocalTransform ());}

#endif
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef JSDPOINT3DDVECTOR3DDVECTOR3D_H_

