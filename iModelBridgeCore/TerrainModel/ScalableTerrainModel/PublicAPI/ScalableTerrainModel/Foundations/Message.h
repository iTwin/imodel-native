/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Foundations/Message.h $
|    $RCSfile: Message.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/09/07 14:20:58 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Foundations/Definitions.h>
#include <memory>

BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Message
    {
private:
    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

public:
    FOUNDATIONS_DLLE explicit           Message                                    (const WChar*              msg, 
                                                                                    UInt                        id = 0);

    FOUNDATIONS_DLLE                    ~Message                                   ();

    FOUNDATIONS_DLLE                    Message                                    (const Message&              rhs);
    FOUNDATIONS_DLLE Message&           operator=                                  (const Message&              rhs);

    FOUNDATIONS_DLLE UInt               GetID                                      () const;
    FOUNDATIONS_DLLE const WChar*     what                                       () const;
    };



END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE