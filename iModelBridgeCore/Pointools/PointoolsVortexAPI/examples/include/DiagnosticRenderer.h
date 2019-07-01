/******************************************************************************

Pointools Vortex API Examples

ClassificationRenderer.h

Provides query based rendering for rendering classifications

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_DIAGNOSTIC_RENDERER_H_
#define POINTOOLS_EXAMPLE_DIAGNOSTIC_RENDERER_H_

#include "VortexExampleApp.h"
#include "QueryRender.h"

class DiagnosticRenderer : public QueryRender
{
public:
	DiagnosticRenderer(int buffersize=1e6);

	void			setDiagnosticChannel( PThandle channel );
	bool			drawPointClouds( bool dynamic, bool clearFrame );

private:

	void			initializeColMap();
	const PTubyte	*mapDiagnosticToColor();

	PTubyte			*m_diagnosticRGB;
	PTubyte			m_colMap[256*3];
	bool			m_mapInit;

	PThandle		m_channel;
};

#endif

