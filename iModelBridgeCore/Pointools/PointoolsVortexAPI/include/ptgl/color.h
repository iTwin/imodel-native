#ifndef POINTOOLS_GLCOLOR
#define POINTOOLS_GLCOLOR

namespace ptgl
{
class Color
{
public:
	Color() {};
	Color(float _r, float _g, float _b)
	{
		r = _r;
		b = _b;
		g = _g;
	}
	Color(unsigned int rgb) { fromUINT(rgb); }

	inline void fromUINT(unsigned int rgb)
	{
		r = 0.00392156862745098  * GetRValue(rgb);// ((unsigned char)(rgb));
		g = 0.00392156862745098  * GetGValue(rgb);//((unsigned char)(((unsigned short)(rgb)) >> 8));
		b = 0.00392156862745098  * GetBValue(rgb);//((unsigned char)((rgb)>>16));
	}
	inline uint toUINT()
	{
		return (unsigned int)(((unsigned char)(r*255)|((unsigned short)((unsigned char)(g*255))<<8))|(((unsigned int)(unsigned char)(b*255))<<16));
	}
	void setGL() const { glColor3f(r,g,b); }
	float r;
	float g;
	float b;
};
}
#endif