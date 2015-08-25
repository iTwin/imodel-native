//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMPointTileStore.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once



#include <ImagePP/all/h/HPMDataStore.h>
#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/IDTMFile.h>
#include <ImagePP/all/h/HGFSpatialIndex.h>
#include <ImagePP/all/h/HVEDTMLinearFeature.h>
#include <ImagePP/all/h/HGFPointTileStore.h>
#if 0
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

     static bool IsSpatialInExtent3D (const Spatial_t& spatialObject, const Extent_t& extent)
        {
        const Extent_t& spatialExtent = SpatialOp<Spatial_t, Coord_t, Extent_t>::GetExtent(spatialObject);

        return ((ExtentOp<Extent_t>::GetXMin(spatialExtent) >= ExtentOp<Extent_t>::GetXMin(extent)) &&
                (ExtentOp<Extent_t>::GetYMin(spatialExtent) >= ExtentOp<Extent_t>::GetYMin(extent)) &&
                (ExtentOp<Extent_t>::GetZMin(spatialExtent) >= ExtentOp<Extent_t>::GetZMin(extent)) &&
                (ExtentOp<Extent_t>::GetXMax(spatialExtent) <= ExtentOp<Extent_t>::GetXMax(extent)) &&
                (ExtentOp<Extent_t>::GetYMax(spatialExtent) <= ExtentOp<Extent_t>::GetYMax(extent)) &&
                (ExtentOp<Extent_t>::GetZMax(spatialExtent) <= ExtentOp<Extent_t>::GetZMax(extent)));      
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
        return Create(min(GetXMin(extent1), GetXMin(extent2)),
                      min(GetYMin(extent1), GetYMin(extent2)),
                      min(GetZMin(extent1), GetZMin(extent2)),
                      max(GetXMax(extent1), GetXMax(extent2)),
                      max(GetYMax(extent1), GetYMax(extent2)),
                      max(GetZMax(extent1), GetZMax(extent2)));
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

    __forceinline static bool IsSpatialInExtent3D (const SPATIAL& spatial, const  IDTMFile::Extent3d64f& extent)
        {
        return ((PointOp<SPATIAL>::GetX(spatial) >= extent.xMin) &&
                (PointOp<SPATIAL>::GetX(spatial) <= extent.xMax) &&
                (PointOp<SPATIAL>::GetY(spatial) >= extent.yMin) &&
                (PointOp<SPATIAL>::GetY(spatial) <= extent.yMax) &&
                (PointOp<SPATIAL>::GetZ(spatial) >= extent.zMin) &&
                (PointOp<SPATIAL>::GetZ(spatial) <= extent.zMax));
        }

    static size_t GetPointCount (const SPATIAL& spatial)
        {
        return 1;
        }

    };
#endif

    /** -----------------------------------------------------------------------------

    The index node header is the base header for any spatial index nodes control. Since the
    spatial index node receives the actual node header type in parameter, the actual
    node header can be another however ALL members using the same name and type must be defined
    and be public for the provided node header. It is usually a lot easier to
    inherit any actual node header from the provided HGFIndexNodeHeader.
    -----------------------------------------------------------------------------
    */
    template <typename EXTENT> class SMIndexNodeHeader
        {
        public:

            size_t      m_SplitTreshold;            // Holds the split treshold
            bool        m_IsLeaf;                   // Indicates if the node is a leaf
            bool        m_IsBranched;               // Indicates if the node is branched. This field is basically only used between node content load time and
            // unsplit sub-node construction as the presence of the unsplit sub-node is the indication
            // of split or unsplit parentage.
            bool        m_IsUnSplitSubLevel;        // Important control variable ... indicates if the node is the sub-node of an unsplit parent
            EXTENT      m_nodeExtent;               // The extent of the node (this extent is always defined immediately at the creation of the node)
            EXTENT      m_contentExtent;            // The extent of the content of the node ... this is not the same as the extent of the node.
            bool        m_contentExtentDefined;     // Indicates if the content extent has been initialised. Even if node is empty the content extent
            // can have been initialized since this extent includes sub-nodes content.
            size_t      m_level;                    // The level depth of the node in the index tree.
            bool        m_balanced;                 // Control variable that indicates if the tree must be balanced
            size_t      m_numberOfSubNodesOnSplit;  // Control value that hold either 4 or 8 to indicate if a quadtree or octtree is used.

            bool        m_totalCountDefined;         // Indicates if the total count of objects in node and subnode is up to date
            uint64_t      m_totalCount;                // This value indicates the total number of points in node all recursively all sub-nodes.
            bool        m_arePoints3d;               //Indicates if the node contains 3D points or 2.5D points only. 


            //INFORMATION NOT PERSISTED
            struct RLC3dPoints
                {
                RLC3dPoints(size_t startIndex, size_t length)
                    {
                    m_startIndex = startIndex;
                    m_length = length;
                    }

                size_t m_startIndex;
                size_t m_length;
                };

            vector<RLC3dPoints> m_3dPointsDescBins;   //Determine if a group of points is 3D or not. 
        };


    /** -----------------------------------------------------------------------------

    The index header is the base header for any spatial index control. Since the
    spatial index receives the actual index header type in parameter, the actual
    index header can be another however ALL members using the same name and type must be defined
    and be public for the provided header type. It is usually a lot easier to
    inherit any actual index header from the provided HGFSpatialIndexHeader.
    -----------------------------------------------------------------------------
    */
    template <class EXTENT> class SMSpatialIndexHeader
        {
        public:

            size_t                  m_SplitTreshold;                // Holds the split treshold
            EXTENT                  m_MaxExtent;                    // Indicates the maximum extent if the spatial index is extent limited
            bool                    m_HasMaxExtent;                 // indicated if the index is extent limited.

            bool                    m_balanced;                     // Control variable that indicates if the tree must be balanced
            size_t                  m_numberOfSubNodesOnSplit;      // Control value that hold either 4 or 8 to indicate if a quadtree or octtree is used.
            size_t                  m_depth;                        // Cached (maximum) number of levels in the tree.
        };



