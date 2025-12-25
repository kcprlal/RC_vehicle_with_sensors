using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

public class SoundLevelChart : Control
{
    private readonly Queue<float> samples = new Queue<float>();
    private const int MaxSamples = 200;

    public float MinValue = 0f;
    public float MaxValue = 5000f;

    public SoundLevelChart()
    {
        DoubleBuffered = true;
        BackColor = Color.Black;
        ForeColor = Color.LimeGreen;
    }

    public void AddSample(float value)
    {
        value = Math.Clamp(value, MinValue, MaxValue);

        if (samples.Count >= MaxSamples)
            samples.Dequeue();

        samples.Enqueue(value);
        Invalidate();
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        base.OnPaint(e);

        if (samples.Count < 2)
            return;

        var g = e.Graphics;
        g.Clear(BackColor);

        var pen = new Pen(ForeColor, 2);

        float xStep = (float)Width / (MaxSamples - 1);
        float[] values = samples.ToArray();

        for (int i = 1; i < values.Length; i++)
        {
            float x1 = (i - 1) * xStep;
            float x2 = i * xStep;

            float y1 = Height - (values[i - 1] / MaxValue) * Height;
            float y2 = Height - (values[i] / MaxValue) * Height;

            g.DrawLine(pen, x1, y1, x2, y2);
        }
    }
}
