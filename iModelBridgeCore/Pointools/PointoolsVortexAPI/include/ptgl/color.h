#ifndef POINTOOLS_GLCOLOR
#define POINTOOLS_GLCOLOR


namespace ptgl
{
#define COL_EPSILON 0.002

class Color
{
public:
	Color() {};
	Color(float _r, float _g, float _b, float _a=1.0f)
	{
		set(_r,_g,_b,_a);
	}
	Color(unsigned int rgb) { fromUINT(rgb); }

	bool operator == ( const Color &c ) const
	{
		return (   fabs(r - c.r) < COL_EPSILON
				&& fabs(g - c.g) < COL_EPSILON
				&& fabs(b - c.b) < COL_EPSILON ) ? true : false;
	}
	bool operator != ( const Color &c ) const	{ return !(c == (*this)); }

	void set(float _r, float _g, float _b, float _a=1.0f)
	{
		r = _r;
		b = _b;
		g = _g;
		a = _a;
	}
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
	void setGL() const 
	{ 
	//	glColor3f(r,g,b); 
	}
	float r;
	float g;
	float b;
	float a;
};
}
#endif