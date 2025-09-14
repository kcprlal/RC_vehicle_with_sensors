using System.Net;
using System.Net.Sockets;
using System.Text.Json;
using SharpDX.XInput;


namespace GuiEsp32;

public partial class Form1 : Form
{
    private Label labelTemp, labelPress, labelHum, labelEco2, labeltvoc, labelxyz, labelstate;
    private PictureBox pictureBoxCam;
    private System.Windows.Forms.Timer timer, snapshotTimer, controlTimer;
    private Controller controller;
    private bool isStopped = false;
    private bool previousYPressed = false;
    private TcpListener server;

    public Form1()
    {
        InitializeComponent();
        StartServer();
        controller = new Controller(UserIndex.One);


        pictureBoxCam = new PictureBox();
        pictureBoxCam.Location = new Point(500, 50);
        pictureBoxCam.Size = new Size(320, 240);
        pictureBoxCam.SizeMode = PictureBoxSizeMode.StretchImage;
        Controls.Add(pictureBoxCam);

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

        //timer odpowiedizalny za obraz z kamerki
        // snapshotTimer = new System.Windows.Forms.Timer();
        // snapshotTimer.Interval = 100;
        // snapshotTimer.Tick += async (s, e) => await UpdateCameraSnapshot(espCamURL);
        // snapshotTimer.Start();

        //timer odpowiedzialny za sterowanie padem
        controlTimer = new System.Windows.Forms.Timer();
        controlTimer.Interval = 50;
        controlTimer.Tick += ControlTimer_Tick;
        controlTimer.Start();

        //_ = GetDataFromHttp(espSensorURL);
    }

    //streaming

    // private async Task UpdateCameraSnapshot(string url)
    // {
    //     try
    //     {
    //         using (HttpClient client = new HttpClient())
    //         {
    //             var imageBytes = await client.GetByteArrayAsync(url);
    //             using (var ms = new MemoryStream(imageBytes))
    //             {
    //                 pictureBoxCam.Image = Image.FromStream(ms);
    //             }
    //         }
    //     }
    //     catch (Exception ex)
    //     {
    //         Console.WriteLine("Błąd pobierania obrazu: " + ex.Message);
    //     }
    // }



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
