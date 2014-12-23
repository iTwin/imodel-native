#include <ptgl/glstate.h>
#include <ptengine/renderMethodGLPoints.h>
#include <assert.h>

using namespace pointsengine;

/*****************************************************************************/
/**
* @brief			Sets up OpenGL client state and vertex pointers and renders points
* @param buffer		Points buffer to be rendered
* @param settings	Current render settings. This is used to decide which buffers to render
* @return void
*/
/*****************************************************************************/
void RenderMethod_GLPoints::renderPoints( PointsBufferI *buffer, const RenderSettings *settings )
{
	const void *positionBuffer = buffer->getBufferPtr( Buffer_Pos );	// is always float
	const ubyte *rgbBuffer = (ubyte*)buffer->getBufferPtr( Buffer_RGB );		// is always ubyte
	const void *tex0Buffer = buffer->getBufferPtr( Buffer_TexCoord0 );			//usually intensity
	
	if (!positionBuffer) return;

	const float *posBufferF = reinterpret_cast<const float*>(positionBuffer);
	const short *posBufferS = reinterpret_cast<const short*>(positionBuffer);

	const float *tex0BufferF = reinterpret_cast<const float*>(tex0Buffer);
	const short *tex0BufferS = reinterpret_cast<const short*>(tex0Buffer);

	glColor3f(1.0f, 1.0f, 1.0f);

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	mmatrix4d mat;
	memcpy(&mat, ( buffer->getBufferTransform4x4( Buffer_Pos ) ), sizeof(double)*16);
	mat.transpose();
	glMultMatrixd( mat.data() );

	ptgl::State::flush();

	glActiveTexture( GL_TEXTURE0 );	//TEST
	glEnable( GL_TEXTURE_1D );		//TEST

	glBegin( GL_POINTS );
	for (int i=0; i<buffer->getNumPoints(); i++)
	{
		if (rgbBuffer && settings->isRGBEnabled())
		{
			glColor3ubv( rgbBuffer );
			rgbBuffer += 3;
		}
		if (tex0Buffer && settings->isIntensityEnabled())
		{
			switch ( buffer->getBufferType( Buffer_TexCoord0 ))
			{
			case pcloud::Float32:
				glTexCoord1f( tex0BufferF[i] );
				break;
			case pcloud::Short16:
				glTexCoord1s( tex0BufferS[i] );
			}
		}
		switch ( buffer->getBufferType( Buffer_Pos ))
		{
		case pcloud::Float32:
			glVertex3fv( posBufferF );
			posBufferF += 3;

			break;
		case pcloud::Short16:
			glVertex3sv( posBufferS );
			posBufferS += 3;
			break;
		default:
			assert(0);
		}
	}
	glEnd();

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

}