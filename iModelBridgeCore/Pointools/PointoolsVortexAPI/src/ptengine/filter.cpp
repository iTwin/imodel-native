#include <ptengine/filters.h>
#include <pt/boundingbox.h>

using namespace pointsengine;
using namespace pt;

#define DEG2RAD(a) a * 0.017453292519943295769236907684886
namespace _simd
{
__declspec(naked) void PIII_Mult00_4x4_4x1(const float *src1, const float *src2, float *dst)
{
	__asm {
		mov ecx, dword ptr [esp+ 8] ; src2
		mov edx, dword ptr [esp+ 4] ; src1
		movlps xmm6, qword ptr [ecx ]
		movlps xmm0, qword ptr [edx ]
		shufps xmm6, xmm6, 0x44
		movhps xmm0, qword ptr [edx+16]
		mulps xmm0, xmm6
		movlps xmm7, qword ptr [ecx+ 8]
		movlps xmm2, qword ptr [edx+ 8]
		shufps xmm7, xmm7, 0x44
		movhps xmm2, qword ptr [edx+24]
		mulps xmm2, xmm7
		movlps xmm1, qword ptr [edx+32]
		movhps xmm1, qword ptr [edx+48]
		mulps xmm1, xmm6
		movlps xmm3, qword ptr [edx+40]
		addps xmm0, xmm2
		movhps xmm3, qword ptr [edx+56]
		mov eax, dword ptr [esp+12] ; dst
		mulps xmm3, xmm7
		movaps xmm4, xmm0
		addps xmm1, xmm3
		shufps xmm4, xmm1, 0x88
		shufps xmm0, xmm1, 0xDD
		addps xmm0, xmm4
		movaps xmmword ptr [eax], xmm0
	}
	__asm ret
}
__declspec(naked) void PIII_Mult10_4x4_4x1( const float *src1, const float *src2, float *dst)
{
	__asm {
	mov ecx, dword ptr [esp+8] ; src2
	mov edx, dword ptr [esp+4] ; src1
	movss xmm0, dword ptr [ecx]
	mov eax, dword ptr [esp+0Ch] ; dst
	shufps xmm0, xmm0, 0
	movss xmm1, dword ptr [ecx+4]
	mulps xmm0, xmmword ptr [edx]
	shufps xmm1, xmm1, 0
	movss xmm2, dword ptr [ecx+8]
	mulps xmm1, xmmword ptr [edx+16]
	shufps xmm2, xmm2, 0
	movss xmm3, dword ptr [ecx+12]
	mulps xmm2, xmmword ptr [edx+32]
	shufps xmm3, xmm3, 0
	addps xmm0, xmm1
	mulps xmm3, xmmword ptr [edx+48]
	addps xmm2, xmm3
	addps xmm0, xmm2
	movaps xmmword ptr [eax], xmm0
	}
__asm ret
}
}
using namespace _simd;

