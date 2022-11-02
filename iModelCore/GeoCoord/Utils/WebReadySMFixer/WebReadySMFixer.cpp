//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------

#include <Bentley\BeFileName.h>
#include <GeoCoord/BaseGeoCoord.h>
#include <Bentley/Desktop/FileSystem.h>


USING_NAMESPACE_BENTLEY


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PrintUsage ()
    {
    printf ("Usage WebReadySMFixer <filename> [-fix | -change] [-verticaldatum verticalDatum\n\n");
    printf ("Opens the designated root file name to a Web Ready 3D Tiles dataset then\n");
    printf ("based on the GCS and bounding box definitions determines if the transform is correct\n");
    printf ("or should be fixed. It will also verify that the Geographic Coordinate System is present.\n\n");
    printf ("Either -fix or -change can be specified.\n\n The fix will set the GCS to explicitely specify\n" );
    printf ("the vertical datum. If the property -verticaldatum is specified then this vertical datum\n");
    printf ("will be used, otherwise the default vertical datum is injected. The default will be: \n");
    printf (" NAVD88 if the horizontal datum is a variation of NAD83 (Not specific canadian variations)\n");
    printf (" NGVD29 if the horizontal datum is a variation of NAD27 (Not specific canadian variations)\n");
    printf (" ELLIPSOID otherwise\n");
    printf (" GEOID can also be indicated as specified vertical datum\n");
    printf ("The fix function will also fix the transformation if it is inconsistent with the GCS.\n\n");
    printf ("If the -change option is indicated then the vertical datum will be modified.\n");
    printf ("Coordinate values will not be modified but the transform will be updated to\n");
    printf ("the new position of the model relative to Earth.\n\n");
    printf ("Indicating neither -fix not -change will perform an analysis only of the file.\n");
    
    exit (-1);
    }
