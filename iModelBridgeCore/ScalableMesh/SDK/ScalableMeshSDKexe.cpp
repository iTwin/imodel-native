
#include "DataPipe.h"
#include "Initialize.h"
#include "ScalableMeshSDKexe.h"
#include "ScalableMeshSDKexeImporter.h"

#include <TerrainModel\Core\bcDTMClass.h>
#include <TerrainModel\Core\IDTM.h>
#include <ScalableTerrainModel\IMrDTM.h>
#include <ScalableTerrainModel\IMrDTMCreator.h>
#include <ScalableTerrainModel\IMrDTMQuery.h>


USING_NAMESPACE_BENTLEY_MRDTM
USING_NAMESPACE_BENTLEY_TERRAINMODEL    


extern DataPipe s_dataPipe;

namespace ScalableMeshSDKexe
    {   

    void ScalableMeshSDKexe::OpenPipe()
        {
        WString name = L"\\\\.\\pipe\\";
        name.append(BeFileName::GetFileNameWithoutExtension(m_outputName));

        m_pipe = CreateNamedPipe(
            name.c_str(),
            PIPE_ACCESS_OUTBOUND, // 1-way pipe -- send only
            PIPE_TYPE_BYTE, // send data as a byte stream
            1, 
            0, 
            0, 
            0,
            NULL 
            );
        if (m_pipe == NULL || m_pipe == INVALID_HANDLE_VALUE) fwprintf(stderr, L"Error creating pipe\n");
        //if (!ConnectNamedPipe(m_pipe, NULL)) fwprintf(stderr, L"No client connected\n");
        }

    BentleyStatus ScalableMeshSDKexe::Initialize(int argc, WCharP argv[])
        {
        BeFileName::FixPathName(m_outputName, m_outputName.c_str());
        m_outputName.BeGetFullPathName();

        BeFileName outputDir = BeFileName(BeFileName::GetDirectoryName(m_outputName.c_str()).c_str());

        if (m_pipe == NULL)
            {
            OpenPipe();
            }
        DgnViewLib::Initialize(*this, true); // this initializes the DgnDb libraries
        InitializeSDK(*this);
        setlocale(LC_CTYPE, "");
        return SUCCESS;
        }

    BentleyStatus ScalableMeshSDKexe::OpenScalableMesh(WCharCP path)
        {
        std::cout << "OPENING MESH " << path << std::endl;
        IScalableMeshPtr ptr = IScalableMesh::GetFor(path, true, true);
        m_sMesh = ptr;
        return SUCCESS;
        }


    size_t  ScalableMeshSDKexe::CountPoints()
        {
        if (m_sMesh == nullptr) std::cout << "ERROR, NO SCALABLE MESH " << std::endl;
        else
            {
            std::cout << "POINTS " << m_sMesh->GetPointCount() << std::endl;
            return m_sMesh->GetPointCount();
            }
        return 0;
        }
    
        
    struct TransmittedDataHeader
        {
        TransmittedDataHeader(bool arePoints, bool is3dData)
            {
            m_arePoints = arePoints;
            m_is3dData = is3dData;
            }

        bool m_arePoints;
        bool m_is3dData;
        };

    struct  TransmittedPointsHeader : public TransmittedDataHeader
        {      
        TransmittedPointsHeader(unsigned int nbOfPoints, bool is3dData)
        : TransmittedDataHeader(true, is3dData)
            {
            m_nbOfPoints = nbOfPoints;
            }

        unsigned int m_nbOfPoints;        
        };

    HANDLE s_pipe;
        

    static FILE*  s_pPointResultFile = 0;

#if 0 

bool WritePointsCallback(const DPoint3d* points, size_t nbOfPoints, bool arePoints3d)
	{
	char coordinateBuffer[300];   
    int  NbChars;        

    for (size_t PointInd = 0; PointInd < nbOfPoints; PointInd++)
        {                
        NbChars = sprintf(coordinateBuffer, 
                          "%.20f,%.20f,%.20f\n", 
                          points[PointInd].x, 
                          points[PointInd].y, 
                          points[PointInd].z);         

        size_t nbCharsWritten = fwrite (coordinateBuffer, 1, NbChars, s_pPointResultFile);
                
        assert(NbChars == nbCharsWritten);    
        }  

    fflush(s_pPointResultFile);

    return true;
	}
#endif

        
    int WritePointsCallback(DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId featureId, DPoint3d *points, size_t numPoints, void* userArg)
        {   
        TransmittedPointsHeader pointHeader((unsigned int)numPoints, false);
                
        DWORD  numBytesWritten;        
        WriteFile(s_pipe,
                  &pointHeader,
                  (DWORD)sizeof(pointHeader),
                  &numBytesWritten,
                  NULL 
                  );

        WriteFile(s_pipe,
                  points,
                  (DWORD)(sizeof(DPoint3d) * numPoints),
                  &numBytesWritten,
                  NULL 
                  );
       
        return true;
        }

    struct TransmittedFeatureHeader : public TransmittedDataHeader
        {      
        TransmittedFeatureHeader(unsigned int nbOfFeaturePoints, short featureType, bool is3dData)
        : TransmittedDataHeader(false, is3dData)
            {
            m_nbOfFeaturePoints = nbOfFeaturePoints;
            m_featureType = featureType;
            }

        short        m_featureType;
        unsigned int m_nbOfFeaturePoints;        
        };
    

    int WriteFeatureCallback(DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId featureId, DPoint3d *points, size_t numPoints, void* userArg)
        {        
        TransmittedFeatureHeader featureHeader((unsigned int)numPoints, (short)dtmFeatureType, false);

        DWORD numBytesWritten;        
        WriteFile(s_pipe,
                  &featureHeader,
                  (DWORD)sizeof(featureHeader),
                  &numBytesWritten,
                  NULL 
                  );
        
        WriteFile(s_pipe,
                  points,
                  (DWORD)(sizeof(DPoint3d) * numPoints),
                  &numBytesWritten,
                  NULL 
                  );

        return true;
        }
    

    DRange3d s_importRange; 
    
    bool StreamPointsCallback(const DPoint3d* points, size_t nbOfPoints, bool arePoints3d)
        {           
        if (nbOfPoints > 0)
            {
            bvector<DPoint3d> pointsInRange; 

            for (size_t ptInd = 0; ptInd < nbOfPoints; ptInd++)
                {
                if (s_importRange.IsContainedXY(points[ptInd]))
                    {
                    pointsInRange.push_back(points[ptInd]);
                    }
                }        

            s_dataPipe.WritePoints(&pointsInRange[0], pointsInRange.size());
            }

        return true;
        }

    static bool s_finishWritingPointDone = false;

    bool StreamFeatureCallback(const DPoint3d* featurePoints, size_t nbOfFeaturesPoints, DTMFeatureType featureType, bool isFeature3d)
        {                        
        static bool s_firstCall = true;

        if (s_firstCall)
            {
            s_dataPipe.FinishWritingPoints();
            s_firstCall = false;
            s_finishWritingPointDone = true;
            }

        if (nbOfFeaturesPoints > 0)
            {
            bvector<DPoint3d> pointsInRange; 
            bool wasContained; 

            wasContained = s_importRange.IsContainedXY(featurePoints[0]);
            pointsInRange.push_back(featurePoints[0]);

            for (size_t ptInd = 0; ptInd < nbOfFeaturesPoints; ptInd++)
                {
                bool isContained = s_importRange.IsContainedXY(featurePoints[ptInd]);

                if (isContained && !wasContained && (ptInd - 1) == 0)
                    {
                    pointsInRange.push_back(featurePoints[ptInd - 1]);                                        
                    }

                if (isContained || wasContained)
                    {
                    pointsInRange.push_back(featurePoints[ptInd]);                                        

                    if (isContained == false)
                        {
                        s_dataPipe.WriteFeature(&pointsInRange[0], pointsInRange.size(), featureType);
                        pointsInRange.clear();
                        }
                    }
                                                                               
                wasContained = isContained;
                } 

            if (pointsInRange.size() > 0)
                s_dataPipe.WriteFeature(&pointsInRange[0], pointsInRange.size(), featureType);
            }

        return true;
        }
    
    void ImportThread(DgnPlatformLib::Host* hostToAdopt, Bentley::ScalableMesh::IScalableMeshSourceImporterPtr& importerPtr)
        {    
        DgnPlatformLib::AdoptHost(*hostToAdopt);

        importerPtr->Import();

        if (!s_finishWritingPointDone)
            {
            s_dataPipe.FinishWritingPoints();
            }

        s_dataPipe.FinishWritingFeatures();
        }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 11/2015
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt GetApproximationNbPtsNeedToExtract(IMrDTMPtr                    mrDTMPtr,                                                                                                   
                                             const std::vector<DPoint3d>& regionPointsInStorageCS,                                             
                                             unsigned int*                nbPointsForPointFeatures, 
                                             unsigned int*                nbPointsForLinearFeatures)
    {
    DTMPtr                   dtmPtr = 0;       
    IMrDTMQueryPtr           fullResLinearQueryPtr;
    IMrDTMQueryParametersPtr mrdtmQueryParamPtr;

    //Query the linears 
    
    //Get the query interfaces
    fullResLinearQueryPtr = mrDTMPtr->GetQueryInterface(Bentley::MrDTM::DTM_QUERY_FULL_RESOLUTION, Bentley::MrDTM::DTM_QUERY_DATA_LINEAR);
    
    if (fullResLinearQueryPtr != 0)
        {              
        IMrDTMFullResolutionLinearQueryParamsPtr mrDtmFullResLinearParametersPtr(IMrDTMFullResolutionLinearQueryParams::CreateParams());                          
        mrDtmFullResLinearParametersPtr->SetTriangulationState(false);                
        mrDtmFullResLinearParametersPtr->SetUseDecimation(false);             
        mrDtmFullResLinearParametersPtr->SetCutLinears(false);

        mrdtmQueryParamPtr = mrDtmFullResLinearParametersPtr;
        
        if (fullResLinearQueryPtr->Query(dtmPtr, &regionPointsInStorageCS[0], (int)regionPointsInStorageCS.size(), mrdtmQueryParamPtr) != IMrDTMQuery::S_SUCCESS)
            return ERROR;           
                                                                    
        *nbPointsForLinearFeatures = (unsigned int)dtmPtr->GetPointCount();
        }   
                          
    dtmPtr = 0;                    
    
    IMrDTMQueryPtr fixResPointQueryPtr = mrDTMPtr->GetQueryInterface(Bentley::MrDTM::DTM_QUERY_FIX_RESOLUTION_VIEW, Bentley::MrDTM::DTM_QUERY_DATA_POINT);                  

    assert(fixResPointQueryPtr != 0 || mrDTMPtr->GetPointCount() == 0);

    if (fixResPointQueryPtr != 0)
        {        
        // Find the last res that have the less than 200000 points
        Bentley::MrDTM::IMrDTMFixResolutionIndexQueryParamsPtr mrDtmFixResqueryParamsPtr(Bentley::MrDTM::IMrDTMFixResolutionIndexQueryParams::CreateParams());                                                           
    
        int  res = 0;
        bool found = false;
        DTMPtr singleResolutionViewDtmPtr = 0;                         
        
        for (res=0; res<mrDTMPtr->GetNbResolutions(Bentley::MrDTM::DTM_QUERY_DATA_POINT); res++) 
            {                
            mrDtmFixResqueryParamsPtr->SetResolutionIndex(res);  
            mrDtmFixResqueryParamsPtr->SetTriangulationState(false);                
            
            mrdtmQueryParamPtr = mrDtmFixResqueryParamsPtr.get();
                
            singleResolutionViewDtmPtr = 0;                             
            if (fixResPointQueryPtr->Query(singleResolutionViewDtmPtr, 0, 0, mrdtmQueryParamPtr) != IMrDTMQuery::S_SUCCESS)
                return ERROR;
        
            if (singleResolutionViewDtmPtr->GetPointCount() < 200000)
                {
                found = true;            
                break;
                }
            }                                
         
        if (!found) 
            return ERROR;
                
        IMrDTMPtr      singleResMrDTMViewPtr = IMrDTMPtr((IMrDTM*)singleResolutionViewDtmPtr.get());
        IMrDTMQueryPtr fullResQueryPtr;
       
        fullResQueryPtr = singleResMrDTMViewPtr->GetQueryInterface(Bentley::MrDTM::DTM_QUERY_FULL_RESOLUTION, Bentley::MrDTM::DTM_QUERY_DATA_POINT);                
        
        assert(fullResQueryPtr != 0);

        IMrDTMFullResolutionQueryParamsPtr mrDtmFullResQueryParam(IMrDTMFullResolutionQueryParams::CreateParams());
        mrDtmFullResQueryParam->SetTriangulationState(false);                
        mrDtmFullResQueryParam->SetReturnAllPtsForLowestLevel(false);
                
        mrdtmQueryParamPtr = mrDtmFullResQueryParam.get();

        if (fullResQueryPtr->Query(dtmPtr, &regionPointsInStorageCS[0], (int)regionPointsInStorageCS.size(), mrdtmQueryParamPtr) != IMrDTMQuery::S_SUCCESS)     
            return ERROR;
                     
        *nbPointsForPointFeatures = (unsigned int)dtmPtr->GetPointCount() * (int)pow(4.0, mrDTMPtr->GetNbResolutions(Bentley::MrDTM::DTM_QUERY_DATA_POINT) - res - 1);
        }
                                                   
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 11/2015
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt QuerySubResolutionData(DTMPtr&         dtmPtr, 
                                 const DPoint3d* regionPts, 
                                 size_t          nbPts, 
                                 IMrDTMPtr       mrDTMPtr, 
                                 size_t&         remainingPointCount, 
                                 double          decimationFactorForPointFeatures, 
                                 unsigned int    maximumNbLinearFeaturePoints)
    {                                            
    StatusInt                status;
    IMrDTMQueryPtr           fullResLinearQueryPtr;
    IMrDTMQueryParametersPtr queryParameterPtr;
     
    //Query the linears     
    fullResLinearQueryPtr = mrDTMPtr->GetQueryInterface(Bentley::MrDTM::DTM_QUERY_FULL_RESOLUTION, Bentley::MrDTM::DTM_QUERY_DATA_LINEAR);

    IMrDTMQueryPtr fixResPointQueryPtr(mrDTMPtr->GetQueryInterface(Bentley::MrDTM::DTM_QUERY_FIX_RESOLUTION_VIEW, Bentley::MrDTM::DTM_QUERY_DATA_POINT));    

    if (fullResLinearQueryPtr != 0)
        {                                    
        IMrDTMFullResolutionLinearQueryParamsPtr mrDtmFullResLinearParametersPtr(IMrDTMFullResolutionLinearQueryParams::CreateParams());
        
        if (fixResPointQueryPtr != 0)
            {                        
            mrDtmFullResLinearParametersPtr->SetTriangulationState(false);                
            }
        
        //Currently the high quality display mode is not influencing the maximum number of linear 
        //points use to obtain a single resolution representation of the MrDTM.        
        unsigned int maxNbLinearFeaturePoints = max(maximumNbLinearFeaturePoints, (unsigned int)11);        

        mrDtmFullResLinearParametersPtr->SetMaximumNumberOfPointsForLinear(maxNbLinearFeaturePoints);
        mrDtmFullResLinearParametersPtr->SetUseDecimation(true);             
       
        mrDtmFullResLinearParametersPtr->SetCutLinears(true);
                
        size_t nbPointsBefore;

        if (dtmPtr != 0)
            {
            nbPointsBefore = (size_t)dtmPtr->GetPointCount();
            }
        else
            {
            nbPointsBefore = 0;
            }                        

        queryParameterPtr = mrDtmFullResLinearParametersPtr;

        if ((status = fullResLinearQueryPtr->Query(dtmPtr, regionPts, (int)nbPts, queryParameterPtr)) != SUCCESS)      
            {                        
            return status;           
            }
                
        size_t nbPointsDuringQuery = (size_t)dtmPtr->GetPointCount() - nbPointsBefore;

        if (remainingPointCount >= nbPointsDuringQuery)
            {
            remainingPointCount -= nbPointsDuringQuery;
            }
        else
            {                        
            remainingPointCount = 0;            
            }
        }

    //Query the points       
    if (fixResPointQueryPtr != 0)
        {
        //Query the view
        DTMPtr singleResolutionViewDtmPtr = 0;             

        IMrDTMFixResolutionIndexQueryParamsPtr mrDtmFixResqueryParamsPtr(IMrDTMFixResolutionIndexQueryParams::CreateParams());                                                           

        if (decimationFactorForPointFeatures == 1.0)
            {
            mrDtmFixResqueryParamsPtr->SetResolutionIndex(mrDTMPtr->GetNbResolutions(Bentley::MrDTM::DTM_QUERY_DATA_POINT)-1);                      
            }
        else
            {
            int nbResolutions = mrDTMPtr->GetNbResolutions(Bentley::MrDTM::DTM_QUERY_DATA_POINT);
            assert(nbResolutions >= 1);
            assert(decimationFactorForPointFeatures <= 1.0);
                        
            double resolutionInd = nbResolutions - 1 - log(1.0 / decimationFactorForPointFeatures) / log(4.0);
            
            resolutionInd = min(max(0.0, floor(resolutionInd)), (double)nbResolutions - 1);
            
            mrDtmFixResqueryParamsPtr->SetResolutionIndex((int)resolutionInd);                      
            }        
        
        mrDtmFixResqueryParamsPtr->SetTriangulationState(false);                
        
        queryParameterPtr = mrDtmFixResqueryParamsPtr;
                    
        status = fixResPointQueryPtr->Query(singleResolutionViewDtmPtr, 0, 0, queryParameterPtr);

        assert(singleResolutionViewDtmPtr != 0);

        IMrDTMPtr      singleResMrDTMViewPtr = IMrDTMPtr((IMrDTM*)singleResolutionViewDtmPtr.get());
        IMrDTMQueryPtr fullResQueryPtr;
                
        fullResQueryPtr = singleResMrDTMViewPtr->GetQueryInterface(Bentley::MrDTM::DTM_QUERY_FULL_RESOLUTION, Bentley::MrDTM::DTM_QUERY_DATA_POINT);                                
        assert(fullResQueryPtr != 0);

        IMrDTMFullResolutionQueryParamsPtr mrDtmFullResQueryParam(IMrDTMFullResolutionQueryParams::CreateParams());
       
        mrDtmFullResQueryParam->SetTriangulationState(false);                              
        mrDtmFullResQueryParam->SetMaximumNumberOfPoints(remainingPointCount);
        mrDtmFullResQueryParam->SetReturnAllPtsForLowestLevel(false);
                            
        size_t nbPointsBefore;

        if (dtmPtr != 0)
            {
            nbPointsBefore = (size_t)dtmPtr->GetPointCount();
            }
        else
            {
            nbPointsBefore = 0;
            }                        
        
        queryParameterPtr = mrDtmFullResQueryParam;

        if ((status = fullResQueryPtr->Query(dtmPtr, regionPts, (int)nbPts, queryParameterPtr)) != SUCCESS)                         
            {            
            return status;                  
            }

        size_t nbPointsDuringQuery = (size_t)dtmPtr->GetPointCount() - nbPointsBefore;

        if (remainingPointCount >= nbPointsDuringQuery)
            {
            remainingPointCount -= nbPointsDuringQuery;
            }
        else
            {            
            assert(!"Should have failed during the query.");
            remainingPointCount = 0;            
            }
        }
            
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 11/2015
+---------------+---------------+---------------+---------------+---------------+------*/    
void GetImportRange(DRange3d& importRange, BeXmlNodeP pRootNode)
    {    
    double value;
    BeXmlStatus status = pRootNode->GetAttributeDoubleValue(value, "xmax");

    if (status != BEXML_Success)
        {
        importRange.high.x = numeric_limits<double>::max();
        }
    else
        {
        importRange.high.x = value;
        }

    status = pRootNode->GetAttributeDoubleValue(value, "xmin");

    if (status != BEXML_Success)
        {
        importRange.low.x = -numeric_limits<double>::max();
        }
    else
        {
        importRange.low.x = value;
        }

    status = pRootNode->GetAttributeDoubleValue(value, "ymax");

    if (status != BEXML_Success)
        {
        importRange.high.y = numeric_limits<double>::max();
        }
    else
        {
        importRange.high.y = value;
        }

    status = pRootNode->GetAttributeDoubleValue(value, "ymin");

    if (status != BEXML_Success)
        {
        importRange.low.y = -numeric_limits<double>::max();
        }
    else
        {
        importRange.low.y = value;
        }

    importRange.low.z = 0;
    importRange.high.z = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 11/2015
+---------------+---------------+---------------+---------------+---------------+------*/    
int QueryStmFromBestResolution(RefCountedPtr<BcDTM>&        singleResolutionDtm,
                               IMrDTMPtr&                   mrdtmPtr, 
                               const std::vector<DPoint3d>& regionPoints,                               
                               UInt64                       maximumNbPoints)
    {    
    DTMPtr        dtmPtr = 0;
    const UInt64 MAX_POINT_COUNT = maximumNbPoints;
    
    size_t  remainingPointCount = static_cast<size_t>(MAX_POINT_COUNT);

    double decimationFactorRequested; 

    vector<__int64> approximateNbPointsForLinearFeaturesList;
    
    size_t approximateTotalNbPoints = 0;
     
#if 0 
    for (ElementAgenda::const_iterator elemIter = agenda.begin(); elemIter != agenda.end(); ++elemIter)
        {            
        unsigned int approximateNbPointsForPointFeatures;
        unsigned int approximateNbPointsForLinearFeatures;            
                               
        StatusInt status = GetApproximationNbPtsNeedToExtract(*elemIter, regionPoints, &approximateNbPointsForPointFeatures, &approximateNbPointsForLinearFeatures);

        if (status != SUCCESS)
            {
            //MST TBD - What should we do?
            approximateNbPointsForLinearFeaturesList.push_back(0);
            }
        else
            {                
            approximateTotalNbPoints += approximateNbPointsForPointFeatures + approximateNbPointsForLinearFeatures;
            approximateNbPointsForLinearFeaturesList.push_back(approximateNbPointsForLinearFeatures);
            }            
        }    
#endif

    unsigned int approximateNbPointsForPointFeatures = 0;
    unsigned int approximateNbPointsForLinearFeatures = 0;          

    StatusInt status = GetApproximationNbPtsNeedToExtract(mrdtmPtr,                                                                                                      
                                                          regionPoints,                          
                                                          &approximateNbPointsForPointFeatures, 
                                                          &approximateNbPointsForLinearFeatures);

    assert(status == SUCCESS);

    approximateTotalNbPoints = approximateNbPointsForPointFeatures + approximateNbPointsForLinearFeatures;

    if (remainingPointCount < approximateTotalNbPoints)
        {
        decimationFactorRequested = (double)remainingPointCount / approximateTotalNbPoints;
        }
    else
        {
        decimationFactorRequested = 1.0;
        }        
     
    StatusInt updateStatus;

    int nbPointsForLinearFeatureInd = 0;    
    int maxNbTries = 20;

    int tryInd = 0;    

    double       decimationFactorForPointFeatures = decimationFactorRequested;   
    unsigned int maximumNbLinearFeaturePoints;
     
    for (; tryInd < maxNbTries; tryInd++)
        {
        updateStatus = IMrDTMQuery::S_SUCCESS;
        nbPointsForLinearFeatureInd = 0;        
                
        maximumNbLinearFeaturePoints = (unsigned int)(approximateNbPointsForLinearFeatures * decimationFactorRequested);
                                
        updateStatus = QuerySubResolutionData(dtmPtr, 
                                              &regionPoints[0], 
                                              regionPoints.size(), 
                                              mrdtmPtr, 
                                              remainingPointCount,
                                              decimationFactorForPointFeatures, 
                                              maximumNbLinearFeaturePoints);
                                                                                                                                 
        if (BSISUCCESS != updateStatus)
            {            
            if (IMrDTMQuery::S_NBPTSEXCEEDMAX == updateStatus)
                {                                
                decimationFactorRequested /= 4;
                remainingPointCount = static_cast<size_t>(MAX_POINT_COUNT);                
                }                        
            else
                {
                break;
                }
            }    
        else
            {
            break;
            }
        }
    
    if (tryInd == maxNbTries)
        {
        return ERROR;   
        }
            
    if (dtmPtr == 0 || dtmPtr->GetPointCount() <= 0)
        return ERROR;
                
    singleResolutionDtm = dtmPtr->GetBcDTM();
    return SUCCESS;
    }      

    BentleyStatus ChooseCorrectSubResolution(RefCountedPtr<BcDTM>& singleResolutionDtm, const DRange3d& importRange, uint32_t maxNbPointsToImport)
        {
        IMrDTMPtr mrDtmPtr(IMrDTM::GetFor(L"D:\\MyDoc\\CC - Iteration 13\\Import terrain STM\\log\\temp.stm", true, true));

        if (mrDtmPtr == 0)
            return ERROR;
       
        std::vector<DPoint3d> regionPoints(5);
                
        regionPoints[0].x = importRange.low.x;
        regionPoints[0].y = importRange.low.y;
        regionPoints[0].z = importRange.low.z;

        regionPoints[1].x = importRange.high.x;
        regionPoints[1].y = importRange.low.y;
        regionPoints[1].z = importRange.low.z;

        regionPoints[2].x = importRange.high.x;
        regionPoints[2].y = importRange.high.y;
        regionPoints[2].z = importRange.low.z;

        regionPoints[3].x = importRange.low.x;
        regionPoints[3].y = importRange.high.y;
        regionPoints[3].z = importRange.low.z;

        regionPoints[4].x = importRange.low.x;
        regionPoints[4].y = importRange.low.y;
        regionPoints[4].z = importRange.low.z;
                        
        int statusInt = QueryStmFromBestResolution(singleResolutionDtm, mrDtmPtr, regionPoints, maxNbPointsToImport);

        if (statusInt != SUCCESS)
            return ERROR;        
        
        mrDtmPtr = 0;
        statusInt = _wremove(L"D:\\MyDoc\\CC - Iteration 13\\Import terrain STM\\log\\temp.stm");
        assert(statusInt == 0);
                       
        if (statusInt != SUCCESS || singleResolutionDtm == 0)
            return ERROR;

        return SUCCESS;
        }

    #define MAX_SPOTS 100000

    BentleyStatus StreamDataToApplication(RefCountedPtr<BcDTM>& singleResolutionDtm)
        {        
        DTMStatusInt status = singleResolutionDtm->BrowseFeatures(DTMFeatureType::Spots,
                                                                  MAX_SPOTS,
                                                                  0,
                                                                  WritePointsCallback);

        if (status != SUCCESS)
            return ERROR;

        //Linear features currently supported by the importer
        static const DTMFeatureType featureTypes[] =
            {
            DTMFeatureType::Breakline,
            // Removed as it was automatically transformed to DTMFeatureType::TinHull when tin was triangulated. Tin types couldn't be added to
            // not triangulated dtm.
            //DTMFeatureType::Hull,
            DTMFeatureType::ContourLine,
            DTMFeatureType::Void,
            DTMFeatureType::BreakVoid,
            DTMFeatureType::Island,
            DTMFeatureType::Hole,
            DTMFeatureType::Polygon,
            DTMFeatureType::ZeroSlopePolygon,
            DTMFeatureType::SoftBreakline,
            // Removed as it was not supported for now and was causing a memory leak.
            //DTMFeatureType::GraphicBreak
            };

          
        for (auto& featureType : featureTypes)
            {
            DTMStatusInt status = singleResolutionDtm->BrowseFeatures(DTMFeatureType::Spots,
                                                                      (long)featureType,
                                                                      0,
                                                                      WriteFeatureCallback);

            if (status != SUCCESS)
                return ERROR;
            }
                    
        return SUCCESS;
        }

    BentleyStatus ScalableMeshSDKexe::ParseImportDefinition(BeXmlNodeP pRootNode)
        {
               
//NEEDS_WORK_MST Remove
        BeXmlStatus status;

       // s_pPointResultFile = fopen("D:\\MyDoc\\CC - Iteration 6\\YIIDataset\\saltLakeWithCC.xyz", "w+");                

        Bentley::ScalableMesh::IScalableMeshSourceImporterPtr importerPtr(Bentley::ScalableMesh::IScalableMeshSourceImporter::Create());
        s_pipe = m_pipe;
        importerPtr->SetFeatureCallback(StreamFeatureCallback);
        importerPtr->SetPointsCallback(StreamPointsCallback);

        if (importerPtr == 0)
            {
            printf("ERROR : cannot create importer\r\n");
            return ERROR;
            }
        else
            {
            WString gcsKeyName;

            status = pRootNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

            if (status == BEXML_Success)
                {
                BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));
                StatusInt status = importerPtr->SetBaseGCS(baseGCSPtr);
                assert(status == SUCCESS);
                }

            uint32_t maximumNbOfPoints;
            status = pRootNode->GetAttributeUInt32Value(maximumNbOfPoints, "maxNbPointsToImport");
            
            if (status != BEXML_Success)
                {
                maximumNbOfPoints = 5000000;
                }
            
            GetImportRange(s_importRange, pRootNode);
                    
            if (ParseSourceSubNodes(importerPtr->EditSources(), pRootNode) == true)
                {
                std::thread workingThread;

                workingThread = std::thread(&ImportThread, DgnPlatformLib::QueryHost(), importerPtr);         
                
                IMrDTMCreatorPtr mrdtmCreatorPtr(IMrDTMCreator::GetFor(L"D:\\MyDoc\\CC - Iteration 13\\Import terrain STM\\log\\temp.stm"));

                Bentley::MrDTM::IDTMSourcePtr stmSourcePtr(CreateSourceFor(L"D:\\MyDoc\\CC - Iteration 13\\Import terrain STM\\dummy.stmstream",
                                                                           Bentley::MrDTM::DTM_SOURCE_DATA_DTM));

                mrdtmCreatorPtr->EditSources().Add(stmSourcePtr);

                StatusInt status = mrdtmCreatorPtr->Create();

                if (status != SUCCESS)
                    return ERROR;
                
                if (workingThread.joinable())
                    workingThread.join();

                mrdtmCreatorPtr = 0;

                RefCountedPtr<BcDTM> singleResolutionDtm;

                status = ChooseCorrectSubResolution(singleResolutionDtm, s_importRange, maximumNbOfPoints);

                if (status != SUCCESS)
                    return ERROR;
               
                status = StreamDataToApplication(singleResolutionDtm);

                if (status != SUCCESS)
                    return ERROR;
                }
            }        

        return SUCCESS;
        }



#if 0 
    //NEEDS_WORK_MST To Remove
    BentleyStatus ScalableMeshSDKexe::ParseImportDefinitionNew(BeXmlNodeP pRootNode)
        {
        BeXmlStatus status;

       // s_pPointResultFile = fopen("D:\\MyDoc\\CC - Iteration 6\\YIIDataset\\saltLakeWithCC.xyz", "w+");                        
        IMrDTMCreatorPtr mrdtmCreatorPtr(IMrDTMCreator::GetFor(L"D:\\MyDoc\\CC - Iteration 13\\Import terrain STM\\log\\temp.stm"));

        //Bentley::ScalableMesh::IScalableMeshSourceImporterPtr importerPtr(Bentley::ScalableMesh::IScalableMeshSourceImporter::Create());
        s_pipe = m_pipe;
        /*
        importerPtr->SetFeatureCallback(WriteFeatureCallback);
        importerPtr->SetPointsCallback(WritePointsCallback);
        */
        
        if (mrdtmCreatorPtr == 0)
            {
            printf("ERROR : cannot create importer\r\n");
            return ERROR;
            }
        
        
        WString gcsKeyName;            
        UInt64  maxNbPointsToImport;
        
        status = pRootNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

        if (status == BEXML_Success)
            {
            BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));
            StatusInt status = mrdtmCreatorPtr->SetBaseGCS(baseGCSPtr);
            assert(status == SUCCESS);
            }

        status = pRootNode->GetAttributeUInt64Value(maxNbPointsToImport, "maxNbPointsToImport");

        if (status != BEXML_Success)
            {            
            maxNbPointsToImport = numeric_limits<UInt64>::max();
            }
        /*
        status = pRootNode->GetAttributeUInt64Value(maxNbPointsToImport, "");

        if (status != BEXML_Success)
            {            
            maxNbPointsToImport = numeric_limits<UInt64>::max();
            }
            */

        if (ParseSourceSubNodes(mrdtmCreatorPtr->EditSources(), pRootNode) == false)
            {                
            return ERROR;
            }

        if (mrdtmCreatorPtr->Create() != SUCCESS)
            {
            return ERROR;
            }                

        mrdtmCreatorPtr = 0;

        IMrDTMPtr mrDtmPtr(IMrDTM::GetFor(L"D:\\MyDoc\\CC - Iteration 13\\Import terrain STM\\log\\temp.stm", true, true));

        if (mrDtmPtr == 0)
            return ERROR;

        RefCountedPtr<BcDTM> singleResolutionDtm;

        DRange3d range;
        std::vector<DPoint3d> regionPoints(8);

        mrDtmPtr->GetRange(range);
        range.Get8Corners(&regionPoints[0]);

        regionPoints.resize(4);
                
        int statusInt = QueryStmFromBestResolution(singleResolutionDtm, mrDtmPtr, regionPoints, maxNbPointsToImport);

        assert(statusInt == SUCCESS);

        mrDtmPtr = 0;
        statusInt = _wremove(L"D:\\MyDoc\\CC - Iteration 13\\Import terrain STM\\log\\temp.stm");
        assert(status == 0);
                       
        if (statusInt != SUCCESS || singleResolutionDtm == 0)
            return ERROR;
                        
        return SUCCESS;
        }
