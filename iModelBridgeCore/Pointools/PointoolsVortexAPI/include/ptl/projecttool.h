/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifndef POINTOOLS_PROJECT_TOOL
#define POINTOOLS_PROJECT_TOOL

#include <ptcmdppe/eventdefs.h>
#include <pttool/tool.h>

namespace pt
{
	class Project : public Tool
	{
	public:
		Project();
		~Project();

		public:
			bool initialize();
		protected:
			static void properties();
	};

}
#endif
