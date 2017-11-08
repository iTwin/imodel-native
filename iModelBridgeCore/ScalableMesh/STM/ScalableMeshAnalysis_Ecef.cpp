#include "ScalableMeshPCH.h"
#include "ScalableMeshAnalysis.h"
#include <omp.h>  

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
//#define SM_ANALYSIS_DEBUG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Stephane.Nullans                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct Ellipsoid
    {
    Ellipsoid();
    Ellipsoid(double _a, double _invf);
    double a, invf, b, e, eprime;

    DMatrix3d m_rot;
    DPoint3d m_center;

    // Convert geodetic latitude,longitude,height (in radians) to/from cartesian coordinates in the Earth-centered Earth-fixed cartesian referential
    DPoint3d LLH2ECEF(const DPoint3d& LatitudeLongitudeHeight) const;
    DPoint3d ECEF2LLH(const DPoint3d& XYZ) const;
    static Ellipsoid WGS84();
    //static Ellipsoid GRS80();

    // From ECEF to local ENU referential and inverse
    void ECEF2ENU(double latitude, double longitude, double alt = 0);

    // Point convertions
    DPoint3d ecef2enu(const DPoint3d &p);
    DPoint3d enu2ecef(const DPoint3d &p);
    DRange3d ecef2enu(const DRange3d &range_ecef);
    DRange3d enu2ecef(const DRange3d &range_enu);
    };

// Parameters from http://en.wikipedia.org/wiki/Geodetic_system
Ellipsoid Ellipsoid::WGS84() {
    return Ellipsoid(6378137.0, 298.257223563);
    }

Ellipsoid::Ellipsoid(double _a, double _invf) : a(_a), invf(_invf)
    {
    b = (a * (invf - 1.)) / invf;
    e = sqrt((a*a - b*b) / (a*a));
    eprime = sqrt((a*a - b*b) / (b*b));
    }

Ellipsoid::Ellipsoid()
    {
    }

// Convert latitude,longitude,altitude (in radians) in the WGS84 system to/from cartesian coordinates in the Earth-centered Earth-fixed cartesian referential
DPoint3d Ellipsoid::LLH2ECEF(const DPoint3d& llh) const
    {
    double
        cos_phi = cos(llh.x),
        sin_phi = sin(llh.x),
        cos_lambda = cos(llh.y),
        sin_lambda = sin(llh.y),
        N = a / sqrt(1. - (e*sin_phi)*(e*sin_phi)),
        h = llh.z;

    return DPoint3d::From(
        (N + h) * cos_phi * cos_lambda,
        (N + h) * cos_phi * sin_lambda,
        ((b*b) * N / (a*a) + h) * sin_phi
    );
    }

DMatrix3d EastNorthUp(double lat, double lon)
    {
    const double
        cos_phi = cos(lat),
        sin_phi = sin(lat),
        cos_lambda = cos(lon),
        sin_lambda = sin(lon);

    RotMatrix R; R.zero();
    R.form3d[0][0] = -sin_lambda;
    R.form3d[0][1] = cos_lambda;
    R.form3d[1][0] = -sin_phi * cos_lambda;
    R.form3d[1][1] = -sin_phi * sin_lambda;
    R.form3d[1][2] = cos_phi;
    R.form3d[2][0] = cos_phi * cos_lambda;
    R.form3d[2][1] = cos_phi * sin_lambda;
    R.form3d[2][2] = sin_phi;

    DMatrix3d M;
    M.initFromRotMatrix(&R);
    M.transpose();
    return M;
    }

// From ECEF to local ENU referential and inverse
void Ellipsoid::ECEF2ENU(double latitude, double longitude, double alt)
    {
    m_rot = EastNorthUp(latitude, longitude);
    m_rot.transpose();
    DPoint3d lla = DPoint3d::From(latitude, longitude, alt);
    m_center = LLH2ECEF(lla);
    }

DPoint3d Ellipsoid::ecef2enu(const DPoint3d &p)
    {
    DPoint3d M = p - m_center;
    m_rot.multiply(&M);
    return M;
    }

