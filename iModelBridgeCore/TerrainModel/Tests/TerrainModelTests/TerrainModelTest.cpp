/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TerrainModelTests/TerrainModelTest.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma warning(disable:4505) // unreferenced local function has been removed [in gtest-port.h]

#include <Bentley/Bentley.h>
#include <gtest/gtest.h>

extern "C"
    {
    /*--------------------------------------------------------------------------------**//**
    * @bsimethod                                                    KevinNyman      03/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    EXPORT_ATTRIBUTE int Run (int argc, char **argv, void*, char const*)
        {
        ::testing::InitGoogleTest (&argc, argv);

        printf ("__START_TESTS__\n");
        int status = RUN_ALL_TESTS ();
        printf ("__END_TESTS__\n");

        return status;
        }

    EXPORT_ATTRIBUTE int InitializeHost (int argc, char* argv[], char const* outFolder)
        {
        printf ("__InitializeHost__");
        return 0;
        }


    EXPORT_ATTRIBUTE void* GetHost ()
        {
        return nullptr;
        }

    EXPORT_ATTRIBUTE int ShutdownHost ()
        {
        return 0;
        }

    //BOOL APIENTRY DllMain (HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
    //    {
    //    return TRUE;
    //    }


    }

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <Bentley/WString.h>
#include <TerrainModel/Drainage/Drainage.h>
#include <TerrainModel/Core\bcdtmInlines.h>
#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Formats/TerrainImporter.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

BcDTMPtr LoadTerrainModel (WCharCP filename, WCharCP name = nullptr)
    {
    TerrainImporterPtr reader = TerrainImporter::CreateImporter (filename);
    WString surfaceName;

    if (name == nullptr)
        {
        TerrainInfoList surfaceInfos = reader->GetTerrains ();

        if (surfaceInfos.size () == 0)
            return nullptr;

        surfaceName = surfaceInfos[0].GetName ();
        }
    else
        surfaceName = name;

    ImportedTerrain dtm = reader->ImportTerrain (surfaceName.GetWCharCP());
    return dtm.GetTerrain ();
    }

TEST (TmTest, Triangulate)
    {
    Bentley::TerrainModel::BcDTMPtr dtm = Bentley::TerrainModel::BcDTM::Create ();

    DPoint3d pts[4];

    pts[0].x = 100; pts[0].y = 100; pts[0].z = 0;
    pts[1].x = 200; pts[1].y = 100; pts[1].z = 0;
    pts[2].x = 200; pts[2].y = 200; pts[2].z = 0;
    pts[3].x = 100; pts[3].y = 200; pts[3].z = 0;
    dtm->AddPoints (pts, 4);
    ASSERT_TRUE (dtm->Triangulate () == SUCCESS);
    SUCCEED ();
    }
int numDtmFeatures = 0;
int bcdtmDrainage_callBackFunction (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId featureId, DPoint3d *featurePtsP, size_t numFeaturePts, void *userP)
    {
    /*
    ** Sample DTM Interrupt Load Function
    **
    ** This Function Receives The Load Features From The DTM
    ** As The DTM Reuses The Feature Points Memory Do Not Free It
    ** If You Require The Feature Points Then Make A Copy
    **
    */
    int  ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    char  dtmFeatureTypeName[100];
    BC_DTM_OBJ *dtmP = NULL;
    DPoint3d  *p3dP;
    /*
    ** Check For DTMFeatureType::CheckStop
    */
    // if(dtmFeatureType == DTMFeatureType::CheckStop) bcdtmWrite_message(0,0,0,"DTMFeatureType::CheckStop") ;
    /*
    ** Initialise
    */
    // if(numDtmFeatures % 1000 == 0) bcdtmWrite_message(0,0,0,"numDtmFeatures = %8ld",numDtmFeatures) ;
    ++numDtmFeatures;
    /*
    ** Write Record
    */
    if (dbg == 1)
        {
        bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType (dtmFeatureType, dtmFeatureTypeName);
        bcdtmWrite_message (0, 0, 0, "Feature[%8ld] ** %s userTag = %10I64d featureId = %10I64d featurePtsP = %p numFeaturePts = %6ld userP = %p", numDtmFeatures, dtmFeatureTypeName, userTag, featureId, featurePtsP, numFeaturePts, userP);
        }
    /*
    ** Write Points
    */
    if (dbg == 1)
        {
        bcdtmWrite_message (0, 0, 0, "Number Of Feature Points = %6ld", numFeaturePts);
        for (p3dP = featurePtsP; p3dP < featurePtsP + numFeaturePts; ++p3dP)
            {
            bcdtmWrite_message (0, 0, 0, "Point[%6ld] = %12.4lf %12.4lf %10.4lf", (long)(p3dP - featurePtsP), p3dP->x, p3dP->y, p3dP->z);
            }
        }
    /*
    ** Store DTM Features In DTM
    */
    //    if( userP != NULL && ( dtmFeatureType == DTMFeatureType::LowPointPond || dtmFeatureType == DTMFeatureType::DescentTrace ))
    if (userP != NULL && dtmFeatureType != DTMFeatureType::CheckStop)
        {
        dtmP = (BC_DTM_OBJ *)userP;
        if (bcdtmObject_testForValidDtmObject (dtmP)) goto errexit;

        if (dtmFeatureType == DTMFeatureType::SumpLine || dtmFeatureType == DTMFeatureType::RidgeLine)
            {
            DPoint3d *lineP = featurePtsP;
            for (lineP = featurePtsP; lineP < featurePtsP + numFeaturePts * 2; lineP = lineP + 2)
                {
                if (bcdtmObject_storeDtmFeatureInDtmObject (dtmP, DTMFeatureType::Breakline, dtmP->nullUserTag, 1, &dtmP->nullFeatureId, lineP, 2)) goto errexit;
                }
            }
        else
            {
            if (bcdtmObject_storeDtmFeatureInDtmObject (dtmP, DTMFeatureType::Breakline, dtmP->nullUserTag, 1, &dtmP->nullFeatureId, featurePtsP, (long)numFeaturePts)) goto errexit;
            }
        /*
        if(userTag == 99)
        {
        if(bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::ContourLine,dtmP->nullUserTag,1,&dtmP->nullFeatureId,featurePtsP,(long)numFeaturePts)) goto errexit ;
        }
        if(userTag == 2)
        {
        if(bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,featurePtsP,(long)numFeaturePts)) goto errexit ;
        }
        */
        }
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Call Back Function Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Call Back Function Error");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

void DoPondTest (BcDTMR iDtm)
    {
//    int ret = DTM_SUCCESS;
    int dbg = DTM_TRACE_VALUE (1);
    long startTime = bcdtmClock ();
    BC_DTM_OBJ *dtmP = NULL;
//    BC_DTM_OBJ* depressionDtmP = NULL;
//    BC_DTM_OBJ *traceDtmP = NULL;
//    void  *userP = NULL;
//    Bentley::TerrainModel::DTMFenceParams  fence;
//    DPoint3d  fencePts[10];
//    DPoint3d  traceStartPoint, sumpPoint;
//    double    maxPondDepth;
//    BcDTMPtr refinedDtm = nullptr;
//    bvector<DPoint3d> catchmentPoints;

    // Variables To Test Maximum Descent And Ascent Tracing

//    int pnt1, pnt2, pnt3, clPtr, voidTriangle, numTrianglesTraced = 0;
//    double x, y, falseLowDepth = 0.0;

    //  Drainage Tests

    DTMDrainageTables *drainageTablesP = NULL;
//    DTMFeatureCallback callBackFunction = (DTMFeatureCallback)bcdtmDrainage_callBackFunction;

    if (bcdtmObject_createDtmObject (&dtmP)) goto errexit;
    //           BcDTMDrainage::CreateDrainageTables(ibcDtm.get(), drainageTablesP) ;
    bcdtmWrite_message (0, 0, 0, "Determing Ponds");
    if (BcDTMDrainage::DeterminePonds (&iDtm, drainageTablesP, (DTMFeatureCallback)bcdtmDrainage_callBackFunction, dtmP) == DTM_SUCCESS)
        {
        bcdtmWrite_message (0, 0, 0, "Determing Ponds Completed");
        bcdtmWrite_message (0, 0, 0, "Number Of Ponds = %8ld", dtmP->numFeatures);
        bcdtmWrite_message (0, 0, 0, "Time To Determine Ponds = %8.3lf Seconds", bcdtmClock_elapsedTime (bcdtmClock (), startTime));
        bcdtmWrite_geopakDatFileFromDtmObject (dtmP, L"ponds.dat");

        //              Analyse Ponds - Have To Set It Up And Test For The Aborted Way It Is Done In CF

        bool analysePonds = true;
        if (analysePonds)
            {
            long dtmFeature, numFeaturePts;
            double area;
            DTMDirection direction;
            DPoint3d *p3dP, *featurePtsP = NULL;
            BC_DTM_FEATURE *dtmFeatureP;
            BC_DTM_OBJ *tempDtmP = NULL;
            if (bcdtmObject_createDtmObject (&tempDtmP) != DTM_SUCCESS) goto errexit;
            for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
                {
                dtmFeatureP = ftableAddrP (dtmP, dtmFeature);
                if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline)
                    {
                    if (bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject (dtmP, dtmFeature, &featurePtsP, &numFeaturePts) != DTM_SUCCESS) goto errexit;
                    bcdtmMath_getPolygonDirectionP3D (featurePtsP, numFeaturePts, &direction, &area);
                    bcdtmWrite_message (0, 0, 0, "Processing Pond Feature %8ld ** numPondPoints = %8ld ** Direction = %2ld Area = %15.8lf", dtmFeature, numFeaturePts, direction, area);
                    if (dtmFeature == -99)
                        {
                        for (p3dP = featurePtsP; p3dP < featurePtsP + numFeaturePts; ++p3dP)
                            {
                            bcdtmWrite_message (0, 0, 0, "FeaturePoint[%4ld] = %12.5lf %12.5lf %10.4lf", (long)(p3dP - featurePtsP), p3dP->x, p3dP->y, p3dP->z);
                            }
                        }
                    if (numFeaturePts >= 4)
                        {
                        tempDtmP->ppTol = tempDtmP->plTol = 0.0;
                        if (bcdtmObject_storeDtmFeatureInDtmObject (tempDtmP, DTMFeatureType::Breakline, tempDtmP->nullUserTag, 1, &tempDtmP->nullFeatureId, featurePtsP, numFeaturePts) != DTM_SUCCESS) goto errexit;
                        if (bcdtmObject_triangulateDtmObject (tempDtmP) != DTM_SUCCESS) goto errexit;
                        if (bcdtmList_removeNoneFeatureHullLinesDtmObject (tempDtmP) != DTM_SUCCESS) goto errexit;

                        //  Get Point For Pond Determination

                        int ap, np, sp = tempDtmP->hullPoint;
                        double x, y, z;
                        bool pointFound = false;
                        do
                            {
                            np = nodeAddrP (tempDtmP, sp)->hPtr;
                            if ((ap = bcdtmList_nextAntDtmObject (tempDtmP, sp, np)) < 0) goto errexit;
                            if (nodeAddrP (tempDtmP, ap)->hPtr != sp)
                                {
                                pointFound = true;
                                x = (pointAddrP (tempDtmP, sp)->x + pointAddrP (tempDtmP, ap)->x) / 2.0;
                                y = (pointAddrP (tempDtmP, sp)->y + pointAddrP (tempDtmP, ap)->y) / 2.0;
                                z = (pointAddrP (tempDtmP, sp)->z + pointAddrP (tempDtmP, ap)->z) / 2.0;
                                }
                            sp = np;
                            } while (pointFound == false && sp != tempDtmP->hullPoint);

                        if (!pointFound && tempDtmP->numPoints == 3)
                            {
                            pointFound = true;
                            x = (pointAddrP (tempDtmP, 0)->x + pointAddrP (tempDtmP, 1)->x + pointAddrP (tempDtmP, 2)->x) / 3.0;
                            y = (pointAddrP (tempDtmP, 0)->y + pointAddrP (tempDtmP, 1)->y + pointAddrP (tempDtmP, 2)->y) / 3.0;
                            z = (pointAddrP (tempDtmP, 0)->z + pointAddrP (tempDtmP, 1)->z + pointAddrP (tempDtmP, 2)->z) / 3.0;
                            }

                        // Determine Pond

                        if (pointFound)
                            {
                            bool pondDetermined = false;
                            double pondElevation, pondDepth, pondVolume, pondArea;
                            Bentley::TerrainModel::DTMDynamicFeatureArray pondFeatures;
                            if (BcDTMDrainage::CalculatePondForPoint (&iDtm, x, y, 0.0, pondDetermined, pondElevation, pondDepth, pondArea, pondVolume, pondFeatures) == DTM_SUCCESS)
                                if (dbg && pondDetermined)
                                    {
                                    bcdtmWrite_message (0, 0, 0, "Pond Determined ** elevation = %8.3lf depth = %8.3lf volume = %12.3lf area = %15.8lf", pondElevation, pondDepth, pondVolume, pondArea);
                                    }
                            }
                        }


                    }
                bcdtmObject_initialiseDtmObject (tempDtmP);
                }
            }
        }
    else
        {
        bcdtmWrite_message (0, 0, 0, "Determing Ponds Error");
        }
errexit:

    // Clean Up
    return;
    }

TEST (TmTest, LoadDTM)
    {
    Bentley::TerrainModel::BcDTMPtr dtm = LoadTerrainModel (L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.Light.NUnit.dll\\groupSpot.tin");
    ASSERT_TRUE (dtm->Triangulate () == SUCCESS);
    DoPondTest (*dtm);
    SUCCEED ();
    }



TEST (TmTest, DeterminePonds)
    {
    Bentley::TerrainModel::BcDTMPtr dtm = Bentley::TerrainModel::BcDTM::CreateFromTinFile (L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\\DTMDrainageCatchmentTests\\mine.dtm");
    ASSERT_TRUE (dtm.IsValid ());
    ASSERT_TRUE (dtm->Triangulate () == SUCCESS);

    DoPondTest (*dtm);
    SUCCEED ();
    }

TEST (TmTest, CreateFromXyz)
    {
    Bentley::TerrainModel::BcDTMPtr dtm = LoadTerrainModel (L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.NUnit.dll\\DTMTriangulationTest\\DTMTriangulationXyzTest\\1M.xyz");
    ASSERT_TRUE (dtm->Triangulate () == SUCCESS);
    DoPondTest (*dtm);
    SUCCEED ();
    }
