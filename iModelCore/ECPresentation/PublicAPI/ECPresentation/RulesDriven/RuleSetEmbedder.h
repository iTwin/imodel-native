/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once 
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! Embeds rulesets into ECDb.
// @bsiclass                                        Saulius.Skliutas            10/2017
//=======================================================================================
struct RuleSetEmbedder      
{
ECPRESENTATION_EXPORT static const Utf8CP FILE_TYPE_PresentationRuleSet;

private:
    BeSQLite::EC::ECDbR m_connection;

public:
    //! Constructor.
    //! @param[in] connection The ECDb to which ruleset should be embedded.
    RuleSetEmbedder(BeSQLite::EC::ECDbR connection) : m_connection(connection) {}

    //! Embeds provided ruleset into ECDb.
    ECPRESENTATION_EXPORT BeSQLite::DbResult Embed(PresentationRuleSetR ruleset);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE