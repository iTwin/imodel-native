//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetAttributesHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetAttributesHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetAttributesHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImaging.h>
#include <Imagepp/all/h/HPMAttributeSet.h>
#include <Imagepp/all/h/HPMAttribute.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>

#include <BeXml/BeXml.h>


USING_NAMESPACE_IMAGEPP

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("HIP-Attributes");

//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetAttributesHandler::HRFInternetAttributesHandler()
    : HRFInternetBinaryHandler("HIP-Attributes")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetAttributesHandler::~HRFInternetAttributesHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetAttributesHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                          HFCBuffer&              pio_rBuffer,
                                          HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection until the binary data
    ReadUntilData(pi_rConnection, pio_rBuffer);

    //
    // Format:
    //
    //  HIP-Attribute-Names/size:<data>
    //

    uint32_t DataSize;

    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    if (sscanf(string((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize()).c_str() + s_Label.size() + 1,
               "%lu:",
               &DataSize) != 1)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ATTRIBUTES);

    // Read the data in the buffer & skip end marker
    ReadData(pi_rConnection, pio_rBuffer, DataSize, true);
    SkipEndMarker(pi_rConnection);

    // verify validity
    if (DataSize == 0)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ATTRIBUTES);

    // parse the XML stream
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, pio_rBuffer.GetData(), pio_rBuffer.GetDataSize());
    if (pXmlDom.IsNull() || BEXML_Success != xmlStatus)
        {
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ATTRIBUTES);
        }

    // there is no nodes obtained from the parsing
    if (pXmlDom->GetRootElement() == NULL)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ATTRIBUTES);

    // parse the first node only.  (There should only be one, anyway)
    if (BeStringUtilities::Stricmp(pXmlDom->GetRootElement()->GetName(), "attributes") != 0)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ATTRIBUTES);

    HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys(new HCPGeoTiffKeys());

    uint32_t codePage;
    BeStringUtilities::GetCurrentCodePage (codePage);

    // parse the DIRECT childs of this node
    for(BeXmlNodeP pSubNode = pXmlDom->GetRootElement()->GetFirstChild (); NULL != pSubNode; pSubNode = pSubNode->GetNextSibling())
        {
        // the node must be an attribute
        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "attribute") == 0)
            {
            //BeXml PORT: Not sure we handle NON UTF8 protocol properly. We used to extract the "name" attribute in raw char and then convert from UTF8 or mutlibytes
            // to wide depending on the protocol version. The new lib assumed that the attributes are always UTF8. Anyway, all multibytes communication must be made in 
            // the same code page (client/server) so maybe this is not an issue since only standard ascii char would convert properly to wide and this is supported by UTF8.
            HASSERT(pi_rFile.GetProtocolVersion().GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol);

            // get the name of the attribute
            WString AttributeType;
            pSubNode->GetAttributeStringValue(AttributeType, "type");

            WString AttributeName;
            pSubNode->GetAttributeStringValue(AttributeName, "name");

            unsigned short GeoKeyID = HCPGeoTiffKeys::DecodeGeoKeyIDFromString(AttributeName);

            if (GeoKeyID != EndGeoKey)
                {
                if (BeStringUtilities::Wcsicmp(AttributeType.c_str(), L"integer") == 0)
                    {
                    // convert the value from string to int.
                    uint32_t Value = 0;
                    pSubNode->GetContentUInt32Value(Value);

                    //Old Imaging server starting from version 8.11.5 are going to send values
                    //greater than 65535 with the ProjectedCSType key.
                    if ((GeoKeyID == ProjectedCSType) && (Value > USHRT_MAX))
                        {
                        GeoKeyID = ProjectedCSTypeLong;
                        }

                    // add the key
                    pGeoTiffKeys->AddKey(GeoKeyID, Value);
                    }
                else if (BeStringUtilities::Wcsicmp(AttributeType.c_str(), L"float") == 0)
                    {
                    // convert the value from string to int.
                    double Value = 0;
                    pSubNode->GetContentDoubleValue(Value);

                    // add the key
                    pGeoTiffKeys->AddKey(GeoKeyID, Value);
                    }
                //String
                else if (BeStringUtilities::Wcsicmp(AttributeType.c_str(), L"") == 0)
                    {
                    WString Value;
                    pSubNode->GetContent(Value);

                    AString ValueA;
                    BeStringUtilities::WCharToLocaleChar (ValueA, codePage, Value.c_str());

                    // add the key
                    pGeoTiffKeys->AddKey(GeoKeyID, ValueA.c_str());
                    }
                }
            else
                {
                if (AttributeType == L"")
                    {
                    WString Value;
                    pSubNode->GetContent(Value);

                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromString(AttributeName, Value);
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                else if (AttributeType == L"VectorHDOUBLE")
                    {
                    WString Value;
                    pSubNode->GetContent(Value);

                    AString ValueA;
                    BeStringUtilities::WCharToLocaleChar (ValueA, codePage, Value.c_str());

                    // convert the value from string to vector<double>
                    HAutoPtr<char> pTempBuffer(new char[ValueA.size() + 1]);

                    strcpy(pTempBuffer.get(), ValueA.c_str());

                    char*          next_token=NULL;
                    char* pValue = BeStringUtilities::Strtok(pTempBuffer.get(), " ",&next_token);
                    vector<double> Vector;
                    double         VectorValue;

                    while (pValue != 0)
                        {
                        VectorValue = atof(pValue);
                        Vector.push_back(VectorValue);
                        pValue = BeStringUtilities::Strtok(NULL, " ",&next_token);
                        }

                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorDouble(AttributeName, Vector);
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                else if (AttributeType == L"float")
                    {
                    // convert the value from string to double
                    double Value = 0;
                    pSubNode->GetContentDoubleValue(Value);

                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromDouble(AttributeName, Value);
                    pi_rFile.m_pAttributes->Set(pTag);          
                    }
                else if (AttributeType == L"VectorHBYTE")
                    {
                    WString Value;
                    pSubNode->GetContent(Value);

                    AString ValueA;
                    BeStringUtilities::WCharToLocaleChar (ValueA, codePage, Value.c_str());

                    // convert the value from string to vector<Byte>.
                    HAutoPtr<char> pTempBuffer(new char[ValueA.size() + 1]);

                    strcpy(pTempBuffer.get(), ValueA.c_str());

                    char*        next_token=NULL;
                    char*        pValue = BeStringUtilities::Strtok(pTempBuffer.get(), " ",&next_token);
                    vector<Byte> Vector;
                    Byte         VectorValue;

                    while (pValue != 0)
                        {
                        VectorValue = (Byte)atoi(pValue);

                        HASSERT(VectorValue >= 0);

                        Vector.push_back((Byte)VectorValue);
                        pValue = BeStringUtilities::Strtok(NULL, " ",&next_token);
                        }

                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorByte(AttributeName, Vector);
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                else if (AttributeType == L"VectorHUSHORT")
                    {
                    WString Value;
                    pSubNode->GetContent(Value);

                    AString ValueA;
                    BeStringUtilities::WCharToLocaleChar (ValueA, codePage, Value.c_str());

                    // convert the value from string to int.
                    HAutoPtr<char> pTempBuffer(new char[ValueA.size() + 1]);

                    strcpy(pTempBuffer.get(), ValueA.c_str());

                    char*          next_token=NULL;
                    char*          pValue = BeStringUtilities::Strtok(pTempBuffer.get(), " ",&next_token);
                    vector<unsigned short> Vector;
                    int32_t       VectorValue;

                    while (pValue != 0)
                        {
                        VectorValue = atoi(pValue);
                        HASSERT((VectorValue >= 0) && (VectorValue <= USHRT_MAX));

                        Vector.push_back((unsigned short)VectorValue);
                        pValue = BeStringUtilities::Strtok(NULL, " ",&next_token);
                        }

                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorUShort(AttributeName, Vector);
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                else if (AttributeType == L"integer")
                    {
                    // convert the value from string to int.
                    uint32_t Value;
                    pSubNode->GetContentUInt32Value(Value);

                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromULong32(AttributeName, Value);
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                else if (AttributeType == L"VectorHCHAR")
                    {
                    WString Value;
                    pSubNode->GetContent(Value);

                    AString ValueA;
                    BeStringUtilities::WCharToLocaleChar (ValueA, codePage, Value.c_str());

                    // convert the value from string to int.
                    vector<char>           Vector;
                    string                 AttrValueStr = ValueA.c_str();
                    string::const_iterator CharIter     = AttrValueStr.begin();
                    string::const_iterator CharIterEnd  = AttrValueStr.end();

                    while (CharIter != CharIterEnd)
                        {
                        Vector.push_back(*CharIter);
                        CharIter++;
                        }

                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorChar(AttributeName, Vector);
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                else if (AttributeType == L"Matrix4X4")
                    {
                    WString Value;
                    pSubNode->GetContent(Value);

                    AString ValueA;
                    BeStringUtilities::WCharToLocaleChar (ValueA, codePage, Value.c_str());

                    // convert the value from string to HFCMatrix
                    HAutoPtr<char> pTempBuffer(new char[ValueA.size() + 1]);

                    strcpy(pTempBuffer.get(), ValueA.c_str());

                    char*          next_token=NULL;
                    char*          pValue = BeStringUtilities::Strtok(pTempBuffer.get(), " ",&next_token);
                    unsigned short RowInd = 0;
                    unsigned short ColInd = 0;
                    HFCMatrix<4,4> Matrix;
                    double         MatrixValue;

                    while ((pValue != 0) && (RowInd < 4))
                        {
                        MatrixValue = atof(pValue);
                        Matrix[RowInd][ColInd] = MatrixValue;

                        if (ColInd < 3)
                            {
                            ColInd++;
                            }
                        else
                            {
                            RowInd++;
                            ColInd = 0;
                            }

                        pValue = BeStringUtilities::Strtok(NULL, " ",&next_token);
                        }

                    HASSERT((RowInd == 4) && (ColInd == 0) && (pValue == 0));

                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromMatrix(AttributeName, Matrix);
                    pi_rFile.m_pAttributes->Set(pTag);  
                    }
                }
            }
        }

    if (pGeoTiffKeys->GetNbKeys() > 0)
        {
        pi_rFile.m_pListOfMetaDataContainers->SetMetaDataContainer((HFCPtr<HMDMetaDataContainer>&)pGeoTiffKeys);
        }
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCPtr<HPMGenericAttribute> HRFInternetAttributesHandler::DecodeHPMAttributeFromString(WStringCR  pi_rAttributeName, 
                                                                                       WStringCR  pi_rValue)
    {
    if (pi_rAttributeName == L"Image Description")
        return new HRFAttributeImageDescription(pi_rValue);
    else if (pi_rAttributeName == L"Page Name")
        return new HRFAttributePageName(pi_rValue);
    else if (pi_rAttributeName == L"Document Name")
        return new HRFAttributeDocumentName(pi_rValue);
    else if (pi_rAttributeName == L"Title")
        return new HRFAttributeTitle(pi_rValue);
    else if (pi_rAttributeName == L"Notes")
        return new HRFAttributeNotes(pi_rValue);
    else if (pi_rAttributeName == L"Key Word")
        return new HRFAttributeKeyWord(pi_rValue);
    else if (pi_rAttributeName == L"Make")
        return new HRFAttributeMake(pi_rValue);
    else if (pi_rAttributeName == L"Model")
        return new HRFAttributeModel(pi_rValue);
    else if (pi_rAttributeName == L"Software")
        return new HRFAttributeSoftware(pi_rValue);
    else if (pi_rAttributeName == L"Version")
        return new HRFAttributeVersion(pi_rValue);
    else if (pi_rAttributeName == L"Copyright")
        return new HRFAttributeCopyright(pi_rValue);
    else if(pi_rAttributeName == L"Artist")
        return new HRFAttributeArtist(pi_rValue);
    else if(pi_rAttributeName == L"Director")
        return new HRFAttributeDirector(pi_rValue);
    else if(pi_rAttributeName == L"Company")
        return new HRFAttributeCompany(pi_rValue);
    else if(pi_rAttributeName == L"Vendor")
        return new HRFAttributeVendor(pi_rValue);
    else if(pi_rAttributeName == L"Host Computer")
        return new HRFAttributeHostComputer(pi_rValue);
    else if(pi_rAttributeName == L"Date Time")
        return new HRFAttributeDateTime(pi_rValue);
    else if(pi_rAttributeName == L"Ink Names")
        return new HRFAttributeInkNames(pi_rValue);
    else if(pi_rAttributeName == L"Security Level")
        return new HRFAttributeSecurityLevel(pi_rValue);
    else if(pi_rAttributeName == L"Legal Disclaimer")
        return new HRFAttributeLegalDisclaimer(pi_rValue);
    else if(pi_rAttributeName == L"Content Warning")
        return new HRFAttributeContentWarning(pi_rValue);
    else if(pi_rAttributeName == L"OnDemand Rasters Info")
        {
        AString tempStrA;
        BeStringUtilities::WCharToCurrentLocaleChar(tempStrA, pi_rValue.c_str());
        return new HRFAttributeOnDemandRastersInfo(tempStrA.c_str());
        }
    else if(pi_rAttributeName == L"PCSCitation")
        return new HRFAttributePCSCitation(pi_rValue);
    else if(pi_rAttributeName == L"GTCitation")
        return new HRFAttributeGTCitation(pi_rValue);
    else if(pi_rAttributeName == L"GeogCitation")
        return new HRFAttributeGeogCitation(pi_rValue);
    else if(pi_rAttributeName == L"VerticalCitation")
        return new HRFAttributeVerticalCitation(pi_rValue);
    else if(pi_rAttributeName == L"GPSLatitudeRef")
        return new HRFAttributeGPSLatitudeRef(pi_rValue);
    else if(pi_rAttributeName == L"GPSLongitudeRef")
        return new HRFAttributeGPSLongitudeRef(pi_rValue);
    else if(pi_rAttributeName == L"GPSSatellites")
        return new HRFAttributeGPSSatellites(pi_rValue);
    else if(pi_rAttributeName == L"GPSStatus")
        return new HRFAttributeGPSStatus(pi_rValue);
    else if(pi_rAttributeName == L"GPSMeasureMode")
        return new HRFAttributeGPSMeasureMode(pi_rValue);
    else if(pi_rAttributeName == L"GPSSpeedRef")
        return new HRFAttributeGPSSpeedRef(pi_rValue);
    else if(pi_rAttributeName == L"GPSTrackRef")
        return new HRFAttributeGPSTrackRef(pi_rValue);
    else if(pi_rAttributeName == L"GPSImgDirectionRef")
        return new HRFAttributeGPSImgDirectionRef(pi_rValue);
    else if(pi_rAttributeName == L"GPSMapDatum")
        return new HRFAttributeGPSMapDatum(pi_rValue);
    else if(pi_rAttributeName == L"GPSDestLatitudeRef")
        return new HRFAttributeGPSDestLatitudeRef(pi_rValue);
    else if(pi_rAttributeName == L"GPSDestLongitudeRef")
        return new HRFAttributeGPSDestLongitudeRef(pi_rValue);
    else if(pi_rAttributeName == L"GPSDestBearingRef")
        return new HRFAttributeGPSDestBearingRef(pi_rValue);
    else if(pi_rAttributeName == L"GPSDestDistanceRef")
        return new HRFAttributeGPSDestDistanceRef(pi_rValue);
    else if(pi_rAttributeName == L"GPSDateStamp")
        return new HRFAttributeGPSDateStamp(pi_rValue);
    else if(pi_rAttributeName == L"SpectralSensitivity")
        return new HRFAttributeSpectralSensitivity(pi_rValue);
    else if(pi_rAttributeName == L"DateTimeOriginal")
        return new HRFAttributeDateTimeOriginal(pi_rValue);
    else if(pi_rAttributeName == L"DateTimeDigitized")
        return new HRFAttributeDateTimeDigitized(pi_rValue);
    else if(pi_rAttributeName == L"SubSecTime")
        return new HRFAttributeSubSecTime(pi_rValue);
    else if(pi_rAttributeName == L"SubSecTimeOriginal")
        return new HRFAttributeSubSecTimeOriginal(pi_rValue);
    else if(pi_rAttributeName == L"SubSecTimeDigitized")
        return new HRFAttributeSubSecTimeDigitized(pi_rValue);
    else if(pi_rAttributeName == L"RelatedSoundFile")
        return new HRFAttributeRelatedSoundFile(pi_rValue);
    else if(pi_rAttributeName == L"ImageUniqueID")
        return new HRFAttributeImageUniqueID(pi_rValue);
    else if(pi_rAttributeName == L"Application Code")
        {
        AString tempStrA;
        BeStringUtilities::WCharToCurrentLocaleChar(tempStrA, pi_rValue.c_str());
        return new HRFAttributeGIFApplicationCode(tempStrA.c_str());
        }
    else if(pi_rAttributeName == L"Time Modification")
        return new HRFAttributeTimeModification(pi_rValue);
    else if(pi_rAttributeName == L"File")
        {
        AString tempStrA;
        BeStringUtilities::WCharToCurrentLocaleChar(tempStrA, pi_rValue.c_str());
        return new HRFAttributeHGRFile(tempStrA.c_str());
        }
    else if(pi_rAttributeName == L"Version")
        {
        AString tempStrA;
        BeStringUtilities::WCharToCurrentLocaleChar(tempStrA, pi_rValue.c_str());
        return new HRFAttributeHGRVersion(tempStrA.c_str());
        }
    else if(pi_rAttributeName == L"Owner")
        {
        AString tempStrA;
        BeStringUtilities::WCharToCurrentLocaleChar(tempStrA, pi_rValue.c_str());
        return new HRFAttributeHGROwner(tempStrA.c_str());
        }
    else if(pi_rAttributeName == L"Description")
        {
        AString tempStrA;
        BeStringUtilities::WCharToCurrentLocaleChar(tempStrA, pi_rValue.c_str());
        return new HRFAttributeHGRDescription(tempStrA.c_str());
        }
    else if(pi_rAttributeName == L"HPS_IMAGE_DESCRIPTION")
        return new HPSAttributeImageDescription(pi_rValue);
    
    BeAssertOnce(!"HRFInternetAttributesHandler::Handle : Unsupported string attribute");
    return 0;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCPtr<HPMGenericAttribute> HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorDouble(WStringCR          pi_rAttributeName, 
                                                                                             vector<double>   pi_rValue)
    {
    if(pi_rAttributeName == L"Sample Minimum Value")
        return new HRFAttributeMinSampleValue(pi_rValue);
    else if(pi_rAttributeName == L"Sample Maximum Value")
        return new HRFAttributeMaxSampleValue(pi_rValue);
    else if(pi_rAttributeName == L"GPSLatitude")
        return new HRFAttributeGPSLatitude(pi_rValue);
    else if(pi_rAttributeName == L"GPSLongitude")
        return new HRFAttributeGPSLongitude(pi_rValue);
    else if(pi_rAttributeName == L"GPSTimeStamp")
        return new HRFAttributeGPSTimeStamp(pi_rValue);
    else if(pi_rAttributeName == L"GPSDestLatitude")
        return new HRFAttributeGPSDestLatitude(pi_rValue);
    else if(pi_rAttributeName == L"GPSDestLongitude")
        return new HRFAttributeGPSDestLongitude(pi_rValue);
    
    BeAssertOnce(!"HRFInternetAttributesHandler::Handle : Unsupported vector<double> attribute");
    return 0;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCPtr<HPMGenericAttribute> HRFInternetAttributesHandler::DecodeHPMAttributeFromDouble(WStringCR  pi_rAttributeName, 
                                                                                       double   pi_rValue)
    {
    if(pi_rAttributeName == L"X Resolution")
        return new HRFAttributeXResolution(pi_rValue);
    else if(pi_rAttributeName == L"Y Resolution")
        return new HRFAttributeYResolution(pi_rValue);
    else if(pi_rAttributeName == L"GPSAltitude")
        return new HRFAttributeGPSAltitude(pi_rValue);
    else if(pi_rAttributeName == L"ProjLinearUnitSize")
        return new HRFAttributeProjLinearUnitSize(pi_rValue);
    else if(pi_rAttributeName == L"GeogLinearUnitSize")
        return new HRFAttributeGeogLinearUnitSize(pi_rValue);
    else if(pi_rAttributeName == L"GeogAngularUnitSize")
        return new HRFAttributeGeogAngularUnitSize(pi_rValue);
    else if(pi_rAttributeName == L"GeogSemiMajorAxis")
        return new HRFAttributeGeogSemiMajorAxis(pi_rValue);
    else if(pi_rAttributeName == L"GeogSemiMinorAxis")
        return new HRFAttributeGeogSemiMinorAxis(pi_rValue);
    else if(pi_rAttributeName == L"GeogInvFlattening")
        return new HRFAttributeGeogInvFlattening(pi_rValue);
    else if(pi_rAttributeName == L"GeogPrimeMeridianLong")
        return new HRFAttributeGeogPrimeMeridianLong(pi_rValue);
    else if(pi_rAttributeName == L"ProjStdParallel1")
        return new HRFAttributeProjStdParallel1(pi_rValue);
    else if(pi_rAttributeName == L"ProjStdParallel2")
        return new HRFAttributeProjStdParallel2(pi_rValue);
    else if(pi_rAttributeName == L"ProjNatOriginLong")
        return new HRFAttributeProjNatOriginLong(pi_rValue);
    else if(pi_rAttributeName == L"ProjNatOriginLat")
        return new HRFAttributeProjNatOriginLat(pi_rValue);
    else if(pi_rAttributeName == L"ProjFalseEasting")
        return new HRFAttributeProjFalseEasting(pi_rValue);
    else if(pi_rAttributeName == L"ProjFalseNorthing")
        return new HRFAttributeProjFalseNorthing(pi_rValue);
    else if(pi_rAttributeName == L"ProjFalseOriginLong")
        return new HRFAttributeProjFalseOriginLong(pi_rValue);
    else if(pi_rAttributeName == L"ProjFalseOriginLat")
        return new HRFAttributeProjFalseOriginLat(pi_rValue);
    else if(pi_rAttributeName == L"ProjFalseOriginEasting")
        return new HRFAttributeProjFalseOriginEasting(pi_rValue);
    else if(pi_rAttributeName == L"ProjFalseOriginNorthing")
        return new HRFAttributeProjFalseOriginNorthing(pi_rValue);
    else if(pi_rAttributeName == L"ProjCenterLong")
        return new HRFAttributeProjCenterLong(pi_rValue);
    else if(pi_rAttributeName == L"ProjCenterLat")
        return new HRFAttributeProjCenterLat(pi_rValue);
    else if(pi_rAttributeName == L"ProjCenterEasting")
        return new HRFAttributeProjCenterEasting(pi_rValue);
    else if(pi_rAttributeName == L"ProjCenterNorthing")
        return new HRFAttributeProjCenterNorthing(pi_rValue);
    else if(pi_rAttributeName == L"ProjScaleAtNatOrigin")
        return new HRFAttributeProjScaleAtNatOrigin(pi_rValue);
    else if(pi_rAttributeName == L"ProjScaleAtCenter")
        return new HRFAttributeProjScaleAtCenter(pi_rValue);
    else if(pi_rAttributeName == L"ProjAzimuthAngle")
        return new HRFAttributeProjAzimuthAngle(pi_rValue);
    else if(pi_rAttributeName == L"ProjStraightVertPoleLong")
        return new HRFAttributeProjStraightVertPoleLong(pi_rValue);
    else if(pi_rAttributeName == L"ProjRectifiedGridAngle")
        return new HRFAttributeProjRectifiedGridAngle(pi_rValue);
    else if(pi_rAttributeName == L"GPSDOP")
        return new HRFAttributeGPSDOP(pi_rValue);
    else if(pi_rAttributeName == L"GPSSpeed")
        return new HRFAttributeGPSSpeed(pi_rValue);
    else if(pi_rAttributeName == L"GPSTrack")
        return new HRFAttributeGPSTrack(pi_rValue);
    else if(pi_rAttributeName == L"GPSImgDirection")
        return new HRFAttributeGPSImgDirection(pi_rValue);
    else if(pi_rAttributeName == L"GPSDestBearing")
        return new HRFAttributeGPSDestBearing(pi_rValue);
    else if(pi_rAttributeName == L"GPSDestDistance")
        return new HRFAttributeGPSDestDistance(pi_rValue);
    else if(pi_rAttributeName == L"ExposureTime")
        return new HRFAttributeExposureTime(pi_rValue);  
    else if(pi_rAttributeName == L"FNumber")
        return new HRFAttributeFNumber(pi_rValue); 
    else if(pi_rAttributeName == L"CompressedBitsPerPixel")
        return new HRFAttributeCompressedBitsPerPixel(pi_rValue);
    else if(pi_rAttributeName == L"ShutterSpeedValue")
        return new HRFAttributeShutterSpeedValue(pi_rValue);
    else if(pi_rAttributeName == L"ApertureValue")
        return new HRFAttributeApertureValue(pi_rValue);
    else if(pi_rAttributeName == L"BrightnessValue")
        return new HRFAttributeBrightnessValue(pi_rValue);
    else if(pi_rAttributeName == L"ExposureBiasValue")
        return new HRFAttributeExposureBiasValue(pi_rValue);
    else if(pi_rAttributeName == L"MaxApertureValue")
        return new HRFAttributeMaxApertureValue(pi_rValue);
    else if(pi_rAttributeName == L"SubjectDistance")
        return new HRFAttributeSubjectDistance(pi_rValue);
    else if(pi_rAttributeName == L"FocalLength")
        return new HRFAttributeFocalLength(pi_rValue);
    else if(pi_rAttributeName == L"FlashEnergy")
        return new HRFAttributeFlashEnergy(pi_rValue);
    else if(pi_rAttributeName == L"ExposureIndex")
        return new HRFAttributeExposureIndex(pi_rValue);
    else if(pi_rAttributeName == L"DigitalZoomRatio")
        return new HRFAttributeDigitalZoomRatio(pi_rValue);
    else if(pi_rAttributeName == L"Image Gamma")
        return new HRFAttributeImageGamma(pi_rValue);
    else if(pi_rAttributeName == L"Original File Modification Date")
        return new HRFAttributeOriginalFileModificationDate((uint64_t)pi_rValue);
    else if (pi_rAttributeName == L"FocalPlaneXResolution")
        return new HRFAttributeFocalPlaneXResolution(pi_rValue);
    else if (pi_rAttributeName == L"FocalPlaneYResolution")
        return new HRFAttributeFocalPlaneYResolution(pi_rValue);
     
    BeAssertOnce(!"HRFInternetpi_rAttributeNamesHandler::Handle : Unsupported double attribute");
    return 0;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCPtr<HPMGenericAttribute> HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorByte(WStringCR          pi_rAttributeName, 
                                                                                           vector<Byte>     pi_rValue)
    {
    if (pi_rAttributeName == L"GPSVersionID")
        return new HRFAttributeGPSVersionID(pi_rValue);
    else if (pi_rAttributeName == L"GPSProcessingMethod")
        return new HRFAttributeGPSProcessingMethod(pi_rValue);
    else if (pi_rAttributeName == L"GPSAreaInformation")
        return new HRFAttributeGPSAreaInformation(pi_rValue);
    else if (pi_rAttributeName == L"OptoelectricConversionFactor")
        return new HRFAttributeOptoElectricConversionFactor(pi_rValue);
    else if (pi_rAttributeName == L"ComponentsConfiguration")
        return new HRFAttributeComponentsConfiguration(pi_rValue);
    else if (pi_rAttributeName == L"MakerNote")
        return new HRFAttributeMakerNote(pi_rValue);
    else if (pi_rAttributeName == L"UserComment")
        return new HRFAttributeUserComment(pi_rValue);
    else if (pi_rAttributeName == L"FlashpixVersion")
        return new HRFAttributeFlashpixVersion(pi_rValue);
    else if (pi_rAttributeName == L"SpatialFrequencyResponse")
        return new HRFAttributeSpatialFrequencyResponse(pi_rValue);
    else if (pi_rAttributeName == L"CFAPattern")
        return new HRFAttributeCFAPattern(pi_rValue);
    else if (pi_rAttributeName == L"DeviceSettingDescription")
        return new HRFAttributeDeviceSettingDescription(pi_rValue);
    
    BeAssertOnce(!"HRFInternetAttributeHandler::Handle : Unsupported vector<Byte> attribute");
    return 0;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCPtr<HPMGenericAttribute> HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorUShort(WStringCR            pi_rAttributeName, 
                                                                                             vector<unsigned short>     pi_rValue)
    {

    if (pi_rAttributeName == L"ISOSpeedRatings")
        return new HRFAttributeISOSpeedRatings(pi_rValue);
    else if (pi_rAttributeName == L"SubjectArea")
        return new HRFAttributeSubjectArea(pi_rValue);
    else if (pi_rAttributeName == L"SubjectLocation")
        return new HRFAttributeSubjectLocation(pi_rValue);
    
    BeAssertOnce(!"HRFInternetAttributeHandler::Handle : Unsupported vector<unsigned short> attribute");
    return 0;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCPtr<HPMGenericAttribute> HRFInternetAttributesHandler::DecodeHPMAttributeFromULong32(WStringCR          pi_rAttributeName, 
                                                                                        uint32_t        pi_rValue)
    {
    if (pi_rAttributeName == L"PixelXDimension")
        return new HRFAttributePixelXDimension(pi_rValue);
    else if (pi_rAttributeName == L"PixelYDimension")
        return new HRFAttributePixelYDimension(pi_rValue);
    else if (pi_rAttributeName == L"ProjectedCSTypeLong")
        return new HRFAttributeProjectedCSTypeLong(pi_rValue);
    
    BeAssertOnce(!"HRFInternetAttributeHandler::Handle : Unsupported uint32_t attribute");
    return 0;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCPtr<HPMGenericAttribute> HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorChar(WStringCR          pi_rAttributeName, 
                                                                                           vector<char>          pi_rValue)
    {
    if (pi_rAttributeName == L"ExifVersion")
        return new HRFAttributeExifVersion(pi_rValue);
    
    BeAssertOnce(!"HRFInternetAttributeHandler::Handle : Unsupported vector<char> attribute"); 
    return 0;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCPtr<HPMGenericAttribute> HRFInternetAttributesHandler::DecodeHPMAttributeFromMatrix(WStringCR          pi_rAttributeName, 
                                                                                       HFCMatrix<4,4>   pi_rValue)
    {
    if(pi_rAttributeName == L"3D Tranformation Matrix")
        return new HRFAttribute3DTransformationMatrix(pi_rValue);
    
    BeAssertOnce(!"HRFInternetAttributeHandler::Handle : Unsupported HFCMatrix<4,4> attribute");
    return 0;
    }