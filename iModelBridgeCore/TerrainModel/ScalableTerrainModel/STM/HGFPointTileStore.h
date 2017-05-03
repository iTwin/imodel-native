//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/HGFPointTileStore.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once



#include <ImagePP/all/h/HPMDataStore.h>
#include <STMInternal/Storage/IDTMTypes.h>
#include <STMInternal/Storage/IDTMFile.h>
#include "HGFSpatialIndex.h"
#include "HVEDTMLinearFeature.h"


class IDTMFilePointFamily {};

template <> struct PointFamilyTrait<IDTMFile::Point3d64f>       {
    typedef IDTMFilePointFamily type;
    };
template <> struct PointFamilyTrait<IDTMFile::Point3d64fM64f>   {
    typedef IDTMFilePointFamily type;
    };

template <class POINT> struct PointOp<POINT, IDTMFilePointFamily>
    {
    static double GetX(const POINT& point) {
        return IDTMFile::PointTrait<POINT>::GetX(point);
        }
    static double GetY(const POINT& point) {
        return IDTMFile::PointTrait<POINT>::GetY(point);
        }
    static double GetZ(const POINT& point) {
        return IDTMFile::PointTrait<POINT>::GetZ(point);
        }
    static void   SetX(POINT& point, double x) {
        IDTMFile::PointTrait<POINT>::SetX(point, x);
        }
    static void   SetY(POINT& point, double y) {
        IDTMFile::PointTrait<POINT>::SetY(point, y);
        }
    static void   SetZ(POINT& point, double z) {
        IDTMFile::PointTrait<POINT>::SetZ(point, z);
        }
    static POINT  Create(double x, double y, double z) {
        return IDTMFile::PointTrait<POINT>::Create(x, y, z);
        }
    static bool   AreEqual(const POINT& point1, const POINT& point2) {
        return IDTMFile::PointTrait<POINT>::Equal(point1, point2);
        }
    };


template <> class SpatialOp<HFCPtr<HVEDTMLinearFeature>, HGF3DCoord<double>, HGF3DExtent<double> >
    {
private:
    typedef HGF3DExtent<double>   Extent_t;
    typedef HFCPtr<HVEDTMLinearFeature>                         Spatial_t;
    typedef HGF3DCoord<double>                                  Coord_t;

public:
    static Extent_t GetExtent(const Spatial_t& spatialObject)
        {
        typedef HGF3DExtent<double> SpatialExtent_t;
        const SpatialExtent_t& r3dExtent = spatialObject->GetExtent();

        return ExtentOp<Extent_t>::Create(ExtentOp<SpatialExtent_t>::GetXMin(r3dExtent),
                                          ExtentOp<SpatialExtent_t>::GetYMin(r3dExtent),
                                          ExtentOp<SpatialExtent_t>::GetZMin(r3dExtent),
                                          ExtentOp<SpatialExtent_t>::GetXMax(r3dExtent),
                                          ExtentOp<SpatialExtent_t>::GetYMax(r3dExtent),
                                          ExtentOp<SpatialExtent_t>::GetZMax(r3dExtent));
        }

    static bool IsPointIn2D(const Spatial_t& spatialObject, const Coord_t& coord)
        {
        return spatialObject->IsPointOn2D(coord);
        }

    static bool IsSpatialInExtent2D (const Spatial_t& spatialObject, const Extent_t& extent)
        {
        const Extent_t& spatialExtent = SpatialOp<Spatial_t, Coord_t, Extent_t>::GetExtent(spatialObject);

        return ((ExtentOp<Extent_t>::GetXMin(spatialExtent) >= ExtentOp<Extent_t>::GetXMin(extent)) &&
                (ExtentOp<Extent_t>::GetYMin(spatialExtent) >= ExtentOp<Extent_t>::GetYMin(extent)) &&
                (ExtentOp<Extent_t>::GetXMax(spatialExtent) <= ExtentOp<Extent_t>::GetXMax(extent)) &&
                (ExtentOp<Extent_t>::GetYMax(spatialExtent) <= ExtentOp<Extent_t>::GetYMax(extent)));

        }

    static size_t GetPointCount (const Spatial_t& spatial)
        {
        return spatial->GetSize();
        }


    };




template <> class ExtentOp <IDTMFile::Extent3d64f>
{
public:
    static double GetXMin(const IDTMFile::Extent3d64f& extent) {return extent.xMin;}
    static double GetXMax(const IDTMFile::Extent3d64f& extent) {return extent.xMax;}
    static double GetYMin(const IDTMFile::Extent3d64f& extent) {return extent.yMin;}
    static double GetYMax(const IDTMFile::Extent3d64f& extent) {return extent.yMax;}
    static double GetZMin(const IDTMFile::Extent3d64f& extent) {return extent.zMin;}
    static double GetZMax(const IDTMFile::Extent3d64f& extent) {return extent.zMax;}
    static void   SetXMin(IDTMFile::Extent3d64f& extent, double xMin) {extent.xMin = xMin;}
    static void   SetXMax(IDTMFile::Extent3d64f& extent, double xMax) {extent.xMax = xMax;}
    static void   SetYMin(IDTMFile::Extent3d64f& extent, double yMin) {extent.yMin = yMin;}
    static void   SetYMax(IDTMFile::Extent3d64f& extent, double yMax) {extent.yMax = yMax;}
    static void   SetZMin(IDTMFile::Extent3d64f& extent, double zMin) {extent.zMin = zMin;}
    static void   SetZMax(IDTMFile::Extent3d64f& extent, double zMax) {extent.zMax = zMax;}
    static double GetWidth(const IDTMFile::Extent3d64f& extent) {return (extent.xMax - extent.xMin);}
    static double GetHeight(const IDTMFile::Extent3d64f& extent) {return (extent.yMax - extent.yMin);}
    static double GetThickness(const IDTMFile::Extent3d64f& extent) {return extent.zMax - extent.zMin;}
    static bool   Contains2d (const IDTMFile::Extent3d64f& extent1, const IDTMFile::Extent3d64f& extent2) {return (extent1.xMax >= extent2.xMax) && (extent1.xMin <= extent2.xMin) && (extent1.yMax >= extent2.yMax) && (extent1.yMin <= extent2.yMin);}                
    static bool   Overlap (const IDTMFile::Extent3d64f& extent1, const IDTMFile::Extent3d64f& extent2) {return HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent1.xMin, extent1.yMin, extent1.xMax, extent1.yMax).Overlaps (HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent2.xMin, extent2.yMin, extent2.xMax, extent2.yMax));}
    static bool   InnerOverlap (const IDTMFile::Extent3d64f& extent1, const IDTMFile::Extent3d64f& extent2) {return HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent1.xMin, extent1.yMin, extent1.xMax, extent1.yMax).InnerOverlaps (HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent2.xMin, extent2.yMin, extent2.xMax, extent2.yMax));}
    static bool   OutterOverlap (const IDTMFile::Extent3d64f& extent1, const IDTMFile::Extent3d64f& extent2) {return HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent1.xMin, extent1.yMin, extent1.xMax, extent1.yMax).OutterOverlaps (HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent2.xMin, extent2.yMin, extent2.xMax, extent2.yMax));}
    __forceinline static IDTMFile::Extent3d64f Create(double xMin, double yMin, double zMin, double xMax, double yMax, double zMax) 
    {
        IDTMFile::Extent3d64f dummy;
        dummy.xMin = xMin;
        dummy.yMin = yMin;
        dummy.zMin = zMin;
        dummy.xMax = xMax;
        dummy.yMax = yMax;
        dummy.zMax = zMax;
        return dummy;
        }
    static IDTMFile::Extent3d64f Create(double xMin, double yMin, double xMax, double yMax)
        {
        IDTMFile::Extent3d64f dummy;
        dummy.xMin = xMin;
        dummy.yMin = yMin;
        dummy.zMin = 0.0;
        dummy.xMax = xMax;
        dummy.yMax = yMax;
        dummy.zMax = 0.0;
        return dummy;
        }

    __forceinline static IDTMFile::Extent3d64f MergeExtents(const IDTMFile::Extent3d64f& extent1, const IDTMFile::Extent3d64f& extent2)
        {
        return Create(MIN(GetXMin(extent1), GetXMin(extent2)),
                      MIN(GetYMin(extent1), GetYMin(extent2)),
                      MIN(GetZMin(extent1), GetZMin(extent2)),
                      MAX(GetXMax(extent1), GetXMax(extent2)),
                      MAX(GetYMax(extent1), GetYMax(extent2)),
                      MAX(GetZMax(extent1), GetZMax(extent2)));
        }

    };



