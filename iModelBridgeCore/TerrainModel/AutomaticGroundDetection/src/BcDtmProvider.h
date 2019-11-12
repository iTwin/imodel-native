/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "IDtmProvider.h"
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/DTMIterators.h>


GROUND_DETECTION_TYPEDEF(BcDtmProvider)

BEGIN_GROUND_DETECTION_NAMESPACE


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     12/2015
+===============+===============+===============+===============+===============+======*/
struct BcDtmProvider : public IDtmProvider
{
public:
     static BcDtmProviderPtr Create();
     static BcDtmProviderPtr CreateFrom(BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM& bcDtm);
     static BcDtmProviderPtr CreateFrom(WChar* filename, WChar* name = nullptr);

     BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr GetBcDTM();

private:
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr     m_pBcDtm;
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMMeshPtr m_pMesh;

    BcDtmProvider();
    BcDtmProvider(BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM& bcDtm);
    ~BcDtmProvider();

    //=======================================================================================
    //! @bsiclass
    //===============+===============+===============+===============+===============+=======
    struct      BcDtmProviderIteratorImpl : IDtmProvider::IDtmProviderIteratorImpl
        {
        public:
            friend BcDtmProvider;

        private:
            BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMMeshPtr m_pMesh;
            size_t                              m_currentFace;
            //mutable because it will be filled during _GetCurrent
            mutable Triangle                    m_current;
            mutable bool                        m_currentIsDirty;

            BcDtmProviderIteratorImpl(BcDTMMesh& mesh, size_t currentItr);
            ~BcDtmProviderIteratorImpl();

            virtual bool          _IsDifferent(IDtmProvider::IDtmProviderIteratorImpl const& rhs) const override;
            virtual void          _MoveToNext() override;
            virtual ReturnType&   _GetCurrent() const override;
            virtual bool          _IsAtEnd() const override;
        };

    typedef  vector<int>                           T_IndexCollection;
    typedef  pair<int, T_IndexCollection>          T_PairIndexAndIndexCollection;
    typedef  vector<T_PairIndexAndIndexCollection> T_PairIndexesCollection;
    typedef  vector<T_PairIndexesCollection>       T_EdgeMap;

    static BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr     LoadTerrainModel(WChar* filename, WChar* name);
    void                AddFaceToEdge(T_EdgeMap& edgeMap, int idx1, int idx2, int i) const;


    virtual  IDtmProvider::const_iterator _begin() const override;
    virtual  IDtmProvider::const_iterator _end() const override;
    virtual  size_t                       _size() const override;

    virtual size_t      _GetTriangleCount() const override;
    virtual size_t      _GetPointCount() const override;
    virtual void        _AddPoint(DPoint3d const& point) override;
    virtual long        _ComputeTriangulation() override;
    virtual bool        _FindNearestTriangleDistanceFromPoint(Triangle* pTri, double& distance, DPoint3d const& point) const override;
    virtual void        _ComputeStatisticsFromDTM(DiscreetHistogram& angleStats, DiscreetHistogram& heightStats) const override;    
    virtual StatusInt   _GetDTMPoints(DPoint3d* pPoints) const override;

}; // BcDtmProvider

END_GROUND_DETECTION_NAMESPACE