FilterResult BoxFilter::filterBounds(const pt::BoundingBox *bb)
{
	if (_bounds.contains(bb)) return Filtered_In;
	if (_bounds.intersects(bb)) return Filtered_Part;
	return Filtered_Out;
}
void BoxFilter::filterPoints(ChannelInfo **data, const pt::Transform* owner, pt::bitvector &bvec)
{
	uint size = data[pcloud::PCloud_Geometry]->_size;
	uint i= 0;

	if (data[pcloud::PCloud_Geometry]->_dtype == pcloud::Float32)
	{
		const pt::vector3 *geom = reinterpret_cast<const pt::vector3*>
			(data[pcloud::PCloud_Geometry]->_data);
		
		pt::vector3 v;

		while (i < size)	
		{
			owner->transform(geom[i], v);
			bvec.assign(i, _bounds.inBounds(v));
			++i;
		}
	}
	else
	{
		const pt::vector3s *geom = reinterpret_cast<const pt::vector3s*>
			(data[pcloud::PCloud_Geometry]->_data);
	
		double off[4], scal[4];
		float v[4],vt[4];
		memcpy(off, data[pcloud::PCloud_Geometry]->_offset, sizeof(double) *3);
		memcpy(scal, data[pcloud::PCloud_Geometry]->_scaler, sizeof(double) * 3);
		off[3] = 1.0;
		scal[3] = 1.0;

		mmatrix4d M = mmatrix4d::scale(scal) >> mmatrix4d::translation(off);
		mmatrix4d MP; for (i=0; i<16; i++) MP.data()[i] = owner->matrix().data()[i];
		M >>= MP;

#ifdef _USE_SIMD
		M.transpose();
#endif
		while (i < size)
		{
			v[0] = geom[i].x;
			v[1] = geom[i].y;
			v[2] = geom[i].z;
			v[3] = 1.0f;

#ifdef _USE_SIMD			
			PIII_Mult10_4x4_4x1(M.data(), v, vt);
#endif
			M.vec3_multiply_mat4f(v, vt);
			bvec.assign(i, _bounds.inBounds(vt));
			++i;
		}
	}
}
uint BoxFilter::stateID()
{
	if (_currentbounds != _bounds)
	{
		_stateid++;
		_currentbounds = _bounds;
	}
	return _stateid;
}
RotatedBoxFilter::~RotatedBoxFilter() {};
//----------------------------------------------------------------------
// Rotated Box Filter
//----------------------------------------------------------------------
FilterResult RotatedBoxFilter::filterBounds(const pt::BoundingBox *bb)
{
	if (_angles.is_zero()) return BoxFilter::filterBounds(bb);
	vector3 v, v1;
	unsigned char res = 0;

	BoundingBox env;
	env.clear();

	for (int i=0;i<8; i++)
	{
		bb->getExtrema(i, v);
		_mat.vec3_multiply_mat4f(v, v1);
		
		env.expand(v1);
	}
	if (_bounds1.contains(&env)) return Filtered_In;
	if (_bounds1.intersects(&env)) return Filtered_Part;
	return Filtered_Out;
}
//----------------------------------------------------------------------
// 
//----------------------------------------------------------------------
void RotatedBoxFilter::filterPoints(ChannelInfo **data, const pt::Transform* owner, pt::bitvector &bvec)
{
	if (_angles.is_zero()) 
	{
		BoxFilter::filterPoints(data, owner, bvec);
		return;
	}

	uint size = data[pcloud::PCloud_Geometry]->_size;
	uint i= 0;
	pt::vector3 v, v1, v2;

	if (data[pcloud::PCloud_Geometry]->_dtype == pcloud::Float32)
	{
		const pt::vector3 *geom = reinterpret_cast<const pt::vector3*>
			(data[pcloud::PCloud_Geometry]->_data);
		
		while (i < size)	
		{
			owner->transform(geom[i], v);
			_mat.vec3_multiply_mat4f(v, v1);
			bvec.assign(i, _bounds1.inBounds(v1));
			++i;
		}
	}
	else
	{
		const pt::vector3s *geom = reinterpret_cast<const pt::vector3s*>
			(data[pcloud::PCloud_Geometry]->_data);
	
		double off[4], scal[4];
		float  v[4],vt[4];
		memcpy(off, data[pcloud::PCloud_Geometry]->_offset, sizeof(double) *3);
		memcpy(scal, data[pcloud::PCloud_Geometry]->_scaler, sizeof(double) * 3);
		off[3] = 1.0;
		scal[3] = 1.0;

		mmatrix4d M = mmatrix4d::scale(scal) >> mmatrix4d::translation(off);
		mmatrix4d MP; for (i=0; i<16; i++) MP.data()[i] = owner->matrix().data()[i];
		MP >>= _mat;
		M >>= MP;
#ifdef _USE_SIMD
		M.transpose();
#endif

		i=0;

		while (i < size)
		{
			v[0] = geom[i].x;
			v[1] = geom[i].y;
			v[2] = geom[i].z;
			v[3] = 1.0f;

#ifdef _USE_SIMD
			PIII_Mult10_4x4_4x1(M.data(), v, vt);
#else
			M.vec3_multiply_mat4f(v, vt);
#endif
			bvec.assign(i, _bounds1.inBounds(vt));
			++i;
		}
	}	
}
uint RotatedBoxFilter::stateID()
{
	if (_currentbounds != _bounds || !_currentangles.equal(_angles, 0.01f))
	{
		_stateid++;
		_currentbounds = _bounds;
		_currentangles = _angles;

		_mat = mmatrix4d::rotation(DEG2RAD(_angles.x), DEG2RAD(_angles.y), DEG2RAD(_angles.z));
		_mat.translate(pt::vector4d(_bounds.center()));
		_mat.invert();

		_bounds1 = _bounds;
		_bounds1.translateBy(-_bounds.center());
	}
	return _stateid;
}