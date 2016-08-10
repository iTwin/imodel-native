/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTM.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include <vcclr.h>
#include "dtm.h"
#include "dtmexception.h"
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

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     11/2010
//=======================================================================================
void GetVectorPointsFromIEnumerable (bvector<DPoint3d>& pts, System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points)
    {
    for each (BGEO::DPoint3d pt in points)
        {
        pts.push_back(DPoint3d::From(pt.X, pt.Y, pt.Z));
        }
    }

//=======================================================================================
/// <summary>
/// BcDtmStream wrapper class for System::IO::Stream.
/// </summary>
/// <author>Daryl.Holmwood</author>                              <date>03/2010</date>
//=======================================================================================
struct ManagedBcDTMStream : public IBcDtmStream
    {
    private:
        gcroot<System::IO::Stream^> m_stream;
        __int64 m_startPos;
    public:
        ManagedBcDTMStream(System::IO::Stream^ stream)
            {
            m_stream = stream;
            }

        virtual int Ftell()
            {
            return ( int ) m_stream->Position  ;
            }

        virtual int Seek(long offset, int origin)
            {
            switch(origin)
                {
                case SEEK_CUR:
                    return (int)m_stream->Seek(offset, System::IO::SeekOrigin::Current);
                case SEEK_SET:
                    return (int)m_stream->Seek(offset, System::IO::SeekOrigin::Begin);
                case SEEK_END:
                    return (int)m_stream->Seek(offset, System::IO::SeekOrigin::End);
                }
            return -1;
            }

        virtual size_t Read(void* dest, size_t elementSize, size_t count)
            {
            // Need Error Checking.
            array<System::Byte>^ buffer = gcnew array<System::Byte>((int)(elementSize * count));
            __int64 readSize = m_stream->Read(buffer, 0, (int)(elementSize * count));
            pin_ptr<System::Byte> bufferd = &buffer[0];
            memcpy(dest, bufferd, (size_t)readSize);
            bufferd = nullptr;
            delete [] buffer;
            return (size_t)(readSize / elementSize);
            }

        virtual size_t Write(void* source, size_t elementSize, size_t count)
            {
            // Need Error Checking.
            array<System::Byte>^ buffer = gcnew array<System::Byte>((int)(elementSize * count));
            pin_ptr<System::Byte> bufferd = &buffer[0];
            memcpy(bufferd, source, elementSize * count);
            bufferd = nullptr;

            m_stream->Write(buffer, 0, (int)(elementSize * count));
            delete [] buffer;
            return (size_t)(elementSize * count);
            }
    };

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
/// <summary>
/// Delegate for DuplicatePointsBrowsing
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private delegate int DuplicatePointsBrowsingDelegateUnsafe
(
 double X,
 double Y,
 DTM_DUPLICATE_POINT_ERROR *dupErrorsP,
 long numDupErrors,
 void      *userP
 );

//=======================================================================================
/// <summary>
/// Delegate for crossing feature browsing
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private delegate int CrossingFeaturesBrowsingDelegateUnsafe
(
 DTM_CROSSING_FEATURE_ERROR crossError,
 void      *userP
 );

//=======================================================================================
/// <summary>
/// Delegate for feature browsing
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private delegate int FeatureBrowsingDelegateUnsafe
(
 int            featureType,
 ::DTMUserTag   featureTag,
 ::DTMFeatureId featureId,
 DPoint3d       *tPoint,
 int            nPoint,
 void           *userP
 );

//=======================================================================================
/// <summary>
/// Delegate for point browsing
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private delegate int SinglePointFeaturesBrowsingDelegateUnsafe
(
 int featureType,
 DPoint3d *point
 );

//=======================================================================================
/// <summary>
/// Delegate for slope indicators
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private delegate int SlopeIndicatorDelegateUnsafe
(
 bool major,
 DPoint3d *point1,
 DPoint3d *point2
 );


