/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/PSSUtilities.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ImageConverterPch.h"

#include <tchar.h>

#include <Imagepp/all/h/HGF2DCoord.h>       // Needed to remove type error !!!
#include "DirectoryScanner.h"
#include "PSSUtilities.h"

//Imagepp
#include <Imagepp/all/h/HFCLocalBinStream.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HPSObjectStore.h>
//#include <all/gra/hps/src/HPSParserScope.h>
#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>

//GetRasterSizeInPixel
#include <Imagepp/all/h/HFCGrid.h>
#include <Imagepp/all/h/HIMStoredRasterEquivalentTransfo.h>

//-----------------------------------------------------------------------------
// GenerateOnDemandMosaicPssFile
//-----------------------------------------------------------------------------
void GenerateOnDemandMosaicPssFile(Utf8String& pi_rpFolderName,                                   
                                   Utf8String& pi_rpPSSFileName)
{    
    HFCPtr<HRFRasterFile> pRasterFile;   
    HFCPtr<HFCURLFile>    pURL;
    DirectoryScanner      DirScanner;
    WChar                Buffer[1000];
    
    DirectoryScanner::FILELIST           FileList(DirScanner.GetFileList(pi_rpFolderName));    
    DirectoryScanner::FILELIST::iterator FileIter    = FileList.begin();
    DirectoryScanner::FILELIST::iterator FileIterEnd = FileList.end();
    uint32_t                            FileInd     = 0;

    //Remove the PSS file if any.
    _tremove(WString(pi_rpPSSFileName.c_str(), BentleyCharEncoding::Utf8).c_str());
    
    HFCLocalBinStream  PSSFile(pi_rpPSSFileName);
            
    while (FileIter != FileIterEnd)
    {
        try 
        {
            pURL = (HFCURLFile*)HFCURL::Instanciate("file://" + *FileIter);
            HASSERT(pURL != 0);

            pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>&)pURL, 
                                                                        true);
        }
        catch(HFCException&)
        {
        }
        
        if (pRasterFile != 0)
        {            
            _stprintf(Buffer, 
                      _TEXT("r%i = IMF(\"file://%s\", 0)\r\n"),
                      FileInd++, 
                      WString((*FileIter).c_str(), BentleyCharEncoding::Utf8).c_str());

            Utf8String pssDataUTF8(Buffer);
            PSSFile.Write(pssDataUTF8.c_str(), pssDataUTF8.size());            
        }
        
        FileIter++;
    }

    Utf8String TextBuffer;

    TextBuffer = "m0 = ODMO(";

    uint32_t MosaicFileInd = 0;

    for (; MosaicFileInd < FileInd - 1; MosaicFileInd++)
    {
        TextBuffer += Utf8String(_TEXT("r")) + 
                      Utf8String(_ultot(MosaicFileInd, Buffer, 10)) + 
                      Utf8String(_TEXT(", "));
    }

    TextBuffer += Utf8String(_TEXT("r")) + 
                  Utf8String(_ultot(MosaicFileInd, Buffer, 10)) + 
                  Utf8String(_TEXT(")\r\n")) + 
                  Utf8String(_TEXT("PG(m0)\r\n"));

    Utf8String pssDataUTF8(TextBuffer);
    PSSFile.Write(pssDataUTF8.c_str(), pssDataUTF8.size());
}


