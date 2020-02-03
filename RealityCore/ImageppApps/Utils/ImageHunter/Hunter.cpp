/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/Hunter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/Hunter.cpp,v 1.11 2011/06/02 19:38:27 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Methods for class Hunter
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Hunter.h"
#include "Criterias.h"
#include "HuntTools.h"
#include "ImageFile.h"

using namespace System::ComponentModel;
/*using namespace System; */
using namespace System::Collections;
using namespace System::Diagnostics;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Hunter::Hunter()
{
    // Attributes initialization
    m_isRecursiveSearch = false;
    m_numberOfFiles = 0;
	m_criteriasBuilder = gcnew ArrayList();
	m_criteriasList = gcnew Hashtable();
    m_validImages = gcnew ArrayList();
    m_supportedCapabilities = gcnew Hashtable();
    m_ExcludedExtensions = gcnew Hashtable();
    Init();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Hunter^ Hunter::GetInstance()
{
    // Singleton pattern
	if (m_instance == nullptr)
	{
		m_instance = gcnew Hunter();
	}
	
	return m_instance;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Hunter::Init()
{
    Hashtable^ fileFormats = gcnew Hashtable();
    Hashtable^ pixelTypes = gcnew Hashtable();
    Hashtable^ codecTypes = gcnew Hashtable();
    Hashtable^ blockTypes = gcnew Hashtable();
    Hashtable^ SLOTypes = gcnew Hashtable();
    Hashtable^ transfoModelTypes = gcnew Hashtable();
    Hashtable^ tagTypes = gcnew Hashtable();

    int tagID = 0;

	const HRFRasterFileFactory::Creators & pCreators = HRFRasterFileFactory::GetInstance()->GetCreators();
	for (HRFRasterFileFactory::Creators::const_iterator itr(pCreators.begin()); itr != pCreators.end(); ++itr)
	{
        // Adding supported image formats
		HRFRasterFileCreator* pCreator = * itr;
        fileFormats->Add(pCreator->GetRasterFileClassID(), gcnew System::String(pCreator->GetLabel().c_str()));
        
        // Looking for Pixel Types
        HFCPtr<HRFRasterFileCapabilities> pRasterCapabilities(pCreator->GetCapabilities());
		const HFCPtr<HRFRasterFileCapabilities>& pPixelTypeCap = pRasterCapabilities->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID);
        for (uint32_t pixelIndex(0); pixelIndex < pPixelTypeCap->CountCapabilities(); ++pixelIndex)
        {
            // Adding supported Pixel Types
            const HFCPtr<HRFPixelTypeCapability> pCurrentPixelType = (const HFCPtr<HRFPixelTypeCapability>& )(pPixelTypeCap->GetCapability(pixelIndex));
            HCLASS_ID pixelClassID = pCurrentPixelType->GetPixelTypeClassID();
            System::String^ pixelTypeLabel = gcnew System::String(HUTClassIDDescriptor::GetInstance()->GetClassLabelPixelType(pixelClassID).c_str());

            if (!pixelTypes->ContainsKey(pixelClassID))
            {
                pixelTypes->Add(pixelClassID, pixelTypeLabel);
            }

            // Looking for Codecs
            for (uint32_t codecIndex(0); codecIndex < pCurrentPixelType->CountCodecs(); ++codecIndex)
            {
                const HFCPtr<HRFCodecCapability> pCodecCapability = pCurrentPixelType->GetCodecCapabilityByIndex(codecIndex); 
                HCLASS_ID codecClassID = pCodecCapability->GetCodecClassID();
                System::String^ codecLabel = gcnew System::String(HUTClassIDDescriptor::GetInstance()->GetClassLabelCodec(codecClassID).c_str());

                if (!codecTypes->ContainsKey(codecClassID))
                {
                    codecTypes->Add(codecClassID, codecLabel);
                }

                // Looking for Block Types
                BlockType key = BlockType::IMAGE;
                if (!blockTypes->ContainsKey((int)key))
                {
                    key = BlockType::IMAGE;
                    blockTypes->Add((int)key, (int)key);
                }
                for (uint32_t blockIndex(0); blockIndex < pCodecCapability->CountBlockType(); ++blockIndex)
                {
                    // Adding supported Block Types
                    const HFCPtr<HRFBlockCapability> pBlockCapability = (const HFCPtr<HRFBlockCapability>& )(pCodecCapability->GetBlockTypeCapability(blockIndex));
                    HRFBlockType::Block blockType = pBlockCapability->GetBlockType().m_BlockType;
                    
                    // Got some problems with dotNET and the BlockType enum.
                    // So, we translate the Image++ BlockType to a native dotNET enum.
                    BlockType key;
                    switch (blockType)
                    {
                    case HRFBlockType::AUTO_DETECT:
                        key = BlockType::AUTO_DETECT;
                        break;
                    case HRFBlockType::LINE:
                        key = BlockType::LINE;
                        break;
                    case HRFBlockType::TILE:
                        key = BlockType::TILE;
                        break;
                    case HRFBlockType::STRIP:
                        key = BlockType::STRIP;
                        break;
                    case HRFBlockType::IMAGE:
                        key = BlockType::STRIP;
                        break;
                    default:
                        key = BlockType::IMAGE;
                        break;
                    }

                    if (!blockTypes->ContainsKey((int)key))
                    {
                        blockTypes->Add((int)key, (int)key);
                    }
                }
            }
        }

        // Scanline Orientation Capabilities
        const HFCPtr<HRFRasterFileCapabilities>& pSLOCap = pRasterCapabilities->GetCapabilitiesOfType(HRFScanlineOrientationCapability::CLASS_ID);
        for (uint32_t SLOIndex(0); SLOIndex < pSLOCap->CountCapabilities(); ++SLOIndex)
        {
            const HFCPtr<HRFScanlineOrientationCapability>& SLOCapability = (const HFCPtr<HRFScanlineOrientationCapability>& ) pSLOCap->GetCapability(SLOIndex);
            System::String^ slo = gcnew System::String(HUTClassIDDescriptor::GetInstance()->GetClassLabelSLO(SLOCapability->GetScanlineOrientation()).c_str());
            slo = static_cast<int>(SLOCapability->GetScanlineOrientation().m_ScanlineOrientation).ToString() + ". " + slo;
            if (!SLOTypes->ContainsKey(static_cast<int>(SLOCapability->GetScanlineOrientation().m_ScanlineOrientation)))
            {
                SLOTypes->Add(static_cast<int>(SLOCapability->GetScanlineOrientation().m_ScanlineOrientation), slo);
            }
        }

        // Transformation model Capabilities
        if (pRasterCapabilities->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID))
        {
            const HFCPtr<HRFRasterFileCapabilities>& pTransfoModelCap = pRasterCapabilities->GetCapabilitiesOfType(HRFTransfoModelCapability::CLASS_ID);
            for (uint32_t tmIndex(0); tmIndex < pTransfoModelCap->CountCapabilities(); ++tmIndex)
            {
                const HFCPtr<HRFTransfoModelCapability>& TransfoModelCapability = (const HFCPtr<HRFTransfoModelCapability>& ) pTransfoModelCap->GetCapability(tmIndex);
                HCLASS_ID transfoModelID = TransfoModelCapability->GetTransfoModelClassKey();
                System::String^ transfoModelLabel = gcnew System::String(HUTClassIDDescriptor::GetInstance()->GetClassLabelTransfoModel(transfoModelID).c_str());

                if (!transfoModelTypes->ContainsKey(transfoModelID))
                {
                    transfoModelTypes->Add(transfoModelID, transfoModelLabel);
                }
            }
        }

        // Tag Capabilities
        if (pRasterCapabilities->HasCapabilityOfType(HRFTagCapability::CLASS_ID))
        {
            const HFCPtr<HRFRasterFileCapabilities>& pTagCap = pRasterCapabilities->GetCapabilitiesOfType(HRFTagCapability::CLASS_ID);
            for (uint32_t tagIndex(0); tagIndex < pTagCap->CountCapabilities(); ++tagIndex)
            {
                const HFCPtr<HRFTagCapability>& tagCapability = (const HFCPtr<HRFTagCapability>& ) pTagCap->GetCapability(tagIndex);
                HFCPtr<HPMGenericAttribute> tagAttr = tagCapability->GetTag();
                System::String^ tag = gcnew System::String(tagAttr->GetName().c_str());
                if (!tagTypes->ContainsValue(tag))
                {
                    // The tagID specified here is not related to Image++.
                    // Since tags does not have an ID, we use a simple counter.
                    tagTypes->Add(tagID, tag);
                    ++tagID;
                }
            }
        }
	}

    // Adding supported capabilities to the Hunter
    m_supportedCapabilities->Add((int)SupportedCapability::FileFormat, fileFormats);
    m_supportedCapabilities->Add((int)SupportedCapability::PixelType, pixelTypes);
    m_supportedCapabilities->Add((int)SupportedCapability::Codec, codecTypes);
    m_supportedCapabilities->Add((int)SupportedCapability::BlockType, blockTypes);
    m_supportedCapabilities->Add((int)SupportedCapability::ScanlineOrientation, SLOTypes);
    m_supportedCapabilities->Add((int)SupportedCapability::TransfoModel, transfoModelTypes);
    m_supportedCapabilities->Add((int)SupportedCapability::Tag, tagTypes);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Hunter::AddCriteria(SupportedCapability capability, ICriteria ^ criteria)
{
    ArrayList^ criterias = static_cast<ArrayList^>(m_criteriasList[static_cast<int>(capability)]);
    criterias->Add(criteria);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Hunter::RemoveCriteria(SupportedCapability capability, ICriteria^ criteria)
{
    ArrayList^ criterias = static_cast<ArrayList^>(m_criteriasList[static_cast<int>(capability)]);
    criterias->Remove(criteria);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Hunter::RemoveAllCriterias()
{
    IDictionaryEnumerator^ itr = m_criteriasList->GetEnumerator();
    while (itr->MoveNext())
    {
        ArrayList^ criterias = static_cast<ArrayList^>(itr->Value);
        criterias->Clear();
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ Hunter::GetCriteriasList(FilterType filter)
{
	return static_cast<ArrayList^>(m_criteriasList[static_cast<int>(filter)]);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Hunter::RegisterBuilder(ICriteriaBuilder^ builder)
{
    if (!m_criteriasList->ContainsKey(static_cast<int>(builder->GetCapabilityType())))
    {
        m_criteriasList[static_cast<int>(builder->GetCapabilityType())] = gcnew ArrayList();
    }
	m_criteriasBuilder->Add(builder);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICriteriaBuilder^ Hunter::GetBuilder(System::String^ builderName)
{
	for each (ICriteriaBuilder^ builder in m_criteriasBuilder)
	{
		if (builder->GetName() == builderName)
		{
			return builder;
		}
	}
	return nullptr;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ Hunter::GetCriteriaBuilders()
{
	return m_criteriasBuilder;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ Hunter::GetValidImages()
{
    return m_validImages;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Hashtable^ Hunter::GetSupportedCapabilities(SupportedCapability index)
{
    if (m_supportedCapabilities->ContainsKey(static_cast<int>(index)))
    {
        return static_cast<Hashtable^>(m_supportedCapabilities[static_cast<int>(index)]);
    }
    return nullptr;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Hunter::Hunt(array<System::String^>^ paths, 
                  bool isRecursive, 
                  Hashtable^ excludedExtensions,
                  ProgressChangedEventHandler^ progressCallback, 
                  RunWorkerCompletedEventHandler^ completedCallback)
{
    // The BackgroundWorker is used to create a search thread.
    // This object uses callbacks to update the UI.
    m_isRecursiveSearch = isRecursive;
    m_ExcludedExtensions = excludedExtensions;
    m_bw = gcnew BackgroundWorker();
    m_bw->WorkerSupportsCancellation = true;
    m_bw->DoWork += gcnew DoWorkEventHandler(this, &Hunter::DoWork);

    // Progress Callback
    if (progressCallback != nullptr)
    {
        m_bw->WorkerReportsProgress = true;
        m_bw->ProgressChanged += progressCallback;
    }
    // End Of Search Callback
    if (completedCallback != nullptr) 
    {
        m_bw->RunWorkerCompleted += completedCallback;
    }
    // Starting the search
    m_bw->RunWorkerAsync(paths);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Hunter::DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e)
{
    // We count the number of files to scan then we start the search thread
    array<System::String^>^ paths = static_cast<array<System::String^>^ >(e->Argument);
    m_validImages->Clear();
    m_numberOfFiles = CountNumberOfFiles(paths);
    e->Result = m_numberOfFiles;
    m_currentFileNo = 0;
    Search(paths, m_isRecursiveSearch);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int Hunter::CountNumberOfFiles(array<System::String^>^ paths)
{
    // Recursive loop that counts files contained in the specified directories
    int numberOfFiles = 0;
    for each (System::String^ path in paths)
    {
        try
        {
            if (path != nullptr)
            {
                numberOfFiles += System::IO::Directory::GetFiles(path)->Length;
                if (m_isRecursiveSearch)
                {
                    numberOfFiles += CountNumberOfFiles(System::IO::Directory::GetDirectories(path));
                }
            }
        }
        catch (...)
        {
            // This try/catch avoids exceptions when unauthorized exceptions happen
        }
    }
    return numberOfFiles;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int Hunter::GetCurrentFileNo()
{
    return m_currentFileNo;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int Hunter::GetTotalNumberOfFiles()
{
    return m_numberOfFiles;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::String^ Hunter::GetCurrentScannedImage()
{
    return m_currentScannedFile;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Hunter::CancelHunt()
{
    m_bw->CancelAsync();
}

/*---------------------------------------------------------------------------------**//**
* Main search method
* This method calls itself recursively if asked to do so.
*
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Hunter::Search(array<System::String^>^ paths, bool isRecursive)
{
    for each (System::String^ path in paths)
    {
        try
        {
            array<System::String^>^ files = System::IO::Directory::GetFiles(path);
            for each (System::String^ file in files)
            {
                if (m_bw->CancellationPending)
                {
                    return;
                }
                // Counting files and reporting progress
                ++m_currentFileNo;
                m_currentScannedFile = file;
                m_bw->ReportProgress(100*m_currentFileNo/m_numberOfFiles);

                // Looking for excluded extensions
                if (!m_ExcludedExtensions->ContainsKey(System::IO::Path::GetExtension(file)->ToUpper()))
                {
                    // Preparing the filename for Image++
                    using namespace System::Runtime::InteropServices;
                    WString tpath;
                    HuntTools::MarshalString(file, tpath);
                    HFCPtr<HFCURLFile> SrcFileName = new HFCURLFile(
                        WString(HFCURLFile::s_SchemeName() + L"://") + tpath);
                    
                    // Opening the raster
                    HFCPtr<HRFRasterFile> pRasterFile;
                    try 
                    {
                        pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(SrcFileName.GetPtr(), HFC_READ_ONLY | HFC_SHARE_READ_ONLY);
                    
                        // Looping through search criteria groups
                        bool isValid = false;
                        IDictionaryEnumerator^ itr = m_criteriasList->GetEnumerator();
                        while (itr->MoveNext())
                        {
                            // Looping through criterias within the criteria group
                            ArrayList^ criterias = static_cast<ArrayList^>(itr->Value);
                            bool isNegativeFilterValid = true;
                            bool isValidWithAtLeastOneCriteria = false;
                            int validFilters = 0;
                            int filters = 0; // does not include negative filters

                            for each (ICriteria^ criteria in criterias)
                            {
                                isValidWithAtLeastOneCriteria = criteria->isValidWithAtLeastOne();

                                if (criteria->isRespected(pRasterFile))
                                {
                                    if (criteria->GetCriteriaOperator() != CriteriaOperator::NotEqual)
                                    {
                                        ++filters;
                                        ++validFilters;
                                    }
                                } 
                                else 
                                {
                                    if (criteria->GetCriteriaOperator() != CriteriaOperator::NotEqual)
                                    {
                                        ++filters;
                                    } 
                                    else 
                                    {
                                        isNegativeFilterValid = false;
                                        break;
                                    }
                                }
                            }

                            if ( ( (isValidWithAtLeastOneCriteria && validFilters > 0) || validFilters == filters )
                                 && isNegativeFilterValid )
                            {
                                isValid = true;
                            } 
                            else 
                            {
                                isValid = false;
                                break;
                            }
                        }

                        if (isValid)
                        {
                            // insert the image path in the list of found images
                            ImageFile^ image = gcnew ImageFile(file);
                            m_validImages->Add(image);
                        }
                    } 
                    catch (System::Runtime::InteropServices::SEHException^ e) 
                    {
                        //System::IO::StreamWriter^ out = gcnew System::IO::StreamWriter(L"error.log", true);
                        //out->WriteLine(file);
                        //out->Close();
                        Debug::WriteLine(file);
                        Debug::WriteLine(e->Message);
                    }
                    catch (System::Exception^ e)
                    {
                        Debug::WriteLine(file);
                        Debug::WriteLine(e->Message);
                    }
                }
            }
            // Recursive loop if required
            if (isRecursive)
            {
                Search(System::IO::Directory::GetDirectories(path), true);
            }
        }
        catch (System::UnauthorizedAccessException^ e)
        {
            Debug::WriteLine(e->Message);
        }
    }
}