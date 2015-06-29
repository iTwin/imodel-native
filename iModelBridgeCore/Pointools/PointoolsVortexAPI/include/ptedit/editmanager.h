#pragma once

#include <pt/rect.h>
#include <pt/fence.h>
#include <pt/viewparams.h>
#include <pt/project.h>
#include <pt/timestamp.h>
#include <pt/datatree.h>
#include <pt/boundingbox.h>

#include <boost/thread/mutex.hpp>

#include <ptgl/glviewstore.h>
#include <ptgl/glcamera.h>

#include <ptcloud2/voxel.h>

#include <ptengine/pointsscene.h>
#include <ptengine/engine.h>

#include <ptedit/editstack.h>
#include <ptedit/editFilters.h>
#include <ptedit/fenceFilter.h>
#include <ptedit/frustumFilter.h>
#include <ptedit/planeFilter.h>
#include <ptedit/paintFilters.h>
#include <ptedit/cloudSelect.h>
#include <ptedit/boxFilter.h>

namespace ptedit
{
class OperationStack;

class PointEditManager
{
public:
	PointEditManager();
	~PointEditManager();

	static PointEditManager *instance();

	void	regenEditQuick();
	void	regenEditQuick_run();
	void	regenEditComplete();
	void	regenOOCComplete( pcloud::Scene *scene=0 );	
	void	regenEditUnprocessed();	// run a regen on any scenes that have not been procesed at all

	void	clearEdit();

	void	setUnits( double unitsPerMetre );
	double	getUnits() const;

	// layers
	bool	lockLayer( int layer, bool lock );
	bool	showLayer( int layer, bool show );
	bool	setCurrentLayer( int layer, bool maskValue=false );
	void	updateActiveLayers();
	void	updateLayerVisibility();

	int		getCurrentLayer() const;
	bool	isLayerLocked( int layer ) const;
	bool	isLayerVisible( int layer ) const;
	bool	doesLayerHavePoints( int layer ) const;

	__int64 getLayerState() const;
	void	setLayerState(__int64 state);

	bool	moveSelToLayer( bool deselect );
	bool	copySelToLayer( bool deselect );

	bool	selectPointsInLayer( int layer );
	bool	deselectPointsInLayer( int layer );

	bool	selectPointcloud( pcloud::PointCloud *cloud );
	bool	selectScene( pcloud::Scene *scene );

	bool	getVisibleLayersBoundingBox( pt::BoundingBoxD &bb, bool fast_approx ) const;
	bool	getLayerBoundingBox( int layer, pt::BoundingBoxD &bb, bool fast_approx ) const;

	//editing commands
	void	paintSelSphere();
	void	paintSelCube();
	void	paintRadius( double radius );
	float	getPaintRadius();
	void	paintSelectAtPoint( const pt::vector3d &pnt, bool limit_range );

	void	rectangleSelect( int l, int r, int b, int t );
	void	fenceSelect( const pt::Fence<int> &fence );
	void	boxSelect( const pt::vector3d &lower, const pt::vector3d &upper );
	void	orientedBoxSelect( const pt::vector3d &lower, const pt::vector3d &upper, const pt::vector3d &position, const pt::vector3d &uAxis, const pt::vector3d &vAxis);
	void	planeSelect( const pt::vector3d &origin, const pt::vector3d &normal, double thickness );

	void	layersFromUserChannel( pointsengine::UserChannel* userChannel );

	// query
	__int64 countVisiblePoints();

	//editing operations
	void	selectAll();
	void	clearAll();
	void	deselectAll();
	void	resetSelection();
	void	showAll();
	void	invertSelection();
	void	invertVisibility();
	void	hideSelPoints();
	void	isolateSelPoints();

	// editing mode
	void			selectMode();
	void			deselectMode();
	void			unhideMode();
	SelectionMode	editMode() const;

	void			workingMode( EditWorkingMode mode );
	EditWorkingMode	workingMode() const;

	//id representing current state - used to check external caching validity
	int				stateId() const;

	//options
	void			setUseMultiThreading( bool use=true );
	bool			getUseMultiThreading() const;
	
	void			setViewParams( const pt::ViewParams &view );

	// management 
	bool				storeEdit( const pt::String name );
	bool				restoreEdit( const pt::String name );
	void				removeEdit (const pt::String name );
	void				removeAllEdits();
	uint				numEdits() const;
	const pt::String	&editName( uint index );
	OperationStack*		getCurrentEdit();
		
	//scope
	void setEditingScope( pcloud::PointCloudGUID guid, bool sceneScope, int sceneInstance=0 );
	void	clearEditingScope();

	//persistence
	pt::datatree::Branch *_getEditDatatree( int index );
	void	_createEditFromDatatree( pt::datatree::Branch * );

private:

	void				updateSceneEditStateID( pcloud::Scene *scene );

	typedef std::map<pt::String, OperationStack*> EditMap;

	OperationStack		m_currentEdit;

	boost::mutex		m_processMutex;

	EditMap				m_edits;
	pt::ViewParams		m_view;
	double				m_units;

	/* filters */ 
	FrustumSelect		m_frustumSelect;
	FenceSelect			m_fenceSelect;
	PlaneSelect			m_planeSelect;
	PaintSelect			m_paintSelect;
	CloudSelect			m_cloudSelect;
	BoxSelect			m_boxSelect;
	OrientedBoxSelect	m_orientedBoxSelect;

	FilterOpLayersFromUserChannel m_layersFromUserChannel;

};


}