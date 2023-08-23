using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using AForge.Video.DirectShow;
using AForge.Video;
using AForge.Imaging.Filters;
using AForge.Imaging;
using AForge.Math.Geometry;
using AForge;
using Emgu.CV;
using Emgu.CV.CvEnum;
using Emgu.CV.Structure;
using Emgu.CV.Reg;
using Emgu.CV.Dai;
using System.IO.Ports;
using System.Management;
using System.Linq;
using System.Reflection.Emit;
using System.Threading.Tasks;
using Celluveyor;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using log4net.Layout;
using System.Threading;
using System.IO;
using ZXing.SkiaSharp;
using ZXing.Common;
using ZXing.QrCode;
using SkiaSharp;
using ZXing;
using OpenCvSharp;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.ProgressBar;
using System.Runtime.InteropServices;
using static System.Net.Mime.MediaTypeNames;

namespace Celluveyor
{

    public partial class Form1 : Form
    {
        private VideoCaptureDevice videoCapture;
        private FilterInfoCollection filterInfoCollection;
        private SerialPort serialPort;

        string QR_text;

        int connected = 0;
        int State = 0, Mode = 5, XLA = 0, des_ctl = 0;    // State: start=1,stop=0;    Mode: AUTO=0,MANU=1,TEST=2;

        int angle;
        int centerX, centerY;

        sbyte[] Path = { -1, -1, -1, -1 };