template <class POINT> class ExtentPointOp<IDTMFile::Extent3d64f, POINT>
    {
public:


    static bool IsPointOutterIn3D(const IDTMFile::Extent3d64f& extent, const POINT& point)
        {
        return (extent.xMin <= PointOp<POINT>::GetX(point) && extent.xMax >= PointOp<POINT>::GetX(point)) &&
               (extent.yMin <= PointOp<POINT>::GetY(point) && extent.yMax >= PointOp<POINT>::GetY(point)) &&
               (extent.zMin <= PointOp<POINT>::GetZ(point) && extent.zMax >= PointOp<POINT>::GetZ(point));
        }


    static bool IsPointOutterIn2D(const IDTMFile::Extent3d64f& extent, const POINT& point)
        {
        return HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent.xMin,
                                                                extent.yMin,
                                                                extent.xMax,
                                                                extent.yMax).IsPointOutterIn (HGF3DCoord<double>(PointOp<POINT>::GetX(point),
                                                                        PointOp<POINT>::GetY(point),
                                                                        PointOp<POINT>::GetZ(point)));
        }
    };


template <class SPATIAL, class POINT> class SpatialOp<SPATIAL, POINT, IDTMFile::Extent3d64f, PointSpatialFamily>
    {
public:
    static  IDTMFile::Extent3d64f GetExtent(const SPATIAL& spatial)
        {
        IDTMFile::Extent3d64f toto;
        toto.xMin = PointOp<SPATIAL>::GetX(spatial);
        toto.xMax = PointOp<SPATIAL>::GetX(spatial);
        toto.yMin = PointOp<SPATIAL>::GetY(spatial);
        toto.yMax = PointOp<SPATIAL>::GetY(spatial);
        toto.zMin = PointOp<SPATIAL>::GetZ(spatial);
        toto.zMax = PointOp<SPATIAL>::GetZ(spatial);

        return  toto;
        }

    static bool IsPointIn2D(const SPATIAL& spatial, const POINT& pi_rCoord)
        {
        return HGF3DCoord<double>(PointOp<SPATIAL>::GetX(spatial),
                                  PointOp<SPATIAL>::GetY(spatial),
                                  PointOp<SPATIAL>::GetZ(spatial)).IsEqualTo2D(HGF3DCoord<double>(PointOp<POINT>::GetX(pi_rCoord),
                                                                               PointOp<POINT>::GetY(pi_rCoord),
                                                                               PointOp<POINT>::GetZ(pi_rCoord)));
        }

    __forceinline static bool IsSpatialInExtent2D (const SPATIAL& spatial, const  IDTMFile::Extent3d64f& extent)
        {
        return ((PointOp<SPATIAL>::GetX(spatial) >= extent.xMin) &&
                (PointOp<SPATIAL>::GetX(spatial) <= extent.xMax) &&
                (PointOp<SPATIAL>::GetY(spatial) >= extent.yMin) &&
                (PointOp<SPATIAL>::GetY(spatial) <= extent.yMax));
        }
    static size_t GetPointCount (const SPATIAL& spatial)
        {
        return 1;
        }

    };




template <typename EXTENT> class HGFPointNodeHeader : public HGFIndexNodeHeader<EXTENT>
    {
public:
    HPMBlockID  m_apSubNodeID[8];
    HPMBlockID  m_SubNodeNoSplitID;
    bool        m_filtered;
    double     m_ViewDependentMetrics[9];
    };


template <class EXTENT> class HGFPointIndexHeader: public HGFSpatialIndexHeader<EXTENT>
    {
public:

    HPMBlockID              m_rootNodeBlockID;
    };




template <typename EXTENT> class HGFFeatureNodeHeader : public HGFIndexNodeHeader<EXTENT>
    {
public:
    HPMBlockID  m_apSubNodeID[8];
    HPMBlockID  m_SubNodeNoSplitID;
    };


template <class EXTENT> class HGFFeatureIndexHeader : public HGFSpatialIndexHeader<EXTENT>
    {
public:

    HPMBlockID              m_rootNodeBlockID;
    };


template <class POINT, class EXTENT> class HGFPointTileStore: public IHPMPermanentStore<POINT, HGFPointIndexHeader<EXTENT>, HGFPointNodeHeader<EXTENT>>
    {
public:
    HGFPointTileStore() {};
    virtual ~HGFPointTileStore() {};
    };

