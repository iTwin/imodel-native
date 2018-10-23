/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnV8OpenRoadsDesigner/DgnV8OpenRoadsDesignerDomain.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "DgnV8OpenRoadsDesigner.h"

BEGIN_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE

//=======================================================================================
//! The DgnDomain for the DgnV8OpenRoadsDesigner schema.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct DgnV8OpenRoadsDesignerDomain : Dgn::DgnDomain
{
DOMAIN_DECLARE_MEMBERS(DgnV8OpenRoadsDesignerDomain, DGNV8OPENROADSDESIGNER_EXPORT)

public:
    //! @private
    DgnV8OpenRoadsDesignerDomain();

private:
    WCharCP _GetSchemaRelativePath() const override { return V8ORD_SCHEMA_PATH; }
}; // DgnV8OpenRoadsDesignerDomain

END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE
