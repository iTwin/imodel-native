/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Foundations/Definitions.h>
#include <memory>

BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE

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
                                                                                    uint32_t                        id = 0);

    FOUNDATIONS_DLLE                    ~Message                                   ();

    FOUNDATIONS_DLLE                    Message                                    (const Message&              rhs);
    FOUNDATIONS_DLLE Message&           operator=                                  (const Message&              rhs);

    FOUNDATIONS_DLLE uint32_t               GetID                                      () const;
    FOUNDATIONS_DLLE const WChar*     what                                       () const;
    };



END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
