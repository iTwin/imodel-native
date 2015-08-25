/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Config/Attachments.h $
|    $RCSfile: Attachments.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/22 21:57:46 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/Config/Base.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct AttachmentsConfig : public ImportConfigComponentMixinBase<AttachmentsConfig>
    {
private:
    bool                                    m_imported;
    void*                                   m_implP; // Reserve space for further use

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    AttachmentsConfig              (bool                            imported);
    IMPORT_DLLE virtual                     ~AttachmentsConfig             ();

    IMPORT_DLLE                             AttachmentsConfig              (const AttachmentsConfig&        rhs);


    bool                                    AreImported                    () const;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
