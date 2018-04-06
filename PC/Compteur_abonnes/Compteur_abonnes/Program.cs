using Newtonsoft.Json.Linq;
using System;
using System.IO.Ports;

namespace Compteur_abonnes
{
    class Program
    {
        //static SerialPort comPort = new SerialPort();
        static FakeSerialPort comPort = new FakeSerialPort();

        static string comPortName         = string.Empty;
        static string youTubeChannelId    = string.Empty;
        static string youTubeApiKey       = string.Empty;
        static string twitterPageName     = string.Empty;
        static string facebookPageId      = string.Empty;
        static string facebookAccessToken = string.Empty;
        static int    mediaDuration       = 0;
        static byte   mediaHold           = 0;

        const byte mediaCount = 3;
        static string[] mediaNames = new string[mediaCount] {"YouTube", "Twitter", "Facebook"};

        static void ExceptionMngt(Exception e)
        {
            // TODO 001 : Implémenter ExceptionMngt ou mieux gérer les exceptions
        }

        static void Main(string[] args)
        {
            // TODO 002 : Un peu trop de blocs try / catch dans le main. Un seul bloc englobant le main devrait suffire.
            // TODO 003 : Faire une méthode init et une méthode run.
            try
            {
                System.IO.StreamReader file = new System.IO.StreamReader("Settings.txt");

                comPortName         = readFileLineExcludingComment(file);
                youTubeChannelId    = readFileLineExcludingComment(file);
                youTubeApiKey       = readFileLineExcludingComment(file);
                twitterPageName     = readFileLineExcludingComment(file);
                facebookPageId      = readFileLineExcludingComment(file);
                facebookAccessToken = readFileLineExcludingComment(file);
                mediaDuration       = int.Parse(readFileLineExcludingComment(file));
                mediaHold           = BitConverter.GetBytes(int.Parse(readFileLineExcludingComment(file)))[0];

                file.Close();
            }
            catch(Exception e)
            {
                Console.WriteLine("Fichier Settings invalide");
                Console.WriteLine(e.Message);
                Console.Read();
                Environment.Exit(0);
            }

            if(comPortName == string.Empty || youTubeChannelId == string.Empty || youTubeApiKey == string.Empty || twitterPageName == string.Empty || facebookPageId == string.Empty || facebookAccessToken == string.Empty || mediaDuration < 1 || mediaHold < 0 || mediaHold > mediaCount)
            {
                Console.WriteLine("Paramètre(s) du fichier Settings invalide(s)");
                Console.Read();
                Environment.Exit(0);
            }

            comPort.BaudRate = 115200;
            comPort.PortName = comPortName;

            try
            {
                comPort.Open();
            }
            catch(Exception e)
            {
                Console.WriteLine("Port COM invalide (" + comPortName + ")");
                Console.WriteLine(e.Message);
                Console.Read();
                Environment.Exit(0);
            }

            System.Threading.Thread.Sleep(1000);

            int count = 0;
            while(true)
            {
                int value = 0;
                byte media = 0;

                if(mediaHold == 0)
                {
                    if (count / mediaDuration == 0)
                    {
                        media = 1;
                    }
                    else if (count / mediaDuration == 1)
                    {
                        media = 2;
                    }
                    else if (count / mediaDuration == 2)
                    {
                        media = 3;
                    }
                    
                    // TODO 004 : media = (int)(count / mediaDuration); // devrait suffire.
                }
                else
                {
                    media = mediaHold;
                }

                try
                {
                    // TODO 005 : Possible d'économiser du code ici.
                    if (media == 1)
                    {
                        media = 1;
                        value = getYoutubeSubscriberCount();
                    }
                    else if (media == 2)
                    {
                        media = 2;
                        value = getTwitterFollowerCount();
                    }
                    else if (media == 3)
                    {
                        media = 3;
                        value = getFacebookLikeCount();
                    }
                }
                catch(Exception e)
                {
                    Console.WriteLine("Erreur de lecture des données pour le média " + mediaNames[media - 1]);
                    Console.WriteLine(e.Message);
                    Console.Read();
                    Environment.Exit(0);
                }

                Console.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " - " + mediaNames[media - 1] + " : " + value);
                
                try
                {
                    refreshCounter(media, value);
                }
                catch(Exception e)
                {
                    Console.WriteLine("Erreur de traitement ou d'envoi des données pour le média " + mediaNames[media - 1] + " sur le port " + comPortName);
                    Console.WriteLine(e.Message);
                    Console.Read();
                    Environment.Exit(0);
                }
                
                count = count < mediaDuration * 3 - 1 ? count + 1 : 0;

                System.Threading.Thread.Sleep(2000);
            }
            
            comPort.Close();
        }

        static string readFileLineExcludingComment(System.IO.StreamReader file)
        {
            // TODO 006 : L'inconvénient c'est que les settings doivent être défini dans un ordre précis.
            bool state = false;

            string line = string.Empty;

            while(!state)
            {
                line = file.ReadLine().Trim(' ');

                if(line != string.Empty)
                {
                    if (line[0] != '#')
                    {
                        state = true;
                    }
                }
            }

            return line;
        }

        static void refreshCounter(byte media, int value)
        {
            byte[] bytesToSend = new byte[5];
            byte[] subCountBytes = BitConverter.GetBytes(value);

            bytesToSend[0] = media;
            
            for(int i = 0; i < 4; i++)
            {
                bytesToSend[i + 1] = subCountBytes[i];
            }

            comPort.Write(bytesToSend, 0, 5);
        }

        // TODO 007 : Mettre tout ça dans des fichiers et des classes séparés.
        static int getYoutubeSubscriberCount()
        {
            System.Net.WebClient wb = new System.Net.WebClient();

            string jsonDatas = wb.DownloadString("https://www.googleapis.com/youtube/v3/channels?part=statistics&id=" + youTubeChannelId + "&key=" + youTubeApiKey);

            dynamic response = JObject.Parse(jsonDatas);
            dynamic items = JObject.Parse(response.items[0].ToString());

            string subCountString = items.statistics.subscriberCount;
            int subCount = int.Parse(subCountString);

            return subCount;
        }

        static int getTwitterFollowerCount()
        {
            System.Net.WebClient wb = new System.Net.WebClient();

            string jsonDatas = wb.DownloadString("http://cdn.syndication.twimg.com/widgets/followbutton/info.json?screen_names=" + twitterPageName);

            jsonDatas = jsonDatas.Substring(1);
            jsonDatas = jsonDatas.Remove(jsonDatas.Length - 1);

            dynamic response = JObject.Parse(jsonDatas);

            string followerCountString = response.followers_count;
            int followerCount = Int32.Parse(followerCountString);

            return followerCount;
        }

        static int getFacebookLikeCount()
        {
            System.Net.WebClient wb = new System.Net.WebClient();

            string jsonDatas = wb.DownloadString("https://graph.facebook.com/v2.8/" + facebookPageId + "?fields=fan_count&access_token=" + facebookAccessToken);

            dynamic response = JObject.Parse(jsonDatas);

            string likeCountString = response.fan_count;
            int likeCount = Int32.Parse(likeCountString);

            return likeCount;
        }
    }
}
