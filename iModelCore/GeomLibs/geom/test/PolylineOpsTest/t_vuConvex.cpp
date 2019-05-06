/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <Vu/vuprint.fdf>
// unused - static int s_noisy = 0;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Vu,ConvexParts)
    {
    double a = 10.0;
    double b = 2.0;
    double c = 4.0;
    double d = 5.0;
    double e = 6.0;

    double f = 4.5;

    bvector<bvector<DPoint3d>> allLoops {
        bvector<DPoint3d> {
            DPoint3d::From (0,0,0),
            DPoint3d::From (a,0,0),
            DPoint3d::From (a,a,0),
            DPoint3d::From (0,a,0),
            },
        bvector<DPoint3d> {
            DPoint3d::From (b,b,0),
            DPoint3d::From (c,b,0),
            DPoint3d::From (c,c,0),
            DPoint3d::From (b,c,0),
            },
        bvector<DPoint3d> {
            DPoint3d::From (d,b,0),
            DPoint3d::From (e,b,0),
            DPoint3d::From (e,c,0),
            DPoint3d::From (d,c,0),
            },
        bvector<DPoint3d> {
            DPoint3d::From (f,d,0),
            DPoint3d::From (e,e,0),
            DPoint3d::From (f,e,0),
            DPoint3d::From (f,d,0),
            },
            // Wow - exact duplicate of [2] -- parity cancels, convex expansion starts 
            // somewhere that fortuitously floods it away.
        bvector<DPoint3d> {
            DPoint3d::From (d,b,0),
            DPoint3d::From (e,b,0),
            DPoint3d::From (e,c,0),
            DPoint3d::From (d,c,0),
            },
        };
    bvector<bvector<DPoint3d>> activeLoops, convexLoops;
    bvector<bvector<bool>> isBoundary;

    for (auto &loop : allLoops)
        {
        SaveAndRestoreCheckTransform shifter (2*a, 0,0);
        activeLoops.push_back (loop);
        vu_splitToConvexParts (activeLoops, 1, convexLoops, &isBoundary);
        Check::SaveTransformed (activeLoops);
        Check::Shift (0, 2.0 * a, 0);
        Check::SaveTransformed (convexLoops);
        }
    Check::ClearGeometry ("Vu.ConvexParts");
    }
// Source code (with complete implementation) for CurveVectorXYOffsetContext struct ...
// BEWARE -- .mke dependencies might not know about this ...
#include "CurveVectorXYOffsetContext.h"

void FractalA(bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor);
void Fractal0(bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor);

TEST(Vu, XYOffset)
    {
    bvector<bvector<DPoint3d>> interiorLoops;
    double a = 10.0;
    double b = 20.0;
    auto pointsA = CreateT(a, b, 4.0, 2.0, 10.0, 12.0);
    bvector<DPoint3d> fractalA;
    FractalA (fractalA, 1, 0.4);
    bvector<DPoint3d> fractalB;
    Fractal0(fractalB, 1, 0.4);
    bvector<bvector<DPoint3d>> allLoops;
    allLoops.push_back (pointsA);
    allLoops.push_back(fractalA);
    allLoops.push_back(fractalB);
    for (bvector<DPoint3d> &singleLoop : allLoops )
        {
        auto range = DRange3d::From(singleLoop);
        double ax = 0.1 * range.XLength();
        double bx = 0.1 * range.XLength();
        double ay = 0.03 * range.YLength();
        double by = 0.03 * range.YLength();
        SaveAndRestoreCheckTransform shifter (0, 2.0 * range.YLength (), 0);
        for (auto degrees : {0.0, 1.0, -1.0, 5.0, 8.0, 20.0, 40.0})
            {
            bvector<bvector<DPoint3d>> inputLoops;
            inputLoops.push_back (singleLoop);
            auto transform = Transform::FromMatrixAndFixedPoint(
                    RotMatrix::FromAxisAndRotationAngle(2, Angle::DegreesToRadians (degrees)),
                    DPoint3d::FromInterpolate (range.low, 0.5, range.high));
            transform.Multiply (inputLoops.back ());
            vu_createXYOffsetLoops(inputLoops, ax, bx, ay, by, interiorLoops);
            Check::SaveTransformed(inputLoops);
            Check::SaveTransformed(interiorLoops);
            Check::Shift (3.0 * range.XLength (), 0, 0);
            }
        }
    Check::ClearGeometry("Vu.XYOffset");
    }

