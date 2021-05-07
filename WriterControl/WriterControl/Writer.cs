using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WriterControl
{
    public class Writer : IDisposable
    {
        private readonly SerialPort serial;
        private List<byte> readBuffer = new List<byte>();

        public bool IsConnected => serial.IsOpen;

        public delegate void ReceiveStringHandler(string s);
        public event ReceiveStringHandler ReceiveString;

        public Writer(string comName)
        {
            serial = new SerialPort(comName);
            serial.DataReceived += SerialReceive;
            serial.ReceivedBytesThreshold = 1;
        }

        private void SerialReceive(object sender, SerialDataReceivedEventArgs e)
        {
            ReceiveString(serial.ReadExisting());
        }

        public void Connect()
        {
            if (!IsConnected)
            {
                serial.Open();
                Write("REQ_C MIWRITER");
            }
        }

        public void Write(byte[] data)
        {
            if (data.Length < 256)
            {
                byte[] buf = { (byte)data.Length };
                serial.Write(buf, 0, 1);
                serial.Write(data, 0, data.Length);
            }
            else
            {
                int count = (int)Math.Ceiling(data.Length / 254.0);
                for (int i=0;i<count;i++)
                {
                    int len = 254;
                    if (i==count-1)
                    {
                        len = data.Length % 254;
                    }
                    byte[] buf = { (byte)(len+1) };
                    serial.Write(buf, 0, 1);
                    serial.Write(data, i * 254, len);
                    serial.Write(new byte[]{ 0xff }, 0, 1);
                }
            }
        }

        public void Write(string data)
        {
            Write(Encoding.ASCII.GetBytes(data));
        }

        public void Close()
        {
            serial.Close();
        }

        public void Dispose()
        {
            Close();
            serial.Dispose();
        }
    }
}
