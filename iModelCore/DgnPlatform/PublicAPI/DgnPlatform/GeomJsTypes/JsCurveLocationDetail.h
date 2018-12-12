/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsCurveLocationDetail.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef JSCURVELOCATIONDETAIL_H_
#define JSCURVELOCATIONDETAIL_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                   Earlin.Lutz  3/16
// WARNING:  This is a PLAIN structure -- non-refcounted -- intended to be carried in arrays.
// Note that compared to the native CurveLocationDetail:
// 1) It does not carry componentFraction, numComponent, and componentIndex
// 2) It has a full ICurvePrimitivePtr, not just the pair ICurvePrimitive*
//=======================================================================================
struct JsCurveLocationDetail : BeProjectedRefCounted
{
private:
ICurvePrimitivePtr m_curve;
double m_fraction;
DPoint3d m_point;
double   m_a;
public:
JsCurveLocationDetail (CurveLocationDetailCR detail)
    {
    m_curve     = const_cast <ICurvePrimitiveP>(detail.curve); // promote to Ptr !!!
    m_fraction = detail.fraction;
    m_point    = detail.point;
    m_a        = detail.a;
    }

double GetA () const { return m_a;}
void   SetA (double a){ m_a = a;}

double GetFraction () const { return m_fraction;}
void   SetFraction (double fraction){ m_fraction = fraction;}

JsDPoint3dP GetPoint () const { return new JsDPoint3d (m_point);}
void SetPoint (JsDPoint3dP point) {m_point = point->Get ();}

JsCurvePrimitiveP GetCurve () const {return JsCurvePrimitive::StronglyTypedJsCurvePrimitive (m_curve, false);}
void SetCurve (JsCurvePrimitiveP curve) {m_curve = curve->GetICurvePrimitivePtr ();}
};


//=======================================================================================
// @bsiclass                                                   Earlin.Lutz  3/16
// WARNING:  This is a PLAIN structure -- non-refcounted -- intended to be carried in arrays.
//=======================================================================================
struct JsPartialCurveDetailPair
{
private:
PartialCurveDetail m_detailA;
PartialCurveDetail m_detailB;
public:
JsPartialCurveDetailPair (PartialCurveDetailCR detailA, PartialCurveDetailCR detailB)
    : m_detailA (detailA), m_detailB (detailB)
    {
    }

double GetFractionA0 () const { return m_detailA.fraction0;}
void   SetFractionA0 (double fraction){ m_detailA.fraction0 = fraction;}

double GetFractionA1 () const { return m_detailA.fraction1;}
void   SetFractionA1 (double fraction){ m_detailA.fraction1 = fraction;}

JsCurvePrimitiveP GetCurveA () const {return JsCurvePrimitive::StronglyTypedJsCurvePrimitive (m_detailA.parentCurve, false);}
void SetCurveA (JsCurvePrimitiveP curveA) {m_detailB.parentCurve = curveA->GetICurvePrimitivePtr ();}

double GetFractionB0 () const { return m_detailB.fraction0;}
void   SetFractionB0 (double fraction){ m_detailB.fraction0 = fraction;}

double GetFractionB1 () const { return m_detailB.fraction1;}
void   SetFractionB1 (double fraction){ m_detailB.fraction1 = fraction;}

JsCurvePrimitiveP GetCurveB () const {return JsCurvePrimitive::StronglyTypedJsCurvePrimitive (m_detailB.parentCurve, false);}
void SetCurveB (JsCurvePrimitiveP curveB) {m_detailA.parentCurve = curveB->GetICurvePrimitivePtr ();}

bool IsSinglePointPair () const {return m_detailA.IsSingleFraction () && m_detailB.IsSingleFraction ();}

};
//=======================================================================================
// @bsiclass                                                    Earlin.Lutz      03/16
//=======================================================================================
struct JsPartialCurveDetailPairArray : BeProjectedRefCounted
{
private:
    bvector<JsPartialCurveDetailPair> m_data;
    JsPartialCurveDetailPair * Data (double number) const
        {
        size_t index;
        if (TryDoubleToIndex (number, m_data.size (), index))
            {
            return const_cast <JsPartialCurveDetailPair*> (&m_data[index]);
            }
        return nullptr;
        }
public:
    JsPartialCurveDetailPairArray() {}
    static JsPartialCurveDetailPairArrayP Create (CurveVectorCR dataA, CurveVectorCR dataB)
        {
        if (dataA.size () == 0 || dataB.size () != dataA.size ())
            return nullptr;
        auto data = new JsPartialCurveDetailPairArray ();
        for (size_t i = 0; i < dataA.size (); i++)
            {
            auto partialA = dataA[i]->GetPartialCurveDetailCP ();
            auto partialB = dataB[i]->GetPartialCurveDetailCP ();
            if (partialA != nullptr && partialB != nullptr)
                {
                data->m_data.push_back (JsPartialCurveDetailPair (*partialA, *partialB));
                }
            }
        return data;
        }

double GetFractionA0 (double number) const {auto data = Data(number);  return nullptr != data ? data->GetFractionA0 () : 0.0;}
double  GetFractionA1 (double number) const { auto data = Data(number);  return nullptr != data ? data->GetFractionA1 () : 0.0;}
JsCurvePrimitiveP GetCurveA (double number) const {auto data = Data(number);  return nullptr != data ? data->GetCurveA () : nullptr;}
double GetFractionB0 (double number) const { auto data = Data(number);  return nullptr != data ? data->GetFractionB0 () : 0.0;}
double GetFractionB1 (double number) const { auto data = Data(number);  return nullptr != data ? data->GetFractionB1 () : 0.0;}
JsCurvePrimitiveP GetCurveB (double number) const {auto data = Data(number);  return nullptr != data ? data->GetCurveB () : nullptr;}

bool IsSinglePointPair (double number) const {auto data = Data(number);  return nullptr != data ? data->IsSinglePointPair () : false;}

};

//=======================================================================================
// @bsiclass                                                    Earlin.Lutz      03/16
//=======================================================================================
struct JsCurveCurve : BeProjectedRefCounted
{
private: JsCurveCurve (){}  // STATIC CLASS -- NO INSTANCES
public:
static JsPartialCurveDetailPairArrayP IntersectPrimitivesXY (JsCurvePrimitiveP curveA, JsCurvePrimitiveP curveB, bool extend)
    {
    CurveVectorPtr intersectionA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr intersectionB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveCurve::IntersectionsXY (
                *intersectionA,  *intersectionB,
                curveA->GetICurvePrimitivePtr ().get (),
                curveB->GetICurvePrimitivePtr ().get (),
                nullptr, extend
                );
    return JsPartialCurveDetailPairArray::Create (*intersectionA, *intersectionB);
    }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef JSCURVELOCATIONDETAIL_H_

