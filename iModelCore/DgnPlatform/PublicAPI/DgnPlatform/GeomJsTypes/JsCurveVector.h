/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsCurveVector.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
struct JsCurveVector: JsGeometry
{
friend struct JsUnionRegion;
friend struct JsParityRegion;
friend struct JsPath;
friend struct JsLoop;
friend struct JsUnstructuredCurveVector;
protected :
    CurveVectorPtr m_curveVector;

    void Set (CurveVectorPtr const &curveVector)
        {
        m_curveVector = curveVector;
        }
public:
    JsCurveVector () {}

    virtual JsCurveVectorP AsCurveVector () {return this;}


    JsCurveVectorP StgronglyTypedJsCurveVector (CurveVectorPtr &data);
    JsCurveVector (CurveVectorPtr curveVector) : m_curveVector (curveVector) {}
    JsCurveVectorP Clone () {return new JsCurveVector (m_curveVector->Clone ());} 

    // wrap a native curve vector as the strongest Js type possible . . .
    static JsCurveVectorP StronglyTypedJsCurveVector (CurveVectorPtr &data);


    virtual CurveVectorPtr GetCurveVectorPtr () override {return m_curveVector;}
    double BoundaryType (){return (double)(int)m_curveVector->GetBoundaryType ();}

    JsCurveVectorP MemberAsCurveVector (double doubleIndex) const;
    JsCurvePrimitiveP MemberAsCurvePrimitive (double index) const;


    virtual JsDRange3dP RangeAfterTransform (JsTransformP jsTransform) override
        {
        DRange3d range;
        Transform transform = jsTransform->Get ();
        m_curveVector->GetRange (range, transform);
        return new JsDRange3d (range);
        }
    virtual JsDRange3dP Range () override
        {
        DRange3d range;
        m_curveVector->GetRange (range);
        return new JsDRange3d (range);
        }
     virtual bool TryTransformInPlace (JsTransformP jsTransform) override
        {
        Transform transform = jsTransform->Get ();
        return m_curveVector->TransformInPlace (transform);
        }


    virtual bool IsSameStructureAndGeometry (JsGeometryP other) override
        {
        CurveVectorPtr otherVector;
        if (other != nullptr
            && (otherVector = other->GetCurveVectorPtr (), otherVector.IsValid ())       // COMMA
            )
            {
            return m_curveVector->IsSameStructureAndGeometry (*otherVector);
            }
        return false;
        }

    virtual bool IsSameStructure (JsGeometryP other) override
        {
        CurveVectorPtr otherVector;
        if (other != nullptr
            && (otherVector = other->GetCurveVectorPtr (), otherVector.IsValid ())       // COMMA
            )
            {
            return m_curveVector->IsSameStructure (*otherVector);
            }
        return false;
        }



};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsUnstructuredCurveVector : JsCurveVector
{

    JsUnstructuredCurveVector (CurveVectorPtr const &curveVector)
        {
        Set (curveVector);
        }

public:
    JsUnstructuredCurveVector ()
        {
        Set (CurveVector::Create(CurveVector::BOUNDARY_TYPE_None));
        }
    void Add (JsCurvePrimitiveP primitive){m_curveVector->Add (primitive->GetICurvePrimitivePtr ());}
    void Add (JsCurveVectorP child){m_curveVector->Add (child->GetCurveVectorPtr ());}
    JsUnstructuredCurveVectorP Clone () {return new JsUnstructuredCurveVector(m_curveVector->Clone ());} 
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
    void Add (JsCurvePrimitiveP primitive){m_curveVector->Add (primitive->GetICurvePrimitivePtr ());}
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
public:
    JsLoop ()
        {
        Set (CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer));
        }

    JsLoopP Clone () {return new JsLoop (m_curveVector->Clone ());} 
    void Add (JsCurvePrimitiveP primitive){m_curveVector->Add (primitive->GetICurvePrimitivePtr ());}
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

