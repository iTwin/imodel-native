#include "ScalableMeshPCH.h"
#include "ScalableMeshAnalysis.h"
#include <omp.h>  

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//#define SM_ANALYSIS_DEBUG
#ifdef SM_ANALYSIS_DEBUG
extern void DumpGrid(std::string filename, ISMGridVolume& grid);
#endif

struct  SMDummyProgressListener : public ISMAnalysisProgressListener
    {
    SMDummyProgressListener() {}
    ~SMDummyProgressListener() {}
    }; // SMDummyProgressListener
static SMDummyProgressListener  s_SMdummyProgressListener; //A do nothing progress listener

SMProgressReport::SMProgressReport(ISMAnalysisProgressListener* pProgressListener) :
    m_pProgressListener(pProgressListener)
    {
    m_workDone = 0.0;
    m_timeDelay = 0.5; // report every 0.5 sec
    m_processCanceled = false;
    if (NULL == m_pProgressListener)
        m_pProgressListener = &s_SMdummyProgressListener;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Stephane.Nullans                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SMProgressReport::SMProgressReport()
    {
    m_workDone = 0.0;
    m_timeDelay = 0.5; // report every 0.5 sec
    m_processCanceled = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Stephane.Nullans                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
// Progression implementation
SMProgressReport::~SMProgressReport()
    {
    }

bool SMProgressReport::_CheckContinueOnProgress(ISMProgressReport const& report)
    {
    return m_pProgressListener->_CheckContinueOnProgress(report);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Stephane.Nullans                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
 // IScalableMeshAnalysis implementation

DTMStatusInt IScalableMeshAnalysis::ComputeDiscreteVolume(const bvector<DPoint3d>& polygon,
                                                            double resolution,
                                                            ISMGridVolume& grid,
                                                            ISMAnalysisProgressListener* pProgressListener)
    {
    return _ComputeDiscreteVolume(polygon, resolution, grid, pProgressListener);
    }

DTMStatusInt IScalableMeshAnalysis::ComputeDiscreteVolume(const bvector<DPoint3d>& polygon,
                                                            IScalableMesh* anotherMesh,
                                                            double resolution, ISMGridVolume& grid,
                                                            ISMAnalysisProgressListener* pProgressListener)
    {
    return _ComputeDiscreteVolume(polygon, anotherMesh, resolution, grid, pProgressListener);
    }

void IScalableMeshAnalysis::SetMaxThreadNumber(int num)
    {
    return _SetMaxThreadNumber(num);
    }

void IScalableMeshAnalysis::SetUnitToMeter(double val)
    {
    return _SetUnitToMeter(val);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Stephane.Nullans                  05/2017
 +---------------+---------------+---------------+---------------+---------------+------*/
// ISMGridVolume implementation
ISMGridVolume::ISMGridVolume() 
    {
    m_direction = DVec3d::From(0, 0, 1); // fixed to Z for now
    m_gridOrigin = DPoint3d::From(0, 0, 0);
    m_resolution = 1.0; // 1 meter
    m_gridSizeLimit = 5000;
    m_totalVolume = m_cutVolume = m_fillVolume = 0;
    m_VolSegments = NULL;
    m_bInitialised = false;
    m_isWorld = true;
    m_isEcef = false;
    m_xSize = m_ySize = 0;
    }

ISMGridVolume::~ISMGridVolume() 
    {
    delete[] m_VolSegments;
    m_VolSegments = NULL;
    }

bool ISMGridVolume::GetGridSize(int &_xSize, int &_ySize)
    {
    if (!m_bInitialised)
        return false;
    _xSize = m_xSize;
    _ySize = m_ySize;
    return true;
    }

bool ISMGridVolume::InitGrid(int _xSize, int _ySize)
    {
    m_xSize = _xSize;
    m_ySize = _ySize;

    if (m_VolSegments != NULL)
        delete[] m_VolSegments; // delete the previous data

    // reserve memory for segments
    m_VolSegments = new SMVolumeSegment[m_xSize*m_ySize];
    if (m_VolSegments == nullptr)
        m_bInitialised = false; // failed allocating memory for the grid
    else
        m_bInitialised = true;
    return m_bInitialised;
    }

//=======================================================================================
// @bsimethod                                                 Stephane.Nullans 16/05/2017
//=======================================================================================
ScalableMeshAnalysis::ScalableMeshAnalysis(IScalableMesh* scMesh) : m_scmPtr(scMesh)
    {
    m_ThreadNumber = omp_get_num_procs();
    m_unit2meter = 1.0;
    }

//=======================================================================================
// @bsimethod                                                 Stephane.Nullans 16/05/2017
//=======================================================================================
ScalableMeshAnalysis::~ScalableMeshAnalysis()
    {

    }

ScalableMeshAnalysis* ScalableMeshAnalysis::Create(IScalableMesh* scMesh)
    {
    return new ScalableMeshAnalysis(scMesh);
    }

bool _CreatePolyfaceFromPoints(const bvector<DPoint3d>& polygon, PolyfaceHeaderPtr polyface)
    {
    // Create the Polyface for intersections and queries
    ICurvePrimitivePtr curvePtr(ICurvePrimitive::CreateLineString(polygon));
    CurveVectorPtr curveVectorPtr(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr));
    IFacetOptionsPtr facetOptions = IFacetOptions::Create();
    facetOptions->SetNormalsRequired(false);
    facetOptions->SetMaxPerFace(3);
    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(*facetOptions);
    polyfaceBuilder->AddRegion(*curveVectorPtr);
    polyface = polyfaceBuilder->GetClientMeshPtr();
    return (polyface != nullptr);
    }

// Compute the intersection range from the polygon and meshe(s)
bool ScalableMeshAnalysis::_GetComputationRange(DRange3d& rangeInter, bool inWorld, 
                                                const bvector<DPoint3d>& polygon, IScalableMesh *mesh1, IScalableMesh *mesh2, bool bAlignZ)
    {
    if (mesh1 == nullptr)
        return false;

    DRange3d rangeMesh; // extend of the scalable mesh
    if (inWorld)
        _GetWorldRange(mesh1, rangeMesh);
    else
        mesh1->GetRange(rangeMesh);

    if (mesh2 != nullptr)
        {
        DRange3d rangeMesh2; // extend of the second scalable mesh
        if (inWorld)
            _GetWorldRange(mesh2, rangeMesh2);
        else
            mesh2->GetRange(rangeMesh2);
        rangeMesh.IntersectionOf(rangeMesh, rangeMesh2);
        if (rangeMesh.IsNull())
            return false; // no intersection
        if (bAlignZ) // align the bottom Z
            {
            double zmin = std::min(rangeMesh.low.z, rangeMesh2.low.z);
            rangeMesh.low.z = zmin;
            }
        }

    DRange3d rangePolygon = DRange3d::From(0, 0);

    if (!inWorld) // nedd to convert in 3sm space
        {
        bvector<DPoint3d> poly3sm;
        _convertTo3SMSpace(polygon, poly3sm);
        for (auto point : poly3sm)
            rangePolygon.Extend(point);
        }
    else // polygon is already in World
        {
        for (auto point : polygon)
            rangePolygon.Extend(point);
        }

    rangeInter.IntersectionOf(rangeMesh, rangePolygon);
    if (rangeInter.IsNull())
        {
        // polygon can be planar on Z, add some elevation
        rangePolygon.low.z += -1.0;
        rangePolygon.high.z += 1.0;
        rangeInter.IntersectionOf(rangeMesh, rangePolygon);
        if (rangeInter.IsNull()) // still null ???
            return false; // no intersection
        }

    if (bAlignZ) // align the bottom Z
        {
        double zmin = std::min(rangeMesh.low.z, rangePolygon.low.z);
        rangeInter.low.z = zmin;
        }
    return true;
    }

void ScalableMeshAnalysis::_CreateFillVolumeRanges(SMVolumeSegment& segment, bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection>& _IPoints, DPoint3d& median, DVec3d& direction)
    {
    // store the first border to consider
    bool bSearchExterior = true; // look first for an external border
    DPoint3d start; start.Zero();
    for (auto ipoint : _IPoints)
        {
        if (ipoint.rayFraction < 0)
            continue; // skip intersection behind ray direction
        if (ipoint.point.z < median.z) // consider only upper points
            continue;
        bool isExterior = direction.DotProduct(ipoint.normal) < 0;
        if (bSearchExterior && isExterior) // we have a external border
            {
            start = ipoint.point;
            bSearchExterior = false; // next to search is an interior border
            }
        else if (!bSearchExterior && !isExterior) // we have a valid segment
            {
            segment.VolumeRanges.push_back(start.z);
            segment.VolumeRanges.push_back(ipoint.point.z);
            bSearchExterior = true; // next to search is an exterior border : new start point
            }
        else if (!bSearchExterior && isExterior)
            {
            start = ipoint.point; // a new start point
            }
        }
    }

void ScalableMeshAnalysis::_CreateCutVolumeRanges(SMVolumeSegment& segment, bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection>& _IPoints, DPoint3d& median, DVec3d& direction)
    {
    // store the first border to consider
    bool bSearchExterior = true; // look first for an external border
    DPoint3d start; start.Zero();
    for (auto ipoint : _IPoints)
        {
        if (ipoint.rayFraction < 0)
            continue; // skip intersection behind ray direction
        if (ipoint.point.z > median.z) // consider only upper points
            break;
        bool isExterior = direction.DotProduct(ipoint.normal) > 0;
        if (bSearchExterior && isExterior) // we have a external border
            {
            start = ipoint.point;
            bSearchExterior = false; // next to search is an interior border
            }
        else if (!bSearchExterior && !isExterior) // we have a valid segment
            {
            segment.VolumeRanges.push_back(ipoint.point.z);
            segment.VolumeRanges.push_back(start.z);
            bSearchExterior = true; // next to search is an exterior border : new start point
            }
        else if (!bSearchExterior && isExterior)
            {
            start = ipoint.point; // a new start point
            }
        }
    }

// Compute "Stock Pile" volume between a polygon region and this scalablemesh
// along a direction : Z by default
// on a grid of given resolution
// polygon points need to be in World coordinates
// Returns a SMGridVolume structures
DTMStatusInt ScalableMeshAnalysis::_ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener)
    {
    bool isECEF = m_scmPtr->IsCesium3DTiles();
    if (isECEF)
        return _ComputeDiscreteVolumeEcef(polygon, resolution, grid, pProgressListener);

    if (grid.m_isWorld)
        return _ComputeDiscreteVolumeWorld(polygon, resolution, grid, pProgressListener);

    // else compute in 3SM space
    return _ComputeDiscreteVolume3SM(polygon, resolution, grid, pProgressListener);
    }

// All values are computed in 3SM space
DTMStatusInt ScalableMeshAnalysis::_ComputeDiscreteVolume3SM(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener)
    {
    if (polygon.size() < 3)
        return DTMStatusInt::DTM_ERROR; // invalid region

    SMProgressReport report(pProgressListener);
    if (!report.CheckContinueOnProgress())
        return DTMStatusInt::DTM_ERROR; //User abort

    DRange3d range; // get the computation range from data intersections
    if (_GetComputationRange(range, grid.m_isWorld, polygon, m_scmPtr, NULL, false) == false)
        return DTMStatusInt::DTM_ERROR; // no intersection

    // Initialize the grid volume structure
    double resUOR = (grid.m_isWorld ? resolution / m_unit2meter : resolution);
    if (!_InitGridFrom(grid, resUOR, range))
        return DTMStatusInt::DTM_ERROR; // could not initialize grid

    int m_xSize, m_ySize;
    grid.GetGridSize(m_xSize, m_ySize);
    if (m_xSize<=0 || m_ySize<=0)
        return DTMStatusInt::DTM_ERROR; // Zero sized Grid

    // Create the Polyface for intersections and queries
    ICurvePrimitivePtr curvePtr(ICurvePrimitive::CreateLineString(polygon));
    CurveVectorPtr curveVectorPtr(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr));
    IFacetOptionsPtr facetOptions = IFacetOptions::Create();
    facetOptions->SetNormalsRequired(false);
    facetOptions->SetMaxPerFace(3);
    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(*facetOptions);
    polyfaceBuilder->AddRegion(*curveVectorPtr);
    PolyfaceHeaderPtr polyface = polyfaceBuilder->GetClientMeshPtr();

    auto draping1 = m_scmPtr->GetDTMInterface()->GetDTMDraping();
    bool isTerrain = m_scmPtr->IsTerrain();

    bool *intersected = new bool[m_xSize*m_ySize];
    memset(intersected, 0, sizeof(bool)*m_xSize*m_ySize);

    double tolerance = std::numeric_limits<double>::min(); // intersection tolerance
    double zsource = range.low.z - tolerance; // be sure to start under the range
    double m_xStep = grid.m_resolution;
    double m_yStep = grid.m_resolution;

    int numProcs = std::min(m_ThreadNumber, omp_get_num_procs());
    if (numProcs > 1)
        numProcs = numProcs - 1; // use all available CPUs minus one
    if (numProcs <= 0)
        numProcs = 1;

    const double progressStep = 1.0 / (m_xSize+1);
    double progress = 0.0; // progress value between 0 and 1
    bool userAborted(false);
    clock_t timer = clock();
    std::thread::id main_id = std::this_thread::get_id(); // Get the id of the main Thread

#pragma omp parallel num_threads(numProcs)
    {

#pragma omp for
    for (int i = 0; i < m_xSize; i++)
        {
        std::thread::id t_id = std::this_thread::get_id(); // Get the id of the current Thread

        if (main_id == t_id)
            {
            float secs = ((float)(clock() - timer)) / CLOCKS_PER_SEC;
            if (secs > report.m_timeDelay)
                {
                userAborted = !report.CheckContinueOnProgress();
                report.m_workDone = progress;
                timer = clock();
                }
            }

        if (userAborted)
            continue;

        double x = range.low.x + m_xStep * i;
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); // added here because of parallelisation

        for (int j = 0; j < m_ySize; j++)
            {
            intersected[i*m_ySize + j] = false;

            double y = range.low.y + m_yStep * j;
            DPoint3d source = DPoint3d::From(x, y, zsource);
            DPoint3d sourceW = DPoint3d::From(x, y, zsource);

            _convert3SMToWorld(m_scmPtr, sourceW); // Intersection interface needs points in World
            bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection> Hits;
            bool bret = draping1->IntersectRay(Hits, grid.m_direction, sourceW);
     
            if (bret && Hits.size() > 0)
                {
                //convert the hits back in 3SM system
                for (auto &ahit : Hits)
                    _convertWorldTo3SM(m_scmPtr, ahit.point);

                auto classif = curveVectorPtr->PointInOnOutXY(source);
                if (classif == CurveVector::InOutClassification::INOUT_In) // we are inside the restriction
                    {
                    DRay3d ray = DRay3d::FromOriginAndVector(source, grid.m_direction);
                    DPoint3d polyHit; polyHit.Zero();
                    double rayFraction = 0;
                    visitor->Reset();
                    if (visitor->AdvanceToFacetBySearchRay(ray, tolerance, polyHit, rayFraction))
                        {
                        SMVolumeSegment aSegment;
                        if (isTerrain) // just stack the 2 intersections
                            {
                            aSegment.VolumeRanges.push_back(polyHit.z);
                            aSegment.VolumeRanges.push_back(Hits[0].point.z);
                            }
                        else
                            {
                            // add the intersection with the polygon in the hit list
                            BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection rayInter;
                            rayInter.point = polyHit;
                            rayInter.rayFraction = rayFraction;
                            rayInter.normal = -1.0 * grid.m_direction; // with inversed direction
                            rayInter.hasNormal = true;
                            Hits.push_back(rayInter);

                            DTMIntersectionCompare Comparator;
                            std::sort(Hits.begin(), Hits.end(), Comparator); // sort by ray fraction

                            _CreateFillVolumeRanges(aSegment, Hits, rayInter.point, grid.m_direction);
                            _CreateCutVolumeRanges(aSegment, Hits, rayInter.point, grid.m_direction);
                            }

                        if (aSegment.VolumeRanges.size() > 0)
                            {
                            grid.m_VolSegments[i*m_ySize + j] = aSegment;
                            intersected[i*m_ySize + j] = true;
                            }
                        }
                    }
                }
            }
#pragma omp critical
        progress += progressStep;
        }
    }

    if (userAborted)
        {
        delete[] intersected;
        return DTMStatusInt::DTM_ERROR; //User abort
        }

    // Sum the discrete volumes
    _FillGridVolumes(grid, intersected, m_unit2meter); // Sum the discrete volumes
    grid.m_gridOrigin = grid.m_range.low; // the computation & range are in World

#ifdef SM_ANALYSIS_DEBUG
    DumpGrid(std::string("c:\\Dev\\logDebugGrid_sdk.txt"), grid);
#endif

    if (!userAborted) // update only if not aborted
        report.m_workDone = 1.0;

    delete[] intersected;

    return DTMStatusInt::DTM_SUCCESS;
    }

