/*--------------------------------------------------------------------------*/ 
/*  Geometry.cpp															*/ 
/*	Geometry base class implementation										*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#include <pt\geometry_1_51.h>

using namespace pt;

class TestObject3D : public Object3D
{
public:
	TestObject3D(const wchar_t* id) : Object3D(id) {};
	virtual ~TestObject3D() {};

};

void globalFunc()
{
	TestObject3D tob(L"New Object");
}