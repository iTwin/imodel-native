#ifndef PTSTATS_HELPER
#define PTSTATS_HELPER

#include <mathimf.h>

namespace ptmath
{
	template <class T>
	double standard_deviation(const T *v, int n)
	{
		double m = mean(v, n);
		double t=0;
		int i;
		for (i=0;i<n;i++) t += (v[i]-m)*(v[i]-m);
		if (t!=0) return sqrt(t/n);
		else return 0;
	}
	template <class T>
	double mean(const T* v, int n)
	{
		int i = 0;
		T t=(T)0;
		for (i=0;i<n;i++) t += v[i];	
		return (double)(t/n);
	}

}
#endif