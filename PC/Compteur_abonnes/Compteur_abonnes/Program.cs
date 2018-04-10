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

        const  byte     mediaCount = 3;
        static string[] mediaNames = new string[mediaCount] {"YouTube", "Twitter", "Facebook"};

        static void Main(string[] args)
        {
            try
            {
                // Initialisation
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

                if (comPortName         == string.Empty ||
                    youTubeChannelId    == string.Empty ||
                    youTubeApiKey       == string.Empty ||
                    twitterPageName     == string.Empty ||
                    facebookPageId      == string.Empty ||
                    facebookAccessToken == string.Empty ||
                    mediaDuration       < 1 ||
                    mediaHold           < 0 ||
                    mediaHold           > mediaCount)
                {
                    throw new Exception("Paramètre(s) du fichier Settings invalide(s)");
                }

                comPort.BaudRate = 115200;
                comPort.PortName = comPortName;
                
                comPort.Open();

                System.Threading.Thread.Sleep(1000);

                int count = 0;
                char c = ' ';

                // Boucle
                while (c != 27)
                {
                    int value = 0;
                    byte media = 0;

                    if (mediaHold == 0) media = (byte)((count / mediaDuration) + 1);
                    else media = mediaHold;

                    switch(media)
                    {
                        case 1:
                            value = getYoutubeSubscriberCount();
                            break;
                        case 2:
                            value = getTwitterFollowerCount();
                            break;
                        case 3:
                            value = getFacebookLikeCount();
                            break;
                        default:
                            throw new Exception("Invalid value.");
                    }

                    Console.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " - " + mediaNames[media - 1] + " : " + value);
                    
                    refreshCounter(media, value);

                    count = count < mediaDuration * 3 - 1 ? count + 1 : 0;

                    System.Threading.Thread.Sleep(2000);

                    // Si on appuie sur ESC (ASCII code = 27), la boucle s'interromp proprement.
                    if (Console.KeyAvailable) c = Console.ReadKey().KeyChar;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception : " + e.Message);
                Console.Read();
            }
            finally
            {
                if(comPort.IsOpen) comPort.Close();
            }
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
