#pragma once

#include <ptedit/editFilter.h>

#define FILTER_IMPL(n,d,i,multithread,consolidate) \
		const char *name() const { return n; } \
		const char *desc() const { return d; } \
		int icon() const { return (i); } \
		void loadStateBranch(const pt::datatree::Branch *b) {} \
		int value() const { return 0; } \
		FILTER_FLAGS(multithread, consolidate)

namespace ptedit
{
	namespace detail
	{
	struct FilterOpClearAll
	{
		FILTER_IMPL("ClearAll", "Reset Editing", 6, true, false)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			voxel->flag(pcloud::WholeSelected, false, false);
			voxel->flag(pcloud::PartSelected, false, false);
			voxel->flag(pcloud::WholeHidden, false, false);
			voxel->flag(pcloud::PartHidden, false, false);
			voxel->destroyEditChannel();
		}
		void processFilter()
		{
			ClearFilterVisitor v;
			TraverseScene::withVisitor(&v);

			TraverseScene::withFlag(pcloud::WholeSelected, false);
			TraverseScene::withFlag(pcloud::PartSelected, false);
			TraverseScene::withFlag(pcloud::WholeHidden, false);
			TraverseScene::withFlag(pcloud::PartHidden, false);
		}
	};

	struct FilterOpDeselectAll
	{
		FILTER_IMPL("DeselectAll", "Deselect All", 6, true, true)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			voxel->flag(pcloud::WholeSelected, false, false);
			if (voxel->layers(1) & g_activeLayers == g_activeLayers)
				voxel->flag(pcloud::PartSelected, false, false);
			
