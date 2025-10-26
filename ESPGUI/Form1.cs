using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text.Json;
using SharpDX.Win32;
using SharpDX.XInput;


namespace ESPGUI;

public partial class Form1 : Form
{

    private System.Windows.Forms.Timer timer, snapshotTimer, controlTimer;
    private Controller controller;
    private bool isStopped = false;
    private bool previousYPressed = false;
    private TcpListener server;
    private UI ui;
    public Form1()
    {
        InitializeComponent();
        Width = 860;
        Height = 600;
        StartServer();
        controller = new Controller(UserIndex.One);
        ui = new UI();
        AddControllersToScreen();

        BackColor = Color.Gray;

        //timer odpowiedzialny za sterowanie padem
        controlTimer = new System.Windows.Forms.Timer();
        controlTimer.Interval = 50;
        controlTimer.Tick += ControlTimer_Tick;
        controlTimer.Start();

        //_ = GetDataFromHttp(espSensorURL);
        //StartCameraUdp(12345);
        var receiver = new UdpReceiver(1234, ui.PictureBoxCam);
    }

    //streaming

    private CancellationTokenSource _cancellationTokenSource;

    private void buttonStart_Click(object sender, EventArgs e)
    {
        StartCameraUdp(12345);
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
                            ui.PictureBoxCam.Invoke((Action)(() =>
                            {
                                ui.PictureBoxCam.Image = img;
                            }));
                        }
                        catch { }
                    }
                }
            }
            catch (SocketException ex)
            {
                Log("Socket error: " + ex.Message);
                udpClient.Close();
            }
            catch (Exception ex)
            {
                Log("udp error: " + ex.Message);
                udpClient.Close();
            }
        }
    }


    private async void ControlTimer_Tick(object sender, EventArgs e)
    {

        if (controller != null && controller.IsConnected)
        {
            var state = controller.GetState();
            var controllerUDP = new RobotController("192.168.137.50", 50002);

            await controllerUDP.SendJoystickCommand(state.Gamepad.RightTrigger, state.Gamepad.LeftTrigger, state.Gamepad.LeftThumbX, (short)state.Gamepad.Buttons);
        }
    }

    private async void StartServer()
    {
        _ = Task.Run(async () =>
    {
        server = new TcpListener(IPAddress.Any, 50001);
        server.Start();
        Log("Serwer uruchomiony...");

        while (true)
        {
            try
            {
                var client = await server.AcceptTcpClientAsync();
                Log("Połączono z ESP: " + client.Client.RemoteEndPoint);
                _ = HandleClient(client); // działa w tle
            }
            catch (Exception ex)
            {
                Log("Błąd serwera: " + ex.Message);
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
                _ = Invoke((MethodInvoker)(() =>
                {
                    ui.LabelTemp.Text = $"Temperature: {data.temperature} °C";
                    ui.LabelPress.Text = $"Pressure: {data.pressure} hPa";
                    ui.LabelHum.Text = $"Humidity: {data.humidity} %";
                    ui.LabelEco2.Text = $"ECO2: {data.eco2} ppm";
                    ui.Labeltvoc.Text = $"TVOC: {data.tvoc} ppm";
                    ui.Labelxyz.Text = $"x: {data.ax}, y: {data.ay}, z: {data.az}";
                    //
                }));
            }
            catch (Exception ex)
            {
                Log("TCP error");
            }
        }
    }

    public void Log(string message)
    {
        if (ui.Logbox.InvokeRequired)
        {
            ui.Logbox.Invoke(new Action(() => Log(message)));
        }
        else
        {
            ui.Logbox.AppendText($"{DateTime.Now:HH:mm:ss}  {message}{Environment.NewLine}");
        }
    }

    public void AddControllersToScreen()
    {
        Controls.Add(ui.PictureBoxCam);
        //Controls.Add(ui.ButtonStart);
        //Controls.Add(ui.ButtonStop);
        Controls.Add(ui.LabelTemp);
        Controls.Add(ui.LabelPress);
        Controls.Add(ui.LabelHum);
        Controls.Add(ui.LabelEco2);
        Controls.Add(ui.Labeltvoc);
        Controls.Add(ui.Labelxyz);
        Controls.Add(ui.Logbox);
    }

}
