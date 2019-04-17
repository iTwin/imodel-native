/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "ArchitecturalPhysicalDefinitions.h"

BEGIN_BENTLEY_NAMESPACE

namespace ArchitecturalPhysical
	{

	//=======================================================================================
	//! The DgnDomain for the Architectural Physical schema.
	//! @ingroup GROUP_ArchitecturalPhyscal
	//=======================================================================================
	struct ArchitecturalPhysicalDomain : Dgn::DgnDomain
		{
		DOMAIN_DECLARE_MEMBERS(ArchitecturalPhysicalDomain, ARCHITECTURAL_PHYSICAL_EXPORT)

		protected:
			WCharCP _GetSchemaRelativePath() const override { return BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_PATH; }
			virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
			virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;
			static void InsertDomainCodeSpecs(Dgn::DgnDbR db);

		public:
			ArchitecturalPhysicalDomain();
			ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::CodeSpecId QueryArchitecturalPhysicalCodeSpecId(Dgn::DgnDbCR dgndb);
		//	ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnDbPtr   CreateBuildingPartionsAndModels(DgnDbPtr db, Utf8StringCR buildingCodeName, Utf8CP description = NULL);
		};



	//=======================================================================================
	// @bsiclass                                    BentleySystems
	//=======================================================================================
	struct  ArchitecturalPhysicalCategory : NonCopyableClass
		{
		friend struct ArchitecturalPhysicalDomain;

		public:


		private:

			static void InsertDomainCategories(Dgn::DgnDbR);
			static Dgn::DgnCategoryId InsertCategory(Dgn::DgnDbR, Utf8CP, Dgn::ColorDef const&);
			static Dgn::DgnSubCategoryId InsertSubCategory(Dgn::DgnDbR, Dgn::DgnCategoryId, Utf8CP, Dgn::ColorDef const&);

		public:
			//! Get the DgnCategoryId to be used for Door Elements
#ifdef NOT_NOW
			ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnCategoryId QueryBuildingPhysicalDoorCategoryId(Dgn::DgnDbR);

			//! Get the DgnSubCategoryId for a Door Panel
			ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnSubCategoryId QueryBuildingPhysicalDoorPanelSubCategoryId(Dgn::DgnDbR);

			//! Get the DgnSubCategoryId for a Door frame
			ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnSubCategoryId QueryBuildingPhysicalDoorFrameSubCategoryId(Dgn::DgnDbR);

			//! Get the DgnCategoryId to be used for Window Elements
			ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnCategoryId QueryBuildingPhysicalWindowCategoryId(Dgn::DgnDbR);

			//! Get the DgnSubCategoryId for a Door Panel
			ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnSubCategoryId QueryBuildingPhysicalWindowPanelSubCategoryId(Dgn::DgnDbR);

			//! Get the DgnSubCategoryId for a Door frame
			ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnSubCategoryId QueryBuildingPhysicalWindowFrameSubCategoryId(Dgn::DgnDbR);

			//! Get the DgnSubCategoryId for a Door frame
			ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnCategoryId QueryBuildingPhysicalWallCategoryId(Dgn::DgnDbR db);
#endif
			//! Get the DgnSubCategoryId for a Door frame
			ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnCategoryId QueryBuildingPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName );
			ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnCategoryId QueryBuildingDrawingCategoryId(Dgn::DgnDbR db, Utf8CP categoryName);

		};

	} // End ArchitecturalPhysical namespace

END_BENTLEY_NAMESPACE



