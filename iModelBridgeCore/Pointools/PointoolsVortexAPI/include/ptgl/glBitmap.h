/*--------------------------------------------------------------------------*/ 
/*	Pointools glBitmap class definition and implementation					*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_GLBITMAP_DEFINITION
#define POINTOOLS_GLBITMAP_DEFINITION
#ifdef _WIN32

namespace ptgl
{
class Bitmap
{
public:
	Bitmap()
	{
		m_dc = 0;
		m_rc = 0;
		m_bits = 0;
		m_bitmap = 0;
	};
	~Bitmap()
	{
		DestroyOffscreen();
	};

	/*member variables for offscreen context*/
	HDC		m_dc;
	HGLRC		m_rc;
	HBITMAP 	m_bitmap;
	GLubyte		*m_bits;
	BITMAPINFO	m_info;

//---------------------------------------------------------------------------
// ReadDIBitmap()
// Reads the current OpenGL Viewport into a 24-bit RGB bitmap
// returns bitmap on success
//---------------------------------------------------------------------------
	static GLubyte *ReadDIBitmap(BITMAPINFO **info)
	{
		long bitsize;
		long width;

		GLint viewport[4];
		GLubyte *bits = 0;
		//GLubyte *rgb;

		//get viewport
		glGetIntegerv(GL_VIEWPORT, viewport);

		//allocate mem - caller must free!!
		if ((*info = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER))) == NULL)
		{
			return NULL;
		}

		width = viewport[2] *3;
		width = (width + 3) & ~3;
		bitsize = width * viewport[3];

		bits = new GLubyte[bitsize];
		if (!bits) return 0;

		//read pixels from frame buffer
		glFinish();
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_SKIP_ROWS, 0);
		glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

		glReadPixels(0,0,viewport[2], viewport[3], GL_BGR_EXT, GL_UNSIGNED_BYTE, bits);

		(*info)->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
		(*info)->bmiHeader.biWidth			= viewport[2];
		(*info)->bmiHeader.biHeight			= viewport[3];
		(*info)->bmiHeader.biPlanes			= 1;
		(*info)->bmiHeader.biBitCount		= 24;
		(*info)->bmiHeader.biCompression	= BI_RGB;
		(*info)->bmiHeader.biSizeImage		= bitsize;
		(*info)->bmiHeader.biXPelsPerMeter	= 2952; //72 dpi
		(*info)->bmiHeader.biYPelsPerMeter	= 2952; //72 dpi
		(*info)->bmiHeader.biClrUsed		= 0;
		(*info)->bmiHeader.biClrImportant	= 0;

		return (bits);
	}
	//
	static int SaveDIBitmap(const char *filename, BITMAPINFO *info, GLubyte *bits)
	{
		FILE	*fp;
		int size, infosize, bitsize;
		BITMAPFILEHEADER	header;

		if ((fp = fopen(filename, "wb")) == NULL)
			return -1;
		if (info->bmiHeader.biSizeImage == 0)
			bitsize = (info->bmiHeader.biWidth * info->bmiHeader.biBitCount + 7) / 8 * abs(info->bmiHeader.biHeight);
		else
			bitsize = info->bmiHeader.biSizeImage;

		infosize = sizeof(BITMAPINFOHEADER);
		size = sizeof(BITMAPFILEHEADER);

		header.bfType = 'MB';
		header.bfSize = size;
		header.bfReserved1 = 0;
		header.bfReserved2 = 0;
		header.bfOffBits = sizeof(BITMAPFILEHEADER) + infosize;

		if (fwrite(&header, 1, sizeof(BITMAPFILEHEADER), fp) < sizeof(BITMAPFILEHEADER))
		{
			fclose(fp);
			return -1;
		}
		if (fwrite(info, 1, infosize, fp) < infosize)
		{
			fclose(fp);
			return -1;
		}

		if (fwrite(bits, 1, bitsize, fp) < bitsize)
		{
			fclose(fp);
			return -1;
		}
		fclose(fp);
		return 0;
	}
	//
	bool CreateOffscreen(int w, int h, bool accum = false)
	{
		if (m_dc) DestroyOffscreen();

		m_dc =  CreateCompatibleDC(NULL);

		memset(&m_info, 0, sizeof(m_info));
		m_info.bmiHeader.biSize	= sizeof(BITMAPINFOHEADER);
		m_info.bmiHeader.biWidth	= w;
		m_info.bmiHeader.biHeight	= h;
		m_info.bmiHeader.biPlanes	= 1;
		m_info.bmiHeader.biBitCount = 24;
		m_info.bmiHeader.biCompression = BI_RGB;

		m_bitmap = CreateDIBSection(m_dc, &m_info, DIB_RGB_COLORS, (void**)&m_bits, m_bitmap, 0);

		SelectObject(m_dc, m_bitmap);

		PIXELFORMATDESCRIPTOR	pfd;
		int						pf;

		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize		= sizeof(pfd);
		pfd.nVersion	= 1;
		pfd.dwFlags		= PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL;
		pfd.iPixelType	= PFD_TYPE_RGBA;
		pfd.cColorBits	= 24;
		pfd.cRedBits	= 8;
		pfd.cGreenBits	= 8;
		pfd.cBlueBits	= 8;
		pfd.cDepthBits	= 32;

		if (accum)
		{
			pfd.cAccumBits	= 32;
		}

		pf = ChoosePixelFormat(m_dc, &pfd);
		SetPixelFormat(m_dc, pf, &pfd);

		m_rc = wglCreateContext(m_dc);

		return (m_rc) ? true : false;
	};
	//
	void DestroyOffscreen()
	{
		wglDeleteContext(m_rc);
		DeleteObject(m_bitmap);
		DeleteDC(m_dc);
		m_dc = 0;
		m_rc = 0;
	};
};
};
#endif
#endif