using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Compteur_abonnes
{
    class FakeSerialPort
    {
        public int BaudRate { get; set; }
        public string PortName { get; set; }

        internal void Open()
        {
            Console.WriteLine("FakeSerialPort : Port " + PortName + " opened with " + BaudRate + ".");
        }

        internal void Close()
        {
            Console.WriteLine("FakeSerialPort : Port closed.");
        }

        internal void Write(byte[] bytesToSend, int v1, int v2)
        {
            Console.Write("FakeSerialPort : ");
            
            for(int i = v1; i < v2; i++)
            {
                Console.Write(bytesToSend[i] + " ");
            }

            Console.WriteLine("");
        }
    }
}
