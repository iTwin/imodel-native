/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/CivilDTMFileImporter.cpp $
|    $RCSfile: CivilDTMFileImporter.cpp,v $
|   $Revision: 1.46 $
|       $Date: 2011/11/09 18:11:03 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"

#include <ScalableMesh/Import/Plugin/InputExtractorV0.h>
#include <ScalableMesh/Import/Plugin/SourceV0.h>

#include <ScalableMesh/Plugin/IScalableMeshPolicy.h>

#include <ScalableMesh/Plugin/IScalableMeshCivilDTMSource.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>

#include <STMInternal/Foundations/PrivateStringTools.h>


#include "CivilDTMHelpers.h"


USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_SCALABLEMESH

using namespace BENTLEY_NAMESPACE_NAME::ScalableMesh::Plugin;

namespace { //BEGIN UNAMED NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class CivilDTMSource : public SourceMixinBase<CivilDTMSource>
    {
private:
    friend class                    CivilDTMFileCreator;

    CivilDTMWrapper                 m_dtm;
    PointHandler                    m_pointHandler;
    LinearHandler                   m_linearHandler;
    TINAsLinearHandler              m_tinLinearHandler;
    GCS                             m_gcs;
    

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                 () override
        {
        m_dtm.Close();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor          _CreateDescriptor  () const override
        {
        DRange3d range;

        m_dtm.Get().GetRange(range);

        DataTypeSet storedTypes;
        switch (m_dtm.GetState())
            {
            case DTMState::PointsSorted:
                storedTypes.push_back(LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create());
                storedTypes.push_back(PointType3d64fCreator().Create());
                break;
            case DTMState::DuplicatesRemoved:
            case DTMState::Tin:
                storedTypes.push_back(TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator().Create());
                storedTypes.push_back(LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create());
                storedTypes.push_back(PointType3d64fCreator().Create());
                break;
            default:
                assert(!"Unexpected!");
                break;
            }

        ScalableMeshData data = ScalableMeshData::GetNull();
        data.AddExtent(range);

        return ContentDescriptor
            (
            L"",
            ILayerDescriptor::CreateLayerDescriptor(L"",
                            storedTypes,
                            m_gcs, 
                            &range,
                            data)
            );
        }


    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*             _GetType                           () const override
        {
        return L"Civil DTM";
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        CivilDTMSource                         (BcDTMR             dtm,
                                                                            const GCS&          gcs) 
        :   m_dtm(dtm),
            m_pointHandler(dtm, CivilImportedTypes::GetInstance().points),
            m_linearHandler(dtm, CivilImportedTypes::GetInstance().linears),
            m_tinLinearHandler(dtm, CivilImportedTypes::GetInstance().tinLinears),
            m_gcs(gcs)
        {
        }

    BcDTM&                         GetDTM                 () { return m_dtm.Get(); }   

    PointHandler&                   GetPointHandler        () { return m_pointHandler; }
    LinearHandler&                  GetLinearHandler       () { return m_linearHandler; }
    TINAsLinearHandler&             GetTINLinearHandler    () { return m_tinLinearHandler; }
    };


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class CivilDTMFileCreator : public LocalFileSourceCreatorBase
    {

    enum FileType
        {
        FILE_TYPE_BCDTM,
        FILE_TYPE_TIN,
        FILE_TYPE_DTM,
        FILE_TYPE_DAT,
        FILE_TYPE_UNSUPPORTED,
        };


    static FileType                 ExtractFileType            (const LocalFileSourceRef&   sourceRef)
        {
        // bcdtm support
        if (sourceRef.HasExtension(L"dat"))
            return FILE_TYPE_DAT; // NTERAY: Isn't there any magic numbers that can be found?

        // Read magic number from file
        static const streamsize MAX_MAGIC_NUMBER_LGT = 4;
        char fileMagicNumber[MAX_MAGIC_NUMBER_LGT];
            {
            ifstream inFile(sourceRef.GetPathCStr(), ios::binary);
            if (!inFile.good())
                return FILE_TYPE_UNSUPPORTED;

            inFile.read(fileMagicNumber, MAX_MAGIC_NUMBER_LGT);
            }

        // dtm support
            {
            static const char DTM_MAGIC_NUMBER[] = {0x44, 0x54, 0x4d};
            static const streamsize DTM_MAGIC_NUMBER_LGT = 3;
            if (0 == memcmp(fileMagicNumber, DTM_MAGIC_NUMBER, DTM_MAGIC_NUMBER_LGT))
                return FILE_TYPE_DTM;
            }

        // tin support
            {
            static const char TIN_MAGIC_NUMBER[] = {0x54, 0x4b, 0x50, 0x47};
            static const streamsize TIN_MAGIC_NUMBER_LGT = 4;

            if (0 == memcmp(fileMagicNumber, TIN_MAGIC_NUMBER, TIN_MAGIC_NUMBER_LGT))
                return FILE_TYPE_TIN;
            }

        // bcdtm support
            {
            static const char BCDTM_MAGIC_NUMBER[] = {0x4d, 0x54};
            static const streamsize BCDTM_MAGIC_NUMBER_LGT = 2;
   
            if (0 == memcmp(fileMagicNumber, BCDTM_MAGIC_NUMBER, BCDTM_MAGIC_NUMBER_LGT) &&
                sourceRef.HasExtension(L"bcdtm")) // Ensure that we're not big endian IDTM file as magic number is the same.
                return FILE_TYPE_BCDTM;
            }

        return FILE_TYPE_UNSUPPORTED;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BcDTMPtr                  CreateDTMFromFilePath      (const LocalFileSourceRef&   sourceRef)
        {
        const FileType fileType = ExtractFileType(sourceRef);

        BcDTMPtr pDTM = 0;

        switch (fileType)
            {
            case FILE_TYPE_TIN:
            case FILE_TYPE_BCDTM:
                pDTM = BcDTM::CreateFromTinFile(sourceRef.GetPathCStr());
                break;
            case FILE_TYPE_DTM:
                {                
                InroadsImporterPtr inroadsImporterPtr(InroadsImporter::Create (sourceRef.GetPathCStr()));

                if (inroadsImporterPtr != 0)
                    {
                    ImportedTerrainList terrainList(inroadsImporterPtr->ImportTerrains ());                    

                    if (terrainList.size() > 0)
                        {
                        assert(terrainList.size() == 1);
                        pDTM = terrainList[0].GetTerrain();
                        }                    
                    }                    
                }
                break;

            case FILE_TYPE_DAT:
                pDTM = BcDTM::CreateFromGeopakDatFile(sourceRef.GetPathCStr());
                break;
            }

        return pDTM;
        }


    virtual ExtensionFilter         _GetExtensions                         () const override
        {
        return L"*.dtm;*.tin;*.dat;*.bcdtm";
        }    

    virtual bool                    _Supports                              (const LocalFileSourceRef&   pi_rSourceRef) const override
        {
        return FILE_TYPE_UNSUPPORTED != ExtractFileType(pi_rSourceRef);
        }

    virtual SourceBase*             _Create                                (const LocalFileSourceRef&   pi_rSourceRef,
                                                                            Log&                        pi_warningLog) const override
        {
        BcDTMPtr pDTM = CreateDTMFromFilePath(pi_rSourceRef);
        if (pDTM.IsNull())
            throw FileIOException(LocalFileError::S_ERROR_COULD_NOT_OPEN);

        //CivilDTMSource will take ownership of pDTM
        return new CivilDTMSource(*pDTM, GCS::GetNull());
        }
    };

const SourceRegistry::AutoRegister<CivilDTMFileCreator> s_RegisterCivilDTMFile;



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class CivilDTMPointExtractor : public InputExtractorBase
    {
private:
    // Dimension groups definition
    enum 
        {
        DG_XYZ,
        DG_QTY,
        };

    const PointHandler&             m_rPointHandler;
    PointHandler::TypeInfoCIter     m_typeInfoIter;
    PointHandler::TypeInfoCIter     m_typeInfoEnd;

    PODPacketProxy<DPoint3d>        m_pointPacket;
    HPU::Array<DPoint3d>            m_ptArray;

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                            (PacketGroup&     pi_rRawEntities) override
        {
        m_pointPacket.AssignTo(pi_rRawEntities[DG_XYZ]);
        m_ptArray.WrapEditable(m_pointPacket.Edit(), 0, m_pointPacket.GetCapacity());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                              () override
        {
        m_ptArray.Clear();

        if (m_typeInfoIter == m_typeInfoEnd ||
            !m_rPointHandler.Copy(m_typeInfoIter, m_ptArray))
            {
            m_pointPacket.SetSize(0);
            return;
            }

        m_pointPacket.SetSize(m_ptArray.GetSize());
        assert(m_typeInfoIter->m_pointCount == m_pointPacket.GetSize());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                              () override
        {
        if (m_typeInfoIter == m_typeInfoEnd)
            return false;

        return ++m_typeInfoIter < m_typeInfoEnd;
        }
public:
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        CivilDTMPointExtractor             (const PointHandler& pi_rPointHandler) 
        :   m_rPointHandler(pi_rPointHandler),
            m_typeInfoIter(pi_rPointHandler.TypesInfoBegin()),
            m_typeInfoEnd(pi_rPointHandler.TypesInfoEnd())
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class CivilDTMPointExtractorCreator : public InputExtractorCreatorMixinBase<CivilDTMSource>
    {

    virtual bool                                _Supports                          (const DataType&                 pi_rType) const override
        {
        return pi_rType.GetFamily() == PointTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (CivilDTMSource&                 sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection) const override
        {
        if (!sourceBase.GetPointHandler().ComputeCounts())
            return RawCapacities(0);

        return RawCapacities (sourceBase.GetPointHandler().GetMaxPointCount()*sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (CivilDTMSource&                 sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const override
        {
        if (!sourceBase.GetPointHandler().ComputeCounts())
            return 0;

        return new CivilDTMPointExtractor(sourceBase.GetPointHandler());
        }

    };

const ExtractorRegistry::AutoRegister<CivilDTMPointExtractorCreator> s_RegisterCivilDTMPointExtractor;




/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class CivilDTMLinearExtractor : public InputExtractorBase
    {
private:

    // Dimension groups definition
    enum 
        {
        DG_Header,
        DG_XYZ,
        DG_QTY,
        };

    const LinearHandler&            m_rLinearFeatureHandler;
    
    LinearHandler::TypeInfoCIter    m_typeInfoIter;
    LinearHandler::TypeInfoCIter    m_typeInfoEnd;

    PODPacketProxy<ISMStore::FeatureHeader>
                                    m_headerPacket;
    PODPacketProxy<DPoint3d>        m_pointPacket;

    IDTMFeatureArray<DPoint3d>      m_featureArray;

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                            (PacketGroup&     pi_rRawEntities) override
        {
        m_headerPacket.AssignTo(pi_rRawEntities[DG_Header]);
        m_pointPacket.AssignTo(pi_rRawEntities[DG_XYZ]);
        m_featureArray.EditHeaders().WrapEditable(m_headerPacket.Edit(), 0, m_headerPacket.GetCapacity());
        m_featureArray.EditPoints().WrapEditable(m_pointPacket.Edit(), 0, m_pointPacket.GetCapacity());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                              () override
        {
        m_featureArray.Clear();

        if (m_typeInfoIter == m_typeInfoEnd ||
            !m_rLinearFeatureHandler.Copy(m_typeInfoIter, m_featureArray))
            {
            m_headerPacket.SetSize(0);
            m_pointPacket.SetSize(0);
            return;
            }

        m_headerPacket.SetSize(m_featureArray.GetHeaders().GetSize());
        m_pointPacket.SetSize(m_featureArray.GetPoints().GetSize());

        assert(m_typeInfoIter->m_linearCount == m_headerPacket.GetSize());
        assert(m_typeInfoIter->m_pointCount == m_pointPacket.GetSize());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                              () override
        {
        if (m_typeInfoIter == m_typeInfoEnd)
            return false;

        return ++m_typeInfoIter < m_typeInfoEnd;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        CivilDTMLinearExtractor (const LinearHandler& pi_rLinearFeatureHandler) 
        :   m_rLinearFeatureHandler(pi_rLinearFeatureHandler),
            m_typeInfoIter(m_rLinearFeatureHandler.TypesInfoBegin()),
            m_typeInfoEnd(m_rLinearFeatureHandler.TypesInfoEnd())
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class CivilDTMLinearExtractorCreator : public InputExtractorCreatorMixinBase<CivilDTMSource>
    {
    virtual bool                                _Supports                          (const DataType&         pi_rType) const override
        {
        return pi_rType.GetFamily() == LinearTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (CivilDTMSource&                 sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection) const override
        {
        if (!sourceBase.GetLinearHandler().ComputeCounts())
            return RawCapacities(0, 0);

        return RawCapacities (sourceBase.GetLinearHandler().GetMaxLinearCount()*sizeof(ISMStore::FeatureHeader),
                              sourceBase.GetLinearHandler().GetMaxPointCount()*sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (CivilDTMSource&                 sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const override
        {
        if (!sourceBase.GetLinearHandler().ComputeCounts())
            return 0;

        return new CivilDTMLinearExtractor(sourceBase.GetLinearHandler());
        }
    };

const ExtractorRegistry::AutoRegister<CivilDTMLinearExtractorCreator> s_RegisterCivilDTMLinearExtractor;



/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class CivilDTMTINLinearExtractorCreator : public InputExtractorCreatorMixinBase<CivilDTMSource>
    {
    virtual bool                                _Supports                          (const DataType&                 pi_rType) const override
        {
        return pi_rType.GetFamily() == TINTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (CivilDTMSource&                 sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        if (!sourceBase.GetTINLinearHandler().ComputeCounts())
            return RawCapacities(0, 0);

        return RawCapacities (sourceBase.GetTINLinearHandler().GetMaxLinearCount()*sizeof(ISMStore::FeatureHeader),
                              sourceBase.GetTINLinearHandler().GetMaxPointCount()*sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (CivilDTMSource&                 sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                            log) const override
        {
        if (!sourceBase.GetTINLinearHandler().ComputeCounts())
            return 0;

        // Reuse linear extractor as it does exactly what is required
        return new CivilDTMLinearExtractor(sourceBase.GetTINLinearHandler());
        }
    };

const ExtractorRegistry::AutoRegister<CivilDTMTINLinearExtractorCreator> s_RegisterCivilDTMTINLinearExtractor;

} //END UNAMED NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
namespace Plugin { namespace V0 {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceBase*                                     CreateCivilDTMSource                   (BcDTMR                     iBcDTM, 
                                                                                        Log&                        log)
    {
    return new CivilDTMSource(iBcDTM, GCS::GetNull());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceBase*                                     CreateCivilDTMSource                   (BcDTMR                     iBcDTM, 
                                                                                        const GCS&                  gcs,
                                                                                        Log&                        log)
    {
    return new CivilDTMSource(iBcDTM, gcs);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorBase*                             CreateCivilDTMPointExtractor           (const PointHandler&         handler)
    {
    return new CivilDTMPointExtractor(handler);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorBase*                             CreateCivilDTMLinearExtractor          (const LinearHandler&        handler)
    {
    return new CivilDTMLinearExtractor(handler);
    }

}} // END namespace Plugin::V0
END_BENTLEY_SCALABLEMESH_NAMESPACE