DTMStatusInt ScalableMeshAnalysis::_ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, IScalableMesh* diffMesh, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener)
    {
    if (polygon.size() < 3)
        return DTMStatusInt::DTM_ERROR; // invalid region
    if (diffMesh == nullptr)
        return DTMStatusInt::DTM_ERROR; // invalid mesh
    if (diffMesh == m_scmPtr)
        return DTMStatusInt::DTM_ERROR; // same meshes

    bool isECEF = m_scmPtr->IsCesium3DTiles();
    if (isECEF)
        return _ComputeDiscreteVolumeEcef(polygon, diffMesh, resolution, grid, pProgressListener);

    // else compute in World coords
    return _ComputeDiscreteVolumeWorld(polygon, diffMesh, resolution, grid, pProgressListener);
    }

DTMStatusInt ScalableMeshAnalysis::_ComputeDiscreteVolumeWorld(const bvector<DPoint3d>& polygon, IScalableMesh* diffMesh, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener)
   {
   SMProgressReport report(pProgressListener);
    if (!report.CheckContinueOnProgress())
        return DTMStatusInt::DTM_ERROR; //User abort

    DRange3d range; // get the computation range from data intersections
    if (_GetComputationRange(range, grid.m_isWorld, polygon, m_scmPtr, diffMesh, true) == false)
        return DTMStatusInt::DTM_ERROR; // no intersection

    // Initialize the grid volume structure
    double resUOR = (grid.m_isWorld ? resolution / m_unit2meter : resolution);
    if (!_InitGridFrom(grid, resUOR, range))
        return DTMStatusInt::DTM_ERROR; // could not initialize grid

    //DRange3d range = grid.m_range; // the intersection range (mesh inter region)
    int m_xSize, m_ySize;
    grid.GetGridSize(m_xSize, m_ySize);
    if (m_xSize <= 0 || m_ySize <= 0)
        return DTMStatusInt::DTM_ERROR; // Zero sized Grid

    // Create the Polyface for intersections and queries
    ICurvePrimitivePtr curvePtr(ICurvePrimitive::CreateLineString(polygon));
    CurveVectorPtr curveVectorPtr(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr));
    IFacetOptionsPtr facetOptions = IFacetOptions::Create();
    facetOptions->SetNormalsRequired(false);
    facetOptions->SetMaxPerFace(3);
    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(*facetOptions);
    polyfaceBuilder->AddRegion(*curveVectorPtr);
    PolyfaceHeaderPtr polyface = polyfaceBuilder->GetClientMeshPtr();

    if (polyface == nullptr)
        return DTMStatusInt::DTM_ERROR; // could not generate polygon from points

    auto draping1 = m_scmPtr->GetDTMInterface()->GetDTMDraping();
    auto draping2 = diffMesh->GetDTMInterface()->GetDTMDraping();

    bool *intersected = new bool[m_xSize*m_ySize];
    memset(intersected, 0, sizeof(bool)*m_xSize*m_ySize);

    DPoint2d *interPoints = new DPoint2d[m_xSize*m_ySize]; // store 
    memset(interPoints, 0, sizeof(DPoint2d)*m_xSize*m_ySize);

    double tolerance = std::numeric_limits<double>::min(); // intersection tolerance - SNU TODO
    double zsource = range.low.z - tolerance; // be sure to start under the range
    double m_xStep = grid.m_resolution;
    double m_yStep = grid.m_resolution;

    int numProcs = std::min(m_ThreadNumber, omp_get_num_procs());
    if (numProcs > 1)
        numProcs = numProcs - 1; // use all available CPUs minus one
    if (numProcs <= 0)
        numProcs = 1;

    double progress = 0.0;
    double progressStep = 1.0 / (m_xSize+1);
    bool userAborted(false);
    clock_t timer = clock();

    std::thread::id main_id = std::this_thread::get_id(); // Get the id of the main Thread