        int[,] weights = {
                    {0, 1, 0, 1, 0, 0, 0, 0, 0, 0},
                    {1, 0, 1, 1, 1, 0, 0, 0, 0, 0},
                    {0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
                    {1, 1, 0, 0, 1, 1, 1, 0, 0, 0},
                    {0, 1, 1, 1, 0, 0, 1, 1, 0, 0},
                    {0, 0, 0, 1, 0, 0, 1, 0, 1, 0},
                    {0, 0, 0, 1, 1, 1, 0, 1, 1, 1},
                    {0, 0, 0, 0, 1, 0, 1, 0, 0, 1},
                    {0, 0, 0, 0, 0, 1, 1, 0, 0, 1},
                    {0, 0, 0, 0, 0, 0, 1, 1, 1, 0}};
        int[,] matranVT = {
                       {-1, -1},
                       {0, 0},
                       {0, 2},
                       {0, 4},
                       {1, 1},
                       {1, 3},
                       {2, 0},
                       {2, 2},
                       {2, 4},
                       {3, 1},
                       {3, 3}};

        int VT_HOP = 0;
        int VT_CUOI = 0;

        int[,] TOADO_TAM =
                            {
                                    {93,386 },
                                    {198,386 },
                                    {303,386 },
                                    {143,293 },
                                    {248 ,293},
                                    {93,193 },
                                    {198,193},
                                    {303,193},
                                    {143,100 },
                                    {248,100 },
                            };//x=120;y=100

        int[,] Thucnghiem = new int[100000, 2];  // = {} ;
        int a = 1;

        private Random random;
        public Form1()
        {
            InitializeComponent();

            random = new Random();
        }

        public static event Action<int> StateChanged;
        private void Form1_Load(object sender, EventArgs e)
        {
            StateChanged += OnStateChanged;

            filterInfoCollection = new FilterInfoCollection(FilterCategory.VideoInputDevice);
            videoCapture = new VideoCaptureDevice(filterInfoCollection[1].MonikerString);
            videoCapture.VideoResolution = videoCapture.VideoCapabilities[0];
            videoCapture.NewFrame += new AForge.Video.NewFrameEventHandler(videoDevice_NewFrame);
            videoCapture.Start();
        }

        private void HT()
        {
            int k;
            for (k = 0; k < 10; k++)
            {
                if (comboBox1.SelectedIndex == k)
                {
                    if (k == 0) button00.BackColor = Color.DeepSkyBlue;
                    if (k == 1) button02.BackColor = Color.DeepSkyBlue;
                    if (k == 2) button04.BackColor = Color.DeepSkyBlue;
                    if (k == 3) button11.BackColor = Color.DeepSkyBlue;
                    if (k == 4) button13.BackColor = Color.DeepSkyBlue;
                    if (k == 5) button20.BackColor = Color.DeepSkyBlue;
                    if (k == 6) button22.BackColor = Color.DeepSkyBlue;
                    if (k == 7) button24.BackColor = Color.DeepSkyBlue;
                    if (k == 8) button31.BackColor = Color.DeepSkyBlue;
                    if (k == 9) button33.BackColor = Color.DeepSkyBlue;
                }
                if (comboBox2.SelectedIndex == k)
                {
                    if (k == 0) button00.BackColor = Color.DeepSkyBlue;
                    if (k == 1) button02.BackColor = Color.DeepSkyBlue;
                    if (k == 2) button04.BackColor = Color.DeepSkyBlue;
                    if (k == 3) button11.BackColor = Color.DeepSkyBlue;
                    if (k == 4) button13.BackColor = Color.DeepSkyBlue;
                    if (k == 5) button20.BackColor = Color.DeepSkyBlue;
                    if (k == 6) button22.BackColor = Color.DeepSkyBlue;
                    if (k == 7) button24.BackColor = Color.DeepSkyBlue;
                    if (k == 8) button31.BackColor = Color.DeepSkyBlue;
                    if (k == 9) button33.BackColor = Color.DeepSkyBlue;
                }
            }
        }


        private void button00_Click(object sender, EventArgs e)
        {
            if (Mode == 2)
            {
                if (comboBox1.Text == "")
                {
                    XOANUTNHAN();
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 0;
                }
                else if (comboBox2.Text == "")
                {
                    comboBox2.SelectedIndex = -1;
                    comboBox2.SelectedIndex = 0;
                }
                else if (comboBox1.Text != "" && comboBox2.Text != "")
                {
                    XOANUTNHAN();
                    comboBox2.Text = "";
                    comboBox1.Text = "";
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 0;

                }
                if (button00.BackColor == Color.White) button00.BackColor = Color.DeepSkyBlue; else button00.BackColor = Color.White;
            }
        }

        private void button02_Click(object sender, EventArgs e)
        {
            if (Mode == 2)
            {
                if (comboBox1.Text == "")
                {
                    XOANUTNHAN();
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 1;
                }
                else if (comboBox2.Text == "")
                {
                    comboBox2.SelectedIndex = -1;
                    comboBox2.SelectedIndex = 1;
                }
                else if (comboBox1.Text != "" && comboBox2.Text != "")
                {
                    XOANUTNHAN();
                    comboBox2.Text = "";
                    comboBox1.Text = "";
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 1;
                }
                if (button02.BackColor == Color.White) button02.BackColor = Color.DeepSkyBlue; else button02.BackColor = Color.White;
            }
        }

        private void button04_Click(object sender, EventArgs e)
        {
            if (Mode == 2)
            {
                if (comboBox1.Text == "")
                {
                    XOANUTNHAN();
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 2;
                }
                else if (comboBox2.Text == "")
                {
                    comboBox2.SelectedIndex = -1;
                    comboBox2.SelectedIndex = 2;
                }
                else if (comboBox1.Text != "" && comboBox2.Text != "")
                {
                    XOANUTNHAN();
                    comboBox2.Text = "";
                    comboBox1.Text = "";
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 2;
                }
                if (button04.BackColor == Color.White) button04.BackColor = Color.DeepSkyBlue; else button04.BackColor = Color.White;
            }
        }

        private void button11_Click(object sender, EventArgs e)
        {
            if (Mode == 2)
            {
                if (comboBox1.Text == "")
                {
                    XOANUTNHAN();
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 3;
                }
                else if (comboBox2.Text == "")
                {
                    comboBox2.SelectedIndex = -1;
                    comboBox2.SelectedIndex = 3;
                }
                else if (comboBox1.Text != "" && comboBox2.Text != "")
                {
                    XOANUTNHAN();
                    comboBox2.Text = "";
                    comboBox1.Text = "";
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 3;
                }
                if (button11.BackColor == Color.White) button11.BackColor = Color.DeepSkyBlue; else button11.BackColor = Color.White;
            }
        }

        private void button13_Click(object sender, EventArgs e)
        {
            if (Mode == 2)
            {
                if (comboBox1.Text == "")
                {
                    XOANUTNHAN();
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 4;
                }
                else if (comboBox2.Text == "")
                {
                    comboBox2.SelectedIndex = -1;
                    comboBox2.SelectedIndex = 4;
                }
                else if (comboBox1.Text != "" && comboBox2.Text != "")
                {
                    XOANUTNHAN();
                    comboBox2.Text = "";
                    comboBox1.Text = "";
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 4;
                }
                if (button13.BackColor == Color.White) button13.BackColor = Color.DeepSkyBlue; else button13.BackColor = Color.White;
            }
        }

        private void button20_Click(object sender, EventArgs e)
        {
            if (Mode == 2)
            {
                if (comboBox1.Text == "")
                {
                    XOANUTNHAN();
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 5;
                }
                else if (comboBox2.Text == "")
                {
                    comboBox2.SelectedIndex = -1;
                    comboBox2.SelectedIndex = 5;
                }
                else if (comboBox1.Text != "" && comboBox2.Text != "")
                {
                    XOANUTNHAN();
                    comboBox2.Text = "";
                    comboBox1.Text = "";
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 5;
                }
                if (button20.BackColor == Color.White) button20.BackColor = Color.DeepSkyBlue; else button20.BackColor = Color.White;
            }
        }

        private void button22_Click(object sender, EventArgs e)
        {
            if (Mode == 2)
            {
                if (comboBox1.Text == "")
                {
                    XOANUTNHAN();
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 6;
                }
                else if (comboBox2.Text == "")
                {
                    comboBox2.SelectedIndex = -1;
                    comboBox2.SelectedIndex = 6;
                }
                else if (comboBox1.Text != "" && comboBox2.Text != "")
                {
                    XOANUTNHAN();
                    comboBox2.Text = "";
                    comboBox1.Text = "";
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 6;
                }
                if (button22.BackColor == Color.White) button22.BackColor = Color.DeepSkyBlue; else button22.BackColor = Color.White;
            }
        }

        private void button24_Click(object sender, EventArgs e)
        {
            if (Mode == 2)
            {
                if (comboBox1.Text == "")
                {
                    XOANUTNHAN();
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 7;
                }
                else if (comboBox2.Text == "")
                {
                    comboBox2.SelectedIndex = -1;
                    comboBox2.SelectedIndex = 7;
                }
                else if (comboBox1.Text != "" && comboBox2.Text != "")
                {
                    XOANUTNHAN();
                    comboBox2.Text = "";
                    comboBox1.Text = "";
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 7;
                }
                if (button24.BackColor == Color.White) button24.BackColor = Color.DeepSkyBlue; else button24.BackColor = Color.White;
            }
        }

        private void button31_Click(object sender, EventArgs e)
        {
            if (Mode == 2)
            {
                if (comboBox1.Text == "")
                {
                    XOANUTNHAN();
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 8;
                }
                else if (comboBox2.Text == "")
                {
                    comboBox2.SelectedIndex = -1;
                    comboBox2.SelectedIndex = 8;
                }
                else if (comboBox1.Text != "" && comboBox2.Text != "")
                {
                    XOANUTNHAN();
                    comboBox2.Text = "";
                    comboBox1.Text = "";
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 8;
                }
                if (button31.BackColor == Color.White) button31.BackColor = Color.DeepSkyBlue; else button31.BackColor = Color.White;
            }
        }

        private void button33_Click(object sender, EventArgs e)
        {
            if (Mode == 2)
            {
                if (comboBox1.Text == "")
                {
                    XOANUTNHAN();
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 9;
                }
                else if (comboBox2.Text == "")
                {
                    comboBox2.SelectedIndex = -1;
                    comboBox2.SelectedIndex = 9;
                }
                else if (comboBox1.Text != "" && comboBox2.Text != "")
                {
                    XOANUTNHAN();
                    comboBox2.Text = "";
                    comboBox1.Text = "";
                    comboBox1.SelectedIndex = -1;
                    comboBox1.SelectedIndex = 9;
                }
                if (button33.BackColor == Color.White) button33.BackColor = Color.DeepSkyBlue; else button33.BackColor = Color.White;
            }
        }


        //CHUONG TRINH XOA NUT NHAN
        private void XOANUTNHAN()
        {
            button00.BackColor = Color.White;
            button02.BackColor = Color.White;
            button04.BackColor = Color.White;
            button11.BackColor = Color.White;
            button13.BackColor = Color.White;
            button31.BackColor = Color.White;
            button33.BackColor = Color.White;
            button20.BackColor = Color.White;
            button22.BackColor = Color.White;
            button24.BackColor = Color.White;
        }
        private void CT_TEST()
        {
            button00.BackColor = Color.DeepSkyBlue;
            button02.BackColor = Color.DeepSkyBlue;
            button04.BackColor = Color.DeepSkyBlue;
            button11.BackColor = Color.DeepSkyBlue;
            button13.BackColor = Color.DeepSkyBlue;
            button31.BackColor = Color.DeepSkyBlue;
            button33.BackColor = Color.DeepSkyBlue;
            button20.BackColor = Color.DeepSkyBlue;
            button22.BackColor = Color.DeepSkyBlue;
            button24.BackColor = Color.DeepSkyBlue;
        }

        private int MinimumDistance(int[] distance, bool[] shortestPathTreeSet)
        {
            int min = int.MaxValue;
            int minIndex = -1;

            for (int v = 0; v < distance.Length; v++)
            {
                if (shortestPathTreeSet[v] == false && distance[v] <= min)
                {
                    min = distance[v];
                    minIndex = v;
                }
            }

            return minIndex;
        }
        public List<int> FindShortestPath(int[,] weights, int source, int destination)
        {
            int verticesCount = weights.GetLength(0);
            int[] distance = new int[verticesCount];
            bool[] shortestPathTreeSet = new bool[verticesCount];
            List<int>[] shortestPaths = new List<int>[verticesCount];

            for (int i = 0; i < verticesCount; ++i)
            {
                distance[i] = int.MaxValue;
                shortestPathTreeSet[i] = false;
                shortestPaths[i] = new List<int>();
            }

            distance[source] = 0;
            shortestPaths[source].Add(source);

            for (int count = 0; count < verticesCount - 1; ++count)
            {
                int u = MinimumDistance(distance, shortestPathTreeSet);
                shortestPathTreeSet[u] = true;

                for (int v = 0; v < verticesCount; ++v)
                {
                    if (!shortestPathTreeSet[v] && weights[u, v] != 0 && distance[u] != int.MaxValue
                        && distance[u] + weights[u, v] < distance[v])
                    {
                        distance[v] = distance[u] + weights[u, v];
                        shortestPaths[v] = new List<int>(shortestPaths[u]);
                        shortestPaths[v].Add(v);
                    }
                }
            }

            return shortestPaths[destination];
        }



        private void Send_UART()
        {
            /*TT - Mode - angle angle - X X - Y Y - Path[0] - Path[1] - Path[2]- Path[3] (Mode = 1, 2)*/

            /*TT - Mode - angle angle - X X - Y Y - Path[0] - Path[1] - Path[2]- Path[3] (Mode = 3)*/

            byte[] data = new byte[12];
            byte[] buffer;

            buffer = BitConverter.GetBytes(State);
            Array.Copy(buffer, 0, data, 0, 1);
            buffer = BitConverter.GetBytes(Mode);
            Array.Copy(buffer, 0, data, 1, 1);

            buffer = BitConverter.GetBytes(angle);
            Array.Copy(buffer, 0, data, 2, 2);
            
            if(Mode == 3)
            {
                buffer = BitConverter.GetBytes(des_ctl);
                Array.Copy(buffer, 0, data, 4, 2);
            }
            else
            {
                buffer = BitConverter.GetBytes(matranVT[VT_HOP, 0]);
                Array.Copy(buffer, 0, data, 4, 2);
            }
            

            buffer = BitConverter.GetBytes(matranVT[VT_HOP, 1]);
            Array.Copy(buffer, 0, data, 6, 2);

            for (int i = 0; i < 4; i++)
            {
                buffer = BitConverter.GetBytes(Path[i]);
                Array.Copy(buffer, 0, data, 8 + i, 1);
            }


            if (serialPort != null && serialPort.IsOpen)
            {
                try
                {
                    serialPort.Write(data, 0, data.Length); // Gửi dữ liệu
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error: " + ex.Message);

                }
            }
            else
            {
                MessageBox.Show("Vui lòng kết nối với cổng giao tiếp UART");
                connected = 0;
                UART_Connect.BackColor = Color.White;
                UART_Connect.Text = "Connect";
                groupBox1.Text = " Disconnect";
                serialPort.Close();
            }
        }

        private void OnStateChanged(int newState) // Sự kiện khi biến State thay đổi
        {
            Send_UART();
        }

        private void STOP_Click(object sender, EventArgs e)
        {
            if (State != 0)
            {
                State = 0;
                StateChanged?.Invoke(State);
                comboBox1.Text = "";
                comboBox2.Text = "";
                XOANUTNHAN();
                STOP.BackColor = Color.OrangeRed;
                START.BackColor = Color.White;
            }

        }

        private async void START_Click(object sender, EventArgs e)
        {
            if (serialPort != null && serialPort.IsOpen)
            {
                if (State != 1)
                {
                    if (Mode <= 0 || Mode > 3) MessageBox.Show("Vui lòng chọn chế độ!", "Thông báo");
                    else
                    {
                        /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- ........CHế độ AUTO...... -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
                        if (Mode == 1)
                        {
                            if (QR_text != null)
                            {
                                int source = VT_HOP - 1;
                                int destination = -1;
                                if (QR_text == "Loại 1")
                                {
                                    destination = 8;
                                }
                                else if (QR_text == "Loại 2")
                                {
                                    destination = 7;
                                }
                                else if (QR_text == "Loại 3")
                                {
                                    destination = 5;
                                }
                                if (source >= 0)
                                {
                                    List<int> shortestPath = FindShortestPath(weights, source, destination);

                                    if (shortestPath.Count > 0)
                                    {
                                        Console.WriteLine("Shortest Path from {0} to {1}:", source, destination);
                                        label3.Text = "";
                                        for (int i = 0; i < 4; i++)
                                        {
                                            Path[i] = -1;
                                        }
                                        int _index = 0;
                                        foreach (int vertex in shortestPath)
                                        {
                                            Path[_index] = (sbyte)(vertex + 1);
                                            _index++;
                                            label3.Text += string.Format(" - {0}", comboBox1.Items[vertex]);
                                        }
                                    }
                                }

                            }

                            State = 1;
                            StateChanged?.Invoke(State);
                            STOP.BackColor = Color.White;
                            START.BackColor = Color.Green;

                            //  Gui du lieu dieu khien
                            Send_UART();
                            await Task.Delay(100);
                            for (int i = 0; i < 4; i++)
                            {
                                Path[i] = -1;
                            }
                            do
                            {
                                Send_UART();                               
                                await Task.Delay(10);
                            } while (State == 1);
                            

                        }

                        /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- .......CHế độ MANU......... -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
                        if (Mode == 2)
                        {

                            if (comboBox1.Text == "" && comboBox2.Text == "") MessageBox.Show("Vui lòng chọn điểm xuất phát và điểm kết thúc!", "Thông báo");
                            else if (comboBox1.Text != "" && comboBox2.Text == "") MessageBox.Show("Vui lòng chọn điểm kết thúc!", "Thông báo");
                            else if (comboBox1.Text == "" && comboBox2.Text != "") MessageBox.Show("Vui lòng chọn điểm xuất phát ", "Thông báo");
                            else if (comboBox1.Text == comboBox2.Text) MessageBox.Show("Điểm xuất phát và kết thúc không được trùng nhau ", "Thông báo");
                            else
                            {

                                int source = comboBox1.SelectedIndex; // Đỉnh nguồn
                                int destination = comboBox2.SelectedIndex; // Đỉnh đích
                                int index = 0;
                                for(int i = 0; i < Thucnghiem.GetLength(0); i++)
                                {
                                    Thucnghiem[i, 0] = 0;
                                    Thucnghiem[i, 1] = 0;
                                }    

                                List<int> shortestPath = FindShortestPath(weights, source, destination);

                                if (shortestPath.Count > 0)
                                {
                                    Console.WriteLine("Shortest Path from {0} to {1}:", source, destination);
                                    label3.Text = "";
                                    for (int i = 0; i < 4; i++)
                                    {
                                        Path[i] = -1;
                                    }
                                    int _index = 0;
                                    foreach (int vertex in shortestPath)
                                    {
                                        Path[_index] = (sbyte)(vertex + 1);
                                        _index++;
                                        label3.Text += string.Format("    {0}", comboBox1.Items[vertex]);
                                    }
                                }
                                
                                State = 1;
                                StateChanged?.Invoke(State);
                                STOP.BackColor = Color.White;
                                START.BackColor = Color.Green;
                                HT();

                                //  Gui du lieu dieu khien
                                Send_UART();
                                await Task.Delay(100);
                                for (int i = 0; i < 4; i++)
                                {
                                    Path[i] = -1;
                                }
                                do
                                {
                                    Send_UART();
                                    if (centerX != null && centerY != null)
                                    {
                                        if (centerX > 10 && centerY > 10)
                                        {
                                            Thucnghiem[index, 0] = centerX;
                                            Thucnghiem[index, 1] = centerY;
                                            index++;
                                        }
                                    }
                                    await Task.Delay(10 );

                                } while (State == 1);
                            }

                        }

                        /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- ...........CHế độ TEST.......... -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
                        if (Mode == 3)
                        {

                            State = 1;
                            StateChanged?.Invoke(State);
                            STOP.BackColor = Color.White;
                            START.BackColor = Color.Green;
                            CT_TEST();
                            Send_UART();
                        }
                    }
                }
            }
            else
            {
                MessageBox.Show("Vui lòng kết nối với cổng giao tiếp UART");
                connected = 0;
                UART_Connect.BackColor = Color.White;
                UART_Connect.Text = "Connect";
                groupBox1.Text = " Disconnect";
            }
        
        }
        private void TEST_Click(object sender, EventArgs e)
        {
            if (State == 0)
            {
                Mode = 3;
                AUTO.BackColor = Color.White;
                MANU.BackColor = Color.White;
                TEST.BackColor = Color.DeepSkyBlue;
                XOANUTNHAN();
                comboBox1.Text = "";
                comboBox2.Text = "";
                label3.Text = "";
                groupBox3.Enabled = true;
                groupBox4.Enabled = false;
            }
            else
            {
                if (Mode == 1) MessageBox.Show("Vui lòng tắt chế độ AUTO!", "Thông báo");
                if (Mode == 2) MessageBox.Show("Vui lòng tắt chế độ MANU!", "Thông báo");
                if (Mode == 3) MessageBox.Show("Chế độ TEST đang chạy!", "Thông báo");
            }
        }

        private void AUTO_Click(object sender, EventArgs e)
        {
            if (State == 0)       //start=1; stop=0;
            {
                Mode = 1;
                AUTO.BackColor = Color.DeepSkyBlue;
                MANU.BackColor = Color.White;
                TEST.BackColor = Color.White;
                XOANUTNHAN();
                comboBox1.Text = "";
                comboBox2.Text = "";
                label3.Text = "";
                groupBox3.Enabled = false;
                groupBox4.Enabled = false;
            }
            else
            {
                if (Mode == 1) MessageBox.Show("Chế độ AUTO đang chạy!", "Thông báo");
                if (Mode == 2) MessageBox.Show("Vui lòng tắt chế độ MANU!", "Thông báo");
                if (Mode == 3) MessageBox.Show("Vui lòng tắt chế độ TEST!", "Thông báo");
            }
        }



        private void MANU_Click(object sender, EventArgs e)
        {
            if (State == 0)
            {
                Mode = 2;
                AUTO.BackColor = Color.White;
                MANU.BackColor = Color.DeepSkyBlue;
                TEST.BackColor = Color.White;
                groupBox3.Enabled = false;
                groupBox4.Enabled = true;
            }
            else
            {
                if (Mode == 1) MessageBox.Show("Vui lòng tắt chế độ AUTO!", "Thông báo");
                if (Mode == 2) MessageBox.Show("Chế độ MANU đang chạy!", "Thông báo");
                if (Mode == 3) MessageBox.Show("Vui lòng tắt chế độ TEST!", "Thông báo");
            }
        }



        private async void videoDevice_NewFrame(object sender, AForge.Video.NewFrameEventArgs eventArgs)
        {
            Bitmap capturedFrame = eventArgs.Frame;

            // Clone the captured frame to avoid conflicts between threads
            Bitmap clonedFrame;
            lock (capturedFrame)
            {
                clonedFrame = (Bitmap)capturedFrame.Clone();
            }

            // Crop the cloned frame
            Rectangle cropRectangle = new Rectangle(125, 410, 915, 1055);
            Bitmap croppedFrame = CropImage(clonedFrame, cropRectangle);

            // Resize the cropped frame to fit the PictureBox size
            Bitmap resizedFrame = ResizeImage(croppedFrame, pictureBox2.Width, pictureBox2.Height);

            // Perform image processing operations asynchronously


            Bitmap processedFrame1 = await Task.Run(() => ProcessQR(resizedFrame));
            Bitmap processedFrame2 = await Task.Run(() => ProcessImage(processedFrame1));


            // Display the processed frame in the PictureBox
            UpdatePictureBox(processedFrame2);
        }

        // Các phương thức CropImage, ResizeImage và ProcessImage giống như phiên bản trước

        private Bitmap CropImage(Bitmap source, Rectangle cropRectangle)
        {
            Bitmap croppedImage = new Bitmap(cropRectangle.Width, cropRectangle.Height);
            using (Graphics graphics = Graphics.FromImage(croppedImage))
            {
                graphics.DrawImage(source, new Rectangle(0, 0, croppedImage.Width, croppedImage.Height),
                    cropRectangle, GraphicsUnit.Pixel);
            }
            return croppedImage;
        }

        private Bitmap ResizeImage(Bitmap source, int width, int height)
        {
            Bitmap resizedImage = new Bitmap(width, height);
            using (Graphics graphics = Graphics.FromImage(resizedImage))
            {
                graphics.DrawImage(source, 0, 0, width, height);
            }
            return resizedImage;
        }

        private SKBitmap ConvertToSKBitmap(Bitmap bitmap)
        {
            // Convert a System.Drawing.Bitmap to a SKBitmap
            using (MemoryStream stream = new MemoryStream())
            {
                bitmap.Save(stream, System.Drawing.Imaging.ImageFormat.Png);
                stream.Position = 0;
                return SKBitmap.Decode(stream);
            }
        }

        private Bitmap ProcessImage(Bitmap source)
        {
            // Convert the frame to grayscale
            Grayscale grayFilter = new Grayscale(0.2125, 0.7154, 0.0721);
            Bitmap gray = grayFilter.Apply(source);


            // Apply Gaussian blur to reduce noise
            GaussianBlur blur = new GaussianBlur(7, 7);
            Bitmap blurred = blur.Apply(gray);

            // Apply morphological operations to enhance edges
            Opening opening = new Opening();
            Bitmap opened = opening.Apply(blurred);

            // Detect edges using Canny algorithm
            CannyEdgeDetector edgeDetector = new CannyEdgeDetector(20, 100);
            Bitmap edges = edgeDetector.Apply(opened);

            // Apply dilation and erosion to expand and shrink edges
            Dilatation dilateFilter = new Dilatation();
            Erosion erodeFilter = new Erosion();
            Bitmap dilatedEdges = dilateFilter.Apply(edges);
            Bitmap erodedEdges = erodeFilter.Apply(dilatedEdges);

            // Find contours in the image
            BlobCounter blobCounter = new BlobCounter();
            blobCounter.FilterBlobs = true;
            blobCounter.MinHeight = 80;
            blobCounter.MinWidth = 80;
            blobCounter.MaxWidth = 250;
            blobCounter.MaxHeight = 250;
            blobCounter.ProcessImage(erodedEdges);

            // Get information about the blobs
            Blob[] blobs = blobCounter.GetObjectsInformation();

            // Draw bounding boxes and other information on the image
            using (Graphics graphics = Graphics.FromImage(source))
            {
                SimpleShapeChecker shapeChecker = new SimpleShapeChecker();
                Pen pen = new Pen(Color.Green, 2);
                SolidBrush brush = new SolidBrush(Color.FromArgb(128, Color.Red));

                int counter = 0;
                angle = -1;
                centerX = -1;
                centerY = -1;
                VT_HOP = 0;
                foreach (Blob blob in blobs)
                {
                    List<IntPoint> edgePoints = blobCounter.GetBlobsEdgePoints(blob);
                    List<IntPoint> corners = PointsCloud.FindQuadrilateralCorners(edgePoints);
                    PointF[] cornerPoints = corners.ConvertAll(point => new PointF(point.X, point.Y)).ToArray();

                    if (shapeChecker.IsQuadrilateral(corners) && blob.Area >= 100)
                    {
                        graphics.DrawPolygon(pen, cornerPoints);

                        Rectangle rect = blob.Rectangle;
                        AForge.Point center = new AForge.Point(rect.X + rect.Width / 2, rect.Y + rect.Height / 2);
                        graphics.FillEllipse(brush, center.X - 3, center.Y - 3, 6, 6);

                        float scale = 1;
                        float length = rect.Height * scale;
                        float widthCm = rect.Width * scale;

                        
                        counter++;

                        int _angle = 0;
                        double _length = Math.Sqrt(Math.Pow((corners[0].X - corners[1].X), 2) + Math.Pow((corners[0].Y - corners[1].Y), 2));
                        double _width = Math.Sqrt(Math.Pow((corners[2].X - corners[1].X), 2) + Math.Pow((corners[2].Y - corners[1].Y), 2));

                        if (_length > _width)
                            _angle = (int)(Math.Atan2((corners[0].Y - corners[1].Y), (corners[0].X - corners[1].X)) * 180 / Math.PI);
                        else
                            _angle = (int)(Math.Atan2((corners[1].Y - corners[2].Y), (corners[1].X - corners[2].X)) * 180 / Math.PI);

                        if (_angle > 90) angle = 180 - _angle;
                        else if (_angle < -90) angle = _angle + 180;
                        else angle = _angle;

                        centerX = (int)center.X;
                        centerY = (int)center.Y;
                        for (int i = 0; i < 10; i++)
                        {
                            if (centerX < (TOADO_TAM[i, 0] + 15) && centerX > (TOADO_TAM[i, 0] - 15) && centerY < (TOADO_TAM[i, 1] + 15) && centerY > (TOADO_TAM[i, 1] - 15))
                                VT_HOP = i + 1;
                        }                       
                        graphics.DrawString("Angle: " + angle.ToString("F2") + "do", new Font("Arial", 8), new SolidBrush(Color.Blue), corners[0].X, corners[0].Y + 15);
                        graphics.DrawString("VT:  " + VT_HOP.ToString("F2"), new Font("Arial", 8), new SolidBrush(Color.Blue), corners[0].X, corners[0].Y);
                        /*graphics.DrawString("CenterX : " + centerX.ToString("F2") + "mm", new Font("Arial", 8), new SolidBrush(Color.Blue), center.X, center.Y + 30);
                        graphics.DrawString("CenterY : " + centerY.ToString("F2") + "mm", new Font("Arial", 8), new SolidBrush(Color.Blue), center.X, center.Y + 40);*/
                    }
                }

                graphics.DrawString("So HOP: " + counter.ToString(), new Font("Arial", 10), new SolidBrush(Color.Red), 10, 10);
            }

            return source;
        }

        private Bitmap ProcessQR(Bitmap source)
        {
            SKBitmap skBitmap = ConvertToSKBitmap(source);


            // Convert the ZXing bitmap to a luminance source
            SKBitmapLuminanceSource luminanceSource = new SKBitmapLuminanceSource(skBitmap);

            // Create a barcode reader instance
            ZXing.SkiaSharp.BarcodeReader barcodeReader = new ZXing.SkiaSharp.BarcodeReader();

            // Set decoding options (if needed)
            barcodeReader.Options = new DecodingOptions
            {
                // Set decoding options here (e.g., barcode formats)
            };

            // Decode the barcode from the source image
            Result result = barcodeReader.Decode(luminanceSource);

            // Check if a barcode was successfully decoded
            if (result != null)
            {
                // Display the result or perform further processing
                string qrText = result.Text;
                QR_text = qrText;
                // MessageBox.Show(qrText, "QR Code Result");

                Bitmap processedImage = new Bitmap(source);

                // Draw a border around the QR code
                using (Graphics graphics = Graphics.FromImage(processedImage))
                {
                    Font textFont = new Font("Arial", 8, FontStyle.Bold);
                    Brush textBrush = new SolidBrush(Color.Red);

                    // Get the corner points of the QR code
                    ResultPoint[] points = result.ResultPoints;
                    PointF[] qrPoints = Array.ConvertAll(points, p => new PointF(p.X, p.Y));

                    // Draw the text above the bounding box
                    int textX = (int)qrPoints[0].X;
                    int textY = (int)qrPoints[0].Y - 10;
                    graphics.DrawString(qrText, textFont, textBrush, textX, textY);
                }

                return processedImage;
            }

            // Return the original source image if no barcode was found
            return source;
        }


        private void UpdatePictureBox(Bitmap image)
        {
            if (pictureBox2.InvokeRequired)
            {
                pictureBox2.Invoke(new Action<Bitmap>(UpdatePictureBox), image);
            }
            else
            {
                pictureBox2.Image = image;
            }
        }


        private static int ComparePortNames(string x, string y)
        {
            int xNumber = int.Parse(x.Replace("COM", ""));
            int yNumber = int.Parse(y.Replace("COM", ""));
            return xNumber.CompareTo(yNumber);
        }

        private void comboBox3_DropDown(object sender, EventArgs e)
        {
            comboBox3.Items.Clear();
            string[] portNames = SerialPort.GetPortNames();
            Array.Sort(portNames, ComparePortNames);

            // Thêm các cổng COM vào comboBox
            comboBox3.Items.AddRange(portNames);
        }



        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {

            if (videoCapture != null)
            {
               videoCapture.Stop();
               pictureBox2.Image.Dispose();
                
            }
        }
        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {

            if (videoCapture != null)
            {
                videoCapture.Stop();
                pictureBox2.Image.Dispose();

            }
        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            comboBox6.Enabled = true;
            comboBox5.Enabled = false;
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            comboBox6.Enabled = false;
            comboBox5.Enabled = true;
        }


        private void comboBox6_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                des_ctl = 1;
                if (comboBox6.Text != null)
                {
                    angle = Convert.ToInt32(comboBox6.Text);
                    Send_UART();
                }

            }
        }