TEST(CurveVector, XYOffset)
    {
    bvector<bvector<DPoint3d>> interiorLoops;
    double a = 10.0;
    double b = 20.0;
    auto pointsA = CreateT(a, b, 4.0, 2.0, 10.0, 12.0);
    bvector<DPoint3d> fractalA;
    FractalA(fractalA, 1, 0.4);
    bvector<DPoint3d> fractalB;
    Fractal0(fractalB, 1, 0.4);
    bvector<bvector<DPoint3d>> allLoops;
    allLoops.push_back(pointsA);
    allLoops.push_back(fractalA);
    allLoops.push_back(fractalB);
    allLoops.push_back(bvector<DPoint3d> {
        DPoint3d::From (0,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (10,10,0),
        DPoint3d::From (0,10,0),
        DPoint3d::From (0,8,0),
        DPoint3d::From (8,8,0),
        DPoint3d::From (8,2,0),
        DPoint3d::From (0,2,0),
        DPoint3d::From (0,0,0)
        });
    for (bvector<DPoint3d> &singleLoop : allLoops)
        {
        auto range = DRange3d::From(singleLoop);
        double ax = 0.1 * range.XLength();
        double bx = 0.1 * range.XLength();
        double ay = 0.03 * range.YLength();
        double by = 0.03 * range.YLength();
        DRange2d chop;
        static double s_expansionFactor = 1.001;
        chop.low = DPoint2d::From (-s_expansionFactor * bx, -s_expansionFactor * by);
        chop.high = DPoint2d::From(s_expansionFactor * ax, s_expansionFactor * ay);

        auto textBox = CurveVector::CreateRectangle(
            -bx, -by, ax, ay, 0.0);
        Check::SaveTransformed(textBox);

        double dy = 1.5 * range.YLength ();
        for (auto degrees : { 0.0, 1.0, -1.0, 5.0, 20.0, 40.0, 60.0 })
            {
            SaveAndRestoreCheckTransform shifter(3.0 * range.XLength(), 0, 0);
            auto rotatedChop = textBox->Clone(
                    Transform::From (RotMatrix::FromAxisAndRotationAngle(2, Angle::DegreesToRadians (degrees))));
            auto shape = CurveVector::CreateLinear(singleLoop, CurveVector::BOUNDARY_TYPE_Outer);
            Check::SaveTransformed (shape);

            bvector<CurveVectorPtr> debugGeometry;
            auto result = CurveVectorXYOffsetContext::ComputeCenterRegionForRectangularBox(*shape, ax + bx, ay + by,
                        Angle::DegreesToRadians (degrees), &debugGeometry);
            Check::Shift (0, 1.5 * range.YLength (), 0);
            for (auto &g : debugGeometry)
                Check::SaveTransformed (g);
            Check::Shift(0, 1.5 * dy, 0);
            if (result.IsValid ())
                {
                static int s_showManyCuts = 0;
                if (s_showManyCuts)
                    {
                    double spacing = result->FastLength () * 0.10;
                    auto strokeOptions = IFacetOptions::CreateForCurves ();
                    strokeOptions->SetMaxEdgeLength (spacing);
                    bvector<DPoint3d> locations;
                    result->AddStrokePoints(locations, *strokeOptions);
                    // output some scattered boxes . ..
                    for (auto &xyz : locations)
                        {
                        auto box = rotatedChop->Clone (Transform::From (xyz));
                        Check::SaveTransformed (box);
                        }
                    Check::SaveTransformed(result);
                    }
                Check::Shift(0, 2.0 * dy, 0);
                Check::SaveTransformed(shape);
                for (int effort : {0, 1, 10})
                    {
                    // and output points with various effort to get to the inside . . .
                    DPoint3d xyzA;
                    if (CurveVectorXYOffsetContext::ChooseBoxCenter (*shape, *result, effort, xyzA))
                        Check::SaveTransformed (*rotatedChop->Clone(Transform::From(xyzA)));
                    }
                }
            }
        }
    Check::ClearGeometry("CurvVector.XYOffset");
    }
CurveVectorPtr CurveVectorA0()
{
auto cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);


cv->push_back(ICurvePrimitive::CreateLineString(
    bvector<DPoint3d>{
    DPoint3d::From(-275357063.70175552, 147908572.4885180, 0.0),
        DPoint3d::From(-644975063.70175552, -39509927.511481971, 0.0),
        DPoint3d::From(-385243563.70175552, -221930427.51148203, 0.0),
        DPoint3d::From(11845936.298244536, -286901927.51148206, 0.0),
        DPoint3d::From(-275357063.70175552, 147908572.4885180, 0.0)}));

return cv;
}

