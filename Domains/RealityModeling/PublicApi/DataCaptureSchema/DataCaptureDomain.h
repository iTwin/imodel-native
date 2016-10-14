/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/DataCaptureSchema/DataCaptureDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

//=======================================================================================
//! The DgnDomain for the DataCapture schema.
//! @ingroup GROUP_DataCapture
//=======================================================================================
struct DataCaptureDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(DataCaptureDomain, DATACAPTURE_EXPORT)

private:
    static Dgn::DgnCategoryId QueryCategoryId(Dgn::DgnDbCR, Utf8CP);

protected:
    void _OnSchemaImported(Dgn::DgnDbR dgndb) const override;

public:
    DataCaptureDomain();

    }; // DataCaptureDomain

END_BENTLEY_DATACAPTURE_NAMESPACE
