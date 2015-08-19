/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsApi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#ifndef _GEOM_JS_API_H_
#define _GEOM_JS_API_H_

#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformLib.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#define JSSTRUCT(_JsStructName_) \
struct _JsStructName_;  \
typedef struct _JsStructName_ * _JsStructName_##P;

template<typename NativeType>
struct JsGeomWrapperBase : RefCountedBase   
{
protected:
NativeType m_data;
public:
NativeType Get () const {return m_data;}
};

JSSTRUCT(JsDPoint3d);
JSSTRUCT(JsDVector3d);
JSSTRUCT(JsDEllipse3d);
JSSTRUCT(JsDSegment3d);
JSSTRUCT(JsDRay3d);
JSSTRUCT(JsDPoint3dDVector3dDVector3d);
JSSTRUCT(JsCurvePrimitive);
JSSTRUCT(JsCurveVector);
JSSTRUCT(JsSolidPrimitive);
JSSTRUCT(JsPolyface);
JSSTRUCT(JsBsplineCurve);
JSSTRUCT(JsBsplineSurface);
JSSTRUCT(JsAngle);
JSSTRUCT(JsBsplineSurface);
JSSTRUCT(JsYawPitchRollAngles);
JSSTRUCT(JsRotMatrix);
JSSTRUCT(JsTransform);

JSSTRUCT(JsDPoint3dArray)
JSSTRUCT(JsDoubleArray)

END_BENTLEY_DGNPLATFORM_NAMESPACE

#include <DgnPlatform/GeomJsTypes/JSDPoint3d.h>
#include <DgnPlatform/GeomJsTypes/JSDVector3d.h>
#include <DgnPlatform/GeomJsTypes/JSYawPitchRollAngles.h>
#include <DgnPlatform/GeomJsTypes/JSDRay3d.h>
#include <DgnPlatform/GeomJsTypes/JSDPoint3dDVector3dDVector3d.h>
#include <DgnPlatform/GeomJsTypes/JsAngle.h>
#include <DgnPlatform/GeomJsTypes/JsDSegment3d.h>
#include <DgnPlatform/GeomJsTypes/JsDEllipse3d.h>
#include <DgnPlatform/GeomJsTypes/JSDRange3d.h>
#include <DgnPlatform/GeomJsTypes/JSRotMatrix.h>
#include <DgnPlatform/GeomJsTypes/JSTransform.h>
#include <DgnPlatform/GeomJsTypes/JsDPoint3dArray.h>
#include <DgnPlatform/GeomJsTypes/JsCurvePrimitive.h>


BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct GeomJsApi : DgnPlatformLib::Host::ScriptAdmin::ScriptLibraryImporter
{
    BeJsContextR m_context;

    GeomJsApi(BeJsContextR);
    ~GeomJsApi();

    void _ImportScriptLibrary(BeJsContextR, Utf8CP) override;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _GEOM_JS_API_H_