#pragma omp parallel num_threads(numProcs) 
    {
#pragma omp for
    for (int i = 0; i < m_xSize; i++)
        {
        std::thread::id t_id = std::this_thread::get_id(); // Get the id of the current Thread

        if (main_id == t_id)
            {
            float secs = ((float)(clock() - timer)) / CLOCKS_PER_SEC;
            if (secs > report.m_timeDelay)
                {
                userAborted = !report.CheckContinueOnProgress();
                report.m_workDone = progress;
                timer = clock();
                }
            }

        if (userAborted)
            continue;

        double x = range.low.x + m_xStep * i;
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); // added here because of parallelisation

        for (int j = 0; j < m_ySize; j++)
            {
            intersected[i*m_ySize + j] = false;
            double y = range.low.y + m_yStep * j;
            DPoint3d sourceW = DPoint3d::From(x, y, zsource); // source is in world

            DPoint3d interP1;
            bool bret = draping1->IntersectRay(interP1, grid.m_direction, sourceW);

            if (bret)
                {
                auto classif = curveVectorPtr->PointInOnOutXY(sourceW);
                if (classif == CurveVector::InOutClassification::INOUT_In) // we are inside the restriction
                    {
                    DPoint3d interP2;
                    bret = draping2->IntersectRay(interP2, grid.m_direction, sourceW); // second SM is supposed to be in same GCS
                    if (bret)
                        {
                        interPoints[i*m_ySize + j] = DPoint2d::From(interP1.z, interP2.z);
                        intersected[i*m_ySize + j] = true;
                        }
                    }
                }
            }
#pragma omp critical
            progress += progressStep;
        }
    }

    if (userAborted)
        {
        delete[] intersected;
        delete[] interPoints;
        return DTMStatusInt::DTM_ERROR; //User abort
        }

    // Fill the grid values - non parallel
    for (int i = 0; i < m_xSize; i++)
        {
        for (int j = 0; j < m_ySize; j++)
            {
            if (intersected[i*m_ySize + j])
                {
                grid.m_VolSegments[i*m_ySize + j].VolumeRanges.push_back(interPoints[i*m_ySize + j].x * m_unit2meter); // in meter
                grid.m_VolSegments[i*m_ySize + j].VolumeRanges.push_back(interPoints[i*m_ySize + j].y * m_unit2meter); // in meter
                }
            }
        }

    // Sum the discrete volumes
    _FillGridVolumes(grid, intersected, m_unit2meter);
    grid.m_gridOrigin = grid.m_range.low; // the computation & range are in World

    delete[] intersected;
    delete[] interPoints;

    if (!userAborted) // update only if not aborted
        report.m_workDone = 1.0;

    return DTMStatusInt::DTM_SUCCESS;
    }

