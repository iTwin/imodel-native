/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>
#include <TerrainModel/AutomaticGroundDetection/IPointsProvider.h>
#include "PCGroundTIN.h"

#pragma warning( disable : 4456 )  

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

GROUND_DETECTION_TYPEDEF(SeedPointContainer)

BEGIN_GROUND_DETECTION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool IDPoint3dCriteria::IsAccepted(DPoint3d const& point) const
    {
    return _IsAccepted(point);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SeedPointContainerPtr SeedPointContainer::Create(GridCellEntry& gridCellEntry)
    {
    return new SeedPointContainer(gridCellEntry);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SeedPointContainer::SeedPointContainer(GridCellEntry& gridCellEntry) 
    {
    m_metersToUors.Copy(gridCellEntry.GetMetersToUors());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SeedPointContainer::~SeedPointContainer()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SeedPointContainer::AddPoint(DPoint3d& ptIndex)
    {
    return _AddPoint(ptIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SeedPointContainer::_AddPoint(DPoint3d& ptIndex)
    {
#if 0
    push_back(ptIndex);
    if (size() > PCGroundTIN::CONTAINER_MAX_SIZE)
        {
        //Sort and keep only the NB_SEEDPOINTS_TO_ADD first entries
        ZValueEntryCompare fPredicat;
        nth_element(begin(), begin() + (PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD-1), end(), fPredicat);
        resize(PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD);
        }
#else
    ZValueEntryCompare fPredicatIsZSmaller;
    if (empty())
        push_back(ptIndex);
    else if (fPredicatIsZSmaller(ptIndex,*begin()))
        *begin() = ptIndex;

#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SeedPointContainer::Draw() const
    {
    // Create a new US element from the vertices.
    PointCollection pointArray;
    for (auto Itr = begin(); Itr != end(); ++Itr)
        {
        DPoint3d PointInUors(*Itr);

        m_metersToUors.Multiply(PointInUors);
        pointArray.push_back(PointInUors);
        }                                         
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GridCellEntryPtr GridCellEntry::Create(DRange3d const& boundingBoxInUors, GroundDetectionParameters const& params)
    {
    return new GridCellEntry(boundingBoxInUors, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GridCellEntry::GridCellEntry(DRange3d const& boundingBoxInUors, GroundDetectionParameters const& params)
:m_metersToUors(params.GetMetersToUors()),
m_IsExpandTinToRange(params.GetExpandTinToRange()),
m_isMultiThread(params.GetUseMultiThread()),
m_boundingBoxUors(boundingBoxInUors),
m_channelFlags(0),
m_memorySize(0),
m_nbPointToAdd(PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD)
    {
    Transform uorsToMeters;
    uorsToMeters.InverseOf(m_metersToUors);    
    IPointsProviderCreatorPtr ptsProviderCreator(params.GetPointsProviderCreator());
    m_pPointsProvider = IPointsProvider::CreateFrom(ptsProviderCreator, &m_boundingBoxUors);    
    m_pPointsProvider->SetUseMultiThread(params.GetUseMultiThread());
    m_pPointsProvider->SetUseMeterUnit(true);//We want to work in meters, faster for pointCloud...
    uorsToMeters.Multiply(m_boundingBoxMeter, m_boundingBoxUors);        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GridCellEntry::~GridCellEntry()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GridCellEntry::QueryFirstSeedPointAndAddToTin(PCGroundTIN& pcGroundTin)
    {
    return _QueryFirstSeedPointAndAddToTin(pcGroundTin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GridCellEntry::PrefetchPoints()
    {
    m_pPointsProvider->PrefetchPoints();
    m_memorySize = m_pPointsProvider->GetMemorySize();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  GridCellEntry::GetMemorySize() const
    {
    return m_memorySize;  
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GridCellEntry::_QueryFirstSeedPointAndAddToTin(PCGroundTIN& pcGroundTin)
    {
    SeedPointContainerPtr  pSeedContainer(SeedPointContainer::Create(*this));

    for (auto itr = m_pPointsProvider->begin(); itr != m_pPointsProvider->end(); ++itr)
        {
        DPoint3d ptIndex(*itr);
        pSeedContainer->AddPoint(ptIndex);
        }
    //Free our memory, we don't need it anymore for now
    m_pPointsProvider->ClearPrefetchedPoints();
    
    _FilterFirstCandidate(pcGroundTin, *pSeedContainer);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GridCellEntry::_FilterFirstCandidate(PCGroundTIN& pcGroundTIN, SeedPointContainer& cellCandidates)
    {
    //if no point, nothing to filter
    if (cellCandidates.size()==0)
        return;

    //Will filter seed point in cell and retains ones that meet criteria
    //On first run, criteria is only to keep lowest point
    ZValueEntryCompare fPredicat;
    nth_element(cellCandidates.begin(), cellCandidates.begin() + (m_nbPointToAdd-1), cellCandidates.end(), fPredicat);

    //We assume that there is at least one seed point in each cell (unless there is no point in the cell)
    uint32_t nbPtsCopied(0);
    for (auto itr = cellCandidates.begin(); (nbPtsCopied < m_nbPointToAdd) && (itr != cellCandidates.end()); itr++, nbPtsCopied++)
        pcGroundTIN.AddPoint(*itr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
//GDZERO
#if 0 
StatusInt GridCellEntry::Classify(IDPoint3dCriteria const& criteria)
    {
    //reset last query and create our query buffer in preparation for our new query

    
    QueryContextGuard qerryContext(pointCloudeh, m_isMultiThread, m_boundingBoxUors);
    PCQueryHandle& queryHandle(qerryContext.GetQuery());
    queryHandle.SetMode(m_queryMode, m_queryView);
    queryHandle.SetIgnoreTransform(false);

    IPointCloudChannelPtr channel = queryHandle.GetChannel();
    IPointCloudChannelVector queryChannels;
    queryChannels.push_back(channel.get());

    uint32_t m_channelFlags = (uint32_t) PointCloudChannelId::Xyz | (uint32_t) PointCloudChannelId::Classification;
    if (m_useViewFilters)
        m_channelFlags |= (uint32_t) PointCloudChannelId::Filter;

    IPointCloudQueryBuffersPtr queryBuffer = queryHandle.CreateBuffers(IPointsProvider::DATA_QUERY_BUFFER_SIZE, m_channelFlags, queryChannels);

    Transform uorToMeter(IPointsProvider::GetUorToMeterTransform(mdlModelRef_getActive(),true));

    //For all points
    bool needSave(false);
    unsigned  queryRun(0);
    for (uint32_t pointsRead = queryHandle.GetPoints(*queryBuffer.get()); pointsRead > 0 && CheckProcessNotAborted(); pointsRead = queryHandle.GetPoints(*queryBuffer.get()), queryRun++)
        {
        UChar* pFilterBuffer(m_useViewFilters ? queryBuffer->GetFilterBuffer() : nullptr);
        byte*  pChannelBuffer((byte*) queryBuffer->GetChannelBuffer(queryChannels[0]));
        DPoint3d* pXyzBuffer(queryBuffer->GetXyzBuffer());

        for (size_t i = 0; i < pointsRead; i++, pFilterBuffer++, pXyzBuffer++, pChannelBuffer++)
            {
            if (m_useViewFilters && !PointCloudChannels_Is_Point_Visible(*pFilterBuffer))
                continue;

            DPoint3d pt(*pXyzBuffer);
            uorToMeter.multiply(&pt);

            if (criteria.IsAccepted(pt))
                {
                *pChannelBuffer = CLASSIFICATION_GROUND;
                needSave = true;
                }
            }

        if (needSave)
            {
            queryHandle.SubmitUpdate(queryChannels[0]);
            }
        }

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GridCellEntry::Draw() const
    {
    // Create a new US element from the vertices.
    PointCollection pointArray;
    DPoint3d point;
    point.x = m_boundingBoxUors.low.x;
    point.y = m_boundingBoxUors.low.y;
    point.z = m_boundingBoxUors.high.z;
    pointArray.push_back(point);
    point.x = m_boundingBoxUors.low.x;
    point.y = m_boundingBoxUors.high.y;
    point.z = m_boundingBoxUors.high.z;
    pointArray.push_back(point);
    point.x = m_boundingBoxUors.high.x;
    point.y = m_boundingBoxUors.high.y;
    point.z = m_boundingBoxUors.high.z;
    pointArray.push_back(point);
    point.x = m_boundingBoxUors.high.x;
    point.y = m_boundingBoxUors.low.y;
    point.z = m_boundingBoxUors.high.z;
    pointArray.push_back(point);
    
    //SeedPointFinder::GetAgenda().Insert(eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GridCellEntry::DrawBoundingBox() const
    {
    Transform identity(Transform::FromIdentity());
    DrawingFacility::DrawBoundingBox(m_boundingBoxUors, identity);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int GridCellEntry::GetPositionFlags(double row, double col, double NbRow, double NbCol)
    {
    int flags(NOT_ON_BORDER);

    if (row == 0)
        flags = flags | BOTTOM;
    if (row == NbRow-1)
        flags = flags | TOP;
    if (col == 0)
        flags = flags | LEFT;
    if (col == NbCol -1)
        flags = flags | RIGHT;

    return flags;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GridCellEntryPtr BorderGridCellEntry::Create(DRange3d const& boundingBoxInUors, GroundDetectionParameters const& params, int position)
    {
    return new BorderGridCellEntry(boundingBoxInUors, params,position);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BorderGridCellEntry::BorderGridCellEntry(DRange3d const& boundingBoxInUors, GroundDetectionParameters const& params, int position)
:GridCellEntry(boundingBoxInUors, params),
m_position(position)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BorderGridCellEntry::~BorderGridCellEntry()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void BorderGridCellEntry::_QueryFirstSeedPointAndAddToTin(PCGroundTIN& pcGroundTin)
    {
    T_Super::_QueryFirstSeedPointAndAddToTin(pcGroundTin);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void BorderGridCellEntry::_FilterFirstCandidate(PCGroundTIN& pcGroundTIN, SeedPointContainer& cellCandidates)
    {
    T_Super::_FilterFirstCandidate(pcGroundTIN, cellCandidates);

    //IF we don't want to expand our TIN to range border, just return
    if (!m_IsExpandTinToRange)
        return;

    //Add one more point on the border
    //if no point, nothing to add
    if (cellCandidates.size() == 0)
        return;

    //We assume that there is at least one seed point in each cell (unless there is no point in the cell)
    DPoint3d seedPoint(*(cellCandidates.begin()));

    //Duplicate point elevation on border according to position
    if (m_position & TOP)
        {
        DPoint3d seedPoint(*(cellCandidates.begin()));
        seedPoint.y = m_boundingBoxMeter.high.y;
        pcGroundTIN.AddPoint(seedPoint);
        //On Corner?
        if (m_position & LEFT)
            {
            seedPoint.x = m_boundingBoxMeter.low.x;
            pcGroundTIN.AddPoint(seedPoint);
            }
        //On Corner?
        if (m_position & RIGHT)
            {
            seedPoint.x = m_boundingBoxMeter.high.x;
            pcGroundTIN.AddPoint(seedPoint);
            }
        }
    if (m_position & BOTTOM)
        {
        DPoint3d seedPoint(*(cellCandidates.begin()));
        seedPoint.y = m_boundingBoxMeter.low.y;
        pcGroundTIN.AddPoint(seedPoint);
        //On Corner?
        if (m_position & LEFT)
            {
            seedPoint.x = m_boundingBoxMeter.low.x;
            pcGroundTIN.AddPoint(seedPoint);
            }
        //On Corner?
        if (m_position & RIGHT)
            {
            seedPoint.x = m_boundingBoxMeter.high.x;
            pcGroundTIN.AddPoint(seedPoint);
            }
        }
    if (m_position & LEFT)
        {
        DPoint3d seedPoint(*(cellCandidates.begin()));
        seedPoint.x = m_boundingBoxMeter.low.x;
        pcGroundTIN.AddPoint(seedPoint);
        }
    if (m_position & RIGHT)
        {
        DPoint3d seedPoint(*(cellCandidates.begin()));
        seedPoint.x = m_boundingBoxMeter.high.x;
        pcGroundTIN.AddPoint(seedPoint);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionGridPtr GroundDetectionGrid::Create(GroundDetectionParameters const& params)
    {
    return new GroundDetectionGrid(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_testGreaterGrid = true;

GroundDetectionGrid::GroundDetectionGrid(GroundDetectionParameters const& params)
:m_gridCellSize(params.GetLargestStructureSize()),
m_density(params.GetDensity())
    {        
    DRange3d boundingBoxInUors;

    params.GetPointsProviderCreator()->GetAvailableRange(boundingBoxInUors);

    //Create a grid cell entry that encompass all the range and extract the box in meters
    GridCellEntryPtr pEntry(GridCellEntry::Create(boundingBoxInUors, params));
    m_boundingBoxMeter = pEntry->GetBoundingBox();
    DPoint3d lowInUors = boundingBoxInUors.low;
    DPoint3d hightInUors = boundingBoxInUors.high;

    Transform metersToUorTrans(params.GetMetersToUors());
    metersToUorTrans.ZeroTranslation();       //Don't want translation part, only scale
    DVec3d vectX(DVec3d::From(1.0, 0.0, 0.0));//One meter length vector
    metersToUorTrans.Multiply(vectX);
    double   uorPerMeter = vectX.Magnitude();
    double   largestStructInUors = m_gridCellSize * uorPerMeter;
    double NbRow(0);
    double NbCol(0);

    //Then process with normal grid for all the point cloud
    //Note: this grid will overlap with border grid - this is intentional.
    //We want to give a try to find real ground point (assuming largest structure size) on the border
    //The border grid is just a trick to prevent having a TIN only in the middle of the cloud...
    NbRow = floor((hightInUors.y - lowInUors.y) / largestStructInUors);
    NbCol = floor((hightInUors.x - lowInUors.x) / largestStructInUors);
    NbRow = max(1.0, NbRow);
    NbCol = max(1.0, NbCol);
    //Use at minimum a 2 x 2 grid so we get at minimum 3 pts to form a triangle.
    /*
    if ((NbRow*NbCol) < 4)
        {
        NbRow = max(2.0, NbRow);
        NbCol = max(3.0, NbCol);
        }    
        */

    double requiredLargestStructInUorsY = (hightInUors.y - lowInUors.y) / NbRow;
    double requiredLargestStructInUorsX = (hightInUors.x - lowInUors.x) / NbCol;

    double row;
    double col;

    if (s_testGreaterGrid)
        {
        NbRow += 2;
        NbCol += 2;
        row = -1;        
        }
    else
        {
        row = 0;        
        }

    for (; row < NbRow; row++)
        {
        if (s_testGreaterGrid)
            col = -1;
        else
            col = 0;
            
        for (; col < NbCol; col++)
            {
            DRange3d gridRange;
            gridRange.low.x = lowInUors.x + col*requiredLargestStructInUorsX;
            gridRange.low.y = lowInUors.y + row*requiredLargestStructInUorsY;
            gridRange.low.z = lowInUors.z;
            gridRange.high.x = gridRange.low.x + requiredLargestStructInUorsX;
            gridRange.high.y = gridRange.low.y + requiredLargestStructInUorsY;
            gridRange.high.z = hightInUors.z;
            //Limit to maximum range
            if (!s_testGreaterGrid)
                {
                gridRange.high.x = min(hightInUors.x, gridRange.high.x);
                gridRange.high.y = min(hightInUors.y, gridRange.high.y);
                }

            GridCellEntryPtr pEntry;
            int position(GridCellEntry::GetPositionFlags(row, col, NbRow, NbCol));

            //Create "border" or "corner" cell entry for all column in upper and lower rows O& on first and last column in other rows.
            if (position == (int) GridCellEntry::NOT_ON_BORDER)
                {
                pEntry = GridCellEntry::Create(gridRange, params);
                }
            else
                {
                pEntry = BorderGridCellEntry::Create(gridRange, params, position);
                }

            m_grids.push_back(pEntry);
            }
        }

    //Compute Nb Points to add by Grid cell entry -> we want at least 4 points total otherwise we cannot compute ground
    if (m_grids.size() < 4)
        {
        //Grid is almost the size of our range box, take severals points by grid entry.
        for (auto itr = m_grids.begin(); itr != m_grids.end(); itr++)
            (*itr)->SetNbPointToAdd(4);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionGrid::~GroundDetectionGrid()
 {
 }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GroundDetectionGrid::GetSize() const
    {
    return m_grids.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GroundDetectionGrid::Draw() const
    {
    for (auto itr = m_grids.begin(); itr != m_grids.end(); itr++)
        {
        (*itr)->Draw();
        }
    }

END_GROUND_DETECTION_NAMESPACE
