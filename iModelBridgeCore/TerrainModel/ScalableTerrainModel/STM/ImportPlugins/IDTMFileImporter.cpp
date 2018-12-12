/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/ImportPlugins/IDTMFileImporter.cpp $
|    $RCSfile: IDTMFileImporter.cpp,v $
|   $Revision: 1.44 $
|       $Date: 2012/01/27 16:45:33 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableTerrainModelPCH.h>


#include <ScalableTerrainModel/Import/Plugin/InputExtractorV0.h>
#include <ScalableTerrainModel/Import/Plugin/SourceV0.h>

#include <ScalableTerrainModel/Plugin/IMrDTMPolicy.h>

#include <ScalableTerrainModel/Import/DataTypeDescription.h>

#include <ScalableTerrainModel/Type/IMrDTMPoint.h>
#include <ScalableTerrainModel/Type/IMrDTMLinear.h>
#include <ScalableTerrainModel/Plugin/IMrDTMCivilDTMSource.h>
#include <ScalableTerrainModel/Plugin/IMrDTMSTMSource.h>
#include <STMInternal/GeoCoords/WKTUtils.h>

USING_NAMESPACE_BENTLEY_MRDTM_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY_MRDTM

using namespace IDTMFile;

namespace { //BEGIN UNAMED NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class IDTMSource : public SourceMixinBase<IDTMSource>
    {
public:
    const File&                     GetFile                () const {return *m_pFile;}

private:
    friend class                    IDTMFileCreator;

    File::Ptr                       m_pFile;
    ContentDescriptor               m_contentDesc;

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        IDTMSource             (const File::Ptr&            filePtr,
                                                            const ContentDescriptor&    contentDesc)
        :   m_pFile(filePtr),
            m_contentDesc(contentDesc)

