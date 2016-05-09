#pragma once

#ifdef HAVE_GDIPLUS
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
#endif

#include <vector>
#include <list>
#include <stdarg.h>
#include <math.h>
#include <algorithm>
#include <PTRMI/Mutex.h>


#define GRAPH_ENTITY_MUTEX_TIMEOUT				(1000 * 10)
#define GRAPH_ENTITY_LABEL_POS_X_OFFSET			500
#define GRAPH_ENTITY_VALUE_POS_X_OFFSET			300
#define GRAPH_ENTITY_MEAN_POS_X_OFFSET			200
#define GRAPH_ENTITY_STDDEV_POS_X_OFFSET		100


namespace ptds
{
        using std::min;
        using std::max;

	void testGraph(void);


	class Vector
	{

	public:

	};


	template<typename T, unsigned int dim> class VectorND : public Vector
	{
	public:

		typedef T						Type;
		typedef		VectorND<T, dim>	thisType;
		typedef		unsigned int		Index;

	public:

		static const Index		X = 0;
		static const Index		Y = 1;
		static const Index		Z = 2;
		static const Index		W = 3;

	protected:

		T	v[dim];

	public:

						VectorND		(T *init = NULL);
						VectorND		(T v1);
						VectorND		(T v1, T v2);
						VectorND		(T v1, T v2, T v3);
						VectorND		(T v1, T v2, T v3, T v4);

		unsigned int	getSize			(void) const;

		void			clear			(void);

		void			set				(T v1);
		void			set				(T v1, T v2);
		void			set				(T v1, T v2, T v3);
		void			set				(T v1, T v2, T v3, T v4);

		thisType		getMin			(const thisType &p);
		thisType		getMax			(const thisType &p);

		void			setRandom		(void);

		void			insertMin		(const thisType &p);
		void			insertMax		(const thisType &p);

		T &				operator[]		(Index index);
		const T &		operator[]		(Index index) const;

		template<typename Y, unsigned int d> thisType &operator=(const VectorND<Y, d> &p)
		{
			unsigned int dp = min(dim, d);

			unsigned int t;

			for(t = 0; t < dp; t++)
			{
				v[t] = p[t];
			}

			return *this;
		}

		template<typename Y> thisType &operator+=(const VectorND<Y, dim> &p1)
		{
			unsigned int t;

			for(t = 0; t < dim; t++)
			{
				(*this)[t] += p1[t];
			}

			return *this;
		}

		thisType &		operator-=		(const thisType &p1);

		template<typename Y> thisType &operator*=(const VectorND<Y, dim> &p1)
		{
			unsigned int t;

			for(t = 0; t < dim; t++)
			{
				(*this)[t] *= p1[t];
			}

			return *this;
		}


		thisType &		operator/=		(const thisType &p1);

		thisType &		operator+=		(T v);
		thisType &		operator-=		(T v);
		thisType &		operator*=		(T v);
		thisType &		operator/=		(T v);

		thisType		operator+		(T v);
		thisType		operator-		(T v);
		thisType		operator*		(T v);
		thisType		operator/		(T v);

		template<typename Y> thisType operator+(const VectorND<Y, dim> &p) const
		{
			thisType		r;

			unsigned int	t;

			for(t = 0; t < dim; t++)
			{
				r[t] = v[t] + p[t];
			}

			return r;
		}

		template<typename Y> thisType operator-(const VectorND<Y, dim> &p) const
		{
			thisType	r;

			unsigned int	t;

			for(t = 0; t < dim; t++)
			{
				r[t] = v[t] - p[t];
			}

			return r;
		}

		template<typename Y> thisType operator*(const VectorND<Y, dim> &p) const
		{
			thisType	r;

			unsigned int	t;

			for(t = 0; t < dim; t++)
			{
				r[t] = v[t] * p[t];
			}

			return r;
		}

		template<typename Y> thisType operator/(const VectorND<Y, dim> &p) const
		{
			thisType	r;

			unsigned int	t;

			for(t = 0; t < dim; t++)
			{
				if(p[t] != 0)
				{
					r[t] = v[t] / p[t];
				}
				else
				{
					r[t] = 0;
				}
			}

			return r;
		}


	};


	typedef VectorND<float, 2>		Vector2f;
	typedef VectorND<float, 3>		Vector3f;
	typedef VectorND<double, 2>		Vector2d;
	typedef VectorND<double, 3>		Vector3d;
	typedef	VectorND<int, 2>		Vector2i;

	class Graph;

	class GraphEntityStyle
	{

	public:

		typedef Vector3f	ColorRGB;

	protected:

		ColorRGB	color;

	public:
					GraphEntityStyle	(void);
					GraphEntityStyle	(ColorRGB &initColor)	{setColor(initColor);}

		void		setColor			(ColorRGB &initColor)	{color = initColor;}
		ColorRGB	getColor			(void)					{return color;}

	};


	class GraphEntity
	{
	public:

		typedef unsigned int	Index;

	protected:

		PTRMI::Mutex			mutex;

		std::wstring			name;