			DeselectPointsVisitor v;
			v.visitNode(voxel);
		}
		void processFilter()
		{
			DeselectPointsVisitor v;
			TraverseScene::withVisitor(&v);
		}
	};

	struct FilterOpSelectAll
	{
		FILTER_IMPL("SelectAll", "Select All", 7, true, false)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			voxel->flag(pcloud::WholeSelected, true, false);
			voxel->flag(pcloud::PartSelected, false, false);
			
			ClearFlagVisitor v;
			v.visitNode(voxel);
		}
		void processFilter()
		{
			ClearFlagVisitor v;
			TraverseScene::withVisitor(&v);

			TraverseScene::withFlag(pcloud::PartSelected, false);
			TraverseScene::withFlag(pcloud::WholeSelected, false);
		}
	};

	struct FilterOpHideSelected
	{
		FILTER_IMPL("HideSelected", "Hide Selected", 9, true, false)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			HidePointsVisitor v;
			v.visitNode(voxel);

			DeselectHiddenVisitor v2;
			v2.visitNode(voxel);
		}
		void processFilter()
		{
			HidePointsVisitor v;
			TraverseScene::withVisitor(&v);

			DeselectHiddenVisitor v2;
			TraverseScene::withVisitor(&v2);
		}
	};

	struct FilterOpShowAll
	{
		FILTER_IMPL("ShowAll", "Unhide All", 10, true, false)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			UnhideVisitor v;
			v.visitNode(voxel);
		}
		void processFilter()
		{
			UnhideVisitor v;
			TraverseScene::withVisitor(&v);
		}
	};

	struct FilterOpInvertSelection
	{
		FILTER_IMPL("InvertSel", "Invert Selection", 8, true, false)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			InvertSelectionVisitor v;
			v.visitNode(voxel);
		}
		void processFilter()
		{
			InvertSelectionVisitor v;
			TraverseScene::withVisitor(&v);
		}
	};

	struct FilterOpInvertVisibility
	{
		FILTER_IMPL("InvertVis", "Invert Visibility", 8, true, false)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			InvertVisibilityVisitor v;
			v.visitNode(voxel);

			DeselectHiddenVisitor v2;
			v2.visitNode(voxel);
		}
		void processFilter()
		{
			InvertVisibilityVisitor v;
			TraverseScene::withVisitor(&v);

			DeselectHiddenVisitor v2;
			TraverseScene::withVisitor(&v2);
		}
	};

	struct FilterOpConsolidate
	{
		FILTER_IMPL("Consolidate", "Consolidate State", 8, true, false)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			ConsolidateVisitor v;
			v.visitNode(voxel);
		}
		void processFilter()
		{
			ConsolidateVisitor v;
			TraverseScene::withVisitor(&v);
		}
	};

	struct FilterOpSetLayer
	{
		FILTER_IMPL("SetLayer", "Set Layer", 10, true, false)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			extern ubyte g_visibleLayers;
			if (g_visibleLayers)
			{
				if (voxel->layers(0) & g_visibleLayers) 
				{
					voxel->flag( pcloud::WholeHidden, false, false );
				}
				else if (voxel->layers(1) & g_visibleLayers) 
				{
					voxel->flag( pcloud::PartHidden, true, false );
					
					UpdateLayerVisitor v;
					v.visitNode(voxel);
				}
			}
			else
			{
				/* hidden layer 7 */ 
				if (!(voxel->layers(0) & 0x7e)) 
				{
					voxel->flag( pcloud::WholeHidden, false, false );
				}
				else if (!(voxel->layers(1) & 0x7e)) 
				{
					voxel->flag( pcloud::PartHidden, true, false );
					
					SetHiddenLayerVisitor v;
					v.visitNode(voxel);
				}
			}
		}
		void processFilter()
		{
			if (g_activeLayers)
			{
				UpdateLayerVisitor v;
				TraverseScene::withVisitor(&v);
			}
			else
			{
				SetHiddenLayerVisitor v;
				TraverseScene::withVisitor(&v);
			}
		}
	};

	struct FilterOpCopyToLayer
	{
		FILTER_IMPL("CopyToLayer", "Copy to Layer", 10, true, true)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			extern ubyte g_currentLayer;

			if (voxel->flag(pcloud::WholeSelected))
			{
				voxel->layers(0) |= g_currentLayer;
				voxel->layers(1) &= ~g_currentLayer;
				voxel->flag(pcloud::WholeHidden, false);
				voxel->flag(pcloud::PartHidden, false);
			}
			else if (voxel->flag( pcloud::PartSelected ))
			{
				voxel->layers(1) |= g_currentLayer;
				voxel->flag(pcloud::PartHidden, true);

				CopyToLayerVisitor v;
				v.visitNode(voxel);
			}
			if (g_autoDeselect)
			{
				FilterOpDeselectAll f;
				f.processVoxel(voxel, mode);
			}
		}
		void processFilter()
		{
			CopyToLayerVisitor v;
			TraverseScene::withVisitor(&v);

			if (g_autoDeselect)
			{
				FilterOpDeselectAll f;
				f.processFilter();
			}
			ConsolidateVisitor cv;
			TraverseScene::withVisitor(&cv);
		}
	};

	struct FilterOpMoveToLayer
	{
		FILTER_IMPL("MoveToLayer", "Move to Layer", 10, true, true)

		void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
		{
			extern ubyte g_currentLayer;

			if (voxel->flag(pcloud::WholeSelected))
			{
				voxel->layers(0) |= g_currentLayer;
				voxel->layers(1) &= ~g_currentLayer;

				voxel->flag(pcloud::WholeSelected, false);
				voxel->flag(pcloud::WholeHidden, false);
			}
			else if (voxel->flag( pcloud::PartSelected ))
			{
				voxel->layers(1) |= g_currentLayer;
				voxel->flag(pcloud::PartHidden, true);

				MoveToLayerVisitor v;
				v.visitNode(voxel);

				if (g_autoDeselect)
				{
					FilterOpDeselectAll f;
					f.processVoxel(voxel, mode);
				}
			}
		}
		void processFilter()
		{
			MoveToLayerVisitor v;
			TraverseScene::withVisitor(&v);

			if (g_autoDeselect)
			{
				FilterOpDeselectAll f;
				f.processFilter();
			}
			ConsolidateVisitor cv;
			TraverseScene::withVisitor(&cv);
		}
	};
	} /* detail */ 
} /* pt_edit */ 