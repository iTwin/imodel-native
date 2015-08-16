/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsTransform.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JsTransform_H_
#define _JsTransform_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsTransform : JsGeomWrapperBase<Transform>
{
public:   
    JsTransform() {m_data.InitIdentity ();}
    JsTransform(TransformCR data) {m_data = data;}
    JsTransform
            (
            double axx, double axy, double axz, double axw,
            double ayx, double ayy, double ayz, double ayw,
            double azx, double azy, double azz, double azw
            )
        {
        m_data.InitFromRowValues (
            axx, axy, axz, axw,
            ayx, ayy, ayz, ayw,
            azx, azy, azz, azw
            );
        }
        
    static JsTransformP CreateIdentity (){return new JsTransform (Transform::FromIdentity ());}
    static JsTransformP CreateMatrix (JsRotMatrixP matrix)
                {return new JsTransform (Transform::From (matrix->Get ()));}
    static JsTransformP CreateMatrixAndTranslation (JsRotMatrixP matrix, JsDPoint3dP translation)
                {return new JsTransform (Transform::From (matrix->Get (), translation->Get ()));}
    static JsTransformP CreateMatrixAndFixedPoint(JsRotMatrixP matrix, JsDPoint3dP fixedPoint)
                {return new JsTransform (Transform::FromMatrixAndFixedPoint (matrix->Get (), fixedPoint->Get ()));}
    static JsTransformP CreateTranslation (JsDPoint3dP translation)
                {return new JsTransform (Transform::From (translation->Get ()));}
    static JsTransformP CreateTranslationXYZ (double x, double y, double z)
                {return new JsTransform (Transform::From (x, y, z));}
    static JsTransformP CreateScaleAroundPoint (JsDPoint3dP fixedPoint, double scaleX, double scaleY, double scaleZ)
                {return new JsTransform (Transform::FromFixedPointAndScaleFactors (fixedPoint->Get (), scaleX, scaleY, scaleZ));}

    static JsTransformP CreateOriginAndVectors (JsDPoint3dP origin, JsDVector3dP xVector, JsDVector3dP yVector, JsDVector3dP zVector)
                {
                return new JsTransform (
                    Transform::FromOriginAndVectors (origin->Get (), xVector->Get (), yVector->Get (), zVector->Get ()));
                }

    static JsTransformP CreateOriginAnd3TargetPoints (JsDPoint3dP origin, JsDPoint3dP xPoint, JsDPoint3dP yPoint, JsDPoint3dP zPoint)
                {
                return new JsTransform (
                    Transform::From4Points (origin->Get (), xPoint->Get (), yPoint->Get (), zPoint->Get ()));
                }

    static JsTransformP CreateOriginAnd2TargetPoints (JsDPoint3dP origin, JsDPoint3dP xPoint, JsDPoint3dP yPoint)
                {
                return new JsTransform (
                    Transform::FromPlaneOf3Points (origin->Get (), xPoint->Get (), yPoint->Get ()));
                }

    static JsTransformP CreateRotationAroundRay (JsDRay3dP ray, JsAngleP angle)
                {
                return new JsTransform (
                    Transform::FromAxisAndRotationAngle (ray->Get (), angle->GetRadians ()));
                }                
    
    
    JsDPoint3dP MultiplyPoint (JsDPoint3dP Point)
        {
        DVec3d result;
        m_data.Multiply (result, Point->Get ());
        return new JsDPoint3d (result);
        }
        

    JsDPoint3dP MultiplyXYZ (JsDPoint3dP Point, double x, double y, double z)
        {
        DVec3d result;
        m_data.Multiply (result, x, y, z);
        return new JsDPoint3d (result);
        }        
        
    JsDVector3dP MultiplyMatrixOnly (JsDVector3dP vector)
        {
        DVec3d result;
        m_data.MultiplyMatrixOnly (result, vector->Get ());
        return new JsDVector3d (result);
        }        
    JsDVector3dP MultiplyTransposeMatrixOnly (JsDVector3dP vector)
        {
        DVec3d result;
        m_data.MultiplyTransposeMatrixOnly (result, vector->Get ());
        return new JsDVector3d (result);
        }        

    JsDVector3dP MultiplyXYZMatrixOnly (JsDVector3dP vector, double x, double y, double z)
        {
        DVec3d result;
        m_data.MultiplyMatrixOnly (result, x, y, z);
        return new JsDVector3d (result);
        }        
        
    JsDVector3dP MultiplyTransposeXYZMatrixOnly (JsDVector3dP vector, double x, double y, double z)
        {
        DVec3d result;
        m_data.MultiplyTransposeMatrixOnly (result, x, y, z);
        return new JsDVector3d (result);
        }        

    JsDPoint3dP Solve (JsDPoint3dP rightHandSide)
        {
        DVec3d result;
        if (m_data.Solve (result, rightHandSide->Get ()))
            return new JsDPoint3d (result);
        return nullptr;
        }

     JsTransformP MultiplyTransform (JsTransformP other)
        {
        Transform product;
        product.InitProduct (m_data, other->m_data);
        return new JsTransform (product);
        }
     JsTransformP Inverse ()
        {
        Transform result;
        if (result.InverseOf (m_data))
            return new JsTransform (result);
        else
            return nullptr;
        }
    JsDVector3dP GetColumnX() {return new JsDVector3d (DVec3d::FromMatrixColumn (m_data, 0));}
    JsDVector3dP GetColumnY() {return new JsDVector3d (DVec3d::FromMatrixColumn (m_data, 1));}
    JsDVector3dP GetColumnZ() {return new JsDVector3d (DVec3d::FromMatrixColumn (m_data, 2));}
    JsDVector3dP GetRowX() {return new JsDVector3d (DVec3d::FromMatrixRow (m_data, 0));}
    JsDVector3dP GetRowY() {return new JsDVector3d (DVec3d::FromMatrixRow (m_data, 1));}
    JsDVector3dP GetRowZ() {return new JsDVector3d (DVec3d::FromMatrixRow (m_data, 2));}
    
    void SetColumnX(JsDVector3dP value) {m_data.SetMatrixColumn (value->Get (), 0);}
    void SetColumnY(JsDVector3dP value) {m_data.SetMatrixColumn (value->Get (), 1);}
    void SetColumnZ(JsDVector3dP value) {m_data.SetMatrixColumn (value->Get (), 2);}

    void SetRowX(JsDVector3dP value) {m_data.SetMatrixRow (value->Get (), 0);}
    void SetRowY(JsDVector3dP value) {m_data.SetMatrixRow (value->Get (), 1);}
    void SetRowZ(JsDVector3dP value) {m_data.SetMatrixRow (value->Get (), 2);}

    JsRotMatrixP GetMatrix (){return new JsRotMatrix (RotMatrix::From (m_data));}
    void SetMatrix (JsRotMatrixP value){m_data.SetMatrix (value->Get ());}

    JsDPoint3dP GetTranslation() {
        DPoint3d xyz;
        m_data.GetTranslation (xyz);
        return new JsDPoint3d (xyz);
        }
    void SetTranslation (JsDPoint3dP value) {m_data.SetTranslation (value->Get ());}
    void ZeroTranslation (){m_data.ZeroTranslation ();}
    void SetFixedPoint (JsDPoint3dP value) {m_data.SetFixedPoint (value->Get ());}
    double MatrixEntryAt (double ai, double aj)
        {
        int i = (int)floor (ai);
        int j = (int)floor (aj);
        return m_data.GetFromMatrixByRowAndColumn (i, j);
        }
    void SetMatrixAt (double ai, double aj, double value)
        {
        int i = (int)floor (ai);
        int j = (int)floor (aj);
        return m_data.SetMatrixByRowAndColumn (i, j, value);
        }

    double Determinant (){return m_data.Determinant ();}
    double MaxDiff (JsTransformP other){return m_data.MaxDiff (other->m_data);}
    double MatrixColumnMagnitude (double axisIndex){return m_data.MatrixColumnMagnitude ((int)floor(axisIndex));}
    bool IsIdentity (){return m_data.IsIdentity ();}
    bool IsRigid (){return m_data.IsRigid ();}
        
    void ScaleMatrixColumnsInPlace (double scaleX, double scaleY, double scaleZ)
        {m_data.ScaleMatrixColumns (scaleX, scaleY, scaleZ);}

};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JsTransform_H_

