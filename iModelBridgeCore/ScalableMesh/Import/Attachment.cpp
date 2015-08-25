/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/Attachment.cpp $
|    $RCSfile: Attachment.cpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/08/05 00:12:45 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include <ScalableMesh/Import/Attachment.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentEntry::AttachmentEntry (const SourceRef& sourceRef)
    :   m_sourceRef(sourceRef),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentEntry::~AttachmentEntry ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentEntry::AttachmentEntry (const AttachmentEntry& rhs)
    :   m_sourceRef(rhs.m_sourceRef),
        m_implP(0)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentEntry& AttachmentEntry::operator= (const AttachmentEntry& rhs)
    {
    m_sourceRef = rhs.m_sourceRef;
    return *this;
    }


struct AttachmentRecord::Impl
    {
    typedef bvector<AttachmentEntry>    EntryList;
    EntryList                           m_entries;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentRecord::AttachmentRecord ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentRecord::~AttachmentRecord  ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentRecord::AttachmentRecord (const AttachmentRecord& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentRecord& AttachmentRecord::operator= (const AttachmentRecord& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool AttachmentRecord::IsEmpty () const
    {
    return m_implP->m_entries.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentRecord::const_iterator AttachmentRecord::begin () const
    {
    return &*m_implP->m_entries.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentRecord::const_iterator AttachmentRecord::end () const
    {
    return &*m_implP->m_entries.end();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void AttachmentRecord::push_back (const AttachmentEntry& entry)
    {
    m_implP->m_entries.push_back(entry);
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
