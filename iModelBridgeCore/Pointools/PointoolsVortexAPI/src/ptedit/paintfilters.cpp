#include "PointoolsVortexAPIInternal.h"
#include <ptedit/paintFilters.h>
#include <ptcloud2/datachannel.h>
#include <ptedit/editstack.h>

using namespace pcloud;

namespace ptedit
{
PaintSettings		g_paint;
SelBrushSettings	g_selBrush;

typedef std::map<const pcloud::Voxel*, pcloud::DataChannel *> RGBMAP;
RGBMAP g_rgbmap; 

	void EraseFilterVisitor::clearRGBCache( pcloud::Voxel *v )
	{
		RGBMAP::iterator it = g_rgbmap.find(v);
		if (it == g_rgbmap.end())
		{
			delete it->second;
			g_rgbmap.erase(it);
		}
		/* TODO: close all file handles */ 
	}
	const ubyte *getOriginalRGB(const pcloud::Voxel*v, int p)
	{
		DataChannel * rgbdc = const_cast<DataChannel*>(v->channel(PCloud_RGB));
		if (!rgbdc) return 0;

		RGBMAP::iterator it = g_rgbmap.find(v);
		if (it == g_rgbmap.end())
		{
			DataChannel *ndc = new pcloud::DataChannel( UByte8, UByte8, 3 );
            ndc->allocate(static_cast<int>(v->fullPointCount()));
						
			/* get the offset position */ 
			uint channel_offset=0;
			for (int i=0; i<MAX_LOAD_CHANNELS; i++)
			{
				const pcloud::DataChannel * dc = v->channel(i);
				if (!dc) continue;
				if (i != (int)pcloud::PCloud_RGB)
				{
                    channel_offset += static_cast<uint>(v->fullPointCount() * dc->typesize() * dc->multiple());
				}
				else
				{
					ptds::DataPointer pos = v->filePointer();
					pos += channel_offset;

					ptds::DataSourcePtr fh = pointsengine::VoxelLoader::openFile(const_cast<Voxel*>(v));
					if(fh->validHandle())
					{
						fh->movePointerTo(pos);
						fh->readBytes(ndc->data(), ndc->bytesize());
						rgbdc = ndc;		

						g_rgbmap.insert( RGBMAP::value_type(v, rgbdc) );
					}
					else rgbdc = 0;
					break;
				}
			}
		}
		else rgbdc = it->second;
		
		if (rgbdc)
		{
			return (ubyte*)rgbdc->element(p);
		}
		return 0;
	}

	PaintSelect::PaintSelect()  : EditNodeDef("PaintSelect"), center(0,0,0)
	{
	}

	void PaintSelect::setCenter(const pt::vector3 &cen) 
	{ 
		center.set(cen); 
		
        float r = static_cast<float>(g_selBrush.brush.radius());
		box.setBox(r,-r, r,-r, r,-r);
		box.centerAt(pt::vector3d(cen.x, cen.y, cen.z));
	}
	void PaintSelect::setCenter(const pt::vector3d &cen) 
	{ 
		center.set(cen); 
		
		float r = static_cast<float>(g_selBrush.brush.radius());
		box.setBox(r,-r, r,-r, r,-r);
		box.centerAt(cen);
	}
	void BrushPaintRGB::setCenter(const pt::vector3 &cen) 
	{ 
		center.set(cen); 
		
		float r = static_cast<float>(g_selBrush.brush.radius());
		box.setBox(r,-r, r,-r, r,-r);
		box.centerAt(pt::vector3d(cen.x, cen.y, cen.z));
	}

	void PaintBrush::radius(double r) 
	{ 
		_radius = r; 
		_radius2 = r*r; 
	}

	SelectionResult PaintSelect::intersect(const PaintSelect &paint, const pt::BoundingBoxD &bb)
	{
		return PaintIntersect::intersect( paint.box, paint.center, bb ); 
	}

	void PaintSettings::readState( const pt::datatree::Branch* b )
	{
		float radius;
		int blendmode;
		int mode;
		int shape;
		unsigned int c;

		b->getNode("alpha", alpha);
		b->getNode("col", c);
		b->getNode("brush.radius", radius);
		b->getNode("brush.shape", shape);
		b->getNode("brush.mode", mode);
		b->getNode("blendmode", blendmode);
		blend = (BlendFunc)blendmode;
		brush.mode(mode);
		brush.radius(radius);
		brush.shape((BrushShape)shape);
		memcpy(col, &c, sizeof(col));
	}

	void PaintSettings::writeState( pt::datatree::Branch* b )
	{
		unsigned int c=0;
		memcpy(&c, col, sizeof(col));

		b->addNode("alpha", alpha);
		b->addNode("col", c);
		b->addNode("brush.radius", brush.radius());
		b->addNode("brush.shape", (int)brush.shape());
		b->addNode("brush.mode", (int)brush.mode());
		b->addNode("blendmode", (int)blend);	
	}
	PaintSettings::PaintSettings() : blend(PaintModeNormal), alpha(1.0f) 
	{
		static StateHandler s;
        s.read = fastdelegate::MakeDelegate(this, &PaintSettings::readState);
        s.write = fastdelegate::MakeDelegate(this, &PaintSettings::writeState);
		OperationStack::addStateHandler( &s );

		col[0] = 255; col[1] = 255; col[2] = 255; 
	}
	SelBrushSettings::SelBrushSettings()
	{
		static StateHandler s;
        s.read = fastdelegate::MakeDelegate(this, &SelBrushSettings::readState);
        s.write = fastdelegate::MakeDelegate(this, &SelBrushSettings::writeState);
		OperationStack::addStateHandler( &s );
	}
	void SelBrushSettings::readState( const pt::datatree::Branch* b )
	{
		float radius;
		int shape;

		if (b->getNode("sbrush.radius", radius))
			brush.radius(radius);
			
		if (b->getNode("sbrush.shape", shape))
			brush.shape((BrushShape)shape);
	}

	void SelBrushSettings::writeState( pt::datatree::Branch* b )
	{
		b->addNode("sbrush.radius", brush.radius());
		b->addNode("sbrush.shape", (int)brush.shape());
	}
}