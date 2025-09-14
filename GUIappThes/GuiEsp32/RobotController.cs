using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

class RobotController
{
    private UdpClient udpClient;
    private IPEndPoint remoteEndPoint;

    public RobotController(string espIp, int port)
    {
        udpClient = new UdpClient();
        remoteEndPoint = new IPEndPoint(IPAddress.Parse(espIp), port);
    }

    public async Task SendJoystickCommand(short YaxisValue, short XaxisValue)
{
    float Ynormalized = Math.Clamp(YaxisValue / 32767.0f, -1f, 1f);
    float Xnormalized = Math.Clamp(XaxisValue / 32767.0f, -1f, 1f);

    // Sprawdzamy martwą strefę joysticka
    if (Math.Abs(Ynormalized) < 0.05f && Math.Abs(Xnormalized) < 0.05f)
    {
        // Jeśli joystick jest w neutralnej pozycji → STOP
        string stopCommand = "0,0,0,0";
        byte[] stopData = Encoding.ASCII.GetBytes(stopCommand);
        await udpClient.SendAsync(stopData, stopData.Length, remoteEndPoint);
        return;
    }

    // Prędkości
    int speedA = (int)(Math.Abs(Ynormalized) * 255);
    int speedB = (int)(Math.Abs(Ynormalized) * 255);

    // Kierunki
    int dirA = Ynormalized > 0 ? 1 : 0;
    int dirB = Ynormalized > 0 ? 1 : 0;

    // Korekta skrętu
    if (Xnormalized > 0.1f)  // w prawo
        speedB = Math.Max(0, speedB - (int)(Xnormalized * 255));
    else if (Xnormalized < -0.1f) // w lewo
        speedA = Math.Max(0, speedA - (int)(-Xnormalized * 255));

    // Wysyłamy pakiet
    string command = $"{speedA},{speedB},{dirA},{dirB}";
    byte[] data = Encoding.ASCII.GetBytes(command);
    await udpClient.SendAsync(data, data.Length, remoteEndPoint);
}

}