        {
        assert(0 != m_pFile);
        assert(0 < m_contentDesc.GetLayerCount());
        }


    


    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                 () override
        {
        m_pFile = 0;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor        _CreateDescriptor      () const override
        {
        return m_contentDesc;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*         _GetType                   () const override
        {
        return L"STM";
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class IDTMFileCreator : public LocalFileSourceCreatorBase
    {

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ExtensionFilter         _GetExtensions                         () const override
        {
        return L"*.mrdtm;*.stm";
        }    

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Supports                              (const LocalFileSourceRef&   pi_rSourceRef) const override
        {
        static const char BIG_ENDIAN_MAGIC_NUMBER[] = {0x54, 0x4d, 0x2b, 0x00, 0x08};
        static const char LITTLE_ENDIAN_MAGIC_NUMBER[] = {0x4d, 0x54, 0x2b, 0x00, 0x08};

        static const streamsize MAGIC_NUMBER_LGT = 5;

        char fileMagicNumber[MAGIC_NUMBER_LGT];

        // Read magic number from file
            {
            ifstream inFile(pi_rSourceRef.GetPath().c_str(), ios::binary);
            if (!inFile.good())
                return false;

            inFile.read(fileMagicNumber, MAGIC_NUMBER_LGT);
            }

        const bool isIDTMFile = (0 == memcmp(fileMagicNumber, BIG_ENDIAN_MAGIC_NUMBER, MAGIC_NUMBER_LGT) ||
                                 0 == memcmp(fileMagicNumber, LITTLE_ENDIAN_MAGIC_NUMBER, MAGIC_NUMBER_LGT));

        return isIDTMFile;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual SourceBase*             _Create                                (const LocalFileSourceRef&   pi_rSourceRef,
                                                                            Log&                 pi_warningLog) const override
        {
        File::Ptr filePtr(File::Open(pi_rSourceRef.GetPathCStr(), HFC_SHARE_READ_WRITE | HFC_READ_ONLY));

        return Create(filePtr, pi_warningLog);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static const DataType*          GetLinearTypeFor                       (FeatureHeaderTypeID         headerType,
                                                                            PointTypeID                 pointType)
        {
        typedef pair<FeatureHeaderTypeID, PointTypeID>  TypeMappingKey;
        typedef map<TypeMappingKey, DataType>           TypeMapping;
        typedef TypeMapping::value_type                 TypePair;

        static const TypePair PAIRS[] = 
            {
            make_pair(make_pair(FEATURE_HEADER_TYPE_FEATURE, POINT_TYPE_XYZf64), LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create()),
            make_pair(make_pair(FEATURE_HEADER_TYPE_FEATURE, POINT_TYPE_XYZMf64), LinearTypeTi32Pi32Pq32Gi32_3d64fM64fCreator().Create()),
            };

        static const size_t PAIRS_QTY = sizeof(PAIRS) / sizeof(PAIRS[0]);        
        static const TypeMapping MAP(PAIRS, PAIRS + PAIRS_QTY);

        TypeMapping::const_iterator foundIt = MAP.find(make_pair(headerType, pointType));
        if (MAP.end() == foundIt)
            {
            assert(!"Point type not found. Add to mapping!");
            return 0;
            }

        return &foundIt->second;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static const DataType*          GetPointTypeFor                        (PointTypeID                 pointType)
        {
        typedef PointTypeID                             TypeMappingKey;
        typedef map<TypeMappingKey, DataType>           TypeMapping;
        typedef TypeMapping::value_type                 TypePair;

        static const TypePair PAIRS[] = 
            {
            make_pair(POINT_TYPE_XYZf64, PointType3d64fCreator().Create()),
            make_pair(POINT_TYPE_XYZMf64, PointType3d64fM64fCreator().Create()),
            };

        static const size_t PAIRS_QTY = sizeof(PAIRS) / sizeof(PAIRS[0]);        
        static const TypeMapping MAP(PAIRS, PAIRS + PAIRS_QTY);

        TypeMapping::const_iterator foundIt = MAP.find(pointType);
        if (MAP.end() == foundIt)
            {
            assert(!"Point type not found. Add to mapping!");
            return 0;
            }

        return &foundIt->second;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool                     DescribeContent                        (const File&                 file,
                                                                            ContentDescriptor&          contentDesc)
        {
        bool success = true;

        for (File::RootDir::LayerDirCIter layerIt = file.GetRootDir()->LayerDirsBegin(), 
             layersEnd = file.GetRootDir()->LayerDirsEnd();
             layerIt != layersEnd;
             ++layerIt)
            {
            const LayerDir* layerDirP = layerIt->Get();
            if (0 == layerDirP)
                {
                success = false;
                continue;
                }

            FeatureHeaderTypeID linearHeaderType = layerDirP->GetFeatureHeaderType();
            PointTypeID pointType = layerDirP->GetFeaturePointType();

            const size_t pointDirCount = layerDirP->CountPointOnlyUniformFeatureDirs();
            const size_t featureDirCount = (layerDirP->CountUniformFeatureDirs() - pointDirCount) + layerDirP->CountMixedFeatureDirs();

            DataTypeSet dataTypes;

            if (0 < featureDirCount)
                {
                const DataType* linearTypeP = GetLinearTypeFor(linearHeaderType, pointType);
                if (0 != linearTypeP)
                    dataTypes.push_back(*linearTypeP);
                }

            if (0 < pointDirCount)
                {
                const DataType* pointTypeP = GetPointTypeFor(pointType);
                if (0 != pointTypeP)
                    dataTypes.push_back(*pointTypeP);
                }

            GCS gcs(GetGCSFactory().Create(Unit::GetMeter()));

            if (layerDirP->HasWkt())
                {
                HCPWKT wkt(layerDirP->GetWkt());
                success &= !wkt.IsEmpty();
                
                WString wktStr(wkt.GetCStr());

                IDTMFile::WktFlavor fileWktFlavor = GetWKTFlavor(&wktStr, wktStr);                

                BaseGCS::WktFlavor baseGcsWktFlavor;
    
                bool result = MapWktFlavorEnum(baseGcsWktFlavor, fileWktFlavor);

                assert(result);    

                GCSFactory::Status gcsCreateStatus = GCSFactory::S_SUCCESS;
                gcs = GetGCSFactory().Create(wktStr.c_str(), baseGcsWktFlavor, gcsCreateStatus);

                success &= (GCSFactory::S_SUCCESS == gcsCreateStatus);
                }

            // TDORAY: Fetch units


            const Extent3d64f& layerExtent(layerDirP->ComputeExtent());

            DRange3d layerRange = {{layerExtent.xMin, layerExtent.yMin, layerExtent.zMin},
                                   {layerExtent.xMax, layerExtent.yMax, layerExtent.zMax}};

            // TODRAY: Fetch layer name when available

            contentDesc.Add(LayerDescriptor(L"",
                            dataTypes,
                            gcs, 
                            &layerRange));
            }

        success &= 0 != contentDesc.GetLayerCount();

        return success;
        }

public:

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                Jean-Francois.Cote   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static SourceBase*                     Create                         (const File::Ptr&            filePtr,
                                                                           Log&                 pi_warningLog)
        {
        if (0 == filePtr)
            throw FileIOException();

        ContentDescriptor contentDesc(L"");

        if (!DescribeContent(*filePtr, contentDesc))
            return 0;

        return new IDTMSource(filePtr, contentDesc);
        }

    };

const SourceRegistry::AutoRegister<IDTMFileCreator> s_RegisterIDTMFile;



/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class IDTMPointExtractor : public InputExtractorBase
    {
private:
    friend class                    IDTMPointExtractorCreator;

   // Dimension groups definition
    enum 
        {
        DG_POINTS,
        DG_QTY,
        };

    typedef vector<const PointDir*> DirList;

    typedef pair<PointPacketHandler::PacketCIter, PointPacketHandler::PacketCIter>
                                    PacketsRange;



    const DataType&                 m_outputType;

    const LayerDir*                 m_pLayer;
    DirList                         m_dirs;

    size_t                          m_maxPointPacketSize;

    PointPacketHandler::CPtr        m_pHandler;
    PacketsRange                    m_packetsRange;

    PODPacketProxy<byte>            m_rawPointPacket;
    HPU::Packet                     m_packet;

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        IDTMPointExtractor             (const DataType&             pi_outputType,
                                                                    const LayerDir*             pi_pLayer,
                                                                    const DirList&              pi_rDirs) 
        :   m_outputType(pi_outputType),
            m_pLayer(pi_pLayer),
            m_dirs(pi_rDirs),
            m_pHandler (PointPacketHandler::CreateFrom(m_dirs.back())),
            m_packetsRange(m_pHandler->PacketBegin(), m_pHandler->PacketEnd())
        {
        m_dirs.pop_back();

        assert(0 != m_pHandler);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                        (PacketGroup&            pi_rRawEntities) override
        {
        m_rawPointPacket.AssignTo(pi_rRawEntities[DG_POINTS]);
        m_packet.WrapEditable(m_rawPointPacket.Edit(), 0, m_rawPointPacket.GetCapacity());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                          () override
        {
        if (m_packetsRange.first == m_packetsRange.second || 
            !m_packetsRange.first->GetPoints(m_packet))
            {
            m_rawPointPacket.SetSize(0);
            return;
            }

        m_rawPointPacket.SetSize(m_packet.GetSize());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        if (m_packetsRange.first == m_packetsRange.second)
            return false;

        if (++m_packetsRange.first != m_packetsRange.second)
            return true;

        if (m_dirs.empty())
            return false;

        m_pHandler = PointPacketHandler::CreateFrom(m_dirs.back());
        
        assert (0 != m_pHandler);
        m_dirs.pop_back();

        m_packetsRange = make_pair(m_pHandler->PacketBegin(), m_pHandler->PacketEnd());

        return m_packetsRange.first != m_packetsRange.second;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class IDTMPointExtractorCreator : public InputExtractorCreatorMixinBase<IDTMSource>
    {
    typedef vector<const PointDir*>             DirList;

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                                _Supports                          (const DataType&         pi_rType) const override
        {
        return pi_rType.GetFamily() == PointTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (IDTMSource&                     sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        const LayerDir* layerPtr = sourceBase.GetFile().GetRootDir()->GetLayerDir(selection.GetLayer());
        
        const size_t maxPointCount = (0 != layerPtr) ? ComputeMaxPointCount(layerPtr->UniformFeatureDirsBegin(), layerPtr->UniformFeatureDirsEnd()) : 0;
        const size_t pointTypeSize = selection.GetType().GetOrgGroup()[0].GetTypeSize();

        return RawCapacities (maxPointCount*pointTypeSize);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (IDTMSource&                     sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const override
        {
        assert(0 < sourceBase.GetFile().GetRootDir()->CountLayerDirs());

        const LayerDir* pLayer = sourceBase.GetFile().GetRootDir()->GetLayerDir(selection.GetLayer());
        if (0 == pLayer || 0 == pLayer->CountUniformFeatureDirs())
            return 0;

        DirList pointDirs;

        if (!FindPointOnlyDirs(*pLayer, pointDirs))
            return false;

 
        return new IDTMPointExtractor(selection.GetType(), pLayer, pointDirs);
        }


    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename DirIterT>
    static size_t                   ComputeMaxPointCount                           (const DirIterT& dirBegin,
                                                                                    const DirIterT& dirEnd) 
        {
        typedef typename DirIterT::value_type EditorType;

        struct MaxPointCount : binary_function<size_t, EditorType, size_t>
            {
            size_t operator () (size_t maxCount, const EditorType& dirEditor) const
                {
                const FeatureDir* dirP = dirEditor.Get();
                const size_t count = ((0 == dirP || !dirP->IsPointOnly()) ? 0 : dirP->GetTileMaxPointCount());

                return (std::max)(maxCount, count);
                }
            };

        size_t maxCount = 0;
        maxCount = std::accumulate(dirBegin, dirEnd, maxCount, MaxPointCount());

        return maxCount;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool                                 FindPointOnlyDirs                  (const LayerDir&         pi_Layer,
                                                                                    DirList&                po_dirs)
        {
        for (LayerDir::UniformFeatureDirCIter dirIt = pi_Layer.UniformFeatureDirsBegin(), 
             dirsEnd = pi_Layer.UniformFeatureDirsEnd();
             dirIt != dirsEnd;
             ++dirIt)
            {
            const UniformFeatureDir* dirP = dirIt->Get();
            if (0 == dirP || !dirP->IsPointOnly())
                continue;

            po_dirs.push_back(dirP->GetPointDir());
            }

        return !po_dirs.empty();
        }

    };

const ExtractorRegistry::AutoRegister<IDTMPointExtractorCreator> s_RegisterIDTMFilePointImporter;



/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class IDTMLinearExtractor : public InputExtractorBase
    {
    friend class                    IDTMLinearExtractorCreator;


    typedef vector<const FeatureDir*> 
                                    DirList;

    typedef pair<FeaturePacketHandler::PacketCIter, FeaturePacketHandler::PacketCIter>
                                    PacketsRange;

    const DataType&                 m_outputType;

    const LayerDir*                 m_pLayer;
    DirList                         m_dirs;

    size_t                          m_maxHeaderPacketSize;
    size_t                          m_maxPointPacketSize;

    FeaturePacketHandler::CPtr      m_pHandler;
    PacketsRange                    m_packetsRange;

    PODPacketProxy<byte>            m_rawHeaderPacket;
    PODPacketProxy<byte>            m_rawPointPacket;

    HPU::Packet                     m_headerPacket;
    HPU::Packet                     m_pointPacket;


    explicit                        IDTMLinearExtractor            (const DataType&             pi_outputType,
                                                                    const LayerDir*             pi_pLayer,
                                                                    const DirList&              pi_rDirs) 
        :   m_outputType(pi_outputType),
            m_pLayer(pi_pLayer),
            m_dirs(pi_rDirs),
            m_pHandler (FeaturePacketHandler::CreateFrom(m_dirs.back())),
            m_packetsRange(m_pHandler->PacketBegin(), m_pHandler->PacketEnd())
        {
        m_dirs.pop_back();

        assert(0 != m_pHandler);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                        (PacketGroup&     pi_rRawEntities) override
        {
        m_rawHeaderPacket.AssignTo(pi_rRawEntities[0]);
        m_rawPointPacket.AssignTo(pi_rRawEntities[1]);

        m_headerPacket.WrapEditable(m_rawHeaderPacket.Edit(), 0, m_rawHeaderPacket.GetCapacity());
        m_pointPacket.WrapEditable(m_rawPointPacket.Edit(), 0, m_rawPointPacket.GetCapacity());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                          () override
        {
        if (m_packetsRange.first == m_packetsRange.second || 
            !m_packetsRange.first->GetFeatures(m_headerPacket, m_pointPacket))
            {
            m_rawHeaderPacket.SetSize(0);
            m_rawPointPacket.SetSize(0);
            return;
            }

        m_rawHeaderPacket.SetSize(m_headerPacket.GetSize());
        m_rawPointPacket.SetSize(m_pointPacket.GetSize());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        if (m_packetsRange.first == m_packetsRange.second)
            return false;

        if (++m_packetsRange.first != m_packetsRange.second)
            return true;

        if (m_dirs.empty())
            return false;

        m_pHandler = FeaturePacketHandler::CreateFrom(m_dirs.back());
        
        assert (0 != m_pHandler);
        m_dirs.pop_back();

        m_packetsRange = make_pair(m_pHandler->PacketBegin(), m_pHandler->PacketEnd());

        return m_packetsRange.first != m_packetsRange.second;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class IDTMLinearExtractorCreator : public InputExtractorCreatorMixinBase<IDTMSource>
    {
    typedef vector<const FeatureDir*>           DirList;

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                                _Supports                          (const DataType&             pi_rType) const override
        {
        return pi_rType.GetFamily() == LinearTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (IDTMSource&                     sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection) const override
        {
        const LayerDir* layerPtr = sourceBase.GetFile().GetRootDir()->GetLayerDir(selection.GetLayer());
        
        if (0 == layerPtr)
            return RawCapacities(size_t(0), size_t(0));

        const size_t maxHeaderCount = (std::max)(ComputeMaxHeaderCount(layerPtr->UniformFeatureDirsBegin(), layerPtr->UniformFeatureDirsEnd()),
                                                 ComputeMaxHeaderCount(layerPtr->MixedFeatureDirsBegin(), layerPtr->MixedFeatureDirsEnd()));
        const size_t headerTypeSize = selection.GetType().GetOrgGroup()[0].GetTypeSize();


        const size_t maxPointCount = (std::max)(ComputeMaxPointCount(layerPtr->UniformFeatureDirsBegin(), layerPtr->UniformFeatureDirsEnd()),
                                                ComputeMaxPointCount(layerPtr->MixedFeatureDirsBegin(), layerPtr->MixedFeatureDirsEnd()));
        const size_t pointTypeSize = selection.GetType().GetOrgGroup()[1].GetTypeSize();


        return RawCapacities (maxHeaderCount * headerTypeSize, maxPointCount * pointTypeSize);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (IDTMSource&                     sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const override
        {
        assert(0 < sourceBase.GetFile().GetRootDir()->CountLayerDirs());

        const LayerDir* pLayer = sourceBase.GetFile().GetRootDir()->GetLayerDir(selection.GetLayer());
        if (0 == pLayer)
            return 0;

        DirList featureDirs;

        if (!FindFeatureDirs(*pLayer, featureDirs))
            return false;

        return new IDTMLinearExtractor(selection.GetType(), pLayer, featureDirs);
        }


    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename DirIterT>
    static size_t                   ComputeMaxHeaderCount                          (const DirIterT& dirBegin,
                                                                                    const DirIterT& dirEnd) 
        {
        typedef typename DirIterT::value_type EditorType;

        struct MaxHeaderCount : binary_function<size_t, EditorType, size_t>
            {
            size_t operator () (size_t maxCount, const EditorType& dirEditor) const
                {
                const FeatureDir* dirP = dirEditor.Get();
                const size_t count = ((0 == dirP || dirP->IsPointOnly()) ? 0 : dirP->GetTileMaxFeatureCount());

                return (std::max)(maxCount, count);
                }
            };

        size_t maxCount = 0;
        maxCount = std::accumulate(dirBegin, dirEnd, maxCount, MaxHeaderCount());

        return maxCount;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename DirIterT>
    static size_t                   ComputeMaxPointCount                           (const DirIterT& dirBegin,
                                                                                    const DirIterT& dirEnd) 
        {
        typedef typename DirIterT::value_type EditorType;

        struct MaxPointCount : binary_function<size_t, EditorType, size_t>
            {
            size_t operator () (size_t maxCount, const EditorType& dirEditor) const
                {
                const FeatureDir* dirP = dirEditor.Get();
                const size_t count = (0 == dirP || dirP->IsPointOnly() ? 0 : dirP->GetTileMaxPointCount());

                return (std::max)(maxCount, count);
                }
            };

        size_t maxCount = 0;
        maxCount = std::accumulate(dirBegin, dirEnd, maxCount, MaxPointCount());

        return maxCount;
        }



    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool                                 FindFeatureDirs                    (const LayerDir&         pi_Layer,
                                                                                    DirList&                po_dirs)
        {
        for (LayerDir::UniformFeatureDirCIter dirIt = pi_Layer.UniformFeatureDirsBegin(), 
             dirsEnd = pi_Layer.UniformFeatureDirsEnd();
             dirIt != dirsEnd;
             ++dirIt)
            {
            const UniformFeatureDir* dirP = dirIt->Get();

            if (0 == dirP || dirP->IsPointOnly())
                continue;

            po_dirs.push_back(dirP);
            }

        for (LayerDir::MixedFeatureDirCIter dirIt = pi_Layer.MixedFeatureDirsBegin(), 
             dirsEnd = pi_Layer.MixedFeatureDirsEnd();
             dirIt != dirsEnd;
             ++dirIt)
            {
            const MixedFeatureDir* dirP = dirIt->Get();
            if (0 == dirP || dirP->IsPointOnly())
                continue;

            po_dirs.push_back(dirP);
            }

        return !po_dirs.empty();
        }

    };

const ExtractorRegistry::AutoRegister<IDTMLinearExtractorCreator> s_RegisterIDTMFileFeatureImporter;


} //END UNAMED NAMESPACE
BEGIN_BENTLEY_MRDTM_NAMESPACE
namespace Plugin { namespace V0 {

SourceBase* CreateSTMSource  (const WChar*              path,
                              Log&                 log)
    {
    File::Ptr filePtr(File::Open(path, HFC_SHARE_READ_WRITE | HFC_READ_ONLY));
    return IDTMFileCreator::Create(filePtr, log);
    }

}} // END namespace Plugin::V0
END_BENTLEY_MRDTM_NAMESPACE