CurveVectorPtr CurveVectorA1()
{
auto cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);


cv->push_back(ICurvePrimitive::CreateLineString(
    bvector<DPoint3d>{
    DPoint3d::From(-277645063.70175552, 16629072.4885180, 0.0),
        DPoint3d::From(-359477563.70175552, -74349927.5114820, 0.0),
        DPoint3d::From(-330147278.16998136, -158326089.95656526, 0.0),
        DPoint3d::From(-195813063.70175546, -101643427.51148196, 0.0),
        DPoint3d::From(-277645063.70175552, 16629072.4885180, 0.0)}));

return cv;
}
CurveVectorPtr CurveVectorBWithHoles()
    {

    CurveVectorPtr parityRegion = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegion->Add (CurveVector::CreateLinear (
        bvector<DPoint3d>{
        DPoint3d::From(23542061000.0, 32822299000.0, 0.0),
            DPoint3d::From(23538424000.0, 32824138000.0, 0.0),
            DPoint3d::From(23534508000.0, 32825315000.000004, 0.0),
            DPoint3d::From(23530450000.0, 32825877999.999996, 0.0),
            DPoint3d::From(23526364000.0, 32825692000.000004, 0.0),
            DPoint3d::From(23522384000.0, 32824803000.0, 0.0),
            DPoint3d::From(23518642000.0, 32823262000.000004, 0.0),
            DPoint3d::From(23515049000.0, 32821449000.0, 0.0),
            DPoint3d::From(23512927000.0, 32820375000.0, 0.0),
            DPoint3d::From(23471706000.0, 32777670000.0, 0.0),
            DPoint3d::From(23465237000.0, 32768773000.0, 0.0),
            DPoint3d::From(23456688000.0, 32759950000.0, 0.0),
            DPoint3d::From(23447640000.0, 32752681000.0, 0.0),
            DPoint3d::From(23420468000.0, 32724472000.0, 0.0),
            DPoint3d::From(23420670000.0, 32715020000.0, 0.0),
            DPoint3d::From(23451699000.0, 32685122000.0, 0.0),
            DPoint3d::From(23455859000.0, 32680890000.0, 0.0),
            DPoint3d::From(23459785000.0, 32676443000.0, 0.0),
            DPoint3d::From(23463465000.0, 32671790000.0, 0.0),
            DPoint3d::From(23466891000.0, 32666948000.0, 0.0),
            DPoint3d::From(23470047000.0, 32661922000.0, 0.0),
            DPoint3d::From(23472929000.0, 32656737000.0, 0.0),
            DPoint3d::From(23475528000.0, 32651404000.0, 0.0),
            DPoint3d::From(23477836000.0, 32645939000.0, 0.0),
            DPoint3d::From(23479844000.0, 32640356000.0, 0.0),
            DPoint3d::From(23481552000.0, 32634675000.0, 0.0),
            DPoint3d::From(23482947000.0, 32628907000.0, 0.0),
            DPoint3d::From(23484111000.0, 32623797000.0, 0.0),
            DPoint3d::From(23493728000.0, 32578757000.0, 0.0),
            DPoint3d::From(23506506000.0, 32601396000.0, 0.0),
            DPoint3d::From(23500763000.0, 32622817000.0, 0.0),
            DPoint3d::From(23527672000.0, 32630037000.0, 0.0),
            DPoint3d::From(23538589000.0, 32590069000.0, 0.0),
            DPoint3d::From(23536787000.0, 32587020000.0, 0.0),
            DPoint3d::From(23539993000.0, 32585137000.0, 0.0),
            DPoint3d::From(23574521000.0, 32594485000.0, 0.0),
            DPoint3d::From(23589558000.0, 32538664000.0, 0.0),
            DPoint3d::From(23592960000.0, 32536699000.0, 0.0),
            DPoint3d::From(23584497000.0, 32522017000.0, 0.0),
            DPoint3d::From(23573354000.0, 32528334000.0, 0.0),
            DPoint3d::From(23555091000.0, 32523359000.0, 0.0),
            DPoint3d::From(23550833000.0, 32526209000.0, 0.0),
            DPoint3d::From(23550251000.0, 32525264000.0, 0.0),
            DPoint3d::From(23553045000.0, 32514866000.0, 0.0),
            DPoint3d::From(23556980000.0, 32512623000.0, 0.0),
            DPoint3d::From(23554765000.0, 32508698000.0, 0.0),
            DPoint3d::From(23550788000.0, 32510933000.0, 0.0),
            DPoint3d::From(23540364000.0, 32508133000.0, 0.0),
            DPoint3d::From(23534716000.0, 32499328000.0, 0.0),
            DPoint3d::From(23511862000.0, 32493302000.0, 0.0),
            DPoint3d::From(23509748000.0, 32478510000.0, 0.0),
            DPoint3d::From(23541927000.0, 32485695000.0, 0.0),
            DPoint3d::From(23640682000.0, 32507062000.0, 0.0),
            DPoint3d::From(23653003000.0, 32453362000.0, 0.0),
            DPoint3d::From(23709776000.0, 32466176000.0, 0.0),
            DPoint3d::From(23711635000.0, 32465195000.0, 0.0),
            DPoint3d::From(23717203000.0, 32465458000.0, 0.0),
            DPoint3d::From(23721067000.0, 32465640000.0, 0.0),
            DPoint3d::From(23732356000.0, 32463044000.0, 0.0),
            DPoint3d::From(23745075000.0, 32458891000.0, 0.0),
            DPoint3d::From(23768238000.0, 32451293000.0, 0.0),
            DPoint3d::From(23769161000.0, 32450933000.0, 0.0),
            DPoint3d::From(23769806000.0, 32450177000.0, 0.0),
            DPoint3d::From(23770023000.0, 32449210000.0, 0.0),
            DPoint3d::From(23769789000.0, 32447644000.0, 0.0),
            DPoint3d::From(23775195000.0, 32458950000.0, 0.0),
            DPoint3d::From(23737081000.0, 32472346000.0, 0.0),
            DPoint3d::From(23744213000.0, 32482135000.0, 0.0),
            DPoint3d::From(23741145000.0, 32483543000.0, 0.0),
            DPoint3d::From(23738432000.0, 32484793000.0, 0.0),
            DPoint3d::From(23725102000.0, 32490935000.0, 0.0),
            DPoint3d::From(23714199000.0, 32495958000.0, 0.0),
            DPoint3d::From(23709599000.0, 32498076000.0, 0.0),
            DPoint3d::From(23704365000.0, 32501197000.0, 0.0),
            DPoint3d::From(23691174000.0, 32509063000.0, 0.0),
            DPoint3d::From(23674070000.0, 32511954000.0, 0.0),
            DPoint3d::From(23663316000.0, 32513773000.0, 0.0),
            DPoint3d::From(23662282000.0, 32518917000.0, 0.0),
            DPoint3d::From(23584277000.0, 32501270000.0, 0.0),
            DPoint3d::From(23582274000.0, 32510124000.0, 0.0),
            DPoint3d::From(23591044000.0, 32512108000.0, 0.0),
            DPoint3d::From(23589970000.0, 32516856000.0, 0.0),
            DPoint3d::From(23593162000.0, 32517578000.0, 0.0),
            DPoint3d::From(23593913000.0, 32514256000.0, 0.0),
            DPoint3d::From(23631472000.0, 32522753000.0, 0.0),
            DPoint3d::From(23630721000.0, 32526075000.0, 0.0),
            DPoint3d::From(23633912000.0, 32526797000.0, 0.0),
            DPoint3d::From(23634663000.0, 32523475000.0, 0.0),
            DPoint3d::From(23655058000.0, 32528089000.0, 0.0),
            DPoint3d::From(23655381000.0, 32526663000.0, 0.0),
            DPoint3d::From(23660505000.0, 32527822000.0, 0.0),
            DPoint3d::From(23652275000.0, 32569119000.0, 0.0),
            DPoint3d::From(23656740000.0, 32619186000.0, 0.0),
            DPoint3d::From(23652500000.0, 32643286000.0, 0.0),
            DPoint3d::From(23651250000.0, 32650384000.0, 0.0),
            DPoint3d::From(23645377000.0, 32683765000.0, 0.0),
            DPoint3d::From(23645141000.0, 32685116000.0, 0.0),
            DPoint3d::From(23639998000.0, 32714315000.0, 0.0),
            DPoint3d::From(23637909000.0, 32726175000.0, 0.0),
            DPoint3d::From(23631473000.0, 32739521000.0, 0.0),
            DPoint3d::From(23627429000.0, 32747905000.0, 0.0),
            DPoint3d::From(23618302000.0, 32766821000.0, 0.0),
            DPoint3d::From(23617511000.0, 32768777000.0, 0.0),
            DPoint3d::From(23542061000.0, 32822299000.0, 0.0)}, CurveVector::BOUNDARY_TYPE_Outer));

    parityRegion->Add(CurveVector::CreateLinear(
        bvector<DPoint3d>{
        DPoint3d::From(23525673000.0, 32804196000.000004, 0.0),
            DPoint3d::From(23535277000.0, 32794936999.999996, 0.0),
            DPoint3d::From(23452158000.0, 32708754000.0, 0.0),
            DPoint3d::From(23442554000.0, 32718016000.0, 0.0),
            DPoint3d::From(23525673000.0, 32804196000.000004, 0.0)}, CurveVector::BOUNDARY_TYPE_Inner));

    parityRegion->Add(CurveVector::CreateLinear(
        bvector<DPoint3d>{
        DPoint3d::From(23535192000.0, 32761611000.0, 0.0),
            DPoint3d::From(23609239000.0, 32753480000.0, 0.0),
            DPoint3d::From(23631753000.0, 32669838000.0, 0.0),
            DPoint3d::From(23618483000.0, 32666266000.0, 0.0),
            DPoint3d::From(23598327000.0, 32741146000.0, 0.0),
            DPoint3d::From(23541437000.0, 32747395000.0, 0.0),
            DPoint3d::From(23513994000.0, 32718938000.0, 0.0),
            DPoint3d::From(23523070000.0, 32685093000.0, 0.0),
            DPoint3d::From(23510077000.0, 32681608000.0, 0.0),
            DPoint3d::From(23498755000.0, 32723828000.0, 0.0),
            DPoint3d::From(23535192000.0, 32761611000.0, 0.0)}, CurveVector::BOUNDARY_TYPE_Inner));
    parityRegion->Add(CurveVector::CreateLinear(
        bvector<DPoint3d>{
        DPoint3d::From(23515694000.0, 32667988000.0, 0.0),
            DPoint3d::From(23522062000.0, 32644238000.0, 0.0),
            DPoint3d::From(23495979000.0, 32637244000.0, 0.0),
            DPoint3d::From(23489612000.0, 32660998000.0, 0.0),
            DPoint3d::From(23515694000.0, 32667988000.0, 0.0)}, CurveVector::BOUNDARY_TYPE_Inner));

    parityRegion->Add(CurveVector::CreateLinear(
        bvector<DPoint3d>{
        DPoint3d::From(23562666000.0, 32650198000.0, 0.0),
            DPoint3d::From(23569033000.0, 32626446000.0, 0.0),
            DPoint3d::From(23542950000.0, 32619454000.0, 0.0),
            DPoint3d::From(23536583000.0, 32643206000.0, 0.0),
            DPoint3d::From(23562666000.0, 32650198000.0, 0.0)}, CurveVector::BOUNDARY_TYPE_Inner));
    parityRegion->Add(CurveVector::CreateLinear(
        bvector<DPoint3d>{
        DPoint3d::From(23596959000.0, 32666368000.0, 0.0),
            DPoint3d::From(23620849000.0, 32577258000.0, 0.0),
            DPoint3d::From(23606181000.0, 32573325000.0, 0.0),
            DPoint3d::From(23599802000.0, 32597113000.0, 0.0),
            DPoint3d::From(23596783000.0, 32596302000.0, 0.0),
            DPoint3d::From(23595103000.0, 32602570000.0, 0.0),
            DPoint3d::From(23598123000.0, 32603380000.0, 0.0),
            DPoint3d::From(23589544000.0, 32635377000.0, 0.0),
            DPoint3d::From(23586526000.0, 32634566000.0, 0.0),
            DPoint3d::From(23584845000.0, 32640834000.0, 0.0),
            DPoint3d::From(23587865000.0, 32641644000.0, 0.0),
            DPoint3d::From(23582290000.0, 32662433000.0, 0.0),
            DPoint3d::From(23596959000.0, 32666368000.0, 0.0)}, CurveVector::BOUNDARY_TYPE_Inner));
    return parityRegion;
    }

