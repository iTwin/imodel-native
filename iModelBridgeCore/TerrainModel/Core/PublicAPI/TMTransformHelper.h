/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/TMTransformHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "bcDTMClass.h"
#include "dtmevars.h"
BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

struct TMTransformHelper : RefCountedBase
    {
#ifdef ToDo
    private:

        struct bcDTMEdgesTransformWrapper : public IBcDTMEdges, NonCopyableClass
            {
            private: BcDTMEdgesPtr m_edges;
            private: bcDTMTransformWrapper* m_wrapper;
            public: bcDTMEdgesTransformWrapper (BcDTMEdgesPtr edges, bcDTMTransformWrapper* wrapper)
                        {
                        m_edges = edges;
                        m_wrapper = wrapper;
                        wrapper->AddRef ();
                        }
            public: virtual ~bcDTMEdgesTransformWrapper ()
                        {
                        if (m_wrapper)
                            m_wrapper->Release();
                        }

            public: int GetEdgeCount()
                        {
                        return m_edges->GetEdgeCount();
                        }
            public: void GetEdgeStartPoint (int index, DPoint3d* pt)
                        {
                        m_edges->GetEdgeStartPoint (index, pt);
                        m_wrapper->convertPointFromDTM (*pt);
                        }
            public: void GetEdgeEndPoint(int index, DPoint3d* pt)
                        {
                        m_edges->GetEdgeEndPoint (index, pt);
                        m_wrapper->convertPointFromDTM (*pt);
                        }
            };

        struct bcDTMFeatureEnumeratorTransform : public IBcDTMFeatureEnumerator, NonCopyableClass
            {
            private:
                BcDTMFeatureEnumeratorPtr m_enumerator;
                bcDTMTransformWrapper* m_wrapper;
            public:
                bcDTMFeatureEnumeratorTransform(BcDTMFeatureEnumeratorPtr enumerator, bcDTMTransformWrapper* wrapper)
                    {
                    m_enumerator = enumerator;
                    m_wrapper = wrapper;
                    m_wrapper->AddRef ();
                    }
                virtual ~bcDTMFeatureEnumeratorTransform()
                    {
                    if (m_wrapper)
                        m_wrapper->Release ();
                    }

                virtual bool moveNext () //BoolInt
                    {
                    return m_enumerator->moveNext();
                    }

                virtual int reset ()
                    {
                    return m_enumerator->reset ();
                    }

                virtual IBcDTMFeature* current ()
                    {
                    IBcDTMFeature* cur = m_enumerator->current();
                    if (cur)
                        m_wrapper->convertBcDTMFeatureFromDTM (cur);
                    return cur;
                    }

                virtual BcDTMFeatureEnumeratorPtr clone()
                    {
                    BcDTMFeatureEnumeratorPtr cloned = m_enumerator->clone ();
                    if (cloned.IsValid())
                        return new bcDTMFeatureEnumeratorTransform (cloned, m_wrapper);
                    return cloned;
                    }

                virtual int setRange (DPoint3d *minRangeP, DPoint3d *maxRangeP)
                    {
                    DPoint3d minRange = *minRangeP;
                    DPoint3d maxRange = *maxRangeP;

                    m_wrapper->convertPointToDTM (minRange);
                    m_wrapper->convertPointToDTM (maxRange);
                    return m_enumerator->setRange (&minRange, &maxRange);
                    }

                virtual int removeRange ()
                    {
                    return m_enumerator->removeRange ();
                    }
                virtual int includeFeatureType (DTMFeatureType featureType)
                    {
                    return m_enumerator->includeFeatureType (featureType);
                    }
                virtual int excludeAllFeatureTypes ()
                    {
                    return m_enumerator->excludeAllFeatureTypes ();
                    }
                virtual int setFeatureTypes (DTMFeatureType *featureTypesP, int nFeature)
                    {
                    return m_enumerator->setFeatureTypes (featureTypesP, nFeature);
                    }
                /// BCivilObject
                ////__BCIVILOBJECTIMPL;
            };
#endif
    public:

        //=======================================================================================
        //! Base class to make a class non-copyable
        // @bsiclass                                                     11/09
        //=======================================================================================
        class NonCopyableClassExceptMove
            {
            private:
                NonCopyableClassExceptMove(NonCopyableClass const&);
                NonCopyableClassExceptMove& operator= (NonCopyableClass const&);

            protected:
                NonCopyableClassExceptMove()
                    {
                    }
                ~NonCopyableClassExceptMove()
                    {
                    }
                NonCopyableClassExceptMove(NonCopyableClassExceptMove&&)
                    {
                    }
            };

        // Help Array Functions for creating and deleting memory if required.
        struct DPoint3dCopy : NonCopyableClassExceptMove
            {
            protected:
                DPoint3d* m_values;
                bool m_ownValues;

            public:
                DPoint3dCopy(DPoint3dCopy&& copy)
                    {
                    m_values = copy.m_values;
                    m_ownValues = copy.m_ownValues;
                    copy.m_values = nullptr;
                    copy.m_ownValues = false;
                    }
                DPoint3dCopy(DPoint3dCP values, bool ownValues)
                    {
                    m_values = const_cast<DPoint3d*>(values);
                    m_ownValues = ownValues;
                    }
                virtual ~DPoint3dCopy ()
                    {
                    if (m_ownValues) delete[] m_values;
                    }
                operator ::DPoint3d* () const
                    {
                    return m_values;
                    }
            };
        struct DTMFenceParamsCopy : DPoint3dCopy
            {
            private:
                DTMFenceParamsCP m_params;
            public:
                DTMFenceParamsCopy (DTMFenceParamsCP params, DPoint3dCP values, bool ownValues) : DPoint3dCopy (values, ownValues)
                    {
                    m_params = params;
                    }
                DTMFenceParamsCopy(DTMFenceParamsCopy&& copy) : DPoint3dCopy((DTMFenceParamsCopy&&)copy)
                    {
                    m_params = copy.m_params;
                    copy.m_params = nullptr;
                    }
                ~DTMFenceParamsCopy()
                    {
                    if (m_ownValues) delete m_params;
                    }
                operator ::DTMFenceParamsCR () const
                    {
                    return *m_params;
                    }
            };
        struct DoubleCopy : NonCopyableClass
            {
            private:
                double* m_values;
                bool m_ownValues;
            public:
                DoubleCopy (const double* values, bool ownValues)
                    {
                    m_values = const_cast<double*>(values);
                    m_ownValues = ownValues;
                    }
                DoubleCopy(DoubleCopy&& copy) : DoubleCopy((DoubleCopy&&)copy)
                    {
                    m_values = copy.m_values;
                    m_ownValues = copy.m_ownValues;
                    copy.m_values = nullptr;
                    copy.m_ownValues = false;
                    }
                ~DoubleCopy ()
                    {
                    if (m_ownValues) delete [] m_values;
                    }
                operator double* () const
                    {
                    return m_values;
                    }
            };
        struct VOLRANGETABCopy : NonCopyableClass
            {
            private:
                VOLRANGETAB* m_values;
                bool m_ownValues;
            public:
                VOLRANGETABCopy (const VOLRANGETAB* values, bool ownValues)
                    {
                    m_values = const_cast<VOLRANGETAB*>(values);
                    m_ownValues = ownValues;
                    }
                VOLRANGETABCopy(VOLRANGETABCopy&& copy) : VOLRANGETABCopy((VOLRANGETABCopy&&)copy)
                    {
                    m_values = copy.m_values;
                    m_ownValues = copy.m_ownValues;
                    copy.m_values = nullptr;
                    copy.m_ownValues = false;
                    }
                ~VOLRANGETABCopy ()
                    {
                    if (m_ownValues) delete [] m_values;
                    }
                operator VOLRANGETAB* () const
                    {
                    return m_values;
                    }
            };
        struct DRange1dCopy : NonCopyableClass
            {
            private:
                DRange1d* m_values;
                bool m_ownValues;
            public:
                DRange1dCopy (const DRange1d* values, bool ownValues)
                    {
                    m_values = const_cast<DRange1d*>(values);
                    m_ownValues = ownValues;
                    }
                DRange1dCopy(DRange1dCopy&& copy) : DRange1dCopy((DRange1dCopy&&)copy)
                    {
                    m_values = copy.m_values;
                    m_ownValues = copy.m_ownValues;
                    copy.m_values = nullptr;
                    copy.m_ownValues = false;
                    }
                ~DRange1dCopy ()
                    {
                    if (m_ownValues) delete [] m_values;
                    }
                operator DRange1d* () const
                    {
                    return m_values;
                    }
            };

        // From DTM convert functions
        double convertDistanceFromDTM (const double& value)
            {
            if (!m_isIdentity)
                {
                return value * m_scale;
                }
            return value;
            }
        double convertElevationFromDTM (const double& value)
            {
            if (!m_isIdentity)
                {
                return (value * m_scale) + m_elevationOffset;
                }
            return value;
            }
        void convertPointFromDTM (double& x, double& y)
            {
            if (!m_isIdentity)
                {
                DPoint3d pt = {x, y, 0};
                m_transform.multiply (&pt);
                x = pt.x;
                y = pt.y;
                }
            }
        void convertPointFromDTM (DPoint3d& pt)
            {
            if (!m_isIdentity)
                {
                m_transform.multiply (&pt);
                }
            }
        void convertPointsFromDTM (bvector<DPoint3d> pts)
            {
            m_transform.multiply (pts.data(), (int)pts.size());
            }

        void convertPointsFromDTM (DPoint3d* pts, int numPts)
            {
            if (!m_isIdentity && pts)
                m_transform.multiply (pts, numPts);
            }

        DPoint3dCopy copyPointsFromDTM (DPoint3dCP pts, int numPts)
            {
            if (!m_isIdentity && pts)
                {
                DPoint3d* newPts = new DPoint3d[numPts];
                m_transform.multiply (newPts, pts, numPts);
                return DPoint3dCopy (newPts, true);
                }
            return DPoint3dCopy (pts, false);
            }

        static DPoint3dCopy copyPointsFromDTM (TMTransformHelperP helper, DPoint3dCP pts, int numPts)
            {
            if (helper && pts)
                {
                return helper->copyPointsFromDTM (pts, numPts);
                }
            return DPoint3dCopy (pts, false);
            }

        double convertSlopeFromDTM (const double& value)
            {
            return value;
            }
        double convertAspectFromDTM (const double& value)
            {
            if (m_isIdentity)
                return value;

            double newValue = value - m_aspectFix;
            if (newValue < 0)
                newValue += 360;
            return newValue;
            }
        double convertAreaFromDTM (const double& value)
            {
            return value * (m_scale * m_scale);
            }
        void convertAreasFromDTM (double* values, int num)
            {
            if (!m_isIdentity && values)
                {
                for (int i = 0; i < num; i++, values++)
                    {
                    *values = convertAreaFromDTM (*values);
                    }
                }
            }
        double convertVolumeFromDTM (const double& value)
            {
            return value * (m_scale * m_scale * m_scale);
            }
        void copyVOLRANGETABFromDTM (VOLRANGETABCopy& volRange, VOLRANGETAB* dest, int numValues)
            {
            if (!m_isIdentity && dest)
                {
                VOLRANGETAB* values = volRange;
                for (int i = 0; i < numValues; i++)
                    {
                    dest[i].Cut = convertVolumeFromDTM (values[i].Cut);
                    dest[i].Fill = convertVolumeFromDTM (values[i].Fill);
                    }
                }
            }
        Bentley::TerrainModel::DTMPointArray convertDTMPointArrayFromDTM (Bentley::TerrainModel::DTMPointArray& in)
            {
            int numPts = (int)in.size();
            DPoint3dP pts = in.data();

            if (pts)
                convertPointsFromDTM (pts, numPts);
            return in;
            }
        Bentley::TerrainModel::DTMDrainageFeaturePtr convertDTMDrainageFeaturePtrFromDTM (Bentley::TerrainModel::DTMDrainageFeaturePtr& in)
            {
            if (in.IsNull ())
                return in;

            for (int i = 0; i < in->GetNumParts(); i++)
                {
                Bentley::TerrainModel::DTMPointArray pts;

                if (in->GetPoints (pts, i) == DTM_SUCCESS)
                    convertDTMPointArrayFromDTM (pts);
                }
            return in;
            }
        void convertDtmStringVectorFromDTM (bvector<DtmString>& value)
            {
            for(DtmString it : value)
                convertPointsFromDTM (it);
            return;
            }
        void convertDTMDynamicFeaturesFromDTM (DTMDynamicFeatureArray& features)
            {
            for(DTMDynamicFeature it : features)
                convertPointsFromDTM (&it.featurePts[0], (int)it.featurePts.size());
            }

#ifdef __BENTLEYDTM_BUILD__
        void convertDrapedPointsFromDTM (bvector<RefCountedPtr<BcDTMDrapedLinePoint>>& drapedPoints)
            {
            for (RefCountedPtr<BcDTMDrapedLinePoint> it : drapedPoints)
                {
                DPoint3d pt;
                it->GetPointCoordinates (pt);
                convertPointFromDTM (pt);
                it->SetPoint (pt);
                it->SetDistance (convertDistanceFromDTM (it->GetDistance ()));
                }
            }
#endif

        //void convertBcDTMDrapedLineFromDTM (BcDTMDrapedLinePtr& drapedLineP)
        //    {
        //    BcDTMDrapedLine* dl = dynamic_cast<BcDTMDrapedLine*>(drapedLineP.get());

        //    convertDrapedPointsFromDTM (dl->GetDrapedPoints ());
        //    }
        //void convertDTMDrapedLineFromDTM (DTMDrapedLinePtr& drapedLineP)
        //    {
        //    BcDTMDrapedLine* dl = dynamic_cast<BcDTMDrapedLine*>(drapedLineP.get());

        //    for (bvector<RefCountedPtr<DrapedPoint>>::iterator it = dl->GetDrapedPoints().begin(); it != dl->GetDrapedPoints().end(); it++)
        //        {
        //        DPoint3d pt;
        //        it->GetPointCoordinates(pt);
        //        convertPointFromDTM (pt);
        //        it->SetPoint (pt);
        //        it->SetDistance (convertDistanceFromDTM (it->GetDistance ()));
        //        }
        //    }
        //void convertBcDTMFeatureFromDTM (BcDTMFeaturePtr feature)
        //    {
        //    BcDTMLinearFeature* linFeature = dynamic_cast<BcDTMLinearFeature*>(feature.get());

        //    if (linFeature)
        //        {
        //        convertPointsFromDTM (linFeature->GetLinearEl().pointsP, linFeature->GetLinearEl().numPoints);
        //        return;
        //        }

        //    BcDTMComplexLinearFeature* complex = dynamic_cast<BcDTMComplexLinearFeature*>(feature.get());

        //    if (complex)
        //        {
        //        BcDtmLinearElArray& elements = complex->GetElmList();

        //        for (BcDtmLinearElArray::iterator it = elements.begin(); it != elements.end(); it++)
        //            {
        //            convertPointsFromDTM (it->getElementReference().pointsP, it->getElementReference().numPoints);
        //            }
        //        return;
        //        }

        //    BcDTMSpot* spot = dynamic_cast<BcDTMSpot*>(feature.get());

        //    if (spot)
        //        {
        //        convertPointsFromDTM (spot->GetPointsReference(), spot->GetPointsCount());
        //        return;
        //        }
        //    // Unknown feature
        //    }
        // To DTM convert functions
        DTMFenceParamsCopy convertDTMFenceParamsToDTM (const DTMFenceParams& fence)
            {
            if (!m_isIdentity && fence.numPoints)
                {
                DPoint3d* newPts = new DPoint3d[fence.numPoints];
                m_transformInv.multiply (newPts, fence.points, fence.numPoints);
                DTMFenceParamsP newParams = new DTMFenceParams (fence.fenceType, fence.fenceOption, newPts, fence.numPoints);
                return DTMFenceParamsCopy (newParams, newParams->points, false);
                }
//            Bentley::TerrainModel::DTMFenceParams ()
            return DTMFenceParamsCopy (&fence, fence.points, false);
            }
        double convertSlopeToDTM (const double& value)
            {
            return value;
            }
        double convertAspectToDTM (const double& value)
            {
            if (m_isIdentity)
                return value;
            double newValue = value + m_aspectFix;
            if (newValue > 360)
                newValue -= 360;
            return newValue;
            }
        double convertDistanceToDTM (const double& value)
            {
            if (!m_isIdentity)
                {
                return value / m_scale;
                }
            return value;
            }
        double convertElevationToDTM (const double& value)
            {
            if (!m_isIdentity)
                {
                return (value - m_elevationOffset) / m_scale ;
                }
            return value;
            }
        DoubleCopy copyElevationTableToDTM (const double* dists, int numDists)
            {
            if (!m_isIdentity && dists)
                {
                double* newDists = new double[numDists];

                for (int i = 0; i < numDists; i++)
                    newDists[i] = convertElevationToDTM (dists[i]);
                return DoubleCopy (newDists, true);
                }
            return DoubleCopy (dists, false);
            }
        DoubleCopy copyDistanceTableToDTM (const double* dists, int numDists)
            {
            if (!m_isIdentity && dists)
                {
                double* newDists = new double[numDists];

                for (int i = 0; i < numDists; i++)
                    newDists[i] = convertDistanceToDTM (dists[i]);
                return DoubleCopy (newDists, true);
                }
            return DoubleCopy (dists, false);
            }
        VOLRANGETABCopy copyVOLRANGETABToDTM (VOLRANGETAB* values, int numValues)
            {
            if (!m_isIdentity && values)
                {
                VOLRANGETAB* newValues = new VOLRANGETAB[numValues];

                for (int i = 0; i < numValues; i++)
                    {
                    newValues[i].Low = convertElevationToDTM (values[i].Low);
                    newValues[i].High = convertElevationToDTM (values[i].High);
                    }
                return VOLRANGETABCopy (newValues, true);
                }
            return VOLRANGETABCopy (values, false);
            }

        void convertRangeFromDTM (DRange3dR range)
            {
            // ToDo look at this logic
            convertPointFromDTM (range.low);
            convertPointFromDTM (range.high);
            }

        DRange1dCopy copyBcDTMElevationRangeToDTM (DRange1d* values, int numValues)
            {
            if (!m_isIdentity && values)
                {
                DRange1d* newValues = new DRange1d[numValues];

                for (int i = 0; i < numValues; i++)
                    {
                    newValues[i].low = convertElevationToDTM (values[i].low);
                    newValues[i].high = convertElevationToDTM (values[i].high);
                    }
                return DRange1dCopy (newValues, true);
                }
            return DRange1dCopy (values, false);
            }
        DRange1dCopy copyBcDTMAspectRangeToDTM (DRange1d* values, int numValues)
            {
            if (!m_isIdentity)
                {
                DRange1d* newValues = new DRange1d[numValues];

                for (int i = 0; i < numValues; i++)
                    {
                    newValues[i].low = convertAspectToDTM (values[i].low);
                    newValues[i].high = convertAspectToDTM (values[i].high);
                    }
                return DRange1dCopy (newValues, true);
                }
            return DRange1dCopy (values, false);
            }
        void convertPointToDTM (double& x, double& y)
            {
            if (!m_isIdentity)
                {
                DPoint3d pt = { x, y, 0 };
                m_transformInv.multiply (&pt);
                x = pt.x;
                y = pt.y;
                }
            }
        void convertPointToDTM (double& x, double& y, double & z)
            {
            if (!m_isIdentity)
                {
                DPoint3d pt = { x, y, z };
                m_transformInv.multiply (&pt);
                x = pt.x;
                y = pt.y;
                z = pt.z;
                }
            }
        void convertPointToDTM (DPoint3d& pt)
            {
            if (!m_isIdentity)
                {
                m_transformInv.multiply (&pt);
                }
            }
        void convertPointsToDTM (DPoint3d* pts, int numPts)
            {
            if (!m_isIdentity && pts)
                {
                m_transformInv.multiply (pts, numPts);
                }
            }
        void convertPointsToDTM (DtmString& string)
            {
            if (!m_isIdentity && !string.empty())
                {
                m_transformInv.multiply (string.data(), (int)string.size());
                }
            }
        DPoint3dCopy copyPointsToDTM (DPoint3dCP pts, int numPts)
            {
            if (!m_isIdentity && pts)
                {
                DPoint3d* newPts = new DPoint3d[numPts];
                m_transformInv.multiply (newPts, pts, numPts);
                return DPoint3dCopy (newPts, true);
                }
            return DPoint3dCopy (pts, false);
            }

        static DPoint3dCopy copyPointsToDTM (TMTransformHelperP helper, DPoint3dCP pts, int numPts)
            {
            if (helper && pts)
                {
                return helper->copyPointsToDTM (pts, numPts);
                }
            return DPoint3dCopy (pts, false);
            }

        void convertDtmStringVectorToDTM (bvector<DtmString>& value)
            {
            for (DtmString it : value)
                convertPointsToDTM (it);
            return;
            }

        BcDTMPtr getDTMToExport (BcDTMP bcdtm)
            {
            if (m_isIdentity)
                return bcdtm;
            BcDTMPtr newDtm = bcdtm->Clone();

            newDtm->Transform (m_transformInv);
            return newDtm;
            }
        BcDTMPtr convertDTMToDTM (BcDTMR dtm)
            {
            Transform trsf;
            if (dtm.GetTransformHelper ())
                dtm.GetTransformHelper ()->GetTransformationFromDTM (trsf);
            else
                trsf.initIdentity ();
            if (m_transform.isEqual (&trsf))
                {
                return &dtm;
                }

            // Need to get the transform between dtm and source and then clone and transform dtm.
            BcDTMPtr newDtm = dtm.Clone ();

            Transform convertTrsf;
            Transform transformInv;
            transformInv.inverseOf (&m_transform);
            convertTrsf.productOf (&trsf, &transformInv);

            newDtm->Transform (convertTrsf);
            return newDtm;
            }
        static BcDTMPtr convertDTMToDTM (BcDTMR source, BcDTMR dtm)
            {
            if (source.GetTransformHelper ())
                return source.GetTransformHelper ()->convertDTMToDTM (dtm);

            Transform trsf;
            if (dtm.GetTransformHelper())
                dtm.GetTransformHelper ()->GetTransformationFromDTM (trsf);
            else
                trsf.initIdentity();
            if (trsf.isIdentity())
                return &dtm;

            // Need to get the transform between dtm and source and then clone and transform dtm.
            BcDTMPtr newDtm = dtm.Clone();

            Transform transformInv;
            transformInv.inverseOf (&trsf);

            newDtm->Transform (transformInv);
            return newDtm;
            }
#ifdef ToDo
        struct PFTransformCallbackWrapper
            {
             int (*callback)(DPoint3d* pts, int numPts, void* userP);
             bcDTMTransformWrapper* wrapper;
             void* userP;
            };
        static int PFTransformCallbackWrapperFunction (DPoint3d* pts, int numPts, void* userP)
            {
            PFTransformCallbackWrapper* data = (PFTransformCallbackWrapper*)userP;

            data->wrapper->convertPointsFromDTM (pts, numPts);
            int status = data->callback (pts, numPts, data->userP);
            data->wrapper->convertPointsToDTM (pts, numPts);
            return status;
            }

        // Call back wrappers for converting the coordinates
        struct PFTriangleMeshCallbackWrapper
            {
            DTMTriangleMeshCallback callback;
            bcDTMTransformWrapper* wrapper;
            void *userP;
            };
        static int PFTriangleMeshCallbackWrapperFunction (DTMFeatureType featureType, int numTriangles, int numMeshPoints, DPoint3d *meshPointsP, int numMeshFaces, int *meshFacesP, void *userP)
            {
            PFTriangleMeshCallbackWrapper* data = (PFTriangleMeshCallbackWrapper*)userP;
            data->wrapper->convertPointsFromDTM (meshPointsP, numMeshPoints);

            return data->callback (featureType, numTriangles, numMeshPoints, meshPointsP, numMeshFaces, meshFacesP, data->userP);
            }

        struct PFDuplicatePointsCallbackWrapper
            {
            DTMDuplicatePointsCallback callback;
            bcDTMTransformWrapper* wrapper;
            void *userP;
            };
        static int PFDuplicatePointsCallbackWrapperFunction (double x, double y, DTM_DUPLICATE_POINT_ERROR *dupErrorsP, long numDupErrors, void *userP)
            {
            PFDuplicatePointsCallbackWrapper* data = (PFDuplicatePointsCallbackWrapper*)userP;
            data->wrapper->convertPointFromDTM (x, y);

            for (int i = 0; i < numDupErrors; i++)
                {
                data->wrapper->convertPointsFromDTM ((DPoint3d*)&dupErrorsP[i].x, 1);
                }
            return data->callback (x, y, dupErrorsP, numDupErrors, data->userP);
            }

        struct PFCrossingFeaturesCallbackWrapper
            {
            DTMCrossingFeaturesCallback callback;
            bcDTMTransformWrapper* wrapper;
            void *userP;
            };
        static int PFCrossingFeaturesCallbackWrapperFunction (DTM_CROSSING_FEATURE_ERROR* crossError, void *userP)
            {
            PFCrossingFeaturesCallbackWrapper* data = (PFCrossingFeaturesCallbackWrapper*)userP;

            data->wrapper->convertPointFromDTM (crossError->intersectionX, crossError->intersectionY);

            crossError->elevation1 = data->wrapper->convertElevationFromDTM (crossError->elevation1);
            crossError->distance1 = data->wrapper->convertDistanceFromDTM (crossError->distance1);
            crossError->elevation2 = data->wrapper->convertElevationFromDTM (crossError->elevation2);
            crossError->distance2 = data->wrapper->convertDistanceFromDTM (crossError->distance2);
            return data->callback (crossError, data->userP);
            }

        struct PFBrowseSinglePointFeatureCallbackWrapper
            {
            DTMBrowseSinglePointFeatureCallback callback;
            bcDTMTransformWrapper* wrapper;
            void *userP;
            };
        static int PFBrowseSinglePointFeatureCallbackWrapperFunction (DTMFeatureType featureType, DPoint3d *point, void *userP)
            {
            PFBrowseSinglePointFeatureCallbackWrapper* data = (PFBrowseSinglePointFeatureCallbackWrapper*)userP;
            DPoint3d pt = *point;

            data->wrapper->convertPointFromDTM (pt);
            return data->callback (featureType, &pt, data->userP);
            }

        struct PFBrowseContourCallbackWrapper
            {
            DTMFeatureCallback callback;
            bcDTMTransformWrapper* wrapper;
            void *userP;
            };
        static int PFBrowseContourCallbackWrapperFunction (DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId, DPoint3d *tPoint, size_t nPoint, void *userP)
            {
            PFBrowseContourCallbackWrapper* data = (PFBrowseContourCallbackWrapper*)userP;
            data->wrapper->convertPointsFromDTM (tPoint, (int)nPoint);
            return data->callback (featureType, featureTag, featureId, tPoint, (int)nPoint, data->userP);
            }

        struct PFBrowseFeatureCacheCallbackWrapper
            {
            DTMBrowseFeatureCacheCallback callback;
            bcDTMTransformWrapper* wrapper;
            void *userP;
            };
        static int PFBrowseFeatureCacheCallbackWrapperFunction (DTMFeatureCache* cache, void *userP)
            {
            PFBrowseFeatureCacheCallbackWrapper* data = (PFBrowseFeatureCacheCallbackWrapper*)userP;

            for (int i = 0; i < cache->NumFeatures(); i++)
                {
                DTMFeatureCache::feature* f = cache->GetFeature(i);
                data->wrapper->convertPointsFromDTM (f->tPoint, f->nPoint);
                }
            return data->callback (cache, data->userP);
            }

        struct PFBrowseFeatureCallbackWrapper
            {
            DTMFeatureCallback callback;
            bcDTMTransformWrapper* wrapper;
            void *userP;
            };
        static int PFBrowseFeatureCallbackWrapperFunction (DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId, DPoint3d *tPoint, size_t nPoint, void *userP)
            {
            PFBrowseFeatureCallbackWrapper* data = (PFBrowseFeatureCallbackWrapper*)userP;
            data->wrapper->convertPointsFromDTM (tPoint, (int)nPoint);
            return data->callback (featureType, featureTag, featureId, tPoint, nPoint, data->userP);
            }
#endif
    private:
        bool m_isIdentity;
        Bentley::Transform m_transform;
        Bentley::Transform m_transformInv;
        double m_scale;
        double m_elevationOffset;
        double m_aspectFix;

    public:
        TMTransformHelper ()
            {
            m_transform.initIdentity();
            m_transformInv = m_transform;
            m_isIdentity = true;

            m_aspectFix = 0;
            m_scale = 1;
            m_elevationOffset = 0;
            }

        TMTransformHelper (TransformCR transform)
            {
            m_transform = transform;
            m_transformInv.inverseOf (&m_transform);
            m_isIdentity = m_transform.isIdentity() != 0;
            m_elevationOffset = m_transform.form3d[2][3];

            DPoint3d fixedPoint;
            DPoint3d directionVector;
            if(!transform.isUniformScaleAndRotateAroundLine (&fixedPoint, &directionVector, &m_aspectFix, &m_scale))
                {
                m_aspectFix = 0;
                m_scale = 1;
                }
            else
                {
                m_aspectFix *= 180.0 / DTM_PYE;
                }
            }
        virtual ~TMTransformHelper()
            {
            }

        static TMTransformHelperP Create ()
            {
            return new TMTransformHelper ();
            }

        static TMTransformHelperP Create (TransformCR transform)
            {
            return new TMTransformHelper (transform);
            }

        bool GetTransformationFromDTM (TransformR transformation) const
            {
            transformation = m_transform;
            return m_isIdentity;
            }
        bool GetTransformationToDTM (TransformR transformation) const
            {
            transformation = m_transformInv;
            return m_isIdentity;
            }
        bool IsIdentity ()
            {
            return m_isIdentity;
            }
    };

    END_BENTLEY_TERRAINMODEL_NAMESPACE