		GraphEntityStyle		style;
		Vector2d				extentsMin;
		Vector2d				extentsMax;

		bool					preserveAspectRatioEnabled;

		bool					indexRangeEnabled;
		Index					indexRangeStart;
		Index					indexRangeEnd;

		Vector2d				unitScale;
		
	protected:

		void					beginStyle						(HWND hWnd, HDC hDC);
		void					endStyle						(void);

	public:
								GraphEntity						(void);
								GraphEntity						(const wchar_t *entityName);

		virtual void			clear							(void);

		virtual Vector2d		get								(void) = 0;

		void					setName							(const wchar_t *entityName)									{if(entityName) name = entityName;}
		const wchar_t		*	getName							(void)														{return name.c_str();}

		void					setStyle						(GraphEntityStyle &initStyle)								{style = initStyle;}
		GraphEntityStyle	&	getStyle						(void)														{return style;}

		void					setUnitScale					(Vector2d &scale)											{unitScale = scale;}
		Vector2d				getUnitScale					(void)														{return unitScale;}

		virtual unsigned int	getNumPoints					(void)														{return 0;}
		virtual unsigned int	getNumPointsRange				(void)														{return 0;}

		void					setIndexRangeEnabled			(bool enabled)												{indexRangeEnabled = enabled;}
		bool					getIndexRangeEnabled			(void)														{return indexRangeEnabled;}

		void					setIndexRange					(Index start, Index end)									{setIndexRangeStart(start); setIndexRangeEnd(end);}
		void					setIndexRangeStart				(Index start)												{indexRangeStart = start;}
		Index					getIndexRangeStart				(void)														{return (getIndexRangeEnabled() ? indexRangeStart : 0);}
		void					setIndexRangeEnd				(Index end)													{indexRangeEnd = end;}
		Index					getIndexRangeEnd				(void)														{return (getIndexRangeEnabled() ? indexRangeEnd : getNumPoints() - 1);}

		void					setExtentsMin					(Vector2d &minimum)											{extentsMin = minimum;}
		Vector2d				getExtentsMin					(void)														{return extentsMin;}
		void					setExtentsMax					(Vector2d &maximum)											{extentsMax = maximum;}
		Vector2d				getExtentsMax					(void)														{return extentsMax;}

		virtual void			calculateMinMax					(Vector2d &minimum, Vector2d &maximum)						{}

		void					setPreserveAspectRatioEnabled	(bool enabled)												{preserveAspectRatioEnabled = enabled;}
		bool					getPreserveAspectRatioEnabled	(void)														{return preserveAspectRatioEnabled;}

		virtual void			updateRandom					(void)														{}

#ifdef HAVE_GDIPLUS
        virtual void			draw                            (HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int &entityCounter, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar) {}
        virtual void			drawEntityLabel					(HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int entityCounter, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar);
#endif
	};




	template<typename T, unsigned int dim> class Series : public GraphEntity
	{

	public:

		typedef VectorND<T, dim>		Point;
		typedef VectorND<double, 2>		Point2d;
		typedef VectorND<double, 3>		Point3d;

		typedef T						Type;

	protected:

		typedef std::vector<Point>		PointSet;

	protected:

		PointSet						points;
		Index							numPointsMax;

	protected:

		Point							projectPoint					(Point &point, Vector2d &offset, Vector2d &scalar, unsigned int windowHeight);

	public:

										Series							(void);
										Series							(const wchar_t *entityName) : GraphEntity(entityName) {}

		void							add								(Point &point);
		Point							get								(Index index);
		Point &							operator[]						(Index index);

		Vector2d						get								(void);

		void							clear							(void);

		void							setNumPointsMax					(Index maxPoints);
		Index							getNumPointsMax					(void);

		unsigned int					getNumPoints					(void);
		unsigned int					getNumPointsRange				(void);

		bool							deleteRange						(Index start, Index end);
		void							deleteFirstToMaxPoints			(void);

		void							calculateMinMax					(Point2d &minimum, Point2d &maximum);
		void							calculateMinMax					(Point &minimum, Point &maximum);

		Point							calculateMean					(void);
		Point							calculateMeanLast				(Index numLastItems);
		Type							calculateVariance				(Index index, T *mean = NULL);
		T								calculateStdDev					(Index index, T *mean = NULL, T *variance = NULL);
		bool							calculateLinearLeastSquares2D	(T &a, T &b, Index dimX, Index dimY);
		
		void							generateRandom					(unsigned int numPoints, Index primaryAxis, T interval, Point &offset, Point &minimum, Point &maximum);
		void							generateSquareWave				(unsigned int numPoints, Index primaryAxis, T interval, Point &offset, Point &minimum, Point &maximum);

		void							updateRandom					(void);

#ifdef HAVE_GDIPLUS
		void							draw							(HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int &entityCounter, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar);
		void							drawAxisLines					(HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar);
		void							drawAxisText					(HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int entityCounter, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar);
		virtual void					drawEntityLabel					(HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int entityCounter, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar);
#endif
	};