TEST(CurveVector, ChrisDFailure)
    {
    static int s_outputIntermediates = 0;
    bvector<bvector<DPoint3d>> interiorLoops;
    // for (bvector<DPoint3d> &singleLoop : allLoops)
    auto parityRegionA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegionA->Add (CurveVectorA0());
    parityRegionA->Add (CurveVectorA1());
    auto parityRegionB = CurveVectorBWithHoles ();
    double zShift = 0.01;
    double scale = 0.0001;
    for (double textSizeFactor : { 0.1, 0.02})
    for (auto parityRegion0 : {parityRegionA, parityRegionB})
        {
        auto parityRegion = CurveVector::ReduceToCCWAreas (*parityRegion0);
        auto scaleTransform = Transform::FromMatrixAndFixedPoint(
            RotMatrix::FromScale(scale),
            DPoint3d::From(0, 0, 0)
        );
        auto shape = parityRegion->Clone (scaleTransform);
            DRange3d range;
        shape->GetRange (range);
        double ax = textSizeFactor * range.XLength();
        double bx = ax;
        double ay = ax * 0.25;
        double by = ay;
        if (true) // 
            {
            SaveAndRestoreCheckTransform  shifter(0, range.YLength() * 20.0, 0.0);
            DRange2d chop;
            static double s_expansionFactor = 1.001;
            chop.low = DPoint2d::From(-s_expansionFactor * bx, -s_expansionFactor * by);
            chop.high = DPoint2d::From(s_expansionFactor * ax, s_expansionFactor * ay);

            auto textBox = CurveVector::CreateRectangle(
                -bx, -by, ax, ay, 0);
            Check::SaveTransformed(textBox);

            
            for (auto degrees : { 0.0, 1.0, -5.0, 20.0, -40.0})
                {
                SaveAndRestoreCheckTransform shifter(3.0 * range.XLength(), 0, 0);
                auto rotatedChop = textBox->Clone(
                    Transform::From(RotMatrix::FromAxisAndRotationAngle(2, Angle::DegreesToRadians(degrees))));
                Check::SaveTransformed(shape);
                bvector<CurveVectorPtr> debugGeometry;
                auto result = CurveVectorXYOffsetContext::ComputeCenterRegionForRectangularBox(*shape, ax + bx, ay + by,
                    Angle::DegreesToRadians(degrees), s_outputIntermediates ? &debugGeometry : nullptr);
                double debugY = 1.1 * range.YLength();
                Check::Shift(0, debugY, 0);
                for (auto &g : debugGeometry)
                    {
                    Check::SaveTransformed(bvector<DPoint3d>{
                        range.low,
                        DPoint3d::From (range.low.x, range.high.y, range.low.z),
                        range.high,
                        DPoint3d::From(range.low.x, range.low.y + debugY, range.low.z)
                        });
                    if (g.IsValid ())
                        Check::SaveTransformed(g);
                    else
                        Check::SaveTransformed (DSegment3d::From (range.low, range.high));
                    Check::Shift(0, debugY, 0);
                    }

                if (result.IsValid())
                    {
                    static int s_verifyLotsOfStrokePoints = 0;
                    if (s_verifyLotsOfStrokePoints)
                        {
                        double spacing = result->FastLength() * 0.10;
                        auto strokeOptions = IFacetOptions::CreateForCurves();
                        strokeOptions->SetMaxEdgeLength(spacing);
                        bvector<DPoint3d> locations;
                        result->AddStrokePoints(locations, *strokeOptions);
                        // output some scattered boxes . ..
                        for (auto &xyz : locations)
                            {
                            auto box = rotatedChop->Clone(Transform::From(xyz.x, xyz.y, 4 * zShift));
                            Check::SaveTransformed(box);
                            }
                        }
                    Check::Shift(0, 0, zShift);
                    Check::SaveTransformed(result);
                    Check::Shift(0, 0, -zShift);
                    Check::SaveTransformed(shape);
                    for (int effort : {0, 10})
                        {
                        // and output points with various effort to get to the inside . . .
                        DPoint3d xyzA;
                        if (CurveVectorXYOffsetContext::ChooseBoxCenter(*shape, *result, effort, xyzA))
                            Check::SaveTransformed(*rotatedChop->Clone(Transform::From(xyzA.x, xyzA.y, 2 * zShift)));
                        }
                    }
                }
            }
        }
    Check::ClearGeometry("CurvVector.ChrisDFailure");
    }

