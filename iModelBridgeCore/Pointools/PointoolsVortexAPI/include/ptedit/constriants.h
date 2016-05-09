#pragma once
#include <pt/trace.h>
#include <pt/color.h>
#include <pt/datatree.h>

#include <ptcloud2/voxel.h>
#include <ptedit/edit.h>
#include <ptedit/editNodeDef.h>

namespace ptedit
{

	//	pt::datatree::Branch* addConstraintBr( pt::datatree::Branch *b, const char *id )
	//{
	//	char node[64];
	//	sprintf(node, "%s_%i", id, stateIndex);
	//	return b->addBranch(node);
	//}

//
// Edit Constraint base class 
//
// used for state save/retrieve
// not for virtual func point test which would be too slow
// instead we use a callback returned by constraintFunc
//
struct EditConstraint : public EditNodeDef
{
	typedef bool (*PointTestFunc)(int, pcloud::Voxel *, const pt::vector3d &, uint);
	virtual PointTestFunc constraintFunc() const = 0;

protected:
	EditConstraint(const char*name) :  EditNodeDef(name), stateIndex(0) {}

	int stateIndex;
	int writtenStateIndex;
};

#ifdef  NEEDS_WORK_VORTEX_DGNDB
//
// Colour Constraint
//
// constrains point to a colour based on RGB channel match
//
// Note use of an instance per thread to keep all locals seperate
// and hack OpenMP to safely enter the constraint Function
//
struct ColConstraint : public EditConstraint
{
	DECLARE_EDIT_NODE( "ColConstraint", "Colour Constraint", -1, EditNodeSilent )

	PointTestFunc constraintFunc() const { return &ColConstraint::match; }

	static void setCol(COLORREF col, int tol);
	static bool match(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i);
	inline static ColConstraint &instance(int t=0) { static ColConstraint c[EDT_MAX_THREADS]; return c[t]; }

	bool readState(const pt::datatree::Branch *b);
	bool writeState( pt::datatree::Branch *b) const;

private:
	ColConstraint();

	int tolerance;
	ubyte matchCol[4];
};
//
// GreyScale constraint
//
// constrains point to those that have grey values, ie R=G=B is true within tolerance
struct GreyConstraint : public EditConstraint
{
	DECLARE_EDIT_NODE( "GreyConstraint", "Grey Constraint", -1, EditNodeSilent )

	PointTestFunc constraintFunc() const { return &GreyConstraint::match; }
	
	static void setTol(int tol);
	static bool match( int t, pcloud::Voxel *v, const pt::vector3d &p, uint i );
	inline static GreyConstraint &instance(int t=0) { static GreyConstraint c[EDT_MAX_THREADS]; return c[t]; }

	bool readState(const pt::datatree::Branch *b);
	bool writeState( pt::datatree::Branch *b) const;

private:
	GreyConstraint() : EditConstraint("GreyConstraint"), tolerance(1){}

	int tolerance;
};
//
// Hue Constraint
//
// constrains colour by hue
struct HueConstraint : public EditConstraint
{
	DECLARE_EDIT_NODE( "HueConstraint", "Hue Constraint", -1, EditNodeSilent )

	PointTestFunc constraintFunc() const { return &HueConstraint::match; }

	static void setHue(COLORREF huecol, int tol);
	static bool match(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i);

	inline static HueConstraint &instance(int t=0) { static HueConstraint c[EDT_MAX_THREADS]; return c[t]; }

	bool readState(const pt::datatree::Branch *b);
	bool writeState( pt::datatree::Branch *b) const;

private:
	HueConstraint() : EditConstraint("HueConstraint"),  tolerance(1), hue(0){}

	int tolerance;
	float hue;
};
//
// Lum constraint
//
// constrains point by its lumiosity (RGB)
struct LumConstraint : public EditConstraint
{
	DECLARE_EDIT_NODE( "LumConstraint", "Luminance Constraint", -1, EditNodeSilent )

	PointTestFunc constraintFunc() const { return &LumConstraint::match; }

	static void setLum(COLORREF lumcol, int tol);
	static bool match(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i);

	inline static LumConstraint &instance(int t=0) { static LumConstraint c[EDT_MAX_THREADS]; return c[t]; }

	bool readState(const pt::datatree::Branch *b);
	bool writeState( pt::datatree::Branch *b) const;

private:
	LumConstraint() : EditConstraint("LumConstraint"), tolerance(1), lum(0){}
	int tolerance;
	int lum;
};

//
// Intensity Constraint
//
// constrains point by its intensity value
struct IntensityConstraint : public EditConstraint
{
	
	DECLARE_EDIT_NODE( "IntensityConstraint", "Intensity Constraint", -1, EditNodeSilent )

	PointTestFunc constraintFunc() const { return &IntensityConstraint::match; }

	static void setIntensity(short intens, int tol);
	static bool match(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i);

	inline static IntensityConstraint &instance(int t=0) { static IntensityConstraint c[EDT_MAX_THREADS]; return c[t]; }

	bool readState(const pt::datatree::Branch *b);
	bool writeState( pt::datatree::Branch *b) const;

private:
	IntensityConstraint() : EditConstraint("IntensityConstraint"), tolerance(1), intensity(0){}
	int tolerance;
	short intensity;
};
#endif
};