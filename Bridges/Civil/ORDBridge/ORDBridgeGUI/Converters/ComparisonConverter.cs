using System;
using System.Windows.Data;

namespace ORDBridgeGUI.Converters
    {
    public class ComparisonConverter : IValueConverter
        {
        public object Convert (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
            {
            //if ( value != null )
            //    return value.Equals(parameter);
            //else
            //    return null;

            return value?.Equals(parameter);
            }

        public object ConvertBack (object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
            {
            //throw new NotSupportedException();
            return value?.Equals(true) == true ? parameter : Binding.DoNothing;
            }
        }
    }
