#include <gl/glew.h>
#include "ExampleApp.h"
#include "png.h"
//-----------------------------------------------------------------------------
void GLHelper::error(const char * s, ...)
//-----------------------------------------------------------------------------
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);

	exit(0);
}
GLubyte* GLHelper::loadPNG(const std::string &path, int &width, int &height, bool &has_alpha)
{
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr;
	png_infop info_ptr;
	int number_of_passes;
	png_bytep * row_pointers;

	png_byte header[8];

	std::string fpath = ExampleApp::instance()->getFullResourcePath(path);

	/* open file and test for it being a png */
	FILE *fp = fopen(fpath.c_str(), "rb");
	if (!fp)
		error("[read_png_file] File %s could not be opened for reading", path.c_str());

	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
		error("[read_png_file] File %s is not recognized as a PNG file", path.c_str());

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		error("[read_png_file] png_create_read_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		error("[read_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		error("[read_png_file] Error during init_io");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		error("[read_png_file] Error during read_image");

	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

	int row_bytes = (int)png_get_rowbytes(png_ptr, info_ptr);

	for (int y = 0; y<height; y++)
		row_pointers[y] = (png_byte*)malloc(row_bytes);

	png_read_image(png_ptr, row_pointers);

	// put this into a contingous array for GL
	GLubyte *image = new GLubyte[height * row_bytes];

	for (int y = 0; y < height; y++)
	{
		memcpy(&image[y * row_bytes], row_pointers[height - y - 1], row_bytes);
		free(row_pointers[height - y - 1]);
	}
	free(row_pointers);

	has_alpha = color_type == 2 ? false : true;

	fclose(fp);

	return image;
}
GLuint GLHelper::loadTexturePNG(const std::string &path, int &width, int &height, GLuint tex_type)
{
	GLuint texID = 0;

	bool has_alpha = true;
	GLubyte *image = loadPNG(path, width, height, has_alpha);
	
	/* create the GL resource */
	glActiveTextureARB(GL_TEXTURE0_ARB);

	glGenTextures(1, &texID);
	glBindTexture(tex_type, texID);

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	GLuint tex_format = has_alpha ? GL_RGBA : GL_RGB;

	glTexImage2D(tex_type, 0, tex_format, width, height, 0, tex_format, GL_UNSIGNED_BYTE, image);

	glTexParameteri(tex_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(tex_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameterf(tex_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(tex_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	delete[] image;

	return texID;
}
GLuint GLHelper::loadTextureJPG(const std::string &path, int &width, int &height, GLuint tex_type)
{
	return 0;
}

void GLHelper::renderFullViewportQuad(bool textureCoords)
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);

	//Projection setup
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 0.1f, 2);

	//Model setup
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Render the quad 
	glLoadIdentity();
	if (textureCoords) glColor3f(1, 1, 1);
	glTranslatef(0, 0, -1);

	glBegin(GL_QUADS);

		if (textureCoords) glTexCoord2f(0, 0);
		glVertex3f(-1, -1, 0.0f);

		if (textureCoords) glTexCoord2f(1, 0);
		glVertex3f(1, -1, 0.0f);

		if (textureCoords) glTexCoord2f(1, 1);
		glVertex3f(1, 1, 0.0f);

		if (textureCoords) glTexCoord2f(0, 1);
		glVertex3f(-1, 1, 0.0f);

	glEnd();

	//Reset to the matrices	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
