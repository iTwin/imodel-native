/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
using System;
using System.Windows;
using System.Windows.Data;

namespace ORDBridgeGUI.Converters
    {
    public class BooleanToVisibilityConverter : IValueConverter
        {
        public object Convert (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
            {
            if ( value.Equals("Other") )
                {
                return Visibility.Visible;
                }
            else
                {
                return Visibility.Collapsed;
                }
            }

        public object ConvertBack (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
            {
            throw new NotSupportedException();
            }
        }
    }
