/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/GDALFileImporter.cpp $
|    $RCSfile: GDALFileImporter.cpp,v $
|   $Revision: 1.0 $
|       $Date: 2015/04/07 12:15:00 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include <ScalableMeshPCH.h>


#include <ScalableMesh/Import/Plugin/InputExtractorV0.h>
#include <ScalableMesh/Import/Plugin/SourceV0.h>

#include <ScalableMesh/Plugin/IScalableMeshPolicy.h>
#include <ScalableMesh/Plugin/IScalableMeshFileUtilities.h>

#include <ScalableMesh/Type/IScalableMeshLinear.h>

#include <Imagepp-gdal/ogr_api.h>
#include <Imagepp-gdal/ogr_srs_api.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY_SCALABLEMESH

using namespace Bentley::ScalableMesh::Plugin;

namespace
    { //BEGIN UNNAMED NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Elenie.Godzaridis   04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
    class GDALSource : public SourceMixinBase<GDALSource>
    {
public:
    OGRDataSourceH                  GetDataSource() { return m_dataset; }
    int                             GetFeatureCount() { return m_featureCount; }
    int                             GetPointCount() { return m_pointCount; }

private:
    friend class                    GDALSourceCreator;

    OGRDataSourceH                  m_dataset;
    int                             m_featureCount = 500;
    int                             m_pointCount = 50000;
    
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Elenie.Godzaridis   04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        GDALSource(const WString&   pi_rFilePath)
        {

        char* str = (char*)malloc(pi_rFilePath.size() + 1);
        m_dataset = OGROpen(pi_rFilePath.ConvertToLocaleChars(str, pi_rFilePath.size() + 1), FALSE, NULL);
        free(str);
        if (NULL == m_dataset)
            throw FileIOException();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Elenie.Godzaridis   04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                 () override
        {
        if (NULL != m_dataset)
            {
            OGR_DS_Destroy(m_dataset);
            m_dataset = NULL;
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Elenie.Godzaridis   04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor          _CreateDescriptor() const override
        {
        // TDORAY: Should we compute an extent??
        OGRLayerH currentLayer = OGR_DS_GetLayer(m_dataset, 0);
        OGRSpatialReferenceH spatialRef = OGR_L_GetSpatialRef(currentLayer);
        char* wkt = NULL;
        OSRExportToWkt(spatialRef, &wkt);
        WString wktStr(wkt, BentleyCharEncoding::Utf8);
        GCSFactory::Status gcsCreateStatus;
            GCS gcs = GetGCSFactory().Create(wktStr.c_str(), gcsCreateStatus);
            ScalableMeshData data = ScalableMeshData::GetNull();
            data.SetIsGISDataType(true);
        return ContentDescriptor
            (
            L"",
            LayerDescriptor(L"",
            LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create(),
            gcs,
            0,
            data)
            );
        }

        /*---------------------------------------------------------------------------------**//**
        * @description
        * @bsimethod                                                  Elenie.Godzaridis   04/2015
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual const WChar*         _GetType() const override
            {
            return L"GIS Vector file";
            }


    };

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsiclass                                                  Elenie.Godzaridis   04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    class GDALLinearExtractor : public InputExtractorBase
        {
        private:
            friend class                            GDALLinearExtractorCreator;

            PODPacketProxy<IDTMFile::FeatureHeader> m_headerPacket;
            PODPacketProxy<DPoint3d>                m_pointPacket;
            IDTMFeatureArray<DPoint3d>              m_featureArray;
            OGRDataSourceH                          m_ogrSource;
            int m_currentLayerID;
            OGRLayerH                              m_currentLayer;
            bool m_hasNext;
            OGRFeatureH                            m_currentFeature;
            OGRGeometryH                           m_currentGeomRef;
            int                                     m_currentGeomID;
            int                                     m_currentGeomChildID;
            int                                     m_defaultPtCount;
            WString                                 m_elevationProperty;

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                  Elenie.Godzaridis   04/2015
            +---------------+---------------+---------------+---------------+---------------+------*/
            explicit                        GDALLinearExtractor(GDALSource& gdalFile)
                {
                m_ogrSource = gdalFile.GetDataSource();
                m_currentLayerID = 0;
                m_currentLayer = OGR_DS_GetLayer(m_ogrSource, m_currentLayerID);
                m_hasNext = false;
                m_currentFeature = NULL;
                m_currentGeomRef = NULL;
                m_currentGeomID = 0;
                m_currentGeomChildID = 0;
                m_defaultPtCount = gdalFile.GetPointCount();
                for (auto iter = gdalFile.GetDescriptor().LayersBegin(); iter != gdalFile.GetDescriptor().LayersEnd(); ++iter)
                    {
                    if (!iter->GetScalableMeshData().ElevationPropertyName().empty())
                        {
                        m_elevationProperty = iter->GetScalableMeshData().ElevationPropertyName();
                        break;
                        }
                    }
                }

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                 Elenie.Godzaridis   04/2015
            +---------------+---------------+---------------+---------------+---------------+------*/
            virtual void                    _Assign(PacketGroup&     rawEntities) override
                {
                m_headerPacket.AssignTo(rawEntities[0]);
                m_pointPacket.AssignTo(rawEntities[1]);
                m_featureArray.EditHeaders().WrapEditable(m_headerPacket.Edit(), 0, m_headerPacket.GetCapacity());
                m_featureArray.EditPoints().WrapEditable(m_pointPacket.Edit(), 0, m_pointPacket.GetCapacity());
                }

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                  Elenie.Godzaridis   04/2015
            +---------------+---------------+---------------+---------------+---------------+------*/

            bool                            _AddNewFeature(unsigned int featureType, OGRGeometryH geomRef)
                {
                if (m_featureArray.GetSize() + 1 < m_featureArray.GetCapacity() && m_featureArray.GetTotalPointQty() + OGR_G_GetPointCount(geomRef) < m_featureArray.GetTotalPointCapacity())
                    {
                    int pointCt = OGR_G_GetPointCount(geomRef);
                    if (pointCt > 0)
                        {
                        double zVal = 0;
                        if (!m_elevationProperty.empty())
                            {
                            char* elevationPropName = new char[m_elevationProperty.GetMaxLocaleCharBytes()];
                            elevationPropName = m_elevationProperty.ConvertToLocaleChars(elevationPropName);
                            OGRFeatureDefnH hFDefn = OGR_F_GetDefnRef(m_currentFeature);
                            int iField;

                            for (iField = 0; iField < OGR_FD_GetFieldCount(hFDefn); iField++)
                                {
                                OGRFieldDefnH hFieldDefn = OGR_FD_GetFieldDefn(hFDefn, iField);
                                if (BeStringUtilities::Stricmp(elevationPropName, OGR_Fld_GetNameRef(hFieldDefn)) == 0)
                                    {
                                    if (OGR_Fld_GetType(hFieldDefn) == OFTInteger)
                                        zVal = (double)OGR_F_GetFieldAsInteger(m_currentFeature, iField);
                                    else if (OGR_Fld_GetType(hFieldDefn) == OFTReal)
                                        zVal = OGR_F_GetFieldAsDouble(m_currentFeature, iField);
                                    else if (OGR_Fld_GetType(hFieldDefn) == OFTString)
                                        zVal = atof(OGR_F_GetFieldAsString(m_currentFeature, iField));
                                    else
                                        zVal = atof(OGR_F_GetFieldAsString(m_currentFeature, iField));

                                    }
                                }
                            delete[] elevationPropName;
                            }
                        bvector<DPoint3d> ptVec(pointCt);
                        for (size_t i = 0; i < pointCt; i++)
                            {
                            OGR_G_GetPoint(geomRef, (int)i, &(ptVec[i].x), &(ptVec[i].y), &(ptVec[i].z));
                            if (!m_elevationProperty.empty()) ptVec[i].z = zVal;
                            }
                        m_featureArray.Append(featureType, &ptVec[0],
                                              &ptVec[0] + pointCt);
                        }
                    return true;
                    }
                return false;
                }

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                  Elenie.Godzaridis   04/2015
            +---------------+---------------+---------------+---------------+---------------+------*/

            void                            _GrowFeatureArray(size_t count)
                {
                if (count >= m_featureArray.GetTotalPointCapacity()) //packet is too small
                    {
                    m_featureArray.Reserve(m_featureArray.GetCapacity(), count + 1);
                    m_pointPacket.Reserve(count + 1);
                    m_featureArray.EditPoints().WrapEditable(m_pointPacket.Edit(), m_pointPacket.GetSize(), m_pointPacket.GetCapacity());
                    }
                }

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                  Elenie.Godzaridis   04/2015
            +---------------+---------------+---------------+---------------+---------------+------*/

            bool                            _TryAddFeature(OGRGeometryH feature)
                {
                auto type = OGR_G_GetGeometryType(feature);
                if (type != wkbPoint && type != wkbLineString && type != wkbPoint25D && type != wkbLineString25D)
                    {
                    //see http://lists.osgeo.org/pipermail/gdal-dev/2003-December/001567.html for description of C API to get points from polygon features
                    for (size_t i = m_currentGeomID; i < OGR_G_GetGeometryCount(feature); i++)
                        {
                        OGRGeometryH singleGeom = OGR_G_GetGeometryRef(feature, (int)i);
                        //is it a multipolygon?
                        type = OGR_G_GetGeometryType(singleGeom);
                        if (type != wkbPoint && type != wkbLineString && type != wkbPoint25D && type != wkbLineString25D)
                            {
                            for (size_t j = m_currentGeomChildID; j < OGR_G_GetGeometryCount(singleGeom); j++)
                                {
                                OGRGeometryH element = OGR_G_GetGeometryRef(singleGeom, (int)j);
                                _GrowFeatureArray(OGR_G_GetPointCount(element));
                                DTMFeatureType feaType = type == wkbPoint25D ? DTMFeatureType::FeatureSpot : DTMFeatureType::Breakline;
                                if (!_AddNewFeature((unsigned int)feaType, element))
                                    {
                                    m_currentGeomRef = feature;
                                    m_currentGeomID = (int)i;
                                    m_currentGeomChildID = (int)j;
                                    return false;
                                    }
                                }
                            m_currentGeomChildID = 0;
                            }
                        else
                            {
                            _GrowFeatureArray(OGR_G_GetPointCount(singleGeom));
                            DTMFeatureType feaType = type == wkbPoint25D ? DTMFeatureType::FeatureSpot : DTMFeatureType::Breakline;
                            if (!_AddNewFeature((unsigned int)feaType, singleGeom))
                                {
                                m_currentGeomRef = feature;
                                m_currentGeomID = (int)i;
                                return false;
                                }
                            }
                        }
                    m_currentGeomID = 0;
                    }
                else
                    {
                    _GrowFeatureArray(OGR_G_GetPointCount(feature));
                    DTMFeatureType feaType = type == wkbPoint25D ? DTMFeatureType::FeatureSpot : DTMFeatureType::Breakline;
                    if (!_AddNewFeature((unsigned int)feaType, feature))
                        {
                        m_currentGeomRef = feature;
                        return false;
                        }
                    }
                return true;
                }

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                  Elenie.Godzaridis   04/2015
            +---------------+---------------+---------------+---------------+---------------+------*/
            virtual void                    _Read() override
                {
                if (!m_hasNext)
                    {
                    m_headerPacket.SetSize(0);
                    m_pointPacket.SetSize(0);
                    }
                m_featureArray.Clear();
                //m_featureArray.Reserve(m_featureArray.GetCapacity(), m_defaultPtCount);
                //m_pointPacket.Reserve(m_defaultPtCount);
                //m_featureArray.EditPoints().WrapEditable(m_pointPacket.Edit(), 0, m_pointPacket.GetCapacity());
                m_hasNext = false;
                if (NULL == m_currentFeature) m_currentFeature = OGR_L_GetNextFeature(m_currentLayer);
                while (NULL == m_currentFeature && m_currentLayerID < OGR_DS_GetLayerCount(m_ogrSource) - 1)
                    {
                    ++m_currentLayerID;
                    m_currentLayer = OGR_DS_GetLayer(m_ogrSource, m_currentLayerID);
                    m_currentFeature = OGR_L_GetNextFeature(m_currentLayer);
                    }
                if (NULL == m_currentFeature && m_currentLayerID == OGR_DS_GetLayerCount(m_ogrSource) - 1) return;
                OGRGeometryH  geomRef = NULL == m_currentGeomRef ? OGR_F_GetGeometryRef(m_currentFeature) : m_currentGeomRef;
                bool bufferExceeded = false;
                while (!bufferExceeded && m_currentFeature != NULL && m_currentLayer != NULL)
                    {
                    if (OGR_G_GetGeometryType(geomRef) == wkbPoint25D || OGR_G_GetGeometryType(geomRef) == wkbLineString25D || OGR_G_GetGeometryType(geomRef) == wkbPolygon25D || OGR_G_GetGeometryType(geomRef) == wkbMultiLineString25D || OGR_G_GetGeometryType(geomRef) == wkbMultiPolygon25D || OGR_G_GetGeometryType(geomRef) == wkbGeometryCollection25D)
                        {
                        bufferExceeded = !_TryAddFeature(geomRef);
                        }
                    if (!bufferExceeded)
                        {
                        OGR_F_Destroy(m_currentFeature);
                        m_currentFeature = OGR_L_GetNextFeature(m_currentLayer);
                        while (NULL == m_currentFeature && m_currentLayerID < OGR_DS_GetLayerCount(m_ogrSource) - 1)
                            {
                            ++m_currentLayerID;
                            m_currentLayer = OGR_DS_GetLayer(m_ogrSource, m_currentLayerID);
                            m_currentFeature = OGR_L_GetNextFeature(m_currentLayer);
                            }
                        if (NULL == m_currentFeature && m_currentLayerID == OGR_DS_GetLayerCount(m_ogrSource) - 1) m_currentLayer = NULL;
                        if (m_currentFeature != NULL && m_currentLayer != NULL)  geomRef = OGR_F_GetGeometryRef(m_currentFeature);
                        }
                    }
                if (m_currentFeature != NULL && m_currentLayer != NULL) m_hasNext = true;
                m_headerPacket.SetSize(m_featureArray.GetHeaders().GetSize());
                m_pointPacket.SetSize(m_featureArray.GetPoints().GetSize());
                }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Elenie.Godzaridis   04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        return m_hasNext;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Elenie.Godzaridis   04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
    class GDALSourceCreator : public LocalFileSourceCreatorBase
    {
    virtual ExtensionFilter         _GetExtensions                         () const override
        {
        return L"*.shp;*.kml;*.kmz";
        }    

    virtual bool                    _Supports                              (const LocalFileSourceRef&   pi_rSourceRef) const override
        {
        if(!DefaultSupports(pi_rSourceRef))
            return false;
        OGRRegisterAll();
        OGRDataSourceH       dataset;

        char* str = (char*)malloc(pi_rSourceRef.GetPath().size()+1);
        //OGRGetDriverByName 
        dataset = OGROpen(pi_rSourceRef.GetPath().ConvertToLocaleChars(str, pi_rSourceRef.GetPath().size()+1), FALSE, NULL);
        free(str);

        if (dataset == NULL)
            {
            return false;
            }
        else OGRReleaseDataSource(dataset);
//        StatusInt status;
        return true;
        }

    virtual SourceBase*             _Create                                (const LocalFileSourceRef&   pi_rSourceRef,
                                                                            Log&                 pi_warningLog) const override
        {
        return new GDALSource(pi_rSourceRef.GetPath());
        }
    };

    const SourceRegistry::AutoRegister<GDALSourceCreator> s_RegisterGDALFile;


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Elenie.Godzaridis   04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
class GDALLinearExtractorCreator : public InputExtractorCreatorMixinBase<GDALSource>
    {
    virtual bool                                _Supports                          (const DataType&             type) const override
        {
        return type.GetFamily() == LinearTypeFamilyCreator().Create();
        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Elenie.Godzaridis   04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities(GDALSource&                 sourceBase,
                                                                                    const Bentley::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        return RawCapacities(sourceBase.GetFeatureCount() * sizeof(IDTMFile::FeatureHeader), sourceBase.GetPointCount()*sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Elenie.Godzaridis   04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create(GDALSource&                 sourceBase,
                                                                                    const Bentley::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                            log) const override
        {
        return new GDALLinearExtractor(sourceBase);
        }
    };


const ExtractorRegistry::AutoRegister<GDALLinearExtractorCreator> s_RegisterGDALImporter;
    }//END UNNAMED NAMESPACE