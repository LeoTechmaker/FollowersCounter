using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Compteur_abonnes
{
    class FakeSerialPort
    {
        private bool FIsOpen;

        public int BaudRate { get; set; }
        public string PortName { get; set; }
        public bool IsOpen
        {
            get
            {
                return FIsOpen;
            }
        }

        public FakeSerialPort() => FIsOpen = false;

        internal void Open()
        {
            FIsOpen = true;

            Console.WriteLine("FakeSerialPort : Port " + PortName + " opened with " + BaudRate + ".");
        }

        internal void Close()
        {
            FIsOpen = false;

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
