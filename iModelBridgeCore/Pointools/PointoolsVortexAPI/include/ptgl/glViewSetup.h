#ifndef POINTOOLS_GL_VIEW_SETUP_HELPERS
#define POINTOOLS_GL_VIEW_SETUP_HELPERS

#include <ptgl/ptgl.h>

namespace ptgl
{
	class PTGL_API PixelView
	{
	public:
		PixelView();
		~PixelView();

		bool inView(int x, int y) const
		{ 
			return (x >= _l && x <= _r && y >= _b && y <= _t) ? true : false; 
		}
		bool inView(double x, double y) const 
		{ 
			return (x >= _l && x <= _r && y >= _b && y <= _t) ? true : false; 
		}
		bool inView(double *pnt) const 
		{ 
			return (pnt[0] >= _l && pnt[0] <= _r && pnt[1] >= _b && pnt[1] <= _t) ? true : false; 
		}
		double _l;
		double _b;
		double _r;
		double _t;
	};
	class PTGL_API XORView
	{
	public:
		XORView();
		~XORView();
	};
};

#endif