#endif

    BentleyStatus ScalableMeshSDKexe::ParseImportDefinitionNew(BeXmlNodeP pRootNode)
        {
        BeXmlStatus status;

       // s_pPointResultFile = fopen("D:\\MyDoc\\CC - Iteration 6\\YIIDataset\\saltLakeWithCC.xyz", "w+");                        
        IMrDTMCreatorPtr mrdtmCreatorPtr(IMrDTMCreator::GetFor(L"D:\\MyDoc\\CC - Iteration 13\\Import terrain STM\\log\\temp.stm"));

        //Bentley::ScalableMesh::IScalableMeshSourceImporterPtr importerPtr(Bentley::ScalableMesh::IScalableMeshSourceImporter::Create());
        s_pipe = m_pipe;
        /*
        importerPtr->SetFeatureCallback(WriteFeatureCallback);
        importerPtr->SetPointsCallback(WritePointsCallback);
        */
        
        if (mrdtmCreatorPtr == 0)
            {
            printf("ERROR : cannot create importer\r\n");
            return ERROR;
            }
        
        
        WString gcsKeyName;            
        UInt64  maxNbPointsToImport;
        
        status = pRootNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

        if (status == BEXML_Success)
            {
            BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));
            StatusInt status = mrdtmCreatorPtr->SetBaseGCS(baseGCSPtr);
            assert(status == SUCCESS);
            }

        status = pRootNode->GetAttributeUInt64Value(maxNbPointsToImport, "maxNbPointsToImport");

        if (status != BEXML_Success)
            {            
            maxNbPointsToImport = numeric_limits<UInt64>::max();
            }
        /*
        status = pRootNode->GetAttributeUInt64Value(maxNbPointsToImport, "");

        if (status != BEXML_Success)
            {            
            maxNbPointsToImport = numeric_limits<UInt64>::max();
            }
            */

        if (ParseSourceSubNodes(mrdtmCreatorPtr->EditSources(), pRootNode) == false)
            {                
            return ERROR;
            }

        if (mrdtmCreatorPtr->Create() != SUCCESS)
            {
            return ERROR;
            }                

        mrdtmCreatorPtr = 0;

        IMrDTMPtr mrDtmPtr(IMrDTM::GetFor(L"D:\\MyDoc\\CC - Iteration 13\\Import terrain STM\\log\\temp.stm", true, true));

        if (mrDtmPtr == 0)
            return ERROR;

        RefCountedPtr<BcDTM> singleResolutionDtm;

        DRange3d range;
        std::vector<DPoint3d> regionPoints(8);

        mrDtmPtr->GetRange(range);
        range.Get8Corners(&regionPoints[0]);

        regionPoints.resize(4);
                
        int statusInt = QueryStmFromBestResolution(singleResolutionDtm, mrDtmPtr, regionPoints, maxNbPointsToImport);

        assert(statusInt == SUCCESS);

        mrDtmPtr = 0;
        statusInt = _wremove(L"D:\\MyDoc\\CC - Iteration 13\\Import terrain STM\\log\\temp.stm");
        assert(status == 0);
                       
        if (statusInt != SUCCESS || singleResolutionDtm == 0)
            return ERROR;
                        
        return SUCCESS;
        }

    int ScalableMeshSDKexe::PrintUsage(WCharCP programName)
        {
        WString exeName = BeFileName::GetFileNameAndExtension(programName);

        fwprintf(stderr,
                 L"\n\
                Imports terrain data from various formats, including DgnV8.\n\
                 \n Usage: \n\
                %ls import -i|--input= -o|--output= \n\
                --input=                (required)  Configuration file listing the input files and options. \n\
                \n\
                --output=               (required)  Named pipe where data is to be transferred.\n\
                ", programName, programName);

        return 1;
        }

    /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
    WString ScalableMeshSDKexe::GetArgValueW(WCharCP arg)
        {
        WString argValue(arg);
        argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
        argValue.Trim(L"\"");
        argValue.Trim();
        return argValue;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      07/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String ScalableMeshSDKexe::GetArgValue(WCharCP arg)
        {
        return Utf8String(GetArgValueW(arg));
        }

    int ScalableMeshSDKexe::ParseCommandLine(int argc, WCharP argv[])
        {
        if (argc < 3)
            return PrintUsage(argv[0]);
        bool isImportMode = false;
        if (0 == wcscmp(argv[1], L"import"))
            {
            isImportMode = true;
            }
        if (!isImportMode)
            {
            fwprintf(stderr, L"Unrecognized command: %ls\nTry 'import'\n.", argv[1]);
            return PrintUsage(argv[0]);
            }
        for (int iArg = 2; iArg < argc; ++iArg)
            {
            if (argv[iArg] == wcsstr(argv[iArg], L"--input=") || argv[iArg] == wcsstr(argv[iArg], L"-i="))
                {
                BeFileName::FixPathName(m_inputFileName, GetArgValueW(argv[iArg]).c_str());
                if (BeFileName::IsDirectory(m_inputFileName.c_str()))
                    return PrintUsage(argv[0]);

                continue;
                }

            if (argv[iArg] == wcsstr(argv[iArg], L"--output=") || argv[iArg] == wcsstr(argv[iArg], L"-o="))
                {
                m_outputName.SetName(GetArgValueW(argv[iArg]).c_str());
                continue;
                }

            fwprintf(stderr, L"Unrecognized command line option: %ls\n", argv[iArg]);
            return PrintUsage(argv[0]);
            }
        return SUCCESS;
        }

    void ScalableMeshSDKexe::Start()
        {
        Import();        
        //Terminate(true);
        }
    
    void ScalableMeshSDKexe::Import()
        {
        BeXmlStatus status;
        WString     errorMsg;

        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(status, m_inputFileName, &errorMsg);

        if (pXmlDom == 0)
            {
            wprintf(L"ERROR : Cannot open input file (%s)", errorMsg.c_str());
            return;
            }

        BeXmlNodeP pRootNode(pXmlDom->GetRootElement());
                
        ParseImportDefinition(pRootNode);
                
        CloseHandle(m_pipe);
        }

   }


int wmain(int argc, wchar_t* argv[])
    {
    ScalableMeshSDKexe::ScalableMeshSDKexe app;
    if (SUCCESS != app.ParseCommandLine(argc, argv))
        return 1;

    if (app.m_outputName.empty())
        {
        fwprintf(stderr, L"No output directory specified\n");
        return app.PrintUsage(argv[0]);
        }
    app.Initialize(argc, argv);
    app.Start();

    ScalableMeshSDKexe::CloseSDK();

    return 0;
    }