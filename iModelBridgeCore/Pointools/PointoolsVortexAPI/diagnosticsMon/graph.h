#pragma once

#include <fltk/widget.h>
#include <pt/ptstring.h>

#include <map>

namespace ptui
{
	class OutputGraph : public fltk::Widget
	{
	public:
		OutputGraph( int x, int y, int w, int h);
		virtual ~OutputGraph();

		void	draw();
		int		handle( int event );

		void	useAutoYRange( bool use )						{ _autoY = use; }
		void	useAutoXScroll( bool use )						{ _autoX = use; }

		void	setXRange( float min, float max )				{ _minX = min; _maxX = max; }
		void	setYRange( float min, float max )				{ _minY = min; _maxY = max; }

		void	getXRange( float &min, float &max )				{ min = _minX; max = _maxX; }
		void	getYRange( float &min, float &max )				{ min = _minY; max = _maxY; }

		void	setGridInterval( float inX, float inY );
		
		void	addValue( int graph, float x, float y );

		void	clearValues();
		void	clearLessThan( float xval );

		bool	valueToPixel( float xval, float yval, int &xp, int &yp ) const;
		bool	pixelToValue( int xp, int yp, float &xval, float &yval ) const;

	private:
		float		_minX;
		float		_maxX;
		float		_minY;
		float		_maxY;
		float		_gridX;
		float		_gridY;

		bool		_autoX;
		bool		_autoY;

		bool		_curValid;
		int			_curIndex;
		float		_curValueX;
		float		_curValueY;

		std::map<float, float>	_graphValues[4];

		pt::String				_xCaption;
		pt::String				_yCaption;
	};
}
