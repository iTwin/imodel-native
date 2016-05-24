#pragma once
#include <ptedit/edit.h>
#include <ptedit/editSelect.h>
#include <ptedit/editNodeDef.h>

namespace ptedit
{
	struct EvaluatedStackNode : public EditNodeDef
	{
		EvaluatedStackNode( ) 
			: EditNodeDef("StackFreeze")
		{
		};
		
		void setFilePath( const ptds::FilePath &file )
		{
			m_file = file;
		}
		void setStateName( const pt::String &name )
		{
			m_name = name;
		}

		SELECTION_FILTER_DETAIL

		const pt::String &name() const { static pt::String p("StackFreeze"); return p; }
		const pt::String &desc() const 
		{ 
			static pt::String s;
			s.format("StackFreeze: %s", m_name.c_u8str());
			return s; 
		}

		bool apply();		//applies previously saved evaluated stack

		int icon() const { return 5; }

        using EditNodeDef::flags; // tell the compiler we want both the flags from EditNodeDef and ours (otherwise clang complains)
		uint flags() const { return 0; }
	
		bool readState(const pt::datatree::Branch *b);
		bool writeState(pt::datatree ::Branch *b) const;

	private:
		pt::String m_name;
		ptds::FilePath m_file;
	};
} // end namespace ptedit