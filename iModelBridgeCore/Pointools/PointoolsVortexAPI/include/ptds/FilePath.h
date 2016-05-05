#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include <pt/unicodeconversion.h>
#include <pt/ptmath.h>

#ifndef NO_DATA_SOURCE_SERVER
#include <PTRMI/Array.h>
#endif

/*-----------------------------------------------------------------------*/
/* FilePath class														 */
/*-----------------------------------------------------------------------*/

#define PT_MAXPATH 512

#ifndef NO_DATA_SOURCE_SERVER
namespace PTRMI { class DataBuffer; }
#endif



namespace ptds
{


class FilePath
{
public:

	FilePath()
	{
		m_parent = 0;
		m_bAbsolute = true;
		memset(m_sPath, 0,PT_MAXPATH*sizeof(wchar_t)); 
		wcscpy_s(m_sPath, PT_MAXPATH, L"(empty)");
	}

	FilePath(const wchar_t* path)
	{
		m_parent = 0;
		m_bAbsolute = true;
		wcsncpy_s(m_sPath, path, PT_MAXPATH);
	}

	FilePath(const char* path)
	{
		m_parent = 0;
		m_bAbsolute = true;
		wcsncpy_s(m_sPath, pt::Ascii2Unicode::convert(path).c_str(), PT_MAXPATH);
	}

	~FilePath() {}

	inline bool operator == (const FilePath &b) const
	{
		return wcscmp(path(), b.path()) == 0;
	}

	inline bool operator < (const FilePath &b) const
	{
		return wcscmp(path(), b.path()) < 0;
	}

	inline bool operator > (const FilePath &b) const
	{
		return wcscmp(path(), b.path()) > 0;
	}

	inline FilePath& operator = (const FilePath &p)
	{
		wcsncpy_s(m_sPath, p.m_sPath, PT_MAXPATH);
		m_bAbsolute = p.m_bAbsolute;
		m_parent = p.m_parent;

		return (*this);
	}

	void setPath(const wchar_t* path)
	{
		wcsncpy_s(m_sPath, path, PT_MAXPATH);
	}

	void makeRelativeToProject()
	{
		makeRelativeTo(FilePath(getProjectDirectory()));
	}

	void stripPath()
	{
		PathStripPathW(m_sPath);
	}

	void stripExtension()
	{
		PathRemoveExtensionW(m_sPath);
	}

	void stripFilename()
	{
		PathRemoveFileSpecW(m_sPath);
	}

	void setFilenameOnly(const wchar_t *fn)
	{
		wchar_t ext[10];
		wchar_t dr[PT_MAXPATH];
		wcscpy_s(ext, 10, extension());
		stripExtension();

		directory(dr);

		_snwprintf_s(m_sPath, PT_MAXPATH, PT_MAXPATH-1, L"%s\\%s.%s", dr, fn, ext);			
	}

	const wchar_t* filename() const
	{
		wchar_t f[PT_MAXPATH];
		wcscpy_s(f, PT_MAXPATH, m_sPath);
		PathStripPathW(f);
		int l = (int)wcsnlen_s(m_sPath, PT_MAXPATH);
		return &m_sPath[l-wcsnlen_s(f, PT_MAXPATH)];
	}

	const wchar_t* extension() const
	{
		return &(PathFindExtensionW(m_sPath))[1];
	}

	void directory(wchar_t *dst) const
	{
		wcscpy_s(dst, PT_MAXPATH, m_sPath);
		PathRemoveFileSpecW(dst);
	}

	void fulldirectory(wchar_t *dst) const
	{
		if (m_bAbsolute)
		{
			directory(dst);
			return;
		}
		wchar_t d[PT_MAXPATH];
		dst[0] = '\0';

		const FilePath *par = m_parent;

		std::vector <const FilePath*> ancestors;

		while (par)
		{
			ancestors.push_back(par);
			par = par->parent();
		}
		std::reverse(ancestors.begin(), ancestors.end());
		std::vector <const FilePath*>::iterator it = ancestors.begin();

		while (it != ancestors.end())
		{
			m_parent->directory(d);
			PathAppendW(dst, d);
			++it;
		}
	}

	const wchar_t* path() const
	{
		return m_sPath;
	}

	void fullpath(wchar_t *dst) const
	{
		if (m_bAbsolute)
		{
			wcscpy_s(dst, PT_MAXPATH, m_sPath);
			return;
		}
		fulldirectory(dst);
		PathAppendW(dst, m_sPath);
	}

	void setExtension(const wchar_t*ext)
	{
		if (ext[0] != '.')
		{
			wchar_t _ext[64];
			_snwprintf_s(_ext, 14, 15, L".%s", ext);
			PathRenameExtensionW(m_sPath, _ext);
		}
		else PathRenameExtensionW(m_sPath, ext);
	}

	const FilePath *parent() const { return m_parent; }

	bool findFile(wchar_t *dst, bool parent_dir = true, bool project_dir = true);

	void setAbsolute()
	{
		m_bAbsolute = true;
	}

	void setRelative()
	{
		m_bAbsolute = false;
	}

	bool isRelative() const
	{
		return !m_bAbsolute;
	}

