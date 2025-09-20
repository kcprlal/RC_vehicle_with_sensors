using System.Net;
using System.Net.Sockets;
using System.Threading;

public class UdpReceiver
{
    private UdpClient udp;
    private Thread receiveThread;
    private PictureBox pictureBox;

    // bufor dla aktualnej ramki
    private Dictionary<ushort, byte[]> frameBuffer = new Dictionary<ushort, byte[]>();
    private int expectedPackets = -1;

    public UdpReceiver(int port, PictureBox pb)
    {
        udp = new UdpClient(port);
        pictureBox = pb;
        receiveThread = new Thread(ReceiveLoop);
        receiveThread.IsBackground = true;
        receiveThread.Start();
    }

    private void ReceiveLoop()
    {
        IPEndPoint ep = new IPEndPoint(IPAddress.Any, 0);

        while (true)
        {
            byte[] data = udp.Receive(ref ep);

            if (data.Length < 4) continue; // za mało danych

            // odczyt nagłówka
            ushort packetIndex = BitConverter.ToUInt16(data, 0);
            ushort totalPackets = BitConverter.ToUInt16(data, 2);

            byte[] chunk = new byte[data.Length - 4];
            Buffer.BlockCopy(data, 4, chunk, 0, chunk.Length);

            // zapisz pakiet
            lock (frameBuffer)
            {
                if (packetIndex == 0)
                {
                    // nowa ramka → czyść bufor
                    frameBuffer.Clear();
                    expectedPackets = totalPackets;
                }

                if (expectedPackets > 0)
                {
                    frameBuffer[packetIndex] = chunk;

                    // sprawdź czy wszystkie dotarły
                    if (frameBuffer.Count == expectedPackets)
                    {
                        try
                        {
                            // sklej dane
                            var ordered = frameBuffer.OrderBy(kv => kv.Key).SelectMany(kv => kv.Value).ToArray();
                            using (var ms = new MemoryStream(ordered))
                            {
                                var img = Image.FromStream(ms);
                                pictureBox.Invoke((Action)(() =>
                                {
                                    pictureBox.Image = img;
                                }));
                            }
                        }
                        catch
                        {
                            // błędna ramka – ignoruj
                        }

                        // gotowe – wyczyść
                        frameBuffer.Clear();
                        expectedPackets = -1;
                    }
                }
            }
        }
    }
}
