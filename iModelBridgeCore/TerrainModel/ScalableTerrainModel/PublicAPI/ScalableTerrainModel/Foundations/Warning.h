/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Foundations/Warning.h $
|    $RCSfile: Warning.h,v $
|   $Revision: 1.6 $
|       $Date: 2011/10/25 18:53:45 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Foundations/Definitions.h>

// NTERAY: See if Bentley.h's forward declaration may suffice.
#include <Bentley/WString.h>
#include <assert.h>
#include <memory>

BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Warning : private Unassignable
    {
    enum Level
        {
        LEVEL_0,
        LEVEL_1,
        LEVEL_2,
        LEVEL_3,
        LEVEL_4,
        };

private:
    friend struct                       WarningItem;

    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;
    Level                               m_level;                                 

    virtual Warning*                    _Clone                             () const = 0;

protected:
    FOUNDATIONS_DLLE explicit           Warning                            (const WChar*          msg,
                                                                            Level                   level = LEVEL_0,
                                                                            StatusInt               errorCode = 0);

    FOUNDATIONS_DLLE                    Warning                            (const Warning&          rhs);

public:
    FOUNDATIONS_DLLE virtual            ~Warning                           () = 0;

    

    FOUNDATIONS_DLLE StatusInt          GetErrorCode                       () const;
    FOUNDATIONS_DLLE const WChar*     what                               () const;

    Level                               GetLevel                           () const { return m_level; }

    };


/*---------------------------------------------------------------------------------**//**
* @description  Template base class for warning that automatically implement the
*               virtual constructor pattern assuming that derived WarningT provides
*               a valid copy constructor.
*    
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename WarningT>
struct WarningMixinBase : public Warning
    {
private:
    virtual Warning*                    _Clone                             () const override
        {
        return new WarningT(static_cast<const WarningT&>(*this));
        }

protected:
    typedef WarningMixinBase<WarningT>  super_class;

    explicit                            WarningMixinBase                   (const WChar*          msg,
                                                                            Level                   level = LEVEL_0,
                                                                            StatusInt               errorCode = 0)
        :   Warning(msg, level, errorCode) {}

    virtual                             ~WarningMixinBase                  () = 0 {};

    };


/*
 * Custom warnings
 */
struct CustomWarning : public WarningMixinBase<CustomWarning>
    {
    explicit                            CustomWarning                      (const WChar*          msg,
                                                                            Level                   level = LEVEL_0,
                                                                            StatusInt               errorCode = 0)   
        :   super_class(msg, level, errorCode) {}

    };



END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE