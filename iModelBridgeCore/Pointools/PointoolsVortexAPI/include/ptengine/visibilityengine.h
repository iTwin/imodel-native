/*----------------------------------------------------------*/ 
/* PointsVisibility.h										*/ 
/* Point Visibility Interface file							*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004-2008						*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTOOLS_VISIBILITY_DETERMINATION_ENGINE
#define POINTOOLS_VISIBILITY_DETERMINATION_ENGINE 1

#include <ptengine/ptengine_api.h>
#include <pt/boundingbox.h>
#include <pt/viewparams.h>

#include <ptengine/pointsscene.h>
#include <ptgl/glfrustum.h>
#include <ptengine/module.h>

namespace pointsengine
{

class PTENGINE_API VisibilityEngine : public Module
{
public:
	VisibilityEngine();
	~VisibilityEngine();

	bool		pause();
	bool		unpause();
	bool		initialize();

	void		setViewParameters( const pt::ViewParams &viewParams );
	void		stop();

	void		optimizerStrength(float strength)				{ m_optimizerStrength = strength; }
	float		optimizerStrength() const						{ return m_optimizerStrength; }

	void		minimumLOD(float min)							{ m_minLOD = min; }
	float		minimumLOD() const							{ return m_minLOD; }

	/* points budget for a view, use -1 for no limit budget */ 
	void		pointsBudget( int maxPnts )						{ m_pointsBudget = maxPnts; }
	int			pointsBudget() const							{ return m_pointsBudget; }

	const pt::Bounds<1, float> &priorityBounds() const	{ return m_priorityBounds; }

	/* unthreaded computation of visibilty - pauses thread if needed*/ 
	void		computeVisibility();

	/* set a fixed visibility value for data that is in view */ 
	void		setFixedVisibility(float vis);
	
	/* a counter indexing the iteration */ 
	int			getIteration() const;

	enum VisibilityBias
	{
		BiasFar,
		BiasNear,
		BiasScreen,
		BiasPoint
	};

	// load priority bias
	void			setBias( VisibilityBias b )					{ m_bias = b; }	
	VisibilityBias	getBias() const								{ return m_bias; }

	void		setBiasPoint( const pt::vector3 &pnt )			{ m_biasPnt = pnt; }
	const		pt::vector3 &getBiasPoint() const				{ return m_biasPnt; }

	/* points loaded shortfall per scene */ 
	typedef		std::map<const pcloud::Scene *, int64_t>		LoadedShortfallMap;

	void		getLastFramePntsShortfall( LoadedShortfallMap &result  )	{ result = m_loadShortfall; }
	void		computeCurrentPntsShortfall( const pcloud::Scene* inSceneOrNullForAll, LoadedShortfallMap &result  );

	static float	computeVoxelLOD( const pt::ViewParams &params, pcloud::Node * node, pt::vector3d basepoint, 
		float &projArea,
		float &altLOD );

	static float	computeVoxelPriority( const pt::ViewParams &params, VisibilityBias bias,  const pt::vector3 &biasPt, 
		pcloud::Node * vox, const pt::BoundingBoxD &extents );

	class OcclusionFrame
	{
	public:
		OcclusionFrame() : m_frame(0), m_vp_x(0), m_vp_y(0) {};

		void	setSize(int viewport_x_res, int viewport_y_res, float factor = 0.1f);
		float	factor() const	{ return m_factor; }
		
		bool	isOccluded(int x, int y, float z, int &vid) const;
		bool	insert(int x, int y, float z, int vid);
	

		void	clearFrame();

	private:

		struct Pixel 
		{ 
			int						vid;
			float					z;
		};
		Pixel	*m_frame;
		float	m_factor;
		int		m_vp_x;
		int		m_vp_y;
	};
private:

    std::mutex                  m_mutex;
	ptgl::Frustum				m_fr;
	pt::ViewParams				m_view;

	int							m_up;
	int							m_run;
	int							m_paused;
	int							m_working;
	int							m_waiting;
		
	int							m_pointsBudget;

	VisibilityBias				m_bias;
	pt::vector3					m_biasPnt;

	pt::Bounds<1, float>		m_priorityBounds;
	int							m_iteration;
		
	LoadedShortfallMap			m_loadShortfall;

	PointsScene::VOXELSLIST		m_voxlist;
	float						m_optimizerStrength;
	float						m_minLOD;
	void						*m_thread;
	
	OcclusionFrame				m_occFrame;
};
}
#endif