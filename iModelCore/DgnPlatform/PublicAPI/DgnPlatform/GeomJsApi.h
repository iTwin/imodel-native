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
#include <Geom/DPoint3d.h>
#include <Geom/Angle.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#define JSSTRUCT(_JsStructName_) \
struct _JsStructName_;  \
typedef struct _JsStructName_ * _JsStructName_##P;

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


//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDPoint3d : RefCountedBase
{
friend struct JsDVector3d;
friend struct JsDEllipse3d;
friend struct JsDSegment3d;
friend struct JsCurvePrimitive;
friend struct JsCurveVector;
friend struct JsDVector3d;
friend struct JsDRay3d;
friend struct JsDPoint3dDVector3dDVector3d;

private:
    DPoint3d m_point;

public:   
    JsDPoint3d() {m_point.Init(0,0,0);}
    JsDPoint3d(DPoint3dCR pt) : m_point(pt) {;}
    JsDPoint3d(double x, double y, double z) {m_point.x=x; m_point.y=y; m_point.z=z;}
    DPoint3d GetDPoint3d (){return m_point;}

    double GetX() {return m_point.x;}
    double GetY() {return m_point.y;}
    double GetZ() {return m_point.z;}
    void SetX(double v) {m_point.x = v;}
    void SetY(double v) {m_point.y = v;}
    void SetZ(double v) {m_point.z = v;}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDVector3d : RefCountedBase
{
    DVec3d m_vector;

    JsDVector3d() {m_vector.Init(0,0,0);}
    JsDVector3d(DVec3dCR pt) : m_vector(pt) {;}
    JsDVector3d(double x, double y, double z) {m_vector.x=x; m_vector.y=y; m_vector.z=z;}

    double GetX() {return m_vector.x;}
    double GetY() {return m_vector.y;}
    double GetZ() {return m_vector.z;}
    void SetX(double v) {m_vector.x = v;}
    void SetY(double v) {m_vector.y = v;}
    void SetZ(double v) {m_vector.z = v;}
};



//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsYawPitchRollAngles : RefCountedBase
{
private:
    YawPitchRollAngles m_angles;
public:    

    JsYawPitchRollAngles() {m_angles.FromDegrees(0,0,0);}
    JsYawPitchRollAngles(YawPitchRollAnglesCR angles) : m_angles(angles) {;}
    JsYawPitchRollAngles(double yaw, double pitch, double roll) : m_angles(YawPitchRollAngles::FromDegrees(yaw,pitch,roll)) {;}

    YawPitchRollAngles GetYawPitchRollAngles () { return m_angles;}
    double GetYawDegrees  () {return m_angles.GetYaw().Degrees();}
    double GetPitchDegrees() {return m_angles.GetPitch().Degrees();}
    double GetRollDegrees () {return m_angles.GetRoll().Degrees();}
    void SetYawDegrees  (double v) {m_angles.SetYaw   (AngleInDegrees::FromDegrees (v));}
    void SetPitchDegrees(double v) {m_angles.SetPitch (AngleInDegrees::FromDegrees (v));}
    void SetRollDegrees (double v) {m_angles.SetRoll  (AngleInDegrees::FromDegrees (v));}
};


//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDRay3d : RefCountedBase
{
    DRay3d m_data;

    JsDRay3d() {}
    JsDRay3d (DPoint3dCR point, DVec3dCR vector) {m_data.InitFromOriginAndVector (point, vector);}
    JsDRay3d (JsDPoint3dP point, JsDVector3dP vector) {m_data.InitFromOriginAndVector (point->m_point, vector->m_vector);}

    JsDRay3d (double x0, double y0, double z0, double ux, double uy, double uz)
        {m_data.InitFromOriginAndVector (DPoint3d::From (x0, y0, z0), DVec3d::From (ux, uy, uz));}

    JsDPoint3dP GetStartPoint() {return new JsDPoint3d (m_data.origin);}
    JsDVector3dP     GetVector () {return new JsDVector3d (m_data.direction);}
    void SetStartPoint (JsDPoint3dP point) {m_data.origin = point->m_point;}
    void SetVector (JsDVector3dP vector) {m_data.direction = vector->m_vector;}

    JsDPoint3dP PointAtFraction (double f)
        {
        return new JsDPoint3d (m_data.FractionParameterToPoint (f));
        }
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDPoint3dDVector3dDVector3d: RefCountedBase
{
private:
    DPlane3dByVectors m_data;
public:
    JsDPoint3dDVector3dDVector3d (DPoint3dCR origin, DVec3dCR vectorU, DVec3dCR vectorV)
        : m_data (origin, vectorU, vectorV)
        {}
    JsDPoint3dDVector3dDVector3d (JsDPoint3dP origin, JsDVector3dP vectorU, JsDVector3dP vectorV)
        : m_data (origin->m_point, vectorU->m_vector, vectorV->m_vector)
        {}

    JsDPoint3dP Point (){return new JsDPoint3d (m_data.origin);}
    JsDVector3dP VectorU (){return new JsDVector3d (m_data.vectorU);}
    JsDVector3dP VectorV (){return new JsDVector3d (m_data.vectorV);}
};


//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDSegment3d : RefCountedBase
{
    DSegment3d m_segment;

    JsDSegment3d() {m_segment.InitZero ();}
    JsDSegment3d (DPoint3dCR point0, DPoint3dCR point1) {m_segment.Init (point0, point1);}
    JsDSegment3d (JsDPoint3dP point0, JsDPoint3dP point1) {m_segment.Init (point0->m_point, point1->m_point);}

    JsDSegment3d (double x0, double y0, double z0, double x1, double y1, double z1)
        {m_segment.Init (x0, y0, z0, x1, y1, z1);}

    JsDPoint3dP GetStartPoint() {return new JsDPoint3d (m_segment.point[0]);}
    JsDPoint3dP GetEndPoint() {return new JsDPoint3d (m_segment.point[1]);}
    void SetStartPoint (JsDPoint3dP point) {m_segment.point[0] = point->m_point;}
    void SetEndPoint (JsDPoint3dP point) {m_segment.point[1] = point->m_point;}

    JsDPoint3dP PointAtFraction (double f)
        {
        DPoint3d xyz;
        m_segment.FractionParameterToPoint (xyz, f);
        return new JsDPoint3d (xyz);
        }
    JsDRay3dP PointAndDerivativeAtFraction (double f)
        {
        DPoint3d xyz;
        DVec3d tangent;
        m_segment.FractionParameterToTangent (xyz, tangent, f);
        return new JsDRay3d (xyz, tangent);
        }
};


//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsAngle : RefCountedBase
{
AngleInDegrees m_angle;

JsAngle (AngleInDegrees a) : m_angle (a){}
JsAngle (){m_angle = AngleInDegrees::FromDegrees (0.0);}
JsAngle *FromDegrees (double degrees){return new JsAngle (AngleInDegrees::FromDegrees (degrees));}
JsAngle *FromRadians (double radians){return new JsAngle (AngleInDegrees::FromRadians (radians));}
double GetRadians (){return m_angle.Radians ();}
double GetDegrees (){return m_angle.Degrees ();}
void SetRadians (double radians){m_angle = AngleInDegrees::FromRadians (radians);}
void SetDegrees (double degrees){m_angle = AngleInDegrees::FromDegrees (degrees);}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDEllipse3d : RefCountedBase
{
    friend struct JsCurvePrimitive;
    private:
    DEllipse3d m_ellipse;

    JsDEllipse3d (DEllipse3dCR ellipse) : m_ellipse (ellipse)
        {
        }
    public:
    JsDEllipse3d() {}
    JsDEllipse3d (JsDPoint3dP center, JsDVector3dP vector0, JsDVector3dP vector90,
                JsAngleP startAngle, JsAngleP sweepAngle)
        : m_ellipse(DEllipse3d::FromVectors (
                    center->m_point, vector0->m_vector, vector90->m_vector,
                    startAngle->m_angle.Radians (),
                    sweepAngle->m_angle.Radians ()
                    ))
         {}
       
    static JsDEllipse3dP FromCoordinates (
            double cx, double cy, double cz,
            double v0x, double v0y, double v0z,
            double v90x, double v90y, double v90z,
            JsAngleP startAngle,
            JsAngleP sweepAngle)
            {
            return new JsDEllipse3d
                (DEllipse3d::From (
                    cx, cy, cz,
                    v0x, v0y, v0z,
                    v90x, v90y, v90z,
                    startAngle->m_angle.Radians (),
                    sweepAngle->m_angle.Radians ()
                ));            
            }

    JsDPoint3d * GetCenter() {return new JsDPoint3d (m_ellipse.center);}
    JsDVector3d * GetVector0() {return new JsDVector3d (m_ellipse.vector0);}
    JsDVector3d * GetVector90() {return new JsDVector3d (m_ellipse.vector90);}
    JsAngle * GetStartAngle () {return new JsAngle (AngleInDegrees::FromRadians (m_ellipse.start));}
    JsAngle * GetSweepAngle () {return new JsAngle (AngleInDegrees::FromRadians (m_ellipse.sweep));}

    void SetCenter     (JsDPoint3dP center) {m_ellipse.center = center->m_point;}
    void SetVector0    (JsDVector3dP vector0) {m_ellipse.vector0 = vector0->m_vector;}
    void SetVector90   (JsDVector3dP vector90) {m_ellipse.vector90 = vector90->m_vector;}
    void SetStartAngle (JsAngleP angle) {m_ellipse.start = angle->m_angle.Radians ();}
    void SetSweepAngle (JsAngleP angle) {m_ellipse.sweep = angle->m_angle.Radians ();}
    
    JsDPoint3dP PointAtFraction (double f)
        {
        DPoint3d xyz;
        m_ellipse.FractionParameterToPoint (xyz, f);
        return new JsDPoint3d (xyz);
        }
    JsDRay3dP PointAndDerivativeAtFraction (double f)
        {
        DPoint3d xyz;
        DVec3d derivative1;
        DVec3d derivative2;
        m_ellipse.FractionParameterToDerivatives (xyz, derivative1, derivative2, f);
        return new JsDRay3d (xyz, derivative1);
        }
    
};



//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsCurvePrimitive: RefCountedBase
{
    ICurvePrimitivePtr m_curvePrimitive;
    JsCurvePrimitive () {}

    JsCurvePrimitive (ICurvePrimitivePtr curvePrimitive) : m_curvePrimitive (curvePrimitive) {}

    static JsCurvePrimitiveP CreateLineSegment (JsDSegment3dP segment)
        {
        ICurvePrimitivePtr cp = ICurvePrimitive::CreateLine (segment->m_segment);
        return new JsCurvePrimitive (cp);
        }
    
    double CurvePrimitiveType (){return (double)(int)m_curvePrimitive->GetCurvePrimitiveType ();}
    JsDPoint3dP PointAtFraction (double f)
        {
        DPoint3d xyz;
        if (m_curvePrimitive->FractionToPoint (f, xyz))
            {
            return new JsDPoint3d (xyz);
            }
        return nullptr;
        }

    JsDRay3dP PointAndDerivativeAtFraction (double f)
        {
        DPoint3d xyz;
        DVec3d derivative1;
        if (m_curvePrimitive->FractionToPoint (f, xyz, derivative1))
            {
            return new JsDRay3d (xyz, derivative1);
            }
        return nullptr;
        }
};


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

