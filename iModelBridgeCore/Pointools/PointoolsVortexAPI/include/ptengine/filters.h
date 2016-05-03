#ifndef POINTOOLS_POINTSENGINE_FILTERS
#define POINTOOLS_POINTSENGINE_FILTERS

#include <ptengine/pointsfilter.h>

namespace pointsengine
{
	class PTENGINE_API BoxFilter : public Filter
	{
	public:
		BoxFilter(const pt::BoundingBox &bb) : _bounds(bb) {};
		virtual ~BoxFilter(){};

		virtual FilterResult filterBounds(const pt::BoundingBox *bb);
		virtual void filterPoints(ChannelInfo **data, const pt::Transform* owner, pt::bitvector &bvec);

		virtual uint stateID();
	
	protected:
		const pt::BoundingBox &_bounds;
		pt::BoundingBox _currentbounds;
	};
	class PTENGINE_API RotatedBoxFilter : public BoxFilter
	{
	public:
		RotatedBoxFilter(const pt::BoundingBox &bb, const pt::vector3 &angles) : BoxFilter(bb), _angles(angles) {};
		~RotatedBoxFilter();

		FilterResult filterBounds(const pt::BoundingBox *bb);
		void filterPoints(ChannelInfo **data, const pt::Transform* owner, pt::bitvector &bvec);

		uint stateID();
	private:
		const pt::vector3 &_angles;
		pt::vector3 _currentangles;
		pt::BoundingBox _bounds1;

		mmatrix4d	_mat;
	};
	//class ProjectedRectFilter : public Filter
	//{
	//	
	//};
	//class ProjectedFenceFilter : public Filter
	//{

	//};
	//class SectionFilter : public Filter
	//{

	//};
	//class ExpressionFilter : public Filter
	//{

	//};
}
#endif