bool ScalableMeshAnalysis::_convert3SMToWorld(IScalableMesh *scmPtr, DPoint3d& _pt)
    {
    if (scmPtr == nullptr)
        return false;
    Transform transform(m_scmPtr->GetReprojectionTransform());
    if (transform.IsIdentity())
        {
        const GeoCoords::GCS& gcs(m_scmPtr->GetGCS());
        double smGcsRatioToMeter = gcs.GetUnit().GetRatioToBase();
        //Convert from SM to UOR unit.
        _pt.Scale(smGcsRatioToMeter);
        }
    else {
        transform.Multiply(_pt);
        }
    return true;
    }

bool ScalableMeshAnalysis::_convertWorldTo3SM(IScalableMesh *scmPtr, DPoint3d& _pt)
    {
    if (scmPtr == nullptr)
        return false;
    Transform transform(m_scmPtr->GetReprojectionTransform());
    if (transform.IsIdentity())
        {
        const GeoCoords::GCS& gcs(m_scmPtr->GetGCS());
        double ratioFromMeter = 1.0 / gcs.GetUnit().GetRatioToBase();
        //Convert from UOR to SM unit.
        _pt.Scale(ratioFromMeter);
        }
    else
        {
        Transform transformToSm;
        bool result = transformToSm.InverseOf(transform);
        assert(result == true);
        transformToSm.Multiply(_pt);
        }
    return true;
    }

