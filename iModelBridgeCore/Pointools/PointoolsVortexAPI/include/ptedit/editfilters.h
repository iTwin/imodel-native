#pragma once

#include <ptedit/editNodeDef.h>
#include <ptedit/pointVisitors.h>
#include <ptengine/userChannels.h>

namespace ptedit
{
	struct FilterOpClearAll : public EditNodeDef
	{
		FilterOpClearAll() : EditNodeDef("ClearAll"){}

		DECLARE_EDIT_NODE("ClearAll", "Reset Editing", 6, EditNodeMultithread);

		bool apply()
		{
			ClearFilterVisitor v;
			TraverseScene::withVisitor(&v);

			TraverseScene::withFlag(pcloud::WholeSelected, false);
			TraverseScene::withFlag(pcloud::PartSelected, false);
			TraverseScene::withFlag(pcloud::WholeHidden, false);
			TraverseScene::withFlag(pcloud::PartHidden, false);

			return true;
		}
	};

	struct FilterOpDeselectAll : public EditNodeDef
	{
		FilterOpDeselectAll() : EditNodeDef("DeselectAll"){}

		DECLARE_EDIT_NODE("DeselectAll", "Deselect All", 6, 
			(EditNodeMultithread | EditNodePostConsolidateSel) );

		bool apply()
		{
			DeselectPointsVisitor v;
			TraverseScene::withVisitor(&v);

			return true;						
		}
	};

	struct FilterResetSelection : public EditNodeDef
	{
		FilterResetSelection() : EditNodeDef("ResetSelection"){}

		DECLARE_EDIT_NODE("ResetSelection", "Reset Selection", 6, 
			(EditNodeMultithread | EditNodePostConsolidateSel) );

		bool apply()
		{
			DeselectAnyPointsVisitor v;
			TraverseScene::withVisitor(&v, false);

			return true;						
		}
	};

	struct FilterOpSelectAll : public EditNodeDef
	{
		FilterOpSelectAll() : EditNodeDef("SelectAll") {}

		DECLARE_EDIT_NODE("SelectAll", "Select All", 7, EditNodeMultithread );
		
		bool apply()
		{
			SelectPointsVisitor v;
			TraverseScene::withVisitor(&v);

			return true;
		}
	};

	struct FilterOpHideSelected : public EditNodeDef
	{
		FilterOpHideSelected() : EditNodeDef("HideSelected"){}

		DECLARE_EDIT_NODE("HideSelected", "Hide Selected", 9, (EditNodeMultithread) );

		bool apply()
		{
			HidePointsVisitor v;
			TraverseScene::withVisitor(&v);

			DeselectAnyPointsVisitor v2;
			TraverseScene::withVisitor(&v2);

			return true;
		}
	};

	struct FilterOpShowAll : public EditNodeDef
	{
		FilterOpShowAll() : EditNodeDef("ShowAll"){}

		DECLARE_EDIT_NODE("ShowAll", "Unhide All", 10, EditNodeMultithread );

		bool apply()
		{
			UnhideVisitor v;
			TraverseScene::withVisitor(&v);
			return true;
		}
	};

	struct FilterOpInvertSelection : public EditNodeDef
	{
		FilterOpInvertSelection() : EditNodeDef("InvertSel"){}

		DECLARE_EDIT_NODE("InvertSel", "Invert Selection", 8, EditNodeMultithread );

		bool apply()
		{
			InvertSelectionVisitor v;
			TraverseScene::withVisitor(&v);

			TraverseScene::consolidateFlags();
			return true;
		}
	};
	
	struct FilterOpIsolateSelected : public EditNodeDef
	{
		FilterOpIsolateSelected() : EditNodeDef("IsolateSelected"){}

		DECLARE_EDIT_NODE("IsolateSelected", "Invert Selected", 8, EditNodeMultithread );

		bool apply()
		{
			IsolateSelectedVisitor v;
			TraverseScene::withVisitor(&v);

			DeselectHiddenVisitor v2;
			TraverseScene::withVisitor(&v2);

			return true;
		}
	};

	struct FilterOpInvertVisibility : public EditNodeDef
	{
		FilterOpInvertVisibility() : EditNodeDef("InvertVis"){}

		DECLARE_EDIT_NODE("InvertVis", "Invert Visibility", 8, EditNodeMultithread );

		bool apply()
		{
			InvertVisibilityVisitor v;
			TraverseScene::withVisitor(&v);

			DeselectHiddenVisitor v2;
			TraverseScene::withVisitor(&v2);

			return true;
		}
	};

	struct FilterOpConsolidate : public EditNodeDef
	{
		FilterOpConsolidate() : EditNodeDef("Consolidate"){}

		DECLARE_EDIT_NODE( "Consolidate", "Consolidate State", 8, EditNodeMultithread )

		bool apply()
		{
			ConsolidateVisitor v(false,false);
			TraverseScene::withVisitor(&v);

			return true;
		}
	};

	struct FilterOpConsolidateAllLayers : public EditNodeDef
	{
		FilterOpConsolidateAllLayers() : EditNodeDef("ConsolidateAllLayers"){}

		DECLARE_EDIT_NODE( "ConsolidateAllLayers", "Consolidate State", 8, EditNodeMultithread )

		bool apply()
		{
			ConsolidateVisitor v(true,true);
			TraverseScene::withVisitor(&v);

			// make sure hidden and layer flags propogate
			TraverseScene::consolidateFlags();
			TraverseScene::consolidateLayers();

			return true;
		}
	};

