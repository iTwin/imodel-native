#pragma once

#include <pt/boundingbox.h>
#include <ptengine/pointsscene.h>
#include <ptengine/engine.h>
#include <ptcloud2/voxel.h>
#ifdef HAVE_OPENGL
#include <ptgl/glviewstore.h>
#include <ptgl/glcamera.h>
#endif
#include <pt/rect.h>
#include <pt/fence.h>
#include <pt/viewparams.h>
#include <pt/project.h>
#include <pt/timestamp.h>
#include <pt/datatree.h>

namespace ptedit
{
class PointEdit
{
public:
	PointEdit();
	~PointEdit();

	static PointEdit *instance();

	void refreshEdit();
	void clearEdit();

	void setUnits( double unitsPerMetre );
	double getUnits() const;

	bool lockLayer( int layer, bool lock );
	bool showLayer( int layer, bool show );
	bool setCurrentLayer( int layer );
	
	int getCurrentLayer() const;
	bool isLayerLocked( int layer ) const;
	bool isLayerVisible( int layer ) const;
	
	int64_t getLayerState() const;
	void setLayerState(int64_t state);

	void paintSelSphere();
	void paintSelCube();
	void paintRadius( float radius );
	float getPaintRadius();
	void paintSelectAtPoint( const pt::vector3 &pnt, bool limit_range );

	void rectangleSelect( int l, int r, int b, int t );
	void fenceSelect( const pt::Fence<int> &fence );
	void boxSelect( const pt::vector3 &lower, const pt::vector3 &upper );
	void orientedBoxSelect( const pt::vector3 &lower, const pt::vector3 &upper, const pt::vector3 &position, const pt::vector3 &uAxis, const pt::vector3 &vAxis);
	void planeSelect( const pt::vector3 &origin, const pt::vector3 &normal, float thickness );
	void selectMode();
	void deselectMode();
	void unhideMode();

	void setUseMultiThreading( bool use=true );
	bool getUseMultiThreading() const;

	void selectAll();
	void clearAll();
	void deselectAll();
	void showAll();
	void invertSelection();
	void invertVisibility();
	void hideSelPoints();

	void setViewParams( const pt::ViewParams &view );

	/* management */ 
	bool storeEdit( const pt::String name );
	bool restoreEdit( const pt::String name );
	void removeEdit (const pt::String name );
	void removeAllEdits();
	uint numEdits() const;
	const pt::String &editName( uint index );
		
	pt::datatree::Branch *_getEditDatatree( int index );
	void _createEditFromDatatree( pt::datatree::Branch * );

	void filterVoxel( pcloud::Voxel *vox );

	bool moveSelToLayer( bool deselect );
	bool copySelToLayer( bool deselect );

	void setEditingScope( pcloud::PointCloudGUID guid, bool sceneScope );
	void clearEditingScope();

private:

	typedef std::map<pt::String, void*> EditMap;
	EditMap _edits;
	pt::ViewParams _view;
	double _units;
};


}