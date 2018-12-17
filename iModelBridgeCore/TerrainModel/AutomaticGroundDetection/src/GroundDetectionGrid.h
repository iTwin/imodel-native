/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/GroundDetectionGrid.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma  once

#include <TerrainModel/TerrainModel.h>
#include "GroundDetectionTypes.h"
#include "GroundDetectionManagerDc.h"
#include <TerrainModel/AutomaticGroundDetection/IPointsProvider.h>

BEGIN_GROUND_DETECTION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     06/2015
+===============+===============+===============+===============+===============+======*/
struct IDPoint3dCriteria
    {
private:
    virtual bool _IsAccepted(DPoint3d const& point) const =0;
public:
    bool IsAccepted(DPoint3d const& point) const;
    };


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     06/2015
+===============+===============+===============+===============+===============+======*/
struct ZValueEntryCompare
    {
    bool operator() (const DPoint3d& a, const DPoint3d& b) const
        {
        return a.z < b.z;
        }
    };

typedef std::vector<DPoint3d> PointCollection;

struct SeedPointContainer : public PointCollection, RefCountedBase
{
public:
    static SeedPointContainerPtr Create(GridCellEntry& gridCellEntry);

    void        AddPoint(DPoint3d& ptIndex);
    void        Draw() const;

protected:

    explicit SeedPointContainer(GridCellEntry& GridCellEntry);
    virtual ~SeedPointContainer();

    virtual void        _AddPoint(DPoint3d& ptIndex);

    Transform                   m_metersToUors;
}; // SeedPointContainer


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     05/2015
+===============+===============+===============+===============+===============+======*/
struct GridCellEntry : public RefCountedBase
{
protected:
    GridCellEntry(DRange3d const& boundingBoxInUors, GroundDetectionParameters const& params);
    virtual ~GridCellEntry();

    virtual void _QueryFirstSeedPointAndAddToTin(PCGroundTIN& pcGroundTin);
    virtual void _FilterFirstCandidate(PCGroundTIN& pcGroundTin, SeedPointContainer& cellCandidates);

    DRange3d                        m_boundingBoxMeter; //In Meters
    DRange3d                        m_boundingBoxUors; //In UORs
    Transform                       m_metersToUors;
    bool                            m_IsExpandTinToRange;
    bool                            m_isMultiThread;    
    int                             m_queryView;
    bool                            m_useViewFilters;
    size_t                           m_memorySize;
    uint32_t                          m_channelFlags;
    size_t                          m_nbPointToAdd;
    IPointsProviderPtr              m_pPointsProvider;

public:
    enum BorderPositionFlags
        {
        NOT_ON_BORDER = 0,
        LEFT    = (1 << 0),
        RIGHT   = (1 << 1),
        TOP     = (1 << 2),
        BOTTOM  = (1 << 3),
        TOP_LEFT = TOP | LEFT,
        TOP_RIGHT = TOP | RIGHT,
        BOTTOM_LEFT = BOTTOM | LEFT,
        BOTTOM_RIGHT = BOTTOM | RIGHT,
        };

    static GridCellEntryPtr Create(DRange3d const& boundingBoxInUors, GroundDetectionParameters const& params);

    static int GetPositionFlags(double row, double col, double NbRow, double NbCol);

    DRange3d const&              GetBoundingBox() const                      { return m_boundingBoxMeter; }
    Transform const&             GetMetersToUors() const                     { return m_metersToUors; }
    void        Draw() const;
    void        DrawBoundingBox() const;

    void        PrefetchPoints();
    void        QueryFirstSeedPointAndAddToTin(PCGroundTIN& pcGroundTin);
    size_t      GetMemorySize() const;
    
    void        SetNbPointToAdd(size_t val) { BeAssert(val >= 1); m_nbPointToAdd = val; }
    

}; // GridCellEntry

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     05/2015
+===============+===============+===============+===============+===============+======*/
struct BorderGridCellEntry : public GridCellEntry
{
DEFINE_T_SUPER(GridCellEntry)

public:

    static GridCellEntryPtr Create(DRange3d const& boundingBoxInUors, GroundDetectionParameters const& params, int position);

protected:
    BorderGridCellEntry(DRange3d const& boundingBoxInUors, GroundDetectionParameters const& params, int position);
    ~BorderGridCellEntry();

    virtual void _QueryFirstSeedPointAndAddToTin(PCGroundTIN& pcGroundTin);
    virtual void _FilterFirstCandidate(PCGroundTIN& pcGroundTin, SeedPointContainer& cellCandidates);

    int m_position;

}; // BorderGridCellEntry



/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     05/2015
+===============+===============+===============+===============+===============+======*/
struct GroundDetectionGrid : public RefCountedBase
{
public:
    static GroundDetectionGridPtr Create(GroundDetectionParameters const& params);

    DRange3d const&              GetBoundingBox() const                      { return m_boundingBoxMeter; }
    GridCellEntryPtr        GetGridCellEntry(size_t index) const        { return m_grids[index]; }
    size_t                  GetSize() const;

    void        Draw() const;

protected:
    GroundDetectionGrid(GroundDetectionParameters const& params);
    GroundDetectionGrid(GroundDetectionGrid const& object);
    virtual ~GroundDetectionGrid();

private:
    double                  GetGridCellSize() const { return m_gridCellSize; }


    DRange3d                                    m_boundingBoxMeter;  //In Meters
    vector<GridCellEntryPtr>                    m_grids;
    vector<GridCellEntryPtr>                    m_borderGrids;
    double                                      m_gridCellSize;
    double                                      m_density;
}; // GroundDetectionGrid




END_GROUND_DETECTION_NAMESPACE
