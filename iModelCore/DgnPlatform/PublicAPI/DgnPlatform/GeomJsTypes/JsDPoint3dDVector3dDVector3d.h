/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDPoint3dDVector3dDVector3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef JSDPOINT3DDVECTOR3DDVECTOR3D_H_
#define JSDPOINT3DDVECTOR3DDVECTOR3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDPoint3dDVector3dDVector3d: JsGeomWrapperBase<DPoint3dDVec3dDVec3d>
{
public:
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

    JsDPoint3dP GetPoint (){return new JsDPoint3d (m_data.origin);}
    JsDVector3dP GetVectorU (){return new JsDVector3d (m_data.vectorU);}
    JsDVector3dP GetVectorV (){return new JsDVector3d (m_data.vectorV);}

    void SetPoint (JsDPoint3dP data){m_data.origin = data->Get ();}
    void SetVectorU (JsDVector3dP data){m_data.vectorU = data->Get ();}
    void SetVectorV (JsDVector3dP data){m_data.vectorV = data->Get ();}

};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef JSDPOINT3DDVECTOR3DDVECTOR3D_H_

