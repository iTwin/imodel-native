/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! An ImagePPHost is an object that uniquely identifies a usage of the ImagePP libraries 
//! for a single purpose. For a given process, there can be more than one ImagePPHost, 
//! but each ImagePPHost must be on a different thread. ImagePPHost holds a collection 
//! of key/pointer pairs that are used to store and retrieve host-based data.
//!
//! @bsiclass                                   Jean-Francois.Cote              03/2015
//=====================================================================================
struct MyImageppLibHost : ImagePP::ImageppLib::Host
    {
    MyImageppLibHost();

    //! Supply the ImageppAdmin for this session. This method is guaranteed to be called once and never again.
    virtual ImagePP::ImageppLibAdmin&   _SupplyImageppLibAdmin() override;

    //! Register supported file format for this session. This method is guaranteed to be called once and never again.
    virtual void                        _RegisterFileFormat() override;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              03/2015
//=====================================================================================
struct MyImageppLibAdmin : ImagePP::ImageppLibAdmin
    {
    DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)
    virtual ~MyImageppLibAdmin() {}
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE