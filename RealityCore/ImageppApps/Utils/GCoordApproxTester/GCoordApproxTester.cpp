//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/GCoordApproxTester/GCoordApproxTester.cpp $
//:>    $RCSfile: HTiffInfo.cpp,v $
//:>   $Revision: 1.15 $
//:>       $Date: 2011/07/18 21:12:32 $
//:>     $Author: Donald.Morissette $
//:>
//:>  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <windows.h>

#include <Imagepp/h/ImageppAPI.h>
#include <Imagepp/all/h/HTIFFFile.h>

#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HRFJpegFile.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <ImagePP/all/h/HRFFileFormats.h>
#include <Imagepp/all/h/ImageppLib.h>
#include <ImagePP/all/h/HCPGCoordModel.h>
#include <ImagePP/all/h/HCPGCoordLatLongModel.h>
#include <ImagePP/all/h/HVE2DRectangle.h>
#include <ImagePP/all/h/HGFPolynomialModelAdapter.h>
#include <ImagePP/all/h/HCPGCoordContiguousModelAdapter.h>

#include <ImagePP/all/h/HGF2DPolygonOfSegments.h>
#include <ImagePP/all/h/HGF2DLinearModelAdapter.h>


#include <GeoCoord/GCSLibrary.h>


USING_NAMESPACE_IMAGEPP

static uint32_t nbGCSTestedTotal = 0;
static uint32_t nbGCSProjective = 0;
static uint32_t nbGCSPoly = 0;
static uint32_t nbGCSExact = 0;
static uint32_t nbGCSError = 0;
static uint32_t nbGCSIntersectionLongitude_180 = 0;
static uint32_t nbGCSNoIntersection = 0;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MyImageppLibAdmin : ImagePP::ImageppLibAdmin
    {
    DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                            05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual ~MyImageppLibAdmin()
        {}
    };


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2015
//----------------------------------------------------------------------------------------
struct MyImageppLibHost : ImagePP::ImageppLib::Host
    {
    virtual ImagePP::ImageppLibAdmin& _SupplyImageppLibAdmin() override
        {
        return *new MyImageppLibAdmin();
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2015
    //----------------------------------------------------------------------------------------
    virtual void _RegisterFileFormat() override
        {
        // Not needed. REGISTER_SUPPORTED_FILEFORMAT
        }
    };


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Alexandre.Gariepy 7/2015
//----------------------------------------------------------------------------------------
void WriteResult(BeFile& file, WString const& fromKeyName, HGF2DLiteExtent const& fromExtent, WString const& toKeyName,
                 HGF2DLiteExtent const& toExtent, double meanErrorMeter, double relativeMeanError, double maxErrorMeter,
                 HGF2DPosition const& maxErrorPosition, double minErrorMeter, HGF2DPosition const& minErrorPosition)
    {
    WPrintfString s(L"%ls,%le,%le,%le,%le,%ls,%le,%le,%le,%le,%le,%le,%le,%le,%le,%le,%le,%le\n", fromKeyName.c_str() , fromExtent.GetXMin(),
                    fromExtent.GetYMin(), fromExtent.GetXMax(), fromExtent.GetYMax(), toKeyName.c_str(), toExtent.GetXMin(),
                    toExtent.GetYMin(), toExtent.GetXMax(), toExtent.GetYMax(), meanErrorMeter, relativeMeanError, maxErrorMeter,
                    maxErrorPosition.GetX(), maxErrorPosition.GetY(), minErrorMeter, minErrorPosition.GetX(), minErrorPosition.GetY());
    uint32_t dataSize = static_cast<uint32_t>(s.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
    file.Write(NULL, (void*) s.c_str(), dataSize);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Alexandre.Gariepy 7/2015
// @return true if the intersection domain intersect the 180 degree longitude line false otherwise
//----------------------------------------------------------------------------------------
bool GetDirectAndInverseDomains(HFCPtr<HGF2DShape>& po_pDomainDirect, HFCPtr<HGF2DShape>& po_pDomainInverse, GeoCoordinates::BaseGCSR pi_GcsSrc,
                                GeoCoordinates::BaseGCSR pi_GcsDst)
    {

    // Create the three coordinate systems required for transformation
    HFCPtr<HGF2DCoordSys> latLongCoordinateSystem = new HGF2DCoordSys();
    HFCPtr<HGF2DTransfoModel> directLatLongTransfoModel = new HCPGCoordLatLongModel(pi_GcsSrc);
    HFCPtr<HGF2DTransfoModel> inverseLatLongTransfoModel = new HCPGCoordLatLongModel(pi_GcsDst);
    HFCPtr<HGF2DCoordSys> directCoordinateSystem = new HGF2DCoordSys(*directLatLongTransfoModel, latLongCoordinateSystem);
    HFCPtr<HGF2DCoordSys> inverseCoordinateSystem = new HGF2DCoordSys(*inverseLatLongTransfoModel, latLongCoordinateSystem);

    //Exclusion of problematic domains (Longitude larger than maximum / discontinuity in the domain).
    bool testMinSuperiorLongitude = (pi_GcsSrc.GetMinimumUsefulLongitude() <= 180.0) && (pi_GcsDst.GetMinimumUsefulLongitude() <= 180.0);
    bool testMaxSuperiorLongitude = (pi_GcsSrc.GetMaximumUsefulLongitude() <= 180.0) || (pi_GcsDst.GetMaximumUsefulLongitude() <= 180.0);
    bool testInferiorLongitude = (pi_GcsSrc.GetMinimumUsefulLongitude() >= -179.999999) || (pi_GcsDst.GetMinimumUsefulLongitude() >= -179.999999);

    if (testMinSuperiorLongitude && testMaxSuperiorLongitude && testInferiorLongitude) //all tests must be true for the intersection domain to be in one block
        {
        // Create shapes from these
        HFCPtr<HVE2DShape> sourceDomainShape = new HVE2DRectangle(pi_GcsSrc.GetMinimumUsefulLongitude(),
                                                                  pi_GcsSrc.GetMinimumUsefulLatitude(),
                                                                  pi_GcsSrc.GetMaximumUsefulLongitude(),
                                                                  pi_GcsSrc.GetMaximumUsefulLatitude(),
                                                                  latLongCoordinateSystem);

        HFCPtr<HVE2DShape> destinationDomainShape = new HVE2DRectangle(pi_GcsDst.GetMinimumUsefulLongitude(),
                                                                       pi_GcsDst.GetMinimumUsefulLatitude(),
                                                                       pi_GcsDst.GetMaximumUsefulLongitude(),
                                                                       pi_GcsDst.GetMaximumUsefulLatitude(),
                                                                       latLongCoordinateSystem);

        HFCPtr<HVE2DShape> resultDomainShape = sourceDomainShape->IntersectShape(*destinationDomainShape);

        // Obtain shapes in direct of inverse coordinate systems, drop and copy points
        HVE2DShape* tempShapeDirect = static_cast<HVE2DShape*>(resultDomainShape->AllocateCopyInCoordSys(directCoordinateSystem));
        HVE2DShape* tempShapeInverse = static_cast<HVE2DShape*>(resultDomainShape->AllocateCopyInCoordSys(inverseCoordinateSystem));
        po_pDomainDirect = tempShapeDirect->GetLightShape();
        po_pDomainInverse = tempShapeInverse->GetLightShape();
        delete tempShapeDirect;
        delete tempShapeInverse;
        return false;
        }
    else
        {
        HFCPtr<HVE2DShape> nullDomain = new HVE2DRectangle(0, 0, 0, 0, latLongCoordinateSystem);
        po_pDomainDirect = nullDomain->GetLightShape();
        po_pDomainInverse = nullDomain->GetLightShape();
        return true;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 01/2016
// Store the results of an exact and approximated transfoModel into vector.
// The points of the boundary in the source and destination domain are saved.
// Otherwise, we take tie points into the destination domain and transforme them into the source domain.
//+---------------+---------------+---------------+---------------+---------------+------
void GetArrayTransformedPoints(const HGF2DTransfoModel& pi_rExactModel, const HGF2DTransfoModel& pi_rApproxModel, const HGF2DShape& pi_rShapeDst,
                               const HGF2DShape& pi_rShapeSrc, double pi_StepX, double pi_StepY, bvector<double>& initialPoints,
                               bvector<double>& exactTransformedPoints, bvector<double>& approxTransformedPoints, bvector<double>& verticesDst,
                               bvector<double>& verticesSrc, bool isDirect)
    {
    // The extent of area must not be empty
    HPRECONDITION(pi_rShapeDst.GetExtent().GetWidth() != 0.0);
    HPRECONDITION(pi_rShapeDst.GetExtent().GetHeight() != 0.0);

    // The step may not be null nor negative
    HPRECONDITION(pi_StepX > 0.0);
    HPRECONDITION(pi_StepY > 0.0);

    double currentX;
    double currentY;
    double transfoExactX, transfoExactY;
    double transfoApproxX, transfoApproxY;
    StatusInt status;

    HGF2DPositionCollection tempVerticesDst = static_cast<const HGF2DPolygonOfSegments&>(pi_rShapeDst).GetListOfPoints();

    HGF2DPositionCollection::const_iterator iterDst;

    //We transform the vertices of the BOUNDARY of the DESTINATION domain
    for (iterDst = tempVerticesDst.begin(); iterDst != tempVerticesDst.end(); ++iterDst)
        {
        currentX = (*iterDst).GetX();
        currentY = (*iterDst).GetY();

        verticesDst.push_back(currentX);
        verticesDst.push_back(currentY);

        if (isDirect)
            {
            status = pi_rExactModel.ConvertDirect(currentX, currentY, &transfoExactX, &transfoExactY);
            pi_rApproxModel.ConvertDirect(currentX, currentY, &transfoApproxX, &transfoApproxY);
            }
        else
            {
            status = pi_rExactModel.ConvertInverse(currentX, currentY, &transfoExactX, &transfoExactY);
            pi_rApproxModel.ConvertInverse(currentX, currentY, &transfoApproxX, &transfoApproxY);
            }

        if (SUCCESS != status)
            continue;

        initialPoints.push_back(currentX);
        initialPoints.push_back(currentY);

        exactTransformedPoints.push_back(transfoExactX);
        exactTransformedPoints.push_back(transfoExactY);

        approxTransformedPoints.push_back(transfoApproxX);
        approxTransformedPoints.push_back(transfoApproxY);
        }
    //END for the DESTINATION BOUNDARY

    //BEGIN the SOURCE BOUNDARY
    HGF2DPositionCollection tempVerticesSrc = static_cast<const HGF2DPolygonOfSegments&>(pi_rShapeSrc).GetListOfPoints();

    HGF2DPositionCollection::const_iterator iterSrc;

    for (iterSrc = tempVerticesSrc.begin(); iterSrc != tempVerticesSrc.end(); ++iterSrc)
        {
        verticesSrc.push_back((*iterSrc).GetX());
        verticesSrc.push_back((*iterSrc).GetY());
        }
    //END for the SOURCE BOUNDARY

    //We transform the tie points of the destination domain into the source domain with exact and approximate models
    for (currentY = pi_rShapeDst.GetExtent().GetYMin(); currentY <= pi_rShapeDst.GetExtent().GetYMax(); currentY += pi_StepY)
        {
        for (currentX = pi_rShapeDst.GetExtent().GetXMin(); currentX <= pi_rShapeDst.GetExtent().GetXMax(); currentX += pi_StepX)
            {
            if (pi_rShapeDst.IsPointIn(HGF2DPosition(currentX, currentY)) || pi_rShapeDst.IsPointOn(HGF2DPosition(currentX, currentY)))
                {

                if (isDirect)
                    {
                    status = pi_rExactModel.ConvertDirect(currentX, currentY, &transfoExactX, &transfoExactY);
                    pi_rApproxModel.ConvertDirect(currentX, currentY, &transfoApproxX, &transfoApproxY);
                    }
                else
                    {
                    status = pi_rExactModel.ConvertInverse(currentX, currentY, &transfoExactX, &transfoExactY);
                    pi_rApproxModel.ConvertInverse(currentX, currentY, &transfoApproxX, &transfoApproxY);
                    }

                if (SUCCESS != status)
                    continue;

                initialPoints.push_back(currentX);
                initialPoints.push_back(currentY);

                exactTransformedPoints.push_back(transfoExactX);
                exactTransformedPoints.push_back(transfoExactY);

                approxTransformedPoints.push_back(transfoApproxX);
                approxTransformedPoints.push_back(transfoApproxY);
                }
            }
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 01/2016
// Format points of the boundary and transformed points in a .cvs file in order to 
// facilitate ploting the domains.
//+---------------+---------------+---------------+---------------+---------------+------
void WriteDataToPlot(BeFile& file, WString const& fromKeyName, WString const& toKeyName, const bvector<double>& initialPoints,
                     const bvector<double>& exactTransformedPoints, const bvector<double>& approxTransformedPoints,
                     const bvector<double>& verticesDst, const bvector<double>& verticesSrc, BeFile& plotFile)
    {
    WPrintfString s(L"%ls,%ls\n\n", fromKeyName.c_str(), toKeyName.c_str());
    uint32_t dataSize = static_cast<uint32_t>(s.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
    plotFile.Write(NULL, (void*) s.c_str(), dataSize);

    WPrintfString t1(L"Vertices in DST,,,Initial points in DST,,,Exact Transformation,,,Approx. Transform.,,,Vertices in SRC\nXValue,Y Value,,"
                     L"XValue,Y Value,,XValue,Y Value,,XValue,Y Value,,XValue,Y Value,,\n");
    dataSize = static_cast<uint32_t>(t1.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
    plotFile.Write(NULL, (void*) t1.c_str(), dataSize);

    bvector<double>::const_iterator iter;
    bvector<double>::const_iterator iterExact = exactTransformedPoints.begin();
    bvector<double>::const_iterator iterPoly = approxTransformedPoints.begin();
    bvector<double>::const_iterator iterDst = verticesDst.begin();
    bvector<double>::const_iterator iterSrc = verticesSrc.begin();

    for (iter = initialPoints.begin(); iter != initialPoints.end(); iter += 2)
        {
        if (iterDst != verticesDst.end())
            {
            WPrintfString t5(L"%le,%le,,", *iterDst, *(iterDst + 1));
            dataSize = static_cast<uint32_t>(t5.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
            plotFile.Write(NULL, (void*) t5.c_str(), dataSize);
            iterDst += 2;

            WPrintfString t2(L"%le,%le,,", *iter, *(iter + 1));
            dataSize = static_cast<uint32_t>(t2.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
            plotFile.Write(NULL, (void*) t2.c_str(), dataSize);
            }
        else
            {
            WPrintfString t2(L",,,%le,%le,,", *iter, *(iter + 1));
            dataSize = static_cast<uint32_t>(t2.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
            plotFile.Write(NULL, (void*) t2.c_str(), dataSize);
            }

        WPrintfString t3(L"%le,%le,,", *iterExact, *(iterExact + 1));
        dataSize = static_cast<uint32_t>(t3.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
        plotFile.Write(NULL, (void*) t3.c_str(), dataSize);

        if (iterSrc != verticesSrc.end())
            {
            WPrintfString t4(L"%le,%le,,", *iterPoly, *(iterPoly + 1));
            dataSize = static_cast<uint32_t>(t4.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
            plotFile.Write(NULL, (void*) t4.c_str(), dataSize);

            WPrintfString t6(L"%le,%le\n", *iterSrc, *(iterSrc + 1));
            dataSize = static_cast<uint32_t>(t6.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
            plotFile.Write(NULL, (void*) t6.c_str(), dataSize);
            iterSrc += 2;
            }
        else
            {
            WPrintfString t4(L"%le,%le\n", *iterPoly, *(iterPoly + 1));
            dataSize = static_cast<uint32_t>(t4.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
            plotFile.Write(NULL, (void*) t4.c_str(), dataSize);
            }

        iterExact += 2;
        iterPoly += 2;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 01/2016
//+---------------+---------------+---------------+---------------+---------------+------
void CreateDataToPlot(const HCPGCoordModel& pi_rExactModel, const HGF2DTransfoModel& pi_rApproxModel, const HGF2DShape& pi_rShapeDst,
                      const HGF2DShape& pi_rShapeSrc, BeFile& plotFile, const WString& gcsKeyNameSrc, const WString& gcsKeyNameDst, bool isDirect = false)
    {
    const uint32_t DIVISIONS_IN_WIDTH = 15;
    const uint32_t DIVISIONS_IN_HEIGHT = 15;

    bvector<double> exactTransformedPoints;
    bvector<double> approxTransformedPoints;
    bvector<double> initialPoints;
    bvector<double> verticesDst;
    bvector<double> verticesSrc;

    GetArrayTransformedPoints(pi_rExactModel, pi_rApproxModel, pi_rShapeDst, pi_rShapeSrc, pi_rShapeDst.GetExtent().GetWidth() / DIVISIONS_IN_WIDTH,
                              pi_rShapeDst.GetExtent().GetHeight() / DIVISIONS_IN_HEIGHT, initialPoints, exactTransformedPoints, approxTransformedPoints,
                              verticesDst, verticesSrc, isDirect);
    BeFile outputPlotData;
    WString outputFileName(L"data_plot.csv");

    WriteDataToPlot(outputPlotData, gcsKeyNameSrc, gcsKeyNameDst, initialPoints, exactTransformedPoints, approxTransformedPoints,
                    verticesDst, verticesSrc, plotFile);
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent.Robert-Veillette   02/2016
//+---------------+---------------+---------------+---------------+---------------+------
void RunBenchmark(const WString& gcsKeyNameSrc, const WString& gcsKeyNameDst, BeFile& outputFile, BeFile& errorFile, BeFile& plotFile, bool outputPlot)
    {
    GeoCoordinates::BaseGCSPtr pGcsSrc = GeoCoordinates::BaseGCS::CreateGCS(gcsKeyNameSrc.c_str());
    GeoCoordinates::BaseGCSPtr pGcsDst = GeoCoordinates::BaseGCS::CreateGCS(gcsKeyNameDst.c_str());
    if (!pGcsSrc->IsValid() || !pGcsDst->IsValid())
        return;

    ++nbGCSTestedTotal;

    HFCPtr<HGF2DShape> domainDirect;
    HFCPtr<HGF2DShape> domainInverse;
    bool IntersectLongitude_180 = GetDirectAndInverseDomains(domainDirect, domainInverse, *pGcsSrc, *pGcsDst);

    if (IntersectLongitude_180)
        {
        ++nbGCSIntersectionLongitude_180;
        WPrintfString errorMessage(L"%ls,%ls,INTERSECTION WITH 180 DEGREES LONGITUDE\n", gcsKeyNameSrc.c_str(), gcsKeyNameDst.c_str());
        uint32_t dataSize = static_cast<uint32_t>(errorMessage.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
        outputFile.Write(NULL, (void*) errorMessage.c_str(), dataSize);
        return;
        }

    if (!domainInverse->IsNull())
        {
        //Test the AFFINE AND PROJECTIVE models
        HCPGCoordModel inverseExactModel(*pGcsDst, *pGcsSrc);
        double minDimension = domainInverse->GetExtent().GetWidth() < domainInverse->GetExtent().GetHeight() ?
            domainInverse->GetExtent().GetWidth() : domainInverse->GetExtent().GetHeight();

        HGF2DLinearModelAdapter projectiveModel(inverseExactModel, domainInverse->GetExtent(), minDimension / 10);

        //Test the error generated by the affine/projective model. Do not return StatusInt. We cannot know if study was successful
        double MeanError, MaxError;
        projectiveModel.StudyPrecisionOver(domainInverse->GetExtent(), minDimension / 10, &MeanError, &MaxError);

        //Creates an output file .cvs with data to visualize the PROJECTIVE transformation in scattered plots.
        //if (outputPlot)
        //    {
        //    CreateDataToPlot(inverseExactModel, projectiveModel, *domainInverse, *domainDirect, plotFile, gcsKeyNameSrc, gcsKeyNameDst, true);
        //    }

        double factorToMeter = pGcsDst->UnitsFromMeters();
        // If MaxError == 0 , then no valid point in the domain has been used to study the precision. We consider this as an error in CsMap and do not consider the results
        if (MaxError * factorToMeter < 0.01 && MaxError > 0)
            {
            ++nbGCSProjective;
            WPrintfString errorMessage(L"AFFINE OR PROJECTIVE MODEL FIT WITH MEAN ERROR = %le AND MAX ERROR = %le,%ls,%ls\n", MeanError, MaxError,
                                       gcsKeyNameSrc.c_str(), gcsKeyNameDst.c_str());
            uint32_t dataSize = static_cast<uint32_t>(errorMessage.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
            outputFile.Write(NULL, (void*) errorMessage.c_str(), dataSize);
            return;
            }

        //If affine/projective model do not fit, test the POLYNOMIAL model
        HCPGCoordModel exactModel(*pGcsSrc, *pGcsDst);
        HGFPolynomialModelAdapter polyModel(exactModel, *domainInverse, domainInverse->GetExtent().GetWidth() / 10,
                                            domainInverse->GetExtent().GetHeight() / 10);

        if (!polyModel.HasEnoughTiePoints())
            {
            ++nbGCSExact;
            WPrintfString errorMessage(L"NOT_ENOUGH_POINTS,%ls,%ls\n", gcsKeyNameSrc.c_str(), gcsKeyNameDst.c_str());
            uint32_t dataSize = static_cast<uint32_t>(errorMessage.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
            errorFile.Write(NULL, (void*) errorMessage.c_str(), dataSize);
            return;
            }

        //Creates an output file .cvs with data to visualize the POLYNOMIAL transformation in scattered plots.
        if (outputPlot)
            {
            CreateDataToPlot(exactModel, polyModel, *domainInverse, *domainDirect, plotFile, gcsKeyNameSrc, gcsKeyNameDst);
            }


        // Study the error for the polynomial model
        double meanErrorInSourceUnits, maxErrorInSourceUnits, minErrorInSourceUnits;
        HGF2DPosition maxErrorPosition, minErrorPosition;
        StatusInt status = polyModel.GetMeanError(&meanErrorInSourceUnits, &maxErrorInSourceUnits, &maxErrorPosition,
                               &minErrorInSourceUnits, &minErrorPosition);

        if (factorToMeter * maxErrorInSourceUnits < 0.01 && SUCCESS == status)
            {
            ++nbGCSPoly;
            double relativeMeanError = meanErrorInSourceUnits / ((domainDirect->GetExtent().GetWidth() + domainDirect->GetExtent().GetHeight()) / 2);

            WriteResult(outputFile, gcsKeyNameSrc, domainDirect->GetExtent(), gcsKeyNameDst, domainInverse->GetExtent(),
                        meanErrorInSourceUnits * factorToMeter, relativeMeanError, maxErrorInSourceUnits * factorToMeter,
                        maxErrorPosition, minErrorInSourceUnits * factorToMeter, minErrorPosition);
            return;
            }
        else if(SUCCESS == status)
            {
            ++nbGCSExact;
            WPrintfString errorMessage(L"CANNOT REPROJECT WITH APPROXIMATIONS,%ls,%ls\n", gcsKeyNameSrc.c_str(), gcsKeyNameDst.c_str());
            uint32_t dataSize = static_cast<uint32_t>(errorMessage.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
            errorFile.Write(NULL, (void*) errorMessage.c_str(), dataSize);
            return;
            }
        else
            {
            ++nbGCSError;
            WPrintfString errorMessage(L"ERROR IN CSMAP,%ls,%ls\n", gcsKeyNameSrc.c_str(), gcsKeyNameDst.c_str());
            uint32_t dataSize = static_cast<uint32_t>(errorMessage.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
            errorFile.Write(NULL, (void*) errorMessage.c_str(), dataSize);
            return;
            }
        }
    else
        {
        ++nbGCSNoIntersection;
        WPrintfString errorMessage(L"NO_INTERSECTION,%ls,%ls\n", gcsKeyNameSrc.c_str(), gcsKeyNameDst.c_str());
        uint32_t dataSize = static_cast<uint32_t>(errorMessage.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
        errorFile.Write(NULL, (void*) errorMessage.c_str(), dataSize);
        return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PrintHelp()
    {
    printf("-h\t\tHelp: Prints this help text.\n\n");
    printf("-p arg1 arg2\tPart: Specifies the part of the benchmark to execute. Example: -p 1 5 does the first fifth of the job.\n\n");
    printf("-s2 arg1 arg2\tSpecific2: Benchmarks a specific transform between arg1 and arg2.\n\n");
    cin.ignore();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Initialize(WString exePath, BeFile& outputFile, WString outputFileName, BeFile& errorFile, WString errorFileName, BeFile& plotFile, WString plotFileName)
    {
    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

    size_t index = exePath.find_last_of(L"/\\");

    WString geoCoordPath = exePath.substr(0, index).append(L"\\GeoCoordinateData\\");
    GeoCoordinates::BaseGCS::Initialize(geoCoordPath.c_str());

    WString outputFilePath = exePath.substr(0, index).append(L"\\").append(outputFileName.c_str());
    outputFile.Create(outputFilePath.c_str(), true);
    outputFile.Open(outputFilePath.c_str(), BeFileAccess::Write);
    WString s(L"SrcGCS,SrcXMin,SrcYMin,SrcXMax,SrcYMax,DstGCS,DstXMin,DstYMin,DstXMax,DstYMax,MeanError(meter),RelativeMeanError,MaxError(meter),"
              L"MaxErrorXCoord,MaxErrorYCoord,MinError(meter),MinErrorXCoord,MinErrorYCoord\n");
    uint32_t dataSize = static_cast<uint32_t>(s.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
    outputFile.Write(NULL, (void*) s.c_str(), dataSize);

    WString errorFilePath = exePath.substr(0, index).append(L"\\").append(errorFileName.c_str());
    errorFile.Create(errorFilePath.c_str(), true);
    errorFile.Open(errorFilePath.c_str(), BeFileAccess::Write);

    WString plotFilePath = exePath.substr(0, index).append(L"\\").append(plotFileName.c_str());
    plotFile.Create(plotFilePath.c_str(), true);
    plotFile.Open(plotFilePath.c_str(), BeFileAccess::Write);
    WString t(L"SrcGCS,DstGCS\n");
    uint32_t dataSize2 = static_cast<uint32_t>(t.SizeInBytes() - sizeof(wchar_t)); //Remove the end of string null char
    plotFile.Write(NULL, (void*) t.c_str(), dataSize2);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Alexandre.Gariepy 07/2015
//----------------------------------------------------------------------------------------
int wmain(int pi_Argc, wchar_t *pi_ppArgv[])
    {
    WString exePath(pi_ppArgv[0]);

    BeFile outputFile;
    BeFile errorFile;
    BeFile plotFile;
    WString outputFileName(L"benchmark_result.csv");
    WString errorFileName(L"error_log.csv");
    WString plotFileName(L"data_plot.csv");

    uint32_t numberOfParts = 1;
    uint32_t partNumber = 1;

    for (int i = 1; i < pi_Argc; ++i)
        {
        if (BeStringUtilities::Wcsicmp(pi_ppArgv[i], L"-h") == 0)
            {
            PrintHelp();
            return 0;
            }
        else if (BeStringUtilities::Wcsicmp(pi_ppArgv[i], L"-p") == 0)
            {
            if (pi_Argc <= i + 2)
                {
                printf("ERROR: Unsupported argument\n");
                cin.ignore();
                return -1;
                }
            partNumber = static_cast<uint32_t>(BeStringUtilities::Wtoi(pi_ppArgv[i + 1]));
            numberOfParts = static_cast<uint32_t>(BeStringUtilities::Wtoi(pi_ppArgv[i + 2]));
            i += 2; //Ignore the two next arguments

            if (numberOfParts < partNumber || partNumber <= 0)
                {
                printf("ERROR: Invalid part numbers\n");
                cin.ignore();
                return -1;
                }
            outputFileName = WPrintfString(L"benchmark_result_%d_%d.csv", partNumber, numberOfParts);
            errorFileName = WPrintfString(L"error_log_%d_%d.csv", partNumber, numberOfParts);
            }
        else if (BeStringUtilities::Wcsicmp(pi_ppArgv[i], L"-s2") == 0)
            {
            if (pi_Argc <= i + 2)
                {
                printf("ERROR: Unsupported argument\n");
                cin.ignore();
                return -1;
                }
            WString gcsKeyNameSrc(pi_ppArgv[i + 1]);
            WString gcsKeyNameDst(pi_ppArgv[i + 2]);
            Initialize(exePath, outputFile, outputFileName, errorFile, errorFileName, plotFile, plotFileName);
            RunBenchmark(gcsKeyNameSrc, gcsKeyNameDst, outputFile, errorFile, plotFile, true);
            return 0;
            }
        else
            {
            printf("ERROR: Unsupported argument\n");
            cin.ignore();
            return -1;
            }
        }

    Initialize(exePath, outputFile, outputFileName, errorFile, errorFileName, plotFile, plotFileName);

    SYSTEMTIME time;

    //Iterate on all gcs combinations
    uint32_t gcsCount = static_cast<uint32_t>(GeoCoordinates::LibraryManager::Instance()->GetLibrary(0)->GetCSCount());
    uint32_t partLenght = gcsCount / numberOfParts;
    uint32_t maxGcsNumber = (partNumber == numberOfParts) ? gcsCount : partNumber*partLenght;

    for (uint32_t gcsIndexSrc = (partNumber - 1) * partLenght; gcsIndexSrc < maxGcsNumber; gcsIndexSrc++)
        {
        WString gcsKeyNameSrc;
        GeoCoordinates::LibraryManager::Instance()->GetLibrary(0)->GetCSName(gcsIndexSrc, gcsKeyNameSrc);

        for (uint32_t gcsIndexDst = 0; gcsIndexDst < gcsCount; gcsIndexDst++)
            {
            if (gcsIndexSrc == gcsIndexDst)
                continue;

            WString gcsKeyNameDst;
            GeoCoordinates::LibraryManager::Instance()->GetLibrary(0)->GetCSName(gcsIndexDst, gcsKeyNameDst);
            RunBenchmark(gcsKeyNameSrc, gcsKeyNameDst, outputFile, errorFile, plotFile, false);
            }
        outputFile.Flush();
        GetLocalTime(&time);
        printf("Finished %u / %u at %02d:%02d:%02d\n", gcsIndexSrc, gcsCount, time.wHour, time.wMinute, time.wSecond);
        }

    printf("Approx. by affine or projective : %le percent\n", (double) (nbGCSProjective) / (nbGCSTestedTotal) * 100);
    printf("Approx. by polynomial : %le percent\n", (double) (nbGCSPoly) / (nbGCSTestedTotal) * 100);
    printf("No approx. Exact model must be use : %le percent\n", (double) (nbGCSExact) / (nbGCSTestedTotal) * 100);
    printf("Error in CsMap : %le percent\n", (double) (nbGCSError) / (nbGCSTestedTotal) * 100);
    printf("No approx. possible. No intersection between domains! : %le percent\n", (double) (nbGCSNoIntersection) / (nbGCSTestedTotal) * 100);
    printf("Intersection at the 180 degree longitude : %le percent\n", (double) (nbGCSIntersectionLongitude_180) / (nbGCSTestedTotal) * 100);
    outputFile.Close();
    errorFile.Close();

    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);

    return 0;
    }