	class Graph
	{
	public:

		typedef unsigned int Index;

		enum GraphEntityOverlayMode
		{
			GraphEntityOverlayCombined,
			GraphEntityOverlaySeparate
		};

	protected:

		typedef std::list<GraphEntity *>	GraphEntityList;


	protected:

		PTRMI::Mutex			mutex;

		HWND					hWnd;

		std::wstring			name;

		GraphEntityList			entities;

		unsigned int			windowWidth;
		unsigned int			windowHeight;

		Vector2i				borderPixelsMin;
		Vector2i				borderPixelsMax;

		unsigned int			windowTimer;

		bool					updateRandomEnabled;

		GraphEntityOverlayMode	entityOverlayMode;

		bool					includeOriginX;
		bool					includeOriginY;

	protected:

		static LRESULT CALLBACK windowMessageHandler		(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

		void					setWindowWidth				(unsigned int width)	{windowWidth = width;}
		void					setWindowHeight				(unsigned int height)	{windowHeight = height;}

		unsigned int			getWindowWidth				(void)					{return windowWidth;}
		unsigned int			getWindowHeight				(void)					{return windowHeight;}

#ifdef HAVE_GDIPLUS
		void					drawAxisLines				(HWND hWnd, HDC hDC, Gdiplus::Graphics &graphics, unsigned int windowWidth, unsigned int windowHeight);
#endif

		void					setWindowTimer				(unsigned int timer)	{windowTimer = timer;}
		unsigned int			getWindowTimer				(void)					{return windowTimer;}

		void					setHWND						(HWND inithWnd)			{hWnd = inithWnd;}

		bool					calculateFrustum			(bool preserveAspectRatio, Vector2d &extentsMin, Vector2d &extentsMax, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar);

	public:
								Graph						(void);
								Graph						(const wchar_t *graphName);

		bool					initialize					(const wchar_t *graphName);

		void					setEntityOverlayMode		(GraphEntityOverlayMode mode)	{entityOverlayMode = mode;}
		GraphEntityOverlayMode	getEntityOverlayMode		(void)							{return entityOverlayMode;}

		void					setName						(const wchar_t *graphName)		{name = graphName;}
		const std::wstring &	getName						(void)							{return name;}

		void					addEntity					(GraphEntity &entity);
		GraphEntity		*		getEntity					(Index index);
		void					removeEntity				(GraphEntity &entity);
		unsigned int			removeAllEntities			(void);
		unsigned int			getNumEntities				(void);

		void					setBorderPixelsMin			(Vector2i &pixels);
		Vector2i				getBorderPixelsMin			(void);

		void					setBorderPixelsMax			(Vector2i &pixels);
		Vector2i				getBorderPixelsMax			(void);

		void					setIncludeOriginX			(bool inc)						{includeOriginX = inc;}
		bool					getIncludeOriginX			(void)							{return includeOriginX;}

		void					setIncludeOriginY			(bool inc)						{includeOriginY = inc;}
		bool					getIncludeOriginY			(void)							{return includeOriginY;}

		HWND					getHWND						(void)							{return hWnd;}

		void					setUpdateRandomEnabled		(bool enabled)					{updateRandomEnabled = enabled;}
		bool					getUpdateRandomEnabled		(void)							{return updateRandomEnabled;}

		void					calculateMinMax				(Vector2d &minimum, Vector2d &maximum);

		void					createWindow				(const wchar_t *windowName, int posX, int posY, int width, int height);
#ifdef HAVE_GDIPLUS
		bool					draw						(HWND hWnd, HDC hDC, PAINTSTRUCT &ps);
#endif

		bool					calculateGraphFrustum		(Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar);
		bool					calculateEntityFrustum		(GraphEntity *entity, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar);

		void					update						(void);

		void					updateRandom				(void);
	};


	class GraphManager
	{
	public:

		typedef unsigned int Index;

	protected:

		typedef std::list<Graph *>	GraphList;

	protected:

		PTRMI::Mutex	mutex;

		GraphList		graphs;

	protected:

		bool			addGraph				(Graph *graph);

	public:

		Graph		*	newGraph				(const wchar_t *graphName);
		bool			deleteGraph				(Graph *graph);
		Index			getNumGraphs			(void);

		unsigned int	removeAllEntities		(void);
		unsigned int	removeAllEntities		(Graph *graph);

		Graph		*	getGraph				(Index graph);
		Graph		*	getGraphWithWnd			(HWND hWnd);

#ifdef HAVE_GDIPLUS
		void			draw					(HWND hWnd, HDC hDC, PAINTSTRUCT &ps);
#endif

		void			update					(void);

		void			updateRandom			(void);
		Graph		*	getGraphWithName		(const wchar_t *graphName);
	};

	extern GraphManager	graphManager;


	template<typename T, unsigned int dim>
	void ptds::VectorND<T, dim>::set(T v1)
	{
		v[0] = v1;
	}