//*---------------------------------------------------------------------------------**//**
/* @bsimethod
/+---------------+---------------+---------------+---------------+---------------+------*/
int wmain(int argc, wchar_t* argv[])
{
    if (argc < 2 || argc > 5)
    {
        PrintUsage();
        return 1;
    }

    Utf8String fileName(argv[1]);

    bool fix = false;
    bool change = false;

    bool nextArgIsVertDatum = false;
    bool verticalDatumSpecified = false;
    GeoCoordinates::VertDatumCode vertDatum = GeoCoordinates::vdcEllipsoid;

    for (int iArg = 1; iArg < argc; iArg++)
    {
        wchar_t     copy[1024];
        wcscpy_s(copy, _countof(copy), argv[iArg]);
        _wcsupr_s(copy, _countof(copy));

        Utf8String     strCopy(copy);

        if (nextArgIsVertDatum)
        {
            verticalDatumSpecified = true;

            if (strCopy == "LOCAL_ELLIPSOID")
                vertDatum = GeoCoordinates::vdcLocalEllipsoid;
            if (strCopy == "ELLIPSOID")
                vertDatum = GeoCoordinates::vdcEllipsoid;
            else if (strCopy == "GEOID")
                vertDatum = GeoCoordinates::vdcGeoid;
            else if (strCopy == "NAVD88")
                vertDatum = GeoCoordinates::vdcNAVD88;
            else if (strCopy == "NGVD29")
                vertDatum = GeoCoordinates::vdcNGVD29;
            else
            {
                PrintUsage();
                return 1;
            }
            nextArgIsVertDatum = false;    
        }

        if (strCopy == "-FIX")
            fix = true;
        else if (strCopy == "-VERTICALDATUM")
            nextArgIsVertDatum = true;
        else if (strCopy == "-CHANGE")
            change = true;
    };


    BeFileName moduleFileName = Desktop::FileSystem::GetExecutableDir();

    BeFileName path(BeFileName::GetDirectoryName(moduleFileName).c_str());

    path.AppendToPath(L"Assets\\DgnGeoCoord");

    GeoCoordinates::BaseGCS::Initialize(path.c_str());



    if (fix && change)
    {
        PrintUsage();
        return 1;
    }


    // Now we have everything to do the job...
    bool isAScalableMesh = false;
    bool GCSPresent = false;
    bool GCSComplete = false;
    bool GCSValid = false;
    GeoCoordinates::BaseGCSPtr theGCS;

    bool transformPresent = false;
    bool transformJSONInvalid = false;
    Transform storedTransform;
    bool transformInconsistentWithVerticalDatum = false;
    bool transformInconsistent = false;
    Utf8String SoftwareNameAndVersion;
    Utf8String theWKT;
    
    // Opening of file and JSON parsing
    char* content = new char[10000000];
    FILE* rootFile = fopen(fileName.c_str(), "r");
    
    if (nullptr == rootFile)
    {
        printf("Could not open file:%s\n", fileName.c_str());
        return 1;
    }

    fread (content, 1, 10000000, rootFile);

    fclose(rootFile);

    Utf8String jsonString(content);
    
    delete [] content;

    Json::Value resultJson;
    if (!Json::Reader::Parse(jsonString, resultJson))
    {
        printf("JSON parsing error for file:%s \n", fileName.c_str());
        return 1;
    }
    
    // Extraction of information
    Json::Value root = resultJson["root"];
    if (root.isNull())
    {
        printf("root node not found in file:%s \n", fileName.c_str());
        return 1;
    }

    Json::Value SMMasterHeader = root["SMMasterHeader"];
    if (SMMasterHeader.isNull())
    {
        printf("SMMasterHeader node not found. DATA is probably a Point Cloud.\n");
        isAScalableMesh = false;
    }
    else
    {
        isAScalableMesh = true;
        printf("SMMasterHeader node found. DATA appears to be  Scalable Mesh.\n");

        Json::Value GCSWKT = SMMasterHeader["GCS"];
        if (!GCSWKT.isNull())
        {
            theWKT = Utf8String(GCSWKT.asString());

            GCSPresent = true;
            printf("A GCS node is present in the SMMasterHeader\n");

            printf("The GCS Well Known Text (WKT) is: %s\n", theWKT.c_str());

            theGCS = GeoCoordinates::BaseGCS::CreateGCS();
            if (SUCCESS != theGCS->InitFromWellKnownText(nullptr, nullptr, GeoCoordinates::BaseGCS::wktFlavorUnknown, theWKT.c_str()))
            {
                printf("The GCS Well Known Text (WKT) could not be parsed into a valid GCS (WKT:%s)\n", theWKT.c_str());
                theGCS = nullptr; // Clear GCS
            }
            else
            {
                GCSValid = true;
                if (theWKT.substr(0, 4) == "COMP")
                    GCSComplete = true;
            }
        }
    }

    Json::Value boundingVolume = root["boundingVolume"];
    if (boundingVolume.isNull())
    {
        printf("boundingVolume node not found in root node of file:%s \n", fileName.c_str());
        return 1;
    }

    Json::Value box = boundingVolume["box"];
    if (box.isNull())
    {
        printf("box node not found in boundingVolume node of root node of file:%s \n", fileName.c_str());
        return 1;
    }

    double boxValues[12];

    if (box.size() != 12)
    {
        printf("box node does not contain 12 values in root/boundingVolume of file:%s \n", fileName.c_str());
        return 1;
    }

    for (unsigned int index = 0; index < box.size() && !box[index].isNull() ; index++)
    {
        boxValues[index] = box[index].asDouble();
    }  

    double transformValues[16];

    Json::Value transform = root["transform"];
    if (transform.isNull())
    {
        printf("No transform present in file:%s \n", fileName.c_str());
    }
    else
    {
        if (transform.size() != 16)
        {
            printf("transform node does not contain 12 values in root of file:%s \n", fileName.c_str());
            transformJSONInvalid = true;
        }
        else
        {

            transformPresent = true;

            for (unsigned int index = 0; index < transform.size(); index++)
            {
                transformValues[index] = transform[index].asDouble();
            }  
        }
    }

    // Analysis
    double maxDistance = 0.0;
    GeoCoordinates::VertDatumCode matchVerticalDatum = GeoCoordinates::vdcEllipsoid;
    bool matchVerticalDatumFound = false;
    double matchDistance = 9999.0;
    GeoCoordinates::BaseGCSPtr copyGCS;
    DRange3d extent;
    DPoint3d center;

    if (transformPresent)
    {
        // Transform box values into DRange3d
        // First three values are the center then the fourth is half xdim, 8th value is half dimy, 12th is half zdim
        // I did not use the code from SMStreamingStore since they immediately cook up with transform which is not what we want.
        center = DPoint3d::From(boxValues[0], boxValues[1], boxValues[2]);
        double xDimDiv2 = boxValues[3];
        double yDimDiv2 = boxValues[7];
        double zDimDiv2 = boxValues[11];
    
        extent = DRange3d::From(center.x - xDimDiv2, center.y - yDimDiv2, center.z - zDimDiv2, 
                                        center.x + xDimDiv2, center.y + yDimDiv2, center.z + zDimDiv2);
    
        // Create the transform
        storedTransform = Transform::FromRowValues(transformValues[0], transformValues[4], transformValues[8], transformValues[12],
                                                   transformValues[1], transformValues[5], transformValues[9], transformValues[13],
                                                   transformValues[2], transformValues[6], transformValues[10], transformValues[14]);

        // Compute the transformed center (stored transform)
        DPoint3d ecefCenter;
        storedTransform.Multiply(ecefCenter, center);

        printf ("ECEF center of data is: %lf, %lf, %lf \n", ecefCenter.x, ecefCenter.y, ecefCenter.z);

        GeoCoordinates::BaseGCSPtr ll84GCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");
        GeoCoordinates::BaseGCSPtr ll84GeoidGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");
        ll84GeoidGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);

        DPoint3d latLongHeight;
        DPoint3d latLongHeightGeoid;

        GeoCoordinates::BaseGCS::CartesianFromECEF(latLongHeight, ecefCenter, *ll84GCS);
        GeoCoordinates::BaseGCS::CartesianFromECEF(latLongHeightGeoid, ecefCenter, *ll84GeoidGCS);

        printf ("WGS84 center of data is: Longitude: %lf, Latitude: %lf, Height: %lf (Ellipsoid Elevation)\n", latLongHeight.x, latLongHeight.y, latLongHeight.z);
        printf ("WGS84 center of data is: Longitude: %lf, Latitude: %lf, Height: %lf (Geoid Elevation)\n", latLongHeightGeoid.x, latLongHeightGeoid.y, latLongHeightGeoid.z);
        printf ("Geoid separation is: %lf\n", latLongHeight.z - latLongHeightGeoid.z);
    }

    if (!GCSPresent && !transformPresent)
    {
        printf ("Dataset appears to be an older version fully ECEF with no transform\n");
        printf ("Nothing can be validated.\n");
    }

    if (GCSPresent && GCSValid && transformPresent && !transformJSONInvalid)
    {
        // Compute the transformed center (stored transform)
        DPoint3d transformedCenter;
        storedTransform.Multiply(transformedCenter, center);
    
    
        // Check consistency
        // Code temporarily copied from Sarah's code but adapted to ECEF target coordinates

        StatusInt res;
        DPoint3d ptTransformedWithElevation;
        Transform transformWithElevation;

        // First calculate distance to current vertical setting
        res = theGCS->GetLinearTransformToECEF(&transformWithElevation, extent, nullptr, nullptr);
        transformWithElevation.Multiply(ptTransformedWithElevation, center);
        double theDistanceWithCurrentGCS = ptTransformedWithElevation.Distance(transformedCenter);

        printf("Current Positioning Distance Error is: %lf\n", theDistanceWithCurrentGCS);

        std::vector<GeoCoordinates::VertDatumCode> verticalDatums{ GeoCoordinates::vdcFromDatum, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcEllipsoid };
        if (theGCS->IsNAD27() || theGCS->IsNAD83())
        {
            verticalDatums.push_back(GeoCoordinates::vdcNGVD29);
            verticalDatums.push_back(GeoCoordinates::vdcNAVD88);
        }

        for (auto verticalDatum : verticalDatums)
        {   
            copyGCS = GeoCoordinates::BaseGCS::CreateGCS(*theGCS);
            copyGCS->SetVerticalDatumCode(verticalDatum);
            res = copyGCS->GetLinearTransformToECEF(&transformWithElevation, extent, nullptr, nullptr);
            if ((REPROJECT_Success == res || REPROJECT_CSMAPERR_OutOfUsefulRange == res || REPROJECT_CSMAPERR_VerticalDatumConversionError == res))
            {
                transformWithElevation.Multiply(ptTransformedWithElevation, center);
                double theDistance = ptTransformedWithElevation.Distance(transformedCenter);
                if (theDistance > maxDistance)
                    maxDistance = theDistance;
    
                if (theDistance <= 0.01 && !matchVerticalDatumFound)
                {
                    matchVerticalDatum = verticalDatum;
                    matchVerticalDatumFound = true;
                    matchDistance = theDistance;
                }
            }
        }
    
    
    
        if (matchVerticalDatumFound)
        {
            if (matchVerticalDatum != theGCS->GetVerticalDatumCode())
            {
            transformInconsistentWithVerticalDatum = true;
            }
        }
        else if (maxDistance > 0.001)
        {
            transformInconsistent = true;
            copyGCS = nullptr;
        }
        else
            copyGCS = nullptr;
    }

    


    // Report
    bool anythingNeedsFixing = (isAScalableMesh && (!GCSPresent || 
                                                   !GCSComplete || 
                                                   !GCSValid)) || 
                               !transformPresent || 
                               transformJSONInvalid || 
                               transformInconsistent ||
                               transformInconsistentWithVerticalDatum;

    if (!anythingNeedsFixing)
    {
        printf ("Nothing to fix! Dataset is clean.\n");
    }
    else
    {
        printf ("Some issues were located:\n");
        if (!GCSPresent)
            printf ("No Geographic Coordinate Reference System found. Assuming pure ECEF.\n");

        if (GCSPresent && !GCSValid)
            printf("Invalid Geographic Coordinate System specified %s\n", theWKT.c_str());

        if (GCSPresent && GCSValid && !GCSComplete)
        {
            printf("Geographic Coordinate System specified but does not specify vertical datum. Vertical Datum Should be added\n");
            if (transformPresent && transformInconsistentWithVerticalDatum)
            {
                printf ("Default vertical datum %s is INCONSISTENT with transform stored\n", theGCS->GetVerticalDatumName());
                printf ("Either vertical datum %s must be set or the transform be recomputed\n", copyGCS->GetVerticalDatumName());
            }
            else if (transformPresent && !transformInconsistent)
            {
                printf ("Default vertical datum %s is consistent with transform stored\n", theGCS->GetVerticalDatumName());
            }
            else
            {
                printf ("Default vertical datum is: %s \n", theGCS->GetVerticalDatumName());
                printf ("Use --verticaldatum parameter to set another one\n");
            }
        }

        if (!transformPresent)
        {
            printf ("File contains no transform. One must be computed.\n");
        }
        else 
        {
            if (transformJSONInvalid)
                printf ("Transform stored is invalid. It must be re-computed.\n");

            if (transformInconsistent)
            {
                printf ("Transform stored is inconsistent with any vertical datum. It must be re-computed.\n");
                printf ("Positioning error: %lf \n", maxDistance);
            }
        }
    }


    // Fix or change
    if (fix && anythingNeedsFixing)
    {
        if (!GCSPresent)
        {
            printf("Geographic Coordinate Reference injection not yet implemented.\n");
            return 1;
        }

        if (GCSPresent && !GCSValid)
        {
            printf("Replacement of Geographic Coordinate System not yet implemented.\n");
            return 1;
        } 

        if (GCSPresent && GCSValid && !GCSComplete)
        {
            // Check is a vertical datum was specified
            if (verticalDatumSpecified)
            {
                theGCS->SetVerticalDatumCode(vertDatum);
            }
            Utf8String fullWellKnownText;
            if (SUCCESS != theGCS->GetCompoundCSWellKnownText(fullWellKnownText, GeoCoordinates::BaseGCS::wktFlavorUnknown, true))
            {
                printf("ERROR Generating the Compound WKT.\n");
                return 1;
            }
            Json::Value newGCS = fullWellKnownText;
            SMMasterHeader["GCS"] = newGCS;
            root["SMMasterHeader"] = SMMasterHeader;
            printf("Vertical datum set to %s \n", theGCS->GetVerticalDatumName());
        }
    
        if (GCSPresent && GCSValid)
        {
            // We recompute the transform anyway (no transform or any inconsistency ...)
            Transform newTransform;
            StatusInt res = theGCS->GetLinearTransformToECEF(&newTransform, extent, nullptr, nullptr);
            if (!((REPROJECT_Success == res || REPROJECT_CSMAPERR_OutOfUsefulRange == res || REPROJECT_CSMAPERR_VerticalDatumConversionError == res)))
            {
                printf("ERROR: Could not compute the new transform");
                return 1;
            }
            auto matrix = DMatrix4d::From(newTransform);

            transform[0]  = matrix.coff[0][0];
            transform[1]  = matrix.coff[1][0];
            transform[2]  = matrix.coff[2][0];
            transform[3]  = matrix.coff[3][0];
            transform[4]  = matrix.coff[0][1];
            transform[5]  = matrix.coff[1][1];
            transform[6]  = matrix.coff[2][1];
            transform[7]  = matrix.coff[3][1];
            transform[8]  = matrix.coff[0][2];
            transform[9]  = matrix.coff[1][2];
            transform[10] = matrix.coff[2][2];
            transform[11] = matrix.coff[3][2];
            transform[12] = matrix.coff[0][3];
            transform[13] = matrix.coff[1][3];
            transform[14] = matrix.coff[2][3];
            transform[15] = matrix.coff[3][3];

            root["transform"] = transform;
        }

        // Write to string
        resultJson["root"] = root;
        Utf8String finalJson = resultJson.toStyledString();

        printf ("Result:\n");
        printf ("%s\n", finalJson.c_str());
    }

    if (change)
    {
        // The only thing that can be changed is the vertical datum.
        if (!(GCSPresent && GCSValid && GCSComplete))
        {
            printf ("GCS is either absent or incomplete; cannot change. Fix first!\n");
            return 1;
        }


        // Check is a vertical datum was specified
        if (verticalDatumSpecified)
        {
            theGCS->SetVerticalDatumCode(vertDatum);
        }
        Utf8String fullWellKnownText;
        if (SUCCESS != theGCS->GetCompoundCSWellKnownText(fullWellKnownText, GeoCoordinates::BaseGCS::wktFlavorUnknown, true))
        {
            printf("ERROR Generating the Compound WKT.\n");
            return 1;
        }
        Json::Value newGCS = fullWellKnownText;
        SMMasterHeader["GCS"] = newGCS;
        root["SMMasterHeader"] = SMMasterHeader;
        printf("Vertical datum set to %s \n", theGCS->GetVerticalDatumName());

        // We recompute the transform anyway (no transform or any inconsistency ...)
        Transform newTransform;
        StatusInt res = theGCS->GetLinearTransformToECEF(&newTransform, extent, nullptr, nullptr);
        if (!((REPROJECT_Success == res || REPROJECT_CSMAPERR_OutOfUsefulRange == res || REPROJECT_CSMAPERR_VerticalDatumConversionError == res)))
        {
            printf("ERROR: Could not compute the new transform");
            return 1;
        }
        auto matrix = DMatrix4d::From(newTransform);

        transform[0] = matrix.coff[0][0];
        transform[1] = matrix.coff[1][0];
        transform[2] = matrix.coff[2][0];
        transform[3] = matrix.coff[3][0];
        transform[4] = matrix.coff[0][1];
        transform[5] = matrix.coff[1][1];
        transform[6] = matrix.coff[2][1];
        transform[7] = matrix.coff[3][1];
        transform[8] = matrix.coff[0][2];
        transform[9] = matrix.coff[1][2];
        transform[10] = matrix.coff[2][2];
        transform[11] = matrix.coff[3][2];
        transform[12] = matrix.coff[0][3];
        transform[13] = matrix.coff[1][3];
        transform[14] = matrix.coff[2][3];
        transform[15] = matrix.coff[3][3];

        root["transform"] = transform;
        

        // Write to string
        resultJson["root"] = root;
        Utf8String finalJson = resultJson.toStyledString();

        printf("Result:\n");
        printf("%s\n", finalJson.c_str());
    }

    return 0;
}
