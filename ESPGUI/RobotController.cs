using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualBasic.Logging;

class RobotController
{
    private UdpClient udpClient;
    private IPEndPoint remoteEndPoint;
    public string com = "";

    public RobotController(string espIp, int port)
    {
        udpClient = new UdpClient();
        remoteEndPoint = new IPEndPoint(IPAddress.Parse(espIp), port);
    }
    //button = 8 - LB, button = 9 - RB
    public async Task SendJoystickCommand(short RT, short LT, short XaxisValue, short button)
    {
        float Xnormalized = Math.Clamp(XaxisValue / 32767.0f, -1f, 1f);

        if (button == 0x200)
        {
            string rightSpin = "255,255,0,1";
            byte[] rData = Encoding.ASCII.GetBytes(rightSpin);
            await udpClient.SendAsync(rData, rData.Length, remoteEndPoint);
            return;
        }
        if (button == 0x100)
        {
            string leftSpin = "255,255,1,0";
            byte[] lData = Encoding.ASCII.GetBytes(leftSpin);
            await udpClient.SendAsync(lData, lData.Length, remoteEndPoint);
            return;
        }
        // Sprawdzamy martwą strefę joysticka
        if (RT < 10 && LT < 10)
        {
            // Jeśli joystick jest w neutralnej pozycji → STOP
            string stopCommand = "0,0,0,0";
            byte[] stopData = Encoding.ASCII.GetBytes(stopCommand);
            await udpClient.SendAsync(stopData, stopData.Length, remoteEndPoint);
            return;
        }
        int speedA, speedB, dirA, dirB;
         //DIR A I B 1 = forward, 0 = backwards
        if (RT >= LT)
        {
            speedA = RT;
            speedB = RT;
            dirA = 1;
            dirB = 1;
        }
        else
        {
            speedA = LT;
            speedB = LT;
            dirA = 0;
            dirB = 0;
        }

        // Korekta skrętu
        if (Xnormalized > 0.1f)  // w prawo
            speedB = Math.Max(0, speedB - (int)(Xnormalized * 255));
        else if (Xnormalized < -0.1f) // w lewo
            speedA = Math.Max(0, speedA - (int)(-Xnormalized * 255));

        // Wysyłamy pakiet
        string command = $"{speedA},{speedB},{dirA},{dirB}";
        com = $"{speedA},   {speedB}";
        byte[] data = Encoding.ASCII.GetBytes(command);
        await udpClient.SendAsync(data, data.Length, remoteEndPoint);
    }

}