//=======================================================================================
/// <summary>
/// Callback frowarder.
/// </summary>
/// <author>Daryl.Holmwood</author>                              <date>5/2011</date>
//=======================================================================================
private class DTMTransformCallbackForwarder
    {
    private:
        gcroot<DTM^>                             m_managedDtm;
        gcroot<System::Object^>                  m_args;
        gcroot<DTMTransformCallbackDelegate^> m_delegate;

    public:

        //=======================================================================================
        // @bsimethod                                               Daryl.Holmwood      5/2011
        //=======================================================================================
        DTMTransformCallbackForwarder (DTM^ dtm, DTMTransformCallbackDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Daryl.Holmwood      5/2011
        //=======================================================================================
        int CallBack
            (
            DPoint3d        *tPoint,
            int             nPoint
            )
            {
            if (nPoint == 0 || tPoint == nullptr)
                return DTM_SUCCESS;
            array<BGEO::DPoint3d>^ managedPoints = gcnew array<BGEO::DPoint3d> (nPoint);
            pin_ptr<BGEO::DPoint3d const> pointsP = &managedPoints [0];
            memcpy ((DPoint3d*)pointsP, tPoint, sizeof (DPoint3d) * nPoint);

            int ret = m_delegate->Invoke (managedPoints, m_args);
            memcpy (tPoint, (DPoint3d*)pointsP, sizeof (DPoint3d) * nPoint);
            return ret;
            }
    };

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      5/2011
//=======================================================================================
int TransformPointsUsingCallbackForwarderDelegate
(
 DPoint3dP pts,
 size_t numPts,
 void      *userP
 )
    {
    DTMTransformCallbackForwarder* forwarder = (DTMTransformCallbackForwarder*)userP;
    return forwarder->CallBack (pts, (int)numPts);
    }

//=======================================================================================
/// <summary>
/// Callback frowarder.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private class DTMDuplicatePointsBrowsingCallbackForwarder
    {
    private:

        gcroot<DTM^>                             m_managedDtm;
        gcroot<System::Object^>                  m_args;
        gcroot<DuplicatePointsBrowsingDelegate^> m_delegate;

    public:

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMDuplicatePointsBrowsingCallbackForwarder (DTM^ dtm, DuplicatePointsBrowsingDelegate^ del)
            {
            m_managedDtm = dtm;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMDuplicatePointsBrowsingCallbackForwarder (DTM^ dtm, DuplicatePointsBrowsingDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        int CallBack
            (
            double X,
            double Y,
            DTM_DUPLICATE_POINT_ERROR *dupErrorsP,
            long numDupErrors
            )
            {
            bool ret = false;
            array<DTMDuplicatePointError^>^ errors = gcnew array<DTMDuplicatePointError^>(numDupErrors);

            for(int i = 0; i < numDupErrors; i++)
                {
                errors[i] = gcnew DTMDuplicatePointError(dupErrorsP[i]);
                }
            ret = m_delegate->Invoke (X, Y, errors, m_args);

            return (ret ? 0 :1);
            }
    };

//=======================================================================================
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
int DuplicatePointsBrowsingDelegateUnsafeCallback
(
 double X,
 double Y,
 DTM_DUPLICATE_POINT_ERROR *dupErrorsP,
 long numDupErrors,
 void      *userP
 )
    {
    DTMDuplicatePointsBrowsingCallbackForwarder* forwarder = (DTMDuplicatePointsBrowsingCallbackForwarder*)userP;
    return forwarder->CallBack(X, Y, dupErrorsP, numDupErrors);
    }


//=======================================================================================
/// <summary>
/// Callback frowarder.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private class DTMCrossingFeaturesBrowsingCallbackForwarder
    {
    private:

        gcroot<DTM^>                                    m_managedDtm;
        gcroot<System::Object^>                         m_args;
        gcroot<CrossingFeaturesBrowsingDelegate^>       m_delegate;

    public:

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMCrossingFeaturesBrowsingCallbackForwarder (DTM^ dtm, CrossingFeaturesBrowsingDelegate^ del)
            {
            m_managedDtm = dtm;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMCrossingFeaturesBrowsingCallbackForwarder (DTM^ dtm, CrossingFeaturesBrowsingDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        int CallBack
            (
            DTM_CROSSING_FEATURE_ERROR& crossError
            )
            {
            bool ret = false;
            ret = m_delegate->Invoke(gcnew DTMCrossingFeatureError(crossError), m_args);

            return (ret ? 0 :1);
            }
    };

//=======================================================================================
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
int CrossingFeaturesBrowsingDelegateUnsafeCallback
(
 DTM_CROSSING_FEATURE_ERROR& crossError,
 void      *userP
 )
    {
    DTMCrossingFeaturesBrowsingCallbackForwarder* forwarder = (DTMCrossingFeaturesBrowsingCallbackForwarder*)userP;
    return forwarder->CallBack (crossError);
    }

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

//=======================================================================================
/// <summary>
/// Callback frowarder.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private class DTMFeaturesBrowsingCallbackForwarder
    {
    private:
        gcroot<DTM^>                                    m_managedDtm;
        gcroot<System::Object^>                         m_args;
        gcroot<LinearFeaturesBrowsingDelegate^>         m_delegate;

    public:

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMFeaturesBrowsingCallbackForwarder (DTM^ dtm, LinearFeaturesBrowsingDelegate^ del)
            {
            m_managedDtm = dtm;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMFeaturesBrowsingCallbackForwarder (DTM^ dtm, LinearFeaturesBrowsingDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Daryl.Holmwood     08/2008
        //=======================================================================================
        int CacheCallback (bvector<DTMCachedFeature>& features)
            {
            for (DTMCachedFeature feat : features)
                {
                int ret = CallBack (feat.featureType, feat.featureTag, feat.featureId, feat.points.data (), feat.points.size ());
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
            size_t            nPoint
            )
            {
            if (nPoint == 0 || tPoint == nullptr)
                return DTM_SUCCESS;
            DTMFeatureType ftType;
            DTMHelpers::Copy (ftType, featureType);
            bool ret = false;
            array<BGEO::DPoint3d>^  tManagedPoint = gcnew array<BGEO::DPoint3d>((int)nPoint);
            pin_ptr<BGEO::DPoint3d> pinManagedPoint= &tManagedPoint[0];

            memcpy(pinManagedPoint, tPoint, sizeof(DPoint3d) * nPoint);
            DTMFeatureInfo^ featureInfo = gcnew DTMStandAloneFeatureInfo (DTMFeatureId (featureId), ftType, featureTag, tManagedPoint);
            ret = m_delegate->Invoke(featureInfo, m_args);

            return (ret ? 0 :1);
            }
    };

//=======================================================================================
/// <summary>
/// Callback frowarder.
/// </summary>
/// <author>Rob.Cormack</author>                              <date>06/2009</date>
//=======================================================================================
private class DTMTriangleMeshBrowsingCallbackForwarder
    {
    private:

        gcroot<DTM^>                                    m_managedDtm;
        gcroot<System::Object^>                         m_args;
        gcroot<TriangleMeshBrowsingDelegate^>           m_delegate;

    public:

        //=======================================================================================
        // @bsimethod                                               Rob.Cormack      06/2009
        //=======================================================================================
        DTMTriangleMeshBrowsingCallbackForwarder (DTM^ dtm, TriangleMeshBrowsingDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Rob.Cormack      06/2009
        //=======================================================================================
        int CallBack
            (
            ::DTMFeatureType   dtmFeatureType,
            int       numTriangles,
            int       numMeshPts,
            DPoint3d  *meshPtsP,
            int       numMeshFaces,
            long      *meshFacesP
            )
            {
            if (numMeshPts == 0 || meshPtsP == nullptr)
                return DTM_SUCCESS;
            bool ret = false;
            if ( dtmFeatureType == ::DTMFeatureType::TriangleMesh )
                {
                array<BGEO::DPoint3d>^  ManagedPoints = gcnew array<BGEO::DPoint3d>(numMeshPts);
                pin_ptr<BGEO::DPoint3d> pinManagedPoints= &ManagedPoints[0];
                memcpy(pinManagedPoints, meshPtsP, sizeof(DPoint3d) * numMeshPts);

                array<int>^  ManagedFaces = gcnew array<int>(numMeshFaces);
                pin_ptr<int> pinManagedFaces= &ManagedFaces[0];
                memcpy(pinManagedFaces,meshFacesP, sizeof(int) * numMeshFaces);
                ret = m_delegate->Invoke(ManagedPoints, ManagedFaces, m_args);
                }

            return (ret ? 0 :1);
            }
    };


//=======================================================================================
/// <summary>
/// Callback frowarder.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private class DTMContoursBrowsingCallbackForwarder
    {
    private:

        gcroot<DTM^>                                    m_managedDtm;
        gcroot<System::Object^>                         m_args;
        gcroot<ContoursBrowsingDelegate^>               m_delegate;

    public:

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMContoursBrowsingCallbackForwarder (DTM^ dtm, ContoursBrowsingDelegate^ del)
            {
            m_managedDtm = dtm;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMContoursBrowsingCallbackForwarder (DTM^ dtm, ContoursBrowsingDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            m_delegate = del;
            }

        int CacheCallback (bvector<DTMCachedFeature>& features)
            {
            for (DTMCachedFeature feat : features)
                {
                int ret = CallBack (feat.featureType, feat.featureTag, feat.featureId, feat.points.data (), feat.points.size ());
                if (ret != DTM_SUCCESS) return ret;
                }
            return DTM_SUCCESS;
            }
        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        int CallBack
            (
            ::DTMFeatureType  featureType,
            ::DTMUserTag    featureTag,
            ::DTMFeatureId  featureId,
            DPoint3dP        tPoint,
            size_t          nPoint
            )
            {
            if (nPoint == 0 || tPoint == nullptr)
                return DTM_SUCCESS;
            DTMDynamicFeatureType ftType;
            DTMHelpers::Copy (ftType, featureType);
            bool ret = false;
            // The last point of the contour line string contains the contour direction of the contour
            BGEO::DPoint3d directionPoint;
            DTMHelpers::Copy (directionPoint, tPoint [nPoint - 1]);
            array<BGEO::DPoint3d>^  tManagedPoint = gcnew array<BGEO::DPoint3d>((int)nPoint);
            pin_ptr<BGEO::DPoint3d> pinManagedPoint= &tManagedPoint[0];

            memcpy(pinManagedPoint, tPoint, sizeof(DPoint3d) * nPoint);
            ret = m_delegate->Invoke(tManagedPoint, directionPoint, m_args);
            return (ret ? 0 :1);
            }
    };

//=======================================================================================
/// <summary>
/// Points Callback Forwarder.
/// </summary>
/// <author>Rob.Cormack</author>                              <date>05/2009</date>
//=======================================================================================

private class DTMPointsCallbackForwarder
    {
    private:

        gcroot<DTM^>                                    m_managedDtm;
        gcroot<System::Object^>                         m_args;
        gcroot<PointsBrowsingDelegate^>                 m_delegate;

    public:

        //=======================================================================================
        // @bsimethod                                               Rob.Cormack      5/2009
        //=======================================================================================
        DTMPointsCallbackForwarder (DTM^ dtm, PointsBrowsingDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Rob.Cormack      5/2009
        //=======================================================================================
        int CallBack                           // Called By Core DTM
            (
            ::DTMFeatureType featureType,
            ::DTMUserTag   featureTag,
            ::DTMFeatureId featureId,
            DPoint3d  *tPoint,
            size_t     nPoint
            )
            {
            if (nPoint == 0 || tPoint == nullptr)
                return DTM_SUCCESS;
            bool ret = false;
            array<BGEO::DPoint3d>^  tManagedPoint = gcnew array<BGEO::DPoint3d>((int)nPoint);
            pin_ptr<BGEO::DPoint3d> pinManagedPoint= &tManagedPoint[0];
            memcpy(pinManagedPoint, tPoint, sizeof(DPoint3d) * nPoint);
            ret = m_delegate->Invoke(tManagedPoint, m_args);
            return (ret ? 0 :1);
            }

    };

//=======================================================================================
/// <summary>
/// Point Features Callback Forwarder.
/// </summary>
/// <author>Rob.Cormack</author>                              <date>05/2009</date>
//=======================================================================================

private class DTMPointFeaturesCallbackForwarder
    {
    private:

        gcroot<DTM^>                                    m_managedDtm;
        gcroot<System::Object^>                         m_args;
        gcroot<PointFeaturesBrowsingDelegate^>          m_delegate;

    public:

        //=======================================================================================
        // @bsimethod                                               Rob.Cormack      5/2009
        //=======================================================================================
        DTMPointFeaturesCallbackForwarder (DTM^ dtm, PointFeaturesBrowsingDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Rob.Cormack      5/2009
        //=======================================================================================
        int CallBack                           // Called By Core DTM
            (
            ::DTMFeatureType featureType,
            ::DTMUserTag   featureTag,
            ::DTMFeatureId featureId,
            DPoint3d  *tPoint,
            size_t       nPoint
            )
            {
            if (nPoint == 0 || tPoint == nullptr)
                return DTM_SUCCESS;
            bool ret = false;
            array<BGEO::DPoint3d>^  tManagedPoint = gcnew array<BGEO::DPoint3d>((int)nPoint);
            pin_ptr<BGEO::DPoint3d> pinManagedPoint= &tManagedPoint[0];
            memcpy(pinManagedPoint, tPoint, sizeof(DPoint3d) * nPoint);
            DTMFeatureInfo^ dtmFeatureInfo = gcnew DTMStandAloneFeatureInfo (DTMFeatureId (featureId), (DTMFeatureType)featureType, featureTag, tManagedPoint);
            ret = m_delegate->Invoke(dtmFeatureInfo, m_args);
            return (ret ? 0 :1);
            }

    };


//=======================================================================================
/// <summary>
/// Callback frowarder.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private class DTMTracePointCallbackForwarder
    {
    private:

        gcroot<array<BGEO::DPoint3d>^> m_tracePoints;

    public:

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        array<BGEO::DPoint3d>^ GetTracePoints ()
            {
            return m_tracePoints;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        int CallBack
            (
            ::DTMFeatureType  featureType,
            ::DTMUserTag   featureTag,
            ::DTMFeatureId featureId,
            DPoint3d  *tPoint,
            size_t     nPoint
            )
            {
            if (nPoint == 0 || tPoint == nullptr)
                return DTM_SUCCESS;

            m_tracePoints = gcnew array<BGEO::DPoint3d> ((int)nPoint);
            interior_ptr<BGEO::DPoint3d> points = &m_tracePoints[0];
            DPoint3d* endPt = &tPoint[nPoint-1];
            for (DPoint3d* pt = &tPoint[0]; pt <= endPt; ++pt, ++points)
                DTMHelpers::Copy (*points, *pt);
            return 0;
            }
    };

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     11/2007
//=======================================================================================
// Temporary function to pass onto the ManagedUnsafeDelegate to fix problem with __cdecl and __stdcall.

int DynamicFeaturesBrowsingCallbackForwarderDelegate
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


int DynamicFeaturesBrowsingCacheCallbackForwarderDelegate (bvector<DTMCachedFeature>& features, void* userP)
    {
    DTMDynamicFeaturesBrowsingCallbackForwarder* forwarder = (DTMDynamicFeaturesBrowsingCallbackForwarder*)userP;
    return forwarder->CacheCallback (features);
    }

int FeaturesBrowsingCallbackForwarderDelegate
(
 ::DTMFeatureType   featureType,
 ::DTMUserTag   featureTag,
 ::DTMFeatureId featureId,
 DPoint3d  *tPoint,
 size_t       nPoint,
 void      *userP
 )
    {
    DTMFeaturesBrowsingCallbackForwarder* forwarder = (DTMFeaturesBrowsingCallbackForwarder*)userP;
    return forwarder->CallBack(featureType, featureTag, featureId, tPoint, nPoint);
    }

//=======================================================================================
/// <summary>
/// Call Back Method Called By Core DTM For Points Browsing
/// As Points As Cached By The Core There Is No Requirement To Use Cache For Managed API
/// </summary>
/// <author>Rob.Cormack</author>                              <date>05/2009</date>
//=======================================================================================
int PointsBrowsingCallbackForwarderDelegate
(
 ::DTMFeatureType featureType,
 ::DTMUserTag   featureTag,
 ::DTMFeatureId featureId,
 DPoint3d  *tPoint,
 size_t     nPoint,
 void      *userP
 )
    {
    DTMPointsCallbackForwarder* forwarder = (DTMPointsCallbackForwarder*)userP;
    return forwarder->CallBack(featureType, featureTag, featureId, tPoint, nPoint);
    }
//=======================================================================================
/// <summary>
/// Call Back Method Called By Core DTM For Points Browsing
/// As Points As Cached By The Core There Is No Requirement To Use Cache For Managed API
/// </summary>
/// <author>Rob.Cormack</author>                              <date>05/2009</date>
//=======================================================================================
int PointFeaturesBrowsingCallbackForwarderDelegate
(
 ::DTMFeatureType   featureType,
 ::DTMUserTag   featureTag,
 ::DTMFeatureId featureId,
 DPoint3d  *tPoint,
 size_t     nPoint,
 void      *userP
 )
    {
    DTMPointFeaturesCallbackForwarder* forwarder = (DTMPointFeaturesCallbackForwarder*)userP;
    return forwarder->CallBack(featureType, featureTag, featureId, tPoint, nPoint);
    }

//=======================================================================================
/// <summary>
/// Call Back Method Called By Core DTM For Triangle Mesh Browsing
/// As Triangles Are Cached By The Core There Is No Requirement To Use Cache For Managed API
/// </summary>
/// <author>Rob.Cormack</author>                              <date>06/2009</date>
//=======================================================================================

int TriangleMeshBrowsingCallbackForwarderDelegate
(
 ::DTMFeatureType   featureType,
 int       numTriangles,
 int       numMeshPoints,
 DPoint3d  *meshPointsP,
 int       numMeshFaces,
 long *meshFacesP,
 void      *userP
 )
    {
    DTMTriangleMeshBrowsingCallbackForwarder* forwarder = (DTMTriangleMeshBrowsingCallbackForwarder*)userP;
    return forwarder->CallBack(featureType,numTriangles,numMeshPoints,meshPointsP,numMeshFaces,meshFacesP);
    }

int FeaturesBrowsingCacheCallbackForwarderDelegate (bvector<DTMCachedFeature>& features, void* userP)
    {
    DTMFeaturesBrowsingCallbackForwarder* forwarder = (DTMFeaturesBrowsingCallbackForwarder*)userP;
    return forwarder->CacheCallback (features);
    }

int ContoursBrowsingCallbackForwarderDelegate
(
 ::DTMFeatureType featureType,
 ::DTMUserTag   featureTag,
 ::DTMFeatureId featureId,
 DPoint3d  *tPoint,
 size_t nPoint,
 void      *userP
 )
    {
    DTMContoursBrowsingCallbackForwarder* forwarder = (DTMContoursBrowsingCallbackForwarder*)userP;
    return forwarder->CallBack(featureType, featureTag, featureId, tPoint, nPoint);
    }

int ContoursBrowsingCacheCallbackForwarderDelegate (bvector<DTMCachedFeature>& features, void* userP)
    {
    DTMContoursBrowsingCallbackForwarder* forwarder = (DTMContoursBrowsingCallbackForwarder*)userP;
    return forwarder->CacheCallback (features);
    }

int TraceBrowsingCallbackForwarderDelegate
(
 ::DTMFeatureType featureType,
 ::DTMUserTag   featureTag,
 ::DTMFeatureId featureId,
 DPoint3d       *tPoint,
 size_t         nPoint,
 void           *userP
 )
    {
    DTMTracePointCallbackForwarder* forwarder = (DTMTracePointCallbackForwarder*)userP;
    return forwarder->CallBack(featureType, featureTag, featureId, tPoint, nPoint);
    }



//=======================================================================================
/// <summary>
/// Callback frowarder.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private class DTMSinglePointFeaturesBrowsingCallbackForwarder
    {
    private:

        gcroot<DTM^>                                    m_managedDtm;
        gcroot<System::Object^>                         m_args;
        gcroot<SinglePointFeaturesBrowsingDelegate^>    m_delegate;

    public:

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMSinglePointFeaturesBrowsingCallbackForwarder (DTM^ dtm, SinglePointFeaturesBrowsingDelegate^ del)
            {
            m_managedDtm = dtm;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMSinglePointFeaturesBrowsingCallbackForwarder (DTM^ dtm, SinglePointFeaturesBrowsingDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        int CallBack
            (
            ::DTMFeatureType featureType,
            DPoint3d&  point
            )
            {
            DTMDynamicFeatureType ftType;
            DTMHelpers::Copy (ftType, featureType);
            BGEO::DPoint3d convPoint;
            DTMHelpers::Copy (convPoint, point);
            bool ret = m_delegate->Invoke(ftType, convPoint, m_args);
            return (ret ? 0: 1);
            }

    };

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     11/2007
//=======================================================================================
// Temporary function to pass onto the ManagedUnsafeDelegate to fix problem with __cdecl and __stdcall.
int SinglePointFeatureBrowsingDelegateUnsafeCallback
(
 ::DTMFeatureType  featureType,
 DPoint3d&  point,
 void      *userP
 )
    {
    DTMSinglePointFeaturesBrowsingCallbackForwarder* forwarder = (DTMSinglePointFeaturesBrowsingCallbackForwarder*)userP;
    return forwarder->CallBack(featureType, point);
    }
//=======================================================================================
/// <summary>
/// Callback frowarder.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>11/2007</date>
//=======================================================================================
private class DTMSlopeIndicatorCallbackForwarder
    {
    private:

        gcroot<DTM^>                                    m_managedDtm;
        gcroot<System::Object^>                         m_args;
        gcroot<SlopeIndicatorsBrowsingDelegate^>        m_delegate;

    public:

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMSlopeIndicatorCallbackForwarder (DTM^ dtm, SlopeIndicatorsBrowsingDelegate^ del)
            {
            m_managedDtm = dtm;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        DTMSlopeIndicatorCallbackForwarder (DTM^ dtm, SlopeIndicatorsBrowsingDelegate^ del, System::Object^ args)
            {
            m_managedDtm = dtm;
            m_args = args;
            m_delegate = del;
            }

        //=======================================================================================
        // @bsimethod                                               Sylvain.Pucci      11/2007
        //=======================================================================================
        int CallBack (bool major, DPoint3d& startPointP, DPoint3d& endPointP)
            {
            BGEO::DPoint3d startPoint;
            BGEO::DPoint3d endPoint;
            DTMHelpers::Copy (startPoint, startPointP);
            DTMHelpers::Copy (endPoint, endPointP);
            bool ret = m_delegate->Invoke(major, startPoint, endPoint, m_args);
            return (ret ? 0 : 1);
            }
    };
//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     11/2007
//=======================================================================================
// Temporary function to pass onto the ManagedUnsafeDelegate to fix problem with __cdecl and __stdcall.
int SlopeIndicatorDelegateUnsafeCallback
(
 bool major,
 DPoint3d&  startPointP,
 DPoint3d&  endPointP,
 void      *userP
 )
    {
    DTMSlopeIndicatorCallbackForwarder* forwarder =(DTMSlopeIndicatorCallbackForwarder*)userP;
    return forwarder->CallBack(major, startPointP, endPointP);
    }


//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      2/2008
//=======================================================================================
void BrowsingCriteria::SetFenceToBlock(BGEO::DRange3d value)
    {
    array<BGEO::DPoint3d>^ aFencePt = gcnew array<BGEO::DPoint3d>(5);

    aFencePt[0].X = value.Low.X  ;
    aFencePt[0].Y = value.Low.Y  ;
    aFencePt[0].Z = 0.0          ;
    aFencePt[1].X = value.High.X ;
    aFencePt[1].Y = value.Low.Y  ;
    aFencePt[1].Z = 0.0          ;
    aFencePt[2].X = value.High.X ;
    aFencePt[2].Y = value.High.Y ;
    aFencePt[2].Z = 0.0          ;
    aFencePt[3].X = value.Low.X  ;
    aFencePt[3].Y = value.High.Y ;
    aFencePt[3].Z = 0.0          ;
    aFencePt[4] = aFencePt[0]    ;

    FencePoints = aFencePt;
    FenceOption = DTMFenceOption::Overlap;
    FenceType   = DTMFenceType::Block;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     5/2010
//=======================================================================================
BcDTMP DTM::Handle::get ()
    {
    if (m_nativeDtm == nullptr)
        {
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("This doesn't support this method"));
        }
    return m_nativeDtm;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM^ DTM::CreateFromFile (String ^fileName)
    {
    if (fileName == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("fileName"));

    pin_ptr<const wchar_t> ch = PtrToStringChars(fileName);

    BcDTMPtr bcDtmP;
    if ( ch )
        bcDtmP = BcDTM::CreateFromTinFile (ch);

    if ( bcDtmP.IsNull() ) DTMException::CheckForErrorStatus (1);

    DTM^ dtm = bcDtmP.IsValid() ? gcnew DTM( bcDtmP.get() ) : nullptr;
    return dtm;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     3/2010
//=======================================================================================
DTM^ DTM::CreateFromStream (System::IO::Stream^ stream)
    {
    if (stream == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("stream"));

    if (!stream->CanSeek)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("Stream needs to be able to seek."));

    BcDTMPtr bcDtmP;
    ManagedBcDTMStream dtmStream(stream);

    bcDtmP = BcDTM::CreateFromStream( dtmStream );

    if ( bcDtmP.IsNull() ) DTMException::CheckForErrorStatus (1);

    DTM^ dtm = bcDtmP.IsValid() ? gcnew DTM( bcDtmP.get() ) : nullptr;
    return dtm;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM^ DTM::CreateFromGeopakTinFile (String ^fileName)
    {
    if (fileName == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("fileName"));
    pin_ptr<const wchar_t> ch = PtrToStringChars(fileName);

    BcDTMPtr bcDtmP;

    if ( ch )
        bcDtmP = BcDTM::CreateFromTinFile( ch ) ;

    if ( bcDtmP.IsNull() ) DTMException::CheckForErrorStatus (1);

    DTM^ dtm = bcDtmP.IsValid() ? gcnew DTM( bcDtmP.get() ) : nullptr;
    return dtm;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM^ DTM::CreateFromGeopakDatFile (String ^fileName)
    {
    if (fileName == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("fileName"));
    pin_ptr<const wchar_t> ch = PtrToStringChars(fileName);

    BcDTMPtr bcDtmP;

    if ( ch )
        bcDtmP = BcDTM::CreateFromGeopakDatFile( ch );

    if ( bcDtmP.IsNull() ) DTMException::CheckForErrorStatus (1);

    DTM^ dtm = bcDtmP.IsValid() ? gcnew DTM( bcDtmP.get() ) : nullptr;
    return dtm;
    }


//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM^ DTM::CreateFromXyzFile  (String ^fileName)
    {

    if (fileName == nullptr)  throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("fileName"));

    pin_ptr<const wchar_t> ch = PtrToStringChars(fileName);

    BcDTMPtr bcDtmP = nullptr;

    if ( ch )
        bcDtmP = BcDTM::CreateFromXyzFile ( ch );

    if ( bcDtmP.IsNull() ) DTMException::CheckForErrorStatus (1);

    DTM^ dtm = bcDtmP.IsValid() ? gcnew DTM( bcDtmP.get() ) : nullptr;
    return dtm;
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack  5/2009
//=======================================================================================
DTM^ DTM::CreateFromMemoryBlock (array<char>^ memoryBlock )
    {

    if ( memoryBlock == nullptr)  throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("memoryBlock"));

    pin_ptr<char const> pBlock = &memoryBlock[0];
    int blockSize = memoryBlock->Length ;

    BcDTMPtr bcDtmP;

    bcDtmP = BcDTM::CreateFromMemoryBlock( ( char *) pBlock , blockSize ) ;

    if ( bcDtmP.IsNull() ) DTMException::CheckForErrorStatus (1);

    DTM^ dtm = bcDtmP.IsValid() ? gcnew DTM( bcDtmP.get() ) : nullptr;
    return dtm;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood    5/2009
//=======================================================================================
DTM^ DTM::CreateFromNativeMemoryBlock (System::IntPtr memoryBlock, unsigned long blockSize )
    {
    char* pBlock = (char*)memoryBlock.ToPointer();

    if ( pBlock == nullptr)  throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("memoryBlock"));

    BcDTMPtr bcDtmP;

    bcDtmP = BcDTM::CreateFromMemoryBlock( ( char *) pBlock , blockSize ) ;

    if ( bcDtmP.IsNull() ) DTMException::CheckForErrorStatus (1);

    DTM^ dtm = bcDtmP.IsValid() ? gcnew DTM( bcDtmP.get() ) : nullptr;
    return dtm;
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack  5/2009
//=======================================================================================
array<char>^ DTM::CopyToMemoryBlock()
    {

    char *blockP=nullptr ;
    unsigned long blockSize=0 ;

    DTMException::CheckForErrorStatus (Handle->CopyToMemoryBlock (&blockP, &blockSize));

    if ( blockP != nullptr )
        {
        array<char>^ memoryBlock  = gcnew array<char>(blockSize) ;
        pin_ptr<char const> pBlock = &memoryBlock[0];
        memcpy((void*)pBlock,blockP,blockSize*sizeof(char)) ;
        delete [] blockP ;
        return memoryBlock ;
        }
    else return nullptr ;
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2009
//=======================================================================================
array<BGEO::DPoint3d>^  DTM::ImportDPoint3DArrayFromXYZFile (String ^fileName)
    {
    if (fileName == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("fileName"));

    pin_ptr<const wchar_t> ch = PtrToStringChars(fileName);

    DPoint3d*  pointsP=nullptr ;
    long        numPoints=0 ;

    DTMException::CheckForErrorStatus (bcdtmRead_xyzFileToPointArray (ch,&pointsP,&numPoints));

    if ( pointsP )
        {
        array<BGEO::DPoint3d>^ pointArray  = gcnew array<BGEO::DPoint3d>(numPoints) ;
        pin_ptr<BGEO::DPoint3d const> pPoints = &pointArray[0];
        memcpy((void*)pPoints,pointsP,numPoints*sizeof(DPoint3d)) ;
        delete [] pointsP ;
        return(pointArray) ;
        }
    else return(nullptr) ;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM^ DTM::FromHandle (IntPtr handle)
    {
    if (handle.ToPointer() == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("handle"));
    return gcnew DTM((BcDTMP)handle.ToPointer());
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      7/2009
//=======================================================================================
DTM^ DTM::FromNativeDtmHandle (IntPtr handle)
    {
    if (handle.ToPointer() == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("handle"));
    BcDTMPtr bcDtmP = BcDTM::CreateFromDtmHandle(*(BC_DTM_OBJ*)handle.ToPointer()) ;
    DTM^ dtm = bcDtmP.IsValid() ? gcnew DTM( bcDtmP.get() ) : nullptr;
    return dtm;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM::DTM (BcDTMP dtm)
    {
    m_marshaller = ReleaseMarshaller::GetMarshaller();
#ifdef DEBUG
    m_stackTrace = (gcnew System::Diagnostics::StackTrace(true))->ToString();
#endif
    m_autoUpdateMemoryPressure = true;
    m_memoryUsed = 0;
    if (dtm == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("Native DTM"));

    // No need to add a reference here because the dtm that is passed in always has one.
    m_nativeDtm = dtm;
    m_nativeDtm->AddRef();

    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM::DTM (int iniPoint, int incPoint)
    {
    m_marshaller = ReleaseMarshaller::GetMarshaller();
#ifdef DEBUG
    m_stackTrace = (gcnew System::Diagnostics::StackTrace(true))->ToString();
#endif
    // No need to add a reference here because the dtm that is passed in always has one.
    m_autoUpdateMemoryPressure = true;
    m_memoryUsed = 0;
    BcDTMPtr dtm = BcDTM::Create (iniPoint, incPoint);
    m_nativeDtm = dtm.get();
    m_nativeDtm->AddRef();
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM::DTM ()
    {
    m_marshaller = ReleaseMarshaller::GetMarshaller();
#ifdef DEBUG
    m_stackTrace = (gcnew System::Diagnostics::StackTrace(true))->ToString();
#endif
    // No need to add a reference here because the dtm that is passed in always has one.
    m_memoryUsed = 0;
    m_autoUpdateMemoryPressure = true;
    BcDTMPtr dtm = BcDTM::Create ();
    m_nativeDtm = dtm.get();
    m_nativeDtm->AddRef();
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM::~DTM ()
    {
    InternalDispose(true);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM::!DTM ()
    {
#ifdef DEBUG
//    System::Diagnostics::Debug::Fail ("Failed to call Dispose " + m_stackTrace);
#endif
    InternalDispose (false);
    }

void DTM::InternalDispose(bool disposing)
    {
    if (m_memoryUsed)
        {
        System::GC::RemoveMemoryPressure(m_memoryUsed);
        m_memoryUsed = 0;
        }
    try
        {
        if (m_nativeDtm != nullptr)
            {
            if (disposing)
                Handle->Release();
            else
                m_marshaller->QueueEntry(Handle);
            }
        }
    catch(...)
        {
        }
    m_nativeDtm = nullptr;

    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     2/2008
//=======================================================================================
bool DTM::SetTriangulationParameters (double pointTol, double lineTol, DTMEdgeOption edgeOption, double maxSide)
    {
    DTMException::CheckForErrorStatus (Handle->SetTriangulationParameters (pointTol, lineTol,(long)edgeOption, maxSide));
    return true ;
    }

//=======================================================================================
// @bsimethod                                               James.Goode     10/2011
//=======================================================================================
void DTM::GetTriangulationParameters ([Out] double% pointTol, [Out] double% lineTol, [Out] DTMEdgeOption%  edgeOption, [Out] double% maxSide)
    {
    double pointTol2;
    double lineTol2;
    long edgeOption2;
    double maxSide2;

    DTMException::CheckForErrorStatus (Handle->GetTriangulationParameters (pointTol2, lineTol2, edgeOption2, maxSide2));

    pointTol = pointTol2;
    lineTol = lineTol2;
    edgeOption = (DTMEdgeOption) edgeOption2;
    maxSide = maxSide2;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
TriangulationReport DTM::Triangulate ()
    {
    TriangulationReport ret;
    ret.Success = true;

    // Set the autoCleanOptions
    Handle->SetCleanUpOptions (DTMCleanupFlags::All);

    DTMException::CheckForErrorStatus (Handle->Triangulate());

    CheckMemoryPressure();
    //ret.Success = false ;
    return ret ;
    }


//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     2/2008
//=======================================================================================
bool DTM::CheckTriangulation ()
    {
    return Handle->CheckTriangulation() == DTM_SUCCESS;
    }


//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTM::Save (String^ fileName)
    {
    if (fileName == nullptr)
        {
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("FileName"));
        }
    pin_ptr<const wchar_t> p = PtrToStringChars (fileName);

    DTMException::CheckForErrorStatus (Handle->Save (p));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     3/2010
//=======================================================================================
void DTM::SaveToStream (System::IO::Stream^ stream)
    {
    if (stream == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("stream"));

    if (!stream->CanSeek)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("Stream needs to be able to seek."));

    ManagedBcDTMStream dtmStream(stream);

    DTMException::CheckForErrorStatus (Handle->SaveToStream (dtmStream));
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      8/2008
//=======================================================================================
void DTM::PopulateFromGeopakTinFile(String^ fileName)
    {
    if (fileName == nullptr)
        {
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("FileName"));
        }    // Conversion to wchar_t* :
    pin_ptr<const wchar_t> p = PtrToStringChars(fileName);

    DTMException::CheckForErrorStatus (Handle->PopulateFromGeopakTinFile (p));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      8/2008
//=======================================================================================
void DTM::SaveAsGeopakTinFile (String^ fileName)
    {
    if (fileName == nullptr)
        {
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("FileName"));
        }
    pin_ptr<const wchar_t> ch = PtrToStringChars(fileName);

    DTMException::CheckForErrorStatus (Handle->SaveAsGeopakTinFile (ch));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
BGEO::DRange3d DTM::Range3d::get ()
    {
    ::DRange3d range;
    DTMException::CheckForErrorStatus (Handle->GetRange(range));
    CheckMemoryPressure();

    BGEO::DRange3d rangeT;

    pin_ptr <BGEO::DRange3d> pinned = (BGEO::DRange3d*)&rangeT;
    *(::DRange3d*)pinned = range;
    return rangeT;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     4/2011
//=======================================================================================
DTMFeatureStatisticsInfo^ DTM::CalculateFeatureStatistics ()
    {
    ::DTMFeatureStatisticsInfo info;
    DTMException::CheckForErrorStatus (Handle->CalculateFeatureStatistics (info));
    DTMFeatureStatisticsInfo^ retInfo = gcnew DTMFeatureStatisticsInfo();
    retInfo->VerticesCount = (long) info.numPoints;
    retInfo->TrianglesCount = info.numTriangles;
    retInfo->TrianglesLinesCount = info.numTinLines;
    retInfo->FeaturesCount = info.numDtmFeatures;
    retInfo->BreakLinesCount = info.numBreaks;
    retInfo->ContourLinesCount = info.numContourLines;
    retInfo->VoidsCount = info.numVoids;
    retInfo->IslandsCount = info.numIslands;
    retInfo->HolesCount = info.numHoles;
    retInfo->PointFeaturesCount = info.numGroupSpots;
    retInfo->HasHull = info.hasHull;
    return retInfo;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     5/2013
//=======================================================================================
System::Int64 DTM::VerticesCount::get ()
    {
    return Handle->GetPointCount ();
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     5/2013
//=======================================================================================
int DTM::TrianglesCount::get ()
    {
    return Handle->GetTrianglesCount ();
    }
//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
array<BGEO::DPoint3d>^ DTM::GetBoundary()
    {
    DTMPointArray boundary;
    int       status = Handle->GetBoundary(boundary);
    DTMException::CheckForErrorStatus (status);
    if (status != SUCCESS) return nullptr;

    if (boundary.size() == 0) return nullptr;

    array<BGEO::DPoint3d>^ lineStrPoint = gcnew array<BGEO::DPoint3d> ((int)boundary.size ());
    pin_ptr<BGEO::DPoint3d> pinManagedPoint= &lineStrPoint[0];
    memcpy (pinManagedPoint, boundary.data (), sizeof(DPoint3d) * boundary.size());
    return lineStrPoint;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMDrapedPoint^ DTM::DrapePoint (BGEO::DPoint3d point)
    {
    double elevation;
    double slope;
    double aspect;
    DPoint3d triangle[3];
    int drapedType;
    DPoint3d pt;

    CheckIsTriangulated();
    DTMHelpers::Copy (pt, point);
    DTMException::CheckForErrorStatus (Handle->DrapePoint (&elevation, &slope, &aspect, triangle, drapedType, pt));

    array<BGEO::DPoint3d>^ tr = gcnew array<BGEO::DPoint3d>(3);
    DTMHelpers::Copy (tr[0], triangle[0]);
    DTMHelpers::Copy (tr[1], triangle[1]);
    DTMHelpers::Copy (tr[2], triangle[2]);
    BGEO::DPoint3d p = BGEO::DPoint3d(pt.x, pt.y, elevation);

    return gcnew DTMDrapedPoint(elevation, slope, aspect, tr, drapedType, p);
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      7/2011
//=======================================================================================
DTMDrapedLinearElement^ DTM::DrapeLinearPoints (array<BGEO::DPoint3d>^ points)
    {
    if(points == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("linearPoints"));

    CheckIsTriangulated();

    BcDTMDrapedLinePtr pDrapedLine;
    pin_ptr<BGEO::DPoint3d> uPoints = &points[0];
    DTMException::CheckForErrorStatus(Handle->DrapeLinearPoints (pDrapedLine, (DPoint3d*)uPoints, nullptr, points->Length));

    DTMDrapedLinearElement^ ret = gcnew DTMDrapedLinearElement (pDrapedLine.get());
    return ret;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      7/2011
//=======================================================================================
DTMDrapedLinearElement^ DTM::DrapeLinearPoints (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points)
    {
    if(points == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("linearPoints"));

    bvector<DPoint3d> pts;
    GetVectorPointsFromIEnumerable (pts, points);
    int nbPt = (int)pts.size();

    CheckIsTriangulated();
    BcDTMDrapedLinePtr pDrapedLine;
    DTMException::CheckForErrorStatus (Handle->DrapeLinearPoints (pDrapedLine, (DPoint3d*)&pts[0], nullptr, nbPt));

    DTMDrapedLinearElement^ ret = gcnew DTMDrapedLinearElement (pDrapedLine.get());
    return ret;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeature^ DTM::GetFeatureById (DTMFeatureId featureId)
    {
    BcDTMFeaturePtr pDTMFeature;
    ::DTMFeatureId id = featureId.Id;
    DTMException::CheckForErrorStatus (Handle->GetFeatureById (pDTMFeature, id));

    if (pDTMFeature.IsNull())
        return nullptr;
    return DTMHelpers::GetAsSpecificFeature (pDTMFeature.get());
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
SlopeAreaResult^ DTM::CalculateSlopeArea ()
    {
    double area;
    CheckIsTriangulated();
    DTMException::CheckForErrorStatus (Handle->CalculateSlopeArea (area));
    return gcnew SlopeAreaResult(area);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
SlopeAreaResult^ DTM::CalculateSlopeArea (array<BGEO::DPoint3d>^ polygon)
    {
    if (polygon == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("polygon"));
    double area;
    pin_ptr<BGEO::DPoint3d const> pLocPoly = &polygon[0];
    int nbPt = polygon->Length;

    CheckIsTriangulated();
    DTMException::CheckForErrorStatus (Handle->CalculateSlopeArea (area, (DPoint3d*)pLocPoly, nbPt));

    return gcnew SlopeAreaResult(area);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
SlopeAreaResult^ DTM::CalculateSlopeArea (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ polygon)
    {
    if (polygon == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("polygon"));
    double area;

    bvector<DPoint3d> pts;
    GetVectorPointsFromIEnumerable (pts, polygon);
    int nbPt = (int)pts.size();

    CheckIsTriangulated();
    DTMException::CheckForErrorStatus (Handle->CalculateSlopeArea (area, &pts[0], nbPt));

    return gcnew SlopeAreaResult(area);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTM::Merge (DTM^ dtm)
    {
    if (dtm == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("dtm"));
    CheckIsTriangulated();
    DTMException::CheckForErrorStatus (Handle->Merge (*dtm->Handle));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTM::MergeAdjacent (DTM^ dtm)
    {
    if (dtm == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("dtm"));
    CheckIsTriangulated();

    DTMException::CheckForErrorStatus (Handle->MergeAdjacent (*dtm->Handle));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM^ DTM::CloneAndClip(array<BGEO::DPoint3d>^ polygon, DTMClippingMethod clipMethod)
    {
    if (polygon == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("Polygon"));

    int size = polygon->Length;
    BcDTMPtr clippedDtm;
    pin_ptr<BGEO::DPoint3d const> pointString = &polygon[0];
    DTMException::CheckForErrorStatus (Handle->ClipByPointString (clippedDtm, (DPoint3d*)pointString, size, (DTMClipOption)clipMethod));
    CheckMemoryPressure();
    if (clippedDtm.IsValid())
        return gcnew DTM (clippedDtm.get());
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM^ DTM::CloneAndClip(System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ polygon, DTMClippingMethod clipMethod)
    {
    if (polygon == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("Polygon"));

    bvector<DPoint3d> pts;
    GetVectorPointsFromIEnumerable (pts, polygon);
    int size = (int)pts.size();
    BcDTMPtr clippedDtm;
    DTMException::CheckForErrorStatus (Handle->ClipByPointString (clippedDtm, &pts[0], size, (DTMClipOption)clipMethod));
    CheckMemoryPressure();
    if (clippedDtm.IsValid())
        return gcnew DTM (clippedDtm.get());
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
double DTM::CalculatePlanarPrismoidalVolume (double elevation, VolumeCriteria^ criteria)
    {
    CheckIsTriangulated();
    pin_ptr<BGEO::DPoint3d const> pointString = nullptr;
    int size;
    if (criteria != nullptr && criteria->FencePoints != nullptr)
        {
        pointString = &criteria->FencePoints[0];
        size = criteria->FencePoints->Length;
        }
    bvector<VOLRANGETAB> ranges;
    int numRanges = 0;
    if (criteria && criteria->RangeTable)
        {
        numRanges = criteria->RangeTable->Length;
        ranges.resize(numRanges);
        for(int i = 0; i < numRanges; i++)
            {
            ranges[i].Low = criteria->RangeTable[i]->Low;
            ranges[i].High= criteria->RangeTable[i]->High;
            }
        }
    BcDTMVolumeAreaResult result;
    DTMException::CheckForErrorStatus (Handle->ComputePlanarPrismoidalVolume (result, elevation, (DPoint3d*)pointString, size, ranges.empty() ? nullptr : ranges.data(), numRanges));
    CheckMemoryPressure();
    if (!ranges.empty())
        {
        for(int i = 0; i < numRanges; i++)
            criteria->RangeTable[i]->SetCutFillValues (ranges[i].Cut, ranges[i].Fill);
        }
    return ( result.fillVolume - result.cutVolume );
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
CutFillResult^ DTM::CalculateCutFillVolume (DTM^ dtm, VolumeCriteria^ criteria)
    {
    if (dtm == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("dtm"));

    CheckIsTriangulated();
    pin_ptr<BGEO::DPoint3d const> pointString = nullptr;
    int size = 0;
    if (criteria != nullptr && criteria->FencePoints != nullptr)
        {
        pointString = &criteria->FencePoints[0];
        size = criteria->FencePoints->Length;
        }
    bvector<VOLRANGETAB> ranges;
    int numRanges = 0;
    if (criteria && criteria->RangeTable)
        {
        numRanges = criteria->RangeTable->Length;
        ranges.resize(numRanges);
        for(int i = 0; i < numRanges; i++)
            {
            ranges[i].Low  = criteria->RangeTable[i]->Low;
            ranges[i].High = criteria->RangeTable[i]->High;
            }
        }
    BcDTMVolumeAreaResult result;
    DTMException::CheckForErrorStatus (Handle->CalculateCutFillVolume (result, *dtm->Handle, (DPoint3d*)pointString, size, ranges.empty() ? nullptr : ranges.data(), numRanges));
    CheckMemoryPressure();
    if (!ranges.empty())
        {
        for(int i = 0; i < numRanges; i++)
            criteria->RangeTable[i]->SetCutFillValues(ranges[i].Cut, ranges[i].Fill);
        }
    return gcnew CutFillResult (result);
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2009
//=======================================================================================
VolumeResult^ DTM::CalculatePrismoidalVolumeToElevation(double elevation, VolumeCriteria^ criteria)
    {
    CheckIsTriangulated();
    pin_ptr<BGEO::DPoint3d const> pointString = nullptr;
    int size=0;
    if (criteria != nullptr && criteria->VolumePolygon != nullptr)
        {
        pointString = &criteria->VolumePolygon[0];
        size = criteria->VolumePolygon->Length;
        }
    bvector<VOLRANGETAB> ranges;
    int numRanges = 0;
    if (criteria && criteria->RangeTable)
        {
        numRanges = criteria->RangeTable->Length;
        ranges.resize(numRanges);
        for(int i = 0; i < numRanges; i++)
            {
            ranges[i].Low = criteria->RangeTable[i]->Low;
            ranges[i].High= criteria->RangeTable[i]->High;
            }
        }
    DtmVectorString volumePolygons ;
    BcDTMVolumeAreaResult result;
    DTMException::CheckForErrorStatus (Handle->CalculatePrismoidalVolumeToElevation (result, &volumePolygons, elevation, (DPoint3d*)pointString, size, ranges.empty() ? nullptr : ranges.data(), numRanges));
    CheckMemoryPressure();
    if (!ranges.empty())
        {
        for(int i = 0; i < numRanges; i++)
            criteria->RangeTable[i]->SetCutFillValues(ranges[i].Cut, ranges[i].Fill);
        }
    return gcnew VolumeResult(result, volumePolygons);
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2009
//=======================================================================================
VolumeResult^ DTM::CalculatePrismoidalVolumeToSurface(DTM^ dtm, VolumeCriteria^ criteria)
    {
    if (dtm == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("dtm"));

    CheckIsTriangulated();
    dtm->CheckIsTriangulated() ;
    pin_ptr<BGEO::DPoint3d const> pointString = nullptr;
    int size=0;
    if (criteria != nullptr && criteria->VolumePolygon != nullptr)
        {
        pointString = &criteria->VolumePolygon[0];
        size = criteria->VolumePolygon->Length;
        }
    bvector<VOLRANGETAB> ranges;
    int numRanges = 0;
    if (criteria && criteria->RangeTable)
        {
        numRanges = criteria->RangeTable->Length;
        ranges.resize(numRanges);
        for(int i = 0; i < numRanges; i++)
            {
            ranges[i].Low = criteria->RangeTable[i]->Low;
            ranges[i].High= criteria->RangeTable[i]->High;
            }
        }
    DtmVectorString volumePolygons ;
    BcDTMVolumeAreaResult result;
    DTMException::CheckForErrorStatus (Handle->CalculatePrismoidalVolumeToSurface (result, &volumePolygons, *dtm->Handle, (DPoint3d*)pointString, size, ranges.empty() ? nullptr : ranges.data(), numRanges));
    CheckMemoryPressure();
    if (!ranges.empty())
        {
        for(int i = 0; i < numRanges; i++)
            criteria->RangeTable[i]->SetCutFillValues(ranges[i].Cut, ranges[i].Fill);
        }
    return gcnew VolumeResult (result, volumePolygons);
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2009
//=======================================================================================
VolumeResult^ DTM::CalculatePrismoidalVolumeBalanceToSurface(DTM^ dtm, VolumeCriteria^ criteria)
    {
    if (dtm == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("dtm"));

    CheckIsTriangulated();
    dtm->CheckIsTriangulated() ;
    double fromArea;
    double toArea;
    double balanceVolume;
    pin_ptr<BGEO::DPoint3d const> pointString = nullptr;
    int size=0;
    if (criteria != nullptr && criteria->VolumePolygon != nullptr)
        {
        pointString = &criteria->VolumePolygon[0];
        size = criteria->VolumePolygon->Length;
        }
    DtmVectorString volumePolygons ;
    DTMException::CheckForErrorStatus (Handle->CalculatePrismoidalVolumeBalanceToSurface (fromArea, toArea, balanceVolume, &volumePolygons, *dtm->Handle, (DPoint3d*)pointString, size));
    CheckMemoryPressure();
    return gcnew VolumeResult(fromArea,toArea,balanceVolume,volumePolygons);
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2009
//=======================================================================================
VolumeResult^ DTM::CalculateGridVolumeToElevation(double elevation,int numGridPoints,VolumeCriteria^ criteria)
    {
    CheckIsTriangulated();
    pin_ptr<BGEO::DPoint3d const> pointString = nullptr;
    int size=0;
    if (criteria != nullptr && criteria->VolumePolygon != nullptr)
        {
        pointString = &criteria->VolumePolygon[0];
        size = criteria->VolumePolygon->Length;
        }
    bvector<VOLRANGETAB> ranges;
    int numRanges = 0;
    if (criteria && criteria->RangeTable)
        {
        numRanges = criteria->RangeTable->Length;
        ranges.resize(numRanges);
        for(int i = 0; i < numRanges; i++)
            {
            ranges[i].Low = criteria->RangeTable[i]->Low;
            ranges[i].High= criteria->RangeTable[i]->High;
            }
        }
    DtmVectorString volumePolygons ;
    BcDTMVolumeAreaResult result;
    long numCellsUsed;
    double cellArea;
    DTMException::CheckForErrorStatus (Handle->CalculateGridVolumeToElevation (result, numCellsUsed, cellArea, &volumePolygons, numGridPoints, elevation, (DPoint3d*)pointString, size, ranges.empty() ? nullptr : ranges.data(), numRanges));
    CheckMemoryPressure();
    if (!ranges.empty())
        {
        for(int i = 0; i < numRanges; i++)
            criteria->RangeTable[i]->SetCutFillValues(ranges[i].Cut, ranges[i].Fill);
        }
    return gcnew VolumeResult(result,numCellsUsed,cellArea,volumePolygons);
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2009
//=======================================================================================
VolumeResult^ DTM::CalculateGridVolumeToSurface(DTM^ dtm,int numGridPoints,VolumeCriteria^ criteria)
    {
    if (dtm == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("dtm"));
    CheckIsTriangulated();
    dtm->CheckIsTriangulated() ;

    CheckIsTriangulated();
    pin_ptr<BGEO::DPoint3d const> pointString = nullptr;
    int size=0;
    if (criteria != nullptr && criteria->VolumePolygon != nullptr)
        {
        pointString = &criteria->VolumePolygon[0];
        size = criteria->VolumePolygon->Length;
        }
    bvector<VOLRANGETAB> ranges;
    int numRanges = 0;
    if (criteria && criteria->RangeTable)
        {
        numRanges = criteria->RangeTable->Length;
        ranges.resize(numRanges);
        for(int i = 0; i < numRanges; i++)
            {
            ranges[i].Low = criteria->RangeTable[i]->Low;
            ranges[i].High= criteria->RangeTable[i]->High;
            }
        }
    DtmVectorString volumePolygons ;
    BcDTMVolumeAreaResult result;
    long numCellsUsed;
    double cellArea;
    DTMException::CheckForErrorStatus (Handle->CalculateGridVolumeToSurface (result, numCellsUsed, cellArea, &volumePolygons, *dtm->Handle, numGridPoints, (DPoint3d*)pointString, size, ranges.empty() ? nullptr : ranges.data(), numRanges));
    CheckMemoryPressure();
    if (!ranges.empty())
        {
        for(int i = 0; i < numRanges; i++)
            criteria->RangeTable[i]->SetCutFillValues(ranges[i].Cut, ranges[i].Fill);
        }
    return gcnew VolumeResult(result,numCellsUsed,cellArea,volumePolygons);
    }


//=======================================================================================
// @bsimethod                                               Rob.Cormack       5/2008
//=======================================================================================
void DTM::Append (DTM^ dtm)
    {
    DTMException::CheckForErrorStatus (Handle->Append (*dtm->Handle));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM^ DTM::Clone ()
    {
    BcDTMPtr dtm = Handle->Clone();
    if (dtm.IsValid())
        return gcnew DTM (dtm.get());
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM^ DTM::CloneNotDeep ()
    {
    return gcnew DTM (Handle);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTM^ DTM::Delta (DTM^ dtm)
    {
    if (dtm == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("dtm"));
    CheckIsTriangulated();
    dtm->CheckIsTriangulated();
    CheckMemoryPressure();

    BcDTMPtr bcDtm = Handle->Delta (*dtm->Handle, nullptr, 0);
    if (bcDtm.IsValid())
        return gcnew DTM (bcDtm.get());
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     4/2008
//=======================================================================================
DTM^ DTM::Delta (array<BGEO::DPoint3d>^ points, DTM^ dtm)
    {
    if (dtm == nullptr)
        throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException ("dtm"));
    CheckIsTriangulated();
    dtm->CheckIsTriangulated();

    if (points == nullptr)
        throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException ("points"));

    pin_ptr<BGEO::DPoint3d const> tPoint = &points[0];
    int size = points->Length;

    BcDTMPtr bcDtm = Handle->Delta (*dtm->Handle, (DPoint3d*)tPoint, size);
    if (bcDtm.IsValid())
        return gcnew DTM (bcDtm.get());
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     4/2008
//=======================================================================================
DTM^ DTM::Delta (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points, DTM^ dtm)
    {
    if (dtm == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("dtm"));
    CheckIsTriangulated();
    dtm->CheckIsTriangulated();

    if (points == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    bvector<DPoint3d> pts;
    GetVectorPointsFromIEnumerable (pts, points);
    int size = (int)pts.size();

    BcDTMPtr bcDtm = Handle->Delta (*dtm->Handle, &pts[0], size);
    if (bcDtm.IsValid())
        return gcnew DTM (bcDtm.get());
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      10/2011
//=======================================================================================
DTM^ DTM::DeltaElevation (double elevation)
    {

    CheckIsTriangulated();
    CheckMemoryPressure();

    BcDTMPtr bcDtm = Handle->DeltaElevation(elevation, nullptr, 0);
    if (bcDtm.IsValid())
        return gcnew DTM (bcDtm.get());
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack     10/2011
//=======================================================================================
DTM^ DTM::DeltaElevation (array<BGEO::DPoint3d>^ points, double elevation)
    {

    CheckIsTriangulated();
    CheckMemoryPressure();

    if (points == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    pin_ptr<BGEO::DPoint3d const> tPoint = &points[0];
    int size = points->Length;

    BcDTMPtr bcDtm = Handle->DeltaElevation(elevation, (DPoint3d*)tPoint, size);
    if (bcDtm.IsValid())
        return gcnew DTM (bcDtm.get());
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTM::OffsetDeltaElevation(double elevation)
    {
    DTMException::CheckForErrorStatus (Handle->OffsetDeltaElevation (elevation));
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::BrowseContours (ContoursBrowsingCriteria^ criteria, ContoursBrowsingDelegate^ hdl, System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    if (hdl == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdl"));

    CheckIsTriangulated();

    DTMContoursBrowsingCallbackForwarder forwarder(this, hdl, oArg);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    double smoothingFactor = criteria->SplineSmoothingFactor ;
    if ( criteria->SmoothingOption ==  DTMContourSmoothingMethod::Vertex ) smoothingFactor = criteria->LinearSmoothingFactor;

    pin_ptr<double const> contourValues=nullptr;
    int numContourValues = 0;
    if (criteria && criteria->ContourValues)
        {
        numContourValues = criteria->ContourValues->Length;
        contourValues = &criteria->ContourValues[0];
        }


    //    DTMException::CheckForErrorStatus (Handle->browseContours(criteria->Interval, criteria->BaseElevation, criteria->ZLow, criteria->ZHigh, !criteria->UseOnlyContourValues,
    //              (DTMContourSmoothing)criteria->SmoothingOption,smoothingFactor,(int)criteria->SplineDensification,
    //              (DPoint3d*)tPoint, fenceSize, (int)criteria->FenceOption, (int)criteria->FenceType, (double*)contourValues, numContourValues,
    //              &forwarder, &ContoursBrowsingCallbackForwarderDelegate));

    long        maxSlopeOption=0;
    double      maxSlopeValue = criteria->MaxSlopeValue ;
    if ( criteria->LimitContoursToSlope   == true ) maxSlopeOption = 1 ;
    DTMFeatureCache cache (Handle, criteria->CacheSize, &ContoursBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseContours (criteria->Interval, criteria->BaseElevation, criteria->ZLow, criteria->ZHigh, !criteria->UseOnlyContourValues,
        (::DTMContourSmoothing)criteria->SmoothingOption,smoothingFactor,(int)criteria->SplineDensification,
        Bentley::TerrainModel::DTMFenceParams ((::DTMFenceType)criteria->FenceType, (::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint, fenceSize), (double*)contourValues, numContourValues,
        maxSlopeOption, maxSlopeValue, criteria->ShowDepressionContours));
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      7/2010
//=======================================================================================
void  DTM::ContourAtPoint (ContoursBrowsingCriteria^ criteria, ContoursBrowsingDelegate^ hdl,double X,double Y, System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    if (hdl == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdl"));

    CheckIsTriangulated();

    DTMContoursBrowsingCallbackForwarder forwarder(this, hdl, oArg);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    double smoothingFactor = criteria->SplineSmoothingFactor ;
    if ( criteria->SmoothingOption ==  DTMContourSmoothingMethod::Vertex ) smoothingFactor = criteria->LinearSmoothingFactor;


    DTMException::CheckForErrorStatus (Handle->ContourAtPoint(X,Y,criteria->Interval,
        (::DTMContourSmoothing)criteria->SmoothingOption,smoothingFactor,(int)criteria->SplineDensification,
        Bentley::TerrainModel::DTMFenceParams ((::DTMFenceType)criteria->FenceType, (::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint, fenceSize),
        &forwarder, &ContoursBrowsingCallbackForwarderDelegate));

    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void DTM::BrowseDynamicFeatures (DynamicFeaturesBrowsingCriteria^ criteria, DTMDynamicFeatureType featureType, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    CheckIsTriangulated();

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    ::DTMFeatureType featType;
    DTMHelpers::Copy (featType, featureType);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria && criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }
    DTMFeatureCache cache (Handle, criteria ? criteria->CacheSize : 1000, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseFeatures (featType, Bentley::TerrainModel::DTMFenceParams (criteria ? (::DTMFenceType)criteria->FenceType : ::DTMFenceType::None, criteria ? (::DTMFenceOption)criteria->FenceOption : ::DTMFenceOption::None, (DPoint3d*)tPoint, fenceSize), criteria ? criteria->MaxSpots : 10000));
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void DTM::BrowseDynamicFeatures (BrowsingCriteria^ criteria, DTMDynamicFeatureType featureType, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {

    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    CheckIsTriangulated();

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    ::DTMFeatureType featType;
    DTMHelpers::Copy (featType, featureType);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria && criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    DTMFeatureCache cache (Handle, criteria ? criteria->CacheSize : 1000, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseFeatures (featType, Bentley::TerrainModel::DTMFenceParams (criteria ? (::DTMFenceType)criteria->FenceType : ::DTMFenceType::None, criteria ? (::DTMFenceOption)criteria->FenceOption : ::DTMFenceOption::None, (DPoint3d*)tPoint, fenceSize), 10000));
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack     6/2009
//=======================================================================================
void  DTM::BrowseTriangleMesh (TriangleMeshBrowsingCriteria^ criteria, TriangleMeshBrowsingDelegate^ hdl, System::Object^ oArg)
    {

    if (criteria == nullptr) throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException ("criteria"));
    if (hdl      == nullptr) throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException ("hdl"));

    CheckIsTriangulated();

    DTMTriangleMeshBrowsingCallbackForwarder forwarder (this, hdl, oArg);

    pin_ptr<BGEO::DPoint3d const> fencePts = nullptr;
    int fenceSize = 0;
    if (criteria && criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        fencePts = &criteria->FencePoints[0];
        }
    DTMException::CheckForErrorStatus (Handle->BrowseTriangleMesh (criteria->MaxTriangles,Bentley::TerrainModel::DTMFenceParams (criteria ? (::DTMFenceType)criteria->FenceType : ::DTMFenceType::None, criteria ? (::DTMFenceOption)criteria->FenceOption : ::DTMFenceOption::None, (DPoint3d*)fencePts,fenceSize), &forwarder,&TriangleMeshBrowsingCallbackForwarderDelegate)) ;
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      5/2009
//=======================================================================================
void DTM::BrowsePoints (PointsBrowsingCriteria^ criteria, PointsBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMPointsCallbackForwarder forwarder(this, hdlP, oArg);

    ::DTMFeatureType featType=::DTMFeatureType::Spots ;

    long pointSelection = (long ) criteria->SelectionOption ;
    if ( pointSelection == 1  ) featType = ::DTMFeatureType::RandomSpots ;
    if ( pointSelection == 2  ) featType = ::DTMFeatureType::FeatureSpot ;
    if ( pointSelection == 3  ) featType = ::DTMFeatureType::Spots ;

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize=0;
    if (criteria && criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    DTMException::CheckForErrorStatus (Handle->BrowseFeatures(featType, Bentley::TerrainModel::DTMFenceParams((::DTMFenceType)criteria->FenceType,(::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint,fenceSize),criteria->MaxPoints,&forwarder,&PointsBrowsingCallbackForwarderDelegate));
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      5/2009
//=======================================================================================
void DTM::BrowsePointFeatures (PointFeaturesBrowsingCriteria^ criteria, PointFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException("hdlP"));

    DTMPointFeaturesCallbackForwarder forwarder (this, hdlP, oArg);

    ::DTMFeatureType featType=::DTMFeatureType::GroupSpots ;

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize=0;
    if (criteria && criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    DTMException::CheckForErrorStatus (Handle->BrowseFeatures (featType,Bentley::TerrainModel::DTMFenceParams((::DTMFenceType)criteria->FenceType,(::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint,fenceSize),criteria->MaxPoints,&forwarder,&PointFeaturesBrowsingCallbackForwarderDelegate));
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      2/2008
//=======================================================================================
void DTM::BrowseLinearFeatures (LinearFeaturesBrowsingCriteria^ criteria, DTMFeatureType featureType, LinearFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    ::DTMFeatureType featType;
    DTMHelpers::Copy (featType, featureType);

    pin_ptr<BGEO::DPoint3d const> tPoint = nullptr;
    int fenceSize;
    if (criteria && criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    DTMFeatureCache cache (Handle, criteria ? criteria->CacheSize : 1000, &FeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseFeatures(featType, Bentley::TerrainModel::DTMFenceParams (criteria ? (::DTMFenceType)criteria->FenceType : ::DTMFenceType::None, criteria ? (::DTMFenceOption)criteria->FenceOption : ::DTMFenceOption::None, (DPoint3d*)tPoint, fenceSize), criteria ? criteria->MaxSpots : 10000));
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2011
//=======================================================================================
void  DTM::CalculateCatchments (CatchmentsCalculationCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    CheckIsTriangulated();

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria && criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    DTMException::CheckForErrorStatus (Handle->CalculateCatchments (criteria->RefineOption, criteria->FalseLowDepth, criteria ? ::DTMFenceParams (::DTMFenceType::Block, (::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint, fenceSize) : ::DTMFenceParams (), &forwarder, &DynamicFeaturesBrowsingCallbackForwarderDelegate));
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::BrowseCatchments (CatchmentsBrowsingCriteria^ criteria, double minDepth, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    CheckIsTriangulated();
    ::DTMFeatureType featType;
    DTMHelpers::Copy (featType, DTMDynamicFeatureType::Catchment);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria && criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    DTMFeatureCache cache (Handle, criteria ? criteria->CacheSize : 1000, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseDrainageFeatures (featType, &minDepth, criteria ? ::DTMFenceParams (::DTMFenceType::Block, (::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint, fenceSize) : ::DTMFenceParams ()));
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::BrowseLowPoints (LowPointsBrowsingCriteria^ criteria, double minDepth, SinglePointFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMSinglePointFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    CheckIsTriangulated();
    ::DTMFeatureType featType;
    long nPoint = 0;
    DTMHelpers::Copy (featType, DTMDynamicFeatureType::LowPoint);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria && criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }
    DTMException::CheckForErrorStatus (Handle->BrowseSinglePointFeatures (featType, &minDepth, Bentley::TerrainModel::DTMFenceParams (criteria ? (::DTMFenceType)criteria->FenceType : ::DTMFenceType::None, criteria ? (::DTMFenceOption)criteria->FenceOption : ::DTMFenceOption::None, (DPoint3d*)tPoint, fenceSize), &nPoint,
        &forwarder, &SinglePointFeatureBrowsingDelegateUnsafeCallback));
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::BrowseHighPoints (HighPointsBrowsingCriteria^ criteria,  SinglePointFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMSinglePointFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    CheckIsTriangulated();

    ::DTMFeatureType featType;
    long nPoint = 0;
    DTMHelpers::Copy (featType, DTMDynamicFeatureType::HighPoint);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria && criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    DTMException::CheckForErrorStatus (Handle->BrowseSinglePointFeatures (featType, nullptr, Bentley::TerrainModel::DTMFenceParams (criteria ? (::DTMFenceType)criteria->FenceType : ::DTMFenceType::None, criteria ? (::DTMFenceOption)criteria->FenceOption : ::DTMFenceOption::None, (DPoint3d*)tPoint, fenceSize), &nPoint,
        &forwarder, &SinglePointFeatureBrowsingDelegateUnsafeCallback));
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::BrowseRidgeLines (RidgeLinesBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (criteria == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    ::DTMFeatureType featType;
    DTMHelpers::Copy (featType, DTMDynamicFeatureType::RidgeLine);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize=0;
    if (criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    CheckIsTriangulated();
    DTMFeatureCache cache (Handle, criteria ? criteria->CacheSize : 1000, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseDrainageFeatures (featType, nullptr, criteria ? ::DTMFenceParams (::DTMFenceType::Block, (::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint, fenceSize) : ::DTMFenceParams ()));

    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::BrowseSumpLines (SumpLinesBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP,System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    ::DTMFeatureType featType;
    DTMHelpers::Copy (featType, DTMDynamicFeatureType::SumpLine);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    CheckIsTriangulated();

    DTMFeatureCache cache (Handle, criteria ? criteria->CacheSize : 1000, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseDrainageFeatures (featType, nullptr, criteria ? ::DTMFenceParams (::DTMFenceType::Block, (::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint, fenceSize) : ::DTMFenceParams ()));
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      1/2011
//=======================================================================================
void  DTM::BrowsePonds( DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException ("hdlP"));
    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder (this, hdlP, oArg);
    CheckIsTriangulated();
    DTMException::CheckForErrorStatus (Handle->BrowsePonds (&forwarder,&DynamicFeaturesBrowsingCallbackForwarderDelegate));
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::TracePath (BGEO::DPoint3d startPoint, double slope, double distance, PathTracingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException ("criteria"));

    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException ("hdlP"));

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder (this, hdlP, oArg);

    CheckIsTriangulated();
    DTMException::CheckForErrorStatus (Handle->TracePath (startPoint.X, startPoint.Y, slope, distance, &forwarder, &DynamicFeaturesBrowsingCallbackForwarderDelegate));
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::TraceLine (BGEO::DPoint3d startPoint, double slope, double distance, LineTracingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException ("criteria"));

    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException ("hdlP"));

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder (this, hdlP, oArg);

    CheckIsTriangulated();
    DTMException::CheckForErrorStatus (Handle->TraceLine (startPoint.X, startPoint.Y, slope, distance, &forwarder, &DynamicFeaturesBrowsingCallbackForwarderDelegate));
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void DTM::AnalyzeElevation (ElevationAnalyzingBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    CheckIsTriangulated();
    DRange1d* targetP = nullptr;
    int  nInterval = DTMHelpers::Copy (&targetP, criteria->DoubleRange);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    DTMFeatureCache cache (Handle, criteria ? criteria->CacheSize : 1000, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.AnalyzeElevation (targetP, nInterval, criteria->PolygonizedResult, Bentley::TerrainModel::DTMFenceParams ((::DTMFenceType)criteria->FenceType, (::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint, fenceSize)));

    bcMem_free (targetP);
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::AnalyzeSlope (SlopeAnalyzingBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    DRange1d* targetP = nullptr;
    int  nInterval = DTMHelpers::Copy (&targetP, criteria->DoubleRange);

    pin_ptr<BGEO::DPoint3d const> tPoint=nullptr;
    int fenceSize;
    if (criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    CheckIsTriangulated();
    DTMFeatureCache cache (Handle, criteria ? criteria->CacheSize : 1000, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.AnalyzeSlope(targetP, nInterval, criteria->PolygonizedResult, Bentley::TerrainModel::DTMFenceParams ((::DTMFenceType)criteria->FenceType, (::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint, fenceSize)));

    bcMem_free (targetP);
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::AnalyzeAspect (AspectAnalyzingBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    DRange1d* targetP = nullptr;
    int  nInterval = DTMHelpers::Copy (&targetP, criteria->DoubleRange);

    pin_ptr<BGEO::DPoint3d const> tPoint = nullptr;
    int fenceSize;
    if (criteria->FencePoints)
        {
        fenceSize = criteria->FencePoints->Length;
        tPoint = &criteria->FencePoints[0];
        }

    CheckIsTriangulated();
    DTMFeatureCache cache (Handle, criteria ? criteria->CacheSize : 1000, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.AnalyzeAspect (targetP, nInterval, criteria->PolygonizedResult, Bentley::TerrainModel::DTMFenceParams ((::DTMFenceType)criteria->FenceType, (::DTMFenceOption)criteria->FenceOption, (DPoint3d*)tPoint, fenceSize)));

    bcMem_free (targetP);
    }


//=======================================================================================
// @bsimethod                                               Rob.Cormack      7/2011
//=======================================================================================
void DTM::TraceMaximumDescent( double minLowPointDepth,BGEO::DPoint3d refPoint,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
{

    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    CheckIsTriangulated();

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    DTMException::CheckForErrorStatus (Handle->GetDescentTrace (minLowPointDepth, refPoint.X, refPoint.Y, &forwarder, &DynamicFeaturesBrowsingCallbackForwarderDelegate));


}
//=======================================================================================
// @bsimethod                                               Rob.Cormack      7/2011
//=======================================================================================
void DTM::TraceMaximumAscent( double minHighPointElevation,BGEO::DPoint3d refPoint,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
{

    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    CheckIsTriangulated();

    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    DTMException::CheckForErrorStatus (Handle->GetAscentTrace (minHighPointElevation, refPoint.X, refPoint.Y,
        &forwarder, &DynamicFeaturesBrowsingCallbackForwarderDelegate));
}
//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::BrowseSlopeIndicators (DTM^ targetDtm, SlopeIndicatorsBrowsingCriteria^ criteria, SlopeIndicatorsBrowsingDelegate^ hdlp, System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    if (hdlp == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlp"));

    CheckIsTriangulated();

    DTMSlopeIndicatorCallbackForwarder forwarder(this, hdlp, oArg);

    DTMException::CheckForErrorStatus (Handle->BrowseSlopeIndicator (*targetDtm->Handle, criteria->MajorInterval, criteria->MinorInterval,
        &forwarder, &SlopeIndicatorDelegateUnsafeCallback));
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::BrowseTriangles (TrianglesBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    CheckIsTriangulated();
    this->BrowseDynamicFeatures (criteria, DTMDynamicFeatureType::Triangle, hdlP, oArg);
    }

//=======================================================================================
// @bsimethod                                               Claude.Bernard      5/2007
//=======================================================================================
void  DTM::BrowseTriangleEdges (TriangleEdgesBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    CheckIsTriangulated();
    this->BrowseDynamicFeatures (criteria, DTMDynamicFeatureType::TriangleEdge, hdlP, oArg);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
Mesh^ DTM::GetMesh (bool firstCall, int maxMeshSize)
    {
    CheckIsTriangulated();

    BcDTMMeshPtr unmanagedMesh = Handle->GetMesh(firstCall, maxMeshSize, nullptr, 0);

    if (!unmanagedMesh.IsValid())
        return nullptr;

    Mesh^ mesh = gcnew Mesh(unmanagedMesh.get());

    return mesh;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTM::Transform(BGEO::DTransform3d transform)
    {
    ::Transform nativeTransform;
    pin_ptr<BGEO::DTransform3d> pTransform = &transform;

    nativeTransform = *(::Transform*)pTransform;
    DTMException::CheckForErrorStatus (Handle->Transform(nativeTransform));
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      5/2011
//=======================================================================================
void DTM::TransformPointsUsingCallback (DTMTransformCallbackDelegate^ hdl, System::Object^ oArg)
    {
    DTMTransformCallbackForwarder forwarder (this, hdl, oArg);
    DTMException::CheckForErrorStatus (Handle->TransformUsingCallback (&TransformPointsUsingCallbackForwarderDelegate, &forwarder));
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      11/2009
//=======================================================================================
void DTM::ConvertUnits (double xyFactor,double zFactor)
    {
    DTMException::CheckForErrorStatus (Handle->ConvertUnits(xyFactor,zFactor));
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      2/2008
//=======================================================================================
void DTM::AddPoint (BGEO::DPoint3d rPoint)
    {
    DPoint3d  point;
    DTMHelpers::Copy (point,rPoint);
    DTMException::CheckForErrorStatus (Handle->AddPoint (point));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      2/2008
//=======================================================================================//
void DTM::AddPoints (array<BGEO::DPoint3d>^ points)
    {
    if (points == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    pin_ptr<BGEO::DPoint3d const> ptsP = &points[0];
    int numPts = points->Length;
    DTMException::CheckForErrorStatus (Handle->AddPoints ((DPoint3d*)ptsP, numPts));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      2/2008
//=======================================================================================//
void DTM::AddPoints (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points)
    {
    if (points == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    bvector<DPoint3d> pts;
    GetVectorPointsFromIEnumerable (pts, points);
    int numPts = (int)pts.size();
    DTMException::CheckForErrorStatus (Handle->AddPoints (&pts[0], numPts));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTM::AddPointFeature (BGEO::DPoint3d point, ::DTMUserTag userTag)
    {
    DPoint3d pt;
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    DTMHelpers::Copy(pt,point);
    DTMException::CheckForErrorStatus (Handle->AddPointFeature (pt, userTag, &featureID));
    CheckMemoryPressure();
    return DTMFeatureId (featureID);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTM::AddPointFeature(BGEO::DPoint3d point)
    {
    DPoint3d pt;
    ::DTMFeatureId featureID = DTM_NULL_FEATURE_ID;
    DTMHelpers::Copy(pt, point);
    DTMException::CheckForErrorStatus (Handle->AddPointFeature (pt, &featureID));
    CheckMemoryPressure();
    return DTMFeatureId (featureID);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTM::AddPointFeature (array<BGEO::DPoint3d>^ points)
    {
    if (points == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    pin_ptr<BGEO::DPoint3d const> tPoint = &points[0];
    int size = points->Length;
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    ::DTMUserTag   userTag = DTM_NULL_USER_TAG ;
    DTMException::CheckForErrorStatus (Handle->AddPointFeature ((DPoint3d*)tPoint, size, userTag, &featureID));
    CheckMemoryPressure();
    return DTMFeatureId (featureID);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTM::AddPointFeature (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points)
    {
    if (points == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    bvector<DPoint3d> pts;
    GetVectorPointsFromIEnumerable (pts, points);
    int size = (int)pts.size();
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    ::DTMUserTag   userTag = DTM_NULL_USER_TAG ;
    DTMException::CheckForErrorStatus (Handle->AddPointFeature (&pts[0], size, userTag, &featureID));
    CheckMemoryPressure();
    return DTMFeatureId (featureID);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTM::AddPointFeature (array<BGEO::DPoint3d>^ points, ::DTMUserTag userTag)
    {
    if (points == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    pin_ptr<BGEO::DPoint3d const> tPoint = &points[0];
    int size = points->Length;
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    DTMException::CheckForErrorStatus (Handle->AddPointFeature ((DPoint3d*)tPoint, size, userTag, &featureID));
    CheckMemoryPressure();
    return DTMFeatureId (featureID);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTM::AddPointFeature (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points, ::DTMUserTag userTag)
    {
    if (points == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    bvector<DPoint3d> pts;
    GetVectorPointsFromIEnumerable (pts, points);
    int size = (int)pts.size();
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    DTMException::CheckForErrorStatus (Handle->AddPointFeature (&pts[0], size, userTag, &featureID));
    CheckMemoryPressure();
    return DTMFeatureId (featureID);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTM::AddLinearFeature (array<BGEO::DPoint3d>^ points, DTMFeatureType featureType, ::DTMUserTag userTag)
    {
    if (points == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));
    pin_ptr<BGEO::DPoint3d const> tPoint = &points[0];
    int size = points->Length;
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    ::DTMFeatureType ftType;

    DTMHelpers::Copy (ftType, featureType);
    DTMException::CheckForErrorStatus (Handle->AddLinearFeature(ftType, (DPoint3d*)tPoint, size, userTag, &featureID));
    CheckMemoryPressure();
    return DTMFeatureId (featureID);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTM::AddLinearFeature (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points, DTMFeatureType featureType, ::DTMUserTag userTag)
    {
    if (points == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    bvector<DPoint3d> pts;
    GetVectorPointsFromIEnumerable (pts, points);
    int size = (int)pts.size();
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    ::DTMFeatureType ftType;

    DTMHelpers::Copy (ftType, featureType);
    DTMException::CheckForErrorStatus (Handle->AddLinearFeature(ftType, &pts[0], size, userTag, &featureID));
    CheckMemoryPressure();
    return DTMFeatureId (featureID);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTM::AddLinearFeature (array<BGEO::DPoint3d>^ points, DTMFeatureType featureType)
    {
    if (points == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    ::DTMUserTag userTag = DTM_NULL_USER_TAG;
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    ::DTMFeatureType ftType;
    DTMHelpers::Copy (ftType, featureType);
    pin_ptr<BGEO::DPoint3d const> tPoint = &points[0];
    int size = points->Length;
    DTMException::CheckForErrorStatus (Handle->AddLinearFeature (ftType, (DPoint3d*)tPoint, size, userTag, &featureID));
    CheckMemoryPressure();
    return DTMFeatureId (featureID);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTM::AddLinearFeature (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points, DTMFeatureType featureType)
    {
    if (points == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("points"));

    ::DTMUserTag userTag = DTM_NULL_USER_TAG;
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    ::DTMFeatureType ftType;
    DTMHelpers::Copy (ftType, featureType);

    bvector<DPoint3d> pts;
    GetVectorPointsFromIEnumerable (pts, points);
    int size = (int)pts.size();;
    DTMException::CheckForErrorStatus (Handle->AddLinearFeature (ftType, &pts[0], size, userTag, &featureID));
    CheckMemoryPressure();
    return DTMFeatureId (featureID);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTM::DeleteFeatureById (DTMFeatureId id)
{
    DTMException::CheckForErrorStatus (Handle->DeleteFeatureById (id.Id));
    CheckMemoryPressure();
}

//=======================================================================================
// @bsimethod                                               Rob.Cormack       8/2011
//=======================================================================================
void DTM::BulkDeleteFeaturesByFeatureId(array<DTMFeatureId>^ featureIds)
    {
    int numFeatureIds = featureIds->Length;
    if (numFeatureIds > 0)
        {
        bvector<::DTMFeatureId> featureIdsP;
        featureIdsP.resize(numFeatureIds);
        for (int n = 0; n < numFeatureIds; ++n)
            featureIdsP[n] = featureIds[n].Id;

        DTMException::CheckForErrorStatus(Handle->BulkDeleteFeaturesByFeatureId(featureIdsP.data(), numFeatureIds));
        CheckMemoryPressure();
        }
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack       8/2011
//=======================================================================================
void DTM::DeleteFeaturesByUserTag(::DTMUserTag userTag)
{
    DTMException::CheckForErrorStatus (Handle->DeleteFeaturesByUserTag (userTag));
    CheckMemoryPressure();
}

//=======================================================================================
// @bsimethod                                               Rob.Cormack       8/2011
//=======================================================================================
void DTM::BulkDeleteFeaturesByUserTag(array<::DTMUserTag>^ userTag)
{
    int numUserTags=userTag->Length ;
    if( numUserTags > 0 )
    {
        pin_ptr<::DTMUserTag const> userTagsP = &userTag[0];
        DTMException::CheckForErrorStatus (Handle->BulkDeleteFeaturesByUserTag((::DTMUserTag *)userTagsP,numUserTags));
        CheckMemoryPressure();
    }
}

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTM::DeleteFeaturesByType (DTMFeatureType featureType)
    {
    ::DTMFeatureType ftType;
    DTMHelpers::Copy (ftType, featureType);
    DTMException::CheckForErrorStatus (Handle->DeleteFeaturesByType(ftType));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTM::JoinFeatures (DTMFeatureType featureType, double tolerance)
    {
    ::DTMFeatureType ftType;
    DTMHelpers::Copy (ftType, featureType);
    DTMException::CheckForErrorStatus (Handle->JoinFeatures(ftType, nullptr, nullptr, tolerance));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTM::RemoveHull ()
    {
    DTMException::CheckForErrorStatus (Handle->RemoveHull());
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2010
//=======================================================================================
void DTM::RemoveNoneFeatureHullLines ()
    {
    DTMException::CheckForErrorStatus (Handle->RemoveNoneFeatureHullLines());
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      2/2008
//=======================================================================================
void DTM::CheckIsTriangulated()
    {
    if (Handle->GetDTMState() != DTMState::Tin)
        Triangulate();
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      2/2008
//=======================================================================================
void DTM::BrowseDuplicatePoints (DuplicatePointsBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMDuplicatePointsBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    DTMException::CheckForErrorStatus (Handle->BrowseDuplicatePoints (&forwarder, &DuplicatePointsBrowsingDelegateUnsafeCallback));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      2/2008
//=======================================================================================
void DTM::BrowseCrossingFeatures (CrossingFeaturesBrowsingCriteria^ criteria, CrossingFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMCrossingFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    int featureListSize = criteria ? criteria->FeatureTypes ? criteria->FeatureTypes->Length : 0 : 0;
    ::DTMFeatureType* dtmFeatureList = (::DTMFeatureType*)alloca(sizeof(::DTMFeatureType) * featureListSize);

    for(int i = 0; i < featureListSize; i++)
        {
        DTMHelpers::Copy (dtmFeatureList[i], criteria->FeatureTypes[i]);
        }

    DTMException::CheckForErrorStatus (Handle->BrowseCrossingFeatures (dtmFeatureList, featureListSize, &forwarder, &CrossingFeaturesBrowsingDelegateUnsafeCallback));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      3/2008
//=======================================================================================
FilterResult^ DTM::TinFilterPoints(TinFilterCriteria^ criteria)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    long numPointsBefore;
    long numPointsAfter;
    double filterReduction;

    DTMException::CheckForErrorStatus (Handle->TinFilterPoints ((int)criteria->Option, (int)criteria->ReinsertOption, criteria->ZTolerance, &numPointsBefore, &numPointsAfter, &filterReduction));
    CheckMemoryPressure();

    return gcnew FilterResult(numPointsBefore, numPointsAfter, filterReduction);
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      3/2008
//=======================================================================================
FilterResult^ DTM::TileFilterPoints(TileFilterCriteria^ criteria)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    long numPointsBefore;
    long numPointsAfter;
    double filterReduction;
    double tileLength = criteria->TileLength;

    // tileLength hasn't been set so set it to the default.
    if (tileLength == 0)
        {
        BGEO::DRange3d range = Range3d;
        tileLength = range.High.X - range.Low.X;
        if (tileLength < range.High.Y - range.Low.Y)
            tileLength = range.High.Y - range.Low.Y;
        tileLength /= 10.0;
        }

    DTMException::CheckForErrorStatus (Handle->TileFilterPoints (criteria->MaxTilePoints, criteria->MaxTileDivide, tileLength, criteria->ZTolerance, &numPointsBefore, &numPointsAfter, &filterReduction));
    CheckMemoryPressure();

    return gcnew FilterResult(numPointsBefore, numPointsAfter, filterReduction);
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack            3/2008
//=======================================================================================
FilterResult^ DTM::TinFilterSinglePointPointFeatures(TinFilterCriteria^ criteria)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    long numPointsBefore;
    long numPointsAfter;
    double filterReduction;

    DTMException::CheckForErrorStatus (Handle->TinFilterSinglePointPointFeatures ((int)criteria->Option, (int)criteria->ReinsertOption, criteria->ZTolerance, &numPointsBefore, &numPointsAfter, &filterReduction));
    CheckMemoryPressure();

    return gcnew FilterResult(numPointsBefore, numPointsAfter, filterReduction);
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      3/2008
//=======================================================================================
FilterResult^ DTM::TileFilterSinglePointPointFeatures(TileFilterCriteria^ criteria)
    {
    if (criteria == nullptr)
        throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("criteria"));

    long numPointsBefore;
    long numPointsAfter;
    double filterReduction;
    double tileLength = criteria->TileLength;

    // tileLength hasn't been set so set it to the default.
    if (tileLength == 0)
        {
        BGEO::DRange3d range = Range3d;
        tileLength = range.High.X - range.Low.X;
        if (tileLength < range.High.Y - range.Low.Y)
            tileLength = range.High.Y - range.Low.Y;
        tileLength /= 10.0;
        }

    DTMException::CheckForErrorStatus (Handle->TileFilterSinglePointPointFeatures (criteria->MaxTilePoints, criteria->MaxTileDivide, tileLength, criteria->ZTolerance, &numPointsBefore, &numPointsAfter, &filterReduction));
    CheckMemoryPressure();

    return gcnew FilterResult(numPointsBefore, numPointsAfter, filterReduction);
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      5/2008
//=======================================================================================
void DTM::CheckMemoryPressure()
    {
    if (!m_autoUpdateMemoryPressure)
        return;

    size_t memoryUsed;
    Handle->GetMemoryUsed (memoryUsed);

    if (m_memoryUsed != memoryUsed)
        {
        if (m_memoryUsed)
            System::GC::RemoveMemoryPressure(m_memoryUsed);
        m_memoryUsed = memoryUsed;
        if (memoryUsed)
            System::GC::AddMemoryPressure(memoryUsed);
        }
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      6/2008
//=======================================================================================
void DTM::PurgeDTM(DTMPurgeFlags flags)
    {
    DTMException::CheckForErrorStatus (Handle->PurgeDTM((unsigned int)flags));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood      6/2008
//=======================================================================================
BGEO::DPoint3d DTM::GetPoint(long index)
    {
    DPoint3d point;
    DTMException::CheckForErrorStatus (Handle->GetPoint(index, point));
    BGEO::DPoint3d mPoint;
    DTMHelpers::Copy (mPoint, point);
    return mPoint;
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack     05/08/2009
//=======================================================================================
void DTM::Clip (array<BGEO::DPoint3d>^ polygon, DTMClippingMethod clipMethod)
    {
    if (polygon == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("Polygon"));

    int size = polygon->Length;
    pin_ptr<BGEO::DPoint3d const> pointString = &polygon[0];
    DTMException::CheckForErrorStatus (Handle->Clip ((DPoint3d*)pointString, size, (DTMClipOption)clipMethod));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack     05/08/2009
//=======================================================================================
void DTM::Clip (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ polygon, DTMClippingMethod clipMethod)
    {
    if (polygon == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("Polygon"));

    bvector<DPoint3d> pts;
    GetVectorPointsFromIEnumerable (pts, polygon);
    int size = (int)pts.size();
    DTMException::CheckForErrorStatus (Handle->Clip (&pts[0], size, (DTMClipOption)clipMethod));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      7/2009
//=======================================================================================
DtmState DTM::GetDtmState()
    {
    DtmState dtmStateClass = (DtmState)Handle->GetDTMState ();
    return dtmStateClass ;
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      05/08/2009
//=======================================================================================
void DTM::ConvertToDataState()
    {
    DTMException::CheckForErrorStatus (Handle->ConvertToDataState());
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      05/08/2009
//=======================================================================================
void DTM::SetMemoryAllocationParameters
(
 int initialPointSize,
 int incrementalPointSize
 )
    {
    DTMException::CheckForErrorStatus (Handle->SetPointMemoryAllocationParameters(initialPointSize,incrementalPointSize));
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      1/2010
//=======================================================================================
InterpolationResult^ DTM::InterpolateDtmFeatureType(DTMFeatureType dtmFeatureType,double snapTolerance,DTM ^pointsDtm, DTM ^interpolatedDtm)
    {
    if (pointsDtm == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("pointsDtm"));
    if (interpolatedDtm == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("interpolatedDtm"));

    long numDtmFeatures=0,numDtmFeaturesInterpolated=0 ;

    ::DTMFeatureType featureType;
    DTMHelpers::Copy (featureType,dtmFeatureType);

    DTMException::CheckForErrorStatus (Handle->InterpolateDtmFeatureType (featureType, snapTolerance, *pointsDtm->Handle, *interpolatedDtm->Handle, &numDtmFeatures, &numDtmFeaturesInterpolated));
    return gcnew InterpolationResult(numDtmFeatures,numDtmFeaturesInterpolated);
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2010
//=======================================================================================
void DTM::BrowseFeaturesWithTinErrors ( LinearFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    DTMException::CheckForErrorStatus (Handle->BrowseFeaturesWithTinErrors(&forwarder, &FeaturesBrowsingCallbackForwarderDelegate));
    CheckMemoryPressure();
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2010
//=======================================================================================
void DTM::BrowseFeaturesWithUserTag ( ::DTMUserTag userTag , LinearFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    DTMException::CheckForErrorStatus (Handle->BrowseFeaturesWithUserTag(userTag, &forwarder, &FeaturesBrowsingCallbackForwarderDelegate));
    CheckMemoryPressure();
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2010
//=======================================================================================
void DTM::BrowseFeaturesWithFeatureId ( DTMFeatureId featureId , LinearFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    DTMFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    DTMException::CheckForErrorStatus (Handle->BrowseFeaturesWithFeatureId(featureId.Id, &forwarder, &FeaturesBrowsingCallbackForwarderDelegate));
    CheckMemoryPressure();
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2010
//=======================================================================================
PondCalculation^ DTM::CalculatePond ( double X , double Y)
    {
    return CalculatePond (X, Y, 0);
    }

 //=======================================================================================
// @bsimethod                                               Rob.Cormack      12/2011
//=======================================================================================
PondCalculation^ DTM::CalculatePond ( double X , double Y, double falseLowDepth )
    {
    bool   calculated=true ;
    double elevation=0.0 ;
    double depth=0.0 ;
    double area=0.0 ;
    double volume=0.0 ;
    DTMDynamicFeatureArray pondFeaturesArray;

    CheckIsTriangulated();
    DTMException::CheckForErrorStatus (Handle->CalculatePond(X,Y,falseLowDepth,calculated,elevation,depth,area,volume,pondFeaturesArray));
    CheckMemoryPressure();

    array<DTMDynamicFeatureInfo^>^ pondFeatures = nullptr ;

    if ( pondFeaturesArray.size() > 0 )
        {
        pondFeatures = gcnew array<DTMDynamicFeatureInfo^>((int)pondFeaturesArray.size()) ;
        for (size_t n = 0 ; n < pondFeaturesArray.size() ; ++n)
            pondFeatures[(int)n] = gcnew DTMDynamicFeatureInfo((DTMDynamicFeatureType)pondFeaturesArray[n].featureType,&pondFeaturesArray[n].featurePts[0],(int)pondFeaturesArray[n].featurePts.size()) ;
        }

    return gcnew PondCalculation(calculated,elevation,depth,area,volume,pondFeatures);
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2010
//=======================================================================================
PointCatchmentResult^ DTM::TraceCatchmentForPoint (double X, double Y, double maxPondDepth)
    {
    DTMDynamicFeatureArray catchmentFeaturesArray;
    bool catchmentDetermined = false;
    bvector<DPoint3d> catchmentPoints;
    DPoint3d sumpPoint;

    CheckIsTriangulated ();
    DTMException::CheckForErrorStatus (Handle->TraceCatchmentForPoint (X, Y, maxPondDepth, catchmentDetermined, sumpPoint, catchmentPoints));
    CheckMemoryPressure ();

    array<DTMDynamicFeatureInfo^>^ catchmentFeatures = nullptr;

    if (catchmentDetermined && catchmentPoints.size () > 0)
        {
        catchmentFeatures = gcnew array<DTMDynamicFeatureInfo^> (1);
        catchmentFeatures[0] = gcnew DTMDynamicFeatureInfo (DTMDynamicFeatureType::Catchment, catchmentPoints.data (), (int)catchmentPoints.size ());
        }
    BGEO::DPoint3d mSumpPoint;
    DTMHelpers::Copy (mSumpPoint, sumpPoint);

    return gcnew PointCatchmentResult (catchmentFeatures, mSumpPoint);
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2010
//=======================================================================================
bool DTM::PointVisibility ( BGEO::DPoint3d Eye , BGEO::DPoint3d Point )
    {

    bool pointVisible = false ;

    CheckIsTriangulated();
    DTMException::CheckForErrorStatus (Handle->PointVisibility(pointVisible,Eye.X,Eye.Y,Eye.Z,Point.X,Point.Y,Point.Z));
    CheckMemoryPressure();

    return pointVisible  ;

    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack      3/2010
//=======================================================================================
VisibilityResult^ DTM::LineVisibility ( BGEO::DPoint3d Eye , BGEO::DPoint3d linePoint1 , BGEO::DPoint3d linePoint2  )
    {
    bool lineVisible= false;
    DTMDynamicFeatureArray visibilityFeaturesArray;

    CheckIsTriangulated();
    DTMException::CheckForErrorStatus (Handle->LineVisibility(lineVisible,Eye.X,Eye.Y,Eye.Z,linePoint1.X,linePoint1.Y,linePoint1.Z,linePoint2.X,linePoint2.Y,linePoint2.Z,visibilityFeaturesArray));
    CheckMemoryPressure();

    array<DTMDynamicFeatureInfo^>^ visibilityFeatures = nullptr ;

    if ( visibilityFeaturesArray.size() > 0 )
        {
        visibilityFeatures = gcnew array<DTMDynamicFeatureInfo^>((int)visibilityFeaturesArray.size()) ;
        for (size_t n = 0 ; n < visibilityFeaturesArray.size() ; ++n)
            visibilityFeatures[(int)n] = gcnew DTMDynamicFeatureInfo((DTMDynamicFeatureType)visibilityFeaturesArray[n].featureType,&visibilityFeaturesArray[n].featurePts[0],(int)visibilityFeaturesArray[n].featurePts.size()) ;
        }

    return gcnew VisibilityResult( (VisibilityType) lineVisible, visibilityFeatures);
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack     3/2010
//=======================================================================================
void  DTM::BrowseTinPointsVisibility (BGEO::DPoint3d Eye,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    CheckIsTriangulated();
    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    long cacheSize=50000 ;
    DTMFeatureCache cache (Handle, cacheSize, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseTinPointsVisibility (Eye.X, Eye.Y, Eye.Z));
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack     3/2010
//=======================================================================================
void  DTM::BrowseTinLinesVisibility (BGEO::DPoint3d Eye,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    CheckIsTriangulated();
    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    long cacheSize=50000 ;
    DTMFeatureCache cache (Handle, cacheSize, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseTinLinesVisibility (Eye.X, Eye.Y, Eye.Z));
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack     3/2010
//=======================================================================================
void  DTM::BrowseRadialViewSheds(RadialViewShedsBrowsingCriteria^ radialViewShedCriteria ,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    CheckIsTriangulated();
    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    BGEO::DPoint3d eye     = radialViewShedCriteria->EyeLocation ;
    long  viewShedMethod   = (long) radialViewShedCriteria->RadialViewShedMethod ;
    long  numberRadials    = radialViewShedCriteria->NumberRadials ;
    double radialIncrement = radialViewShedCriteria->RadialIncrement ;

    long cacheSize=50000 ;

    DTMFeatureCache cache (Handle, cacheSize, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseRadialViewSheds (eye.X, eye.Y, eye.Z, viewShedMethod, numberRadials, radialIncrement));
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack     3/2010
//=======================================================================================
void  DTM::BrowseRegionViewSheds(BGEO::DPoint3d eye,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg)
    {
    if (hdlP == nullptr) throw ThrowingPolicy::Apply(gcnew System::ArgumentNullException("hdlP"));

    CheckIsTriangulated();
    DTMDynamicFeaturesBrowsingCallbackForwarder forwarder(this, hdlP, oArg);

    long cacheSize=50000 ;

    DTMFeatureCache cache (Handle, cacheSize, &DynamicFeaturesBrowsingCacheCallbackForwarderDelegate, &forwarder);
    DTMException::CheckForErrorStatus (cache.BrowseRegionViewSheds (eye.X, eye.Y, eye.Z));
    }
//=======================================================================================
// @bsimethod                                               Rob.Cormack     01/2011
//=======================================================================================
void  DTM::ReplaceFeaturePoints (DTMFeatureId dtmFeatureId , array<BGEO::DPoint3d>^ points)
    {
    pin_ptr<BGEO::DPoint3d const> pointsP = &points[0];
    DTMException::CheckForErrorStatus(Handle->ReplaceFeaturePoints(dtmFeatureId.Id,( DPoint3d *) pointsP,points->Length));
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack     01/2011
//=======================================================================================
bool DTM::TryReplaceFeaturePoints (DTMFeatureId dtmFeatureId , array<BGEO::DPoint3d>^ points)
    {
    pin_ptr<BGEO::DPoint3d const> pointsP = &points[0];
    return Handle->ReplaceFeaturePoints(dtmFeatureId.Id,( DPoint3d *) pointsP,points->Length) == SUCCESS;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     07/2011
//=======================================================================================
System::DateTime DTM::LastModifiedTime::get()
    {
    __int64 time;

    Handle->GetLastModifiedTime (time);
    return System::DateTime (1601,1,1).AddTicks (time);
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     08/2011
//=======================================================================================
void DTM::Commit()
    {
    Handle->SetMemoryAccess(DTMAccessMode::Commit);
    }

//=======================================================================================
// @bsimethod                                               Rob.Cormack     10/2012
//=======================================================================================

StockPileResult^  DTM::CreateStockPile (StockPileCriteria^ stockPileCriteria )
    {
     BcDTMPtr  stockPileDtm;
     BcDTMPtr  mergedDtm;
     DPoint3d  headPoint,*headPointsP=nullptr ;
     double    stockPileSlope,volume=0.0 ;
     DTM^      stockPileTM ;
     DTM^      mergedTM ;

     long numHeadPoints = stockPileCriteria->StockPilePoints->Length ;

     if( numHeadPoints > 0 )
       {
        pin_ptr<BGEO::DPoint3d> headPoints = &stockPileCriteria->StockPilePoints[0];

        stockPileSlope = stockPileCriteria->StockPileSlope ;

        //  Point Stock Piles

        if( stockPileCriteria->StockPileType == StockPileFeature::PointStockPile )
          {
           headPoint.x = headPoints[0].X;
           headPoint.y = headPoints[0].Y;
           headPoint.z = headPoints[0].Z;
           DTMException::CheckForErrorStatus (Handle->CreatePointStockPile(headPoint,stockPileSlope,stockPileCriteria->MergeOption,volume,&stockPileDtm,&mergedDtm));
          }

        // Alignment Stock Piles

        if( stockPileCriteria->StockPileType == StockPileFeature::AlignmentStockpile )
          {
           headPointsP = (DPoint3d *) malloc ( numHeadPoints * sizeof(DPoint3d)) ;
           memcpy(headPointsP,headPoints,numHeadPoints*sizeof(DPoint3d)) ;
           DTMException::CheckForErrorStatus (Handle->CreateAlignmentStockPile(headPointsP,numHeadPoints,stockPileSlope,stockPileCriteria->MergeOption,volume,&stockPileDtm,&mergedDtm));
          }

        CheckMemoryPressure();

        // Create The Stock Pile DTM

        if (stockPileDtm.IsValid())
           {
            stockPileTM = gcnew DTM (stockPileDtm.get());
           }

        // Create The Merged DTM

        if (mergedDtm.IsValid())
           {
            mergedTM = gcnew DTM (mergedDtm.get());
           }
       }

     // Return Stock Pile Result

     return gcnew StockPileResult(volume,stockPileTM,mergedTM);

    }

bool DTM::IntersetVector ([System::Runtime::InteropServices::Out]BGEO::DPoint3d% intersectionPoint, BGEO::DPoint3d startPoint, BGEO::DPoint3d endPoint)
    {
    ::DPoint3d uStartPoint;
    ::DPoint3d uEndPoint;
    ::DPoint3d uIntersectionPoint;
    DTMHelpers::Copy (uStartPoint, startPoint);
    DTMHelpers::Copy (uEndPoint, endPoint);

    bool ret = Handle->IntersectVector (uIntersectionPoint, uStartPoint, uEndPoint);

    if (true == ret)
        {
        BGEO::DPoint3d mPoint;
        DTMHelpers::Copy (intersectionPoint, uIntersectionPoint);
        }
    return ret;
    }

void DTM::Clean ()
    {
    DTMException::CheckForErrorStatus (Handle->Clean ());
    }

//=======================================================================================
// @bsimethod
//=======================================================================================
DTMFeatureId DTM::AddFeatureWithMultipleSegments (System::Collections::Generic::IEnumerable<System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^>^ features, DTMFeatureType featureType, DTMUserTag userTag)
    {
    DtmVectorString uFeatures;

    for each (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points in features)
        {
        bvector<::DPoint3d> pts;
        GetVectorPointsFromIEnumerable (pts, points);
        DtmString dtmString (&pts[0], (int)pts.size ());
        uFeatures.push_back (dtmString);
        }
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    DTMException::CheckForErrorStatus (Handle->AddFeatureWithMultipleSegments ((::DTMFeatureType)featureType, uFeatures, userTag, &featureID));
    return DTMFeatureId (featureID);
    }

DTMFeatureId DTM::AddFeatureWithMultipleSegments (System::Collections::Generic::IEnumerable<array<BGEO::DPoint3d>^>^ features, DTMFeatureType featureType, DTMUserTag userTag)
    {
    DtmVectorString uFeatures;

    for each (array<BGEO::DPoint3d>^ points in features)
        {
        pin_ptr<BGEO::DPoint3d const> tPoint = &points[0];
        DtmString dtmString ((::DPoint3d*)tPoint, points->Length);
        uFeatures.push_back (dtmString);
        }
    ::DTMFeatureId  featureID = DTM_NULL_FEATURE_ID;
    DTMException::CheckForErrorStatus (Handle->AddFeatureWithMultipleSegments ((::DTMFeatureType)featureType, uFeatures, userTag, &featureID));
    return DTMFeatureId (featureID);
    }

bool DTM::TryReplaceFeatureWithMultipleSegments (System::Collections::Generic::IEnumerable<System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^>^ features, DTMFeatureId featureId)
    {
    DtmVectorString uFeatures;

    for each (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points in features)
        {
        bvector<::DPoint3d> pts;
        GetVectorPointsFromIEnumerable (pts, points);
        DtmString dtmString (&pts[0], (int)pts.size ());
        uFeatures.push_back (dtmString);
        }
    return Handle->ReplaceFeatureWithMultipleSegments (uFeatures, (::DTMFeatureId)featureId.Id) == DTM_SUCCESS;
    }

bool DTM::TryReplaceFeatureWithMultipleSegments (System::Collections::Generic::IEnumerable<array<BGEO::DPoint3d>^>^ features, DTMFeatureId featureId)
    {
    DtmVectorString uFeatures;

    for each (array<BGEO::DPoint3d>^ points in features)
        {
        pin_ptr<BGEO::DPoint3d const> tPoint = &points[0];
        DtmString dtmString ((::DPoint3d*)tPoint, points->Length);
        uFeatures.push_back (dtmString);
        }
    return Handle->ReplaceFeatureWithMultipleSegments (uFeatures, (::DTMFeatureId)featureId.Id) == DTM_SUCCESS;
    }

bool DTM::GetProjectedPointOnDTM ( [System::Runtime::InteropServices::Out]BGEO::DPoint3d% pointOnDTM, BGEO::DMatrix4d% w2vMap, BGEO::DPoint3d testPoint)
    {
    ::DPoint3d uTestPoint;
    ::DPoint3d uPointOnDtm;
    ::DMatrix4d uW2vMap;
    DTMHelpers::Copy  (uTestPoint, testPoint);
    DTMHelpers::Copy  (uW2vMap, w2vMap);

    bool ret = Handle->GetProjectedPointOnDTM (uPointOnDtm, uW2vMap, uTestPoint);
    if (ret)
        DTMHelpers::Copy  (pointOnDTM, uPointOnDtm);

    return ret;
    }

bool DTM::GetProjectedPointOnDTM ( [System::Runtime::InteropServices::Out]BGEO::DPoint3d% pointOnDTM, BGEO::DMap4d% w2vMap, BGEO::DPoint3d testPoint)
    {
    ::DPoint3d uTestPoint;
    ::DPoint3d uPointOnDtm;
    ::DMap4d uW2vMap;
    DTMHelpers::Copy  (uTestPoint, testPoint);
    DTMHelpers::Copy  (uW2vMap, w2vMap);

    bool ret = Handle->GetProjectedPointOnDTM (uPointOnDtm, uW2vMap.M0, uTestPoint);
    if (ret)
        DTMHelpers::Copy  (pointOnDTM, uPointOnDtm);

    return ret;
    }

void DTM::FilterPoints (long numPointsToRemove, double percentageToRemove, long% pointsBefore, long% pointsAfter)
    {
    long _pointsBefore;
    long _pointsAfter;

    Handle->FilterPoints (numPointsToRemove, percentageToRemove, _pointsBefore, _pointsAfter);
    pointsBefore = _pointsBefore;
    pointsAfter = _pointsAfter;
    }

DTM^ DTM::GetTransformedDTM(BGEO::DTransform3d transform)
    {
    ::Transform nativeTransform;
    pin_ptr<BGEO::DTransform3d> pTransform = &transform;

    nativeTransform = *(::Transform*)pTransform;
    DTMPtr dtm = nullptr;
    DTMException::CheckForErrorStatus(Handle->GetTransformDTM(dtm, nativeTransform));
    if (dtm.IsValid())
        return gcnew DTM(dynamic_cast<BcDTMP>(dtm.get()));
    return nullptr;
    }
END_BENTLEY_TERRAINMODELNET_NAMESPACE


