using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BezierFontEditor
{
    public class Vector2
    {
        public readonly double X;
        public readonly double Y;

        public Vector2(double X, double Y)
        {
            this.X = X;
            this.Y = Y;
        }

        public override string ToString()
        {
            return X.ToString("0.00")+","+Y.ToString("0.00");
        }

        public static Vector2 operator+(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.X + v2.X, v1.Y + v2.Y);
        }

        public static Vector2 operator-(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.X - v2.X, v1.Y - v2.Y);
        }

        public static Vector2 operator*(Vector2 v1, double s2)
        {
            return new Vector2(v1.X * s2, v1.Y * s2);
        }
    }
}
