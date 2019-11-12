/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "ElectricalPhysicalDefinitions.h"

BEGIN_BENTLEY_NAMESPACE

namespace ElectricalPhysical
	{

	//=======================================================================================
	//! The DgnDomain for the Electrical Physical schema.
	//=======================================================================================
	struct ElectricalPhysicalDomain : Dgn::DgnDomain
		{
		DOMAIN_DECLARE_MEMBERS(ElectricalPhysicalDomain, ELECTRICAL_PHYSICAL_EXPORT)
		protected:
			WCharCP _GetSchemaRelativePath() const override { return BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA_PATH; }
			virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
			virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;
			static void InsertDomainCodeSpecs(Dgn::DgnDbR db);

		public:
			ElectricalPhysicalDomain();
			ELECTRICAL_PHYSICAL_EXPORT static Dgn::CodeSpecId QueryElectricalPhysicalCodeSpecId(Dgn::DgnDbCR dgndb);
			ELECTRICAL_PHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
		};

	//=======================================================================================
	// @bsiclass                                    BentleySystems
	//=======================================================================================
	struct  ElectricalPhysicalCategory : NonCopyableClass
		{
			friend struct ElectricalPhysicalDomain;

		public:


		private:

			static void InsertDomainCategories(Dgn::DgnDbR);
			static Dgn::DgnCategoryId InsertCategory(Dgn::DgnDbR, Utf8CP, Dgn::ColorDef const&);
			static Dgn::DgnSubCategoryId InsertSubCategory(Dgn::DgnDbR, Dgn::DgnCategoryId, Utf8CP, Dgn::ColorDef const&);

		public:
			//! Get the DgnCategoryId to be used for Device Elements
			ELECTRICAL_PHYSICAL_EXPORT static Dgn::DgnCategoryId QueryElectricalPhysicalDeviceCategoryId(Dgn::DgnDbR);

			//! Get the DgnSubCategoryId for a Device
			ELECTRICAL_PHYSICAL_EXPORT static Dgn::DgnCategoryId QueryElectricalPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName);

		};

	} // End ElectricalPhysical namespace

END_BENTLEY_NAMESPACE

