/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/DTMIterators.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMBaseDef.h>

TERRAINMODEL_TYPEDEFS (DTMMeshInfo)

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

struct DTMFeatureInfo
    {
    private: BcDTMP m_dtm;
    private: long m_feature;
    public: DTMFeatureInfo (BcDTMP dtm, long feature);
    public: BENTLEYDTM_EXPORT void GetFeaturePoints (bvector<DPoint3d>& points) const;
    public: BENTLEYDTM_EXPORT void GetFeatureInfo (DTMFeatureType& featureType, DTMFeatureId& featureId, DTMUserTag& userTag) const;
    public: BENTLEYDTM_EXPORT DTMFeatureType FeatureType () const;
    public: BENTLEYDTM_EXPORT DTMFeatureId FeatureId () const;
    public: BENTLEYDTM_EXPORT DTMUserTag UserTag () const;
    public: BENTLEYDTM_EXPORT long FeatureIndex () const;
    };

struct DTMFeatureEnumerator : RefCountedBase
    {
    struct iterator
        {
        iterator (const DTMFeatureEnumerator* p_vec, long pos, size_t nextSourcePos)
        : m_pos (pos)
        , m_nextSourcePos (nextSourcePos)
        , m_p_vec (p_vec)
            {
            }

        // these three methods form the basis of an iterator for use with 
        // a range-based for loop 
        bool
            operator!= (const iterator& other) const
            {
            return m_pos != other.m_pos;
            }

        // this method must be defined after the definition of IntVector 
        // since it needs to use it 
        BENTLEYDTM_EXPORT DTMFeatureInfo operator* () const;
        BENTLEYDTM_EXPORT DRange3d GetRange3d () const;
        BENTLEYDTM_EXPORT const iterator& operator++ ();

        private:
            long m_pos;
            size_t m_nextSourcePos;
            const DTMFeatureEnumerator *m_p_vec;
        };

    private: struct sortedFeatureId
        {
        DTMFeatureId featureId;
        long featureNum;

        static bool compare (sortedFeatureId& f1, sortedFeatureId& f2)
            {
            return f1.featureId < f2.featureId;
            }
        };

    private: mutable bvector<sortedFeatureId> m_sortedIds;
    private: mutable bvector<DTMFeatureId> m_sourceFeatureIds;

    private: BcDTMPtr m_dtm;
    private: BC_DTM_OBJ* m_dtmP;
    private: mutable bool m_isInitialized;
    private: bool m_sort;
    private: bool m_readSourceFeatures;
    private: DTMUserTag m_userTagLow;
    private: DTMUserTag m_userTagHigh;

    private: bool m_filterByFeatureId;
    private: DTMFeatureId m_featureIdFilter;
    private: enum class FeatureTypeFilter : UInt8 { Default, Include, Exclude };
    private: bool m_filterByFeatureType;
    private: bvector<FeatureTypeFilter> m_featureTypeFilter;
    private: bool m_excludeFeatureTypeByDefault;

    public: BENTLEYDTM_EXPORT DTMFeatureEnumerator (BcDTMCR dtm);
            //public: void Reset ();
    public: static BENTLEYDTM_EXPORT DTMFeatureEnumeratorPtr Create (BcDTMCR dtm);

    public: bool MoveNext (long& m_index, size_t& m_nextSourceFeatureIndex) const;
            //public: void GetFeaturePoints (bvector<DPoint3d>& points) const;
            //public: void GetFeatureInfo (DTMFeatureType& featureType, DTMFeatureId& featureId, DTMUserTag& userTag) const;

    private: void Initialize () const;
    private: BENTLEYDTM_EXPORT void InitializeForSorting () const;

    public: BENTLEYDTM_EXPORT iterator begin () const;
    public: BENTLEYDTM_EXPORT iterator end () const;

    public: bool GetSort () const
        {
        return m_sort;
        }
    public: void SetSort (bool value)
        {
        m_sort = value;
        InitializeForSorting ();
        }
    public: bool GetReadSourceFeatures () const
        {
        return m_readSourceFeatures;
        }
    public: void SetReadSourceFeatures (bool value)
        {
        m_readSourceFeatures = value;
        }

    public: void ClearFilterByUserTag ()
        {
        m_userTagLow = 0xffffffffffffffff;
        m_userTagHigh = 0;
        }
    public: bool GetUserTagFilterRange (DTMUserTag& low, DTMUserTag& high) const
        {
        low = m_userTagLow;
        high = m_userTagHigh;
        return m_userTagLow < m_userTagHigh;
        }
    public: void SetUserTagFilter(DTMUserTag userTag)
        {
        m_userTagLow = userTag;
        m_userTagHigh = userTag;
        }
    public: void SetUserTagFilterRange (DTMUserTag low, DTMUserTag high)
        {
        m_userTagLow = low;
        m_userTagHigh = high;
        }

    public: bool GetFilterByFeatureId () const
        {
        return m_filterByFeatureId;
        }
    public: void ClearFilterByFeatureId ()
        {
        m_filterByFeatureId = false;
        }
    public: DTMFeatureId GetFeatureIdFilter () const
        {
        return m_featureIdFilter;
        }
    public: void SetFeatureIdFilter (DTMFeatureId value)
        {
        m_filterByFeatureId = true;
        m_featureIdFilter = value;
        }

    public: void IncludeAllFeatures ()
        {
        m_featureTypeFilter.clear ();
        m_excludeFeatureTypeByDefault = false;
        }
    public: void ExcludeAllFeatures ()
        {
        m_featureTypeFilter.clear ();
        m_excludeFeatureTypeByDefault = true;
        }
    public: void IncludeFeature (DTMFeatureType featureType)
        {
        if (m_featureTypeFilter.size () < (unsigned int)featureType)
            m_featureTypeFilter.resize (1 + (unsigned int)featureType);
        m_featureTypeFilter[(unsigned int)featureType] = FeatureTypeFilter::Include;
        m_filterByFeatureType = true;
        }
    public: void ExcludeFeature (DTMFeatureType featureType)
        {
        if (m_featureTypeFilter.size () < (unsigned int)featureType)
            m_featureTypeFilter.resize (1 + (unsigned int)featureType);
        m_featureTypeFilter[(unsigned int)featureType] = FeatureTypeFilter::Exclude;
        m_filterByFeatureType = true;
        }

    };

