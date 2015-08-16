/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsRotMatrix.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JsRotMatrix_H_
#define _JsRotMatrix_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsRotMatrix : JsGeomWrapperBase<RotMatrix>
{
public:   
    JsRotMatrix() {m_data.Zero();}
    JsRotMatrix(RotMatrixCR data) {m_data = data;}
    JsRotMatrix
            (
            double axx, double axy, double axz,
            double ayx, double ayy, double ayz,
            double azx, double azy, double azz
            )
        {
        m_data.InitFromRowValues (
            axx, axy, axz,
            ayx, ayy, ayz,
            azx, azy, azz
            );
        }
        
    static JsRotMatrixP CreateIdentity (){return new JsRotMatrix (RotMatrix::FromIdentity ());}
    static JsRotMatrixP CreateUniformScale (double s){return new JsRotMatrix (RotMatrix::FromScale (s));}
    static JsRotMatrixP CreateScale (double scaleX, double scaleY, double scaleZ){return new JsRotMatrix (RotMatrix::FromScaleFactors (scaleX, scaleY, scaleZ));}
    static JsRotMatrixP CreateColumns (JsDVector3dP vectorU, JsDVector3dP vectorV, JsDVector3dP vectorZ)
        {return new JsRotMatrix (RotMatrix::FromColumnVectors (vectorU->Get  (), vectorV->Get  (), vectorZ->Get  ()));}
    static JsRotMatrixP CreateRows (JsDVector3dP vectorU, JsDVector3dP vectorV, JsDVector3dP vectorZ)
        {return new JsRotMatrix (RotMatrix::FromRowVectors (vectorU->Get  (), vectorV->Get  (), vectorZ->Get  ()));}
    static JsRotMatrixP CreateRotationAroundVector (JsDVector3dP axis, JsAngleP angle)
        {return new JsRotMatrix (RotMatrix::FromVectorAndRotationAngle (axis->Get  (), angle->GetRadians ()));}
    static JsRotMatrixP Create90DegreeRotationAroundVector (JsDVector3dP axis)
        {return new JsRotMatrix (RotMatrix::FromRotate90 (axis->Get  ()));}
    static JsRotMatrixP CreateDirectionalScale (JsDVector3dP direction, double scale)
        {return new JsRotMatrix (RotMatrix::FromDirectionAndScale (direction->Get  (), scale));}
    // TODO: square and normalize !!!
    
    static JsRotMatrixP Create1Vector (JsDVector3dP direction, double axisIndex)
        {
        // remark: always normalize.
        return new JsRotMatrix (RotMatrix::From1Vector (direction->Get  (), (int)floor (axisIndex), true));
        }
    static JsRotMatrixP CreateFromXYVectors (JsDVector3dP vectorX, JsDVector3dP vectorY, double axisIndex)
        {
        return new JsRotMatrix (RotMatrix::From2Vectors     (vectorX->Get  (), vectorY->Get ()));
        }
    JsDVector3dP MultiplyVector (JsDVector3dP vector)
        {
        DVec3d result;
        m_data.Multiply (result, vector->Get ());
        return new JsDVector3d (result);
        }        
    JsDVector3dP MultiplyTransposeVector (JsDVector3dP vector)
        {
        DVec3d result;
        m_data.MultiplyTranspose (result, vector->Get ());
        return new JsDVector3d (result);
        }        

    JsDVector3dP MultiplyXYZ (JsDVector3dP vector, double x, double y, double z)
        {
        DVec3d result;
        m_data.MultiplyComponents (result, x, y, z);
        return new JsDVector3d (result);
        }        
        
    JsDVector3dP MultiplyTransposeXYZ (JsDVector3dP vector, double x, double y, double z)
        {
        DVec3d result;
        m_data.MultiplyTransposeComponents (result, x, y, z);
        return new JsDVector3d (result);
        }        

    JsDVector3dP Solve (JsDVector3dP rightHandSide)
        {
        DVec3d result;
        if (m_data.Solve (result, rightHandSide->Get ()))
            return new JsDVector3d (result);
        return nullptr;
        }

     JsRotMatrixP MultiplyMatrix (JsRotMatrixP other)
        {
        RotMatrix product;
        product.InitProduct (m_data, other->m_data);
        return new JsRotMatrix (product);
        }

     JsRotMatrixP Transpose (){return new JsRotMatrix (RotMatrix::FromTransposeOf (m_data));}
     JsRotMatrixP Inverse ()
        {
        RotMatrix result;
        if (result.InverseOf (m_data))
            return new JsRotMatrix (result);
        else
            return nullptr;
        }
    
    JsDVector3dP GetColumnX() {return new JsDVector3d (DVec3d::FromColumn (m_data, 0));}
    JsDVector3dP GetColumnY() {return new JsDVector3d (DVec3d::FromColumn (m_data, 1));}
    JsDVector3dP GetColumnZ() {return new JsDVector3d (DVec3d::FromColumn (m_data, 2));}
    JsDVector3dP GetRowX() {return new JsDVector3d (DVec3d::FromRow (m_data, 0));}
    JsDVector3dP GetRowY() {return new JsDVector3d (DVec3d::FromRow (m_data, 1));}
    JsDVector3dP GetRowZ() {return new JsDVector3d (DVec3d::FromRow (m_data, 2));}
    
    void SetColumnX(JsDVector3dP value) {m_data.SetColumn (value->Get (), 0);}
    void SetColumnY(JsDVector3dP value) {m_data.SetColumn (value->Get (), 1);}
    void SetColumnZ(JsDVector3dP value) {m_data.SetColumn (value->Get (), 2);}

    void SetRowX(JsDVector3dP value) {m_data.SetRow (value->Get (), 0);}
    void SetRowY(JsDVector3dP value) {m_data.SetRow (value->Get (), 1);}
    void SetRowZ(JsDVector3dP value) {m_data.SetRow (value->Get (), 2);}


    
    
    double At (double ai, double aj)
        {
        int i = (int)floor (ai);
        int j = (int)floor (aj);
        return m_data.GetComponentByRowAndColumn (i, j);
        }
    void SetAt (double ai, double aj, double value)
        {
        int i = (int)floor (ai);
        int j = (int)floor (aj);
        return m_data.SetComponentByRowAndColumn (i, j, value);
        }
        
    void ScaleColumnsInPlace (double scaleX, double scaleY, double scaleZ)
        {m_data.ScaleColumns (scaleX, scaleY, scaleZ);}

    void ScaleRowsInPlace (double scaleX, double scaleY, double scaleZ)
        {m_data.ScaleRows (m_data,scaleX, scaleY, scaleZ);}

    double Determinant (){return m_data.Determinant ();}
    double ConditionNumber (){return m_data.ConditionNumber ();}
    double SumSquares (){return m_data.SumSquares ();}
    double SumDiagonalSquares (){return m_data.SumDiagonalSquares ();}
    double MaxAbs (){return m_data.MaxAbs ();}
    double MaxDiff (JsRotMatrixP other){return m_data.MaxDiff (other->m_data);}
    
    bool IsIdentity (){return m_data.IsIdentity ();}
    bool IsDigaonal (){return m_data.IsDiagonal ();}
    bool IsSignedPermutation (){return m_data.IsSignedPermutation ();}
    bool IsRigid (){return m_data.IsRigid ();}
    bool HasUnitLengthMutuallyPerpendicularRowsAndColumns (){return m_data.IsOrthogonal ();}
};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JsRotMatrix_H_

