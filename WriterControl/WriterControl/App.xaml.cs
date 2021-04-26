using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media;

namespace WriterControl
{
    /// <summary>
    /// App.xaml 的交互逻辑
    /// </summary>
    public partial class App : Application
    {

        private void CloseButtonClick(object sender, RoutedEventArgs e)
        {
            UIUtils.GetParentObject<Window>((DependencyObject)sender).Close();
        }

        private void MinimizeButtonClick(object sender, RoutedEventArgs e)
        {
            UIUtils.GetParentObject<Window>((DependencyObject)sender).WindowState = WindowState.Minimized;
        }

        private void TitleBarMouseMove(object sender, System.Windows.Input.MouseEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
                UIUtils.GetParentObject<Window>((DependencyObject)sender).DragMove();
        }
    }
}