	template<typename T, unsigned int dim>
	void ptds::VectorND<T, dim>::setRandom(void)
	{
		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			v[t] = static_cast<float>((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)));
		}
	}


	template<typename T, unsigned int dim>
	void ptds::VectorND<T, dim>::insertMin(const thisType &p)
	{
		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			if(p[t] < v[t])
			{
				v[t] = p[t];
			}
		}
	}

	template<typename T, unsigned int dim>
	void ptds::VectorND<T, dim>::insertMax(const thisType &p)
	{
		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			if(p[t] > v[t])
			{
				v[t] = p[t];
			}
		}
	}


	template<typename T, unsigned int dim>
	void ptds::VectorND<T, dim>::set(T v1, T v2)
	{
		set(v1);
		if(dim >= 2)
		{
			v[1] = v2;
		}
	}

	template<typename T, unsigned int dim>
	void ptds::VectorND<T, dim>::set(T v1, T v2, T v3)
	{
		set(v1, v2);
		if(dim >= 3)
		{
			v[2] = v3;
		}
	}

	template<typename T, unsigned int dim>
	void ptds::VectorND<T, dim>::set(T v1, T v2, T v3, T v4)
	{
		set(v1, v2, v3);
		if(dim >= 4)
		{
			v[3] = v4;
		}
	}


	template<typename T, unsigned int dim>
	inline VectorND<T, dim> VectorND<T, dim>::getMin(const thisType &p)
	{
		thisType		r;
		unsigned int	i;

		for(i = 0; i < dim; i++)
		{
			r[i] = min(v[i], p[i]);
		}

		return r;
	}

	template<typename T, unsigned int dim>
	inline VectorND<T, dim> VectorND<T, dim>::getMax(const thisType &p)
	{
		thisType		r;
		unsigned int	i;

		for(i = 0; i < dim; i++)
		{
			r[i] = max(v[i], p[i]);
		}

		return r;
	}



	template<typename T, unsigned int dim>
	bool ptds::Series<T, dim>::deleteRange(Index start, Index end)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return false;

		PointSet::iterator	itStart, itEnd;
		Index				n = getNumPoints();

		if(start >= n || end >= n || start > end)
		{
			return false;
		}

		itStart = points.begin() + start;
		itEnd	= points.begin() + end + 1;

		points.erase(itStart, itEnd);

		return true;
	}

	template<typename T, unsigned int dim>
	inline void Series<T, dim>::clear(void)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		points.clear();
	}


	template<typename T, unsigned int dim>
	void ptds::Series<T, dim>::setNumPointsMax(Index maxPoints)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		numPointsMax = maxPoints;
	}


	template<typename T, unsigned int dim>
	inline typename Series<T, dim>::Index Series<T, dim>::getNumPointsMax(void)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return 0;

		return numPointsMax;
	}


	template<typename T, unsigned int dim>
	inline unsigned int Series<T, dim>::getNumPoints(void)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return 0;

        return static_cast<uint>(points.size());
	}

	template<typename T, unsigned int dim>
	inline unsigned int Series<T, dim>::getNumPointsRange(void)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return 0;

		if(getIndexRangeEnabled() == false)
		{
			return getNumPoints();
		}

		return (getIndexRangeEnd() - getIndexRangeStart() + 1);
	}


	template<typename T, unsigned int dim>
	inline Series<T, dim>::Series(void)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		setNumPointsMax(1024 * 1024);
	}

	template<typename T, unsigned int dim>
	inline void ptds::Series<T, dim>::add(Point &point)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		points.push_back(point);

		deleteFirstToMaxPoints();
	}

	template<typename T, unsigned int dim>
	inline void ptds::Series<T, dim>::deleteFirstToMaxPoints(void)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		if(getNumPoints() > getNumPointsMax())
		{
			deleteRange(0, getNumPoints() - getNumPointsMax() - 1);
		}
	}


	template<typename T, unsigned int dim>
	inline Vector2d Series<T, dim>::get(void)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return Vector2d();

		unsigned int	numPoints;
		Vector2d		p;

		if((numPoints = getNumPoints()) > 0)
		{
			p = points[numPoints - 1];

			return p;		
		}

		p.clear();

		return p;
	}


	template<typename T, unsigned int dim>
	inline typename Series<T, dim>::Point Series<T, dim>::get(Index index)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return Point();

		if(getNumPoints() > 0)
		{
			return points[index];		
		}

		Point	p;

		p.clear();

		return p;
	}

	template<typename T, unsigned int dim>
	inline typename Series<T, dim>::Point &Series<T, dim>::operator[](Index index)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return points[0];

		return points[index];
	}


	template<typename T, unsigned int dim>
	void ptds::Series<T, dim>::updateRandom(void)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		unsigned int n	= getNumPoints();

		Point	p1 = (*this)[n-2];
		Point	p2 = (*this)[n-1];
		Point	p3;
		Point	delta = p2 - p1;

		deleteRange(0, 1);

		p3 = p2 + delta;

		p3[1] = static_cast<float>((getExtentsMax() - getExtentsMin())[1] * static_cast<double>(rand()) / static_cast<double>(RAND_MAX));

		add(p3);
	}


	template<typename T, unsigned int dim>
	void ptds::Series<T, dim>::generateSquareWave(unsigned int numPoints, Index primaryAxis, T interval, Point &offset, Point &minimum, Point &maximum)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		unsigned int t;

		Point delta = maximum - minimum;

		unsigned int	phase = 0;
		Point::Type		val;

		for(t = 0; t < numPoints; t++)
		{
			Point	p;

			p.clear();

			switch(phase)
			{
			case 0:
			case 1:
				val = 0;
				break;

			case 2:
			case 3:
				val = 1;
				break;

			default:
				;

			}

			p += val;

			p *= delta;

			p += minimum;

			p[primaryAxis] = offset[primaryAxis] + (static_cast<T>(t) * interval);

			add(p);

			phase++;
			if(phase == 4)
			{
				phase = 0;
			}
		}
	}


	template<typename T, unsigned int dim>
	void ptds::Series<T, dim>::generateRandom(unsigned int numPoints, Index primaryAxis, T interval, Point &offset, Point &minimum, Point &maximum)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		unsigned int t;

		Point delta = maximum - minimum;

		for(t = 0; t < numPoints; t++)
		{
			Point	p;
			
			p.setRandom();

			p *= delta;

			p += minimum;

			p[primaryAxis] = offset[primaryAxis] + (static_cast<T>(t) * interval);

			add(p);
		}
	}


	template<typename T, unsigned int dim>
	void Series<T, dim>::calculateMinMax(Vector2d &minimum, Vector2d &maximum)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		Index			i;
		unsigned int	numPoints;

		if((numPoints = getNumPointsRange()) < 2)
		{
			minimum.clear();
			maximum.clear();
			return;
		}

		i = this->getIndexRangeStart();

		minimum = maximum = get(i);

		for(i++; i <= getIndexRangeEnd(); i++)
		{
			Vector2d p;
			
			p = get(i);

			minimum = minimum.getMin(p);
			maximum = maximum.getMax(p);
		}

		setExtentsMin(minimum);
		setExtentsMax(maximum);
	}

	template<typename T, unsigned int dim>
	void Series<T, dim>::calculateMinMax(Point &minimum, Point &maximum)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		unsigned int	i;
		unsigned int	numPoints;

		if((numPoints = getNumPointsRange()) == 0)
		{
			minimum.clear();
			maximum.clear();
			return;
		}

		minimum = maximum = get(0);

		for(i = 1; i < numPoints; i++)
		{
			Point p = get(i);

			minimum = minimum.getMin(p);
			maximum = maximum.getMax(p);
		}

	}

	template<typename T, unsigned int dim>
	inline typename Series<T, dim>::Point Series<T, dim>::calculateMean(void)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return Point();

		Series<T, dim>::Point p((float *) NULL);

		unsigned int	i;
		unsigned int	numPoints;

		if((numPoints = getNumPointsRange()) == 0)
		{
			return p;
		}

		for(i = getIndexRangeStart(); i <= getIndexRangeEnd(); i++)
		{
			p += get(i);
		}

		p *= 1.0f / static_cast<T>(numPoints);

		return p;
	}

	template<typename T, unsigned int dim>
	inline typename Series<T, dim>::Point Series<T, dim>::calculateMeanLast(Index numLastItems)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return Point();

		Series<T, dim>::Point p((float *) NULL);

		unsigned int	i;
		unsigned int	numPoints;

		if((numPoints = getNumPointsRange()) == 0)
		{
			return p;
		}

		for(i = max(getIndexRangeStart(), getIndexRangeEnd() + 1 - numLastItems); i <= getIndexRangeEnd(); i++)
		{
			p += get(i);
		}

		p *= 1.0f / static_cast<T>(numPoints);

		return p;
	}


	template<typename T, unsigned int dim>
	typename Series<T, dim>::Type Series<T, dim>::calculateVariance(Index index, T *mean = NULL)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return Type();

		unsigned int	i;
		Index			numPoints;

		if((numPoints = getNumPointsRange()) == 0)
		{
			return 0;
		}

		Point meanPoint = calculateMean();

		double	variance = 0;
		double	deltaSqr;

		double meanIndex = meanPoint[index];

		for(i = getIndexRangeStart(); i <= getIndexRangeEnd(); i++)
		{
			deltaSqr = (get(i))[index] - meanIndex;
			deltaSqr *= deltaSqr;

			variance += deltaSqr;
		}

		variance /= numPoints;

		if(mean)
		{
			*mean = static_cast<T>(meanIndex);
		}

		return static_cast<T>(variance);
	}


	template<typename T, unsigned int dim>
	typename Series<T, dim>::Type Series<T, dim>::calculateStdDev(Index index, T *mean, T *variance)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return Type();

		T v = calculateVariance(index, mean);

		if(variance)
		{
			*variance = v;
		}

		return sqrt(v);
	}


	template<typename T, unsigned int dim>
	bool Series<T, dim>::calculateLinearLeastSquares2D(T &a, T &b, Index dimX, Index dimY)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return false;

		double					x, y, t;
		double					sx	= 0;
		double					sy	= 0;
		double					sty	= 0;
		double					stt	= 0;
		double					x_mean;
		unsigned int			i;
		unsigned int			n = getNumPoints();
															// If less than two samples, can't calculate
		if(n < 2)
		{
			a = 0;
			b = 0;
															// Return failed
			return false;
		}

															// Calculate sum x and sum y (sx and sy)
		for(i = 0; i < n; i++)
		{
			x = get(i)[dimX];
			y = get(i)[dimY];
															// Calculate sum x and sum y
			sx += x;
			sy += y;
		}

		x_mean = sx / n;
															// Calculate stt and sty constants
		for(i = 0; i < n; i++)
		{
			x = get(i)[dimX];
			y = get(i)[dimY];
															// Get x difference from mean
			t	= x - x_mean;
															// Calculate stt constant
			stt	+= t * t;
															// Calculate sty constant
			sty += t * y;
		}

															// Calculate a and b constants where y = ax + b
		a = static_cast<T>(sty / stt);

		b = static_cast<T>((sy - a * sx) / n);
															// Return OK
		return true;
	}


	template<typename T, unsigned int dim>
	typename Series<T, dim>::Point Series<T, dim>::projectPoint(Point &point, Vector2d &offset, Vector2d &scalar, unsigned int windowHeight)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return Point();

		Point	r;

		r = point;

		r += offset;
		r *= scalar;

		r[1] = windowHeight - r[1];
		
		return r;
	}

