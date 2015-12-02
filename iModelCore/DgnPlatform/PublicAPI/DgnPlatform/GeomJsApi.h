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

// Templatized wrapper for a native value class.
// A Js wrapper instantiates this as a base class.
// This instance implements:
//   1) A copy of the native structure as private member var m_data
//   2) A "copy out" Get() method.
//      REMARK: The Get() method ensures Js-side classes that have Js members can readily get to (copies of)
//            their native data.
//      REMARK: This is really important for macros to implement consistent Point/Vector ops.
//      REMARK: In "that case" (the point/Vector ops) the cost of "copy out" is insignificant.
//      REMARK: When used for DgnXXXDetail solid that contain CurveVectorPtr, "copy out" has
//                 the (tolerable?) side effect of making ref counts do gymnastics.
//  3) Hence add a GetR() method to get a reference.  As with all "return a reference" methods,
//       caller takes responsibility for (restricting) the life of the reference.
template<typename NativeType>
struct JsGeomWrapperBase : RefCountedBase   
{
protected:
NativeType m_data;
public:
NativeType Get () const {return m_data;}
NativeType &GetR () {return m_data;}
NativeType const &GetCR () const {return m_data;}
};


JSSTRUCT(JsDPoint3d);
JSSTRUCT(JsDPoint2d);
JSSTRUCT(JsDVector3d);
JSSTRUCT(JsDVector2d);
JSSTRUCT(JsDEllipse3d);
JSSTRUCT(JsDSegment3d);
JSSTRUCT(JsDRay3d);
JSSTRUCT(JsDRange3d);
JSSTRUCT(JsDPoint3dDVector3dDVector3d);
JSSTRUCT(JsCurvePrimitive);
JSSTRUCT(JsCurveVector);
JSSTRUCT(JsSolidPrimitive);
JSSTRUCT(JsBsplineCurve);
JSSTRUCT(JsBsplineSurface);
JSSTRUCT(JsAngle);
JSSTRUCT(JsYawPitchRollAngles);
JSSTRUCT(JsRotMatrix);
JSSTRUCT(JsTransform);
JSSTRUCT(JsDPoint3dArray)
JSSTRUCT(JsDoubleArray)

JSSTRUCT(JsPolyfaceMesh)
JSSTRUCT(JsPolyfaceVisitor)


JSSTRUCT(JsDgnConeDetail)
JSSTRUCT(JsDgnSphereDetail)
JSSTRUCT(JsDgnTorusPipeDetail)
JSSTRUCT(JsDgnBoxDetail)
JSSTRUCT(JsSolidPrimitive)


// Forward declare access methods so JsDPoint2d and JsDPoint3d can query their vector peers.
DVec3d GetData (JsDVector3dP);
DVec2d GetData (JsDVector2dP);
JsDVector3dP CreateJsVector (DVec3dCR data);
JsDVector2dP CreateJsVector (DVec2dCR data);
// Create vector with specfied length -- return nullptr if input vector is zero length
JsDVector3dP CreateJsVector (DVec3dCR data, double length);
// Create vector with specfied length -- return nullptr if input vector is zero length
JsDVector2dP CreateJsVector (DVec2dCR data, double length);

END_BENTLEY_DGNPLATFORM_NAMESPACE

