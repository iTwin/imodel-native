//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>

#include <GeoCoord/BaseGeoCoord.h>
#include <GeoCoord/GCSLibrary.h>
#include "GCSSpecificTransformTester.h"

using namespace ::testing;

 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt local_reproject
(
DPoint3d& outCartesian,
const DPoint3d&  inCartesian,
GeoCoordinates::BaseGCSCR SrcGcs,
GeoCoordinates::BaseGCSCR DstGcs
)
    {

    StatusInt   status = SUCCESS;
    StatusInt   stat1;
    StatusInt   stat2;
    StatusInt   stat3;


    GeoPoint inLatLong;
    stat1 = SrcGcs.LatLongFromCartesian (inLatLong, inCartesian);

    GeoPoint outLatLong;
    stat2 = SrcGcs.LatLongFromLatLong(outLatLong, inLatLong, DstGcs);

    stat3 = DstGcs.CartesianFromLatLong(outCartesian, outLatLong);

    if (SUCCESS == status)
        {
        // Status returns hardest error found in the three error statuses
        // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
        if (SUCCESS != stat1)
            status = stat1;
        if ((SUCCESS != stat2) && ((SUCCESS == status) || (cs_CNVRT_USFL == status))) // If stat2 has error and status not already hard error
            {
            if (0 > stat2) // If stat2 is negative ... this is the one ...
                status = stat2;
            else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
                status = (stat2 > status ? stat2 : status);
            }
        if ((SUCCESS != stat3) && ((SUCCESS == status) || (cs_CNVRT_USFL == status))) // If stat3 has error and status not already hard error
            {
            if (0 > stat3) // If stat3 is negative ... this is the one ...
                status = stat3;
            else  // Both are positive (status may be SUCCESS) we use the highest value
                status = (stat3 > status ? stat3 : status);
            }
        }
    
    return status;
    }

bool GCSSpecificTransformTester::s_initialized = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain Robert  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
GCSSpecificTransformTester::GCSSpecificTransformTester() 
    {
    if (!s_initialized)
        {
        BeTest::Host& host = BeTest::GetHost();

        BeFileName path;
        host.GetDgnPlatformAssetsDirectory(path);

        path.AppendToPath(L"DgnGeoCoord");

        GeoCoordinates::BaseGCS::Initialize(path.c_str());

        s_initialized = true;
        }
    }





