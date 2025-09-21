class UI
{
    public Label LabelTemp { private set; get; }
    public Label LabelPress { private set; get; }
    public Label LabelHum { private set; get; }
    public Label LabelEco2 { private set; get; }
    public Label Labeltvoc { private set; get; }
    public Label Labelxyz { private set; get; }
    public PictureBox PictureBoxCam { private set; get; }
    public Button ButtonStart { private set; get; }
    public Button ButtonStop { private set; get; }
    public TextBox Logbox { private set; get; }
    //tutaj sa definicje wszystkich elementow UI
    public UI()
    {

        PictureBoxCam = new PictureBox
        {
            Location = new Point(500, 100),
            Size = new Size(320, 240),
            SizeMode = PictureBoxSizeMode.StretchImage,
            BackColor = Color.Aqua
        };



        ButtonStart = new Button
        {
            Text = "Start",
            Dock = DockStyle.Top,
            Height = 40
        };
        //buttonStart.Click += ButtonStart_Click;

        ButtonStop = new Button
        {
            Text = "Stop",
            Dock = DockStyle.Top,
            Height = 40
        };
        //buttonStop.Click += ButtonStop_Click;

        LabelTemp = new Label
        {
            Location = new Point(50, 50),
            Size = new Size(400, 50),
            Font = new Font("Arial", 12),
            Text = "Temperature: x"
        };



        LabelPress = new Label
        {
            Location = new Point(50, 100),
            Size = new Size(400, 50),
            Font = new Font("Arial", 12),
            Text = "Pressure: x"
        };



        LabelHum = new Label
        {
            Location = new Point(50, 150),
            Size = new Size(400, 50),
            Font = new Font("Arial", 12),
            Text = "Humidity: x"
        };



        LabelEco2 = new Label
        {
            Location = new Point(50, 200),
            Size = new Size(400, 50),
            Font = new Font("Arial", 12),
            Text = "ECO2: x"
        };



        Labeltvoc = new Label
        {
            Location = new Point(50, 250),
            Size = new Size(400, 50),
            Font = new Font("Arial", 12),
            Text = "TVOC: x"
        };

        Labelxyz = new Label
        {
            Location = new Point(50, 300),
            Size = new Size(400, 50),
            Font = new Font("Arial", 12),
            Text = "x: x, y: x, z: x"
        };

        Logbox = new TextBox
        {
            Multiline = true,
            ScrollBars = ScrollBars.Vertical,
            ReadOnly = true,
            Dock = DockStyle.Bottom,
            Height = 150
        };
    }
}