#ifndef POINTOOLS_GL_CLIPBOX
#define POINTOOLS_GL_CLIPBOX

#include <pt/boundingbox.h>
#include <ptgl/ptgl.h>

namespace ptgl
{
	class PTGL_API ClipBox
	{
	public:
		ClipBox(const pt::BoundingBox *_bb);
		~ClipBox();

		inline const pt::BoundingBox *box() const { return _bb; };
	private:
		const pt::BoundingBox *_bb;
	};
}
#endif