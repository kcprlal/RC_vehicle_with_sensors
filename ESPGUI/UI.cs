using System.Drawing;
using System.Windows.Forms;

public class UI
{
    public Label LabelTemp { get; private set; }
    public Label LabelPress { get; private set; }
    public Label LabelHum { get; private set; }
    public Label LabelEco2 { get; private set; }
    public Label LabelTvoc { get; private set; }
    public Label LabelXyz { get; private set; }
    public Label LabelCo {get; private set; }

    public PictureBox PictureBoxCam { get; private set; }
    // public Button ButtonStart { get; private set; }
    // public Button ButtonStop { get; private set; }
    public TextBox LogBox { get; private set; }

    public Control Root { get; private set; }

    private readonly Font labelFont = new Font("Segoe UI", 10);
    private readonly Font valueFont = new Font("Segoe UI", 10, FontStyle.Bold);

    public SoundLevelChart SoundChart { get; private set; }

    public UI()
    {
        Root = BuildLayout();
    }

    private Control BuildLayout()
    {
        var root = new Panel
        {
            Dock = DockStyle.Fill,
            BackColor = Color.DarkGray
        };

        var dataPanel = BuildDataPanel();
        var cameraPanel = BuildCameraPanel();
        // var buttonPanel = BuildButtonPanel();
        var logPanel = BuildLogPanel();

        root.Controls.Add(dataPanel);
        root.Controls.Add(cameraPanel);
        // root.Controls.Add(buttonPanel);
        root.Controls.Add(logPanel);

        var soundPanel = BuildSoundPanel();
        root.Controls.Add(soundPanel);

        return root;
    }
    
    private Control BuildSoundPanel()
{
    var group = new GroupBox
    {
        Text = "Sound detection",
        Font = new Font("Segoe UI", 10, FontStyle.Bold),
        Location = new Point(830, 100),
        Size = new Size(360, 200)
    };

    SoundChart = new SoundLevelChart
    {
        Dock = DockStyle.Fill
    };

    group.Controls.Add(SoundChart);
    return group;
}

    private Control BuildDataPanel()
    {
        var group = new GroupBox
        {
            Text = "Acquired data",
            Font = new Font("Segoe UI", 10, FontStyle.Bold),
            Location = new Point(20, 20),
            Size = new Size(400, 280)
        };

        var table = new TableLayoutPanel
        {
            Dock = DockStyle.Fill,
            ColumnCount = 2,
            RowCount = 6
        };

        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));

        LabelTemp = CreateValueLabel();
        LabelPress = CreateValueLabel();
        LabelHum = CreateValueLabel();
        LabelEco2 = CreateValueLabel();
        LabelTvoc = CreateValueLabel();
        LabelXyz = CreateValueLabel();
        LabelCo = CreateValueLabel();

        AddRow(table, "Temperature", LabelTemp, 0);
        AddRow(table, "Pressure", LabelPress, 1);
        AddRow(table, "Humidity", LabelHum, 2);
        AddRow(table, "ECO2", LabelEco2, 3);
        AddRow(table, "TVOC", LabelTvoc, 4);
        AddRow(table, "XYZ", LabelXyz, 5);
        AddRow(table, "Dangerous gas level", LabelCo, 6);

        group.Controls.Add(table);
        return group;
    }

    private Control BuildCameraPanel()
    {
        var group = new GroupBox
        {
            Text = "Camera",
            Font = new Font("Segoe UI", 10, FontStyle.Bold),
            Location = new Point(450, 20),
            Size = new Size(360, 300)
        };

        PictureBoxCam = new PictureBox
        {
            Dock = DockStyle.Fill,
            SizeMode = PictureBoxSizeMode.StretchImage,
            BackColor = Color.Black,
            BorderStyle = BorderStyle.FixedSingle
        };

        group.Controls.Add(PictureBoxCam);
        return group;
    }

    private Control BuildLogPanel()
    {
        var group = new GroupBox
        {
            Text = "Log",
            Dock = DockStyle.Bottom,
            Height = 180
        };

        LogBox = new TextBox
        {
            Multiline = true,
            ReadOnly = true,
            Dock = DockStyle.Fill,
            ScrollBars = ScrollBars.Vertical,
            Font = new Font("Consolas", 9),
            BackColor = Color.Black,
            ForeColor = Color.LightGreen
        };

        group.Controls.Add(LogBox);
        return group;
    }

    private Label CreateTitleLabel(string text)
    {
        return new Label
        {
            Text = text,
            Font = labelFont,
            Dock = DockStyle.Fill,
            TextAlign = ContentAlignment.MiddleLeft
        };
    }

    private Label CreateValueLabel()
    {
        return new Label
        {
            Text = "-",
            Font = valueFont,
            Dock = DockStyle.Fill,
            TextAlign = ContentAlignment.MiddleLeft,
            ForeColor = Color.DarkBlue
        };
    }

    private void AddRow(TableLayoutPanel table, string title, Label value, int row)
    {
        table.RowStyles.Add(new RowStyle(SizeType.Percent, 16));
        table.Controls.Add(CreateTitleLabel(title), 0, row);
        table.Controls.Add(value, 1, row);
    }

    public void UpdateTemperature(float value) => LabelTemp.Text = $"{value:F1} °C";
    public void UpdatePressure(float value) => LabelPress.Text = $"{value:F0} hPa";
    public void UpdateHumidity(float value) => LabelHum.Text = $"{value:F1} %";
    public void UpdateEco2(int value) => LabelEco2.Text = $"{value} ppm";
    public void UpdateTvoc(int value) => LabelTvoc.Text = $"{value} ppb";
    public void UpdateXyz(float x, float y, float z)
        => LabelXyz.Text = $"x:{x:F2} y:{y:F2} z:{z:F2}";
    public void UpdateCo(float value) => LabelCo.Text = $"{value}";
    public void Log(string text)
    {
        LogBox.AppendText(text + "\r\n");
    }
}
