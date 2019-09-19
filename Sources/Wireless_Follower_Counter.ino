#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <JsonStreamingParser.h>
#include <YoutubeApi.h>
#include <FacebookApi.h>
#include <InstagramStats.h>
#include <TwitchApi.h>
#include <Adafruit_NeoPixel.h>
#include "Config.h"

#define mediaCount 5
#define mediaDurationDefaultValue 4

#define refreshInterval 2000

#define ledPin D8
#define ledAmount 320
#define brightnessDefaultValue 5

#define settingsResetPin D7
#define settingsResetGndPin D6

#define pannelHeight 8
#define pannelWidth 40
#define digitAmount 6

#define eepromCheckValue 123

ESP8266WebServer server(80);
WiFiClientSecure client;

FacebookApi facebookApi(client, facebookAccessToken, facebookAppId, facebookAppSecret);
YoutubeApi youtubeApi(youtubeApiKey, client);
InstagramStats instaStats(client);
TwitchApi twitch(client, twitchClientId);

Adafruit_NeoPixel bande = Adafruit_NeoPixel(ledAmount, ledPin, NEO_GRB + NEO_KHZ800);

const String mediaName[mediaCount] = {"YouTube", "Twitter", "Facebook", "Instagram", "Twitch"};
unsigned int mediaDuration[mediaCount] = {mediaDurationDefaultValue, mediaDurationDefaultValue, mediaDurationDefaultValue, mediaDurationDefaultValue, mediaDurationDefaultValue};
const unsigned int mediaCallLimits[mediaCount] = {0, 0, 0, 0, 1800}; // Limite de calls à l'API en calls/h
unsigned long mediaLastCallMillis[mediaCount];
unsigned long mediaLastValue[mediaCount];
bool firstCallDone[mediaCount];
bool mediaEnabled[mediaCount];

byte pannel[pannelHeight][pannelWidth][3];

byte brightness = brightnessDefaultValue;

byte previousMedia = 0;
unsigned long previousNumber = 0;

//const byte lettres[26][6][4];
const byte digits[35] =
{
  0b11111111,
  0b10011000,
  0b10011000,
  0b10011111,
  0b10010001,
  0b10010001,
  0b11111111,

  0b00011111,
  0b00011000,
  0b00011000,
  0b00011111,
  0b00011001,
  0b00011001,
  0b00011111,

  0b11111111,
  0b00010001,
  0b00010001,
  0b11110001,
  0b10000001,
  0b10000001,
  0b11110001,

  0b11111111,
  0b00011001,
  0b00011001,
  0b11111111,
  0b00011001,
  0b00011001,
  0b11111111,

  0b10011111,
  0b10011001,
  0b10011001,
  0b11111111,
  0b00010001,
  0b00010001,
  0b00011111
};

const byte errorBitmap[21] =
{
  0b11111110,
  0b10001001,
  0b10001001,
  0b11111110,
  0b10001100,
  0b10001010,
  0b11111001,
  
  0b11101111,
  0b10011000,
  0b10011000,
  0b11101111,
  0b11001000,
  0b10101000,
  0b10011111,

  0b10011110,
  0b10011001,
  0b10011001,
  0b10011110,
  0b10011100,
  0b10011010,
  0b11111001
};

const byte YtLogoColors[3][3] = {{0, 0, 0}, {255, 0, 0}, {255, 255, 255}};
const byte YtLogo[8][10] = 
{
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
  {1, 1, 1, 1, 2, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 2, 2, 1, 1, 1, 1},
  {1, 1, 1, 1, 2, 2, 2, 1, 1, 1},
  {1, 1, 1, 1, 2, 2, 2, 1, 1, 1},
  {1, 1, 1, 1, 2, 2, 1, 1, 1, 1},
  {1, 1, 1, 1, 2, 1, 1, 1, 1, 1},
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 0}
};