bool ScalableMeshAnalysis::_convertTo3SMSpace(const bvector<DPoint3d>& polygon, bvector<DPoint3d>& area)
    {
    if (m_scmPtr == nullptr)
        return false;
    Transform transform(m_scmPtr->GetReprojectionTransform());

    if (transform.IsIdentity())
        {
        area.insert(area.end(), polygon.begin(), polygon.end()); // init from polygon

        const GeoCoords::GCS& gcs(m_scmPtr->GetGCS());
        double smGcsRatioToMeter = gcs.GetUnit().GetRatioToBase();
        double ratioFromMeter = 1.0 / smGcsRatioToMeter;

        //Convert from UOR to SM unit.
        for (auto& pt : area)
            pt.Scale(ratioFromMeter);
        }
    else
        {
        Transform transformToSm;
        bool result = transformToSm.InverseOf(transform);
        assert(result == true);
        transformToSm.Multiply(area, polygon);
        }
    return true;
    }

bool ScalableMeshAnalysis::_GetWorldRange(IScalableMesh *scmPtr, DRange3d& _range)
    {
    DRange3d range3sm;
    scmPtr->GetRange(range3sm);
    DPoint3d Corners[8]; // get the corners in world, and compute the range of them
    range3sm.Get8Corners(Corners);

    _range.InitFrom(0, 0); // reset the range
    for (int i = 0; i < 8; i++)
        {
        _convert3SMToWorld(scmPtr, Corners[i]);
        _range.Extend(Corners[i]);
        }
    return true;
    }

