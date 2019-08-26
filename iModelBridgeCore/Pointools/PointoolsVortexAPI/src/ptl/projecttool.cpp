#include <ptl/projecttool.h>
#include <ptl/project.h>
#include <pt/parametermap.h>

static pt::Project* _instance = 0;
static pt::ParameterMap *dtable() { static pt::ParameterMap dt; return &dt; };

//
// construct
//
pt::Project::Project()
{
	if (_instance) 
	{
		exit(0);
	}
	_instance = this;
}
//
// destruct
//
pt::Project::~Project()
{

}
//
// initialize
//
bool pt::Project::initialize()
{
	registerCmd("Project.properties", &pt::Project::properties);
	return true;
}
//
// properties
//
void pt::Project::properties()
{
	static bool init = false;
	if (!init)
	{
		/*register tools*/ 
		buildWindow("scripts\\ui\\projprop.script", dtable());

		init = true;
	}
	dtable()->set("ptitle", String(ptl::Project::project()->title()));
	dtable()->set("author", String(ptl::Project::project()->author()));
	dtable()->set("company", String(ptl::Project::project()->company()));
	dtable()->set("keywords", String(ptl::Project::project()->keywords()));
	dtable()->set("commentsstr", String(ptl::Project::project()->comments()));
	
	updateUI("properties");
	showWindow("properties");
	
	pt::String title, company, author, keywords, comments;

	dtable()->get("ptitle", title);
	dtable()->get("company", company);
	dtable()->get("author", author);
	dtable()->get("keywords", keywords);
	dtable()->get("commentsstr", comments);

	ptl::Project::project()->title(title.c_str());
	ptl::Project::project()->author(author.c_str());
	ptl::Project::project()->company(company.c_str());
	ptl::Project::project()->keywords(keywords.c_str());
	ptl::Project::project()->comments(comments.c_str());
}
