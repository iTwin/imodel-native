#ifndef POINTOOLS_POINTSENGINE_IMPORT_H
#define POINTOOLS_POINTSENGINE_IMPORT_H

#include <vector>
#include <assert.h>

#include <pt/ptmath.h>

#include <ptfs/filepath.h>
#include <pt/datatable.h>
#include <pttool/tool.h>
#include <ptcloud2/indexstream.h>
#include <ptcmdppe/cmdprogress.h>
#include <ptengine/ptengine_api.h>

#ifdef POINTSIO_EXPORTS
#define PNTSIO_API __declspec(dllexport)
#else
#define PNTSIO_API __declspec(dllimport)
#endif
//! Points Importer plugin base class
/*!
Override this class to create a points importer plugin

This should be registered against the extensions it supports
Future versions will allow multiple plugins to support the same
extension and give the user the choice of which one to use.
This will be useful in the case of ascii data for example where
lidar ascii or small object may be handled differently, or to support
advanced vs. simple interfaces.

Dependencies on other libraries will be reduced in the future by providing
addGroup,Cloud and Point methods here and remove direct use of IndexStream
and Progress classes.

*/
namespace pt
{
class PNTSIO_API PointsImporter : public Tool
{
public:
	PointsImporter();
	virtual ~PointsImporter();

	static void registerImportType(const ptds::FileType &ft, PointsImporter*pi);
	static PointsImporter *importer(const char*extension);

	virtual bool initialize();

	virtual char* interfaceScript() const=0;
	virtual bool importSingleFile(const ptds::FilePath &p)=0;
	virtual void fileType(ptds::FileType &f) const=0;

	virtual bool prepareOptionsDialog();
	virtual bool readUserOptions();

	static void addFile();
	static int64_t totalFileSize();
	static int64_t fileSize(const ptds::FilePath &path);
	static void clearFiles();
	static void addFolder();

	void addCloud(int ibound, int jbound, uint spec, mmatrix4d reg);
	void addPoint(const pt::vector3d &p);
	void addNull();

	virtual bool importFiles(const ptds::FilePath &first);
	virtual const char* windowName() const;
	virtual bool handlesVersion(const ptds::FilePath &file) const { return true; }
	void updateHeader();

protected:

	const ptds::FilePath &firstFile() const;
	
	bool _readintensity;
	bool _readrgb;
	bool _readnormals;
	bool _combine;
	bool _gennormals;
	bool _pointorder;

	double _geomscaler;
	double _intensityscaler;
	double _rgbscaler;
	float _accuracy;
	float _uniform_spacing;
	
	float _normalquality;
	bool _scaleintensities;

	pt::ParameterMap _parameterMap;
	pt::vector3d _basepoint;

	ptapp::CmdProgress *_progress;
	pcloud::IndexStream *_stream;

	uint prepareImport();
	static void setActiveImporter(PointsImporter *i);
};
}
#endif