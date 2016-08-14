using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Tod.Wrapper.UnitTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Tod.Wrapper.I2DRenderer rdr = null;
            if (Tod.Wrapper.C2DRendererFactory.Create(out rdr, Tod.Wrapper.E2DRenderer.Direct2D))
            {
                rdr.Initialise(new WindowInteropHelper(this).Handle, (uint)this.Width, (uint)this.Height);
            }

            var renderTarget = rdr.CreateRenderTarget(true);
            rdr.SetRenderTarget(renderTarget);

            Task.Factory.StartNew(
                () =>
                {
                    Dispatcher.Invoke(() =>
                        {
                            rdr.BeginDraw();
                            rdr.EndDraw();
                        });
                });
        }
    }
}
