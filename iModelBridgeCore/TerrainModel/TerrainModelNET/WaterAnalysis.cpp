/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/WaterAnalysis.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include <vcclr.h>
#include "dtm.h"
#include "dtmexception.h"
#include "TerrainModel\Drainage\WaterAnalysis.h"

#if defined(Public)

#undef Public
#endif
#using <mscorlib.dll>

#include <TerrainModel\Core\bcDTMStream.h>
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

public ref class WaterAnalysis
    {
    public: enum class ZeroSlopeTraceOption
        {
        None = 0,
        TraceLastAngle = 1,
        Pond = 2,
        };
    private:
    DrainageTracer* m_drainageTracer;
    DTM^ m_dtm;
    private:
        WaterAnalysis(DrainageTracer* drainageTracer)
            {
            m_drainageTracer = drainageTracer;
            m_drainageTracer->AddRef();
            }
    public:
        WaterAnalysis(DTM^ dtm) : m_dtm(dtm)
            {
            auto drainageTracer = DrainageTracer::Create(*dtm->Handle);
            m_drainageTracer = drainageTracer.get();
            m_drainageTracer->AddRef();
            }
        !WaterAnalysis()
            {
            if (m_drainageTracer)
                {
                m_drainageTracer->Release();
                m_drainageTracer = nullptr;
                }
            }
        ~WaterAnalysis()
            {
            WaterAnalysis::!WaterAnalysis();
            }

        WaterAnalysis^ Clone()
            {
            auto drainageTracer = m_drainageTracer->Clone();
            return gcnew WaterAnalysis(drainageTracer.get());
            }

        void DoTrace(BGEO::DPoint3d startPt)
            {
            DPoint3d pt;
            DTMHelpers::Copy (pt, startPt);

            m_drainageTracer->DoTrace(pt);
            }

        void AddWaterVolume(BGEO::DPoint3d startPt, double volume)
            {
            DPoint3d pt;
            DTMHelpers::Copy (pt, startPt);

            m_drainageTracer->AddWaterVolume(pt, volume);
            }

        void DoTraceCallback(DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
            {
            DTMDynamicFeaturesBrowsingCallbackForwarder forwarder (m_dtm, hdlP, oArg);
            m_drainageTracer->DoTraceCallback(&DynamicFeaturesBrowsingCallbackForwarderDelegate, &forwarder);
            }


        property double PondElevationTolerance
            {
            double get()
                {
                return m_drainageTracer->GetPondElevationTolerance();
                }
            void set(double value)
                {
                m_drainageTracer->SetPondElevationTolerance(value);
                }
            }

        property double PondVolumeTolerance
            {
            double get()
                {
                return m_drainageTracer->GetPondVolumeTolerance();
                }
            void set(double value)
                {
                m_drainageTracer->SetPondVolumeTolerance(value);
                }
            }

        property double MinimumDepth
            {
            double get()
                {
                return m_drainageTracer->GetMinimumDepth();
                }
            void set(double value)
                {
                m_drainageTracer->SetMinimumDepth(value);
                }
            }

        property WaterAnalysis::ZeroSlopeTraceOption ZeroSlopeOption
            {
            WaterAnalysis::ZeroSlopeTraceOption get()
                {
                return (ZeroSlopeTraceOption)m_drainageTracer->GetZeroSlopeOption();
                }
            void set(WaterAnalysis::ZeroSlopeTraceOption value)
                {
                m_drainageTracer->SetZeroSlopeOption((Bentley::TerrainModel::ZeroSlopeTraceOption)value);
                }
            }


    };

END_BENTLEY_TERRAINMODELNET_NAMESPACE