//-----------------------------------------------------------------------------
// ReplacePSSMosaicByOnDemandMosaic
//-----------------------------------------------------------------------------
void ReplacePSSMosaicByOnDemandMosaic(HFCPtr<HFCURL>          pi_pFileName, 
                                      HFCPtr<HPSObjectStore>& pi_rpObjectStore)
{    
    HASSERT(!"Cannot use PSS internal(all/gra/hps/src/HPSParserScope.h)"); // Should we make HPSParserScope.h public?
#if defined(NOT_NOW)
    HPRECONDITION(pi_rpObjectStore->CountPages() >= 1);
    HPRECONDITION(pi_rpObjectStore->GetPageStatementNode(0)->GetSubNodes().size() >= 3);

    MosaicExpressionNode*       pMosaicExpressNode = 0; 
    const PageStatementNode*    pPageStatementNode = pi_rpObjectStore->GetPageStatementNode(0);
    const HPANodeList&          rNodeList(pPageStatementNode->GetSubNodes()[2]->GetSubNodes());    
    HPANodeList::const_iterator NodeIter(rNodeList.begin());
    HPANodeList::const_iterator NodeIterEnd(rNodeList.end());
    
    while (NodeIter != NodeIterEnd)
    {        
        if (dynamic_cast<VariableRefNode*>((*NodeIter).GetPtr()) != 0)
        {
            const HPANodeList& rNodeList = ((HFCPtr<VariableRefNode>&)(*NodeIter))->
                                                                   GetVariableTokenNode()->
                                                                   GetExpressionNode()->GetSubNodes();
            
            NodeIter    = rNodeList.begin();
            NodeIterEnd = rNodeList.end();            
        }
        else
        {
            //There should always be one node to the mosaic node.        
            HASSERT((*NodeIter)->GetSubNodes().size() == 1);

            HFCPtr<HPANode> pNode = (*NodeIter)->GetSubNodes()[0];

            pMosaicExpressNode = dynamic_cast<MosaicExpressionNode*>(pNode.GetPtr());

            if (pMosaicExpressNode != 0)
            {
                break;
            }            
            else
            {
                NodeIter    = pNode->GetSubNodes().begin();
                NodeIterEnd = pNode->GetSubNodes().end();                            
            }
        }        
    }

    HPASourcePos MosaicStatementStartPos(pMosaicExpressNode->GetStartPos());
    HPASourcePos MosaicStatementEndPos(pMosaicExpressNode->GetEndPos());

    pi_rpObjectStore = 0;
    
    if (pMosaicExpressNode != 0)
    {
        HAutoPtr<HFCBinStream> pFileStream(HFCBinStream::Instanciate(pi_pFileName->GetURL(), HFC_READ_WRITE | HFC_SHARE_READ_ONLY));       

        HArrayAutoPtr<Byte> pDataBuffer(new Byte[(size_t)pFileStream->GetSize()]);
        size_t               NbBytesRead = pFileStream->Read(pDataBuffer.get(), 
                                                             (size_t)pFileStream->GetSize());
       
        HASSERT(NbBytesRead == pFileStream->GetSize());

        HFCPtr<HFCBuffer> pStreamBuffer(new HFCBuffer(pDataBuffer.release(), 
                                                      (size_t)pFileStream->GetSize()));
        
        HAutoPtr<HFCMemoryLineStream> pLineStream(new HFCMemoryLineStream(pi_pFileName->GetURL(),
                                                                          '\n', 
                                                                          pStreamBuffer));
    
        HASSERT(MosaicStatementStartPos.m_Line == MosaicStatementEndPos.m_Line);

        uint64_t LinePosStart = pLineStream->GetLinePos(MosaicStatementStartPos.m_Line - 1);     
        uint64_t LinePosEnd   = pLineStream->GetLinePos(MosaicStatementStartPos.m_Line);     

        if (LinePosEnd == _UI64_MAX)
        {            
            LinePosEnd = pFileStream->GetSize();
        }
        
        HArrayAutoPtr<char> pMosaicLine(new char[(size_t)(LinePosEnd - LinePosStart + 1)]);        
        memcpy(pMosaicLine.get(),  &(pStreamBuffer->GetData()[LinePosStart]), (size_t)(LinePosEnd - LinePosStart));
        pMosaicLine[LinePosEnd - LinePosStart] = '\0';

        _strupr_s(pMosaicLine, (size_t)(LinePosEnd - LinePosStart + 1));                

        bool IsAbbrUsed          = false;
        char* pMosaicStatementPos = strstr(pMosaicLine.get(), "MOSAIC");

        if (pMosaicStatementPos == 0)
        {
            pMosaicStatementPos = strstr(pMosaicLine.get(), "MO");
            IsAbbrUsed          = true;
        }

        string  OnDemandMosaicStatement;
        uint64_t MosaicStatementPosStart = LinePosStart + (pMosaicStatementPos - pMosaicLine.get());
        uint64_t MosaicStatementPosEnd   = MosaicStatementPosStart + (IsAbbrUsed ? 2 : 6);
                        
        pFileStream->SeekToPos(MosaicStatementPosStart);
        
        if (IsAbbrUsed == true)
        {
            OnDemandMosaicStatement = "ODMO";            
        }
        else
        {
            OnDemandMosaicStatement = "OnDemandMosaic";        
        }        

        pFileStream->Write((void*)OnDemandMosaicStatement.c_str(), OnDemandMosaicStatement.size());
        pFileStream->Write(&pStreamBuffer->GetData()[MosaicStatementPosEnd], (size_t)(pStreamBuffer->GetDataSize() - MosaicStatementPosEnd));        
    }
#endif
}

