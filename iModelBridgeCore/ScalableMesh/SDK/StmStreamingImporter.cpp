/*--------------------------------------------------------------------------------------+
|
|     $Source: SDK/StmStreamingImporter.cpp $
|    $RCSfile: StmStreamingFileImporter.cpp,v $
|   $Revision: 1.46 $
|       $Date: 2011/11/09 18:11:03 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DataPipe.h"
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP\all\h\IDTMTypes.h>
#include <ImagePP\all\h\IDTMFileDirectories\FeatureHeaderTypes.h>
#include <ImagePP\all\h\IDTMFeatureArray.h>
#include <ImagePP\all\h\HPUArray.h>
#include <GeoCoord\BaseGeoCoord.h>
#include <TerrainModel\TerrainModel.h>
#include <ScalableTerrainModel\MrDTMDefs.h>
#include <ScalableTerrainModel/Import/Plugin/SourceV0.h>
#include <ScalableTerrainModel/Type/IMrDTMPoint.h>
#include <ScalableTerrainModel/Import/Plugin/InputExtractorV0.h>
#include <ScalableTerrainModel/Plugin/IMrDTMPolicy.h>
#include <ScalableTerrainModel/Type/IMrDTMLinear.h>
#include <ScalableTerrainModel/Type/IMrDTMTIN.h>


//#include <STMInternal/Foundations/PrivateStringTools.h>


  
USING_NAMESPACE_BENTLEY_MRDTM_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_MRDTM

//using namespace Bentley::MrDTM::Plugin;



extern DataPipe s_dataPipe = DataPipe();


namespace { //BEGIN UNAMED NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class StmStreamingSource : public SourceMixinBase<StmStreamingSource>
    {
private:
    friend class                    StmStreamingFileCreator;
                
    GCS                             m_gcs;
    

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                 () override
        {        
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor          _CreateDescriptor  () const override
        {
        DataTypeSet storedTypes;

        //storedTypes.push_back(TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator().Create());
        storedTypes.push_back(PointType3d64fCreator().Create());
        storedTypes.push_back(LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create());
                        
        return ContentDescriptor
            (
            L"",
            LayerDescriptor(L"",
                            storedTypes,
                            m_gcs, 
                            0)
            );
        }


    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*             _GetType                           () const override
        {
        return L"StmStreaming";
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        StmStreamingSource                         (const GCS&          gcs) 
        : m_gcs(gcs)
        {
        }        
    };


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class StmStreamingFileCreator : public LocalFileSourceCreatorBase
    {
           
    virtual ExtensionFilter         _GetExtensions                         () const override
        {
        return L"*.stmstream";
        }    

    virtual bool                    _Supports                              (const LocalFileSourceRef&   pi_rSourceRef) const override
        {
        return true;
        }

    virtual SourceBase*             _Create                                (const LocalFileSourceRef&   pi_rSourceRef,
                                                                            Log&                        pi_warningLog) const override
        {                
        return new StmStreamingSource(GCS::GetNull());
        }
    };

const SourceRegistry::AutoRegister<StmStreamingFileCreator> s_RegisterStmStreamingFile;



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class StmStreamingPointExtractor : public InputExtractorBase
    {
private:
                    
    PODPacketProxy<DPoint3d>        m_pointPacket;
    HPU::Array<DPoint3d>            m_ptArray;
    bvector<DPoint3d>               m_points;
    int                             m_nextIndex;
    bool                            m_anyMorePoints;

    // Dimension groups definition
    enum
    {
        DG_XYZ,
        DG_QTY,
    };

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
        m_pointPacket.SetSize(0);

        if (!m_anyMorePoints)
            return;

        if (m_nextIndex == -1)
            {        
            if (m_points.size() > 0)
                {
                s_dataPipe.FinishProcessingData();
                m_points.clear();
                }
            
            s_dataPipe.ReadPoints(m_points);
            m_nextIndex = 0;
            }

        if (m_points.size() > 0)
            {                    
            size_t sizeToCopy = min(m_points.size() - m_nextIndex,  m_pointPacket.GetCapacity());
            m_ptArray.Append(&m_points[m_nextIndex], &m_points[m_nextIndex] + sizeToCopy);
            m_pointPacket.SetSize(m_ptArray.GetSize());

            m_nextIndex += (int)sizeToCopy;

            assert(m_nextIndex <= m_points.size());

            if (m_nextIndex == m_points.size())
                {
                m_nextIndex = -1;
                }
            }
        else
            {
            m_anyMorePoints = false;
            s_dataPipe.FinishProcessingData();            
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                              () override
        {        
        return m_anyMorePoints;        
        }
public:
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        StmStreamingPointExtractor             ()         
        {
        m_nextIndex = -1;
        m_anyMorePoints = true;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class StmStreamingPointExtractorCreator : public InputExtractorCreatorMixinBase<StmStreamingSource>
    {

    virtual bool                                _Supports                          (const DataType&                 pi_rType) const override
        {
        return pi_rType.GetFamily() == PointTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (StmStreamingSource&             sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection) const override
        {        
        return RawCapacities (100000 *sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (StmStreamingSource&                 sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const override
        {        
        return new StmStreamingPointExtractor();
        }

    };

const ExtractorRegistry::AutoRegister<StmStreamingPointExtractorCreator> s_RegisterStmStreamingPointExtractor;




/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class StmStreamingLinearExtractor : public InputExtractorBase
    {
private:

    // Dimension groups definition
    enum 
        {
        DG_Header,
        DG_XYZ,
        DG_QTY,
        };
            
    PODPacketProxy<IDTMFile::FeatureHeader>
                                    m_headerPacket;
    PODPacketProxy<DPoint3d>        m_pointPacket;
    IDTMFeatureArray<DPoint3d>      m_featureArray;    
    bvector<DPoint3d>               m_points;
    DTMFeatureType                  m_featureType;
    bool                            m_hasAnyNewFeature;

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

        m_headerPacket.SetSize(0);
        m_pointPacket.SetSize(0);
        m_points.clear();

        s_dataPipe.ReadFeature(m_points, m_featureType);

        if (m_points.size() > 0)
            {
            m_featureArray.Append((IDTMFile::FeatureType)m_featureType,
                                  &m_points[0],
                                  &m_points[0] + m_points.size());

            m_headerPacket.SetSize(m_featureArray.GetHeaders().GetSize());
            m_pointPacket.SetSize(m_featureArray.GetPoints().GetSize());            
            }
        else
            {
            m_hasAnyNewFeature = false;
            }

        s_dataPipe.FinishProcessingData();

        return;

/*        
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
        */
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                              () override
        {
        return m_hasAnyNewFeature;         
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        StmStreamingLinearExtractor ()                                 
        {
        m_hasAnyNewFeature = true;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class StmStreamingLinearExtractorCreator : public InputExtractorCreatorMixinBase<StmStreamingSource>
    {
    virtual bool                                _Supports                          (const DataType&         pi_rType) const override
        {
        return pi_rType.GetFamily() == LinearTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (StmStreamingSource&                 sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection) const override
        {        
        return RawCapacities (sizeof(IDTMFile::FeatureHeader),
                              500000*sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (StmStreamingSource&                 sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const override
        {        
        return new StmStreamingLinearExtractor();
        }
    };

const ExtractorRegistry::AutoRegister<StmStreamingLinearExtractorCreator> s_RegisterStmStreamingLinearExtractor;

} //END UNAMED NAMESPACE

BEGIN_BENTLEY_MRDTM_NAMESPACE
namespace Plugin { namespace V0 {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceBase*                                     CreateStmStreamingSource                   (Log&                        log)
    {
    return new StmStreamingSource(GCS::GetNull());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceBase*                                     CreateStmStreamingSource                   (const GCS&                  gcs,
                                                                                            Log&                        log)
    {
    return new StmStreamingSource(gcs);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorBase*                             CreateStmStreamingPointExtractor           ()
    {
    return new StmStreamingPointExtractor();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorBase*                             CreateStmStreamingLinearExtractor          ()
    {
    return new StmStreamingLinearExtractor();
    }

}} // END namespace Plugin::V0
END_BENTLEY_MRDTM_NAMESPACE