template <typename EXTENT> class SMPointNodeHeader : public SMIndexNodeHeader<EXTENT>
    {
public:
   
    HPMBlockID  m_parentNodeID; //Required when loading 
    vector<HPMBlockID>  m_apSubNodeID;
    HPMBlockID  m_SubNodeNoSplitID;
    bool        m_filtered;    
    double      m_ViewDependentMetrics[9];

    //NEEDS_WORK_SM - m_meshed ?
    size_t      m_nbFaceIndexes;
    HPMBlockID  m_graphID;
    size_t      m_numberOfMeshComponents;
    int*        m_meshComponents;


    vector<HPMBlockID> m_apNeighborNodeID[IDTMFile::NeighborNodesTable::MAX_QTY];    
    bool               m_apAreNeighborNodesStitched[IDTMFile::NeighborNodesTable::MAX_QTY];    

    ~SMPointNodeHeader()
         {
              //  if (nullptr != m_meshComponents) delete[] m_meshComponents;
         }
    };


template <class EXTENT> class SMPointIndexHeader: public SMSpatialIndexHeader<EXTENT>
    {
public:

    HPMBlockID              m_rootNodeBlockID;
    };



template <class POINT, class EXTENT> class SMPointTileStore: public IHPMPermanentStore<POINT, SMPointIndexHeader<EXTENT>, SMPointNodeHeader<EXTENT>>
    {
public:
    SMPointTileStore() {};
    virtual ~SMPointTileStore() {};
    };


