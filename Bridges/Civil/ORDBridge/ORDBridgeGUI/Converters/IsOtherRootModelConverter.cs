using System;
using System.Windows.Data;

namespace ORDBridgeGUI.Converters
    {
    public class IsOtherRootModelConverter : IValueConverter
        {
        public object Convert (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
            {
            //if ( value != null )
            //    return value.Equals("Other");
            //else
            //    return null;

            return value?.Equals("Other");
            }

        public object ConvertBack (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
            {
            throw new NotSupportedException();
            }
        }
    }