template <class POINT, class EXTENT> class GenericDTMTileStore: public HGFPointTileStore<POINT, EXTENT>
    {
public:
    GenericDTMTileStore (const char* fileName)
        : m_InternalStore (fileName, sizeof (HGFPointNodeHeader<EXTENT>))
        {
        }
    virtual ~GenericDTMTileStore ()
        {
        }

    virtual void Close()
        {
        m_InternalStore.Close();
        }


    virtual bool StoreMasterHeader (HGFPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {
        return m_InternalStore.StoreMasterHeader ((Byte*)indexHeader, sizeof (HGFPointIndexHeader<EXTENT>));
        }

    virtual size_t LoadMasterHeader (HGFPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {
        return m_InternalStore.LoadMasterHeader ((Byte*)indexHeader, sizeof(HGFPointIndexHeader<EXTENT>));
        }

    // New interface
    virtual HPMBlockID StoreNewBlock (POINT* DataTypeArray, size_t countData)
        {
        return m_InternalStore.StoreNewBlock (DataTypeArray, countData);
        }

    virtual HPMBlockID StoreBlock (POINT* DataTypeArray, size_t countData, HPMBlockID blockID)
        {
        return m_InternalStore.StoreBlock (DataTypeArray, countData, blockID);

        }

    virtual size_t GetBlockDataCount (HPMBlockID blockID) const
        {
        return m_InternalStore.GetBlockDataCount (blockID);

        }


    virtual size_t StoreHeader (HGFPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
        return m_InternalStore.StoreHeader ((Byte*)header, blockID);

        }

    virtual size_t LoadHeader (HGFPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
        return m_InternalStore.LoadHeader ((Byte*)header, blockID);
        }


    virtual size_t LoadBlock (POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
        {
        return m_InternalStore.LoadBlock (DataTypeArray, maxCountData, blockID);
        }

    virtual bool DestroyBlock (HPMBlockID blockID)
        {
        return m_InternalStore.DestroyBlock (blockID);
        }



private:
    HPMGenericDataStore<POINT> m_InternalStore;
    };


template <class POINT, class EXTENT> class GenericCompressedDTMTileStore: public HGFPointTileStore<POINT, EXTENT>
    {
public:
    GenericCompressedDTMTileStore (const char* fileName, int compressionFactor)
        : m_InternalStore (fileName, sizeof (HGFPointNodeHeader<EXTENT>), compressionFactor)
        {
        }
    virtual ~GenericCompressedDTMTileStore ()
        {
        }

    virtual void Close()
        {
        m_InternalStore.Close();
        }


    virtual bool StoreMasterHeader (HGFPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {
        return m_InternalStore.StoreMasterHeader ((Byte*)indexHeader, sizeof (HGFPointIndexHeader<EXTENT>));
        }

    virtual size_t LoadMasterHeader (HGFPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {
        return m_InternalStore.LoadMasterHeader ((Byte*)indexHeader, sizeof(HGFPointIndexHeader<EXTENT>));
        }

    // New interface
    virtual HPMBlockID StoreNewBlock (POINT* DataTypeArray, size_t countData)
        {
        return m_InternalStore.StoreNewBlock (DataTypeArray, countData);
        }

    virtual HPMBlockID StoreBlock (POINT* DataTypeArray, size_t countData, HPMBlockID blockID)
        {
        return m_InternalStore.StoreBlock (DataTypeArray, countData, blockID);

        }

    virtual size_t GetBlockDataCount (HPMBlockID blockID) const
        {
        return m_InternalStore.GetBlockDataCount (blockID);

        }


    virtual size_t StoreHeader (HGFPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
        return m_InternalStore.StoreHeader ((Byte*)header, blockID);

        }

    virtual size_t LoadHeader (HGFPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
        return m_InternalStore.LoadHeader ((Byte*)header, blockID);
        }


    virtual size_t LoadBlock (POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
        {
        return m_InternalStore.LoadBlock (DataTypeArray, maxCountData, blockID);
        }


    virtual bool DestroyBlock (HPMBlockID blockID)
        {
        return m_InternalStore.DestroyBlock (blockID);
        }



private:
    HPMGenericCompressedDataStore<POINT> m_InternalStore;
    };




template <typename POINT, typename EXTENT> class DTMPointTaggedTileStore : public HGFPointTileStore<POINT, EXTENT>// , public HFCShareableObject<HGFPointTileStore<POINT, EXTENT> >
    {
private:
    typedef IDTMFile::PointTileHandler<POINT> TileHandler;
        typedef IDTMFile::BTreeIndexHandler IndexHandler;

    typedef typename TileHandler::PointArray PointArray;

    static IDTMFile::NodeID ConvertBlockID (const HPMBlockID& blockID)
        {
        return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
        }

    static IDTMFile::SubNodesTable::value_type ConvertChildID (const HPMBlockID& childID)
        {
        return static_cast<IDTMFile::SubNodesTable::value_type>(childID.m_integerID);
        }

public:
    // Constructor / Destroyer
    DTMPointTaggedTileStore(const char* filename, const IDTMFile::AccessMode& accessMode, bool compress, size_t layerID = 0)
        {
        m_receivedOpenedFile = false;

        m_DTMFile = IDTMFile::Open (filename, accessMode);

        if (m_DTMFile == NULL)
            throw;

        m_compress = compress;

        m_layerID = layerID;

        }

    DTMPointTaggedTileStore(IDTMFile::File::Ptr openedDTMFile, bool compress, size_t layerID = 0)
            :   m_filteringDir(0),
                m_layerID(layerID),
            m_compress(compress),
            m_receivedOpenedFile(true),
            m_DTMFile(openedDTMFile)
        {
        if (m_DTMFile == NULL)
            throw;
        }

    virtual ~DTMPointTaggedTileStore ()
        {
        if (!m_receivedOpenedFile)
            if (m_DTMFile != NULL)
                m_DTMFile->Close ();

        m_DTMFile = NULL;
        }

    // New function
    virtual bool HasSpatialReferenceSystem()
        {
            IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir (m_layerID);
        if (NULL == layerDir)
            return false;


            return layerDir->HasWkt();

        }


    // New function
    virtual std::string GetSpatialReferenceSystem()
        {
        IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir (m_layerID);
        if (NULL == layerDir)
            return false;

        size_t  destinationBuffSize = WString(layerDir->GetWkt().GetCStr()).GetMaxLocaleCharBytes();
        char*  valueMBS= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(valueMBS, layerDir->GetWkt().GetCStr(),destinationBuffSize);


        return string(valueMBS);
        }

    // ITileStore interface
    virtual void Close ()
        {
        if (!m_receivedOpenedFile)
            if (m_DTMFile != NULL)
                m_DTMFile->Close ();

        m_DTMFile = NULL;
        }

    virtual bool StoreMasterHeader (HGFPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {
        if (m_DTMFile == NULL)
            return false;

        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        if (NULL == m_tileHandler)
            {
            IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir (m_layerID);

            if (NULL == layerDir)
                {
                layerDir = m_DTMFile->GetRootDir()->AddLayerDir ();
                HASSERT(NULL != layerDir);
                if (NULL == layerDir)
                    return false;

                m_layerID = layerDir->GetIndex();
                HASSERT(0 == m_layerID);
                }

            IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(MASS_POINT_FEATURE_TYPE);
            if (NULL == featureDir)
                {
                HASSERT(0 == layerDir->CountUniformFeatureDirs());

                // No Point dir ... we create one
                featureDir = layerDir->CreatePointsOnlyUniformFeatureDir(MASS_POINT_FEATURE_TYPE,
                                                                         IDTMFile::PointTypeIDTrait<POINT>::value,
                                                                         (m_compress) ? HTGFF::Compression::Deflate::Create() :
                                                                         HTGFF::Compression::None::Create());

                HASSERT(NULL != featureDir);
                if (NULL == featureDir)
                    return false;
                }


            m_tileHandler = TileHandler::CreateFrom(featureDir->GetPointDir());

            m_indexHandler = IndexHandler::CreateFrom(featureDir->GetSpatialIndexDir());
            if (NULL == m_indexHandler)
                {
                m_indexHandler = IndexHandler::CreateFrom(featureDir->CreateSpatialIndexDir(IndexHandler::Options(true, // TDORAY: Set correct value for is progressive
                                                                                            indexHeader->m_SplitTreshold)));
                HASSERT(NULL != m_indexHandler);
                if (NULL == m_indexHandler)
                    return false;
                }

            m_filteringDir = featureDir->GetFilteringDir();
            if (NULL == m_filteringDir)
                {
                // TDORAY: Match real filter type. Ask alain where to find this information.
                m_filteringDir = featureDir->CreateFilteringDir(IDTMFile::DumbFilteringHandler::Options());
                HASSERT(NULL != m_filteringDir);
                if (NULL == m_filteringDir)
                    return false;
                }
            }
        HASSERT(NULL != m_tileHandler);
        HASSERT(NULL != m_indexHandler);
        HASSERT(NULL != m_filteringDir);

        // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
        m_indexHandler->SetBalanced(indexHeader->m_balanced);


        if (indexHeader->m_rootNodeBlockID.m_integerInitialized)
            m_indexHandler->SetTopNode(ConvertBlockID(indexHeader->m_rootNodeBlockID));
        else
            m_indexHandler->SetTopNode(IDTMFile::GetNullNodeID());


        return true;
        }

    virtual size_t LoadMasterHeader (HGFPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {
        if (m_DTMFile == NULL)
            return 0;

        if (NULL == m_tileHandler)
            {
                IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir (m_layerID);
            if (NULL == layerDir)
                return 0;

                IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(MASS_POINT_FEATURE_TYPE);
            if (NULL == featureDir)
                return 0;

            m_tileHandler = TileHandler::CreateFrom(featureDir->GetPointDir());

            m_indexHandler = IndexHandler::CreateFrom(featureDir->GetSpatialIndexDir());
            if (NULL == m_indexHandler)
                return 0;

            m_filteringDir = featureDir->GetFilteringDir();
            if (NULL == m_filteringDir)
                return 0;

            }
        HASSERT(NULL != m_tileHandler);
        HASSERT(NULL != m_indexHandler);
        HASSERT(NULL != m_filteringDir);

        // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
        indexHeader->m_SplitTreshold = m_indexHandler->GetSplitTreshold();
        indexHeader->m_balanced = m_indexHandler->IsBalanced();




        if (m_indexHandler->GetTopNode() != IDTMFile::GetNullNodeID())
            indexHeader->m_rootNodeBlockID = m_indexHandler->GetTopNode();
        else
            indexHeader->m_rootNodeBlockID = HPMBlockID();

        return headerSize;
        }

    // New interface
    virtual HPMBlockID StoreNewBlock (POINT* DataTypeArray, size_t countData)
        {
        HPRECONDITION(m_tileHandler != NULL);
        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        PointArray MyArray(DataTypeArray, countData);

        IDTMFile::NodeID newNodeID;
        if (!m_tileHandler->AddPoints(newNodeID, MyArray))
            {
            HASSERT(!"Write failed!");
            throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
            }

        return HPMBlockID(newNodeID);
        }

    virtual HPMBlockID StoreBlock (POINT* DataTypeArray, size_t countData, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);
        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        if (!blockID.IsValid() || blockID.m_integerID == IDTMFile::SubNodesTable::GetNoSubNodeID())
            return StoreNewBlock (DataTypeArray, countData);

        PointArray arrayOfPoints (DataTypeArray, countData);

        if (!m_tileHandler->SetPoints(ConvertBlockID(blockID), arrayOfPoints))
            {
            HASSERT(!"Write failed!");
            throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
            }

        return blockID;
        }

    virtual size_t GetBlockDataCount (HPMBlockID blockID) const
        {
        HPRECONDITION(m_tileHandler != NULL);
        return m_tileHandler->GetDir().CountPoints(ConvertBlockID(blockID));

        }


    virtual size_t StoreHeader (HGFPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);
        HPRECONDITION(m_indexHandler != NULL);
        HPRECONDITION(m_filteringDir != NULL);
        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        if (header->m_contentExtentDefined)
            {
            HASSERT(ExtentOp<EXTENT>::GetXMin(header->m_contentExtent) >= ExtentOp<EXTENT>::GetXMin(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetXMax(header->m_contentExtent) <= ExtentOp<EXTENT>::GetXMax(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetYMin(header->m_contentExtent) >= ExtentOp<EXTENT>::GetYMin(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetYMax(header->m_contentExtent) <= ExtentOp<EXTENT>::GetYMax(header->m_nodeExtent));
            }

        m_tileHandler->GetDir().SetResolution (ConvertBlockID(blockID), (IDTMFile::ResolutionID)header->m_level);
        m_filteringDir->SetFiltered (ConvertBlockID(blockID), header->m_filtered);

        IDTMFile::SubNodesTable& rChildren = m_indexHandler->EditSubNodes(ConvertBlockID(blockID));

        if (header->m_IsLeaf)
            {
            rChildren[0] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            rChildren[1] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            rChildren[2] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            rChildren[3] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            }
        else if (!header->m_IsBranched)
            {
            rChildren[0] = ConvertChildID(header->m_SubNodeNoSplitID);
            rChildren[1] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            rChildren[2] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            rChildren[3] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            }
        else
            {
            rChildren[0] = ConvertChildID(header->m_apSubNodeID[0]);
            rChildren[1] = ConvertChildID(header->m_apSubNodeID[1]);
            rChildren[2] = ConvertChildID(header->m_apSubNodeID[2]);
            rChildren[3] = ConvertChildID(header->m_apSubNodeID[3]);
            }

        IDTMFile::Extent3d64f& rExtent = m_tileHandler->GetDir().EditExtent (ConvertBlockID(blockID));
        rExtent.xMin = ExtentOp<EXTENT>::GetXMin(header->m_nodeExtent);
        rExtent.yMin = ExtentOp<EXTENT>::GetYMin(header->m_nodeExtent);
        rExtent.zMin = ExtentOp<EXTENT>::GetZMin(header->m_nodeExtent);
        rExtent.xMax = ExtentOp<EXTENT>::GetXMax(header->m_nodeExtent);
        rExtent.yMax = ExtentOp<EXTENT>::GetYMax(header->m_nodeExtent);
        rExtent.zMax = ExtentOp<EXTENT>::GetZMax(header->m_nodeExtent);


        if (header->m_contentExtentDefined)
            {
            IDTMFile::Extent3d64f& rContentExtent = m_indexHandler->EditContentExtent (ConvertBlockID(blockID));
            rContentExtent.xMin = ExtentOp<EXTENT>::GetXMin(header->m_contentExtent);
            rContentExtent.yMin = ExtentOp<EXTENT>::GetYMin(header->m_contentExtent);
            rContentExtent.zMin = ExtentOp<EXTENT>::GetZMin(header->m_contentExtent);
            rContentExtent.xMax = ExtentOp<EXTENT>::GetXMax(header->m_contentExtent);
            rContentExtent.yMax = ExtentOp<EXTENT>::GetYMax(header->m_contentExtent);
            rContentExtent.zMax = ExtentOp<EXTENT>::GetZMax(header->m_contentExtent);
            }
        else
            {
            IDTMFile::Extent3d64f& rContentExtent = m_indexHandler->EditContentExtent (ConvertBlockID(blockID));
            rContentExtent.xMin = 0.0;
            rContentExtent.yMin = 0.0;
            rContentExtent.zMin = 0.0;
            rContentExtent.xMax = 0.0;
            rContentExtent.yMax = 0.0;
            rContentExtent.zMax = 0.0;
            }


        m_indexHandler->SetTotalPointCount(ConvertBlockID(blockID), header->m_totalCount);

        return 1;
        }

    virtual size_t LoadHeader (HGFPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);
        HPRECONDITION(m_indexHandler != NULL);
        HPRECONDITION(m_filteringDir != NULL);


        const IDTMFile::Extent3d64f& rExtent = m_tileHandler->GetDir().GetExtent (ConvertBlockID(blockID));
        ExtentOp<EXTENT>::SetXMin(header->m_nodeExtent, rExtent.xMin);
        ExtentOp<EXTENT>::SetYMin(header->m_nodeExtent, rExtent.yMin);
        ExtentOp<EXTENT>::SetZMin(header->m_nodeExtent, rExtent.zMin);
        ExtentOp<EXTENT>::SetXMax(header->m_nodeExtent, rExtent.xMax);
        ExtentOp<EXTENT>::SetYMax(header->m_nodeExtent, rExtent.yMax);
        ExtentOp<EXTENT>::SetZMax(header->m_nodeExtent, rExtent.zMax);

        header->m_level = m_tileHandler->GetDir().GetResolution (ConvertBlockID(blockID));
        header->m_filtered = m_filteringDir->IsFiltered (ConvertBlockID(blockID));

        const IDTMFile::SubNodesTable& rChildren = m_indexHandler->GetSubNodes(ConvertBlockID(blockID));

        if (rChildren[0] != IDTMFile::SubNodesTable::GetNoSubNodeID())
            header->m_SubNodeNoSplitID = rChildren[0];
        else
            header->m_SubNodeNoSplitID = HPMBlockID();

        size_t lastKnownGoodChild = 0;
        for (size_t indexNodes = 0 ; indexNodes < 8; indexNodes++)
            {
            if (rChildren[indexNodes] != IDTMFile::SubNodesTable::GetNoSubNodeID())
                {
                lastKnownGoodChild = indexNodes;
                header->m_apSubNodeID[indexNodes] = rChildren[indexNodes];
                }
            else
                header->m_apSubNodeID[indexNodes] = HPMBlockID();
            }

        header->m_IsLeaf = (!header->m_apSubNodeID[0].IsValid());
        header->m_IsBranched = !header->m_IsLeaf && (header->m_apSubNodeID[1].IsValid());

        header->m_SplitTreshold = m_indexHandler->GetSplitTreshold();
        // header->m_balanced = m_indexHandler->IsBalanced();


        const IDTMFile::Extent3d64f& rContentExtent = m_indexHandler->GetContentExtent (ConvertBlockID(blockID));
        ExtentOp<EXTENT>::SetXMin(header->m_contentExtent, rContentExtent.xMin);
        ExtentOp<EXTENT>::SetYMin(header->m_contentExtent, rContentExtent.yMin);
        ExtentOp<EXTENT>::SetZMin(header->m_contentExtent, rContentExtent.zMin);
        ExtentOp<EXTENT>::SetXMax(header->m_contentExtent, rContentExtent.xMax);
        ExtentOp<EXTENT>::SetYMax(header->m_contentExtent, rContentExtent.yMax);
        ExtentOp<EXTENT>::SetZMax(header->m_contentExtent, rContentExtent.zMax);

        header->m_contentExtentDefined =  (rContentExtent.xMin != 0.0 || rContentExtent.xMax != 0.0 || rContentExtent.yMin != 0.0 ||
                                           rContentExtent.yMax != 0.0 || rContentExtent.zMin != 0.0 || rContentExtent.zMax != 0.0);

        if (header->m_contentExtentDefined)
            {
            HASSERT(ExtentOp<EXTENT>::GetXMin(header->m_contentExtent) >= ExtentOp<EXTENT>::GetXMin(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetXMax(header->m_contentExtent) <= ExtentOp<EXTENT>::GetXMax(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetYMin(header->m_contentExtent) >= ExtentOp<EXTENT>::GetYMin(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetYMax(header->m_contentExtent) <= ExtentOp<EXTENT>::GetYMax(header->m_nodeExtent));
            }

        header->m_totalCountDefined = true;
        header->m_totalCount = m_indexHandler->GetTotalPointCount(ConvertBlockID(blockID));

        return 1;
        }


    virtual size_t LoadBlock (POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);

        PointArray arrayOfPoints;
        arrayOfPoints.WrapEditable(DataTypeArray, 0, maxCountData);

        if (!m_tileHandler->GetPoints(ConvertBlockID(blockID), arrayOfPoints))
            throw; //TDORAY: Do something else

        return 1;
        }

    virtual bool DestroyBlock (HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);
        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        return m_tileHandler->RemovePoints(ConvertBlockID(blockID));
        }

    static const IDTMFile::FeatureType MASS_POINT_FEATURE_TYPE = 0;

protected:
    const IDTMFile::File::Ptr& GetFileP () const
        {
        return m_DTMFile;
        }

private:

    IDTMFile::File::Ptr m_DTMFile;
    bool m_compress;
    typename TileHandler::Ptr m_tileHandler;
    IndexHandler::Ptr m_indexHandler;
        IDTMFile::FilteringDir* m_filteringDir;
    size_t m_layerID;
    bool m_receivedOpenedFile;

    };




template <class SPATIAL, class POINT, class EXTENT> class HGFFeatureTileStore: public IHPMPermanentStore<SPATIAL, HGFFeatureIndexHeader<EXTENT>, HGFFeatureNodeHeader<EXTENT>>
    {
public:
    HGFFeatureTileStore() {};
    virtual ~HGFFeatureTileStore() {};
    };


template <typename SPATIAL, typename POINT, typename EXTENT> class DTMFeatureTaggedTileStore : public HGFFeatureTileStore<SPATIAL, POINT, EXTENT>// , public HFCShareableObject<HGFFeatureTileStore<SPATIAL, POINT, EXTENT> >
    {
private:
    typedef IDTMFile::FeatureTileHandler<IDTMFile::Point3d64f> TileHandler;
        typedef IDTMFile::BTreeIndexHandler IndexHandler;

    static IDTMFile::NodeID ConvertBlockID (const HPMBlockID&   blockID)
        {
        //TDORAY: Put some checks here
        return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
        }

    static IDTMFile::SubNodesTable::value_type ConvertChildID (const HPMBlockID&   childID)
        {
        //TDORAY: Put some checks here
        return static_cast<IDTMFile::SubNodesTable::value_type>(childID.m_integerID);
        }

    struct IsSmallerSizeFeature : public binary_function<SPATIAL, SPATIAL, bool>
        {
        bool operator () (const SPATIAL& pi_rLeft, const SPATIAL& pi_rRight)
            {
            return pi_rLeft->GetSize() < pi_rRight->GetSize();
            }
        };

    struct AddToFeatureList : public unary_function<SPATIAL, void>
        {
        explicit        AddToFeatureList       (IDTMFeatureArray<IDTMFile::Point3d64f>& pi_rFeatureList,
                                                IDTMFile::Point3d64f*                   pi_pPointBuffer)
            :   m_rFeatureList(pi_rFeatureList),
                m_pPointBuffer(pi_pPointBuffer) {}

        void            operator()             (const SPATIAL&              pi_rRight)
            {
            HPRECONDITION(NULL != pi_rRight);

            std::transform(&pi_rRight->GetStartPoint(), &pi_rRight->GetStartPoint() + pi_rRight->GetSize(), m_pPointBuffer,
                           IndexPointToStoragePoint());

            m_rFeatureList.Append(pi_rRight->GetFeatureType(),
                                  m_pPointBuffer,
                                  m_pPointBuffer + pi_rRight->GetSize());
            }

    private:

        struct IndexPointToStoragePoint : public unary_function<POINT, IDTMFile::Point3d64f>
            {
            IDTMFile::Point3d64f    operator()             (const POINT&                pi_rRight) const
                {
                // TDORAY: Bad... Will loose other dimensions (like m)
                return PointOp<IDTMFile::Point3d64f>::Create(PointOp<POINT>::GetX(pi_rRight),
                                                             PointOp<POINT>::GetY(pi_rRight),
                                                             PointOp<POINT>::GetZ(pi_rRight));
                }
            };

        IDTMFeatureArray<IDTMFile::Point3d64f>&  m_rFeatureList;
        IDTMFile::Point3d64f*                   m_pPointBuffer;
        };

protected:


public:
    // Constructor / Destroyer
    DTMFeatureTaggedTileStore(const char* filename, const IDTMFile::AccessMode& accessMode, bool compress, size_t layerID = 0)
        {
        m_receivedOpenedFile = false;

        int ret = _access(filename, 00);

        m_DTMFile = IDTMFile::Open (filename, accessMode);


        if (m_DTMFile == NULL)
            throw;

        m_compress = compress;

        m_layerID = layerID;
        }


    DTMFeatureTaggedTileStore(IDTMFile::File::Ptr openedDTMFile, bool compress, size_t layerID = 0)
        {
        m_receivedOpenedFile = true;

        m_DTMFile = openedDTMFile;

        if (m_DTMFile == NULL)
            throw;

        m_compress = compress;

        m_layerID = layerID;
        }

    virtual ~DTMFeatureTaggedTileStore ()
        {
        if (!m_receivedOpenedFile)
            if (m_DTMFile != NULL)
                m_DTMFile->Close ();
        }

    // ITileStore interface
    virtual void Close ()
        {
        if (!m_receivedOpenedFile)
            if (m_DTMFile != NULL)
                m_DTMFile->Close ();

        m_DTMFile = NULL;
        }

    virtual bool StoreMasterHeader (HGFFeatureIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {
        if (m_DTMFile == NULL)
            return false;

        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        if (NULL == m_tileHandler)
            {
                IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir (m_layerID);

            if (NULL == layerDir)
                {
                layerDir = m_DTMFile->GetRootDir()->AddLayerDir ();
                HASSERT(NULL != layerDir);
                if (NULL == layerDir)
                    return false;

                m_layerID = layerDir->GetIndex();
                HASSERT(0 == m_layerID);
                }

            m_tileHandler = TileHandler::CreateFrom(layerDir->GetMixedFeatureDir(0));
            if (NULL == m_tileHandler)
                {
                HASSERT(0 == layerDir->CountMixedFeatureDirs());

                // No feature dir ... we create one
                m_tileHandler =
                    TileHandler::CreateFrom(layerDir->AddMixedFeatureDir(IDTMFile::PointTypeIDTrait<IDTMFile::Point3d64f>::value,
                                                                         (m_compress) ? HTGFF::Compression::Deflate::Create() :
                                                                         HTGFF::Compression::None::Create()));

                HASSERT(NULL != m_tileHandler);
                if (NULL == m_tileHandler)
                    return false;
                }

            m_indexHandler = IndexHandler::CreateFrom(m_tileHandler->GetDir().GetSpatialIndexDir());
            if (NULL == m_indexHandler)
                {
                m_indexHandler
                = IndexHandler::CreateFrom(m_tileHandler->GetDir().CreateSpatialIndexDir(IndexHandler::Options(true, // TDORAY: Set correct value for is progressive
                                                                                         indexHeader->m_SplitTreshold)));
                HASSERT(NULL != m_indexHandler);
                if (NULL == m_indexHandler)
                    return false;
                }
            }
        HASSERT(NULL != m_tileHandler);
        HASSERT(NULL != m_indexHandler);

        // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
        m_indexHandler->SetBalanced(indexHeader->m_balanced);


        if (indexHeader->m_rootNodeBlockID.m_integerInitialized)
            m_indexHandler->SetTopNode(ConvertBlockID(indexHeader->m_rootNodeBlockID));
        else
            m_indexHandler->SetTopNode(IDTMFile::GetNullNodeID());

        return true;
        }

    virtual size_t LoadMasterHeader (HGFFeatureIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {
        if (m_DTMFile == NULL)
            return 0;

        if (NULL == m_tileHandler)
            {
                IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir (m_layerID);
            if (NULL == layerDir)
                return 0;

            m_tileHandler = TileHandler::CreateFrom(layerDir->GetMixedFeatureDir(0));
            if (NULL == m_tileHandler)
                return 0;

            m_indexHandler = IndexHandler::CreateFrom(m_tileHandler->GetDir().GetSpatialIndexDir());
            if (NULL == m_indexHandler)
                return 0;
            }
        HASSERT(NULL != m_tileHandler);
        HASSERT(NULL != m_indexHandler);

        // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
        indexHeader->m_SplitTreshold = m_indexHandler->GetSplitTreshold();
        indexHeader->m_balanced = m_indexHandler->IsBalanced();



        if (m_indexHandler->GetTopNode() != IDTMFile::GetNullNodeID())
            indexHeader->m_rootNodeBlockID = m_indexHandler->GetTopNode();
        else
            indexHeader->m_rootNodeBlockID = HPMBlockID();

        return headerSize;
        }

    // New interface
    virtual HPMBlockID StoreNewBlock (SPATIAL* DataTypeArray, size_t countData)
        {
        HPRECONDITION(m_tileHandler != NULL);

        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        // Compute how many points we need to reserve for the point conversion
        const size_t FeatureMaxPointQty = (0 == countData) ? 0 : (*std::max_element(DataTypeArray, DataTypeArray + countData,
                                                                                    IsSmallerSizeFeature()))->GetSize();
        // Compute how many points we need to reserve for the feature list
        size_t FeatureTotalPointQty = 0;
        std::transform(DataTypeArray, DataTypeArray + countData, BentleyApi::ImagePP::AccumulateIter(FeatureTotalPointQty),
                       mem_fun(&SPATIAL::POINTED_TYPE::GetSize));

        // Convert index features to store features
        IDTMFeatureArray<IDTMFile::Point3d64f> newFeatureBlock(countData, FeatureTotalPointQty);
        HArrayAutoPtr<IDTMFile::Point3d64f> pointsConversionBuffer(new IDTMFile::Point3d64f[FeatureMaxPointQty]);
        for_each(DataTypeArray, DataTypeArray + countData, AddToFeatureList(newFeatureBlock,
                                                                            pointsConversionBuffer.get()));

        // Save to feature dir
        IDTMFile::NodeID newNodeID;
        if (!m_tileHandler->AddFeatures (newNodeID, newFeatureBlock))
            {
            HASSERT(!"Write failed!");
            throw HFCWriteFaultException( L"Unable to obtain file name ... IDTMFile::File API should be modified.");
            }

        return HPMBlockID(newNodeID);
        }

    virtual HPMBlockID StoreBlock (SPATIAL* DataTypeArray, size_t countData, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);

        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        if (!blockID.IsValid() || blockID.m_integerID == IDTMFile::SubNodesTable::GetNoSubNodeID())
            return StoreNewBlock (DataTypeArray, countData);

        // Compute how many points we need to reserve for the point conversion
        const size_t FeatureMaxPointQty = (0 == countData) ? 0 : (*std::max_element(DataTypeArray, DataTypeArray + countData,
                                                                                    IsSmallerSizeFeature()))->GetSize();

        // Compute how many points we need to reserve for the feature list
        size_t FeatureTotalPointQty = 0;
        std::transform(DataTypeArray, DataTypeArray + countData, BentleyApi::ImagePP::AccumulateIter(FeatureTotalPointQty),
                       mem_fun(&SPATIAL::POINTED_TYPE::GetSize));

        // Convert index features to store features
        IDTMFeatureArray<IDTMFile::Point3d64f> featureBlock(countData, FeatureTotalPointQty);
        HArrayAutoPtr<IDTMFile::Point3d64f> pointsConversionBuffer(new IDTMFile::Point3d64f[FeatureMaxPointQty]);
        for_each(DataTypeArray, DataTypeArray + countData, AddToFeatureList(featureBlock,
                                                                            pointsConversionBuffer.get()));

        if (!m_tileHandler->SetFeatures (ConvertBlockID(blockID), featureBlock))
            {
            HASSERT(!"Write failed!");
            throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
            }

        return blockID;
        }

    virtual size_t GetBlockDataCount (HPMBlockID blockID) const
        {
        HPRECONDITION(m_tileHandler != NULL);

        return m_tileHandler->GetDir().CountFeatures(ConvertBlockID(blockID));

        }


    virtual size_t StoreHeader (HGFFeatureNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);
        HPRECONDITION(m_indexHandler != NULL);

        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        IDTMFile::SubNodesTable& rChildren = m_indexHandler->EditSubNodes(ConvertBlockID(blockID));

        if (header->m_IsLeaf)
            {
            rChildren[0] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            rChildren[1] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            rChildren[2] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            rChildren[3] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            }
        else if (!header->m_IsBranched)
            {
            rChildren[0] = ConvertChildID(header->m_SubNodeNoSplitID);
            rChildren[1] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            rChildren[2] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            rChildren[3] = IDTMFile::SubNodesTable::GetNoSubNodeID();
            }
        else
            {
            rChildren[0] = ConvertChildID(header->m_apSubNodeID[0]);
            rChildren[1] = ConvertChildID(header->m_apSubNodeID[1]);
            rChildren[2] = ConvertChildID(header->m_apSubNodeID[2]);
            rChildren[3] = ConvertChildID(header->m_apSubNodeID[3]);
            }

        if (header->m_contentExtentDefined)
            {
            IDTMFile::Extent3d64f& rContentExtent = m_indexHandler->EditContentExtent (ConvertBlockID(blockID));
            rContentExtent.xMin = ExtentOp<EXTENT>::GetXMin(header->m_contentExtent);
            rContentExtent.yMin = ExtentOp<EXTENT>::GetYMin(header->m_contentExtent);
            rContentExtent.zMin = ExtentOp<EXTENT>::GetZMin(header->m_contentExtent);
            rContentExtent.xMax = ExtentOp<EXTENT>::GetXMax(header->m_contentExtent);
            rContentExtent.yMax = ExtentOp<EXTENT>::GetYMax(header->m_contentExtent);
            rContentExtent.zMax = ExtentOp<EXTENT>::GetZMax(header->m_contentExtent);
            }
        else
            {
            IDTMFile::Extent3d64f& rContentExtent = m_indexHandler->EditContentExtent (ConvertBlockID(blockID));
            rContentExtent.xMin = 0.0;
            rContentExtent.yMin = 0.0;
            rContentExtent.zMin = 0.0;
            rContentExtent.xMax = 0.0;
            rContentExtent.yMax = 0.0;
            rContentExtent.zMax = 0.0;
            }

        IDTMFile::Extent3d64f& rExtent = m_tileHandler->GetDir().EditExtent (ConvertBlockID(blockID));
        rExtent.xMin = ExtentOp<EXTENT>::GetXMin(header->m_nodeExtent);
        rExtent.yMin = ExtentOp<EXTENT>::GetYMin(header->m_nodeExtent);
        rExtent.zMin = ExtentOp<EXTENT>::GetZMin(header->m_nodeExtent);
        rExtent.xMax = ExtentOp<EXTENT>::GetXMax(header->m_nodeExtent);
        rExtent.yMax = ExtentOp<EXTENT>::GetYMax(header->m_nodeExtent);
        rExtent.zMax = ExtentOp<EXTENT>::GetZMax(header->m_nodeExtent);

        m_tileHandler->GetDir().SetResolution (ConvertBlockID(blockID), (uint32_t)header->m_level);

        m_indexHandler->SetTotalPointCount(ConvertBlockID(blockID), header->m_totalCount);

        return 1;
        }

    virtual size_t LoadHeader (HGFFeatureNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);
        HPRECONDITION(m_indexHandler != NULL);

        const IDTMFile::SubNodesTable& rChildren = m_indexHandler->GetSubNodes(ConvertBlockID(blockID));

        if (rChildren[0] != IDTMFile::SubNodesTable::GetNoSubNodeID())
            header->m_SubNodeNoSplitID = rChildren[0];
        else
            header->m_SubNodeNoSplitID = HPMBlockID();

        size_t lastKnownGoodChild = 0;
        for (size_t indexNodes = 0 ; indexNodes < 8; indexNodes++)
            {
            if (rChildren[indexNodes] != IDTMFile::SubNodesTable::GetNoSubNodeID())
                {
                lastKnownGoodChild = indexNodes;
                header->m_apSubNodeID[indexNodes] = rChildren[indexNodes];
                }
            else
                header->m_apSubNodeID[indexNodes] = HPMBlockID();
            }

        header->m_IsLeaf = (!header->m_apSubNodeID[0].IsValid());
        header->m_IsBranched = !header->m_IsLeaf && (header->m_apSubNodeID[1].IsValid());



        const IDTMFile::Extent3d64f& rContentExtent = m_indexHandler->GetContentExtent (ConvertBlockID(blockID));
        header->m_contentExtent = ExtentOp<EXTENT>::Create(rContentExtent.xMin, rContentExtent.yMin, rContentExtent.zMin,
                                                           rContentExtent.xMax, rContentExtent.yMax, rContentExtent.zMax);

        header->m_contentExtentDefined =  (rContentExtent.xMin != 0.0 || rContentExtent.xMax != 0.0 || rContentExtent.yMin != 0.0 ||
                                           rContentExtent.yMax != 0.0 || rContentExtent.zMin != 0.0 || rContentExtent.zMax != 0.0);


        header->m_SplitTreshold = m_indexHandler->GetSplitTreshold();
        header->m_balanced = m_indexHandler->IsBalanced();

        const IDTMFile::Extent3d64f& rExtent = m_tileHandler->GetDir().GetExtent (ConvertBlockID(blockID));
        header->m_nodeExtent = ExtentOp<EXTENT>::Create(rExtent.xMin, rExtent.yMin, rExtent.zMin, rExtent.xMax, rExtent.yMax, rExtent.zMax);

        header->m_level = m_tileHandler->GetDir().GetResolution (ConvertBlockID(blockID));

        header->m_totalCountDefined = true;
        header->m_totalCount = m_indexHandler->GetTotalPointCount(ConvertBlockID(blockID));

        return 1;
        }


    virtual size_t LoadBlock (SPATIAL* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);

        IDTMFeatureArray<IDTMFile::Point3d64f> featureBlock;

        if (!m_tileHandler->GetFeatures(ConvertBlockID(blockID), featureBlock))
            throw;

        // Fill in the feature list with features
        size_t countData = 0;
        for (IDTMFeatureArray<IDTMFile::Point3d64f>::const_iterator myFeature = featureBlock.Begin(); countData < maxCountData && myFeature != featureBlock.End() ; myFeature++)
            {
            DataTypeArray[countData] = new HVEDTMLinearFeature(myFeature->GetType(), myFeature->GetSize());
            DataTypeArray[countData]->SetAutoToleranceActive(false);
            for (IDTMFeatureArray<IDTMFile::Point3d64f>::value_type::const_iterator myPoint = myFeature->Begin(); myPoint != myFeature->End() ; ++myPoint)
                {

                // TDORAY: Bad... Will loose other dimensions (like m)
                DataTypeArray[countData]->AppendPoint(PointOp<POINT>::Create(PointOp<IDTMFile::Point3d64f>::GetX(*myPoint),
                                                                             PointOp<IDTMFile::Point3d64f>::GetY(*myPoint),
                                                                             PointOp<IDTMFile::Point3d64f>::GetZ(*myPoint)));
                }
            countData++;
            }

        return countData;
        }

    virtual bool DestroyBlock (HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);
        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        return m_tileHandler->RemoveFeatures(ConvertBlockID(blockID));
        }

protected:
    const IDTMFile::File::Ptr& GetFileP () const
        {
        return m_DTMFile;
        }

private:
    IDTMFile::File::Ptr m_DTMFile;
    bool m_compress;
    typename TileHandler::Ptr m_tileHandler;
    IndexHandler::Ptr m_indexHandler;

    size_t m_layerID;
    bool m_receivedOpenedFile;
    };










template <typename SPATIAL, typename POINT, typename EXTENT, typename FEATUREPOINT = POINT, typename FEATUREEXTENT = EXTENT> class DTMTaggedTileStore : public DTMFeatureTaggedTileStore<SPATIAL, FEATUREPOINT, FEATUREEXTENT>,
    public DTMPointTaggedTileStore<POINT, EXTENT>
    {
public:
    // Constructor / Destroyer
    DTMTaggedTileStore(const char* filename, const IDTMFile::AccessMode& accessMode, bool compress, size_t layerID = 0)
        : DTMFeatureTaggedTileStore(OpenIDTMFile(filename, accessMode), compress, layerID),
          DTMPointTaggedTileStore (DTMFeatureTaggedTileStore::GetFileP(), compress, layerID)
        {
        // Nothing to do!
        }

    DTMTaggedTileStore(IDTMFile::File::Ptr openedDTMFile, const IDTMFile::AccessMode& accessMode, bool compress, size_t layerID = 0)
        : DTMFeatureTaggedTileStore(openedDTMFile, compress, layerID),
          DTMPointTaggedTileStore (openedDTMFile, compress, layerID)
        {
        // Nothing to do!
        }



    ~DTMTaggedTileStore()
        {
        if (DTMFeatureTaggedTileStore::GetFileP() != NULL)
            DTMFeatureTaggedTileStore::GetFileP()->Close ();
        }


    size_t GetPointLayerID() const
        {
        return DTMPointTileStore::m_layerID;
        }

    size_t GetFeatureLayerID() const
        {
        return DTMFeatureTileStore::m_layerID;
        }



protected:
    IDTMFile::File::Ptr OpenIDTMFile (const char* filename, const IDTMFile::AccessMode& accessMode)
        {
        IDTMFile::File::Ptr SuperDTMFile;
            //size_t SuperlayerID;

        SuperDTMFile = IDTMFile::File::Open (filename, accessMode);
        if (SuperDTMFile == NULL)
            throw;



        return SuperDTMFile;

        }

private:

    };