	bool isEmpty() const
	{
		if ((wcscmp(m_sPath, L"(empty)") == 0)
			|| wcslen(m_sPath) == 0 )
			return true;
		return false;
	}

	void setParent(const FilePath *f)
	{
		m_parent = f;
	}

	bool checkExists() const
	{
		// Note: Handle structured storage here ?

		wchar_t p[PT_MAXPATH];
		fullpath(p);
		return (_waccess(p,0) == 0) ? true : false;
	}

	void makeRelativeTo(const FilePath &f)
	{
		m_bAbsolute = false;
		m_parent = &f;

		wchar_t path[PT_MAXPATH];
		wchar_t fp[PT_MAXPATH];

		f.fullpath(fp);

		PathRelativePathToW(path,fp, FILE_ATTRIBUTE_NORMAL,	m_sPath, FILE_ATTRIBUTE_NORMAL);
		setPath(path);
	}

	static bool checkExists(const std::wstring &p)
	{
		return (_waccess(p.c_str(),0) == 0) ? true : false;
	}

	static bool isSlash(wchar_t c)
	{
		return (c == '\\') || (c == '/');
	}

	static void parseFilename(const std::wstring&  filename, std::wstring& root, std::vector<std::wstring>& path, std::wstring &base, std::wstring &ext)
	{
		std::wstring f = filename;

		root = L"";
		path.clear();
		base = L"";
		ext = L"";

		if (f == L"")
		{
				// Empty filename
			return;
		}

			// See if there is a root/drive spec.
		if ((f.size() >= 2) && (f[1] == ':'))
		{

			if ((f.size() > 2) && isSlash(f[2]))
			{
					// e.g.  c:\foo
				root = f.substr(0, 3);
				f = f.substr(3, f.size() - 3);

			} 
			else 
			{

					// e.g.  c:foo
				root = f.substr(2);
				f = f.substr(2, f.size() - 2);
			}

		}
		else if ((f.size() >= 2) & isSlash(f[0]) && isSlash(f[1]))
		{
				// e.g. //foo
			root = f.substr(0, 2);
			f = f.substr(2, f.size() - 2);
		}
		else if(isSlash(f[0]))
		{
			root = f.substr(0, 1);
			f = f.substr(1, f.size() - 1);
		}

			// Pull the extension off
		{
				// Find the period
			size_t i = f.rfind(L'.');

				// Make sure it is before a slash!
			size_t j = pt::iMax((int)f.rfind(L'/'), (int)f.rfind(L'\\'));
			if ((i != std::wstring::npos) && (i > j))
			{
				ext = f.substr(i + 1, f.size() - i - 1);
				f = f.substr(0, i);
			}
		}

			// Pull the basename off
		{
				// Find the last slash
			size_t i = (size_t)pt::iMax((int)f.rfind(L'/'), (int)f.rfind(L'\\'));

			if (i == std::wstring::npos)
			{
					// There is no slash; the basename is the whole thing
				base = f;
				f    = L"";
			}
			else if ((i != std::wstring::npos) && (i < f.size() - 1))
			{
				base = f.substr(i + 1, f.size() - i - 1);
				f    = f.substr(0, i);

			}
		}

			// Parse what remains into path.
		size_t prev, cur = 0;

		while (cur < f.size())
		{
			prev = cur;

				// Allow either slash
			size_t i = f.find(L'/', prev + 1);
			size_t j = f.find(L'\\', prev + 1);
			if (i == std::wstring::npos)
			{
				i = f.size();
			}

			if (j == std::wstring::npos)
			{
				j = f.size();
			}

			cur = (int)pt::iMin((int)i, (int)j);

			if (cur == std::wstring::npos)
			{
				cur = f.size();
			}

			path.push_back(f.substr(prev, cur - prev));
			++cur;
		}
	}

	/*project directory*/ 
	static const wchar_t* getProjectDirectory()
	{
		return projectDirectory();
	}

	static void setProjectDirectory(const wchar_t *path)
	{
		wcscpy_s(projectDirectory(), PT_MAXPATH, path);
		PathRemoveFileSpecW(projectDirectory());
	}

	static void makeProjectDirectoryCurrent()
	{
		SetCurrentDirectoryW(projectDirectory());
	}

	static int maxFilePath()
	{ 
		return PT_MAXPATH; 
	}

#ifdef NEEDS_WORK_VORTEX_DGNDB
	static ptds::FilePath applicationDataFolder()
	{
		LPITEMIDLIST pidl;
		if (SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pidl) == NOERROR)	
		{
			wchar_t p[PT_MAXPATH];
			SHGetPathFromIDListW(pidl,p);
			return ptds::FilePath(p);
		}
	}
#endif

#ifndef NO_DATA_SOURCE_SERVER

	void read	(PTRMI::DataBuffer &buffer);
	void write	(PTRMI::DataBuffer &buffer) const;

#endif

private:

	static wchar_t *projectDirectory()
	{
		static wchar_t dir[PT_MAXPATH] = L"\0";
		return dir;
	}
	wchar_t  m_sPath[PT_MAXPATH];

	bool m_bAbsolute;

	const FilePath *m_parent;
};

} // End ptfs namespace