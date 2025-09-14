using System.Text.Json;
using SharpDX.XInput;
using System.Net.Http;

namespace GuiEsp32;

public partial class Form1 : Form
{
    private Label labelTemp, labelPress, labelHum, labelEco2, labeltvoc, labelxyz, labelstate;
    private PictureBox pictureBoxCam;
    private System.Windows.Forms.Timer timer, snapshotTimer, controlTimer;
    private string espSensorURL = "http://192.168.1.123/sensors";
    //private string espCamURL = "http://192.168.1.29/jpg";
    private Controller controller;
    private HttpClient httpClient = new HttpClient();
    private bool isStopped = false;
    private bool previousYPressed = false;

    public Form1()
    {
        InitializeComponent();
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

        //timer odpowidzialny za wykonywanie poamirow
        timer = new System.Windows.Forms.Timer();
        timer.Interval = 500;
        timer.Tick += Timer_Tick;
        timer.Start();
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



    private async void Timer_Tick(object sender, EventArgs e)
    {
        // Pobieranie danych z ESP
        string data = await GetDataFromHttp(espSensorURL);

        try
        {
            var sensors = JsonSerializer.Deserialize<EspData>(data);

            labelTemp.Text = "Temperatura: " + sensors.temperature + "°C";
            labelPress.Text = "Cisnienie: " + sensors.pressure + " hPa";
            labelHum.Text = "Wilgotnosc: " + sensors.humidity + "%";
            labelEco2.Text = "ECO2: " + sensors.eco2 + "ppm";
            labeltvoc.Text = "TVOC: " + sensors.tvoc + "ppm";
            labelxyz.Text = "X: " + sensors.x + " Y: " + sensors.y + " Z: " + sensors.z;
        }
        catch (Exception ex)
        {
            Console.WriteLine("Błąd deserializacji: " + ex.Message);
        }
    }

    private async void ControlTimer_Tick(object sender, EventArgs e)
    {

        // Odczyt pozycji gałki i wysyłka HTTP
        if (controller != null && controller.IsConnected)
        {
            var state = controller.GetState();
            var buttons = state.Gamepad.Buttons;

            bool currentYPressed = (buttons & SharpDX.XInput.GamepadButtonFlags.Y) != 0;

            await SendJoystickCommand(state.Gamepad.RightThumbY, state.Gamepad.RightThumbX);
            // Wykrywanie wciśnięcia przycisku Y (zamiast trzymania)
            // if (currentYPressed && !previousYPressed)
            // {
            //     isStopped = !isStopped;

            //     string url = $"http://192.168.1.123/{(isStopped ? "stop" : "start")}";
            //     try
            //     {
            //         await httpClient.GetAsync(url);
            //         labelstate.Text = isStopped ? "STOPPED" : "READY";
            //     }
            //     catch (Exception ex)
            //     {
            //         Console.WriteLine("Błąd wysyłania HTTP: " + ex.Message);
            //     }
            // }
            // previousYPressed = currentYPressed;

            // if (!isStopped)
            // {
            //     await SendJoystickCommand(state.Gamepad.RightThumbY, state.Gamepad.RightThumbX);
            // }
        }
    }

    private async Task SendJoystickCommand(short YaxisValue, short XaxisValue)
    {
        string url;
        float Ynormalized = Math.Clamp(YaxisValue / 32767.0f, -1f, 1f);
        float Xnormalized = Math.Clamp(XaxisValue / 32767.0f, -1f, 1f);

        // Mix dla sterowania typu "tank"
        float left = Ynormalized + Xnormalized;
        float right = Ynormalized - Xnormalized;

        // Ogranicz do zakresu [-1, 1]
        left = Math.Clamp(left, -1f, 1f);
        right = Math.Clamp(right, -1f, 1f);

        // Oblicz PWM
        int speedA = (int)(Math.Abs(left) * 255);
        int speedB = (int)(Math.Abs(right) * 255);

        // Kierunek
        string direction;
        if (left > 0 && right > 0)
            direction = "forward";
        else if (left < 0 && right < 0)
            direction = "backward";
        else if (left > 0 && right < 0)
            direction = "turnright";
        else if (left < 0 && right > 0)
            direction = "turnleft";
        else
            direction = "stop";

        // Wyślij zapytanie
        if (direction == "stop")
        {
            url = $"http://192.168.1.123/{direction}";
        }
        else
        {
            url = $"http://192.168.1.123/{direction}?speedA={speedA}&speedB={speedB}";
        }

        try
        {
            await httpClient.GetAsync(url);
        }
        catch (Exception ex)
        {
            Console.WriteLine("Błąd wysyłania HTTP: " + ex.Message);
        }
    }



    private async Task<string> GetDataFromHttp(string url)
    {
        using (HttpClient client = new HttpClient())
        {
            try
            {
                HttpResponseMessage response = await client.GetAsync(url);
                response.EnsureSuccessStatusCode();

                string responseData = await response.Content.ReadAsStringAsync();
                return responseData;
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Wystapil blad: {ex.Message}");
                return "Blad podczas pobierania danych";
            }
        }
    }

    public class EspData
    {
        public float temperature { get; set; }
        public float pressure { get; set; }
        public float humidity { get; set; }
        public float eco2 { get; set; }
        public float tvoc { get; set; }
        public float x { get; set; }
        public float y { get; set; }
        public float z { get; set; }
    }

}