// Consistent method names for A.Plus(U) === A+U and similar expressions.
// To be expanded in DPoint2d, DPoint3d, DVector2d, DVector3d with "AType" as the immediate class
// and "BType" as the vector type.
// ASSUME there is a GetData (BTypeP) method.
#define DeclareAndImplementMethods_AddVector(NativeAType,AType,ATypeP,NativeBType,BType,BTypeP) \
    ATypeP Interpolate (double fraction, ATypeP right)\
        {return new AType(NativeAType::FromInterpolate(m_data, fraction,right->m_data));}\
    ATypeP Plus (BTypeP vector)\
        {return new AType(NativeAType::FromSumOf(m_data, GetData (vector), 1.0)); }\
    ATypeP Minus (BTypeP vector)\
        {return new AType (NativeAType::FromSumOf (m_data, GetData (vector), -1.0));}\
    ATypeP PlusScaled (BTypeP vector,double scalar)\
        {return new AType(NativeAType::FromSumOf(m_data, GetData (vector), scalar));}\
    ATypeP Plus2Scaled (BTypeP vectorA, double scalarA,  BTypeP vectorB, double scalarB)\
        {return new AType(NativeAType::FromSumOf(m_data, GetData (vectorA), scalarA, GetData(vectorB),scalarB)); }\
    ATypeP Plus3Scaled (BTypeP vectorA, double scalarA,  BTypeP vectorB, double scalarB, BTypeP vectorC, double scalarC)\
        {return new AType(NativeAType::FromSumOf(m_data, GetData (vectorA), scalarA, GetData(vectorB),scalarB,  GetData(vectorC),scalarC));}\
    BTypeP VectorTo (ATypeP other)\
        {return CreateJsVector (NativeBType::FromStartEnd(m_data, other->m_data));}\
    BTypeP UnitVectorTo (ATypeP other)\
        {return CreateJsVector (NativeBType::FromStartEnd(m_data, other->m_data), 1.0);}

// Distance and DistanceSquared methods between instance and other ...
#define DeclareAndImplementMethods_Distance(JsTypeP) \
    double Distance (JsTypeP other){return m_data.Distance (other->m_data);}\
    double DistanceSquared (JsTypeP other){return m_data.DistanceSquared (other->m_data);}\
    double MaxAbsDiff (JsTypeP other){return m_data.MaxDiff (other->m_data);}\
    double MaxAbs (){return m_data.MaxAbs ();}

// Magnitude, MagnitudeSquared, and other vector methods for instance
#define DeclareAndImplementMethods_VectorMagnitudeAndScaling(JsVectorTypeP) \
    double Magnitude (){return m_data.Magnitude ();}\
    double MagnitudeSquared (){return m_data.MagnitudeSquared ();}\
    JsVectorTypeP Normalize () { return CreateJsVector (Get (), 1.0);}\
    JsVectorTypeP ScaleToLength (double length) { return CreateJsVector (Get(), length);}

 // Methods that have point inputs and vector outputs
 #define DeclareAndImplementMethods_VectorTo(NativeVectorType,PointTypeP,VectorTypeP) \
    VectorTypeP VectorTo(PointTypeP other){return NativeVectorType::FromStartEnd (m_data, other->m_data);}

#include <DgnPlatform/GeomJsTypes/JsAngle.h>
#include <DgnPlatform/GeomJsTypes/JSDPoint3d.h>
#include <DgnPlatform/GeomJsTypes/JSDVector3d.h>
#include <DgnPlatform/GeomJsTypes/JSDPoint2d.h>
#include <DgnPlatform/GeomJsTypes/JSDVector2d.h>
#include <DgnPlatform/GeomJsTypes/JSDVector2d.h>
#include <DgnPlatform/GeomJsTypes/JSDRange3d.h>


#include <DgnPlatform/GeomJsTypes/JSYawPitchRollAngles.h>
#include <DgnPlatform/GeomJsTypes/JSDRay3d.h>
#include <DgnPlatform/GeomJsTypes/JSDPoint3dDVector3dDVector3d.h>
#include <DgnPlatform/GeomJsTypes/JsDSegment3d.h>
#include <DgnPlatform/GeomJsTypes/JsDEllipse3d.h>
#include <DgnPlatform/GeomJsTypes/JSRotMatrix.h>
#include <DgnPlatform/GeomJsTypes/JSTransform.h>


#include <DgnPlatform/GeomJsTypes/JsDPoint3dArray.h>

#include <DgnPlatform/GeomJsTypes/JSBsplineCurve.h>

#include <DgnPlatform/GeomJsTypes/JsCurvePrimitive.h>
#include <DgnPlatform/GeomJsTypes/JsPolyfaceMesh.h>
#include <DgnPlatform/GeomJsTypes/JsPolyfaceVisitor.h>
#include <DgnPlatform/GeomJsTypes/JsDgnXXXDetail.h>


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

