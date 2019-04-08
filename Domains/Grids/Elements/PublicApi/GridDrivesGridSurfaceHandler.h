/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridDrivesGridSurfaceHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

struct GridDrivesGridSurfaceHandler : Dgn::ElementDependency::Handler
{
private: 
    DOMAINHANDLER_DECLARE_MEMBERS (GRIDS_REL_GridDrivesGridSurface, GridDrivesGridSurfaceHandler, Dgn::DgnDomain::Handler, GRIDELEMENTS_EXPORT)

private:
    //! Returns relationship class
    //! @return a reference to GridDrivesGridSurface relationship class
    static ECN::ECRelationshipClassCR GetECClass(Dgn::DgnDbR db);

protected:
    void _OnRootChanged (Dgn::ElementDependency::Graph const& graph, Dgn::ElementDependency::Edge const& edge) override;
    void _OnDeletedDependency (Dgn::ElementDependency::Graph const& graph, Dgn::ElementDependency::Edge const& edge) override;

public:
    //! Inserts relationship between grid surfaces and created grid curve
    //! @param[in]  db              db to create relationship in
    //! @param[in]  thisSurface     first surface creating grid intersection curve
    //! @param[in]  otherSurface    other surface creating grid intersection curve. This is the base surface in the relationship
    //! @param[in]  targetModel     model to create new grid curve in
    //! @return key to created relationship
    GRIDELEMENTS_EXPORT static BeSQLite::EC::ECInstanceKey Insert (Dgn::DgnDbR db, GridCR thisGrid, GridSurfaceCR surface);
};

END_GRIDS_NAMESPACE