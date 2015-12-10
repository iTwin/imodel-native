/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsCurveVector.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSCURVEVECTOR_H_
#define _JSCURVEVECTOR_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsCurveVector: RefCountedBase
{
friend struct JsUnionRegion;
protected :
    CurveVectorPtr m_curveVector;

    void Set (CurveVectorPtr const &curveVector)
        {
        m_curveVector = curveVector;
        }
public:
    JsCurveVector () {}

    JsCurveVector (CurveVectorPtr curveVector) : m_curveVector (curveVector) {}
    JsCurveVectorP Clone () {return new JsCurveVector (m_curveVector->Clone ());} 


    CurveVectorPtr GetCurveVectorPtr (){return m_curveVector;}
    double BoundaryType (){return (double)(int)m_curveVector->GetBoundaryType ();}
};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsPath : JsCurveVector
{
    JsPath (CurveVectorPtr const &path)
        {
        Set (path);
        }

public:
    JsPath ()
        {
        Set (CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open));
        }
    void Add (JsCurvePrimitiveP primitive){m_curveVector->Add (primitive->Get ());}
    JsPathP Clone () {return new JsPath (m_curveVector->Clone ());} 
};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsLoop : JsCurveVector
{
    JsLoop (CurveVectorPtr const &path)
        {
        Set (path);
        }
friend struct JsParityRegion;
friend struct JsUnionRegion;
public:
    JsLoop ()
        {
        Set (CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer));
        }

    JsLoopP Clone () {return new JsLoop (m_curveVector->Clone ());} 
    void Add (JsCurvePrimitiveP primitive){m_curveVector->Add (primitive->Get ());}
};


//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsParityRegion : JsCurveVector
{
    JsParityRegion (CurveVectorPtr const &path)
        {
        Set (path);
        }

public:
    JsParityRegion ()
        {
        Set (CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion));
        }

    JsParityRegion * Clone () {return new JsParityRegion (m_curveVector->Clone ());} 
    void Add (JsLoopP loop){m_curveVector->Add (loop->m_curveVector);}
};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsUnionRegion : JsCurveVector
{
    JsUnionRegion (CurveVectorPtr const &path)
        {
        Set (path);
        }

public:
    JsUnionRegion ()
        {
        Set (CurveVector::Create(CurveVector::BOUNDARY_TYPE_UnionRegion));
        }

    JsUnionRegion * Clone () {return new JsUnionRegion (m_curveVector->Clone ());} 
    // The curveVector types that are permitted in a union region are: JsLoop, JsParityRegion.
    // This is not checked ... (which matches the native side behavior)
    void Add (JsCurveVectorP child){m_curveVector->Add (child->m_curveVector);}
};



END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSCURVEVECTOR_H_