void ScalableMeshAnalysis::_SetMaxThreadNumber(int num)
    {
    m_ThreadNumber = num;
    }

void ScalableMeshAnalysis::_SetUnitToMeter(double val)
    {
    m_unit2meter = val;
    }

// Compute "Stock Pile" volume between a polygon region and this scalablemesh
// along a direction : Z by default
// on a grid of given resolution (in meter)
// polygon points need to be in world coordinates
// Returns a SMGridVolume structures in World
DTMStatusInt ScalableMeshAnalysis::_ComputeDiscreteVolumeWorld(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener)
    {
    if (polygon.size() < 3)
        return DTMStatusInt::DTM_ERROR; // invalid region

#ifdef SM_ANALYSIS_DEBUG
    std::ofstream f;
    f.open("c:\\Dev\\logDebugGrid_sdk.txt", ios_base::app);
    f << setprecision(12);
    f << "==  _ComputeDiscreteVolumeWorld() =============" << endl;
    for (auto pp : polygon)
        f << "Polygon :: " << pp.x << " , " << pp.y << " , " << pp.z << endl;
    f << "==============================================" << endl;
    f.close();
#endif

    SMProgressReport report(pProgressListener);
    if (!report.CheckContinueOnProgress())
        return DTMStatusInt::DTM_ERROR; //User abort

    DRange3d range; // get the computation range from data intersections
    if (_GetComputationRange(range, grid.m_isWorld, polygon, m_scmPtr, NULL, true) == false)
        return DTMStatusInt::DTM_ERROR; // no intersection

    // Initialize the grid volume structure
    double resUOR = (grid.m_isWorld ? resolution / m_unit2meter : resolution);
    if (!_InitGridFrom(grid, resUOR, range))
        return DTMStatusInt::DTM_ERROR; // could not initialize grid

    int m_xSize, m_ySize;
    grid.GetGridSize(m_xSize, m_ySize);
    if (m_xSize <= 0 || m_ySize <= 0)
        return DTMStatusInt::DTM_ERROR; // Zero sized Grid

    // Create the Polyface for intersections and queries
    ICurvePrimitivePtr curvePtr(ICurvePrimitive::CreateLineString(polygon));
    CurveVectorPtr curveVectorPtr(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr));
    IFacetOptionsPtr facetOptions = IFacetOptions::Create();
    facetOptions->SetNormalsRequired(false);
    facetOptions->SetMaxPerFace(3);
    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(*facetOptions);
    polyfaceBuilder->AddRegion(*curveVectorPtr);
    PolyfaceHeaderPtr polyface = polyfaceBuilder->GetClientMeshPtr();

    auto draping1 = m_scmPtr->GetDTMInterface()->GetDTMDraping();
    bool isTerrain = m_scmPtr->IsTerrain();

    bool *intersected = new bool[m_xSize*m_ySize];
    memset(intersected, 0, sizeof(bool)*m_xSize*m_ySize);

    double tolerance = std::numeric_limits<double>::min(); // intersection tolerance
    double zsource = range.low.z - tolerance; // be sure to start under the range
    double m_xStep = resUOR;
    double m_yStep = resUOR;

    int numProcs = 1; // std::min(m_ThreadNumber, omp_get_num_procs());
    if (numProcs > 1)
        numProcs = numProcs - 1; // use all available CPUs minus one
    if (numProcs <= 0)
        numProcs = 1;

    const double progressStep = 1.0 / (m_xSize + 1);
    double progress = 0.0; // progress value between 0 and 1
    bool userAborted(false);
    clock_t timer = clock();
    std::thread::id main_id = std::this_thread::get_id(); // Get the id of the main Thread