DPoint3d Ellipsoid::enu2ecef(const DPoint3d &p)
    {
    DPoint3d M = p;// -m_center;
    m_rot.multiplyTranspose(&M);
    M = M + DVec3d::From(m_center);
    return M;
    }

DRange3d Ellipsoid::ecef2enu(const DRange3d &range_ecef)
    {
    DRange3d rangeEnu = DRange3d::From(0, 0);
    DPoint3d Corners[8];
    range_ecef.Get8Corners(Corners);
    for (int k = 0; k < 8; k++)
        rangeEnu.Extend(ecef2enu(Corners[k]));
    return rangeEnu;
    }

DRange3d Ellipsoid::enu2ecef(const DRange3d &range_enu)
    {
    DRange3d rangeEcef = DRange3d::From(0, 0);
    DPoint3d Corners[8];
    range_enu.Get8Corners(Corners);
    for (int k = 0; k < 8; k++)
        rangeEcef.Extend(enu2ecef(Corners[k]));
    return rangeEcef;
    }

double degree2radian(double deg)
    {
    return deg / 180.0*PI;
    }

// Returns the WSG84 ellipsoid from a given Ecef center, 
// and computes the local direction
Ellipsoid GetEllipsoidFromWorldLocation(IScalableMesh* m_scmPtr, DPoint3d smCenter, DVec3d &dirW)
    {
    Ellipsoid ewgs84 = Ellipsoid::WGS84();
    GeoCoordinates::BaseGCSPtr myBase = m_scmPtr->GetBaseGCS();
    GeoPoint latlong; // get the latlong from mesh center
    myBase->LatLongFromXYZ(latlong, smCenter);
    // Init the converter from given latlong
    ewgs84.ECEF2ENU(degree2radian(latlong.latitude), degree2radian(latlong.longitude), latlong.elevation);
    
    GeoPoint latlongUp = latlong;
    latlongUp.elevation += 100;
    DPoint3d ecefup;
    myBase->XYZFromLatLong(ecefup, latlongUp);
    dirW = (ecefup - smCenter);
    dirW.Normalize();

    return ewgs84;
    }

bool ScalableMeshAnalysis::_GetComputationParamsInEnu(DRange3d& rangeEnu, bvector<DPoint3d>& polygonEnu,
                                    Ellipsoid *ewgs84, const bvector<DPoint3d>& polygonWorld, IScalableMesh* diffMesh)
    {
    DRange3d rangeMesh; // extend of the scalable mesh in Ecef coords
    m_scmPtr->GetRange(rangeMesh);
    if (diffMesh != nullptr)
        {
        DRange3d rangeMesh2;
        diffMesh->GetRange(rangeMesh2);
        DRange3d rangeInter;
        rangeInter.IntersectionOf(rangeMesh, rangeMesh2);
        if (rangeInter.IsNull())
            return false; // cannot compute volume, no intersection
        rangeMesh = rangeInter;
        }

    bvector<DPoint3d> polygonEcef = polygonWorld; // convert the polygon points first in ECEF
    for (auto &point : polygonEcef)
        _convertWorldTo3SM(m_scmPtr, point);

    DRange3d rangeRegionEnu = DRange3d::From(0, 0);
    for (auto point : polygonEcef)
        {
        DPoint3d ecef = ewgs84->ecef2enu(point);
        polygonEnu.push_back(ecef);
        rangeRegionEnu.Extend(ecef);
        }

    DRange3d rangeMeshEnu; // the mesh range in ENU
    rangeMeshEnu = ewgs84->ecef2enu(rangeMesh);

    rangeEnu = DRange3d::From(0, 0);
    rangeEnu.IntersectionOf(rangeMeshEnu, rangeRegionEnu);
    if (rangeEnu.IsNull())
        return false; // cannot compute volume, no intersection with restriction region

    // keep the lowest Z value for volume 
    rangeEnu.low.z = std::min(rangeMeshEnu.low.z, rangeEnu.low.z);
    return true;
    }