        private void comboBox6_SelectedIndexChanged(object sender, EventArgs e)
        {
            des_ctl = 1;
            if (comboBox6.Text != null)
            {
                angle = Convert.ToInt32(comboBox6.Text);
                Send_UART();
            }

        }

        private void pictureBox3_Click(object sender, EventArgs e)
        {
            Bitmap image = new Bitmap("C:\\Users\\PDP\\OneDrive - hcmute.edu.vn\\DATN\\CODE\\CODE_FULL\\Celluveyor\\Resources\\Thucnghiem.png");
            pictureBox_Paint(image);

            image.Save("C:\\Users\\PDP\\OneDrive - hcmute.edu.vn\\DATN\\CODE\\CODE_FULL\\Celluveyor\\Resources\\Thucnghiem"+a+".png");
            a++;
        }

        private void pictureBox_Paint(Bitmap image)
        {
            using (Graphics graphics = Graphics.FromImage(image))
            {
                Graphics g = graphics;
                Pen pen = new Pen(Color.Red, 2); // Màu và độ dày của đường thẳng

                // Vẽ các đoạn thẳng nối các điểm
                for (int i = 1; i < Thucnghiem.GetLength(0); i++)
                {
                    if (Thucnghiem[i, 0] > 10 && Thucnghiem[i, 1] > 10)
                    {
                        int startX = Thucnghiem[i - 1, 0];
                        int startY = Thucnghiem[i - 1, 1];
                        int endX = Thucnghiem[i, 0];
                        int endY = Thucnghiem[i, 1];
                        g.DrawLine(pen, startX, startY, endX, endY);
                    }
                }
            }
        }

