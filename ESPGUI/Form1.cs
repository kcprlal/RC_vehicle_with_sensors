using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text.Json;
using SharpDX.Win32;
using SharpDX.XInput;


namespace ESPGUI;

public partial class Form1 : Form
{

    private System.Windows.Forms.Timer controlTimer;
    private Controller controller;
    private TcpListener server;
    private UI ui;
    public Form1()
    {
        InitializeComponent();
        Width = 1280;
        Height = 600;
        ui = new UI();
        StartServer();
        controller = new Controller(UserIndex.One);

        ui.Log("Initializing server...");
        //AddControllersToScreen();

        Controls.Add(ui.Root);
        BackColor = Color.Black;

        //timer odpowiedzialny za sterowanie padem
        controlTimer = new System.Windows.Forms.Timer();
        controlTimer.Interval = 50;
        controlTimer.Tick += ControlTimer_Tick;
        controlTimer.Start();

        var receiver = new UdpReceiver(1234, ui.PictureBoxCam);
    }

    //streaming

    private CancellationTokenSource _cancellationTokenSource;

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
                ui.Log("Socket error: " + ex.Message);
                udpClient.Close();
            }
            catch (Exception ex)
            {
                ui.Log("udp error: " + ex.Message);
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
        ui.Log("Server initialized...");

        while (true)
        {
            try
            {
                var client = await server.AcceptTcpClientAsync();
                ui.Log("Connected to ESP: " + client.Client.RemoteEndPoint);
                _ = HandleClient(client); // działa w tle
            }
            catch (Exception ex)
            {
                ui.Log("Server error: " + ex.Message);
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
                ui.Log(line);
                var data = JsonSerializer.Deserialize<EspData>(line);
                _ = Invoke((MethodInvoker)(() =>
                {
                    ui.UpdateTemperature(data.temperature);
                    ui.UpdatePressure(data.pressure);
                    ui.UpdateHumidity(data.humidity);
                    ui.UpdateEco2(data.eco2);
                    ui.UpdateTvoc(data.tvoc);
                    ui.UpdateXyz(data.ax, data.ay, data.az);
                    ui.UpdateCo(data.co);
                    ui.SoundChart.AddSample(data.sound);
                }));
            }
            catch (Exception ex)
            {
                ui.Log("TCP error");
            }
        }
    }
}
