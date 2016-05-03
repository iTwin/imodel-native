/*--------------------------------------------------------------------------*/ 
/*  BoundingBox.cpp															*/ 
/*	Axis Aligned Bounding Box class implentation							*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#include <pt/BoundingSphere.h>
#include <pt/BoundingBox.h>

using namespace pt;

/*--------------------------------------------------------------------------*/ 
/*	Constructor																*/ 
/*--------------------------------------------------------------------------*/ 
BoundingSphere::BoundingSphere()
{
	_radius = -1.0f;
}
/*--------------------------------------------------------------------------*/ 
/*	Destructor																*/ 
/*--------------------------------------------------------------------------*/ 
BoundingSphere::~BoundingSphere()
{}
/*--------------------------------------------------------------------------*/ 
/*	Expansion to include vertex												*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingSphere::expandBy(const float *v)
{
	const vector3 &v1 = *reinterpret_cast<const vector3*>(v);

    if (valid())
    {
        vector3 dv = v1 - _center;
        float r = dv.length();
        if (r>_radius)
        {
            float dr = (r-_radius)*0.5f;
            _center += dv*(dr/r);
            _radius += dr;
        }
    }
    else
    {
        _center = v;
        _radius = 0.0f;
    }
}
/*--------------------------------------------------------------------------*/ 
/*	Expansion to include Bounding Box										*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingSphere::expandBy(const BoundingBox *bb)
{
    if (bb->valid())
    {
        if (valid())
        {
            BoundingBox newbb(*bb);

            for(unsigned int c=0;c<8;++c)
            {
                vector3 v;
				bb->getExtrema(c, v);
				v -= _center;		// get the direction vector from corner
                v.normalize();		// normalise it.
                v *= -_radius;		// move the vector in the opposite direction distance radius.
                v += _center;		// move to absolute position.
                newbb.expandBy(v);	// add it into the new bounding box.
            }
            
            _center = newbb.center();
            _radius = newbb.radius();
            
        }
        else
        {
            _center = bb->center();
            _radius = bb->radius();
        }
    }
}
/*--------------------------------------------------------------------------*/ 
/*	Expansion to include Bounding Box										*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingSphere::expandBy(const BoundingSphere *sh)
{
    if (sh->valid())
    {
        if (valid())
        {
            vector3 dv = sh->_center-_center;
            float dv_len = dv.length();
            if (dv_len+sh->_radius>_radius)
            {
                vector3 e1 = _center-(dv*(_radius/dv_len));
                vector3 e2 = sh->_center+(dv*(sh->_radius/dv_len));
                _center = (e1+e2)*0.5f;
                _radius = (e2-_center).length();
            }                   
        }
        else
        {
            _center = sh->_center;
            _radius = sh->_radius;
        }
    }
}
/*--------------------------------------------------------------------------*/ 
/*	Expansion Radius only to include Point									*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingSphere::expandRadiusBy(const float *v)
{
	const vector3 &v1 = *reinterpret_cast<const vector3*>(v);

    if (valid())
    {
        float r = (v1-_center).length();
        if (r>_radius) _radius = r;
    }
    else
    {
        _center = v1;
        _radius = 0.0f;
    }
}
/*--------------------------------------------------------------------------*/ 
/*	Expansion Radius only to include Point									*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingSphere::expandRadiusBy(const BoundingBox *bb)
{
    if (bb->valid())
    {
        if (valid())
        {
            for(unsigned int c=0;c<8;++c)
            {
                vector3 corner;
				bb->getExtrema(c, corner);
				expandRadiusBy(corner);
            }
        }
        else
        {
            _center = bb->center();
            _radius = bb->radius();
        }
    }	
}
/*--------------------------------------------------------------------------*/ 
/*	Expansion Radius only to include Point									*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingSphere::expandRadiusBy(const BoundingSphere *sh)
{
    if (sh->valid())
    {
        if (valid())
        {
            float r = (sh->_center-_center).length()+sh->_radius;
            if (r>_radius) _radius = r;
        }
        else
        {
            _center = sh->_center;
            _radius = sh->_radius;
        }
    }
}
