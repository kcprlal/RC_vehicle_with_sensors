using System.Text.Json;

namespace GuiEsp32;

public partial class Form1 : Form
{
    private Label labelTemp, labelPress, labelHum, labelEco2, labeltvoc, labelxyz;
    private PictureBox pictureBoxCam;
    private System.Windows.Forms.Timer timer, snapshotTimer;
    private string espSensorURL = "http://192.168.1.30";
    private string espCamURL = "http://192.168.1.29/jpg";

    public Form1()
    {
        InitializeComponent();
        pictureBoxCam = new PictureBox();
        pictureBoxCam.Location = new Point(500, 50);
        pictureBoxCam.Size = new Size(320, 240);
        pictureBoxCam.SizeMode = PictureBoxSizeMode.StretchImage;
        Controls.Add(pictureBoxCam);

        labelTemp = new Label();
        labelTemp.Location = new Point(50, 50);
        labelTemp.Size = new Size(400, 100);
        labelTemp.Font = new Font("Arial", 12);
        Controls.Add(labelTemp);

        labelPress = new Label();
        labelPress.Location = new Point(50, 150);
        labelPress.Size = new Size(400, 100);
        labelPress.Font = new Font("Arial", 12);
        Controls.Add(labelPress);

        labelHum = new Label();
        labelHum.Location = new Point(50, 250);
        labelHum.Size = new Size(400, 100);
        labelHum.Font = new Font("Arial", 12);
        Controls.Add(labelHum);

        labelEco2 = new Label();
        labelEco2.Location = new Point(50, 350);
        labelEco2.Size = new Size(400, 100);
        labelEco2.Font = new Font("Arial", 12);
        Controls.Add(labelEco2);

        labeltvoc = new Label();
        labeltvoc.Location = new Point(50, 450);
        labeltvoc.Size = new Size(400, 100);
        labeltvoc.Font = new Font("Arial", 12);
        Controls.Add(labeltvoc);

        labelxyz = new Label();
        labelxyz.Location = new Point(50, 550);
        labelxyz.Size = new Size(400, 100);
        labelxyz.Font = new Font("Arial", 12);
        Controls.Add(labelxyz);

        //timer odpowidzialny za wykonywanie poamirow
        timer = new System.Windows.Forms.Timer();
        timer.Interval = 1000;
        timer.Tick += Timer_Tick;
        timer.Start();
        //timer odpowiedizalny za obraz z kamerki
        snapshotTimer = new System.Windows.Forms.Timer();
        snapshotTimer.Interval = 100;
        snapshotTimer.Tick += async (s, e) => await UpdateCameraSnapshot(espCamURL);
        snapshotTimer.Start();

        _ = GetDataFromHttp(espSensorURL);
    }

    //streaminh

    private async Task UpdateCameraSnapshot(string url)
    {
        try
        {
            using (HttpClient client = new HttpClient())
            {
                var imageBytes = await client.GetByteArrayAsync(url);
                using (var ms = new MemoryStream(imageBytes))
                {
                    pictureBoxCam.Image = Image.FromStream(ms);
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine("Błąd pobierania obrazu: " + ex.Message);
        }
    }



    private async void Timer_Tick(object sender, EventArgs e)
    {
        string data = await GetDataFromHttp(espSensorURL);

        var sensors = JsonSerializer.Deserialize<EspData>(data);

        labelTemp.Text = "Temperatura: " + sensors.temperatura + "°C";
        labelPress.Text = "Cisnienie: " + sensors.cisnienie + " hPa";
        labelHum.Text = "Wilgotnosc: " + sensors.wilgotnosc + "%";
        labelEco2.Text = "ECO2: " + sensors.eco2 + "ppm";
        labeltvoc.Text = "TVOC: " + sensors.tvoc + "ppm";
        labelxyz.Text = "X: " + sensors.x + " Y: " + sensors.y + " Z: " + sensors.z;
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
        public float temperatura { get; set; }
        public int cisnienie { get; set; }
        public float wilgotnosc { get; set; }
        public float eco2 { get; set; }
        public float tvoc { get; set; }
        public int x { get; set; }
        public int y { get; set; }
        public int z { get; set; }
    }

}
