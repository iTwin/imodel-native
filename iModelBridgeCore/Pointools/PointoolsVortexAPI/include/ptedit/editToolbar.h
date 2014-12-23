#include <fltk/group.h>
#include <fltk/button.h>
#include <ptui/ptpanel.h>
#include <pt_edit/layersWidget.h>

namespace ptui
{
	class EditToolbar : public fltk::Group
	{
	public:
		EditToolbar(pt::String *layerNames);
		
		void layout();
		void draw();

		fltk::Button *_btnSelect;
		fltk::Button *_btnPaint;
		fltk::Button *_btnFilter;

		fltk::Widget *_lblEdit;
		ptui::LayerSelector *_layers;

		ptui::Panel *_pnlSelect;
		ptui::Panel *_pnlPaint;
		ptui::Panel *_pnlFilter;

		pt::String *_layerNames;

	};
}