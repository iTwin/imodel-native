
#pragma once

#include <PTRMI/ArrayDirect.h>

namespace PTRMI
{


template<typename P> class In
{
public:
	typedef P	T;
};

template<typename P> class Out
{
public:
	typedef P	T;
};

template<typename P> class InOut
{
public:
	typedef P	T;
};


class PVoid
{
public:
	typedef	void		T;

public:
	void			*	v;

public:

	PVoid(void)
	{

	}
};



template<typename P> class PC
{
public:
	typedef P				T;

protected:

	T *		v;

public:

	PC(void) {}
	PC(T &item) {v = &item;}

	T &		get	(void)	{return *v;}
};



template<typename P> class PC<In<P> > : public PC<P>
{
public:
	PC(void) {}
	PC(P &item)	{v = &item;}
};

template<typename P> class PC<Out<P> > : public PC<P>
{
public:
	PC(void)	{}
	PC(P &item)	{v = &item;}
};

template<typename P> class PC<InOut<P> > : public PC<P>
{
public:
	PC(void)	{}
	PC(P &item)	{v = &item;}
};



template<typename P> class PS
{
public:
	typedef P	T;

protected:

	T 		v;

public:

	PS(void) {}
	PS(T &item) {v = item;}

	T &		get	(void)	{return v;}
};


template<typename P> class PS<In<P> > : public PS<P>
{
public:
	PS(void) {}
	PS(P &item)	{v = item;}
};

template<typename P> class PS<In<P *> > : public PS<P *>
{
protected:

	P	v;

public:
	PS(void) {}
	PS(P &item)	{v = item;}

	P *get() {return &v;}
};


template<typename P> class PS<Out<P> > : public PS<P>
{
public:
	PS(void) {}
	PS(P &item)	{v = &item;}
};


template<typename P> class PS<Out<P *> > : public PS<P *>
{
protected:

	P	v;

public:
	PS(void) {}
	PS(P &item)	{v = item;}

	P *get() {return &v;}
};


template<typename P> class PS<InOut<P> > : public PS<P>
{
public:
	PS(void) {}
	PS(P &item)	{v = &item;}
};

typedef PC<In<PVoid>>	PCVoid;
typedef PS<In<PVoid>>	PSVoid;

// ********************************************************************


template<typename P> class PC<Out<ArrayDirect<P>>>
{
public:

	typedef ArrayDirect<P>	T;

	ArrayDirect<P>	v;

public:

	PC(void)	{}
//	PC(P &item)	{v = &item;}
	PC(ArrayDirect<P> &item) {v = item;}

	ArrayDirect<P> *getContainer(void) {return &v;}

	ArrayDirect<P> *get(void) {return &v;}

};


template<typename P> class PS<Out<ArrayDirect<P>>>
{

public:

	typedef P *	T;

protected:

	ArrayDirect<P>	v;

public:

	PS(void) {}
	PS(ArrayDirect<P> &item) {v = item;}

	P *get() {return v.getArray();}		// Pass pointer to array in function calls

	ArrayDirect<P> *getContainer(void) {return &v;}
};



} // End PTRMI namespace