static bvector<conversionTest> s_listOfConversionTests = 
    {

    {L"LL84", L"LL83", 34.0, 30.0, 0.0, 34.0, 30.0, 0.0, false},
    {L"LL84", L"LL83", 32.0, 29.0, 0.0, 32.0, 29.0, 0.0, false},

    // England Highway grids
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A1H1",  170370.71800000000000,  11572.40500000000000, 0.0,   32359.66731680,    182510.07861576, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A2H1",  170370.71800000000000,  11572.40500000000000, 0.0,   23364.00093735,    182519.93120423, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A3H1",  170370.71800000000000,  11572.40500000000000, 0.0,   14367.32017350,    182529.23740348, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A4H1",  170370.71800000000000,  11572.40500000000000, 0.0,    5369.71656122,    182538.36204954, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A5H1",  170370.71800000000000,  11572.40500000000000, 0.0,   -3628.78667738,    182547.48760794, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A6H1",  170370.71800000000000,  11572.40500000000000, 0.0,  -13628.11681271,    182556.79661752, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A7H1",  170370.71800000000000,  11572.40500000000000, 0.0,  -23628.39594992,    182565.55890564, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A8H1",  170370.71800000000000,  11572.40500000000000, 0.0,  -33629.66874161,    182574.50460909, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A9H1",  170370.71800000000000,  11572.40500000000000, 0.0,  -44631.98220814,    182583.45118925, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A10H1", 170370.71800000000000,  11572.40500000000000, 0.0,  -56635.59679896,    182592.76386721, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A11H1", 170370.71800000000000,  11572.40500000000000, 0.0,  -70640.97294592,    182602.62538527, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A12H1", 170370.71800000000000,  11572.40500000000000, 0.0,  -85647.73887040,    182611.75737034, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A13H1", 170370.71800000000000,  11572.40500000000000, 0.0, -101656.06805499,    182620.52493535, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A14H1", 170370.71800000000000,  11572.40500000000000, 0.0, -118666.36479400,    182629.47602640, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A15H1", 170370.71800000000000,  11572.40500000000000, 0.0, -141681.06580438,    182639.15880665, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A16H1", 170370.71800000000000,  11572.40500000000000, 0.0, -175701.40655328,    182647.38084117, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A17H1", 170370.71800000000000,  11572.40500000000000, 0.0, -253725.56968973,    182641.71669362, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A18H1", 170370.71800000000000,  11572.40500000000000, 0.0, -287723.22250283,    182632.03364215, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A19H1", 170370.71800000000000,  11572.40500000000000, 0.0, -317715.85853888,    182622.16894752, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A20H1", 170370.71800000000000,  11572.40500000000000, 0.0, -334704.42225583,    182613.40122465, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A21H1", 170370.71800000000000,  11572.40500000000000, 0.0, -351688.89257710,    182603.35591047, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A22H1", 170370.71800000000000,  11572.40500000000000, 0.0, -368668.54476683,    182591.85081757, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A23H1", 170370.71800000000000,  11572.40500000000000, 0.0, -385650.29973887,    182582.35564253, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A24H1", 170370.71800000000000,  11572.40500000000000, 0.0, -394633.82029387,    182574.50460909, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A25H1", 170370.71800000000000,  11572.40500000000000, 0.0, -405614.47732002,    182565.74146226, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A26H1", 170370.71800000000000,  11572.40500000000000, 0.0, -415594.16479698,    182556.97915661, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A27H1", 170370.71800000000000,  11572.40500000000000, 0.0, -425572.04334216,    182547.85264925, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A28H1", 170370.71800000000000,  11572.40500000000000, 0.0, -435548.48883330,    182538.54455177, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A29H1", 170370.71800000000000,  11572.40500000000000, 0.0, -444524.59783524,    182529.41988746, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A30H1", 170370.71800000000000,  11572.40500000000000, 0.0, -453498.90276608,    182519.93120423, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B15H1", 170370.71800000000000,  11572.40500000000000, 0.0, -141681.06580438,    222653.78397558, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B16H1", 170370.71800000000000,  11572.40500000000000, 0.0, -175701.40655328,    222663.80738518, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B17H1", 170370.71800000000000,  11572.40500000000000, 0.0, -253725.56968973,    222656.90227297, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B18H1", 170370.71800000000000,  11572.40500000000000, 0.0, -287723.22250283,    222645.09775053, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B19H1", 170370.71800000000000,  11572.40500000000000, 0.0, -317715.85853888,    222633.07178851, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B20H1", 170370.71800000000000,  11572.40500000000000, 0.0, -334704.42225583,    222622.38313507, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B21H1", 170370.71800000000000,  11572.40500000000000, 0.0, -351688.89257710,    222610.13698135, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B22H1", 170370.71800000000000,  11572.40500000000000, 0.0, -368668.91347040,    222596.33384156, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B23H1", 170370.71800000000000,  11572.40500000000000, 0.0, -385650.29973887,    222584.53574034, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B24H1", 170370.71800000000000,  11572.40500000000000, 0.0, -394634.21492769,    222575.18718984, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B25H1", 170370.71800000000000,  11572.40500000000000, 0.0, -405614.47732002,    222564.28154004, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B26H1", 170370.71800000000000,  11572.40500000000000, 0.0, -415594.16479698,    222553.59949069, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B27H1", 170370.71800000000000,  11572.40500000000000, 0.0, -425572.04334216,    222542.47344624, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B28H1", 170370.71800000000000,  11572.40500000000000, 0.0, -435548.48883330,    222531.12602689, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B29H1", 170370.71800000000000,  11572.40500000000000, 0.0, -444525.04225006,    222520.22469434, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B30H1", 170370.71800000000000,  11572.40500000000000, 0.0, -453499.35612942,    222508.65710137, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B31H1", 170370.71800000000000,  11572.40500000000000, 0.0, -462471.81238854,    222496.64587385, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B32H1", 170370.71800000000000,  11572.40500000000000, 0.0, -471442.82876558,    222484.41354911, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C13H1", 170370.71800000000000,  11572.40500000000000, 0.0, -101656.16973668,    237635.25877740, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C14H1", 170370.71800000000000,  11572.40500000000000, 0.0, -118666.36479400,    237646.66869279, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C15H1", 170370.71800000000000,  11572.40500000000000, 0.0, -141681.06580438,    237659.26841393, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C16H1", 170370.71800000000000,  11572.40500000000000, 0.0, -175701.40655328,    237669.96733919, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C17H1", 170370.71800000000000,  11572.40500000000000, 0.0, -253725.56968973,    237662.59686523, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C18H1", 170370.71800000000000,  11572.40500000000000, 0.0, -287723.22250283,    237649.99679118, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C19H1", 170370.71800000000000,  11572.40500000000000, 0.0, -317715.85853888,    237637.16035388, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A3H2",  170370.71800000000000,  11572.40500000000000, 0.0,   14367.67936997,    182533.80080554, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A5H2",  170370.71800000000000,  11572.40500000000000, 0.0,   -3628.87740045,    182552.05146627, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A6H2",  170370.71800000000000,  11572.40500000000000, 0.0,  -13628.45752840,    182561.36070859, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A7H2",  170370.71800000000000,  11572.40500000000000, 0.0,  -23628.98668197,    182570.12321578, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A8H2",  170370.71800000000000,  11572.40500000000000, 0.0,  -33630.50951486,    182579.06914287, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A9H2",  170370.71800000000000,  11572.40500000000000, 0.0,  -44633.09804954,    182588.01594671, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A10H2", 170370.71800000000000,  11572.40500000000000, 0.0,  -56637.01274197,    182597.32885750, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A11H2", 170370.71800000000000,  11572.40500000000000, 0.0,  -70642.73903647,    182607.19062211, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A12H2", 170370.71800000000000,  11572.40500000000000, 0.0,  -85649.88014417,    182616.32283548, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A13H2", 170370.71800000000000,  11572.40500000000000, 0.0, -101658.60955199,    182625.09061969, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A14H2", 170370.71800000000000,  11572.40500000000000, 0.0, -118669.33156437,    182634.04193452, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A15H2", 170370.71800000000000,  11572.40500000000000, 0.0, -141684.60796386,    182643.72495685, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A16H2", 170370.71800000000000,  11572.40500000000000, 0.0, -175705.79925317,    182651.94719692, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A17H2", 170370.71800000000000,  11572.40500000000000, 0.0, -253731.91306685,    182646.28290777, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A18H2", 170370.71800000000000,  11572.40500000000000, 0.0, -287730.41585315,    182636.59961422, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A19H2", 170370.71800000000000,  11572.40500000000000, 0.0, -317723.80173321,    182626.73467296, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A20H2", 170370.71800000000000,  11572.40500000000000, 0.0, -334712.79018018,    182617.96673089, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A21H2", 170370.71800000000000,  11572.40500000000000, 0.0, -351697.68512914,    182607.92116556, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A22H2", 170370.71800000000000,  11572.40500000000000, 0.0, -368677.76182609,    182596.41578502, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A23H2", 170370.71800000000000,  11572.40500000000000, 0.0, -385659.94135792,    182586.92037260, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A24H2", 170370.71800000000000,  11572.40500000000000, 0.0, -394643.68650936,    182579.06914287, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A25H2", 170370.71800000000000,  11572.40500000000000, 0.0, -405624.61806223,    182570.30577696, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A26H2", 170370.71800000000000,  11572.40500000000000, 0.0, -415604.55504074,    182561.54325224, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A27H2", 170370.71800000000000,  11572.40500000000000, 0.0, -425582.68304223,    182552.41651671, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A28H2", 170370.71800000000000,  11572.40500000000000, 0.0, -435559.37795387,    182543.10818652, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B15H2", 170370.71800000000000,  11572.40500000000000, 0.0, -141684.60796386,    222659.35052893, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B16H2", 170370.71800000000000,  11572.40500000000000, 0.0, -175705.79925317,    222669.37418912, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B17H2", 170370.71800000000000,  11572.40500000000000, 0.0, -253731.91306685,    222662.46890428, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B18H2", 170370.71800000000000,  11572.40500000000000, 0.0, -287730.41585315,    222650.66408672, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C14H2", 170370.71800000000000,  11572.40500000000000, 0.0, -118669.33156437,    237652.61008231, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C15H2", 170370.71800000000000,  11572.40500000000000, 0.0, -141684.60796386,    237665.21011846, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C16H2", 170370.71800000000000,  11572.40500000000000, 0.0, -175705.79925317,    237675.90931119, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C17H2", 170370.71800000000000,  11572.40500000000000, 0.0, -253731.91306685,    237668.53865297, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C18H2", 170370.71800000000000,  11572.40500000000000, 0.0, -287730.41585315,    237655.93826390, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C19H2", 170370.71800000000000,  11572.40500000000000, 0.0, -317723.80173321,    237643.10150568, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A11H3", 170370.71800000000000,  11572.40500000000000, 0.0,  -70644.50521534    ,182611.75608721, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-A12H3", 170370.71800000000000,  11572.40500000000000, 0.0,  -85652.02152500    ,182620.88852890, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B15H3", 170370.71800000000000,  11572.40500000000000, 0.0, -141688.15030046,    222664.91736062, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B16H3", 170370.71800000000000,  11572.40500000000000, 0.0, -175710.19217270,    222674.94127142, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C14H3", 170370.71800000000000,  11572.40500000000000, 0.0, -118672.29848310,    237658.55176892, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C15H3", 170370.71800000000000,  11572.40500000000000, 0.0, -141688.15030046,    237671.15212009, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C16H3", 170370.71800000000000,  11572.40500000000000, 0.0, -175710.19217270,    237681.85158032, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-B16H4", 170370.71800000000000,  11572.40500000000000, 0.0, -175714.58531191,    222680.50863209, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C14H4", 170370.71800000000000,  11572.40500000000000, 0.0, -118675.26555018,    237664.49375264, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C15H4", 170370.71800000000000,  11572.40500000000000, 0.0, -141691.69281418,    237677.09441884, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C16H4", 170370.71800000000000,  11572.40500000000000, 0.0, -175714.58531191,    237687.79414658, 0.0, true},
    {L"OSTN15.BritishNatGrid", L"EnglandHighway-C15H5", 170370.71800000000000,  11572.40500000000000, 0.0, -141695.23550506,    237683.03701474, 0.0, true},


    // Slovak JTSK03 to JTSK (Through NADCON file) values come from https://zbgis.skgeodesy.sk/rts/en/Transform
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 16.7 , 47.7 , 0.0, 16.700021658, 47.699998944, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 16.8 , 47.7 , 0.0, 16.800021936, 47.699999656, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 16.9 , 47.7 , 0.0, 16.900021331, 47.699999908, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17   , 47.7 , 0.0, 17.000021544, 47.700000447, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.1 , 47.7 , 0.0, 17.100021550, 47.700000953, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 18.4 , 47.7 , 0.0, 18.400008828, 47.700009753, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19.2 , 47.7 , 0.0, 19.199998958, 47.700007733, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19.8 , 47.7 , 0.0, 19.799994964, 47.700004461, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 21.1 , 47.7 , 0.0, 21.099988586, 47.700000358, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.5 , 47.7 , 0.0, 22.499986258, 47.699995439, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.7 , 47.8 , 0.0, 17.700016478, 47.800006444, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 18.2 , 47.8 , 0.0, 18.200010650, 47.800008136, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 20.4 , 47.8 , 0.0, 20.399993389, 47.800004281, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 21.8 , 47.8 , 0.0, 21.799985603, 47.799995947, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 16.6 , 47.9 , 0.0, 16.600019822, 47.899998253, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.8 , 47.9 , 0.0, 17.800013839, 47.900005492, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.6 , 47.9 , 0.0, 22.599985858, 47.899995119, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 16.7 , 48.0 , 0.0, 16.700020033, 47.999997858, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.9 , 48.0 , 0.0, 17.900012517, 48.000005128, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19   , 48.0 , 0.0, 18.999999597, 48.000006525, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 21.2 , 48.0 , 0.0, 21.199988614, 47.999998856, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.3 , 48.0 , 0.0, 22.299986022, 47.999995433, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 16.7 , 48.1 , 0.0, 16.700019319, 48.099996867, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.8 , 48.1 , 0.0, 17.800012136, 48.100003436, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19.9 , 48.1 , 0.0, 19.899995164, 48.100004514, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.3 , 48.1 , 0.0, 22.299986142, 48.099995464, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.1 , 48.2 , 0.0, 17.100018831, 48.199997717, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 18.9 , 48.2 , 0.0, 18.900002122, 48.200004475, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 20.7 , 48.2 , 0.0, 20.699989175, 48.200001614, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.3 , 48.2 , 0.0, 22.299986175, 48.199995347, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 16.9 , 48.3 , 0.0, 16.900018508, 48.299995300, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19.5 , 48.3 , 0.0, 19.499995469, 48.300004022, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 21.1 , 48.3 , 0.0, 21.099988842, 48.299998622, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.6 , 48.3 , 0.0, 22.599986769, 48.299994722, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.3 , 48.4 , 0.0, 17.300013517, 48.399997228, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19.6 , 48.4 , 0.0, 19.599995589, 48.400003853, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.5 , 48.4 , 0.0, 22.499987422, 48.399994983, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.2 , 48.5 , 0.0, 17.200012769, 48.499995942, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19.3 , 48.5 , 0.0, 19.299996317, 48.500002981, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 21.4 , 48.5 , 0.0, 21.399987475, 48.499997083, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.6 , 48.5 , 0.0, 22.599988189, 48.499995044, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.7 , 48.6 , 0.0, 17.700011092, 48.600000175, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19.9 , 48.6 , 0.0, 19.899992525, 48.600002325, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.0 , 48.6 , 0.0, 21.999988072, 48.599995619, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.6 , 48.7 , 0.0, 17.600010817, 48.699999311, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 20.1 , 48.7 , 0.0, 20.099993631, 48.700001303, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 21.9 , 48.7 , 0.0, 21.899988292, 48.699996150, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.6 , 48.8 , 0.0, 17.600009703, 48.799998008, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19.8 , 48.8 , 0.0, 19.799993722, 48.800000572, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.5 , 48.8 , 0.0, 22.499989792, 48.799994789, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 18   , 48.9 , 0.0, 18.000007661, 48.899999850, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 20.1 , 48.9 , 0.0, 20.099992475, 48.899999631, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.2 , 48.9 , 0.0, 22.199989475, 48.899995564, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.8 , 49.0 , 0.0, 17.800007619, 48.999999256, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 20.3 , 49.0 , 0.0, 20.299991278, 48.999998353, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.3 , 49.0 , 0.0, 22.299990144, 48.999995664, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 18.2 , 49.1 , 0.0, 18.200004019, 49.100001139, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 20.8 , 49.1 , 0.0, 20.799991178, 49.099996681, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.4 , 49.1 , 0.0, 22.399990553, 49.099995831, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 18.8 , 49.2 , 0.0, 18.800002011, 49.200001203, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 21.2 , 49.2 , 0.0, 21.199989189, 49.199996339, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 16.7 , 49.3 , 0.0, 16.700007628, 49.299994444, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19.6 , 49.3 , 0.0, 19.599998119, 49.299999550, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.3 , 49.3 , 0.0, 22.299990500, 49.299995931, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.7 , 49.4 , 0.0, 17.700005208, 49.399998628, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19.6 , 49.4 , 0.0, 19.599998211, 49.399999125, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.0 , 49.4 , 0.0, 21.999990839, 49.399995239, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 19   , 49.5 , 0.0, 19.000001153, 49.500000181, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 21.7 , 49.5 , 0.0, 21.699990811, 49.499995172, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 17.7 , 49.6 , 0.0, 17.700004078, 49.599998719, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 20.7 , 49.6 , 0.0, 20.699993378, 49.599995397, 0.0, false},
    {L"Slov/JTSK03.LL", L"Slov/JTSK.LL", 22.4 , 49.6 , 0.0, 22.399990831, 49.599995694, 0.0, false},

    { L"Slov/JTSK03.LL", L"Slov/JTSK03.Krovak", 16.7 , 47.7 , 0.0, -609254.984, -1326822.821, 0.0, true },
    { L"Slov/JTSK03.LL", L"Slov/JTSK03.Krovak", 19.2 , 47.7 , 0.0, -422379.264, -1343668.587, 0.0, true },
    { L"Slov/JTSK03.LL", L"Slov/JTSK03.Krovak", 22.6 , 47.9 , 0.0, -166928.118, -1334520.451, 0.0, true },
    { L"Slov/JTSK03.LL", L"Slov/JTSK03.Krovak", 17.8 , 48.1 , 0.0, -523014.011, -1290705.929, 0.0, true },
    { L"Slov/JTSK03.LL", L"Slov/JTSK03.Krovak", 22.6 , 48.3 , 0.0, -165634.692, -1290071.390, 0.0, true },
    { L"Slov/JTSK03.LL", L"Slov/JTSK03.Krovak", 20.1 , 48.7 , 0.0, -348130.272, -1237269.005, 0.0, true },
    { L"Slov/JTSK03.LL", L"Slov/JTSK03.Krovak", 22.3 , 49.0 , 0.0, -185308.631, -1211595.909, 0.0, true },
    { L"Slov/JTSK03.LL", L"Slov/JTSK03.Krovak", 16.7 , 49.3 , 0.0, -590360.321, -1149928.977, 0.0, true },
    { L"Slov/JTSK03.LL", L"Slov/JTSK03.Krovak", 21.7 , 49.5 , 0.0, -226905.949, -1154420.633, 0.0, true },
    { L"Slov/JTSK03.LL", L"Slov/JTSK03.Krovak", 20.7 , 49.6 , 0.0, -298658.354, -1139888.320, 0.0, true },

    { L"Slov/JTSK03.Krovak", L"ETRS89.UTM-33N", -609254.984, -1326822.821, 0.0,  627455.174, 5284306.364, 0.0, true },
    { L"Slov/JTSK03.Krovak", L"ETRS89.UTM-33N", -166928.118, -1334520.451, 0.0, 1067704.873, 5333170.354, 0.0, true },
    { L"Slov/JTSK03.Krovak", L"ETRS89.UTM-33N", -523014.011, -1290705.929, 0.0,  708353.772, 5331154.991, 0.0, true },
    { L"Slov/JTSK03.Krovak", L"ETRS89.UTM-33N", -165634.692, -1290071.390, 0.0, 1063292.358, 5377580.647, 0.0, true },
    { L"Slov/JTSK03.Krovak", L"ETRS89.UTM-33N", -348130.272, -1237269.005, 0.0,  875073.789, 5406613.662, 0.0, true },
    { L"Slov/JTSK03.Krovak", L"ETRS89.UTM-33N", -185308.631, -1211595.909, 0.0, 1033591.434, 5453129.382, 0.0, true },
    { L"Slov/JTSK03.Krovak", L"ETRS89.UTM-33N", -590360.321, -1149928.977, 0.0,  623495.043, 5462125.530, 0.0, true },
    { L"Slov/JTSK03.Krovak", L"ETRS89.UTM-33N", -226905.949, -1154420.633, 0.0,  984814.538, 5504586.861, 0.0, true },
    { L"Slov/JTSK03.Krovak", L"ETRS89.UTM-33N", -298658.354, -1139888.320, 0.0,  911639.270, 5509716.043, 0.0, true },


    };