TEST(CurveVector, OffsetXYThinSection)
    {
    static int s_outputIntermediates = 0;
    double a = 8;

    for (double b : {0.5, 1.0, 1.5})
        {
        auto shape = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
        shape->Add(CurveVector::CreateRectangle(0, 0, 10, 10, 0, CurveVector::BOUNDARY_TYPE_Outer));
        shape->Add(CurveVector::CreateRectangle(a, 2, a + b, 5, 0, CurveVector::BOUNDARY_TYPE_Inner));
        double zShift = 0.01;
        for (double textSizeFactor : { 0.1, 0.02})
            {
            auto parityRegion = CurveVector::ReduceToCCWAreas(*shape);

            DRange3d range;
            shape->GetRange(range);
            double ax = textSizeFactor * range.XLength();
            double bx = ax;
            double ay = ax * 0.25;
            double by = ay;
            if (true) // 
                {
                SaveAndRestoreCheckTransform shifter(0, range.YLength() * 20.0, 0.0);
                DRange2d chop;
                static double s_expansionFactor = 1.001;
                chop.low = DPoint2d::From(-s_expansionFactor * bx, -s_expansionFactor * by);
                chop.high = DPoint2d::From(s_expansionFactor * ax, s_expansionFactor * ay);

                auto textBox = CurveVector::CreateRectangle(
                    -bx, -by, ax, ay, 0);
                Check::SaveTransformed(textBox);

                double dy = 1.5 * range.YLength();
                for (auto degrees : { 0.0, 4.0, -10.0, 70.0, 40.0 })
                    {
                    SaveAndRestoreCheckTransform shifter(3.0 * range.XLength(), 0, 0);
                    auto rotatedChop = textBox->Clone(
                        Transform::From(RotMatrix::FromAxisAndRotationAngle(2, Angle::DegreesToRadians(degrees))));
                    Check::SaveTransformed(shape);
                    bvector<CurveVectorPtr> debugGeometry;
                    auto result = CurveVectorXYOffsetContext::ComputeCenterRegionForRectangularBox(*shape, ax + bx, ay + by,
                        Angle::DegreesToRadians(degrees), s_outputIntermediates ? &debugGeometry : nullptr);
                    double debugY = 1.1 * range.YLength();
                    Check::Shift(0, debugY, 0);
                    for (auto &g : debugGeometry)
                        {
                        Check::SaveTransformed(bvector<DPoint3d>{
                            range.low,
                                DPoint3d::From(range.low.x, range.high.y, range.low.z),
                                range.high,
                                DPoint3d::From(range.low.x, range.low.y + debugY, range.low.z)
                            });
                        if (g.IsValid())
                            Check::SaveTransformed(g);
                        else
                            Check::SaveTransformed(DSegment3d::From(range.low, range.high));
                        Check::Shift(0, debugY, 0);
                        }

                    if (result.IsValid())
                        {
                        static int s_verifyLotsOfStrokePoints = 0;
                        if (s_verifyLotsOfStrokePoints)
                            {
                            double spacing = result->FastLength() * 0.10;
                            auto strokeOptions = IFacetOptions::CreateForCurves();
                            strokeOptions->SetMaxEdgeLength(spacing);
                            bvector<DPoint3d> locations;
                            result->AddStrokePoints(locations, *strokeOptions);
                            // output some scattered boxes . ..
                            for (auto &xyz : locations)
                                {
                                auto box = rotatedChop->Clone(Transform::From(xyz.x, xyz.y, 4 * zShift));
                                Check::SaveTransformed(box);
                                }
                            }
                        Check::Shift(0, 0, zShift);
                        Check::SaveTransformed(result);
                        Check::Shift(0, 0, -zShift);

                        Check::Shift(0, 2.0 * dy, 0);
                        Check::Shift(0, 0, zShift);
                        Check::SaveTransformed(result);
                        Check::Shift(0, 0, -zShift);
                        Check::SaveTransformed(shape);
                        for (int effort : {0, 10})
                            {
                            // and output points with various effort to get to the inside . . .
                            DPoint3d xyzA;
                            if (CurveVectorXYOffsetContext::ChooseBoxCenter(*shape, *result, effort, xyzA))
                                Check::SaveTransformed(*rotatedChop->Clone(Transform::From(xyzA.x, xyzA.y, 2 * zShift)));
                            }
                        }
                    }
                }
            }
        }
    Check::ClearGeometry("CurveVector.OffsetXYThinSection");
    }

