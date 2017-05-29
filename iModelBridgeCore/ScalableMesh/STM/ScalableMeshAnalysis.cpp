#include "ScalableMeshPCH.h"
#include "ScalableMeshAnalysis.h"
#include <omp.h>  

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//=======================================================================================
// @bsimethod                                                 Stephane.Nullans 16/05/2017
//=======================================================================================
ScalableMeshAnalysis::ScalableMeshAnalysis(IScalableMesh* scMesh) : m_scmPtr(scMesh)
    {

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

bool ScalableMeshAnalysis::_InitGridFrom(ISMGridVolume& grid, double _resolution, const DRange3d& _rangeMesh, const DRange3d& _rangeRegion)
    {
    grid.m_range.IntersectionOf(_rangeMesh, _rangeRegion);
    if (grid.m_range.IsNull())
        return DTMStatusInt::DTM_ERROR; // cannot compute volume, no intersection

    grid.m_resolution = _resolution;
    double maxLength = std::max(grid.m_range.XLength(), grid.m_range.YLength());
    int gridSize = maxLength / grid.m_resolution;
    if (gridSize > grid.m_gridSizeLimit)
        {
        grid.m_resolution = maxLength / grid.m_gridSizeLimit; // we limit the resolution
        }

    grid.m_center = DPoint3d::From(_rangeMesh.low.x + _rangeMesh.XLength() / 2,
        _rangeMesh.low.y + _rangeMesh.YLength() / 2,
        _rangeMesh.low.z + _rangeMesh.ZLength() / 2);

    // split the range in regular grid xy
    double xSize = (grid.m_range.high.x - grid.m_range.low.x) / grid.m_resolution; // Nb of cells on X
    double ySize = (grid.m_range.high.y - grid.m_range.low.y) / grid.m_resolution; // Nb of cells on Y

    // Init and reserve the memory for the volume segments
    bool bRes = grid.InitGrid(xSize, ySize);

    return bRes;
    }

// Compute "Stock Pile" volume between a polygon region and this scalablemesh
// along a direction : Z by default
// on a grid of given resolution
// polygon points need to be in world coordinates
// Returns a SMGridVolume structures
DTMStatusInt ScalableMeshAnalysis::_ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid)
    {
    if (polygon.size() < 3)
        return DTMStatusInt::DTM_ERROR; // invalid region

    DRange3d rangeMesh; // extend of the scalable mesh
    m_scmPtr->GetRange(rangeMesh);

    DRange3d rangeRegion; // Extend of the polygon region - extended to mesh Z
    DPoint3d P1 = polygon[0]; P1.z = rangeMesh.low.z;  // keep only xy
    DPoint3d P2 = polygon[1]; P2.z = rangeMesh.high.z;
    rangeRegion = DRange3d::From(P1, P2);
    for (auto point : polygon)
        rangeRegion.Extend(point);
    rangeRegion.low.z = rangeMesh.low.z;
    rangeRegion.high.z = rangeMesh.high.z;

    // Initialize the grid volume structure
    if (!_InitGridFrom(grid, resolution, rangeMesh, rangeRegion))
        return DTMStatusInt::DTM_ERROR; // could not initialize grid

    DRange3d range = grid.m_range; // the intersection range (mesh inter region)
    int m_xSize, m_ySize;
    grid.GetGridSize(m_xSize, m_ySize);

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

    bool *intersected = new bool[m_xSize*m_ySize];
    //SMVolumeSegment *Segments = grid.m_VolSegments;

    double tolerance = std::numeric_limits<double>::min(); // intersection tolerance
    double zsource = range.low.z - tolerance; // be sure to start under the range
    double m_xStep = grid.m_resolution;
    double m_yStep = grid.m_resolution;

    int numProcs = omp_get_num_procs();
    if (numProcs>1)
        numProcs = numProcs-1; // use all available CPUs minus one

#pragma omp parallel for num_threads(numProcs) 
    for (int i = 0; i < m_xSize; i++)
        {
        double x = range.low.x + m_xStep * i;
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); // added here because of parallelisation

        for (int j = 0; j < m_ySize; j++)
            {
            intersected[i*m_ySize + j] = false;

            double y = range.low.y + m_yStep * j;
            DPoint3d source = DPoint3d::From(x, y, zsource);
            bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection> Hits;
            bool bret = draping1->IntersectRay(Hits, grid.m_direction, source);

            if (bret && Hits.size() > 0)
                {
                DPoint3d source_local = source;// -DVec3d::From(grid.m_center);
                auto classif = curveVectorPtr->PointInOnOutXY(source_local);
                if (classif == CurveVector::InOutClassification::INOUT_In) // we are inside the restriction
                    {
                    DRay3d ray = DRay3d::FromOriginAndVector(source_local, grid.m_direction);
                    DPoint3d polyHit; polyHit.Zero();
                    double rayFraction = 0;
                    visitor->Reset();
                    if (visitor->AdvanceToFacetBySearchRay(ray, tolerance, polyHit, rayFraction))
                        {
                        // add the intersection with the polygon in the hit list
                        BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection rayInter;
                        rayInter.point = polyHit;// +DVec3d::From(grid.m_center);
                        rayInter.rayFraction = rayFraction;
                        rayInter.normal = -1.0 * grid.m_direction; // with inversed direction
                        rayInter.hasNormal = true;
                        Hits.push_back(rayInter); 

                        DTMIntersectionCompare Comparator;
                        std::sort(Hits.begin(), Hits.end(), Comparator); // sort by ray fraction

                        SMVolumeSegment aSegment;
                        _CreateFillVolumeRanges(aSegment, Hits, rayInter.point, grid.m_direction);
                        _CreateCutVolumeRanges(aSegment, Hits, rayInter.point, grid.m_direction);

                        if (aSegment.VolumeRanges.size() > 0)
                            {
                            grid.m_VolSegments[i*m_ySize + j] = aSegment;
                            intersected[i*m_ySize + j] = true;
                            }
                        }
                    }
                }
            }
        }

    // Sum the discrete volumes
    double AreaCell = m_xStep*m_yStep;
    double _cutVolume=0, _fillVolume=0;