#pragma omp parallel num_threads(numProcs)
    {
#pragma omp for
    for (int i = 0; i < m_xSize; i++)
        {
        std::thread::id t_id = std::this_thread::get_id(); // Get the id of the current Thread

        if (main_id == t_id)
            {
            float secs = ((float)(clock() - timer)) / CLOCKS_PER_SEC;
            if (secs > report.m_timeDelay)
                {
                userAborted = !report.CheckContinueOnProgress();
                report.m_workDone = progress;
                timer = clock();
                }
            }

        if (userAborted)
            continue;

        double x = range.low.x + m_xStep * i;
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); // added here because of parallelisation

        for (int j = 0; j < m_ySize; j++)
            {
            intersected[i*m_ySize + j] = false;

            double y = range.low.y + m_yStep * j;
            DPoint3d sourceW = DPoint3d::From(x, y, zsource); // world coords

            bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection> Hits;
            bool bret = draping1->IntersectRay(Hits, grid.m_direction, sourceW);

            if (bret && Hits.size() > 0)
                {
                    DRay3d ray = DRay3d::FromOriginAndVector(sourceW, grid.m_direction);
                    DPoint3d polyHit; polyHit.Zero();
                    double rayFraction = 0;
                    visitor->Reset();
                    if (visitor->AdvanceToFacetBySearchRay(ray, tolerance, polyHit, rayFraction))
                        {
                        // convert the intersections in meter
                        SMVolumeSegment aSegment;
                        if (isTerrain) // just stack the 2 intersections
                            {
                            aSegment.VolumeRanges.push_back(polyHit.z);
                            aSegment.VolumeRanges.push_back(Hits[0].point.z);
                            }
                        else
                            {
                            // add the intersection with the polygon in the hit list
                            BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection rayInter;
                            rayInter.point = polyHit;
                            rayInter.rayFraction = rayFraction * m_unit2meter;
                            rayInter.normal = -1.0 * grid.m_direction; // with inversed direction
                            rayInter.hasNormal = true;
                            Hits.push_back(rayInter);

                            DTMIntersectionCompare Comparator;
                            std::sort(Hits.begin(), Hits.end(), Comparator); // sort by ray fraction

                            _CreateFillVolumeRanges(aSegment, Hits, rayInter.point, grid.m_direction);
                            _CreateCutVolumeRanges(aSegment, Hits, rayInter.point, grid.m_direction);
                            }

                        if (aSegment.VolumeRanges.size() > 0)
                            {
                            for (auto &seg : aSegment.VolumeRanges)
                                {
                                seg *= m_unit2meter; // store the Z-volume in meter
                                }
                            grid.m_VolSegments[i*m_ySize + j] = aSegment;
                            intersected[i*m_ySize + j] = true;
                            }
                        }
                }
            }
#pragma omp critical
        progress += progressStep;
        }
    }

    if (userAborted)
        {
        delete[] intersected;
        return DTMStatusInt::DTM_ERROR; //User abort
        }

    _FillGridVolumes(grid, intersected, m_unit2meter); // Sum the discrete volumes
    grid.m_gridOrigin = grid.m_range.low; // the computation & range are in World

