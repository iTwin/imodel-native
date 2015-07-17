/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/ImportPlugins/CivilDTMHelpers.h $
|    $RCSfile: CivilDTMHelpers.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/08/08 19:02:26 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ImagePP/all/h/IDTMFeatureArray.h>
#include <ImagePP/all/h/HPUArray.h>

#include <TerrainModel/Core/bcDTMClass.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE
namespace Plugin {

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CivilDTMWrapper
    {
private:
    Bentley::TerrainModel::BcDTMPtr m_pDTM;
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

    Bentley::TerrainModel::BcDTM&           Get                                () { return *m_pDTM; }      

    // NOTE: Return type should be const, but as their API is not const correct it would burden us to return const.
    Bentley::TerrainModel::BcDTM&           Get                                () const { return *m_pDTM; } 


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
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PointHandler
    {
    const Bentley::TerrainModel::BcDTM&     m_rDTM;
    const CivilImportedTypes::TypeIdList&   
                                    m_typeIDList;
    bool                            m_initialized;
    LinearFeatureTypeInfoList       m_typesInfo;  
    size_t                          m_maxPtCount;


public:
    typedef LinearFeatureTypeInfoList::const_iterator
                                    TypeInfoCIter;

    explicit                        PointHandler               (const Bentley::TerrainModel::BcDTM&                 dtm,
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
    const Bentley::TerrainModel::BcDTM&     m_rDTM;
    const CivilImportedTypes::TypeIdList&   
                                    m_typeIDList;

    bool                            m_initialized;
    LinearFeatureTypeInfoList       m_typesInfo;  

    size_t                          m_maxPtCount;
    size_t                          m_maxLinearCount;


public:
    typedef LinearFeatureTypeInfoList::const_iterator
                                    TypeInfoCIter;

    explicit                        LinearHandler              (const Bentley::TerrainModel::BcDTM&                 dtm,
                                                                const CivilImportedTypes::TypeIdList&       typeIDList);

    bool                            ComputeCounts              ();


    TypeInfoCIter                   TypesInfoBegin             () const;
    TypeInfoCIter                   TypesInfoEnd               () const;

    size_t                          GetMaxLinearCount          () const;
    size_t                          GetMaxPointCount           () const;

    bool                            Copy                       (TypeInfoCIter                   pi_typeInfoIter,
                                                                IDTMFeatureArray<DPoint3d>&     po_featureArray) const;
    };


typedef LinearHandler               TINAsLinearHandler;


} // END namespace Plugin
END_BENTLEY_MRDTM_NAMESPACE
