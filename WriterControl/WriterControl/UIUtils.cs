using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace WriterControl
{
    public class UIUtils
    {
        public static T GetParentObject<T>(DependencyObject obj) where T : DependencyObject
        {
            DependencyObject parent = VisualTreeHelper.GetParent(obj);
            while (parent != null)
            {
                if (parent is T)
                {
                    return (T)parent;
                }
                parent = VisualTreeHelper.GetParent(parent);
            }
            return null;
        }
    }
}