/*---------------------------------------------------------------------------------**//**
* Parametrized conversion tests.
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(GCSSpecificTransformTester, SpecificCoordConversionTest)
{
    conversionTest theConversionTestParam = GetParam(); 

    GeoCoordinates::BaseGCSPtr sourceGCS = GeoCoordinates::BaseGCS::CreateGCS(theConversionTestParam.m_sourceGCS.c_str());
    GeoCoordinates::BaseGCSPtr targetGCS = GeoCoordinates::BaseGCS::CreateGCS(theConversionTestParam.m_targetGCS.c_str());

    ASSERT_TRUE(sourceGCS.IsValid() && sourceGCS->IsValid());
    ASSERT_TRUE(targetGCS.IsValid() && targetGCS->IsValid());

    DPoint3d resultPoint;
    DPoint3d inputPoint;

    inputPoint.x = theConversionTestParam.m_inputCoordinateX;
    inputPoint.y = theConversionTestParam.m_inputCoordinateY;
    inputPoint.z = theConversionTestParam.m_inputCoordinateZ;


    ASSERT_TRUE(SUCCESS == local_reproject(resultPoint, inputPoint, *sourceGCS, *targetGCS));

    if (theConversionTestParam.m_linearUnit)
        {
        EXPECT_NEAR(resultPoint.x, theConversionTestParam.m_outputCoordinateX, 0.002);
        EXPECT_NEAR(resultPoint.y, theConversionTestParam.m_outputCoordinateY, 0.002);
        EXPECT_NEAR(resultPoint.z, theConversionTestParam.m_outputCoordinateZ, 0.002);
        }
    else
        {
        EXPECT_NEAR(resultPoint.x, theConversionTestParam.m_outputCoordinateX, 0.00000002);
        EXPECT_NEAR(resultPoint.y, theConversionTestParam.m_outputCoordinateY, 0.00000002);
        EXPECT_NEAR(resultPoint.z, theConversionTestParam.m_outputCoordinateZ, 0.00000002);
        }
}

    
INSTANTIATE_TEST_CASE_P(GCSSpecificTransformTester_Combined,
                        GCSSpecificTransformTester,
                        ValuesIn(s_listOfConversionTests));


/*---------------------------------------------------------------------------------**//**
* Domain tests
* @bsimethod                                                    StephanePoulin  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificElevationTesgtIsrael)
    {
    GeoCoordinates::BaseGCSPtr firstGCS;
    GeoCoordinates::BaseGCSPtr secondGCS;

   
    firstGCS = GeoCoordinates::BaseGCS::CreateGCS(L"UTM84-36N");
    secondGCS = GeoCoordinates::BaseGCS::CreateGCS();


    WString wellKnownText = L"PROJCS[\"Israel 1993 / Israeli T\",GEOGCS[\"EPSG:4141\",DATUM[\"EPSG:6141\",SPHEROID[\"EPSG:7019\",6378137.000,298.25722210],TOWGS84[-48.0000,55.0000,52.0000,0.000000,0.000000,0.000000,0.00000000]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"false_easting\",219529.584],PARAMETER[\"false_northing\",626907.390],PARAMETER[\"scale_factor\",1.000006700000],PARAMETER[\"central_meridian\",35.20451694444446],PARAMETER[\"latitude_of_origin\",31.73439361111112],UNIT[\"Meter\",1.00000000000000]]";
    secondGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str());


    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());
    ASSERT_TRUE(secondGCS.IsValid() && secondGCS->IsValid());

    ASSERT_TRUE(firstGCS->GetVerticalDatumCode() == GeoCoordinates::vdcFromDatum);
    ASSERT_TRUE(secondGCS->GetVerticalDatumCode() == GeoCoordinates::vdcFromDatum);

    DPoint3d resultPoint;
    DPoint3d secondResultPoint;
    DPoint3d inputPoint;

    inputPoint.x = 800000;
    inputPoint.y = 3000000;
    inputPoint.z = 10.24;

    firstGCS->SetReprojectElevation(true);
    secondGCS->SetReprojectElevation(true);

    ASSERT_TRUE(SUCCESS == local_reproject(resultPoint, inputPoint, *firstGCS, *secondGCS));

    EXPECT_NEAR(resultPoint.x, 300862.38969781203, 0.001);
    EXPECT_NEAR(resultPoint.y, 112309.01716421195, 0.001);
    EXPECT_NEAR(resultPoint.z, -7.6771248336881399, 0.001);

    ASSERT_TRUE(SUCCESS == local_reproject(secondResultPoint, resultPoint, *secondGCS, *firstGCS));

    EXPECT_NEAR(secondResultPoint.x, 800000, 0.001);
    EXPECT_NEAR(secondResultPoint.y, 3000000, 0.001);
    EXPECT_NEAR(secondResultPoint.z, 10.24, 0.001);
    }


/*---------------------------------------------------------------------------------**//**
* Domain tests
* @bsimethod                                                    StephanePoulin  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, LatLongToFromXYZ)
    {
    GeoCoordinates::BaseGCSPtr currentGCSEllipsoid;
    GeoCoordinates::BaseGCSPtr currentGCSGeoid;

   
    // First test using WGS84 lat/long
    currentGCSEllipsoid = GeoCoordinates::BaseGCS::CreateGCS(L"LL84");

    GeoPoint point1;
    point1.longitude = -71;
    point1.latitude = 48;
    point1.elevation = 0.0;

    DPoint3d xyz = {0.0, 0.0, 0.0};

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, point1));
   
    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->LatLongFromXYZ(point1, xyz));

    EXPECT_NEAR(point1.longitude, -71, 0.00001);
    EXPECT_NEAR(point1.latitude, 48, 0.00001);
    EXPECT_NEAR(point1.elevation, 0.0, 0.001);

    point1.longitude = -71;
    point1.latitude = 48;
    point1.elevation = 0.0;

    // Same test but this time we use a geoid based GCS
    currentGCSGeoid =  GeoCoordinates::BaseGCS::CreateGCS(L"LL84");
    currentGCSGeoid->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);

    DPoint3d xyz2 = {0.0, 0.0, 0.0};
    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, point1));
   
    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->LatLongFromXYZ(point1, xyz2));

    // Round trip will yield the same result.
    EXPECT_NEAR(point1.longitude, -71, 0.00001);
    EXPECT_NEAR(point1.latitude, 48, 0.00001);
    EXPECT_NEAR(point1.elevation, 0.0, 0.001);

    // All we know is that XYZ values are different
    EXPECT_TRUE(xyz2.x != xyz.x);
    EXPECT_TRUE(xyz2.y != xyz.y);
    EXPECT_TRUE(xyz2.z != xyz.z);

    // Now we will make computation at precise locations on Earth

    // Equator on Greenwhich
    GeoPoint pointEquatorGreenwich;
    pointEquatorGreenwich.longitude = 0;
    pointEquatorGreenwich.latitude = 0;
    pointEquatorGreenwich.elevation = 0.0;

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, pointEquatorGreenwich));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, pointEquatorGreenwich));

    EXPECT_NEAR(xyz.x - xyz2.x, -17.1630, 0.01); // The X value should represent the geoid separation
    EXPECT_NEAR(xyz.y, xyz2.y, 0.001);
    EXPECT_NEAR(xyz.z, xyz2.z, 0.001);

    GeoPoint pointEquatorMinus90;
    pointEquatorMinus90.longitude = -90;
    pointEquatorMinus90.latitude = 0;
    pointEquatorMinus90.elevation = 0.0;

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, pointEquatorMinus90));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, pointEquatorMinus90));

    EXPECT_NEAR(xyz.x, xyz2.x, 0.001); 
    EXPECT_NEAR(xyz.y - xyz2.y, -4.2873, 0.01); // The Y value should represent the geoid separation
    EXPECT_NEAR(xyz.z, xyz2.z, 0.001);

    GeoPoint pointEquatorPlus90;
    pointEquatorPlus90.longitude = 90;
    pointEquatorPlus90.latitude = 0;
    pointEquatorPlus90.elevation = 0.0;

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, pointEquatorPlus90));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, pointEquatorPlus90));

    EXPECT_NEAR(xyz.x, xyz2.x, 0.001); 
    EXPECT_NEAR(xyz.y - xyz2.y, 63.2371, 0.01); // The Y value should represent the geoid separation
    EXPECT_NEAR(xyz.z, xyz2.z, 0.001);

    GeoPoint pointNorthPole;
    pointNorthPole.longitude = 0.0;
    pointNorthPole.latitude = 90;
    pointNorthPole.elevation = 0.0;

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, pointNorthPole));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, pointNorthPole));

    EXPECT_NEAR(xyz.x, xyz2.x, 0.001); 
    EXPECT_NEAR(xyz.y, xyz2.y, 0.001); 
    EXPECT_NEAR(xyz.z - xyz2.z, -13.605, 0.001); // The Z value should represent the geoid separation

    GeoPoint pointSouthPole;
    pointSouthPole.longitude = 0.0;
    pointSouthPole.latitude = -90;
    pointSouthPole.elevation = 0.0;

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, pointSouthPole));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, pointSouthPole));

    EXPECT_NEAR(xyz.x, xyz2.x, 0.001); 
    EXPECT_NEAR(xyz.y, xyz2.y, 0.001); 
    EXPECT_NEAR(xyz.z - xyz2.z, -29.5350, 0.001); // The Z value should represent the geoid separation
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, KuwaitUtilityInstanciationFailureTest)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"KuwaitUtility.KTM");

    EXPECT_TRUE(currentGCS.IsValid());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, InitFromEPSGFailureTest)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromEPSGCode(NULL, NULL, 4326));


    EXPECT_TRUE(currentGCS.IsValid());
    }
    
/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT1)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"COMPD_CS[\"LKS92 / Latvia TM + EGM96 geoid height\",PROJCS[\"LKS92 / Latvia TM\",GEOGCS[\"LKS92\",DATUM[\"Latvia_1992\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6661\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4661\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",24],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",-6000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"3059\"]],VERT_CS[\"EGM96 geoid height\",VERT_DATUM[\"EGM96 geoid\",2005,AUTHORITY[\"EPSG\",\"5171\"],EXTENSION[\"PROJ4_GRIDS\",\"egm96_15.gtx\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Up\",UP],AUTHORITY[\"EPSG\",\"5773\"]]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    EXPECT_TRUE(currentGCS->GetVerticalDatumCode() == GeoCoordinates::vdcGeoid);
    }

/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT2)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"PROJCS[\"WGS 84 / UTM zone 13N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-105],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32613\"]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT3)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"PROJCS[\"WGS 84 / UTM zone 13N\",GEOGCS[\"EPSG:6326\",DATUM[\"EPSG:6326\",SPHEROID[\"EPSG:7030\",6378137.000,298.25722360]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse Mercator\"],PARAMETER[\"False Easting\",500000.000],PARAMETER[\"False Northing\",0.000],PARAMETER[\"Scale Reduction\",0.999600000000],PARAMETER[\"Central Meridian\",-105.00000000000003],PARAMETER[\"Origin Latitude\",0.00000000000000],UNIT[\"Meter\",1.00000000000000]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT4)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"PROJCS[\"UTM84-13N\",GEOGCS[\"LL84\",DATUM[\"WGS84\",SPHEROID[\"WGS84\",6378137.000,298.25722293]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Universal Transverse Mercator System\"],PARAMETER[\"UTM Zone Number (1 - 60)\",13.0],PARAMETER[\"Hemisphere, North or South\",1.0],UNIT[\"Meter\",1.00000000000000]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT5)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"COMPD_CS[\"WGS84/UTM17N + EGM96 geoid height\",PROJCS[\"WGS 84 / UTM zone 17N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-81],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32617\"]],VERT_CS[\"EGM96 geoid height\",VERT_DATUM[\"EGM96 geoid\",2005]]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }


/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT6)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"PROJCS[\"NAD83(2011) / Florida East (ftUS)\",GEOGCS[\"NAD83(2011)\",DATUM[\"NAD_1983_2011\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"1116\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"6318\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",24.33333333333333],PARAMETER[\"central_meridian\",-81],PARAMETER[\"scale_factor\",0.999941177],PARAMETER[\"false_easting\",656166.667],PARAMETER[\"false_northing\",0],UNIT[\"US survey foot\",0.3048006096012192,AUTHORITY[\"EPSG\",\"9003\"]],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH],AUTHORITY[\"EPSG\",\"6438\"]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT7)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    // This WKT originates from a 3MX file that did not work. As expected the WKT parsing worked so the problem was elsewhere but we kept this test as regression.
    WString wellKnownText = L"PROJCS[\"WGS 84 / UTM zone 18N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-75],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32618\"]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT8)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    // This WKT originates from a client, apparently obtained through FME (which should use CSMAP?!)
    WString wellKnownText = L"PROJCS[\"WGS_1984_Web_Mercator_Auxiliary_Sphere\", GEOGCS[\"GCS_WGS_1984\", DATUM[\"D_WGS_1984\", SPHEROID[\"WGS_1984\",6378137.0,298.257223563]], PRIMEM[\"Greenwich\",0.0], UNIT[\"Degree\",0.0174532925199433]], PROJECTION[\"Mercator_Auxiliary_Sphere\"], PARAMETER[\"False_Easting\",0.0], PARAMETER[\"False_Northing\",0.0], PARAMETER[\"Central_Meridian\",0.0], PARAMETER[\"Standard_Parallel_1\",0.0],PARAMETER[\"Auxiliary_Sphere_Type\",0.0], UNIT[\"Meter\",1.0]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT9)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    // This WKT originates from a client for a 3MX
    WString wellKnownText = L"PROJCS[\"NAD83 / Pennsylvania South (ftUS)\",GEOGCS[\"NAD83\",DATUM[\"North_American_Datum_1983\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6269\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4269\"]],PROJECTION[\"Lambert_Conformal_Conic_2SP\"],PARAMETER[\"standard_parallel_1\",40.96666666666667],PARAMETER[\"standard_parallel_2\",39.93333333333333],PARAMETER[\"latitude_of_origin\",39.33333333333334],PARAMETER[\"central_meridian\",-77.75],PARAMETER[\"false_easting\",1968500],PARAMETER[\"false_northing\",0],UNIT[\"US survey foot\",0.3048006096012192,AUTHORITY[\"EPSG\",\"9003\"]],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH],AUTHORITY[\"EPSG\",\"2272\"]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }


/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT10)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    // This WKT originates from a client for a 3MX
    WString wellKnownText = L"PROJCS[\"Quebec MTM Zone 10 (NAD 27)\",GEOGCS[\"NAD 27 (Canada)\",DATUM[\"NAD 27 (Canada)\",SPHEROID[\"Clarke 1866\",6378206.4,294.9786982]],PRIMEM[\"Greenwich\",0],UNIT[\"Decimal Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"Scale_Factor\",0.9999],PARAMETER[\"Central_Meridian\",-79.500000],PARAMETER[\"False_Easting\",304800.00000],UNIT[\"Meter\",1.000000]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKT11)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    // This WKT originates from a client for a 3MX
    WString wellKnownText = L"PROJCS[\"Czech/JTSK.Krovak\",GEOGCS[\"Czech/JTSK.LL\",DATUM[\"Czech/JTSK\",SPHEROID[\"BESSEL\",6377397.155,299.15281283],TOWGS84[570.6934,85.6936,462.8393,-4.998250,-1.586630,-5.261140,3.54301550]],PRIMEM[\"Ferro\", -17.6666666666667],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Krovak Oblique Conformal Conic\"],PARAMETER[\"Central Meridian\",-17.66666666666667],PARAMETER[\"Origin Latitude\",49.50000000000000],PARAMETER[\"Oblique Pole Longitude\",42.50000000000000],PARAMETER[\"Oblique Pole Latitude\",59.75759855555555],PARAMETER[\"Oblique Cone Standard Parallel\",78.50000000000000],PARAMETER[\"False Easting\",0.000],PARAMETER[\"False Northing\",0.000],PARAMETER[\"Scale Reduction\",0.999900000000],UNIT[\"Meter\",1.00000000000000]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }


/*---------------------------------------------------------------------------------**//**
* Specific WKT test (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, ElevationTransfo1)
    {
    GeoCoordinates::BaseGCSPtr fromGCS;
    GeoCoordinates::BaseGCSPtr toGCS;

   
    fromGCS = GeoCoordinates::BaseGCS::CreateGCS();
    toGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText1 = L"PROJCS[\"WGS 84 / UTM zone 32N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",9],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32632\"]]";

    WString wellKnownText2 = L"COMPD_CS[\"WGS 84 / UTM zone 32N + EGM96 geoid height\",PROJCS[\"WGS 84 / UTM zone32N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",9],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32632\"]],VERT_CS[\"EGM96 geoid height\",VERT_DATUM[\"EGM96 geoid\",2005,EXTENSION[\"PROJ4_GRIDS\",\"egm96_15.gtx\"],AUTHORITY[\"EPSG\",\"5171\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Up\",UP],AUTHORITY[\"EPSG\",\"5773\"]]]";

    EXPECT_TRUE(SUCCESS == fromGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText1.c_str()));


    EXPECT_TRUE(fromGCS->IsValid());

    EXPECT_TRUE(SUCCESS == toGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText2.c_str()));

    EXPECT_TRUE(toGCS->IsValid());

    fromGCS->SetReprojectElevation(true);
    toGCS->SetReprojectElevation(true);

    GeoPoint fromGeoPoint;
    GeoPoint toGeoPoint;
   

    fromGeoPoint.longitude=6.6922634698675436;
    fromGeoPoint.latitude=43.955776401074210;
    fromGeoPoint.elevation=611.48800000000006;


    EXPECT_TRUE(REPROJECT_Success == fromGCS->LatLongFromLatLong(toGeoPoint, fromGeoPoint, *toGCS));

    EXPECT_TRUE(toGeoPoint.elevation != fromGeoPoint.elevation);


    }

/*---------------------------------------------------------------------------------**//**
* Specific LondonGrid WKT generation
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, SpecificWKTGenerationLondonGrid)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"LondonGrid");

    WString wellKnownText = L"";

    EXPECT_TRUE(SUCCESS == currentGCS->GetWellKnownText(wellKnownText, GeoCoordinates::BaseGCS::wktFlavorOGC, false));

    EXPECT_TRUE(currentGCS->IsValid());

    }


/*---------------------------------------------------------------------------------**//**
* Indiana DOT Coordinate Tests (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, IndianaDOT_InGCSTests)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

  
    GeoPoint2d testGeoPoint;
    testGeoPoint.latitude = 42.0;
    testGeoPoint.longitude = -85.0;
    
    DPoint2d resultPoint;
    
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Adams");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 235857.321, 0.001);
    EXPECT_NEAR(resultPoint.y, 197042.576, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Allen");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.667, 0.001);
    EXPECT_NEAR(resultPoint.y, 158173.879, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Bartholomew");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 310425.254, 0.001);
    EXPECT_NEAR(resultPoint.y, 369491.117, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Benton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 430567.721, 0.001);
    EXPECT_NEAR(resultPoint.y, 210705.421, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Blackford");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 273141.593, 0.001);
    EXPECT_NEAR(resultPoint.y, 252641.732, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Boone");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364282.128, 0.001);
    EXPECT_NEAR(resultPoint.y, 303618.467, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Brown");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 347710.206, 0.001);
    EXPECT_NEAR(resultPoint.y, 369960.596, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Carroll");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 376709.323, 0.001);
    EXPECT_NEAR(resultPoint.y, 215014.436, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Cass");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 355995.543, 0.001);
    EXPECT_NEAR(resultPoint.y, 197988.761, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Clark");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 289711.598, 0.001);
    EXPECT_NEAR(resultPoint.y, 463672.324, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Clay");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 418137.860, 0.001);
    EXPECT_NEAR(resultPoint.y, 354724.927, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Clinton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 372567.295, 0.001);
    EXPECT_NEAR(resultPoint.y, 242697.736, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Crawford");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364280.761 , 0.001);
    EXPECT_NEAR(resultPoint.y, 470138.638, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Daviess");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 413993.876, 0.001);
    EXPECT_NEAR(resultPoint.y, 432329.722, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Dearborn");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 231714.683, 0.001);
    EXPECT_NEAR(resultPoint.y, 408002.771, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Decatur");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 293855.057, 0.001);
    EXPECT_NEAR(resultPoint.y, 358247.257, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-DeKalb");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 235857.313, 0.001);
    EXPECT_NEAR(resultPoint.y, 119303.715, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Delaware");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 273141.593, 0.001);
    EXPECT_NEAR(resultPoint.y, 252641.732, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Dubois");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 401565.536, 0.001);
    EXPECT_NEAR(resultPoint.y, 459787.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Elkhart");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 310425.747, 0.001);
    EXPECT_NEAR(resultPoint.y, 186285.784, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Fayette");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.696 , 0.001);
    EXPECT_NEAR(resultPoint.y, 341391.242, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Floyd");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 289711.598, 0.001);
    EXPECT_NEAR(resultPoint.y, 463672.324, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Fountain");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 430566.959, 0.001);
    EXPECT_NEAR(resultPoint.y, 266225.351, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Franklin");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.696, 0.001);
    EXPECT_NEAR(resultPoint.y, 341391.242, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Fulton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 347710.313, 0.001);
    EXPECT_NEAR(resultPoint.y, 158990.378, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Gibson");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 459565.693, 0.001);
    EXPECT_NEAR(resultPoint.y, 466893.589, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Grant");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 297997.659, 0.001);
    EXPECT_NEAR(resultPoint.y, 219487.851, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Greene");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 413993.876, 0.001);
    EXPECT_NEAR(resultPoint.y, 432329.722, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Hamilton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 322854.027, 0.001);
    EXPECT_NEAR(resultPoint.y, 269702.978, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Hancock");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 306283.225, 0.001);
    EXPECT_NEAR(resultPoint.y, 297287.836, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Harrison");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335281.630, 0.001);
    EXPECT_NEAR(resultPoint.y, 486340.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Hendricks");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364282.128 , 0.001);
    EXPECT_NEAR(resultPoint.y, 303618.467, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Henry");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 277284.487, 0.001);
    EXPECT_NEAR(resultPoint.y, 285974.624, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Howard");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335282.011, 0.001);
    EXPECT_NEAR(resultPoint.y, 219890.110, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Huntington");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 281426.845, 0.001);
    EXPECT_NEAR(resultPoint.y, 186057.323, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Jackson");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 318710.339 , 0.001);
    EXPECT_NEAR(resultPoint.y, 402881.394, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Jasper");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 413995.442, 0.001);
    EXPECT_NEAR(resultPoint.y, 182516.908, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Jay");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 240000.000, 0.001);
    EXPECT_NEAR(resultPoint.y, 224803.768, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Jefferson");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 268998.598, 0.001);
    EXPECT_NEAR(resultPoint.y, 419157.924, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Jennings");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 306282.496, 0.001);
    EXPECT_NEAR(resultPoint.y, 391654.136, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Johnson");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335282.011, 0.001);
    EXPECT_NEAR(resultPoint.y, 336476.565, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Knox");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 442993.988 , 0.001);
    EXPECT_NEAR(resultPoint.y, 438649.761, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Kosciusko");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 310425.747, 0.001);
    EXPECT_NEAR(resultPoint.y, 186285.784, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-LaGrange");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 277284.263 , 0.001);
    EXPECT_NEAR(resultPoint.y, 119400.561, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Lake");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 438853.181, 0.001);
    EXPECT_NEAR(resultPoint.y, 183170.285, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-LaPorte");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 384995.147, 0.001);
    EXPECT_NEAR(resultPoint.y, 159654.089, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Lawrence");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364280.761 , 0.001);
    EXPECT_NEAR(resultPoint.y, 470138.638, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Madison");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 306283.225 , 0.001);
    EXPECT_NEAR(resultPoint.y, 297287.836, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Marion");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335282.011, 0.001);
    EXPECT_NEAR(resultPoint.y, 336476.565, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Marshall");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 347710.313, 0.001);
    EXPECT_NEAR(resultPoint.y, 158990.378, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Martin");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 401565.536 , 0.001);
    EXPECT_NEAR(resultPoint.y, 459787.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Miami");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335282.011, 0.001);
    EXPECT_NEAR(resultPoint.y, 219890.110, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Monroe");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364281.134, 0.001);
    EXPECT_NEAR(resultPoint.y, 375781.826, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Montgomery");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 401567.313, 0.001);
    EXPECT_NEAR(resultPoint.y, 321022.846, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Morgan");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364281.134, 0.001);
    EXPECT_NEAR(resultPoint.y, 375781.826, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Newton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 438853.181, 0.001);
    EXPECT_NEAR(resultPoint.y, 183170.285, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Noble");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 277284.263, 0.001);
    EXPECT_NEAR(resultPoint.y, 119400.561, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Ohio");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 231714.683, 0.001);
    EXPECT_NEAR(resultPoint.y, 408002.771, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Orange");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364280.761, 0.001);
    EXPECT_NEAR(resultPoint.y, 470138.638, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Owen");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 397423.611, 0.001);
    EXPECT_NEAR(resultPoint.y, 354235.478, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Parke");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 434709.379, 0.001);
    EXPECT_NEAR(resultPoint.y, 305198.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Perry");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 380851.312, 0.001);
    EXPECT_NEAR(resultPoint.y, 503745.518, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Pike");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 430565.053, 0.001);
    EXPECT_NEAR(resultPoint.y, 499355.108, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Porter");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 413995.442, 0.001);
    EXPECT_NEAR(resultPoint.y, 182516.908, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Posey");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 484424.394, 0.001);
    EXPECT_NEAR(resultPoint.y, 512105.909, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Pulaski");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 384995.147, 0.001);
    EXPECT_NEAR(resultPoint.y, 159654.089, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Putnam");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 401567.313, 0.001);
    EXPECT_NEAR(resultPoint.y, 321022.846, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Randolph");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.720, 0.001);
    EXPECT_NEAR(resultPoint.y, 291429.823, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Ripley");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 264856.185, 0.001);
    EXPECT_NEAR(resultPoint.y, 380290.971, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Rush");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 293855.057, 0.001);
    EXPECT_NEAR(resultPoint.y, 358247.257, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-St-Joseph");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 347710.313 , 0.001);
    EXPECT_NEAR(resultPoint.y, 158990.378, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Scott");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 289711.598, 0.001);
    EXPECT_NEAR(resultPoint.y, 463672.324, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Shelby");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 314568.249, 0.001);
    EXPECT_NEAR(resultPoint.y, 336228.284, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Spencer");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 409850.295, 0.001);
    EXPECT_NEAR(resultPoint.y, 509927.647, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Starke");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 384995.147, 0.001);
    EXPECT_NEAR(resultPoint.y, 159654.089, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Steuben");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 240000.000, 0.001);
    EXPECT_NEAR(resultPoint.y, 91536.493, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Sullivan");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 447137.413, 0.001);
    EXPECT_NEAR(resultPoint.y, 383265.043, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Switzerland");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 231714.683, 0.001);
    EXPECT_NEAR(resultPoint.y, 408002.771, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Tippecanoe");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 397423.611, 0.001);
    EXPECT_NEAR(resultPoint.y, 237652.628, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Tipton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 322854.027 , 0.001);
    EXPECT_NEAR(resultPoint.y, 269702.978, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Union");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.696, 0.001);
    EXPECT_NEAR(resultPoint.y, 341391.242, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Vanderburgh");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 451280.026, 0.001);
    EXPECT_NEAR(resultPoint.y, 505491.860, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Vermillion");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 434709.379, 0.001);
    EXPECT_NEAR(resultPoint.y, 305198.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Vigo");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 442995.003, 0.001);
    EXPECT_NEAR(resultPoint.y, 344289.561, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Wabash");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 310425.747, 0.001);
    EXPECT_NEAR(resultPoint.y, 186285.784, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Warren");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 430566.959, 0.001);
    EXPECT_NEAR(resultPoint.y, 266225.351, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Warrick");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 430565.053, 0.001);
    EXPECT_NEAR(resultPoint.y, 499355.108, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Washington");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335281.630, 0.001);
    EXPECT_NEAR(resultPoint.y, 486340.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Wayne");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.720, 0.001);
    EXPECT_NEAR(resultPoint.y, 291429.823, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Wells");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 260713.402, 0.001);
    EXPECT_NEAR(resultPoint.y, 197071.604, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-White");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 397423.611, 0.001);
    EXPECT_NEAR(resultPoint.y, 237652.628, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Whitley");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 281426.845, 0.001);
    EXPECT_NEAR(resultPoint.y, 186057.323, 0.001);

    }
   

/*---------------------------------------------------------------------------------**//**
* Specific Virginia DOT Tests (to be removed)
* @bsimethod                                                    Alain.Robert  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTester, VirginiaDOT_Tests)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

    // TEST COUNTY IN THE SOUTH ZONE
    DPoint3d testPointUSFoot;
    // Longitude      Latitude
    // -76.632285     36.980671
    testPointUSFoot.x = 12028435.08574;
    testPointUSFoot.y = 3521917.71242;
    testPointUSFoot.z = 0.0;
    
    DPoint3d resultPoint;
    resultPoint.z = 0.0;
    
    GeoCoordinates::BaseGCSPtr VirginiaSouthFoot = GeoCoordinates::BaseGCS::CreateGCS(L"VA83/2011-SF");

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTAccomack-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826504.8065, 0.001);
    EXPECT_NEAR(resultPoint.y, 241094.0225, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTAlbermarle-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826428.2795, 0.001);
    EXPECT_NEAR(resultPoint.y, 241089.2008, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTAlleghany-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826925.7052, 0.001);
    EXPECT_NEAR(resultPoint.y, 241120.5417, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTAmelia-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826619.5970, 0.001);
    EXPECT_NEAR(resultPoint.y, 241101.2550, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTAmherst-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826696.1241, 0.001);
    EXPECT_NEAR(resultPoint.y, 241106.0767, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTAppomattox-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826657.8606, 0.001);
    EXPECT_NEAR(resultPoint.y, 241103.6658, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTBedford-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826696.1241, 0.001);
    EXPECT_NEAR(resultPoint.y, 241106.0767, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTBland-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3827002.2322, 0.001);
    EXPECT_NEAR(resultPoint.y, 241125.3634, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTBotetourt-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826772.6511, 0.001);
    EXPECT_NEAR(resultPoint.y, 241110.8984, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTBrunswick-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826390.0159, 0.001);
    EXPECT_NEAR(resultPoint.y, 241086.7899, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTBuchanan-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826925.7052, 0.001);
    EXPECT_NEAR(resultPoint.y, 241120.5417, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTBuckingham-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826619.5970, 0.001);
    EXPECT_NEAR(resultPoint.y, 241101.2550, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTCampbell-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826619.5970, 0.001);
    EXPECT_NEAR(resultPoint.y, 241101.2550, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTCarroll-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826772.6511, 0.001);
    EXPECT_NEAR(resultPoint.y, 241110.8984, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTCharlesCity-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826581.3335, 0.001);
    EXPECT_NEAR(resultPoint.y, 241098.8442, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTCharlotte-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826581.3335, 0.001);
    EXPECT_NEAR(resultPoint.y, 241098.8442, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTChesterfield-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826581.3335, 0.001);
    EXPECT_NEAR(resultPoint.y, 241098.8442, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTCraig-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3827002.2322, 0.001);
    EXPECT_NEAR(resultPoint.y, 241125.3634, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTCumberland-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826619.5970, 0.001);
    EXPECT_NEAR(resultPoint.y, 241101.2550, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTDickenson-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826887.4417, 0.001);
    EXPECT_NEAR(resultPoint.y, 241118.1309, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTDinwiddie-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826543.0700, 0.001);
    EXPECT_NEAR(resultPoint.y, 241096.4333, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTCityOfHampton-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826504.8065, 0.001);
    EXPECT_NEAR(resultPoint.y, 241094.0225, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTEssex-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826390.0159, 0.001);
    EXPECT_NEAR(resultPoint.y, 241086.7899, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTFloyd-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826810.9146, 0.001);
    EXPECT_NEAR(resultPoint.y, 241113.3092, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTFluvanna-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826504.8065, 0.001);
    EXPECT_NEAR(resultPoint.y, 241094.0225, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTFranklin-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826696.1241, 0.001);
    EXPECT_NEAR(resultPoint.y, 241106.0767, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTGiles-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3827002.2322, 0.001);
    EXPECT_NEAR(resultPoint.y, 241125.3634, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTGloucester-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826543.0700, 0.001);
    EXPECT_NEAR(resultPoint.y, 241096.4333, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTGoochland-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826543.0700, 0.001);
    EXPECT_NEAR(resultPoint.y, 241096.4333, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTGrayson-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826772.6511, 0.001);
    EXPECT_NEAR(resultPoint.y, 241110.8984, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTGreensville-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826351.7524, 0.001);
    EXPECT_NEAR(resultPoint.y, 241084.3791, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTHalifax-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826428.2795, 0.001);
    EXPECT_NEAR(resultPoint.y, 241089.2008, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTHanover-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826504.8065, 0.001);
    EXPECT_NEAR(resultPoint.y, 241094.0225, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTHenrico-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826581.3335, 0.001);
    EXPECT_NEAR(resultPoint.y, 241098.8442, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTHenry-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826466.5430, 0.001);
    EXPECT_NEAR(resultPoint.y, 241091.6116, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTIsleOfWight-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826428.2795, 0.001);
    EXPECT_NEAR(resultPoint.y, 241089.2008, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTJamesCity-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826543.0700, 0.001);
    EXPECT_NEAR(resultPoint.y, 241096.4333, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTKingAndQueen-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826504.8065, 0.001);
    EXPECT_NEAR(resultPoint.y, 241094.0225, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTKingWilliam-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826504.8065, 0.001);
    EXPECT_NEAR(resultPoint.y, 241094.0225, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTLancaster-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826466.5430, 0.001);
    EXPECT_NEAR(resultPoint.y, 241091.6116, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTLee-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826581.3335, 0.001);
    EXPECT_NEAR(resultPoint.y, 241098.8442, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTLouisa-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826428.2795, 0.001);
    EXPECT_NEAR(resultPoint.y, 241089.2008, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTLunenburg-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826543.0700, 0.001);
    EXPECT_NEAR(resultPoint.y, 241096.4333, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTMathews-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826543.0700, 0.001);
    EXPECT_NEAR(resultPoint.y, 241096.4333, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTMecklenburg-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826351.7524, 0.001);
    EXPECT_NEAR(resultPoint.y, 241084.3791, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTMiddlesex-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826543.0700, 0.001);
    EXPECT_NEAR(resultPoint.y, 241096.4333, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTMontgomery-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826925.7052, 0.001);
    EXPECT_NEAR(resultPoint.y, 241120.5417, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTCityOfHR-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826351.7524, 0.001);
    EXPECT_NEAR(resultPoint.y, 241084.3791, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTNelson-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826619.5970, 0.001);
    EXPECT_NEAR(resultPoint.y, 241101.2550, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTNewKent-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826543.0700, 0.001);
    EXPECT_NEAR(resultPoint.y, 241096.4333, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTNorChesPort-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826351.7524, 0.001);
    EXPECT_NEAR(resultPoint.y, 241084.3791, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTNorthampton-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826504.8065, 0.001);
    EXPECT_NEAR(resultPoint.y, 241094.0225, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTNorthumberland-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826428.2795, 0.001);
    EXPECT_NEAR(resultPoint.y, 241089.2008, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTNottoway-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826619.5970, 0.001);
    EXPECT_NEAR(resultPoint.y, 241101.2550, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTPatrick-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826504.8065, 0.001);
    EXPECT_NEAR(resultPoint.y, 241094.0225, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTPittsylvania-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826428.2795, 0.001);
    EXPECT_NEAR(resultPoint.y, 241089.2008, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTPowhatan-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826581.3335, 0.001);
    EXPECT_NEAR(resultPoint.y, 241098.8442, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTPrinceEdward-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826619.5970, 0.001);
    EXPECT_NEAR(resultPoint.y, 241101.2550, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTPrinceGeorge-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826581.3335, 0.001);
    EXPECT_NEAR(resultPoint.y, 241098.8442, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTVirginiaBeach-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826351.7524, 0.001);
    EXPECT_NEAR(resultPoint.y, 241084.3791, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTPulaski-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826887.4417, 0.001);
    EXPECT_NEAR(resultPoint.y, 241118.1309, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTRichmond-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826390.0159, 0.001);
    EXPECT_NEAR(resultPoint.y, 241086.7899, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTRoanoke-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826849.1781, 0.001);
    EXPECT_NEAR(resultPoint.y, 241115.7201, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTRockbridge-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826657.8606, 0.001);
    EXPECT_NEAR(resultPoint.y, 241103.6658, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTRussell-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826810.9146, 0.001);
    EXPECT_NEAR(resultPoint.y, 241113.3092, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTScott-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826772.6511, 0.001);
    EXPECT_NEAR(resultPoint.y, 241110.8984, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTSmyth-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826925.7052, 0.001);
    EXPECT_NEAR(resultPoint.y, 241120.5417, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTSouthampton-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826351.7524, 0.001);
    EXPECT_NEAR(resultPoint.y, 241084.3791, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTSurry-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826543.0700, 0.001);
    EXPECT_NEAR(resultPoint.y, 241096.4333, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTSussex-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826466.5430, 0.001);
    EXPECT_NEAR(resultPoint.y, 241091.6116, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTTazewell-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3827078.7592, 0.001);
    EXPECT_NEAR(resultPoint.y, 241130.1851, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTNewportNews-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826504.8065, 0.001);
    EXPECT_NEAR(resultPoint.y, 241094.0225, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTWashington-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826428.2795, 0.001);
    EXPECT_NEAR(resultPoint.y, 241089.2008, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTWise-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826925.7052, 0.001);
    EXPECT_NEAR(resultPoint.y, 241120.5417, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTWythe-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826887.4417, 0.001);
    EXPECT_NEAR(resultPoint.y, 241118.1309, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTYork-SF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaSouthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3826543.0700, 0.001);
    EXPECT_NEAR(resultPoint.y, 241096.4333, 0.001);


    // TEST COUNTY IN THE NORTH ZONE
    // Longitude      Latitude
    // -76.632285     36.980671
    testPointUSFoot.x = 11475744.52;
    testPointUSFoot.y = 6875090.213;
    testPointUSFoot.z = 0.0;
    
   
    GeoCoordinates::BaseGCSPtr VirginiaNorthFoot = GeoCoordinates::BaseGCS::CreateGCS(L"VA83/2011-NF");

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTArlington-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273857.6022, 0.005);
    EXPECT_NEAR(resultPoint.y, 313442.3515, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTAugusta-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273955.8120, 0.005);
    EXPECT_NEAR(resultPoint.y, 313451.7542, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTBath-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3274054.0218, 0.005);
    EXPECT_NEAR(resultPoint.y, 313461.1569, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTCaroline-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273693.9191, 0.005);
    EXPECT_NEAR(resultPoint.y, 313426.6804, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTClarke-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273792.1289, 0.005);
    EXPECT_NEAR(resultPoint.y, 313436.0831, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTCulpeper-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273890.3388, 0.005);
    EXPECT_NEAR(resultPoint.y, 313445.4858, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTFairfax-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273857.6022, 0.005);
    EXPECT_NEAR(resultPoint.y, 313442.3515, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTFauquier-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273923.0754, 0.005);
    EXPECT_NEAR(resultPoint.y, 313448.6200, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTFrederick-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273792.1289, 0.005);
    EXPECT_NEAR(resultPoint.y, 313436.0831, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTGreene-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273890.3388, 0.005);
    EXPECT_NEAR(resultPoint.y, 313445.4858, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTHighland-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3274184.9683, 0.005);
    EXPECT_NEAR(resultPoint.y, 313473.6939, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTKingGeorge-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273792.1289, 0.005);
    EXPECT_NEAR(resultPoint.y, 313436.0831, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTLoudoun-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273824.8656, 0.005);
    EXPECT_NEAR(resultPoint.y, 313439.2173, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTMadison-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273890.3388, 0.005);
    EXPECT_NEAR(resultPoint.y, 313445.4858, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTOrange-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273857.6022, 0.005);
    EXPECT_NEAR(resultPoint.y, 313442.3515, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTPage-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273988.5486, 0.005);
    EXPECT_NEAR(resultPoint.y, 313454.8885, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTPrinceWilliam-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273857.6022, 0.005);
    EXPECT_NEAR(resultPoint.y, 313442.3515, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTRappahannock-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273955.8120, 0.005);
    EXPECT_NEAR(resultPoint.y, 313451.7542, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTRockhingham-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3274021.2852, 0.005);
    EXPECT_NEAR(resultPoint.y, 313458.0227, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTShenandoah-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273955.8120, 0.005);
    EXPECT_NEAR(resultPoint.y, 313451.7542, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTSpotsylvania-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273792.1289, 0.005);
    EXPECT_NEAR(resultPoint.y, 313436.0831, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTStafford-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273857.6022, 0.005);
    EXPECT_NEAR(resultPoint.y, 313442.3515, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTWarren-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3273890.3388, 0.005);
    EXPECT_NEAR(resultPoint.y, 313445.4858, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"VDOTWestmoreland-NF");
    EXPECT_TRUE(SUCCESS == local_reproject(resultPoint, testPointUSFoot, *VirginiaNorthFoot, *currentGCS));
    EXPECT_NEAR(resultPoint.x, 3274152.2317, 0.005);
    EXPECT_NEAR(resultPoint.y, 313470.5597, 0.001);

    }
   


