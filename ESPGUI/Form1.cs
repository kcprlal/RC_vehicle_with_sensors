using System.Net;
using System.Net.Sockets;
using System.Text.Json;
using SharpDX.XInput;


namespace ESPGUI;

public partial class Form1 : Form
{
    private Label labelTemp, labelPress, labelHum, labelEco2, labeltvoc, labelxyz, labelstate;
    private PictureBox pictureBoxCam;
    private System.Windows.Forms.Timer timer, snapshotTimer, controlTimer;
    private Controller controller;
    private bool isStopped = false;
    private bool previousYPressed = false;
    private TcpListener server;
    private Button buttonStart, buttonStop;
    public Form1()
    {
        InitializeComponent();
        StartServer();
        controller = new Controller(UserIndex.One);
        this.BackColor = Color.Gray;

        pictureBoxCam = new PictureBox();
        pictureBoxCam.Location = new Point(500, 100);
        pictureBoxCam.Size = new Size(320, 240);
        pictureBoxCam.SizeMode = PictureBoxSizeMode.StretchImage;
        pictureBoxCam.BackColor = Color.Aqua;
        Controls.Add(pictureBoxCam);

        buttonStart = new Button
        {
            Text = "Start",
            Dock = DockStyle.Top,
            Height = 40
        };
        //buttonStart.Click += ButtonStart_Click;

        buttonStop = new Button
        {
            Text = "Stop",
            Dock = DockStyle.Top,
            Height = 40
        };
        //buttonStop.Click += ButtonStop_Click;

        this.Controls.Add(buttonStop);
        this.Controls.Add(buttonStart);

        labelTemp = new Label();
        labelTemp.Location = new Point(50, 50);
        labelTemp.Size = new Size(400, 50);
        labelTemp.Font = new Font("Arial", 12);
        labelTemp.Text = "Temperature: x";
        Controls.Add(labelTemp);

        labelPress = new Label();
        labelPress.Location = new Point(50, 100);
        labelPress.Size = new Size(400, 50);
        labelPress.Font = new Font("Arial", 12);
        labelPress.Text = "Pressure: x";
        Controls.Add(labelPress);

        labelHum = new Label();
        labelHum.Location = new Point(50, 150);
        labelHum.Size = new Size(400, 50);
        labelHum.Font = new Font("Arial", 12);
        labelHum.Text = "Humidity: x";
        Controls.Add(labelHum);

        labelEco2 = new Label();
        labelEco2.Location = new Point(50, 200);
        labelEco2.Size = new Size(400, 50);
        labelEco2.Font = new Font("Arial", 12);
        labelEco2.Text = "ECO2: x";
        Controls.Add(labelEco2);

        labeltvoc = new Label();
        labeltvoc.Location = new Point(50, 250);
        labeltvoc.Size = new Size(400, 50);
        labeltvoc.Font = new Font("Arial", 12);
        labeltvoc.Text = "TVOC: x";
        Controls.Add(labeltvoc);

        labelxyz = new Label();
        labelxyz.Location = new Point(50, 300);
        labelxyz.Size = new Size(400, 50);
        labelxyz.Font = new Font("Arial", 12);
        labelxyz.Text = "x: x, y: x, z: x";
        Controls.Add(labelxyz);

        labelstate = new Label();
        labelstate.Location = new Point(50, 350);
        labelstate.Size = new Size(400, 50);
        labelstate.Font = new Font("Arial", 12);
        labelstate.Text = "State: off";
        Controls.Add(labelstate);

        //timer odpowiedzialny za sterowanie padem
        controlTimer = new System.Windows.Forms.Timer();
        controlTimer.Interval = 50;
        controlTimer.Tick += ControlTimer_Tick;
        controlTimer.Start();

        //_ = GetDataFromHttp(espSensorURL);
        //StartCameraUdp(12345);
        var receiver = new UdpReceiver(1234, pictureBoxCam);
    }

    //streaming

    private CancellationTokenSource _cancellationTokenSource;

    private void buttonStart_Click(object sender, EventArgs e)
    {
        StartCameraUdp(12345); // port musi się zgadzać z ESP32-CAM
    }

    private void buttonStop_Click(object sender, EventArgs e)
    {
        StopCameraUdp();
    }