        private void pictureBox4_Click(object sender, EventArgs e)
        {

        }

        private void comboBox5_KeyDown(object sender, KeyEventArgs e)
    {
        if (e.KeyCode == Keys.Enter)
        {
            des_ctl = 2;
            if (comboBox5.Text != null)
            {
                if (Convert.ToInt32(comboBox5.Text) > 170)
                {
                    MessageBox.Show("Tốc độ động cơ NHỎ HƠN 170 RPM", "Thông báo");
                }
                else
                {
                    angle = Convert.ToInt32(comboBox5.Text);
                    Send_UART();
                }
            }
        }
    }
    private void comboBox5_SelectedIndexChanged(object sender, EventArgs e)
    {
        des_ctl = 2;
        if (comboBox5.Text != null)
        {
            if (Convert.ToInt32(comboBox5.Text) > 170)
            {
                MessageBox.Show("Tốc độ động cơ NHỎ HƠN 170 RPM", "Thông báo");
            }
            else
            {
                angle = Convert.ToInt32(comboBox5.Text);
                Send_UART();
            }
            }
        }

        private void comboBox4_DropDown(object sender, EventArgs e)
        {
            comboBox4.Items.Clear();
            string[] baud = {"600", "1200", "2400", "4800", "9600", "28800",
                              "57600", "76800", "115200", "230400", "460800"};
            comboBox4.Items.AddRange(baud);
        }