bool ScalableMeshAnalysis::_InitGridFrom(ISMGridVolume& grid, double _resolution, const DRange3d& _range)
    {
    if (_range.IsNull())
        return false; // cannot compute volume, no intersection
    grid.m_range = _range;
    grid.m_resolution = _resolution;
    double maxLength = std::max(grid.m_range.XLength(), grid.m_range.YLength());
    int gridSize = maxLength / grid.m_resolution;
    if (gridSize > grid.m_gridSizeLimit)
        grid.m_resolution = maxLength / grid.m_gridSizeLimit; // we limit the resolution

    grid.m_center = DPoint3d::From(grid.m_range.low.x + grid.m_range.XLength() / 2,
        grid.m_range.low.y + grid.m_range.YLength() / 2,
        grid.m_range.low.z + grid.m_range.ZLength() / 2);

    // split the range in regular grid xy
    double xSize = (grid.m_range.high.x - grid.m_range.low.x) / grid.m_resolution; // Nb of cells on X
    double ySize = (grid.m_range.high.y - grid.m_range.low.y) / grid.m_resolution; // Nb of cells on Y
    if (xSize <= 0 || ySize <= 0)
        return DTMStatusInt::DTM_ERROR; // Zero sized Grid

    // Init and reserve the memory for the volume segments
    bool bRes = grid.InitGrid(xSize, ySize);
    return bRes;
    }