	struct FilterOpSetLayer : public EditNodeDef
	{
		FilterOpSetLayer() : EditNodeDef("SetLayer"){}

		DECLARE_EDIT_NODE( "SetLayer", "Set Layer", 10, EditNodeMultithread )

		bool apply()
		{
			if (g_activeLayers)
			{
				UpdateLayerVisitor v;
				TraverseScene::withVisitor(&v, false); // don't use the scope here, it is possible to unhide a layer in a hidden cloud/scene, the scope prevents this working

				// make sure hidden and layer flags propogate
				TraverseScene::consolidateFlags();
				TraverseScene::consolidateLayers();
			}
			return true;
		}
	};

	struct FilterOpCopyToLayer : public EditNodeDef
	{
		FilterOpCopyToLayer() : EditNodeDef("CopyToLayer"){}

		DECLARE_EDIT_NODE("CopyToLayer", "Copy to Layer", 10, 
			EditNodeMultithread | EditNodePostConsolidateVis )

		bool apply()
		{
			CopyToLayerVisitor v;
			TraverseScene::withVisitor(&v);

			if (g_state.autoDeselect)
			{
				FilterOpDeselectAll f;
				f.apply();
			}
			return true;
		}
	};

	struct FilterOpMoveToLayer : public EditNodeDef
	{
		FilterOpMoveToLayer() : EditNodeDef("MoveToLayer"){}

		DECLARE_EDIT_NODE("MoveToLayer", "Move to Layer", 10, 
			EditNodeMultithread | EditNodePostConsolidateVis )

		bool apply()
		{
			MoveToLayerVisitor v;
			TraverseScene::withVisitor(&v);

			if (g_state.autoDeselect)
			{
				FilterOpDeselectAll f;
				f.apply();
			}
			return true;
		}
	};

	struct FilterOpDeselectLayer : public EditNodeDef
	{
		FilterOpDeselectLayer() : EditNodeDef( "DeselectLayer")
			,targetLyr(0) {}

		DECLARE_EDIT_NODE( "DeselectLayer", "Deselect Layer", 6, 
			(EditNodeMultithread | EditNodePostConsolidateSel) );

		void targetLayer( int layerIndex )	{ targetLyr = layerIndex; }

		bool apply()
		{
			DeselectPointsInLayerVisitor v(targetLyr);
			TraverseScene::withVisitor(&v);

			return true;
		}
		bool writeState( pt::datatree::Branch *b) const 
		{ 
			b->addNode("targetLayer", targetLyr);
			return true; 
		};
		bool readState(const pt::datatree::Branch *b) 
		{
			targetLyr = 0;
			b->getNode("targetLayer", targetLyr);
			return true;
		};
	private:
		int		targetLyr;
	};

	struct FilterOpSelectLayer : public EditNodeDef
	{
		FilterOpSelectLayer() : EditNodeDef("SelectLayer"), targetLyr(0) {}

		DECLARE_EDIT_NODE("SelectLayer", "Select Layer", 7, EditNodeMultithread );
		
		void targetLayer( int layerIndex )	{ targetLyr = layerIndex; }
	
		bool apply()
		{
			SelectPointsInLayerVisitor v(targetLyr);
			TraverseScene::withVisitor(&v);

			return true;
		}
		bool writeState( pt::datatree::Branch *b) const 
		{ 
			b->addNode("targetLayer", targetLyr);
			return true; 
		};
		bool readState(const pt::datatree::Branch *b) 
		{ 
			targetLyr = 0;
			b->getNode("targetLayer", targetLyr);
			return true;
		};
	private:
		int		targetLyr;
	};

	struct FilterOpLayersFromUserChannel : public EditNodeDef
	{
		FilterOpLayersFromUserChannel() : EditNodeDef("LayersFromUserChannel"), m_currentUserChannel(NULL) { }

		DECLARE_EDIT_NODE("LayersFromUserChannel", "Load Layers", 5, EditNodeMultithread );		

		void setUserChannel( pointsengine::UserChannel* userChannel )
		{			
			if (!userChannel)
				return;

			char name[64] = {0};
			sprintf(name, "layerUserChannel%d", s_layerFromChannelCount++);
			pt::String channelName(name);	
			m_currentUserChannel = userChannel; // the channel data will be copied when written to the branch in writeState() below
		}
	
		bool apply()
		{			
			pointsengine::pauseEngine();
			pointsengine::UserChannelManager::instance()->applyChannelToLayers( m_currentUserChannel );
			pointsengine::unpauseEngine();

			return true;
		}
		bool writeState( pt::datatree::Branch *b) const 
		{ 	
			if (m_currentUserChannel)
			{
				if (pt::datatree::Branch *layers = b->addBranch("layersUserChannel"))
				{
					m_currentUserChannel->writeToBranch(layers, true); // makes a copy of the data
				}
			}

			return true; 
		};
		bool readState(const pt::datatree::Branch *b) 
		{ 
			if (pt::datatree::Branch *layers = b->getBranch("layersUserChannel"))
			{
				m_currentUserChannel = pointsengine::UserChannel::createFromBranch(layers);
				return (m_currentUserChannel != NULL);
			}
			return true;
		};	

	private:
		pointsengine::UserChannel* m_currentUserChannel;
		static int s_layerFromChannelCount;
	};

} /* pt_edit */ 