struct DTMMeshEnumerator : RefCountedBase
    {
    public: enum class RegionMode
        {
        Normal = 0,
        NonRegion = 1,
        RegionUserTag = 2,
        RegionFeatureId = 3
        };
    public: struct iterator
        {
        iterator (const DTMMeshEnumerator* p_vec, long pos, long pos2) : m_pos (pos), m_pos2(pos2), m_p_vec (p_vec)
            {
            m_polyface = nullptr;
            }
        virtual ~iterator ()
            {
            if (m_polyface)
                delete m_polyface;
            }

        // these three methods form the basis of an iterator for use with 
        // a range-based for loop 
        bool operator!= (const iterator& other) const
            {
            return m_pos != other.m_pos && m_pos2 != other.m_pos2;
            }

        // this method must be defined after the definition of IntVector 
        // since it needs to use it 
        BENTLEYDTM_EXPORT PolyfaceQueryP operator* () const;
        BENTLEYDTM_EXPORT DRange3d GetRange () const;
        BENTLEYDTM_EXPORT const iterator& operator++ ();

        private:
            long m_pos;
            long m_pos2;
            const DTMMeshEnumerator *m_p_vec;
            mutable PolyfaceQueryCarrier* m_polyface;
        };

    private: BcDTMPtr m_dtm;
    private: BC_DTM_OBJ* m_dtmP;

    private: long maxTriangles;        /* ==> Maximum Number Of Triangles To Load Per Call       */
    private: long vectorOption;        /* ==> Vector Option <1=Surface Derivatives,2=Averaged Triangle Surface Normals> */
    private: double zAxisFactor;       /* ==> Factor To Exaggerate The z Axis default value 1.0  */
    private: DTMFenceParams m_fence;

    private: mutable long m_pointMark;
    private: mutable bool voidsInDtm;
    private: mutable long startPnt, lastPnt, leftMostPnt;
    private: mutable BlockedVector<int> meshFaces;
    private: mutable BlockedVector<DPoint3d> meshPoints;
    private: mutable BlockedVector<DVec3d> meshNormals;
    private: mutable BcDTMPtr clipDtm;
    private: mutable BC_DTM_OBJ* clipDtmP;
    private: mutable bool m_initialized;
    private: mutable DRange3d m_range;
    private: mutable bool m_calcRange = false;
    private: bool m_tilingMode;
    private: bool m_useRealPointIndexes = false;
    private: RegionMode m_regionMode = RegionMode::Normal;
    private: DTMUserTag m_regionUserTag;
    private: DTMFeatureId m_regionFeatureId;
    private: mutable std::vector<bool> m_regionPointMask;
    private: DTMStatusInt Initialize () const;
    private: bool MoveNext (long& pnt1, long& pnt2) const;
    private: void ScanAndMarkRegions () const;
    private: void ScanAndMarkRegion (long featureId) const;

    protected: BENTLEYDTM_EXPORT DTMMeshEnumerator (BcDTMR dtm);
    public: BENTLEYDTM_EXPORT static DTMMeshEnumeratorPtr Create (BcDTMR dtm);
    public: BENTLEYDTM_EXPORT virtual ~DTMMeshEnumerator ();
    public: BENTLEYDTM_EXPORT void Reset ();
    public: BENTLEYDTM_EXPORT iterator begin () const;
    public: BENTLEYDTM_EXPORT iterator end () const;
    public: BENTLEYDTM_EXPORT DRange3d DTMMeshEnumerator::GetRange () const;


    public: void SetUseRealPointIndexes(bool value)
        {
        m_useRealPointIndexes = value;
        Reset();
        }
    public: bool GetUseRealPointIndexes() const
        {
        return m_useRealPointIndexes;
        }
    public: void SetFilterRegionByUserTag (DTMUserTag value)
        {
        m_regionMode = RegionMode::RegionUserTag;
        m_regionUserTag = value;
        Reset();
        }

    public: DTMUserTag GetFilterRegionByUserTag () const
        {
        return m_regionUserTag;
        }

    public: void SetFilterRegionByFeatureId (DTMFeatureId value)
        {
        m_regionMode = RegionMode::RegionFeatureId;
        m_regionFeatureId = value;
        Reset();
        }

    public: DTMFeatureId  GetFilterRegionByFeatureId () const
        {
        return m_regionFeatureId;
        }

    public: void SetExcludeAllRegions ()
        {
        m_regionMode = RegionMode::NonRegion;
        Reset();
        }

    public: bool GetExcludeAllRegions ()
        {
        return m_regionMode == RegionMode::NonRegion;
        }

    public: bool GetExcludeAllRegions () const
        {
        return m_regionMode == RegionMode::NonRegion;
        }

    public: void SetFence (DTMFenceParams& fence)
        {
        m_fence = DTMFenceParams (fence.fenceType, fence.fenceOption, fence.points, fence.numPoints);
        Reset();
        }
    public: void SetMaxTriangles (int value)
        {
        maxTriangles = value;
        Reset();
        }
    public: void SetTilingMode (bool value)
        {
        m_tilingMode = value;
        Reset();
        }

    };

// ToDo FeatureEnumerator
//
// --- Allow Browsing features that are TinErrors.
//
// Create a points browsing enumerator.
// Create a face browsing enumerator.
// Create a Random Points browsing.
//
// For the following call the old functions and return the bvector<>
//  --- DuplicatePoints
//  --- CrossingFeatures
//
//  --- Contours
//
// Other Enumerators
//  --- Meshes/Triangles with Shading.
//  --- Ponds  
//  --- tinPointsVisibility, tinLinesVisibility, RadialView, RegionView
//  --- DrainageFeatures



END_BENTLEY_TERRAINMODEL_NAMESPACE