        private void UART_Connect_Click(object sender, EventArgs e)
        {
            if (connected == 0)
            {
                try
                {
                    string portName;
                    int baud;
                    if (comboBox3.SelectedItem != null)
                    {
                        portName = comboBox3.SelectedItem.ToString();
                        if (comboBox4.SelectedItem != null)
                        {
                            ManagementObjectSearcher searcher = new ManagementObjectSearcher("root\\CIMV2", "SELECT * FROM Win32_PnPEntity WHERE Caption LIKE '%" + portName + "%'");
                            ManagementObjectCollection portCollection = searcher.Get();
                            ManagementObject port = portCollection.OfType<ManagementObject>().FirstOrDefault();

                            string description = port["Description"].ToString();
                            if (description.Contains("USB") && description.Contains("UART"))
                            {
                                baud = Convert.ToInt32(comboBox4.SelectedItem.ToString());
                                // Thiết lập thông số kết nối COM           
                                serialPort = new SerialPort();
                                serialPort.PortName = portName;
                                serialPort.BaudRate = baud;
                                serialPort.DataBits = 8;
                                serialPort.Parity = Parity.None;
                                serialPort.StopBits = StopBits.One;
                                serialPort.Handshake = Handshake.None;
                                // Kết nối đến cổng COM
                                serialPort.Open();
                                // Kết nối thành công
                                connected = 1;
                                UART_Connect.BackColor = Color.PaleGreen;
                                groupBox1.Text = portName.ToString() + " Connected";
                                UART_Connect.Text = "Connected";

                            }
                            else MessageBox.Show("Không thể kết nối UART đến " + portName + ".", "Thông báo");
                        }
                        else MessageBox.Show("Vui lòng chọn Baud Rate!", "Thông báo");
                    }
                    else MessageBox.Show("Vui lòng chọn cổng COM để kết nối!", "Thông báo");
                }
                catch (Exception ex)
                {
                    // Kết nối thất bại, hiển thị thông báo lỗi
                    MessageBox.Show(ex.Message);
                }
            }
            else
            {
                connected = 0;
                UART_Connect.BackColor = Color.White;
                UART_Connect.Text = "Connect";
                groupBox1.Text = " Disconnect";
                serialPort.Close();
            }
        }
    } 
}