#ifdef HAVE_GDIPLUS
	inline void GraphEntity::drawEntityLabel(HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int entityCounter, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		wchar_t	currentValueStr[64];

		swprintf(currentValueStr, L"%.6f", static_cast<double>(get()[1]) * getUnitScale()[1]);

		GraphEntityStyle::ColorRGB	colorRGB = style.getColor();

		Gdiplus::Pen pen(Gdiplus::Color(255, (BYTE) colorRGB[0],(BYTE) colorRGB[1], (BYTE) colorRGB[2]));

		Gdiplus::Font* myFont = new Gdiplus::Font(L"Times New Roman", 10);
		Gdiplus::SolidBrush* myBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, (BYTE) colorRGB[0], (BYTE) colorRGB[1], (BYTE) colorRGB[2]));

		graphics.DrawString(name.c_str(), name.length(), myFont, Gdiplus::PointF((Gdiplus::REAL) (windowWidth - GRAPH_ENTITY_LABEL_POS_X_OFFSET), (Gdiplus::REAL) 5 + ((entityCounter - 1) * 20)), myBrush);

		graphics.DrawString(currentValueStr, wcslen(currentValueStr), myFont, Gdiplus::PointF((Gdiplus::REAL) (windowWidth - GRAPH_ENTITY_VALUE_POS_X_OFFSET), (Gdiplus::REAL) 5 + ((entityCounter - 1) * 20)), myBrush);
	}




	template<typename T, unsigned int dim>
	void ptds::Series<T, dim>::drawEntityLabel(HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int entityCounter, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		GraphEntity::drawEntityLabel(hWnd, hDC, graph, graphics, entityCounter, windowWidth, windowHeight, frustumMin, frustumMax, offset, scalar);

		T		mean;
		Point	p;
		wchar_t	currentValueStr[64];
		
		T stdDev = calculateStdDev(1, &mean);

		GraphEntityStyle::ColorRGB	colorRGB = style.getColor();
		Gdiplus::Pen pen(Gdiplus::Color(255, (BYTE) colorRGB[0],(BYTE) colorRGB[1], (BYTE) colorRGB[2]));
		Gdiplus::Font* myFont = new Gdiplus::Font(L"Times New Roman", 10);
		Gdiplus::SolidBrush* myBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, (BYTE) colorRGB[0], (BYTE) colorRGB[1], (BYTE) colorRGB[2]));

		swprintf(currentValueStr, L"%.6f", static_cast<double>(mean) * getUnitScale()[1]);
		graphics.DrawString(currentValueStr, wcslen(currentValueStr), myFont, Gdiplus::PointF((Gdiplus::REAL) (windowWidth - GRAPH_ENTITY_MEAN_POS_X_OFFSET), (Gdiplus::REAL) 5 + ((entityCounter - 1) * 20)), myBrush);

		swprintf(currentValueStr, L"%.6f", static_cast<double>(stdDev) * getUnitScale()[1]);
		graphics.DrawString(currentValueStr, wcslen(currentValueStr), myFont, Gdiplus::PointF((Gdiplus::REAL) (windowWidth - GRAPH_ENTITY_STDDEV_POS_X_OFFSET), (Gdiplus::REAL) 5 + ((entityCounter - 1) * 20)), myBrush);
	}


	template<typename T, unsigned int dim>
	void ptds::Series<T, dim>::drawAxisLines(HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		if(getNumPoints() == 0)
			return;

		GraphEntityStyle::ColorRGB	colorRGB = style.getColor();

		Gdiplus::Pen pen(Gdiplus::Color(255, (BYTE) colorRGB[0],(BYTE) colorRGB[1], (BYTE) colorRGB[2]));

		pen.SetWidth(2);
		pen.SetDashStyle(Gdiplus::DashStyleDash);

		if(frustumMin[0] < 0 && frustumMax[0] > 0)
		{
															// Draw Y axis
			Point	p1	= projectPoint(Point(0, frustumMin[1]), offset, scalar, windowHeight);
			Point	p2	= projectPoint(Point(0, frustumMax[1]), offset, scalar, windowHeight);

			graphics.DrawLine(&pen, p1[0], p1[1], p2[0], p2[1]);
		}

		if(frustumMin[1] < 0 && frustumMax[1] > 0)
		{
															// Draw X axis
			Point	p1	= projectPoint(Point(frustumMin[0], 0), offset, scalar, windowHeight);
			Point	p2	= projectPoint(Point(frustumMax[0], 0), offset, scalar, windowHeight);

			graphics.DrawLine(&pen, p1[0], p1[1], p2[0], p2[1]);
		}

	}


	template<typename T, unsigned int dim>
	void ptds::Series<T, dim>::drawAxisText(HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int entityCounter, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		Vector2d	extentsMin, extentsMax, extentsDelta;

		Vector2d	frustumDelta = frustumMax - frustumMin;

		Vector2d	pixelSize;

		if(getNumPoints() == 0)
			return;

		extentsMin = getExtentsMin();
		extentsMax = getExtentsMax();

		extentsDelta = extentsMax - extentsMin;

//		pixelSize[0] = frustumDelta[1] / static_cast<double>(windowHeight);
//		pixelSize[1] = frustumDelta[0] / static_cast<double>(windowWidth);

		pixelSize[0] = frustumDelta[0] / static_cast<double>(windowWidth);
		pixelSize[1] = frustumDelta[1] / static_cast<double>(windowHeight);

		unsigned int t;

		GraphEntityStyle::ColorRGB	colorRGB = style.getColor();

		if(graph.getEntityOverlayMode() == Graph::GraphEntityOverlayCombined)
		{
			colorRGB[0] = 255;
			colorRGB[1] = 255;
			colorRGB[2] = 255;
		}

		Gdiplus::Font* myFont = new Gdiplus::Font(L"Times New Roman", 7);
		Gdiplus::SolidBrush* myBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, (BYTE) colorRGB[0], (BYTE) colorRGB[1], (BYTE) colorRGB[2]));

		wchar_t	lineString[32];

		Point	left;
		Point	bottom;

		Vector2i	borderPixels = graph.getBorderPixelsMin();
		
		borderPixels *= graph.getNumEntities();

		for(t = borderPixels[1]; t < windowHeight; t += 20)
		{

			left[0]	= get(getIndexRangeStart())[0];

			left[1]	= (frustumMin[1] + (static_cast<double>(t) * pixelSize[1])) * getUnitScale()[1];

			swprintf(lineString, L"%.3f", left[1]);

			graphics.DrawString(lineString, wcslen(lineString), myFont, Gdiplus::PointF((Gdiplus::REAL) (5 + (entityCounter - 1) * 50), (Gdiplus::REAL) (windowHeight - t)), myBrush);
		}

		for(t = borderPixels[0]; t < windowWidth; t += 50)
		{
			bottom[0] = frustumMin[0] + (static_cast<double>(t) * pixelSize[0] * getUnitScale()[0]);

			swprintf(lineString, L"%.3f", bottom[0]);

			graphics.DrawString(lineString, wcslen(lineString), myFont, Gdiplus::PointF((Gdiplus::REAL) t, (Gdiplus::REAL) (windowHeight - 50 + (entityCounter * 15))), myBrush);
		}

		if(myFont)
			delete myFont;

		if(myBrush)
			delete myBrush;
	}


	template<typename T, unsigned int dim>
	void ptds::Series<T, dim>::draw(HWND hWnd, HDC hDC, Graph &graph, Gdiplus::Graphics &graphics, unsigned int &entityCounter, unsigned int windowWidth, unsigned int windowHeight, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar)
	{
		PTRMI::MutexScope mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
		if(mutexScope.isLocked() == false)
			return;

		beginStyle(hWnd, hDC);

		GraphEntityStyle::ColorRGB	colorRGB = style.getColor();

		Gdiplus::Pen pen(Gdiplus::Color(255, (BYTE) colorRGB[0], (BYTE)colorRGB[1], (BYTE) colorRGB[2]));

		Index			t;
		unsigned int	n = getNumPointsRange();

		if(n < 2)
		{
			entityCounter--;
			return;
		}

		drawAxisLines(hWnd, hDC, graph, graphics, windowWidth, windowHeight, frustumMin, frustumMax, offset, scalar);

		drawEntityLabel(hWnd, hDC, graph, graphics, entityCounter, windowWidth, windowHeight, frustumMin, frustumMax, offset, scalar);

		if(graph.getEntityOverlayMode() == Graph::GraphEntityOverlaySeparate || (graph.getEntityOverlayMode() == Graph::GraphEntityOverlayCombined && entityCounter == 1))
		{
			drawAxisText(hWnd, hDC, graph, graphics, entityCounter, windowWidth, windowHeight, frustumMin, frustumMax, offset, scalar);
		}


		Point p1, p2;


		t = getIndexRangeStart();

		p1 = get(t);
		p1 = projectPoint(p1, offset, scalar, windowHeight);

		for(t++; t <= getIndexRangeEnd(); t++)
		{
			p2 = get(t);

			p2 = projectPoint(p2, offset, scalar, windowHeight);

			graphics.DrawLine(&pen, p1[0], p1[1], p2[0], p2[1]);

			p1 = p2;
		}

		endStyle();
	}