#pragma omp parallel for num_threads(numProcs) reduction(+:_fillVolume,_cutVolume)
    for (int i = 0; i < m_xSize; i++)
        {
        for (int j = 0; j < m_ySize; j++)
            {
            if (intersected[i*m_ySize + j] != true)
                {
                //SMVolumeSegment emptySegment;
                //grid.m_VolSegments[i*m_ySize + j] = emptySegment;
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
                }
            }
        }
    grid.m_cutVolume = _cutVolume;
    grid.m_fillVolume = _fillVolume;

    grid.m_totalVolume = grid.m_fillVolume + grid.m_cutVolume;

    delete[] intersected;

    return DTMStatusInt::DTM_SUCCESS;
    }

DTMStatusInt ScalableMeshAnalysis::_ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, IScalableMesh* diffMesh, double resolution, ISMGridVolume& grid)
    {
    if (polygon.size() < 3)
        return DTMStatusInt::DTM_ERROR; // invalid region
    if (diffMesh == nullptr)
        return DTMStatusInt::DTM_ERROR; // invalid mesh

    DRange3d rangeMesh; // extend of the scalable mesh
    m_scmPtr->GetRange(rangeMesh);

    DRange3d rangeMesh2; // extend of the second scalable mesh
    diffMesh->GetRange(rangeMesh2);

    double zmin = std::min(rangeMesh.low.z, rangeMesh2.low.z);
    double zmax = std::max(rangeMesh.high.z, rangeMesh2.high.z);

    DRange3d rangeInter;
    rangeInter.IntersectionOf(rangeMesh, rangeMesh2);
    rangeInter.low.z = zmin;
    rangeInter.high.z = zmax;

    DRange3d rangeRegion; // Extend of the polygon region - extended to mesh Z
    DPoint3d P1 = polygon[0]; P1.z = zmin;  // keep only xy
    DPoint3d P2 = polygon[1]; P2.z = zmax;
    rangeRegion = DRange3d::From(P1, P2);
    for (auto point : polygon)
        rangeRegion.Extend(point);

    // Initialize the grid volume structure
    if (!_InitGridFrom(grid, resolution, rangeInter, rangeRegion))
        return DTMStatusInt::DTM_ERROR; // could not initialize grid

    DRange3d range = grid.m_range; // the intersection range (mesh inter region)
    int m_xSize, m_ySize;
    grid.GetGridSize(m_xSize, m_ySize);

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
        return DTMStatusInt::DTM_SUCCESS; // could not generate polygon from points

    auto draping1 = m_scmPtr->GetDTMInterface()->GetDTMDraping();
    auto draping2 = diffMesh->GetDTMInterface()->GetDTMDraping();

    bool *intersected = new bool[m_xSize*m_ySize];

    double tolerance = std::numeric_limits<double>::min(); // intersection tolerance - SNU TODO
    double zsource = range.low.z - tolerance; // be sure to start under the range
    double m_xStep = grid.m_resolution;
    double m_yStep = grid.m_resolution;

    int numProcs = omp_get_num_procs();
    if (numProcs>1)
        numProcs = numProcs - 1; // use all available CPUs minus one

#pragma omp parallel for num_threads(numProcs) 
    for (int i = 0; i < m_xSize; i++)
        {
        double x = range.low.x + m_xStep * i;
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); // added here because of parallelisation

        for (int j = 0; j < m_ySize; j++)
            {
            intersected[i*m_ySize + j] = false;
            double y = range.low.y + m_yStep * j;
            DPoint3d source = DPoint3d::From(x, y, zsource);

            DPoint3d interP1;
            bool bret = draping1->IntersectRay(interP1, grid.m_direction, source);

            if (bret)
                {
                auto classif = curveVectorPtr->PointInOnOutXY(source);
                if (classif == CurveVector::InOutClassification::INOUT_In) // we are inside the restriction
                    {
                    DPoint3d interP2;
                    bret = draping2->IntersectRay(interP2, grid.m_direction, source);
                    if (bret)
                        {
                        // Add one segment
                        grid.m_VolSegments[i*m_ySize + j].VolumeRanges.push_back(interP1.z);
                        grid.m_VolSegments[i*m_ySize + j].VolumeRanges.push_back(interP2.z);
                        intersected[i*m_ySize + j] = true;
                        }
                    }
                }
            }
        }

    // Sum the discrete volumes
    double AreaCell = m_xStep*m_yStep;
    double _cutVolume = 0, _fillVolume = 0;
#pragma omp parallel for num_threads(numProcs) reduction(+:_fillVolume,_cutVolume)
    for (int i = 0; i < m_xSize; i++) // need to parallelize
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
                }
            }
        }
    grid.m_fillVolume = _fillVolume;
    grid.m_cutVolume = _cutVolume;
    grid.m_totalVolume = grid.m_fillVolume + grid.m_cutVolume;

    delete[] intersected;

    return DTMStatusInt::DTM_SUCCESS;
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
