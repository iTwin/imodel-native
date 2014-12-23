#include "graph.h"
#include <ptui/ptbox.h>
#include <fltk/draw.h>
#include <fltk/events.h>

using namespace ptui;
using namespace fltk;

OutputGraph::OutputGraph( int x, int y, int w, int h) : fltk::Widget(x,y,w,h)
{
	box( ptui::SIMPLE_DOWN_BOX );
	buttoncolor( fltk::color(60,60,60) );
	color( fltk::color(60,60,60) );

	_autoY = true;
	_autoX = true;
	_curValid = false;
}

OutputGraph::~OutputGraph()
{

}
void	OutputGraph::addValue( int graph, float x, float y )
{
	float xrange = _maxX - _minX;
	if (_autoY)
	{
		if (y < _minY) _minY = y;
		if (y > _maxY) _maxY = y;
	}
	if (_autoX)
	{
		if (x > _maxX - xrange * 0.1)
		{
			_maxX = x + xrange * 0.1;
			_minX = _maxX - xrange;
		}
	}
	_graphValues[graph].insert( std::pair<float, float>(x, y) );
}

void	OutputGraph::clearValues()
{
	_graphValues[0].clear();
	_graphValues[1].clear();
	_graphValues[2].clear();
	_graphValues[3].clear();
}
void	OutputGraph::clearLessThan( float xval )
{
	for (int g=0; g<4; g++)
	{
		std::map<float,float> newvals;
		std::map<float,float>::iterator i = _graphValues[g].begin();

		while (i != _graphValues[g].end())
		{
			if (i->first > xval)
				newvals.insert( std::pair<float, float>(i->first, i->second) );
		}	
		_graphValues[g].clear();
		_graphValues[g] = newvals;
	}
}

bool OutputGraph::valueToPixel( float xval, float yval, int &xp, int &yp ) const
{
	xval -= _minX;
	yval -= _minY;
	float xrange = (_maxX - _minX);
	float yrange =  (_maxY - _minY);
	bool oor = false;

	xval /= xrange;
	yval /= yrange;
	yval = 1.0f - yval; //(flip for scren coords)
	if (xval < 0 || yval < 0 || xval > 1.0f || yval > 1.0f)
	{
		oor = true;
	}

	xval *= w();
	yval *= h();

	xp = (int)xval;
	yp = (int)yval;

	return !oor;
}
bool OutputGraph::pixelToValue( int xp, int yp, float &xval, float &yval ) const
{
	float xrange = (_maxX - _minX);
	float yrange =  (_maxY - _minY);
	
	xval = xrange * ((float)xp / w()) + _minX;
	yval = yrange * ((float)(h() - yp) / h()) + _maxY;

	return true;
}

int		OutputGraph::handle( int event )
{
	int res =  Widget::handle( event );

	switch( event )
	{
	case fltk::MOVE:
		{
		_curValid = false;
		float xval, yval;
		pixelToValue( event_x(), event_y(), xval, yval );
		
		//find nearest x val
		std::map<float,float>::iterator i = _graphValues[0].begin();
		
		int index =0;

		while (i != _graphValues[0].end())
		{
			if (i->first > xval)
			{
				_curValueX = i->first;
				_curValueY = i->second;
		
				_curValid = true;

				if (index != _curIndex)	// only draw if changed
					redraw();

				_curIndex = index;
				break;
			}
			++i;
			++index;
		}
		break;
		}
	case fltk::LEAVE:
		_curValid = false;
		_curIndex = -1;
		redraw();
		break;
	}
	return res;
}

void OutputGraph::draw()
{
	Widget::draw_box();

	// draw cursor point
	fltk::setcolor( fltk::color(190, 130, 0) );
	int cx, cy;
	if (_curValid)
	{
		valueToPixel( _curValueX, _curValueY, cx, cy);
		fltk::fillrect( cx-2, cy-2, 5, 5);
	
		//crosshair
		fltk::setcolor( fltk::color(90,90,90) );
		fltk::drawline( cx, 0, cx, h() );
		fltk::drawline( 0, cy, w(), cy );

		//text
		fltk::setcolor( fltk::color(190, 130, 0) );

		char valuestr[64];
		float yval = _curValueY;
		const char *multiple = " ";

		if (yval > 1000000)
		{	
			yval /= 1000000;
			multiple = "m";
		}
		else if (yval > 1000)
		{
			yval /= 1000;
			multiple = "k";		
		}
	
		sprintf_s( valuestr, 64, "Timestamp: %d, Value: %0.2f%s", (int)_curValueX, yval, multiple);
	
		int xtpos = cx + 20;
		int ytpos = cy - 20;

		if (cx > w() * 0.5)
			xtpos = cx - 200;

		fltk::drawtext( valuestr, xtpos, ytpos );
	}
	// grid ??
	// graph
	float pxPerYVal = h() / (_maxY - _minY);
	float pxPerXVal = w() / (_maxX - _minX);

	// recompute max/min on sample
	float miny = _maxY;
	float maxy = _minY;

	for (int g=0; g<4; g++)
	{
		std::map<float,float>::iterator i = _graphValues[g].begin();

		switch (g)
		{
		case 0: fltk::setcolor( fltk::color(255, 255, 255) ); break;
		case 1: fltk::setcolor( fltk::color(255, 90, 90) ); break;
		case 2: fltk::setcolor( fltk::color(90, 255, 90) ); break;
		case 3: fltk::setcolor( fltk::color(90, 128, 255) ); break;
		}
		bool first = true;

		int lxp=0, lyp=0;
		
		while (i != _graphValues[g].end())
		{
			if (i->first > _minX && i->first < _maxX)
			{
				if (i->second < miny) miny = i->second;
				if (i->second > maxy) maxy = i->second;

				int xp=0, yp=0;

				valueToPixel( i->first, i->second, xp, yp );

				if (first)
					fltk::drawpoint( xp, yp );
				else
					fltk::drawline( lxp, lyp, xp, yp );

				first = false;
				lxp = xp;
				lyp = yp;
			}
			++i;
		}
	}
	if (_autoY)
	{
		float rangey = _maxY - _minY;
		float adj = rangey * 0.1f;

		if (_minY < miny - adj) _minY = miny - adj;
		if (_maxY > maxy + adj) _maxY = maxy + adj;
	}
}
