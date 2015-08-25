/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/ElemSourceRef.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>

#include <ScalableMesh\Import\SourceReference.h>
#include <ScalableMesh\Import\Plugin\SourceReferenceV0.h>
#include <ScalableMesh\ScalableMeshLib.h>

#include "ElemSourceRef.h"
#include "DGNModelUtilities.h"


//#include <DcInternal\DcStmCore\ScalableMeshUtil.h>


#define PointCloudMinorId_Handler 1

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_POINTCLOUD
//USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT

 
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Jean-Francois.Cote   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNElemSourceRefBase::Impl
    {
    UInt                            m_elemType;
    UInt                            m_elemHandlerID;
    ElementRefP                      m_elemRef;
    DGNModelRefHolder                  m_modelRef;
    auto_ptr<LocalFileSourceRef>    m_localFileRefP;

    explicit                        Impl                           (UInt                        elemType,
                                                                    UInt                        elemHandlerID,
                                                                    ElementRefP                  elemRef,
                                                                    const DGNModelRefHolder&    modelRef,
                                                                    LocalFileSourceRef*         localFileRefP)
        :   m_elemType(elemType),
            m_elemHandlerID(elemHandlerID),
            m_elemRef(elemRef),
            m_modelRef(modelRef),
            m_localFileRefP(localFileRefP)
        {
        }

                                    Impl                           (const Impl&         rhs)
        :   m_elemType(rhs.m_elemType),
            m_elemHandlerID(rhs.m_elemHandlerID),
            m_elemRef(rhs.m_elemRef),
            m_modelRef(rhs.m_modelRef),
            m_localFileRefP(new LocalFileSourceRef(*rhs.m_localFileRefP))
        {
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef DGNElemSourceRefBase::CreateFromImpl (UInt                                elemType,
                                                UInt                                elemHandlerID,
                                                ElementRefP                          elemRef,
                                                const DGNModelRefHolder&            modelRef,
                                                LocalFileSourceRef*                 localFileRefP)
    {
    assert(elemType == elemRef->GetElementType());

    return DGNElementSourceRef::CreateFrom(new DGNElemSourceRefBase(new Impl(elemType, elemHandlerID, elemRef, modelRef, localFileRefP)));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElemSourceRefBase::DGNElemSourceRefBase (const DGNElemSourceRefBase& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElemSourceRefBase::DGNElemSourceRefBase (Impl*                       implP)
    :   m_implP(implP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElemSourceRefBase::~DGNElemSourceRefBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElemSourceRefBase* DGNElemSourceRefBase::_Clone () const
    {
    return new DGNElemSourceRefBase(*this);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt DGNElemSourceRefBase::_GetElementType () const
    {
    return m_implP->m_elemType;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt DGNElemSourceRefBase::_GetElementHandlerID () const
    {
    return m_implP->m_elemHandlerID;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP DGNElemSourceRefBase::_GetElementRef () const
    {
    return m_implP->m_elemRef;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelRefP DGNElemSourceRefBase::_GetModelRef () const
    {
    return m_implP->m_modelRef.GetP();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const LocalFileSourceRef* DGNElemSourceRefBase::_GetLocalFileP () const
    {
    return m_implP->m_localFileRefP.get();
    }



namespace {

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline wstring ExtractFileFolderPathFromFilePath (const WChar* filePath)
    {
    struct IsDirSeparator
        {
        bool operator () (WChar c) const { return (L'\\' == c || L'/' == c); }
        };

    typedef reverse_iterator<WCharCP> RevWCharIt;

    RevWCharIt foundSeparatorIt = find_if (RevWCharIt(filePath + wcslen(filePath)),
                                           RevWCharIt(filePath),
                                           IsDirSeparator());
    assert(foundSeparatorIt != RevWCharIt(filePath));
    return wstring(filePath, foundSeparatorIt.base());
    }

}


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef RasterElemSourceRef::CreateFrom(const ElementRefP& elemRef, const DGNModelRefHolder& modelRef)
    {
    USING_NAMESPACE_RASTER

    EditElementHandle elHandle(elemRef, modelRef.GetP());    

    IRasterAttachmentEdit* pEdit = dynamic_cast<IRasterAttachmentEdit*>(&elHandle.GetHandler());
    assert(pEdit != NULL);        
    
    const WString rasterElementFilePath(pEdit->GetFilespec(elHandle).erase(0, 7).c_str());
    
    WString rasterDgnFilePath = modelRef.GetP()->GetDgnFileP()->GetFileName();

    const WString dgnFolderPath(ExtractFileFolderPathFromFilePath(rasterDgnFilePath.c_str()).c_str());
    
    DgnDocumentMonikerPtr monikerPtr(DgnDocumentMoniker::CreateFromFileName(rasterElementFilePath.c_str(),
                                                                            dgnFolderPath.c_str()));

    assert(0 != monikerPtr.get());
    StatusInt status = BSIERROR;
    DgnDocumentPtr docPtr(DgnDocument::CreateFromMoniker(status, *monikerPtr, 0, DgnDocument::FetchMode::InfoOnly));

    auto_ptr<LocalFileSourceRef> fileSourceRefP(0 != docPtr.get() ?
                                                    new LocalFileSourceRef(docPtr->GetFileName().c_str()) :
                                                    0);

    // TDORAY: Cascade base dir up until root? Rasterlib seems to do that as it is able to find the raster in any dgn
    // folder of the hierarchy. It would be costly to do so with the moniker interface tough... For the moment, we will
    // only consider the path of the dgn to which the raster is attached.

    return CreateFromImpl(RASTER_FRAME_ELM,
                          0, // No handler Id as this is not an element 106
                          elemRef,
                          modelRef,
                          fileSourceRefP.release());
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef CivilElemSourceRef::CreateFrom(const ElementRefP& elemRef, const DGNModelRefHolder& modelRef)
    {
    return CreateFromImpl(EXTENDED_ELM,
                          DTMElementHandler::GetElemHandlerId().GetId(),
                          elemRef,
                          modelRef,
                          0);
    }
    

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef STMElemSourceRef::CreateFrom(const ElementRefP& elemRef, const DGNModelRefHolder& modelRef)
    {
    EditElementHandle elHandle(elemRef, modelRef.GetP());

    WString fileName; 
        
    StatusInt status = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._ResolveMrDtmFileName(fileName, elHandle);
               
    assert(status == SUCCESS);
   
    auto_ptr<LocalFileSourceRef> fileSourceRefP(fileName.empty() ? 0 : new LocalFileSourceRef(fileName.c_str()));

    return CreateFromImpl(EXTENDED_ELM,
                          MrDTMDefaultElementHandler::GetElemHandlerId().GetId(),
                          elemRef,
                          modelRef,
                          fileSourceRefP.release());
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef PODElemSourceRef::CreateFrom(const ElementRefP& elemRef, const DGNModelRefHolder& modelRef)
    {    
    EditElementHandle elHandle(elemRef, modelRef.GetP());
    IPointCloudFileQueryPtr fileQueryPtr = IPointCloudFileQuery::CreateFileQuery(elHandle);

    // TODRAY: Investigate whether we really need to extract the file path for POD elements. I do believe that we
    // use the element directly...

    //Note: ElementHandlerId is hardcoded here because PointCloudHandler::GetElemHandlerId() is not exported.
    return CreateFromImpl(EXTENDED_ELM,
                          ElementHandlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_Handler).GetId(),
                          elemRef,
                          modelRef,
                          (0 != fileQueryPtr.get()) ? new LocalFileSourceRef(fileQueryPtr->GetFileName()) : 0);
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
