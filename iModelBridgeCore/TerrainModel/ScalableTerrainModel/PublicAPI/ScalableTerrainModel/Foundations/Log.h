/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Foundations/Log.h $
|    $RCSfile: Log.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/10/20 18:48:38 $
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

struct Warning;
struct Error;
struct Message;

struct Log;


FOUNDATIONS_DLLE Log&                   GetDefaultLog                      ();

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Log : private Unassignable
    {
private:
    const void*                         m_implP; // Reserve some space for further use

    virtual Log*                        _Clone                             () const = 0;

    virtual void                        _Add                               (const Error&                error) = 0;
    virtual void                        _Add                               (const Warning&              warning) = 0;
    virtual void                        _Add                               (const Message&              message) = 0;


protected:
    FOUNDATIONS_DLLE explicit           Log                                ();
    FOUNDATIONS_DLLE                    Log                                (const Log&                  rhs);
public:    
    FOUNDATIONS_DLLE virtual            ~Log                               () = 0;

    FOUNDATIONS_DLLE Log*               Clone                              () const;

    FOUNDATIONS_DLLE void               Add                                (const Error&                error);
    FOUNDATIONS_DLLE void               Add                                (const Warning&              warning);
    FOUNDATIONS_DLLE void               Add                                (const Message&              message);
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct WarningItem
    {
private:
    std::auto_ptr<Warning>              m_warningP;

public:
    // Can be implicitly created from a Warning
    FOUNDATIONS_DLLE                    WarningItem                        (const Warning&              warning);
    FOUNDATIONS_DLLE                    ~WarningItem                       ();

    FOUNDATIONS_DLLE                    WarningItem                        (const WarningItem&          rhs);
    FOUNDATIONS_DLLE WarningItem&       operator=                          (const WarningItem&          rhs);

    FOUNDATIONS_DLLE StatusInt          GetErrorCode                       () const;
    FOUNDATIONS_DLLE const WChar*     what                               () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ErrorItem
    {
private:
    std::auto_ptr<Error>                m_errorP;

public:
    // Can be implicitly created from an Error or an Exception
    FOUNDATIONS_DLLE                    ErrorItem                          (const Error&                error);
    FOUNDATIONS_DLLE                    ~ErrorItem                         ();

    FOUNDATIONS_DLLE                    ErrorItem                          (const ErrorItem&            rhs);
    FOUNDATIONS_DLLE ErrorItem&         operator=                          (const ErrorItem&            rhs);

    FOUNDATIONS_DLLE StatusInt          GetErrorCode                       () const;
    FOUNDATIONS_DLLE const WChar*     what                               () const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
typedef Message                         MessageItem;

END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE