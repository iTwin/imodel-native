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
    DATACAPTURE_EXPORT static Dgn::DgnAuthorityId QueryDataCaptureAuthorityId(Dgn::DgnDbCR dgndb);
    DATACAPTURE_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    //! Format a BeBriefcaseBasedId as BeBriefcaseId-LocalId 
    DATACAPTURE_EXPORT static Utf8String FormatId(BeSQLite::BeBriefcaseBasedId id);

    //! Build a default name from the specified prefix and BeBriefcaseBasedId
    DATACAPTURE_EXPORT static Utf8String BuildDefaultName(Utf8CP prefix, BeSQLite::BeBriefcaseBasedId id);


    }; // DataCaptureDomain

END_BENTLEY_DATACAPTURE_NAMESPACE