const byte FbLogoColors[3][3] = {{0, 0, 0}, {40, 40, 125}, {255, 255, 255}};
const byte FbLogo[8][10] = 
{
  {0, 1, 1, 1, 1, 2, 2, 2, 1, 0},
  {1, 1, 1, 1, 2, 2, 2, 2, 1, 1},
  {1, 1, 1, 1, 2, 2, 1, 1, 1, 1},
  {1, 1, 1, 1, 2, 2, 1, 1, 1, 1},
  {1, 1, 1, 2, 2, 2, 2, 2, 1, 1},
  {1, 1, 1, 1, 2, 2, 1, 1, 1, 1},
  {1, 1, 1, 1, 2, 2, 1, 1, 1, 1},
  {0, 1, 1, 1, 2, 2, 1, 1, 1, 0}
};

const byte TwLogoColors[3][3] = {{0, 0, 0}, {40, 70, 125}, {255, 255, 255}};
const byte TwLogo[8][10] = 
{
  {0, 1, 1, 2, 2, 1, 1, 1, 1, 0},
  {1, 1, 1, 2, 2, 1, 1, 1, 1, 1},
  {1, 1, 1, 2, 2, 2, 2, 2, 1, 1},
  {1, 1, 1, 2, 2, 2, 2, 2, 1, 1},
  {1, 1, 1, 2, 2, 1, 1, 1, 1, 1},
  {1, 1, 1, 2, 2, 1, 1, 1, 1, 1},
  {1, 1, 1, 2, 2, 2, 2, 2, 1, 1},
  {0, 1, 1, 1, 2, 2, 2, 2, 1, 0}
};

const byte InstaLogoColors[8][3] = {{0, 0, 0}, {255, 255, 255}, {255, 190, 0}, {205, 80, 0}, {80, 10, 10}, {175, 0, 170}, {45, 0, 85}, {40, 20, 180}};
const byte InstaLogo[8][10] = 
{
  {0, 5, 1, 1, 1, 1, 1, 1, 7, 0},
  {4, 1, 5, 5, 6, 6, 6, 1, 1, 7},
  {4, 1, 4, 5, 1, 1, 6, 6, 1, 7},
  {4, 1, 4, 1, 4, 6, 1, 6, 1, 7},
  {3, 1, 3, 1, 4, 5, 1, 6, 1, 7},
  {3, 1, 3, 3, 1, 1, 4, 5, 1, 6},
  {2, 1, 2, 2, 3, 3, 4, 4, 1, 5},
  {0, 2, 1, 1, 1, 1, 1, 1, 4, 0}
};

const byte TwitchLogoColors[3][3] = {{0, 0, 0}, {255, 255, 255}, {100, 39, 135}};
const byte TwitchLogo[8][10] = 
{
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 2, 2, 2, 2, 2, 2, 2, 2, 1},
  {1, 2, 2, 2, 2, 2, 2, 2, 2, 1},
  {1, 2, 2, 2, 1, 2, 1, 2, 2, 1},
  {1, 2, 2, 2, 1, 2, 1, 2, 2, 1},
  {1, 2, 2, 2, 2, 2, 2, 2, 1, 0},
  {1, 1, 1, 2, 1, 1, 1, 1, 0, 0},
  {0, 0, 0, 1, 0, 0, 0, 0, 0, 0}
};


long power(long base, long exponent)
{
  long result = base;
  
  for(long i = 0; i <= exponent; i++)
  {
    if(i == 0)
    {
      result = 1;
    }
    else
    {
      result = result * base;
    }
  }

  return result;
}

int getDigit(unsigned long number, int index)
{
    for (int i = 0; i < index - 1; i++) { 
      number /= 10; 
    }
    return number % 10;
}

void refreshDisplay()
{
  for(int p = 0; p < pannelWidth / 8; p++)
  {
    for (int i = 0; i < 8; i++)
    {
      for(int j = 0; j < 8; j++)
      {
        bande.setPixelColor(p * 64 + i * 8 + j,pannel[i][p * 8 + j][0], pannel[i][p * 8 + j][1], pannel[i][p * 8 + j][2]);
      }
    }
  }

  bande.show();
}

void setAreaColor(int xStart, int xEnd, int yStart, int yEnd, char rValue, char gValue, char bValue)
{
  for(int i = yStart; i <= yEnd; i++)
  {
    for(int j = xStart; j <= xEnd; j++)
    {
      pannel[i][j][0] = rValue;
      pannel[i][j][1] = gValue;
      pannel[i][j][2] = bValue;
    }
  }
}

