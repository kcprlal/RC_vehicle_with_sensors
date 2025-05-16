using System.Text.Json;

namespace GuiEsp32;

public partial class Form1 : Form
{
    private Label labelTemp, labelPress, labelHum;
    private System.Windows.Forms.Timer timer;
    private string espSensorURL = "http://192.168.247.167";
    private string espCamURL = "";

    public Form1()
    {
        InitializeComponent();
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

        timer = new System.Windows.Forms.Timer();
        timer.Interval = 1000;
        timer.Tick += Timer_Tick;
        timer.Start();

        _ = GetDataFromHttp(espSensorURL);

    }

    private async void Timer_Tick(object sender, EventArgs e)
    {
        string data = await GetDataFromHttp(espSensorURL);

        var weather = JsonSerializer.Deserialize<WeatherData>(data);

        labelTemp.Text = "Temperatura: " + weather.temperatura + "°C";
        labelPress.Text = "Cisnienie: " + weather.cisnienie + " hPa";
        labelHum.Text = "Wilgotnosc: " + weather.wilgotnosc + "%";
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

    public class WeatherData
    {
        public float temperatura { get; set; }
        public int cisnienie { get; set; }
        public float wilgotnosc { get; set; }
    }

}
