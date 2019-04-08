/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Attachment.h $
|    $RCSfile: Attachment.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/08/05 00:13:01 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/SourceReference.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct AttachmentEntry
    {
    SourceRef                                   m_sourceRef;
    const void*                                 m_implP; // Reserved for further use

public:
    IMPORT_DLLE explicit                        AttachmentEntry                    (const SourceRef&                    sourceRef);
    IMPORT_DLLE                                 ~AttachmentEntry                   ();

    IMPORT_DLLE                                 AttachmentEntry                    (const AttachmentEntry&              rhs);
    IMPORT_DLLE AttachmentEntry&                operator=                          (const AttachmentEntry&              rhs);

    const SourceRef&                            GetSourceRef                       () const { return m_sourceRef; }

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct AttachmentRecord
    {
    struct                                      Impl;
    std::auto_ptr<Impl>                         m_implP;
public:
    typedef AttachmentEntry                     value_type;
    typedef value_type&                         reference;
    typedef const value_type&                   const_reference;

    typedef const value_type*                   const_iterator;

    IMPORT_DLLE explicit                        AttachmentRecord                   ();
    IMPORT_DLLE                                 ~AttachmentRecord                  ();

    IMPORT_DLLE                                 AttachmentRecord                   (const AttachmentRecord&             rhs);
    IMPORT_DLLE AttachmentRecord&               operator=                          (const AttachmentRecord&             rhs);

    IMPORT_DLLE bool                            IsEmpty                            () const;
    
    IMPORT_DLLE const_iterator                  begin                              () const;
    IMPORT_DLLE const_iterator                  end                                () const;

    IMPORT_DLLE void                            push_back                          (const AttachmentEntry&              entry);
    };



END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
