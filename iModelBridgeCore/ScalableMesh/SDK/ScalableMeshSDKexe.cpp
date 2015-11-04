#include "Initialize.h"
#include "ScalableMeshSDKexe.h"
#include "ScalableMeshSDKexeImporter.h"

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
        if (!ConnectNamedPipe(m_pipe, NULL)) fwprintf(stderr, L"No client connected\n");
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

    bool WritePointsCallback(const DPoint3d* points, size_t nbOfPoints, bool arePoints3d)
        {   
        TransmittedPointsHeader pointHeader((unsigned int)nbOfPoints, arePoints3d);
                
        DWORD  numBytesWritten;        
        WriteFile(s_pipe,
                  &pointHeader,
                  (DWORD)sizeof(pointHeader),
                  &numBytesWritten,
                  NULL 
                  );

        WriteFile(s_pipe,
                  points,
                  (DWORD)(sizeof(DPoint3d) * nbOfPoints),
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

    bool WriteFeatureCallback(const DPoint3d* featurePoints, size_t nbOfFeaturesPoints, DTMFeatureType featureType, bool isFeature3d)
        {        
        TransmittedFeatureHeader featureHeader((unsigned int)nbOfFeaturesPoints, (short)featureType, isFeature3d);

        DWORD numBytesWritten;        
        WriteFile(s_pipe,
                  &featureHeader,
                  (DWORD)sizeof(featureHeader),
                  &numBytesWritten,
                  NULL 
                  );
        
        WriteFile(s_pipe,
                  featurePoints,
                  (DWORD)(sizeof(DPoint3d) * nbOfFeaturesPoints),
                  &numBytesWritten,
                  NULL 
                  );

        return true;
        }

    BentleyStatus ScalableMeshSDKexe::ParseImportDefinition(BeXmlNodeP pTestNode)
        {
        BeXmlStatus status;

       // s_pPointResultFile = fopen("D:\\MyDoc\\CC - Iteration 6\\YIIDataset\\saltLakeWithCC.xyz", "w+");                

        Bentley::ScalableMesh::IScalableMeshSourceImporterPtr importerPtr(Bentley::ScalableMesh::IScalableMeshSourceImporter::Create());
        s_pipe = m_pipe;
        importerPtr->SetFeatureCallback(WriteFeatureCallback);
        importerPtr->SetPointsCallback(WritePointsCallback);

        if (importerPtr == 0)
            {
            printf("ERROR : cannot create importer\r\n");
            return ERROR;
            }
        else
            {
            WString gcsKeyName;

            status = pTestNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

            if (status == BEXML_Success)
                {
                BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));
                StatusInt status = importerPtr->SetBaseGCS(baseGCSPtr);
                assert(status == SUCCESS);
                }

            if (ParseSourceSubNodes(importerPtr->EditSources(), pTestNode) == true)
                {

                return importerPtr->Import() ? SUCCESS : ERROR;
                }
            }

        //fclose(s_pPointResultFile);

        return SUCCESS;
        }

