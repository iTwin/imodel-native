/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include <vcclr.h>
#include "dtm.h"
#include "dtmexception.h"
#include "TerrainModel/Drainage/WaterAnalysis.h"

#if defined(Public)

#undef Public
#endif
#using <mscorlib.dll>

#include <TerrainModel/Core/bcDTMStream.h>
#include "Caching.h"

using namespace System;
USING_NAMESPACE_BENTLEY_TERRAINMODEL
using namespace System::Runtime::InteropServices;

namespace SRI = System::Runtime::InteropServices;

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
/// <summary>
/// Callback frowarder.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private class DTMDynamicFeaturesBrowsingCallbackForwarder
    {
    private:
        gcroot<DTM^>                             m_managedDtm;
        gcroot<System::Object^>                  m_args;
        gcroot<DynamicFeaturesBrowsingDelegate^> m_delegate;
        long pointsLength;

    public:

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMDynamicFeaturesBrowsingCallbackForwarder (DTM^ dtm, DynamicFeaturesBrowsingDelegate^ del)
            {
            m_managedDtm = dtm;
            m_delegate = del;
            pointsLength = -1;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMDynamicFeaturesBrowsingCallbackForwarder (DTM^ dtm, DynamicFeaturesBrowsingDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            pointsLength = -1;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Daryl.Holmwood     08/2008
        //=======================================================================================
        int CacheCallback(bvector<DTMCachedFeature>& features)
            {
            for(DTMCachedFeature feat : features)
                {
                int ret = CallBack(feat.featureType, feat.featureTag, feat.featureId, feat.points.data(), feat.points.size());
                if (ret != DTM_SUCCESS) return ret;
                }
            return DTM_SUCCESS;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        int CallBack
        (
            ::DTMFeatureType featureType,
            ::DTMUserTag    featureTag,
            ::DTMFeatureId  featureId,
            DPoint3dP       tPoint,
            size_t          nPoint
        )
            {
            if (nPoint == 0 || tPoint == nullptr)
                return DTM_SUCCESS;
            DTMDynamicFeatureType ftType;
            DTMHelpers::Copy (ftType, featureType);
            bool ret = false;
            ret = m_delegate->Invoke (gcnew DTMDynamicFeatureInfo (ftType, tPoint, (int)nPoint), m_args);

            return (ret ? 0 : 1);
            }
    };

static int DynamicFeaturesBrowsingCallbackForwarderDelegate
(
    ::DTMFeatureType     featureType,
    ::DTMUserTag   featureTag,
    ::DTMFeatureId featureId,
    DPoint3d       *tPoint,
    size_t           nPoint,
    void           *userP
)
    {
    DTMDynamicFeaturesBrowsingCallbackForwarder* forwarder = (DTMDynamicFeaturesBrowsingCallbackForwarder*)userP;
    return forwarder->CallBack(featureType, featureTag, featureId, tPoint, nPoint);
    }

ref class WaterAnalysisResultPoint;
ref class WaterAnalysisResultStream;
ref class WaterAnalysisResultPond;

public ref class WaterAnalysisResultItem
    {
    private:
        WaterAnalysisResultItemP m_native;
    internal:
        WaterAnalysisResultItem(WaterAnalysisResultItemR native) : m_native(&native)
            {
            m_native->AddRef();
            }
        static WaterAnalysisResultItem^ Create(WaterAnalysisResultItemR item);
    public:
        !WaterAnalysisResultItem()
            {
            if (m_native)
                {
                m_native->Release();
                m_native = nullptr;
                }
            }
        ~WaterAnalysisResultItem()
            {
            WaterAnalysisResultItem::!WaterAnalysisResultItem();
            }
        virtual WaterAnalysisResultPoint^ AsPoint() { return nullptr; }
        virtual WaterAnalysisResultStream^ AsStream() { return nullptr; }
        virtual WaterAnalysisResultPond^ AsPond() { return nullptr; }

        property double WaterVolume
            {
            double get()
                {
                return m_native->GetWaterVolume();
                }
            }
    };

public ref class WaterAnalysisResultPoint : public WaterAnalysisResultItem
    {
    public:
        enum class PointType
            {
            Start,
            Low,
            Exit
            };

    private:
    WaterAnalysisResultPointP m_point;
    internal:
    WaterAnalysisResultPoint(WaterAnalysisResultPointR native) : WaterAnalysisResultItem(native), m_point (&native)
        { }
    public:
        virtual WaterAnalysisResultPoint^ AsPoint() override { return this; }

        property BGEO::DPoint3d Point
            {
            BGEO::DPoint3d get()
                {
                DPoint3d pt = m_point->GetPoint();
                return BGEO::DPoint3d(pt.x, pt.y, pt.z);
                }
            }
        property PointType Type
            {
            PointType get()
                {
                return (PointType)m_point->GetType();
                }
            }
    };

public ref class WaterAnalysisResultGeometry : public WaterAnalysisResultItem
    {
    private:
        WaterAnalysisResultGeometryP m_geom;
    internal:
        WaterAnalysisResultGeometry(WaterAnalysisResultGeometryR native) : WaterAnalysisResultItem(native), m_geom(&native)
            { }
    public:
        property BGEO::CurveVector^ Geometry
            {
            BGEO::CurveVector^ get()
                {
                CurveVectorCR geometry = m_geom->GetGeometry();

                return BGEO::CurveVector::CreateFromNative((System::IntPtr)const_cast<void*>((const void*)&geometry));
                }
            }
    };

public ref class WaterAnalysisResultStream : public WaterAnalysisResultGeometry
    {
    private:
        WaterAnalysisResultStreamP m_stream;
    internal:
        WaterAnalysisResultStream(WaterAnalysisResultStreamR native) : WaterAnalysisResultGeometry(native), m_stream(&native)
            { }
    public:
        virtual WaterAnalysisResultStream^ AsStream() override { return this; }
    };

public ref class WaterAnalysisResultPond : public WaterAnalysisResultGeometry
    {
    private:
        WaterAnalysisResultPondP m_pond;
    internal:
        WaterAnalysisResultPond(WaterAnalysisResultPondR native) : WaterAnalysisResultGeometry(native), m_pond (&native)
            { }
    public:
        virtual WaterAnalysisResultPond^ AsPond() override { return this; }

        property bool IsFull
            {
            bool get()
                {
                return m_pond->IsFull();
                }
            }
        property double Depth
            {
            double get()
                {
                return m_pond->Depth();
                }
            }
    };

WaterAnalysisResultItem^ WaterAnalysisResultItem::Create(WaterAnalysisResultItemR item)
    {
    if (nullptr != item.AsPoint())
        return gcnew WaterAnalysisResultPoint(*item.AsPoint());
    if (nullptr != item.AsStream())
        return gcnew WaterAnalysisResultStream(*item.AsStream());
    if (nullptr != item.AsPond())
        return gcnew WaterAnalysisResultPond(*item.AsPond());
    return nullptr;
    }

public ref class WaterAnalysisResult : public System::Collections::Generic::IEnumerable<WaterAnalysisResultItem^>
    {
    private:

    System::Collections::Generic::List<WaterAnalysisResultItem^>^ m_items = gcnew System::Collections::Generic::List<WaterAnalysisResultItem^>();
internal:
    WaterAnalysisResult(WaterAnalysisResultR results)
        {
        for (auto&& item : results)
            m_items->Add(WaterAnalysisResultItem::Create(*item));
        }
    protected:
    // Inherited via IEnumerable
    virtual System::Collections::IEnumerator ^ GetEnumerator2() sealed = System::Collections::IEnumerable::GetEnumerator
        {
        return m_items->GetEnumerator();
        }
    public:
        virtual System::Collections::Generic::IEnumerator<Bentley::TerrainModelNET::WaterAnalysisResultItem ^> ^ GetEnumerator() sealed
        {
        return m_items->GetEnumerator();
        }
    };

public ref class WaterAnalysis
    {
    public: enum class ZeroSlopeTraceOption
        {
        None = 0,
        TraceLastAngle = 1,
        Pond = 2,
        };
    private:
    TerrainModel::WaterAnalysis* m_analysis;
    DTM^ m_dtm;
    private:
        WaterAnalysis(TerrainModel::WaterAnalysis* analysis)
            {
            m_analysis = analysis;
            m_analysis->AddRef();
            }
    public:
        WaterAnalysis(DTM^ dtm) : m_dtm(dtm)
            {
            auto analysis = TerrainModel::WaterAnalysis::Create(*dtm->Handle);
            m_analysis = analysis.get();
            m_analysis->AddRef();
            }
        !WaterAnalysis()
            {
            if (m_analysis)
                {
                m_analysis->Release();
                m_analysis = nullptr;
                }
            }
        ~WaterAnalysis()
            {
            WaterAnalysis::!WaterAnalysis();
            }

        WaterAnalysisResult^ GetResult()
            {
            return gcnew WaterAnalysisResult(*m_analysis->GetResult());
            }
        WaterAnalysis^ Clone()
            {
            auto drainageTracer = m_analysis->Clone();
            return gcnew WaterAnalysis(drainageTracer.get());
            }

        void DoTrace(BGEO::DPoint3d startPt)
            {
            DPoint3d pt;
            DTMHelpers::Copy (pt, startPt);

            m_analysis->DoTrace(pt);
            }

        void AddWaterVolume(BGEO::DPoint3d startPt, double volume)
            {
            DPoint3d pt;
            DTMHelpers::Copy (pt, startPt);

            m_analysis->AddWaterVolume(pt, volume);
            }

        void DoTraceCallback(DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
            {
            DTMDynamicFeaturesBrowsingCallbackForwarder forwarder (m_dtm, hdlP, oArg);
            m_analysis->DoTraceCallback(&DynamicFeaturesBrowsingCallbackForwarderDelegate, &forwarder);
            }

        property double PondElevationTolerance
            {
            double get()
                {
                return m_analysis->GetPondElevationTolerance();
                }
            void set(double value)
                {
                m_analysis->SetPondElevationTolerance(value);
                }
            }

        property double PondVolumeTolerance
            {
            double get()
                {
                return m_analysis->GetPondVolumeTolerance();
                }
            void set(double value)
                {
                m_analysis->SetPondVolumeTolerance(value);
                }
            }

        property double MinimumDepth
            {
            double get()
                {
                return m_analysis->GetMinimumDepth();
                }
            void set(double value)
                {
                m_analysis->SetMinimumDepth(value);
                }
            }

        property WaterAnalysis::ZeroSlopeTraceOption ZeroSlopeOption
            {
            WaterAnalysis::ZeroSlopeTraceOption get()
                {
                return (ZeroSlopeTraceOption)m_analysis->GetZeroSlopeOption();
                }
            void set(WaterAnalysis::ZeroSlopeTraceOption value)
                {
                m_analysis->SetZeroSlopeOption((Bentley::TerrainModel::ZeroSlopeTraceOption)value);
                }
            }
    };

END_BENTLEY_TERRAINMODELNET_NAMESPACE