    private void StartCameraUdp(int port)
    {
        _cancellationTokenSource = new CancellationTokenSource();
        Task.Run(() => ReceiveUdpStream(port, _cancellationTokenSource.Token));
    }

    private void StopCameraUdp()
    {
        _cancellationTokenSource?.Cancel();
    }

    private void ReceiveUdpStream(int port, CancellationToken token)
    {
        UdpClient udpClient = new UdpClient(port);
        udpClient.Client.ReceiveTimeout = 2000;

        Dictionary<int, byte[]> packetBuffer = new Dictionary<int, byte[]>();
        int totalPackets = 0;

        IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, 0);

        while (true)
        {
            try
            {
                byte[] data = udpClient.Receive(ref remoteEP);

                if (data.Length < 16) continue;

                // Parsowanie nagłówka
                int index = BitConverter.ToInt32(data, 0);
                int total = BitConverter.ToInt32(data, 8);
                byte[] payload = new byte[data.Length - 16];
                Buffer.BlockCopy(data, 16, payload, 0, payload.Length);

                if (totalPackets == 0 || total != totalPackets)
                {
                    packetBuffer.Clear();
                    totalPackets = total;
                }

                packetBuffer[index] = payload;

                // Jeśli mamy komplet
                if (packetBuffer.Count == totalPackets)
                {
                    using (MemoryStream ms = new MemoryStream())
                    {
                        for (int i = 0; i < totalPackets; i++)
                        {
                            if (packetBuffer.ContainsKey(i))
                                ms.Write(packetBuffer[i], 0, packetBuffer[i].Length);
                        }

                        packetBuffer.Clear();
                        totalPackets = 0;

                        try
                        {
                            Image img = Image.FromStream(ms);
                            pictureBoxCam.Invoke((Action)(() =>
                            {
                                pictureBoxCam.Image = img;
                            }));
                        }
                        catch { }
                    }
                }
            }
            catch (SocketException)
            {
                // Timeout – pozwala sprawdzać token i zamknąć czysto
            }
            catch (Exception ex)
            {
                Console.WriteLine("Błąd UDP: " + ex.Message);
            }
        }

        udpClient.Close();
    }



    // private async void Timer_Tick(object sender, EventArgs e)
    // {
    //     var client = server.AcceptTcpClient();


    // }

    private async void ControlTimer_Tick(object sender, EventArgs e)
    {

        if (controller != null && controller.IsConnected)
        {
            var state = controller.GetState();
            var buttons = state.Gamepad.Buttons;
            var controllerUDP = new RobotController("192.168.137.50", 4210);

            await controllerUDP.SendJoystickCommand(state.Gamepad.RightThumbY, state.Gamepad.RightThumbX);
        }
    }

    private async void StartServer()
    {
        _ = Task.Run(async () =>
    {
        server = new TcpListener(IPAddress.Any, 5000);
        server.Start();
        //AppendLog("Serwer uruchomiony...");

        while (true)
        {
            try
            {
                var client = await server.AcceptTcpClientAsync();
                //AppendLog("Połączono z ESP: " + client.Client.RemoteEndPoint);
                _ = HandleClient(client); // działa w tle
            }
            catch (Exception ex)
            {
                //AppendLog("Błąd serwera: " + ex.Message);
            }
        }
    });
    }

    private async Task HandleClient(TcpClient client)
    {
        using var reader = new StreamReader(client.GetStream());

        while (client.Connected)
        {
            string line = await reader.ReadLineAsync();
            if (line == null) break;

            try
            {
                var data = JsonSerializer.Deserialize<EspData>(line);
                this.Invoke((MethodInvoker)(() =>
                {
                    labelTemp.Text = $"Temperature: {data.Temperature} °C";
                    labelPress.Text = $"Pressure: {data.Pressure} hPa";
                    labelHum.Text = $"Humidity: {data.Humidity} %";
                    labelEco2.Text = $"ECO2: {data.Eco2} ppm";
                    labeltvoc.Text = $"TVOC: {data.Tvoc} ppm";
                    labelxyz.Text = $"x: {data.Ax}, y: {data.Ay}, z: {data.Az}";
                    //
                }));
            }
            catch (Exception ex)
            {
                //
            }
        }
    }



}