#if 0 
int ConvertMrDTMtoFullResDTM(RefCountedPtr<IBcDTM>&        singleResolutionDtm,
                             const std::vector<DPoint3d>&  regionPoints,
                             bool                          applyClip, 
                             UInt64                        maximumNbPoints)
    {    
    DTMPtr        dtmPtr = 0;
    const UInt64 MAX_POINT_COUNT = maximumNbPoints;
    
    size_t  remainingPointCount = static_cast<size_t>(MAX_POINT_COUNT);

    double decimationFactorRequested; 

    vector<__int64> approximateNbPointsForLinearFeaturesList;

    if (useFullResolution == false)  
        {
        __int64 approximateTotalNbPoints = 0;
                               
        for (ElementAgenda::const_iterator elemIter = agenda.begin(); elemIter != agenda.end(); ++elemIter)
            {            
            unsigned int approximateNbPointsForPointFeatures;
            unsigned int approximateNbPointsForLinearFeatures;            
                           
            //MST TBD - Check what occurs with this param : applyClip
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
        

        if (remainingPointCount < approximateTotalNbPoints)
            {
            decimationFactorRequested = (double)remainingPointCount / approximateTotalNbPoints;
            }
        else
            {
            decimationFactorRequested = 1.0;
            }        
        }
    else
        {
        decimationFactorRequested = DECIMATION_FACTOR_REQUESTED_FULL_RESOLUTION;
        }   
    
    StatusInt updateStatus;


    int nbPointsForLinearFeatureInd = 0;    
    int maxNbTries;

    if (useFullResolution == false)
        {
        maxNbTries = 20;
        }
    else
        {
        maxNbTries = 1;
        }

    int tryInd = 0;    

    double       decimationFactorForPointFeatures = decimationFactorRequested;   
    unsigned int maximumNbLinearFeaturePoints;
     
    for (; tryInd < maxNbTries; tryInd++)
        {
        updateStatus = IMrDTMQuery::S_SUCCESS;
        nbPointsForLinearFeatureInd = 0;        

    for (ElementAgenda::const_iterator elemIter = agenda.begin(); elemIter != agenda.end(); ++elemIter)
        {      
        if (decimationFactorRequested == DECIMATION_FACTOR_REQUESTED_FULL_RESOLUTION)
            {
            maximumNbLinearFeaturePoints = NB_LINEAR_FEATURE_POINTS_REQUESTED_FULL_RESOLUTION;            
            }
        else
            {
            maximumNbLinearFeaturePoints = (unsigned int)(approximateNbPointsForLinearFeaturesList[nbPointsForLinearFeatureInd] * decimationFactorRequested);
            }        

        if (IsProcessTerminatedByUser())
            return Extract_ABORT;

        ElemHandleCP eh = &*elemIter;                            

        Transform fromModelRefToActiveTransform;
        Transform invertFromModelRefToActiveTransform;

        GetFromModelRefToActiveTransform(fromModelRefToActiveTransform, eh->GetModelRef());

        BoolInt invertSuccess = bsiTransform_invertTransform(&invertFromModelRefToActiveTransform, &fromModelRefToActiveTransform);
        assert(invertSuccess != 0);
                    
        Transform trsf;
        Transform invertTrsf;
        DTMElementHandlerManager::GetStorageToUORTransformation(*eh, trsf);

        invertSuccess = bsiTransform_invertTransform(&invertTrsf, &trsf);
        assert(invertSuccess != 0);
        
        Transform activeToStorageTrsf;
        bsiTransform_multiplyTransformTransform(&activeToStorageTrsf, &invertTrsf, &invertFromModelRefToActiveTransform);
        
        std::vector<DPoint3d> regionPointsInStorageCS(regionPoints);
        bsiTransform_multiplyDPoint3dArrayInPlace(&activeToStorageTrsf, &regionPointsInStorageCS[0], regionPointsInStorageCS.size());           

        s_queryProcessTerminatedByUser = false;
       
        updateStatus = updateDTMPtrWithSTM(dtmPtr, 
                                           &regionPointsInStorageCS[0], regionPointsInStorageCS.size(), 
                                           *eh, 
                                           remainingPointCount, 
                                           applyClip,
                                           DTMOutputUnit, 
                                           decimationFactorForPointFeatures, 
                                           maximumNbLinearFeaturePoints);       

        if (useFullResolution == false)
            {            
            nbPointsForLinearFeatureInd++;
            }

        if ((BSISUCCESS != updateStatus) || (s_queryProcessTerminatedByUser == true))
            {
            if (IMrDTMQuery::S_NBPTSEXCEEDMAX == updateStatus)
                {
                break; 
                }
            else if (s_queryProcessTerminatedByUser)
                {   
                break;
                }
            else
                {
                break;
                }
            } 
        }

        if (BSISUCCESS != updateStatus)
            {            
            if (IMrDTMQuery::S_NBPTSEXCEEDMAX == updateStatus)
                {
                if (useFullResolution == false)
                    {   
                    //MST TBD - When there is only with STM containing only points dividing by 4 is more optimal.
                    decimationFactorRequested /= 2;
                    remainingPointCount = static_cast<size_t>(MAX_POINT_COUNT);
                    }
                else
                    {
                    return Extract_NBPTSEXCEEDMAX;
                    }
                }
            else if (s_queryProcessTerminatedByUser)
                {   
                return Extract_ABORT;
                }
            else
                {
                return Extract_ERROR;                 
                }
            } 
        else
            {
            break;
            }
        }

    if (mdlSystem_extendedAbortRequested() || ESC_KEY == mdlSystem_getChar())
        {
        return Extract_ABORT;
        }

    if (tryInd == maxNbTries)
        {
        return Extract_ERROR;   
        }
        
    if (NULL == &*dtmPtr)
        return Extract_SUCCESS; // NTERAY: Should it really be a success??

    if (dtmPtr->GetPointCount() <= 0)
        return Extract_NOPOINTS;

    MrDTMClipContainer clips;
    if (BSISUCCESS != addClipsToClipContainer(clips, agenda, regionPoints, applyClip, DTMOutputUnit))
        return Extract_ERROR;

    if (s_outputTINinMemDTMbeforeTri)
        {            
        dumpDTMInTinFile(dtmPtr->GetIBcDTM(), 
                                     wstring(L"C:\\Users\\Richard.Bois\\Documents\\TRs\\Bug - Wrong triangle 5\\InMemDTMBeforeTri%I64i.tin"), 
                                     &s_outputTINinMemDTMindexBeforeTri);

        s_outputTINinMemDTMindexBeforeTri++;
        }

    s_queryProcessTerminatedByUser = false;

    if (BSISUCCESS != triangulateDTMAndApplyClips(dtmPtr, selectedElement, clips))
        {
        if (s_queryProcessTerminatedByUser == true)
            {
            return Extract_ABORT;
            }
        else
            {
            return Extract_ERROR;
            }
        }

    singleResolutionDtm = dtmPtr->GetIBcDTM();
    return Extract_SUCCESS;
    }      

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 08/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt QuerySubResolutionData(DTMPtr& dtmPtr, const DPoint3d* regionPts, size_t nbPts, IMrDTMPtr mrDTMPtr, 
                                 bool applyTriangulation, size_t& remainingPointCount, vector<ClipInfo> clips, 
                                 bool applyClip, int edgeMethod, double edgeMethodLength, DRange3d drange, MstnGCSP destinationGCS, 
                                 double decimationFactorForPointFeatures, unsigned int maximumNbLinearFeaturePoints)

StatusInt QuerySubResolutionData(DTMPtr& dtmPtr, const DPoint3d* regionPts, size_t nbPts, IMrDTMPtr mrDTMPtr, 
                                 size_t& remainingPointCount, vector<ClipInfo> clips, 
                                 bool applyClip, int edgeMethod, double edgeMethodLength, DRange3d drange, MstnGCSP destinationGCS, 
                                 double decimationFactorForPointFeatures, unsigned int maximumNbLinearFeaturePoints)
    {   
    bool isSet = SetTriangulationTerminationCallback(CheckTriangulationStopCallback);
    assert(true == isSet);
    //Not implemented yet. 
    assert((applyClip == false) || (applyTriangulation == false));
                             
    StatusInt            status;
    IMrDTMQueryPtr fullResLinearQueryPtr;
     
    //Query the linears 
    if ((mrDTMPtr->GetBaseGCS() == 0)|| (destinationGCS == 0))
        {            
        //Get the query interfaces
        fullResLinearQueryPtr = mrDTMPtr->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, DTM_QUERY_DATA_LINEAR);
        }
    else
        {                                                
        fullResLinearQueryPtr = mrDTMPtr->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, DTM_QUERY_DATA_LINEAR,
            Bentley::GeoCoordinates::BaseGCSPtr(destinationGCS), drange);              
        }

    IMrDTMQueryPtr fixResPointQueryPtr(mrDTMPtr->GetQueryInterface(DTM_QUERY_FIX_RESOLUTION_VIEW, DTM_QUERY_DATA_POINT));    

    if (fullResLinearQueryPtr != 0)
        {                                    
        MrDTMFullResolutionLinearQueryParamsPtr mrDtmFullResLinearParametersPtr(MrDTMFullResolutionLinearQueryParams::CreateParams());

        mrDtmFullResLinearParametersPtr->SetEdgeOptionTriangulationParam(edgeMethod);            
        mrDtmFullResLinearParametersPtr->SetMaxSideLengthTriangulationParam(edgeMethodLength);

        if (fixResPointQueryPtr != 0 || !applyTriangulation)
            {                        
            mrDtmFullResLinearParametersPtr->SetTriangulationState(false);                
            }
        
        //Currently the high quality display mode is not influencing the maximum number of linear 
        //points use to obtain a single resolution representation of the MrDTM.
        
        if (maximumNbLinearFeaturePoints == NB_LINEAR_FEATURE_POINTS_REQUESTED_FULL_RESOLUTION)
            {
            unsigned int maxNbLinearFeaturePoints = max(remainingPointCount, 11);        

            mrDtmFullResLinearParametersPtr->SetMaximumNumberOfPointsForLinear(maxNbLinearFeaturePoints);
            mrDtmFullResLinearParametersPtr->SetUseDecimation(false);             
            }
        else
            {
            unsigned int maxNbLinearFeaturePoints = max(maximumNbLinearFeaturePoints, 11);        

            mrDtmFullResLinearParametersPtr->SetMaximumNumberOfPointsForLinear(maxNbLinearFeaturePoints);
            mrDtmFullResLinearParametersPtr->SetUseDecimation(true);             
            }
       
        mrDtmFullResLinearParametersPtr->SetCutLinears(true);
        
        status = AddClipOnQuery(fullResLinearQueryPtr, clips);
        assert(status == SUCCESS);        

        size_t nbPointsBefore;

        if (dtmPtr != 0)
            {
            nbPointsBefore = (size_t)dtmPtr->GetPointCount();
            }
        else
            {
            nbPointsBefore = 0;
            }                        

        if ((status = fullResLinearQueryPtr->Query(dtmPtr, regionPts, nbPts, (IMrDTMQueryParametersPtr&)mrDtmFullResLinearParametersPtr)) != SUCCESS)      
            {
            bool isSet = SetTriangulationTerminationCallback(0);
            assert(true == isSet);
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

        MrDTMFixResolutionIndexQueryParamsPtr mrDtmFixResqueryParamsPtr(MrDTMFixResolutionIndexQueryParams::CreateParams());                                                           

        if (decimationFactorForPointFeatures == DECIMATION_FACTOR_REQUESTED_FULL_RESOLUTION)
            {
            mrDtmFixResqueryParamsPtr->SetResolutionIndex(mrDTMPtr->GetNbResolutions(DTM_QUERY_DATA_POINT)-1);                      
            }
        else
            {
            int nbResolutions = mrDTMPtr->GetNbResolutions(DTM_QUERY_DATA_POINT);
            assert(nbResolutions >= 1);
            assert(decimationFactorForPointFeatures <= 1.0);
                        
            double resolutionInd = nbResolutions - 1 - log(1.0 / decimationFactorForPointFeatures) / log(4.0);
            
            resolutionInd = min(max(0, floor(resolutionInd)), nbResolutions - 1);
            
            mrDtmFixResqueryParamsPtr->SetResolutionIndex((int)resolutionInd);                      
            }        

        mrDtmFixResqueryParamsPtr->SetEdgeOptionTriangulationParam(edgeMethod);            
        mrDtmFixResqueryParamsPtr->SetMaxSideLengthTriangulationParam(edgeMethodLength);                
            
        if (!applyTriangulation)
            {                        
            mrDtmFixResqueryParamsPtr->SetTriangulationState(false);                
            }
            
        status = fixResPointQueryPtr->Query(singleResolutionViewDtmPtr, 0, 0, (IMrDTMQueryParametersPtr&)mrDtmFixResqueryParamsPtr);

        assert(singleResolutionViewDtmPtr != 0);

        IMrDTMPtr      singleResMrDTMViewPtr = IMrDTMPtr((IMrDTM*)singleResolutionViewDtmPtr.get());
        IMrDTMQueryPtr fullResQueryPtr;

        if ((mrDTMPtr->GetBaseGCS() == 0) || (destinationGCS == 0))
            {
            fullResQueryPtr = singleResMrDTMViewPtr->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, DTM_QUERY_DATA_POINT);                
            }
        else
            {           
            fullResQueryPtr = singleResMrDTMViewPtr->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, DTM_QUERY_DATA_POINT, 
                Bentley::GeoCoordinates::BaseGCSPtr(destinationGCS), drange);     
            }

        assert(fullResQueryPtr != 0);

        MrDTMFullResolutionQueryParamsPtr mrDtmFullResQueryParam(MrDTMFullResolutionQueryParams::CreateParams());

        if (!applyTriangulation)
            {                        
            mrDtmFullResQueryParam->SetTriangulationState(false); 
            }

        mrDtmFullResQueryParam->SetEdgeOptionTriangulationParam(edgeMethod);            
        mrDtmFullResQueryParam->SetMaxSideLengthTriangulationParam(edgeMethodLength);

        /*
        if (decimationFactorRequested == DECIMATION_FACTOR_REQUESTED_FULL_RESOLUTION)
            {
            mrDtmFullResQueryParam->SetMaximumNumberOfPoints(remainingPointCount);
            }
            /*
            }
        else
            {
            mrDtmFullResQueryParam->SetMaximumNumberOfPoints(remainingPointCount);
            }
            */
       
        mrDtmFullResQueryParam->SetMaximumNumberOfPoints(remainingPointCount);
        mrDtmFullResQueryParam->SetReturnAllPtsForLowestLevel(false);
            
        status = AddClipOnQuery(fullResQueryPtr, clips);
        assert(status == SUCCESS);                    

        size_t nbPointsBefore;

        if (dtmPtr != 0)
            {
            nbPointsBefore = (size_t)dtmPtr->GetPointCount();
            }
        else
            {
            nbPointsBefore = 0;
            }                        
        
        if ((status = fullResQueryPtr->Query(dtmPtr, regionPts, nbPts, (IMrDTMQueryParametersPtr&)mrDtmFullResQueryParam)) != SUCCESS)                         
            {
            bool isSet = SetTriangulationTerminationCallback(0);
            assert(true == isSet);
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
        
    isSet = SetTriangulationTerminationCallback(0);
    assert(true == isSet);

    return SUCCESS;
    }

#endif

    BentleyStatus ScalableMeshSDKexe::ParseImportDefinitionNew(BeXmlNodeP pTestNode)
        {
#if 0 
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
        
        status = pTestNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

        if (status == BEXML_Success)
            {
            BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));
            StatusInt status = importerPtr->SetBaseGCS(baseGCSPtr);
            assert(status == SUCCESS);
            }

        status = pTestNode->GetAttributeUInt64Value(maxNbPointsToImport, "maxNbPointsToImport");

        if (status != BEXML_Success)
            {            
            maxNbPointsToImport = numeric_limits<UInt64>::max();
            }

        if (ParseSourceSubNodes(mrdtmCreatorPtr->EditSources(), pTestNode) == false)
            {                
            return ERROR;
            }

        if (mrdtmCreatorPtr->Create() != SUCCESS)
            {
            return ERROR;
            }                

        mrdtmCreatorPtr
#endif

        //fclose(s_pPointResultFile);

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

    static bool s_useStm = true;

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

        if (!s_useStm)
            {
            ParseImportDefinition(pRootNode);
            }
        else
            {
            ParseImportDefinitionNew(pRootNode);
            }
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