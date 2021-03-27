using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace BezierFontEditor
{
    public partial class Form1 : Form
    {
        public List<List<Vector2>> ControlPoints = new List<List<Vector2>>();

        public Form1()
        {
            InitializeComponent();
            ControlPoints.Add(new List<Vector2>());
        }

        private Point vtp(Vector2 vec)
        {
            int bdl = Math.Min(panel1.Width, panel1.Height) - 20;
            return new Point { X = panel1.Width/2 + (int)(vec.X * bdl/2), Y = panel1.Height/2 - (int)(vec.Y * bdl/2) };
        }

        private Vector2 ptv(Point p)
        {
            double bdl = Math.Min(panel1.Width, panel1.Height) - 20;
            return new Vector2((p.X * 2 - panel1.Width) / bdl, (panel1.Height - p.Y * 2) / bdl);
        }

        private Vector2 BezierCurve(List<Vector2> points, double t)
        {
            if (points.Count == 2)
            {
                return (points[1] - points[0]) * t + points[0];
            }
            else if (points.Count > 2)
            {
                List<Vector2> newPoints = new List<Vector2>();
                for (int i = 1; i < points.Count; i++)
                {
                    newPoints.Add((points[i] - points[i - 1]) * t + points[i - 1]);
                }
                return BezierCurve(newPoints, t);
            }
            else if (points.Count == 1)
            {
                return points[0];
            }
            else
            {
                return new Vector2(0, 0);
            }
        }

        private void Panel1_Paint(object sender, PaintEventArgs e)
        {
            var graph = e.Graphics;
            Point p1 = vtp(new Vector2(-1, 1)), p2 = vtp(new Vector2(1, -1));
            graph.FillRectangle(Brushes.White, new Rectangle { X = p1.X, Y = p1.Y, Height = p2.Y-p1.Y, Width = p2.X-p1.X });
            graph.DrawRectangle(Pens.Black, new Rectangle { X = p1.X, Y = p1.Y, Height = p2.Y - p1.Y, Width = p2.X - p1.X });
            foreach (List<Vector2> line in ControlPoints)
            {
                if (line.Count > 0) {
                    foreach (Vector2 p in line)
                    {
                        Point loc = vtp(p);
                        graph.FillRectangle(Brushes.Black, new Rectangle { X = loc.X - 1, Y = loc.Y - 1, Width = 3, Height = 3 });
                    }
                }
                if (line.Count >= 2) {
                    Vector2 lastPoint = BezierCurve(line, 0);
                    for (double t = 0.01; t < 1.0; t += 0.01)
                    {
                        Vector2 newPoint = BezierCurve(line, t);
                        graph.DrawLine(Pens.Black, vtp(lastPoint), vtp(newPoint));
                        lastPoint = newPoint;
                    }
                }
            }
        }

        private void Panel1_SizeChanged(object sender, EventArgs e)
        {
            (sender as Panel).Invalidate();
        }

        private void Panel1_MouseDown(object sender, MouseEventArgs e)
        {
            switch (e.Button)
            {
                case MouseButtons.Left:
                    ControlPoints.Last().Add(ptv(e.Location));
                    break;
                case MouseButtons.Right:
                    if (ControlPoints.Last().Count != 0)
                    {
                        ControlPoints.Add(new List<Vector2>());
                    }
                    break;
                case MouseButtons.Middle:
                    var line = ControlPoints.Last();
                    if (line.Count > 0)
                    {
                        line.RemoveAt(line.Count - 1);
                    }
                    break;
            }
            (sender as Panel).Invalidate();
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            if (ControlPoints.Last().Count == 0 && ControlPoints.Count >= 2)
            {
                ControlPoints.RemoveAt(ControlPoints.Count - 1);
                ControlPoints.RemoveAt(ControlPoints.Count - 1);
            } else if (ControlPoints.Last().Count > 0)
            {
                ControlPoints.RemoveAt(ControlPoints.Count - 1);
            }
            panel1.Invalidate();
        }

        private void Button2_Click(object sender, EventArgs e)
        {
            ControlPoints.Clear();
            ControlPoints.Add(new List<Vector2>());
            panel1.Invalidate();
        }

        private void Button3_Click(object sender, EventArgs e)
        {
            List<string> lines = new List<string>();
            foreach (List<Vector2> line in ControlPoints)
            {
                if (line.Count >= 2)
                {
                    List<string> points = new List<string>();
                    foreach (Vector2 point in line)
                    {
                        points.Add(point.ToString());
                    }
                    lines.Add(string.Join("+", points.ToArray()));
                }
            }
            var data = string.Join(";", lines.ToArray());
            Clipboard.SetText(data);
        }
    }
}
