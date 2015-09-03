#include <pt/os.h>
#include <gl/glew.h>
#include <ptgl/glstate.h>
#include <ptengine/renderMethodGLVertexArray.h>
#include <ptengine/renderDiagnostics.h>
#include <assert.h>

#include <ptengine/renderVoxelBuffer.h>

using namespace pointsengine;

/*****************************************************************************/
/**
* @brief			Sets up OpenGL client state and vertex pointers and renders points
* @param buffer		Points buffer to be rendered
* @param settings	Current render settings. This is used to decide which buffers to render
* @return void
*/
/*****************************************************************************/
void RenderMethod_GLVertexArray::renderPoints( PointsBufferI *buffer, const RenderSettings *settings )
{
	const void *positionBuffer = buffer->getBufferPtr( Buffer_Pos );			// is always float
	const void *rgbBuffer = buffer->getBufferPtr( Buffer_RGB );					// is always ubyte
	const void *tex0Buffer = buffer->getBufferPtr( Buffer_TexCoord0 );			//usually intensity
	const void *normalBuffer = buffer->getBufferPtr( Buffer_Normal );			// can be float or short
	
	if (!positionBuffer)		return;		// serious failure ????, not always for voxelbuffer
	
	//------------------- position ----------------------
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	mmatrix4d mat;
	memcpy(mat.data(), buffer->getBufferTransform4x4(Buffer_Pos), sizeof(double) * 16);
	mat.transpose();				//transpose for GL
	glMultMatrixd( mat.data() );

	ptgl::ClientState::enable( GL_VERTEX_ARRAY );

	switch ( buffer->getBufferType( Buffer_Pos ))
	{
	case pcloud::Float32:
		glVertexPointer( 3, GL_FLOAT, 12, positionBuffer );
		break;
	case pcloud::Short16:
		glVertexPointer( 3, GL_SHORT, 6, positionBuffer );
		break;
	default:
		assert(0);
	}

	//------------------- rgb ----------------------
	if (rgbBuffer && settings->isRGBEnabled())
	{
		ptgl::ClientState::enable( GL_COLOR_ARRAY );
		glColorPointer( 3, GL_UNSIGNED_BYTE, 3, rgbBuffer );
	}
	else
	{
		ptgl::ClientState::disable( GL_COLOR_ARRAY );
	}

	//------------------- tex0 ----------------------
	if (tex0Buffer && settings->isIntensityEnabled())
	{
		ptgl::ClientState::enable( GL_TEXTURE_COORD_ARRAY );

		int dims = buffer->getBufferMultiple( Buffer_TexCoord0 );

		switch ( buffer->getBufferType( Buffer_TexCoord0 ))
		{
		case pcloud::Float32:
			glTexCoordPointer( dims, GL_FLOAT, dims * 4, tex0Buffer );
			break;
		case pcloud::Short16:
			glTexCoordPointer( dims, GL_SHORT, dims * 2, tex0Buffer );
			break;
		default:
			assert(0);	// not supported type
		}
	}
	else
	{
		ptgl::ClientState::disable( GL_TEXTURE_COORD_ARRAY );
	}	
	//------------------- lighting  ----------------------
	if (normalBuffer && settings->isLightingEnabled())
	{
		ptgl::ClientState::enable( GL_NORMAL_ARRAY );

		switch ( buffer->getBufferType( Buffer_Normal ))
		{
		case pcloud::Float32:
			glNormalPointer(GL_FLOAT, sizeof(GLfloat)*3, normalBuffer );
			break;

		case pcloud::Short16:
			glNormalPointer(GL_SHORT, sizeof(GLshort)*3, normalBuffer );
			break;
		default:
			assert(0);	// not supported type
		}
	}
	else
	{
		ptgl::ClientState::disable( GL_NORMAL_ARRAY );
	}
	//------------------- layers ----------------------
	// todo

	ptgl::ClientState::flush();
	ptgl::State::flush();

	texUnitHack( buffer, settings );

	// render points!
	uint numPoints = buffer->getNumPoints();
	glDrawArrays( GL_POINTS, 0, numPoints );

	glActiveTexture( GL_TEXTURE0 );

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}

void RenderMethod_GLVertexArray::texUnitHack( const PointsBufferI *buffer, const RenderSettings *settings ) const
{
	// a little hack - state flush does not handle texture units correctly
	glActiveTexture( GL_TEXTURE0 );

	const void *tex0Buffer = buffer->getBufferPtr( Buffer_TexCoord0 );			//usually intensity

	if (tex0Buffer && settings->isIntensityEnabled())
	{
		glEnable( GL_TEXTURE_1D );
	}
	else
	{
		glDisable( GL_TEXTURE_1D );
	}
	// this is a hack too, need to get proper texture unit handling sorted
	if (settings->isGeomShaderEnabled())
	{
		glActiveTexture( GL_TEXTURE1 );
		glEnable( GL_TEXTURE_1D );
	}
	else
	{
		glActiveTexture( GL_TEXTURE1 );
		glDisable( GL_TEXTURE_1D );
	}	
	if (buffer->getAvailableBuffers() & Buffer_Layers)
	{
		glActiveTexture( GL_TEXTURE2 );
		glEnable( GL_TEXTURE_2D );
	}	
	else
	{
		glActiveTexture( GL_TEXTURE2 );
		glDisable( GL_TEXTURE_2D );
	}
	glActiveTexture( GL_TEXTURE0 );
}