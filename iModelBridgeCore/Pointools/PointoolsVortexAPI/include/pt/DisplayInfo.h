#ifndef POINTOOLS_DISPLAY_INFO_H
#define POINTOOLS_DISPLAY_INFO_H

#include <pt\flags.h>

namespace pt
{
//-------------------------------------------------------------------------
/// DisplayInfo class holds information for display of objects
//-------------------------------------------------------------------------
class DisplayInfo
{
public:
	enum FlagValues { DIVisible = 1, DICulled = 2, 
		DIUser0 = 4, DIUser1 = 8, DIUser2 = 0x10, DIUser3 = 0x20, DIUser4 = 0x40,
		DIUser5 = 0x80, DIUser6 = 0x100, DIUser7 = 0x200, DIUser8 = 0x400, DIUser9 = 0x800, 
		DIUser10 = 0x1000, DIUser11 = 0x2000, DIUser12 = 0x4000, DIUser13 = 0x8000, 
		DIUser14 = 0x10000, DIUser15 = 0x20000 
	};

	DisplayInfo() : m_flags(DIVisible) {}
	
	const Flags &flags() const { return m_flags; }

	inline bool visible() const { return m_flags.flag(DIVisible); }
	inline void visible(bool v) {  if (v) show(); else hide(); }

	inline void hide() { m_flags.clear(DIVisible); }
	inline void show() { m_flags.set(DIVisible); }
	inline void culled(bool v) { if (v) m_flags.set(DICulled); else m_flags.clear(DICulled); }
	inline bool culled() { return m_flags.flag(DICulled); }

private:	
	Flags			m_flags;
};
}
#endif