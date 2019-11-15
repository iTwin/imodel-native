/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ptedit/edit.h>
#include <ptedit/editSelect.h>
#include <ptedit/editNodeDef.h>
#include <vector>

namespace ptedit
{
	struct CloudSelect : public EditNodeDef
	{
		CloudSelect();
		DECLARE_EDIT_NODE("CloudSelect", "Cloud Select", 10, 0)
		
		bool apply();
		bool writeState( pt::datatree::Branch *b) const;
		bool readState(const pt::datatree::Branch *b);

		void setCloud( const pcloud::PointCloud *pc );

	private:
		int64_t cloudGuid;
	};

} // end namespace ptedit
