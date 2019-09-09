using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace ORDBridgeGUI.Converters
    {
    class TextToInt : IValueConverter
        {
        public object Convert (object value, Type targetType, object parameter, CultureInfo culture)
            {
            return value.ToString();
            }

        public object ConvertBack (object value, Type targetType, object parameter, CultureInfo culture)
            {

            if ( string.IsNullOrEmpty((string) value) )
                return -1;

            return int.Parse(value as string);
            }
        }
    }