template <typename POINT, typename EXTENT> class SMPointTaggedTileStore : public SMPointTileStore<POINT, EXTENT>// , public HFCShareableObject<SMPointTileStore<POINT, EXTENT> >
    {
private:
    typedef IDTMFile::PointTileHandler<POINT> TileHandler;
        typedef IDTMFile::BTreeIndexHandler IndexHandler;

    typedef typename TileHandler::PointArray PointArray;

    static IDTMFile::NodeID ConvertBlockID (const HPMBlockID& blockID)
        {
        return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
        }

    static IDTMFile::VariableSubNodesTable::value_type ConvertChildID(const HPMBlockID& childID)
        {
        return static_cast<IDTMFile::VariableSubNodesTable::value_type>(childID.m_integerID);
        }

    static IDTMFile::NeighborNodesTable::value_type ConvertNeighborID (const HPMBlockID& neighborID)
        {
        return static_cast<IDTMFile::NeighborNodesTable::value_type>(neighborID.m_integerID);
        }

public:
    // Constructor / Destroyer
    SMPointTaggedTileStore(const char* filename, const IDTMFile::AccessMode& accessMode, bool compress, size_t layerID = 0)
        {
        m_receivedOpenedFile = false;

        m_DTMFile = IDTMFile::Open (filename, accessMode);

        if (m_DTMFile == NULL)
            throw;

        m_compress = compress;

        m_layerID = layerID;

        }

    SMPointTaggedTileStore(IDTMFile::File::Ptr openedDTMFile, bool compress, size_t layerID = 0)
            :   m_filteringDir(0),
                m_layerID(layerID),
            m_compress(compress),
            m_receivedOpenedFile(true),
            m_DTMFile(openedDTMFile)
        {
        if (m_DTMFile == NULL)
            throw;
        }

    virtual ~SMPointTaggedTileStore ()
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

    virtual bool StoreMasterHeader (SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
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
                    indexHeader!= NULL ? indexHeader->m_SplitTreshold : 0)));
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
        if (indexHeader != NULL)
            {
            // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
            m_indexHandler->SetBalanced(indexHeader->m_balanced);
            m_indexHandler->SetDepth((uint32_t)indexHeader->m_depth);

            if (indexHeader->m_rootNodeBlockID.m_integerInitialized)
                m_indexHandler->SetTopNode(ConvertBlockID(indexHeader->m_rootNodeBlockID));
            else
                m_indexHandler->SetTopNode(IDTMFile::GetNullNodeID());
            }

        return true;
        }

    virtual size_t LoadMasterHeader (SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {
        if (m_DTMFile == NULL)
            return 0;
        IDTMFile::BTreeIndexHandler::s_FixedSizeSubNodes = false;
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
        if (indexHeader != NULL)
            {
            // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
            indexHeader->m_SplitTreshold = m_indexHandler->GetSplitTreshold();
            indexHeader->m_balanced = m_indexHandler->IsBalanced();

            indexHeader->m_depth = m_indexHandler->GetDepth();


            if (m_indexHandler->GetTopNode() != IDTMFile::GetNullNodeID())
                indexHeader->m_rootNodeBlockID = m_indexHandler->GetTopNode();
            else
                indexHeader->m_rootNodeBlockID = HPMBlockID();
            }
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
            //&&MM TODO we cannot use custom exception string. AND the message seems wrong.
            throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
            }

        return HPMBlockID(newNodeID);
        }

    virtual HPMBlockID StoreBlock (POINT* DataTypeArray, size_t countData, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);
        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        if (!blockID.IsValid() || blockID.m_integerID == IDTMFile::VariableSubNodesTable::GetNoSubNodeID())
            return StoreNewBlock (DataTypeArray, countData);

        PointArray arrayOfPoints (DataTypeArray, countData);

        if (!m_tileHandler->SetPoints(ConvertBlockID(blockID), arrayOfPoints))
            {
            HASSERT(!"Write failed!");
            //&&MM TODO we cannot use custom exception string. AND the message seems wrong.
            throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
            }

        return blockID;
        }

    virtual size_t GetBlockDataCount (HPMBlockID blockID) const
        {
        HPRECONDITION(m_tileHandler != NULL);
        return m_tileHandler->GetDir().CountPoints(ConvertBlockID(blockID));

        }


    virtual size_t StoreHeader (SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);
        HPRECONDITION(m_indexHandler != NULL);
        HPRECONDITION(m_filteringDir != NULL);
        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        if (header->m_contentExtentDefined)
            {
            //NEEDS_WORK_SM : Currently stitching triangles most often cross the node extent.
            /*
            HASSERT(ExtentOp<EXTENT>::GetXMin(header->m_contentExtent) >= ExtentOp<EXTENT>::GetXMin(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetXMax(header->m_contentExtent) <= ExtentOp<EXTENT>::GetXMax(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetYMin(header->m_contentExtent) >= ExtentOp<EXTENT>::GetYMin(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetYMax(header->m_contentExtent) <= ExtentOp<EXTENT>::GetYMax(header->m_nodeExtent));
            */
            }

        m_tileHandler->GetDir().SetResolution (ConvertBlockID(blockID), (IDTMFile::ResolutionID)header->m_level);
        m_filteringDir->SetFiltered (ConvertBlockID(blockID), header->m_filtered);                

        if (header->m_parentNodeID.IsValid())
            {
            m_indexHandler->EditParentNode(ConvertBlockID(blockID)) = ConvertBlockID(header->m_parentNodeID);
            }
        else
            {
            m_indexHandler->EditParentNode(ConvertBlockID(blockID)) = IDTMFile::GetNullNodeID();
            }

        IDTMFile::VariableSubNodesTable& rChildren = m_indexHandler->EditSubNodesVar(ConvertBlockID(blockID));
        
        size_t nbChildren = header->m_IsLeaf || (!header->m_IsBranched  && !header->m_SubNodeNoSplitID.IsValid())? 0 : (!header->m_IsBranched ? 1 : header->m_numberOfSubNodesOnSplit);


        rChildren.SetNbVarData((uint32_t)nbChildren);

        IDTMFile::NodeInfo* childNodeInfo = m_indexHandler->EditSubNodesVarData(rChildren, nbChildren);

        assert(childNodeInfo != 0 || rChildren.GetNbVarData() == 0);

        if (rChildren.GetNbVarData() > 0)
            {
            if (nbChildren > 1)
                {
                for (size_t childInd = 0; childInd < nbChildren; childInd++)
                    {
                    childNodeInfo[childInd].m_nodePos = (uint8_t)childInd;
                    childNodeInfo[childInd].m_nodeId = ConvertChildID(header->m_apSubNodeID[childInd]);
                    }
                }
            else
                {
                childNodeInfo[0].m_nodePos = 0;
                childNodeInfo[0].m_nodeId = header->m_SubNodeNoSplitID.IsValid() ? ConvertChildID(header->m_SubNodeNoSplitID) : ConvertChildID(header->m_apSubNodeID[0]);
                }
            }
       
        IDTMFile::NeighborNodesTable& rNeighbors = m_indexHandler->EditNeighborNodes(ConvertBlockID(blockID));        

        size_t nbNeighbors = 0; 

        for (int neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd ++)
            {       
            rNeighbors.m_AreNeighborTilesStitched[neighborPosInd] = header->m_apAreNeighborNodesStitched[neighborPosInd];
            nbNeighbors += header->m_apNeighborNodeID[neighborPosInd].size();            
            }

        rNeighbors.SetNbVarData((uint32_t)nbNeighbors);
        
        IDTMFile::NodeInfo* neighborNodeInfo = m_indexHandler->EditNeighborNodesVarData (rNeighbors);
            
        assert(neighborNodeInfo != 0 || rNeighbors.GetNbVarData() == 0);

        size_t neighborInfoInd = 0; 

        if (rNeighbors.GetNbVarData() > 0)
            {        
            for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
                {       
                for (size_t neighborInd = 0; neighborInd < header->m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
                    {
                    neighborNodeInfo[neighborInfoInd].m_nodePos = (uint8_t)neighborPosInd;
                    neighborNodeInfo[neighborInfoInd].m_nodeId = ConvertNeighborID(header->m_apNeighborNodeID[neighborPosInd][neighborInd]);                                     
                    neighborInfoInd++;
                    }                                     
                }
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
        m_indexHandler->SetArePoints3d(ConvertBlockID(blockID), header->m_arePoints3d);  

        /*
        assert(header->m_3dPointsDescBins.size() <= USHORT_MAX);
        m_indexHandler->SetNb3dPointsBins(ConvertBlockID(blockID), header->m_3dPointsDescBins.size());  
        */
        
        
        m_indexHandler->SetMeshIndexesCount(ConvertBlockID(blockID), header->m_nbFaceIndexes);
        if (header->m_graphID.IsValid())
            {
            m_indexHandler->EditGraphBlockID(ConvertBlockID(blockID)) = ConvertBlockID(header->m_graphID);
            }
        else
            {
            m_indexHandler->EditGraphBlockID(ConvertBlockID(blockID)) = IDTMFile::GetNullNodeID();
            }
        m_indexHandler->SetMeshComponentsCount(ConvertBlockID(blockID), header->m_numberOfMeshComponents);
        if (header->m_numberOfMeshComponents > 0)
            {
            IDTMFile::MeshComponentsList& list = m_indexHandler->EditMeshComponents(ConvertBlockID(blockID));
            int* allComponents = m_indexHandler->EditMeshComponentsVarData(list, header->m_numberOfMeshComponents);
            memcpy(allComponents, header->m_meshComponents, header->m_numberOfMeshComponents * sizeof(int));
            }

        return 1;
        }

    virtual size_t LoadHeader (SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
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
        header->m_parentNodeID = m_indexHandler->GetParentNode(ConvertBlockID(blockID));   

        const IDTMFile::VariableSubNodesTable& rChildren = m_indexHandler->GetSubNodesVar(ConvertBlockID(blockID));
        header->m_numberOfSubNodesOnSplit = rChildren.GetNbVarData();
        header->m_apSubNodeID.resize(header->m_numberOfSubNodesOnSplit);
        if (rChildren.GetNbVarData() > 0)
            {

            const IDTMFile::NodeInfo* pChildrenInfo(m_indexHandler->GetSubNodesVarData(rChildren));

            for (size_t indexNodes = 0; indexNodes < (size_t)rChildren.GetNbVarData(); indexNodes++)
                {
                header->m_apSubNodeID[pChildrenInfo[indexNodes].m_nodePos]= pChildrenInfo[indexNodes].m_nodeId;
                }
            header->m_SubNodeNoSplitID = header->m_apSubNodeID[0];
            }

        const IDTMFile::NeighborNodesTable& rNeighbors = m_indexHandler->GetNeighborNodes(ConvertBlockID(blockID));

        for (int neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
            {       
            header->m_apAreNeighborNodesStitched[neighborPosInd] = rNeighbors.m_AreNeighborTilesStitched[neighborPosInd];            
            }        

        if (rNeighbors.GetNbVarData() > 0)
            {
            const IDTMFile::NodeInfo* pNeighborNodeInfo(m_indexHandler->GetNeighborNodesVarData (rNeighbors));                    
                       
            for (size_t indexNodes = 0 ; indexNodes < (size_t)rNeighbors.GetNbVarData(); indexNodes++)
                {                
                assert(pNeighborNodeInfo[indexNodes].m_nodePos < IDTMFile::NeighborNodesTable::MAX_QTY);

                header->m_apNeighborNodeID[pNeighborNodeInfo[indexNodes].m_nodePos].push_back(pNeighborNodeInfo[indexNodes].m_nodeId);                        
                }           
            }
               
        header->m_IsLeaf = (!header->m_apSubNodeID[0].IsValid());
        header->m_IsBranched = !header->m_IsLeaf && (header->m_apSubNodeID.size() > 1 && header->m_apSubNodeID[1].IsValid());

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
            //NEEDS_WORK_SM : Stitching triangles usually cross node frontier. 
            /*
            HASSERT(ExtentOp<EXTENT>::GetXMin(header->m_contentExtent) >= ExtentOp<EXTENT>::GetXMin(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetXMax(header->m_contentExtent) <= ExtentOp<EXTENT>::GetXMax(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetYMin(header->m_contentExtent) >= ExtentOp<EXTENT>::GetYMin(header->m_nodeExtent));
            HASSERT(ExtentOp<EXTENT>::GetYMax(header->m_contentExtent) <= ExtentOp<EXTENT>::GetYMax(header->m_nodeExtent));
            */
            }

        header->m_totalCountDefined = true;
        header->m_totalCount = m_indexHandler->GetTotalPointCount(ConvertBlockID(blockID));
        header->m_arePoints3d = m_indexHandler->GetArePoints3d(ConvertBlockID(blockID));        
        /*
        Points3dDescBinsInfo points3dDescBinInfo = m_indexHandler->Get3dPointsBinsInfo(ConvertBlockID(blockID));        

        if (nbBins > 0)
            {             
            const IDTMFile::NeighborNodeInfo* pNeighborNodeInfo(m_indexHandler->Get3dPointsBins (points3dDescBinInfo));                    
                       
            for (size_t indexNodes = 0 ; indexNodes < rNeighbors.GetNbVarData(); indexNodes++)
                {                
                assert(pNeighborNodeInfo[indexNodes].m_nodePos < IDTMFile::NeighborNodesTable::MAX_QTY);

                header->m_apNeighborNodeID[pNeighborNodeInfo[indexNodes].m_nodePos].push_back(pNeighborNodeInfo[indexNodes].m_nodeId);                        
                }                       
            }
            */

        header->m_nbFaceIndexes = (size_t)m_indexHandler->GetMeshIndexesCount(ConvertBlockID(blockID));
        header->m_graphID = m_indexHandler->GetGraphBlockID(ConvertBlockID(blockID));
        header->m_numberOfMeshComponents = (size_t)m_indexHandler->GetMeshComponentsCount(ConvertBlockID(blockID));
        const IDTMFile::MeshComponentsList& list = m_indexHandler->GetMeshComponents(ConvertBlockID(blockID));
        header->m_meshComponents = new int[header->m_numberOfMeshComponents];
        if (header->m_numberOfMeshComponents > 0)
            {
            const int* allComponents = m_indexHandler->GetMeshComponentsVarData(list);
            memcpy(header->m_meshComponents, allComponents, header->m_numberOfMeshComponents * sizeof(int));
            }
              
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

