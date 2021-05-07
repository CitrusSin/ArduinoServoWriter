using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace WriterControl
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        private Writer writer;

        private readonly Storyboard errorPopupStoryboard;

        public MainWindow()
        {
            InitializeComponent();
            this.errorPopupStoryboard = (Storyboard)Resources["popupError"];
            errorPopupStoryboard.Completed += ErrorPopupStoryboardCompleted;
        }

        private void ErrorPopupStoryboardCompleted(object sender, EventArgs e)
        {
            errorText.Text = "";
        }

        private void ShowException(Exception e)
        {
            errorText.Text = e.ToString();
            errorPopupStoryboard.Begin();
        }

        private void OutputInfoSafely(string str)
        {
            infoOutputBox.Dispatcher.Invoke(() =>
            {
                infoOutputBox.AppendText(str);
                infoOutputBox.ScrollToEnd();
            });
        }

        private void WindowLoaded(object sender, RoutedEventArgs e)
        {
            comSelect.ItemsSource = SerialPort.GetPortNames();
            char[] allChars = new char[10];
            for (int i=0;i<10;i++)
            {
                char p = (char)(0x30 | i);
                allChars[i] = p;
            }
            characterSelectComboBox.ItemsSource = allChars;
        }

        private void ConnectClick(object sender, RoutedEventArgs e)
        {
            try
            {
                if (writer == null || !writer.IsConnected)
                {
                    string com = comSelect.SelectedItem as string;
                    writer = new Writer(com);
                    writer.ReceiveString += OutputInfoSafely;
                    writer.Connect();
                    connectButton.Content = "断开";
                }
                else
                {
                    writer.Dispose();
                    connectButton.Content = "连接";
                }
            }
            catch (Exception ex)
            {
                ShowException(ex);
            }
        }

        private void SendCharClick(object sender, RoutedEventArgs e)
        {
            writer.Write("C"+characterSelectComboBox.SelectedItem);
        }

        private void ComSelect_MouseEnter(object sender, MouseEventArgs e)
        {
            comSelect.ItemsSource = SerialPort.GetPortNames();
        }
    }
}