void printDigit(char digit, int xPos, int yPos, char rValue, char gValue, char bValue, int shift = 0)
{
  int subIndex = digit / 5;
  int index = digit - subIndex * 5;

  for(int i = 0; i < 7 - abs(shift); i++)
  {
    for(int j = 0; j < 4; j++)
    {
      if(xPos + j < pannelWidth && yPos + i < pannelHeight)
      {
        if(bitRead(digits[index * 7 + i + (shift > 0 ? shift : 0)], 4 * (1 - subIndex) + 3 - j))
        {
          pannel[yPos + i][xPos + j][0] = rValue;
          pannel[yPos + i][xPos + j][1] = gValue;
          pannel[yPos + i][xPos + j][2] = bValue;
        }
      }
    }
  }
}

void printNumber(unsigned long number, int xPos, int yPos, char rValue, char gValue, char bValue)
{
  for(int i = 0; i < digitAmount; i++)
  {
    printDigit(getDigit(number, digitAmount - i), xPos + i * 5, yPos, rValue, gValue, bValue);
  }
}

void printNumberWithAnimation(unsigned long number, int xPos, int yPos, char rValue, char gValue, char bValue, int inOutAnim = 0)
{
  float startNumber = previousNumber;
  float stepDelta = ((float) number - previousNumber) / 3.0;
  
  for(int animStep = 0; animStep < (inOutAnim == 0 ? 3 : 1); animStep++)
  {
    if(inOutAnim == 0)
    {
      number = round(((float) startNumber) + stepDelta * (animStep + 1));
    }
    
    for(int a = 0; a < 8; a++)
    //for(int a = 7; a >= 0; a--)
    {
      setAreaColor(xPos, xPos + (digitAmount * 5 - 2), yPos, yPos + 7, 0, 0, 0);
      
      bool firstDigitAppeared = false;

      for(int i = 0; i < digitAmount; i++)
      {
        int newDigit = getDigit(number, digitAmount - i);

        if(newDigit != 0)
        {
          firstDigitAppeared = true;
        }
        else if(!showLeftZeros && !firstDigitAppeared && i != digitAmount - 1)
        {
          continue;
        }

        int previousDigit = getDigit(previousNumber, digitAmount - i);
        int delta = newDigit - previousDigit;
        bool upShift = stepDelta > 0;
        
        if(delta == 0 && inOutAnim == 0)
        {
          printDigit(newDigit, xPos + i * 5, yPos + 1, rValue, gValue, bValue);
        }
        else
        {
          if(inOutAnim != 0)
          {
            upShift = false;
          }
          
          int previousDigitShift = upShift ? a : -a;
          int newDigitShift = upShift ? a - 7 : 7 - a;

          int previousDigitYPos = upShift ? yPos : yPos + a + 2;
          int newDigitYPos = upShift ? yPos + 1 + 7 - a : yPos + 1;

          if(inOutAnim != 1)
          {
            printDigit(previousDigit, xPos + i * 5, previousDigitYPos, rValue, gValue, bValue, previousDigitShift);
          }
          
          if(inOutAnim != -1)
          {
            printDigit(newDigit, xPos + i * 5, newDigitYPos, rValue, gValue, bValue, newDigitShift);
          }
        }
      }
      refreshDisplay();
      
      delay(50);
    }

    previousNumber = number;
  }
}

void printError(int xPos, int yPos)
{
  setAreaColor(xPos, xPos + (digitAmount * 5 - 2), yPos, yPos + 7, 0, 0, 0);
  
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 7; j++)
    {
      for(int k = 0; k < 8; k++)
      {
        int offset = k < 4 ? k : k + 1;
        offset += i * 10;

        if(bitRead(errorBitmap[i * 7 + j], 7 - k))
        {
          pannel[yPos + j + 1][xPos + offset][0] = 255;
          pannel[yPos + j + 1][xPos + offset][1] = 255;
          pannel[yPos + j + 1][xPos + offset][2] = 255;
        }
      }
    }
  }
  refreshDisplay();
}