//-----------------------------------------------------------------------------
// IsValidPSSCacheFile
//-----------------------------------------------------------------------------
bool IsValidPSSCacheFile(HFCPtr<HFCURL>&            pi_rpSrcFileName, 
                          HFCPtr<HIMOnDemandMosaic>& pi_rpOnDemandMosaic)
{        
    bool IsValidCache = false;

    if (pi_rpOnDemandMosaic->HasCache() == true)
    {           
        HFCPtr<HFCURL>    pCacheUrl(HRFiTiffCacheFileCreator::GetInstance()->GetCacheURLFor(pi_rpSrcFileName));    
        HFCStat           CacheFileStat(pCacheUrl);
        ListOfRelatedURLs SourceFileURLs;

        pi_rpOnDemandMosaic->GetSourceFileURLs(SourceFileURLs);        

        ListOfRelatedURLs::const_iterator URLIter    = SourceFileURLs.begin();
        ListOfRelatedURLs::const_iterator URLIterEnd = SourceFileURLs.end();

        while (URLIter != URLIterEnd)
        {
            HFCStat SourceFileStat(*URLIter);

            if (HRFiTiffCacheFileCreator::GetInstance()->
                            IsModificationTimeValid(SourceFileStat.GetModificationTime(), 
                                                    CacheFileStat.GetModificationTime()) == false)
            {
                break;
            }         

            URLIter++;
        }             

        if (URLIter == URLIterEnd)
        {
            IsValidCache = true;
        }
    }    

    return IsValidCache;
} 

//-----------------------------------------------------------------------------
// GetRasterSizeInPixel : Based on the HUTImportFromRasterExportToFile 
//                        constructor.
//-----------------------------------------------------------------------------
void GetRasterSizeInPixel(HFCPtr<HRARaster>                pi_pRaster, 
                          const HFCPtr<HGF2DWorldCluster>& pi_pWorldCluster, 
                          uint64_t&                         po_rHeight, 
                          uint64_t&                         po_rWidth)
{   
    // A raster must be provided
    HPRECONDITION(pi_pRaster != 0);

    // A cluster must be provided
    HPRECONDITION(pi_pWorldCluster != 0);    
    
    HFCPtr<HVEShape> pEffectiveShape(pi_pRaster->GetEffectiveShape());
    
    HASSERT(!pEffectiveShape->IsEmpty());
                                        
    HGF2DPosition OriginalSize;
    HVEShape      ClipShape(*pEffectiveShape);    

    HIMStoredRasterEquivalentTransfo SRETransfo(pi_pRaster);
    if (SRETransfo.EquivalentTransfoCanBeComputed())
    {
        HVEShape ClipShapeOrig(*pEffectiveShape);
        SRETransfo.TransformLogicalShapeIntoPhysical(ClipShapeOrig);

        HFCGrid OriginalGrid(0.0, 
                             0.0,
                             ClipShapeOrig.GetExtent().GetWidth(), 
                             ClipShapeOrig.GetExtent().GetHeight());

        // Set Original size in HRFImportExport
        CHECK_HSINT64_TO_HDOUBLE_CONV(OriginalGrid.GetWidth())
        CHECK_HSINT64_TO_HDOUBLE_CONV(OriginalGrid.GetHeight())

        OriginalSize.SetX((double)OriginalGrid.GetWidth()); 
        OriginalSize.SetY((double)OriginalGrid.GetHeight());     
    }   

    // Calculate the resample size and scale factor.
    ClipShape.ChangeCoordSys(pi_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
    HGF2DExtent TmpExtentMin;
    HGF2DExtent TmpExtentMax;
    pi_pRaster->GetPixelSizeRange(TmpExtentMin, TmpExtentMax);

    // The minimum pixel size must be defined and its width and height must be greater than 0.0
    HASSERT(TmpExtentMin.IsDefined());                                                
    HASSERT(TmpExtentMin.GetWidth() != 0.0);    
    HASSERT(TmpExtentMin.GetHeight() != 0.0);    

    HGF2DExtent PixelSize(TmpExtentMin.CalculateApproxExtentIn(pi_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD)));

    // PixelSize Area.
    double DefaultResampleScaleFactor = min(PixelSize.GetWidth(), PixelSize.GetHeight());

    // The pixel size may not be 0.0 (exact compare)
    HASSERT(DefaultResampleScaleFactor != 0.0);

    // ScaleFactor                   
    DefaultResampleScaleFactor = 1.0 / DefaultResampleScaleFactor;     
   
    // Exceptionally, we specify a precision. We don't want to create
    // pixels that are not useful. 0.01 is quite arbitrary ;-)
    HFCGrid ResampleGrid(0.0, 
                         0.0,
                         ClipShape.GetExtent().GetWidth()  * DefaultResampleScaleFactor, 
                         ClipShape.GetExtent().GetHeight() * DefaultResampleScaleFactor,
                         0.01);

    // Set resample size in HRFImportExport.
    po_rWidth = ResampleGrid.GetWidth();
    po_rHeight = ResampleGrid.GetHeight();        
}