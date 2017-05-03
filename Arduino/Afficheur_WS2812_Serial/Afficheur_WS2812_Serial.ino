#include <Adafruit_NeoPixel.h>

const int brocheLEDs = 2;
const int nombreLEDs = 64 * 5;
const int luminosite = 5;

const int pannelHeight = 8;
const int pannelWidth = 40;

byte pannel[pannelHeight][pannelWidth][3];

byte previousMedia = 0;
long previousNumber = 0;

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

// On crée une instance de NeoPixel
Adafruit_NeoPixel bande = Adafruit_NeoPixel(nombreLEDs, brocheLEDs, NEO_GRB + NEO_KHZ800);

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

int getDigit(long number, int index)
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

void print6DigitsNumber(long number, int xPos, int yPos, char rValue, char gValue, char bValue)
{
  for(int i = 0; i < 6; i++)
  {
    printDigit(getDigit(number, 6 - i), xPos + i * 5, yPos, rValue, gValue, bValue);
  }
}

void print6DigitsNumberWithAnimation(long number, int xPos, int yPos, char rValue, char gValue, char bValue, int inOutAnim = 0)
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
      setAreaColor(xPos, xPos + 28, yPos, yPos + 7, 0, 0, 0);
      
      for(int i = 0; i < 6; i++)
      {
        int newDigit = getDigit(number, 6 - i);
        int previousDigit = getDigit(previousNumber, 6 - i);
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

void printLogo(byte logo[][10], byte colors[][3], int animation = 4)
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

void printLogoWithAnimation(byte logo[][10], byte colors[][3])
{
  for(int i = 0; i < 5; i++)
  {
    printLogo(logo, colors, i);
    refreshDisplay();
    delay(50);
  }
}

void setup()
{
  Serial.begin(115200);

  //randomSeed(analogRead(0));

  bande.begin();

  bande.setBrightness(luminosite);

  printLogoWithAnimation(YtLogo, YtLogoColors);

  print6DigitsNumberWithAnimation(0, 11, 0, 255, 255, 255, 1);

  refreshDisplay();
}

void loop()
{
  if(Serial.available() > 4)
  {
    byte media = Serial.read();

    long value = 0;

    for(int i = 0; i < 4; i++)
    {
      value += ((long)Serial.read()) * (power(2, i * 8));
    }

    if(media != previousMedia)
    {
      print6DigitsNumberWithAnimation(value, 11, 0, 255, 255, 255, -1);
      
      if(media == 1)
      {
        printLogoWithAnimation(YtLogo, YtLogoColors);
      }
      else if(media == 2)
      {
        printLogoWithAnimation(TwLogo, TwLogoColors);
      }
      else if(media == 3)
      {
        printLogoWithAnimation(FbLogo, FbLogoColors);
      }

      print6DigitsNumberWithAnimation(value, 11, 0, 255, 255, 255, 1);
    }
    else
    {
      print6DigitsNumberWithAnimation(value, 11, 0, 255, 255, 255);
    }

    previousMedia = media;
    previousNumber = value;
    
    refreshDisplay();
  }
}