void serialPrintPannel()
{
  for(int i = 0; i < pannelHeight; i++)
  {
    for(int j = 0; j < pannelWidth; j++)
    {
      Serial.print(String(pannel[i][j][0]));
      Serial.print("\t");
      Serial.print(String(pannel[i][j][1]));
      Serial.print("\t");
      Serial.print(String(pannel[i][j][2]));
      Serial.print("\t\t");
    }
    Serial.print("\n");
  }
}

void printLogo(const byte logo[][10], const byte colors[][3], int animation = 4)
{
  for(int i = 0; i < 8; i++)
  {
    for(int j = 4 - animation; j < 6 + animation; j++)
    {
      for(int k = 0; k < 3; k++)
      {
        pannel[i][j][k] = colors[logo[i][j]][k];
      }
    }
  }
}

void printLogoWithAnimation(const byte logo[][10], const byte colors[][3])
{
  for(int i = 0; i < 5; i++)
  {
    printLogo(logo, colors, i);
    refreshDisplay();
    delay(50);
  }
}

String sendGet(String url, String sslCertificateFootprint = "")
{
  HTTPClient http;

  if(sslCertificateFootprint == "")
  {
    http.begin(url);
  }
  else
  {
    http.begin(url, sslCertificateFootprint);
  }
  
  int httpCode = http.GET();

  String returnValue;

  returnValue = httpCode > 0 ? http.getString() : "Error : no HTTP code";

  http.end();;
  return returnValue;
}

int getTwitterFollowerCount(String profileId)
{
  String answer = sendGet("http://cdn.syndication.twimg.com/widgets/followbutton/info.json?screen_names=" + profileId);

  // On enlève les crochets [] qui entourent le json et le rendent illisible par le pasrer
  answer.remove(0, 1);
  answer.remove(answer.length() - 1, 1);
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(answer);

  if (root.success()) {
    if (root.containsKey("followers_count"))
    {
      return root["followers_count"].as<int>();
    }
    
    Serial.println("Incompatible JSON");
  }
  else
  {
    Serial.println("Failed to parse JSON");
  }
  
  return -1;
}

int getInstagramFollowerCount(String pageName)
{
  InstagramUserStats response = instaStats.getUserStats(pageName);
  return response.followedByCount;
}

int getYoutubeSubscriberCount(String channelId)
{
  if(youtubeApi.getChannelStatistics(youtubeChannelId))
  {
    return youtubeApi.channelStats.subscriberCount;
  }
  else
  {
    return -1;
  }
}

int getFacebookFanCount(String pageId)
{
  return facebookApi.getPageFanCount(pageId);
}

int getTwitchFollowerCount()
{
  UserData user = twitch.getUserData(twitchUsername);
  FollowerData followerData = twitch.getFollowerData(user.id);
  if(!followerData.error)
  {
    return followerData.total;
  }
  else
  {
    return -1;
  }
}

void printMediaSettings()
{
  for(int media = 0; media < mediaCount; media++)
  {
    Serial.print(mediaName[media]);
    Serial.print(" - Enabled : ");
    Serial.print(mediaEnabled[media]);
    Serial.print(" - Duration : ");
    Serial.println(mediaDuration[media]);
  }
}

void EEPROMWriteInt(int address, int value)
{
  byte lowByte = ((value >> 0) & 0xFF);
  byte highByte = ((value >> 8) & 0xFF);

  EEPROM.write(address, lowByte);
  EEPROM.write(address + 1, highByte);
}

