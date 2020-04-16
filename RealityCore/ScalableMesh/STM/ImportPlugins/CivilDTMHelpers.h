/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#ifndef VANCOUVER_API
#include "../IDTMFeatureArray.h"
#include "../HPUArray.h"
#else
#include <ImagePP/all/h/IDTMFeatureArray.h>
#include <ImagePP/all/h/HPUArray.h>
#endif

#include <TerrainModel/Core/bcDTMClass.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
namespace Plugin {

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CivilDTMWrapper
    {
private:
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr m_pDTM;
    DTMState m_state;
    // Disable copy
                                    CivilDTMWrapper                    (const CivilDTMWrapper&);
    CivilDTMWrapper&                operator=                          (const CivilDTMWrapper&);

public:
    explicit                        CivilDTMWrapper                    (BcDTMR dtm) 
        :   m_pDTM(&dtm),
            m_state(DTMState::TinError)
        {
        // m_state used to be set to one of the value below. Now they come from DTMState.
        //  CIVIL_DTMState::DATA = 1,
        //  CIVIL_DTMState::TIN = 2,
        //  CIVIL_DTMState::MIXED = 3,
        //  CIVIL_DTMState::INVALID = 4,
        bcdtmUtility_getDtmStateDtmObject (m_pDTM->GetTinHandle(), &m_state);
        }
                                    ~CivilDTMWrapper                   ()
        {
        Close();
        }

    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM&           Get                                () { return *m_pDTM; }      

    // NOTE: Return type should be const, but as their API is not const correct it would burden us to return const.
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM&           Get                                () const { return *m_pDTM; } 


    DTMState GetState                           () const
        {
        return m_state;
        }

    bool                            HasTIN                             () const
        {
        return DTMState::DuplicatesRemoved == m_state || DTMState::Tin == m_state;
        }

    void                            Close                              ()
        {
        m_pDTM = NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct LinearFeatureTypeInfo
    {
    explicit                    LinearFeatureTypeInfo          (DTMFeatureType          pi_TypeID, 
                                                                size_t                  pi_FeatureCount,
                                                                size_t                  pi_PointCount) 
        :   m_typeID(pi_TypeID), m_linearCount(pi_FeatureCount),  m_pointCount(pi_PointCount)
        {}

    DTMFeatureType                  m_typeID;
    size_t                          m_linearCount;
    size_t                          m_pointCount;
    };

typedef vector<LinearFeatureTypeInfo>        
                                    LinearFeatureTypeInfoList;

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Mathieu.St-Pierre   09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct MeshFeatureTypeInfo 
    {
    explicit                    MeshFeatureTypeInfo(size_t                  pi_PointCount,
                                                    size_t                  pi_ptIndicesCount)
        : m_pointCount(pi_PointCount), m_ptIndicesCount(pi_ptIndicesCount)
    {}

    size_t                          m_pointCount;
    size_t                          m_ptIndicesCount;
    };

typedef vector<MeshFeatureTypeInfo>MeshFeatureTypeInfoList;

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class CivilImportedTypes
    {
    explicit                        CivilImportedTypes         ();

public:

    static const CivilImportedTypes&
                                    GetInstance                ();

    typedef vector<DTMFeatureType>  TypeIdList;
    TypeIdList                      points;
    TypeIdList                      linears;
    TypeIdList                      tinLinears;
    TypeIdList                      mesh;
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PointHandler
    {
    const BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM&     m_rDTM;
    const CivilImportedTypes::TypeIdList&   
                                    m_typeIDList;
    bool                            m_initialized;
    LinearFeatureTypeInfoList       m_typesInfo;  
    size_t                          m_maxPtCount;


public:
    typedef LinearFeatureTypeInfoList::const_iterator
                                    TypeInfoCIter;

    explicit                        PointHandler               (const BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM&                 dtm,
                                                                const CivilImportedTypes::TypeIdList&       typeIDList);

    bool                            ComputeCounts              ();


    TypeInfoCIter                   TypesInfoBegin             () const;
    TypeInfoCIter                   TypesInfoEnd               () const;

    size_t                          GetMaxPointCount           () const;

    bool                            Copy                       (TypeInfoCIter                   pi_typeInfoIter,
                                                                HPU::Array<DPoint3d>&           po_pointArray) const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LinearHandler
    {
    const BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM&     m_rDTM;
    const CivilImportedTypes::TypeIdList&   
                                    m_typeIDList;

    bool                            m_initialized;
    LinearFeatureTypeInfoList       m_typesInfo;  

    size_t                          m_maxPtCount;
    size_t                          m_maxLinearCount;
    size_t                          m_boundaryPtCount;

public:
    typedef LinearFeatureTypeInfoList::const_iterator
                                    TypeInfoCIter;

    explicit                        LinearHandler              (const BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM&                 dtm,
                                                                const CivilImportedTypes::TypeIdList&       typeIDList);

    bool                            ComputeCounts              ();


    TypeInfoCIter                   TypesInfoBegin             () const;
    TypeInfoCIter                   TypesInfoEnd               () const;

    size_t                          GetMaxLinearCount          () const;
    size_t                          GetMaxPointCount           () const;
    size_t                          GetBoundaryPointCount      () const;

    bool                            Copy                       (TypeInfoCIter                   pi_typeInfoIter,
                                                                IDTMFeatureArray<DPoint3d>&     po_featureArray) const;
    };

typedef LinearHandler               TINAsLinearHandler;

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Mathieu.St-Pierre   09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
class MeshHandler
    {
    const BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM&     m_rDTM;
    const CivilImportedTypes::TypeIdList&
        m_typeIDList;
    bool                    m_initialized;
    MeshFeatureTypeInfoList m_typesInfo;
    size_t                  m_maxPtCount;
    size_t                  m_maxPtIndicesCount;


public:
    /*
    typedef MeshFeatureTypeInfoList::const_iterator
        TypeInfoCIter;
        */

    explicit                        MeshHandler(const BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM& dtm,
                                                const CivilImportedTypes::TypeIdList&              typeIDList);

    bool                            ComputeCounts();

    /*
    TypeInfoCIter                   TypesInfoBegin() const;
    TypeInfoCIter                   TypesInfoEnd() const;
    */

    size_t                          GetMaxPointCount() const;
    size_t                          GetMaxPtIndicesCount() const;

    bool                            Copy(//TypeInfoCIter         pi_typeInfoIter,
                                         HPU::Array<DPoint3d>& po_pointArray, 
                                         HPU::Array<int32_t>&  po_ptIndicesArray) const;
    };


} // END namespace Plugin
END_BENTLEY_SCALABLEMESH_NAMESPACE