#ifdef SM_ANALYSIS_DEBUG
    DumpGrid(std::string("c:\\Dev\\logDebugGrid_sdk.txt"), grid);
#endif

    if (!userAborted) // update only if not aborted
        report.m_workDone = 1.0;

    delete[] intersected;

    return DTMStatusInt::DTM_SUCCESS;
    }

// Volumes are computed in meters
void ScalableMeshAnalysis::_FillGridVolumes(ISMGridVolume& grid, bool *intersected, double U2M)
    {
    double AreaCell = grid.m_resolution*grid.m_resolution* U2M * U2M;
    double _cutVolume = 0, _fillVolume = 0;
    int m_xSize, m_ySize;
    grid.GetGridSize(m_xSize, m_ySize);
#ifdef SM_ANALYSIS_DEBUG
    std::ofstream f;
    f.open("c:\\Dev\\logDebugGrid_sdk.txt" , ios_base::app);
    f << setprecision(12);
    f << "  _FillGridVolumes :::::::::::::::::::::::::::::::::::::::::::::::::::::::: " << endl;
    f << "  Grid Size :: " << m_xSize << " , " << m_ySize << endl;
    f << "  Grid Resolution :: " << grid.m_resolution << endl;
    f << "  Grid Range :: low :" << grid.m_range.low.x << " , "  << grid.m_range.low.y << " , " << grid.m_range.low.z <<endl;
    f << "  Grid Range :: high :" << grid.m_range.high.x << " , " << grid.m_range.high.y << " , " << grid.m_range.high.z << endl;
    f << "  Unit2meter :: " << U2M << endl;
#endif

//    #pragma omp parallel for num_threads(numProcs) reduction(+:_fillVolume,_cutVolume)
    for (int i = 0; i < m_xSize; i++)
        {
        for (int j = 0; j < m_ySize; j++)
            {
            if (intersected[i*m_ySize + j] != true)
                {
                continue;
                }
            SMVolumeSegment &segment = grid.m_VolSegments[i*m_ySize + j];
            for (int k = 0; k < segment.VolumeRanges.size() - 1; k += 2)
                {
                double deltaZ = segment.VolumeRanges[k + 1] - segment.VolumeRanges[k];
                if (deltaZ > 0)
                    _cutVolume += deltaZ * AreaCell;
                else
                    _fillVolume += -deltaZ * AreaCell;

                segment.volume += deltaZ * AreaCell; // keep sum of volume
#ifdef SM_ANALYSIS_DEBUG
                f << "  Volume [" << i << "\t ," << j << "\t] = " << deltaZ * AreaCell << std::endl;
#endif
                }
            }
        }
    grid.m_cutVolume = _cutVolume;
    grid.m_fillVolume = _fillVolume;
    grid.m_totalVolume = grid.m_fillVolume + grid.m_cutVolume;

#ifdef SM_ANALYSIS_DEBUG
    f.close();
#endif
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