unsigned int EEPROMReadInt(int address)
{
  byte lowByte = EEPROM.read(address);
  byte highByte = EEPROM.read(address + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void readSettingsFromEeprom()
{
  if(EEPROM.read(0) != eepromCheckValue)
  {
    Serial.println("Bad EEPROM format. Settings read abort.");
    return;
  }
  else
  {
    for(int media = 0; media < mediaCount; media++)
    {
      int offset = 100 + media * 5;
      mediaEnabled[media] = EEPROM.read(offset) == 1 ? true : false;
      mediaDuration[media] = EEPROMReadInt(offset + 1);
    }

    brightness = EEPROM.read(1);
  }
}

void writeMediaSettingsToEeprom(int media)
{
  int offset = 100 + media * 5;
  EEPROM.write(offset, mediaEnabled[media] ? 1 : 0);
  EEPROMWriteInt(offset + 1, mediaDuration[media]);
  EEPROM.commit();
}

void writeBrightnessSettingToEeprom()
{
  EEPROM.write(1, brightness);
  EEPROM.commit();
}

void writeSettingsToEeprom()
{
  for(int media = 0; media < mediaCount; media++)
  {
    writeMediaSettingsToEeprom(media);
  }

  writeBrightnessSettingToEeprom();

  EEPROM.write(0, eepromCheckValue);

  EEPROM.commit();
}

String generateIndexHtml(String message = "")
{
  String html = "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n<title>Follower Counter</title>\n";
  html += "</head>\n<body>\n";
  html += message + "\n<br>\n";
  html += "<form action=\"/index\" method=\"get\">Brightness % <input type=\"number\" min=\"0\" max=\"100\" name=\"brightness\" value=\"" + String(brightness) + "\"><input type=\"submit\" value=\"Apply\"></form>";
  html += "<table class=\"table\">\n";
  html += "<tr><td>Media</td><td>Enabled</td><td>Duration</td></tr>";

  for(int media = 0; media < mediaCount; media++)
  {
    html += "<tr>\n<form action=\"/index\" method=\"get\">\n<input type=\"hidden\" name=\"media\" value=\"" + String(media) + "\">\n";
    html += "<td>" + mediaName[media] + "</td>\n";
    html += "<td><input type=\"hidden\" name=\"enabled\" value=\"" + String(mediaEnabled[media] ? "true" : "false") + "\" checked>";
    html += "<input type=\"checkbox\" onclick=\"this.previousSibling.value = !(this.previousSibling.value == 'true')\" " + String(mediaEnabled[media] ? "checked" : "") + "></td>\n";
    html += "<td><input type=\"number\" name=\"duration\" value=\"" + String(mediaDuration[media]) + "\"></td>\n";
    html += "<td><input type=\"submit\" value=\"Apply\"></td>\n";
    html += "</form>\n</tr>\n";
  }

  html += "</table>\n";
  html += "<button onClick=\"window.location = window.location.pathname\">Refresh</button>";
  html += "\n</body>\n</html>";

  return html;
}

void handleIndex()
{
  if (!server.authenticate(webServerUsername, webServerPassword))
  {
    return server.requestAuthentication();
  }
  
  String message = "";
  if (server.arg("media") != "" && server.arg("enabled") != "")
  {
    int media = server.arg("media").toInt();
    bool enabled = server.arg("enabled") == "true";
    
    if(media >= 0 && media < mediaCount)
    {
      if(server.arg("duration") != "")
      {
        int duration = server.arg("duration").toInt();
        if(duration > 0)
        {
          mediaDuration[media] = duration;
          mediaEnabled[media] = enabled;

          writeMediaSettingsToEeprom(media);

          message = mediaName[media] + " settings changed.";
        }
        else
        {
          message = "Incorrect query";
        }
      }
    }
  }
  else if (server.arg("brightness") != "")
  {
    int newBrightness = server.arg("brightness").toInt();
    if(newBrightness >= 0 && newBrightness <= 100) // La vraie limite max de brightness est 255, mais le courant nécessaire devient trop élevée pour un port USB
    {
      brightness = newBrightness;

      bande.setBrightness(brightness);
      bande.show();

      writeBrightnessSettingToEeprom();

      message = "Brightness changed.";
    }
  }

  server.send(200, "text/html", generateIndexHtml(message));
}

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(wirelessHostname);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  
  ArduinoOTA.setPassword(otaPassword);
  ArduinoOTA.setHostname(wirelessHostname);
  
  ArduinoOTA.begin();

  server.on("/index", handleIndex);
  server.begin();

  pinMode(settingsResetPin, INPUT_PULLUP);
  pinMode(settingsResetGndPin, OUTPUT);

  digitalWrite(settingsResetGndPin, LOW);
  
  EEPROM.begin(512);

  if(EEPROM.read(0) == eepromCheckValue && digitalRead(settingsResetPin))
  {
    readSettingsFromEeprom();
  }
  else
  {
    for(int i = 0; i < mediaCount; i++)
    {
      mediaEnabled[i] = true;
    }

    writeSettingsToEeprom();
  }
  
  printMediaSettings();
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  bande.begin();

  bande.setBrightness(brightness);

  printLogoWithAnimation(YtLogo, YtLogoColors);

  printNumberWithAnimation(0, 11, 0, 255, 255, 255, 1);

  refreshDisplay();
}

void printMediaLogoWithAnimation(int media)
{
  if(media == 0)
  {
    printLogoWithAnimation(YtLogo, YtLogoColors);
  }
  else if(media == 1)
  {
    printLogoWithAnimation(TwLogo, TwLogoColors);
  }
  else if(media == 2)
  {
    printLogoWithAnimation(FbLogo, FbLogoColors);
  }
  else if(media == 3)
  {
    printLogoWithAnimation(InstaLogo, InstaLogoColors);
  }
  else if(media == 4)
  {
    printLogoWithAnimation(TwitchLogo, TwitchLogoColors);
  }

  return;
}

int getMediaValue(int media)
{
  int value = -1;

  if(mediaCallLimits[media] != 0)
  {
    unsigned long minTimeBetweenCalls = 3600000 / mediaCallLimits[media];
    if(millis() - mediaLastCallMillis[media] < minTimeBetweenCalls && firstCallDone[media])
    {
      value = mediaLastValue[media];
      Serial.print("Skipped ");
      Serial.print(mediaName[media]);
      Serial.println(" call for API restriction. Last known value used.");
      return value;
    }
  }

  Serial.print(mediaName[media]);
  Serial.println(" API call.");
  
  if(media == 0)
  {
    value = getYoutubeSubscriberCount(youtubeChannelId);
  }
  else if(media == 1)
  {
    value = getTwitterFollowerCount(twitterPageName);
  }
  else if(media == 2)
  {
    value = getFacebookFanCount(facebookPageId);
  }
  else if(media == 3)
  {
    value = getInstagramFollowerCount(instagramPageName);
  }
  else if(media == 4)
  {
    value = getTwitchFollowerCount();
  }

  firstCallDone[media] = true;
  mediaLastCallMillis[media] = millis();
  mediaLastValue[media] = value;
  
  return value;
}

void delayWithHandling(long ms)
{
  unsigned long delayStartMillis = millis();

  while(millis() - delayStartMillis < ms)
  {
    delay(50);
    ArduinoOTA.handle();
    server.handleClient();
  }
}

void loop()
{
  ArduinoOTA.handle();
  server.handleClient();

  for(int media = 0; media < mediaCount; media++)
  {
    if(!mediaEnabled[media])
    {
      continue; // Le média n'est pas activé
    }
    else
    {
      bool logoDisplayed = previousMedia == media;
      
      previousMedia = media;

      unsigned long mediaStartMillis = millis();
      unsigned long lastRefreshMillis = 0;
      
      while(millis() - mediaStartMillis < mediaDuration[media] * 1000)
      { 
        if(!mediaEnabled[media])
        {
          break; // Le média n'est pas activé
        }
        
        if(millis() - lastRefreshMillis >= refreshInterval)
        {
          lastRefreshMillis = millis();

          int value = getMediaValue(media);
          
          Serial.print(mediaName[media]);
          Serial.print(" : ");
          Serial.println(value);
          Serial.println();
  
          if(!logoDisplayed)
          {
            printMediaLogoWithAnimation(media);
            
          }
            
          if(value != -1)
          {
            printNumberWithAnimation(value, 11, 0, 255, 255, 255, !logoDisplayed);
          }
          else
          {
            printError(11, 0);
          }
          
          logoDisplayed = true;
  
          previousNumber = value;
  
          refreshDisplay();
        }
  
        delayWithHandling(50);
      }
    }
  }
}
