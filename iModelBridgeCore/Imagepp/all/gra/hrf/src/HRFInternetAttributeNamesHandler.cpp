//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetAttributeNamesHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetAttributeNamesHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetAttributesHandler.h>
#include <Imagepp/all/h/HRFInternetAttributeNamesHandler.h>
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

const static string s_Label("HIP-Attribute-Names");

//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetAttributeNamesHandler::HRFInternetAttributeNamesHandler()
    : HRFInternetBinaryHandler("HIP-Attribute-Names")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetAttributeNamesHandler::~HRFInternetAttributeNamesHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetAttributeNamesHandler::Handle(HRFInternetImagingFile& pi_rFile,
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
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ATTRIBUTE_NAMES);

    // Read the data in the buffer & skip end marker
    ReadData(pi_rConnection, pio_rBuffer, DataSize, true);
    SkipEndMarker(pi_rConnection);

    // verify validity
    if (DataSize == 0)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ATTRIBUTE_NAMES);

    // parse the XML stream
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, pio_rBuffer.GetData(), pio_rBuffer.GetDataSize());
    if (pXmlDom.IsNull() || BEXML_Success != xmlStatus)
        {
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ATTRIBUTE_NAMES);
        }


    // there is no nodes obtained from the parsing
    if (pXmlDom->GetRootElement() == NULL)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ATTRIBUTE_NAMES);

    // parse the first node only.  (There should only be one, anyway)
    if (BeStringUtilities::Stricmp(pXmlDom->GetRootElement()->GetName(), "attribute-names") != 0)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ATTRIBUTE_NAMES);

    HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys(new HCPGeoTiffKeys());

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
                    // add the key
                    pGeoTiffKeys->AddKey(GeoKeyID, (uint32_t)0);
                    }
                else if (BeStringUtilities::Wcsicmp(AttributeType.c_str(), L"float") == 0)
                    {
                    // add the key
                    pGeoTiffKeys->AddKey(GeoKeyID, 0.0);
                    }
                //String
                else if (BeStringUtilities::Wcsicmp(AttributeType.c_str(), L"") == 0)
                    {
                    // add the key
                    pGeoTiffKeys->AddKey(GeoKeyID, "");
                    }
                }
            else
                {
                if (AttributeType == L"")
                    {
                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromString(AttributeName, L"");
                    pi_rFile.m_pAttributes->Set(pTag);          
                    }
                else if (AttributeType == L"VectorHDOUBLE")
                    {
                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorDouble(AttributeName, vector<double>());
                    pi_rFile.m_pAttributes->Set(pTag);     
                    }
                else if (AttributeType == L"float")
                    {
                    double Value = 0.0;
                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromDouble(AttributeName, Value);
                    pi_rFile.m_pAttributes->Set(pTag);    
                    }
                else if (AttributeType == L"VectorHBYTE")
                    {
                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorByte(AttributeName, vector<Byte>());
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                else if (AttributeType == L"VectorHUSHORT")
                    {
                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorUShort(AttributeName, vector<unsigned short>());
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                else if (AttributeType == L"integer")
                    {
                    uint32_t Value = 0;
                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromULong32(AttributeName, Value);
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                else if (AttributeType == L"VectorHCHAR")
                    {
                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromVectorChar(AttributeName, vector<char>());
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                else if (AttributeType == L"Matrix4X4")
                    {
                    HFCPtr<HPMGenericAttribute> pTag = HRFInternetAttributesHandler::DecodeHPMAttributeFromMatrix(AttributeName, HFCMatrix<4,4>());
                    pi_rFile.m_pAttributes->Set(pTag); 
                    }
                }    
            }
        }

    if (pGeoTiffKeys->GetNbKeys() > 0)
        {
        IRasterBaseGcsPtr pBaseGCS = HRFGeoCoordinateProvider::CreateRasterGcsFromGeoTiffKeys(NULL, NULL, *pGeoTiffKeys);
        //&&AR review:  We are doing nothing with pBaseGCS 
//        pi_rFile.SetGeocoding(pBaseGCS);

        pi_rFile.m_pListOfMetaDataContainers->SetMetaDataContainer((HFCPtr<HMDMetaDataContainer>&)pGeoTiffKeys);
        }
    }
