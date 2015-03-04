/*--------------------------------------------------------------------------------------+
|
|     $Source: Properties/RasterFileFormats.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

#include <Imagepp/all/h/HRFRasterFileFactory.h>

HFC_IMPLEMENT_SINGLETON(RasterFileFormats)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileFormats::RasterFileFormats(void)
    {
    // Query I++ about supported file formats and fill out info map.
    const HRFRasterFileFactory::Creators (& creators)(HRFRasterFileFactory::GetInstance()->GetCreators(HFC_READ_WRITE));
    for(HRFRasterFileFactory::Creators::const_iterator creatorItor(creators.begin()); creatorItor != creators.end(); ++ creatorItor)
        {
        HRFRasterFileCreator(* pFileCreator)(*creatorItor);

        WString formatExtenstions = pFileCreator->GetExtensions();          // format: "*.abc[;*.def]"
        WString label             = pFileCreator->GetLabel();
        for (WString::size_type dotPos = formatExtenstions.find(L"."); dotPos != WString::npos; dotPos = formatExtenstions.find(L"."))
            {
            WString::size_type delimeterPos = formatExtenstions.find(L";");
            WString oneExt = formatExtenstions.substr(dotPos+1, delimeterPos-(dotPos+1));
            m_supportedExtension.insert(SupportedExtensionListType::value_type(oneExt,label));
            WString newFormatExtensions(delimeterPos != WString::npos ? formatExtenstions.substr(delimeterPos+1) : L"");
            formatExtenstions = newFormatExtensions;
            }       
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileFormats::~RasterFileFormats(void)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFileFormats::IsExtensionSupported(WCharCP extension)
    {
    WString inputExtension(extension);
    WString::size_type dotPos = inputExtension.find(L".");
    WString oneExt(inputExtension);
    if (dotPos!=WString::npos)
        oneExt = inputExtension.substr(dotPos+1);

    return m_supportedExtension.find(SupportedExtensionListType::key_type(oneExt)) != m_supportedExtension.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFileFormats::OutputSupportedExtensions()
    {
    for (SupportedExtensionListType::const_iterator itr=m_supportedExtension.begin(); itr != m_supportedExtension.end(); ++itr)
        {
        WString output(L".");
        output+=itr->first;
        output+=L"\n";
        OutputDebugStringW((output.c_str()));
        }
    }