#endif // HAVE_GDIPLUS

	template<typename T, unsigned int dim>
	inline VectorND<T, dim>::VectorND(T *init)
	{
		if(init == NULL)
		{
			clear();
			return;
		}

		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			v[t] = init[t];
		}
	}

	template<typename T, unsigned int dim>
	inline VectorND<T, dim>::VectorND(T v1)
	{
		set(v1);
	}

	template<typename T, unsigned int dim>
	inline VectorND<T, dim>::VectorND(T v1, T v2)
	{
		set(v1, v2);
	}

	template<typename T, unsigned int dim>
	inline VectorND<T, dim>::VectorND(T v1, T v2, T v3)
	{
		set(v1, v2, v3);
	}

	template<typename T, unsigned int dim>
	inline VectorND<T, dim>::VectorND(T v1, T v2, T v3, T v4)
	{
		set(v1, v2, v3, v4);
	}

	template<typename T, unsigned int dim>
	inline void VectorND<T, dim>::clear(void)
	{
		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			v[t] = 0;
		}
	}

	template<typename T, unsigned int dim>
	inline unsigned int VectorND<T, dim>::getSize(void) const
	{
		return dim;
	}

	template<typename T, unsigned int dim>
	inline typename VectorND<T, dim>::Type &VectorND<T, dim>::operator[](Index index)
	{
		if(index < getSize())
		{
			return v[index];
		}

		return v[0];
	}


	template<typename T, unsigned int dim>
	inline typename const VectorND<T, dim>::Type &VectorND<T, dim>::operator[](Index index) const
	{
		if(index < getSize())
		{
			return v[index];
		}

		return v[0];
	}


	template<typename T, unsigned int dim>
	inline VectorND<T, dim> &VectorND<T, dim>::operator-=(const thisType &p)
	{
		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			(*this)[t] -= p[t];
		}

		return *this;
	}


	template<typename T, unsigned int dim>
	inline VectorND<T, dim> &VectorND<T, dim>::operator/=(const thisType &p)
	{
		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			if(p[t] != 0)
			{
				(*this)[t] /= p[t];
			}
		}

		return *this;
	}

	template<typename T, unsigned int dim>
	inline VectorND<T, dim> &VectorND<T, dim>::operator+=(T v)
	{
		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			(*this)[t] += v;
		}

		return *this;
	}

	template<typename T, unsigned int dim>
	inline VectorND<T, dim> &VectorND<T, dim>::operator-=(T v)
	{
		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			(*this)[t] -= v;
		}

		return *this;
	}


	template<typename T, unsigned int dim>
	inline VectorND<T, dim> &VectorND<T, dim>::operator*=(T v)
	{
		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			(*this)[t] *= v;
		}

		return *this;
	}


	template<typename T, unsigned int dim>
	inline VectorND<T, dim> &VectorND<T, dim>::operator/=(T v)
	{
		unsigned int t;

		for(t = 0; t < dim; t++)
		{
			if(v != 0)
			{
				(*this)[t] /= v;
			}
		}

		return *this;
	}


	template<typename T, unsigned int dim>
	inline VectorND<T, dim> VectorND<T, dim>::operator+(T val)
	{
		unsigned int	t;
		thisType		r;

		for(t = 0; t < dim; t++)
		{
			r[t] = v[t] + val;
		}

		return r;
	}

	template<typename T, unsigned int dim>
	inline VectorND<T, dim> VectorND<T, dim>::operator-(T val)
	{
		unsigned int	t;
		thisType		r;

		for(t = 0; t < dim; t++)
		{
			r[t] = v[t] - val;
		}

		return r;
	}


	template<typename T, unsigned int dim>
	inline VectorND<T, dim> VectorND<T, dim>::operator*(T val)
	{
		unsigned int	t;
		thisType		r;

		for(t = 0; t < dim; t++)
		{
			r[t] = v[t] * val;
		}

		return r;
	}


	template<typename T, unsigned int dim>
	inline VectorND<T, dim> VectorND<T, dim>::operator/(T val)
	{
		unsigned int	t;
		thisType		r;

		for(t = 0; t < dim; t++)
		{
			if(val[t] != 0)
			{
				r[t] = v[t] / val;
			}
			else
			{
				r[t] = 0;
			}
		}

		return r;
	}


} // end ptds namespace