/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
using System.Windows;
using ORDBridgeGUI.ViewModel;

namespace ORDBridgeGUI.View
    {
    /// <summary>
    /// Interaction logic for BridgeGUIView.xaml
    /// </summary>
    public partial class BridgeGUIView : Window
        {

        BridgeGUIViewModel ViewModel;

        public BridgeGUIView ()
            {
            InitializeComponent();
            ViewModel = new BridgeGUIViewModel();
            this.DataContext = ViewModel;
            }
        }
    }