DTMStatusInt ScalableMeshAnalysis::_ComputeDiscreteVolumeEcef(const bvector<DPoint3d>& polygon, double resolutionInMeter, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener)
    {
    if (polygon.size() < 3)
        return DTMStatusInt::DTM_ERROR; // invalid region

#ifdef SM_ANALYSIS_DEBUG
    std::ofstream f;
    f.open("c:\\Dev\\logDebugGrid_sdk.txt", ios_base::app);
    f << "==  _ComputeDiscreteVolumeEcef() =============" << endl;
    for (auto pp : polygon)
        f << "Polygon :: " << pp.x << " , " << pp.y << " , " << pp.z << endl;
    f << "==============================================" << endl;
    f.close();
#endif

    SMProgressReport report(pProgressListener);
    if (!report.CheckContinueOnProgress())
        return DTMStatusInt::DTM_ERROR; //User abort

    DVec3d directionEnu = DVec3d::From(0, 0, 1);
    DVec3d directionWorld;
    DRange3d rangeMesh;
    m_scmPtr->GetRange(rangeMesh);
    DPoint3d smCenter = DPoint3d::From(rangeMesh.low.x + rangeMesh.XLength() / 2, rangeMesh.low.y + rangeMesh.YLength() / 2, rangeMesh.low.z + rangeMesh.ZLength() / 2);
    Ellipsoid ewgs84 = GetEllipsoidFromWorldLocation(m_scmPtr, smCenter, directionWorld);
    
    DRange3d rangeEnu;
    bvector<DPoint3d> polygonEnu;
    _GetComputationParamsInEnu(rangeEnu, polygonEnu, &ewgs84, polygon, nullptr);

    if (!_InitGridFrom(grid, resolutionInMeter, rangeEnu))
        return DTMStatusInt::DTM_ERROR; // could not initialize grid

    int m_xSize, m_ySize;
    grid.GetGridSize(m_xSize, m_ySize);

    // Create the Polyface for intersections and queries
    PolyfaceHeaderPtr polyface;
    //ICurvePrimitivePtr curvePtr(ICurvePrimitive::CreateLineString(polygon));
    ICurvePrimitivePtr curvePtr(ICurvePrimitive::CreateLineString(polygonEnu));
    CurveVectorPtr curveVectorPtr(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr));
    IFacetOptionsPtr facetOptions = IFacetOptions::Create();
    facetOptions->SetNormalsRequired(false);
    facetOptions->SetMaxPerFace(3);
    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(*facetOptions);
    polyfaceBuilder->AddRegion(*curveVectorPtr);
    polyface = polyfaceBuilder->GetClientMeshPtr();

    auto draping1 = m_scmPtr->GetDTMInterface()->GetDTMDraping();
    bool isTerrain = true; // m_scmPtr->IsTerrain();

    bool *intersected = new bool[m_xSize*m_ySize];
    memset(intersected, 0, sizeof(bool)*m_xSize*m_ySize);

    double tolerance = std::numeric_limits<double>::min(); // intersection tolerance
    double zsource = rangeEnu.low.z - tolerance; // be sure to start under the range
    double m_xStep = resolutionInMeter;
    double m_yStep = resolutionInMeter;

    int numProcs = 1;
    if (numProcs > 1)
        numProcs = numProcs - 1; // use all available CPUs minus one

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

        double x = rangeEnu.low.x + m_xStep * i;
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); // added here because of parallelisation

        for (int j = 0; j < m_ySize; j++)
            {
            intersected[i*m_ySize + j] = false;

            double y = rangeEnu.low.y + m_yStep * j;
            DPoint3d sourceEnu = DPoint3d::From(x, y, zsource);
            DPoint3d sourceEcef = ewgs84.enu2ecef(sourceEnu);
            DPoint3d sourceW = sourceEcef;
            _convert3SMToWorld(m_scmPtr, sourceW); // 3SM Intersection interface needs world coords

            bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection> Hits;
            bool bret = draping1->IntersectRay(Hits, directionWorld, sourceW);

            if (bret && Hits.size() > 0)
                {
                        {
                        DRay3d ray1 = DRay3d::FromOriginAndVector(sourceW, directionWorld);
                        DRay3d ray = DRay3d::FromOriginAndVector(sourceEnu, directionEnu);
                        DPoint3d polyHit; polyHit.Zero();
                        double rayFraction = 0;
                        visitor->Reset();
                        if (visitor->AdvanceToFacetBySearchRay(ray, tolerance, polyHit, rayFraction))
                            {
                            // convert the intersections in meter
                            SMVolumeSegment aSegment;
                            if (isTerrain) // just stack the 2 polyHit
                                {
                                aSegment.VolumeRanges.push_back(rayFraction); // polyHit.z);
                                double elevation = directionWorld.DotProduct(Hits[0].point - sourceW);
                                double hitFraction = Hits[0].rayFraction;
                                aSegment.VolumeRanges.push_back(hitFraction);
                                std::cout << "elevation " << elevation;
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

    _FillGridVolumes(grid, intersected); // Sum the discrete volumes

    // Convert the range from Enu to World ???
    DRange3d _range = grid.m_range;
    grid.m_range = ewgs84.enu2ecef(_range);
    grid.m_isEcef = true;

    if (!userAborted) // update only if not aborted
        report.m_workDone = 1.0;

    delete[] intersected;

    return DTMStatusInt::DTM_SUCCESS;
    }


DTMStatusInt ScalableMeshAnalysis::_ComputeDiscreteVolumeEcef(const bvector<DPoint3d>& polygon, IScalableMesh* diffMesh, double resolutionInMeter, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener)
{
    if (polygon.size() < 3)
        return DTMStatusInt::DTM_ERROR; // invalid region

#ifdef SM_ANALYSIS_DEBUG
    std::ofstream f;
    f.open("c:\\Dev\\logDebugGrid_sdk.txt", ios_base::app);
    f << "==  _ComputeDiscreteVolumeEcef() =============" << endl;
    f << setprecision(12);
    for (auto pp : polygon)
        f << "Polygon :: " << pp.x << " , " << pp.y << " , " << pp.z << endl;
    f << "==============================================" << endl;
    f.close();
#endif

    SMProgressReport report(pProgressListener);
    if (!report.CheckContinueOnProgress())
        return DTMStatusInt::DTM_ERROR; //User abort

    DVec3d directionEnu = DVec3d::From(0, 0, 1);
    DVec3d directionWorld;
    DRange3d rangeMesh;
    m_scmPtr->GetRange(rangeMesh);
    DPoint3d smCenter = DPoint3d::From(rangeMesh.low.x + rangeMesh.XLength() / 2, rangeMesh.low.y + rangeMesh.YLength() / 2, rangeMesh.low.z + rangeMesh.ZLength() / 2);
    Ellipsoid ewgs84 = GetEllipsoidFromWorldLocation(m_scmPtr, smCenter, directionWorld);
    DRange3d rangeEnu;
    bvector<DPoint3d> polygonEnu;
    _GetComputationParamsInEnu(rangeEnu, polygonEnu, &ewgs84, polygon, diffMesh);

    // Create the Grid in Enu for the computation
    if (!_InitGridFrom(grid, resolutionInMeter, rangeEnu))
        return DTMStatusInt::DTM_ERROR; // could not initialize grid

    int m_xSize, m_ySize;
    grid.GetGridSize(m_xSize, m_ySize);

    // Create the Polyface for intersections and queries
    ICurvePrimitivePtr curvePtr(ICurvePrimitive::CreateLineString(polygonEnu));
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
    memset(intersected, 0, sizeof(bool)*m_xSize*m_ySize);

    DPoint2d *interPoints = new DPoint2d[m_xSize*m_ySize]; // store 
    memset(interPoints, 0, sizeof(DPoint2d)*m_xSize*m_ySize);

    double tolerance = std::numeric_limits<double>::min(); // intersection tolerance - SNU TODO
    double zsource = rangeEnu.low.z - tolerance; // be sure to start under the range
    double m_xStep = grid.m_resolution;
    double m_yStep = grid.m_resolution;

    int numProcs = std::min(m_ThreadNumber, omp_get_num_procs());
    if (numProcs > 1)
        numProcs = numProcs - 1; // use all available CPUs minus one

    double progress = 0.0;
    double progressStep = 1.0 / (m_xSize + 1);
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

        double x = rangeEnu.low.x + m_xStep * i;
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); // added here because of parallelisation

        for (int j = 0; j < m_ySize; j++)
            {
            intersected[i*m_ySize + j] = false;
            double y = rangeEnu.low.y + m_yStep * j;

            DPoint3d sourceEnu = DPoint3d::From(x, y, zsource); // polyface is in Enu
            DPoint3d sourceEcef = ewgs84.enu2ecef(sourceEnu);
            DPoint3d sourceW = sourceEcef;
            _convert3SMToWorld(m_scmPtr, sourceW); // 3SM Intersection interface needs world coords

            DPoint3d interP1;
            bool bret = draping1->IntersectRay(interP1, directionWorld, sourceW);

            if (bret)
                {
                auto classif = curveVectorPtr->PointInOnOutXY(sourceEnu);
                if (classif == CurveVector::InOutClassification::INOUT_In) // we are inside the restriction
                    {
                    DPoint3d interP2;
                    DPoint3d sourceW2 = sourceEcef;
                    _convert3SMToWorld(diffMesh, sourceW2); // 3SM Intersection interface needs world coords

                    bret = draping2->IntersectRay(interP2, directionWorld, sourceW2); // second SM is supposed to be in same GCS
                    if (bret)
                        {
                        interPoints[i*m_ySize + j] = DPoint2d::From(interP1.z, interP2.z);
                        intersected[i*m_ySize + j] = true;
                        }
                    }
                }
            }
#pragma omp critical
        {
        progress += progressStep;
        }
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
                grid.m_VolSegments[i*m_ySize + j].VolumeRanges.push_back(interPoints[i*m_ySize + j].x);
                grid.m_VolSegments[i*m_ySize + j].VolumeRanges.push_back(interPoints[i*m_ySize + j].y);
                }
            }
        }

    // Sum the discrete volumes
    _FillGridVolumes(grid, intersected, m_unit2meter);

    // Convert the range from Enu to World
    DRange3d _range = grid.m_range;
    grid.m_range = ewgs84.enu2ecef(_range);
    grid.m_isEcef = true;

    if (!userAborted) // update only if not aborted
        report.m_workDone = 1.0;

    delete[] intersected;

    return DTMStatusInt::DTM_SUCCESS;
}


END_BENTLEY_SCALABLEMESH_NAMESPACE