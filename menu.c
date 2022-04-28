#define LCD_RS 3
#define LCD_CE 4
#define LCD_DC 5
#define LCD_Din 11
#define LCD_CLK 13
#define LCD_BL 7

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <ClickEncoder.h>
#include <TimerOne.h>



int menuItem = 1;
int lastMenuItem = 1;
int frame = 1;
int page = 1;
int contrast=60;

String menuItem1 = "Speed (RPM)";
String menuItem2 = "Time (sec)";
String menuItem3 = "Mode1";
String menuItem4 = "Mode2";
String menuItem5 = "Mode3";
String menuItem6 = "Reset";

int speeds = 0; // RPM, negative means CCW, should I display the direction
int timeForRandom = 1; // second

// inside each mode will be some explaination about that mode
// ??? should I include measuring method, or some sort of feedback the actual speed, base on accelerometer

boolean up = false;
boolean down = false;
boolean middle = false;

ClickEncoder *encoder;
int16_t last, value;

Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_DC, LCD_CE, LCD_RS);

void setup()
{
  //
  pinMode(7,OUTPUT);

  encoder = new ClickEncoder(A1, A0, A2);
  encoder->setAccelerationEnabled(false);

  display.begin();
  display.clearDisplay();
  setContrast();

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  Serial.begin(9600);
  
  last = encoder->getValue();
}


// -----------------------------------------------------
void loop()
{
  drawMenu();

  readRotaryEncoder();

  //might have to modify here to use for double layer :))
  ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open) {
    switch (b) 
      {
        case ClickEncoder::Clicked:
          middle = true;
          Serial.print('B');
        break;
      }
    }
    Serial.print(menuItem); 
//    Serial.print(' '); Serial.print(frame); 
    Serial.println(page);
    operateMenu();
}
// -----------------------------------------------------


void drawMenu()
{
  if (page == 1)
  {
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(15, 0);
    display.print("MAIN MENU");
    display.drawFastHLine(0,10,83,BLACK);

      if (menuItem == 1 && frame == 1)
      {
        displayMenuItem(menuItem1, 15, true);
        displayMenuItem(menuItem2, 25, false);
        displayMenuItem(menuItem3, 35, false);
      }
      else if (menuItem == 2 && frame == 1)
      {
        displayMenuItem(menuItem1, 15, false);
        displayMenuItem(menuItem2, 25, true);
        displayMenuItem(menuItem3, 35, false);
      }
      else if (menuItem == 3 && frame == 1)
      {
        displayMenuItem(menuItem1, 15, false);
        displayMenuItem(menuItem2, 25, false);
        displayMenuItem(menuItem3, 35, true);
      }
      else if (menuItem == 2 && frame == 2)
      {
        displayMenuItem(menuItem2, 15, true);
        displayMenuItem(menuItem3, 25, false);
        displayMenuItem(menuItem4, 35, false);
      }
      else if (menuItem == 3 && frame == 2)
      {
        displayMenuItem(menuItem2, 15, false);
        displayMenuItem(menuItem3, 25, true);
        displayMenuItem(menuItem4, 35, false);
      }
      else if (menuItem == 4 && frame == 2)
      {
        displayMenuItem(menuItem2, 15, false);
        displayMenuItem(menuItem3, 25, false);
        displayMenuItem(menuItem4, 35, true);
      }
      else if (menuItem == 3 && frame == 3)
      {
        displayMenuItem(menuItem3, 15, true);
        displayMenuItem(menuItem4, 25, false);
        displayMenuItem(menuItem5, 35, false);
      }
    else if (menuItem == 4 && frame == 3)
      {
        displayMenuItem(menuItem3, 15, false);
        displayMenuItem(menuItem4, 25, true);
        displayMenuItem(menuItem5, 35, false);
      }
      else if (menuItem == 5 && frame == 3)
      {
        displayMenuItem(menuItem3, 15, false);
        displayMenuItem(menuItem4, 25, false);
        displayMenuItem(menuItem5, 35, true);
      }
      else if (menuItem == 4 && frame == 4)
      {
        displayMenuItem(menuItem4, 15, true);
        displayMenuItem(menuItem5, 25, false);
        displayMenuItem(menuItem6, 35, false);
      }
    else if (menuItem == 5 && frame == 4)
      {
        displayMenuItem(menuItem4, 15, false);
        displayMenuItem(menuItem5, 25, true);
        displayMenuItem(menuItem6, 35, false);
      }
      else if (menuItem == 6 && frame == 4)
      {
        displayMenuItem(menuItem4, 15, false);
        displayMenuItem(menuItem5, 25, false);
        displayMenuItem(menuItem6, 35, true);
      }
      display.display();
  }

  else if (page == 2 && menuItem == 1)
  {
    displayIntMenuPage(menuItem1, speeds);
  }
  else if (page == 2 && menuItem == 2)
  {
    displayIntMenuPage(menuItem2, timeForRandom); 
  }
//  else if (page == 2 && menuItem == 3)
//  {
//    // consider adding what type of string????
//  }
//  else if (page == 2 && menuItem == 4)
//  {
//    
//  }
//  else if (page == 2 && menuItem == 5)
//  {
//    
//  }
}

void operateMenu()
{

  // this part is to rotate counter clockwise
  if (up && page == 1)
  {
    up = false;
    if (menuItem == 2 && frame == 2)
    {
      frame--;  
    }
    if (menuItem == 3 && frame == 3)
    {
      frame--;  
    }
    if (menuItem == 4 && frame == 4)
    {
      frame--;  
    }

    lastMenuItem = menuItem;
    menuItem--;

    if (menuItem == 0)
    {
      menuItem = 1;
    }
  }

  else if (up && page == 2 && menuItem == 1)
  {
    up = false;
    speeds -= 5; // note, consider negative num cases, remember to account for the set maximum speed.
  }

  else if (up && page == 2 && menuItem == 2)
  {
    up = false;
    timeForRandom--;
    if (timeForRandom <= 0)
    {
      timeForRandom = 1;
    }
  }

  // 
  if (down && page == 1)
  {
    down = false;
    if (menuItem == 3 && lastMenuItem == 2)
    {
      frame++;  
    }
    else if (menuItem == 4 && lastMenuItem == 3)
    {
      frame++;  
    }
    else if (menuItem == 5 && lastMenuItem == 4 && frame != 4)
    {
      frame++;  
    }

    lastMenuItem = menuItem;
    menuItem++;

    if (menuItem == 7)
    {
      menuItem--;
    }
  }

  else if (down && page == 2 && menuItem == 1)
  {
    down = false;
    speeds += 5; // note, consider negative num cases
  }

  else if (down && page == 2 && menuItem == 2)
  {
    down = false;
    timeForRandom++;
    /* - add a max variable of time, maybe no need.
    if (timeForRandom >= )
    {
      timeForRandom = ;
    }
    */
  }


// when pressing the middle button, if you are in page 1, you will go direct to page 2
  // and when you are in page 2, you will return to page 1
  if (middle)
  {
    Serial.print("here");
    middle = false;

    if (page == 1 && menuItem == 3)
    {
      mode1();
    }

    if (page == 1 && menuItem == 4)
    {
      mode2();
    }

    if (page == 1 && menuItem == 5)
    {
      mode3();
    }

    if (page == 1 && menuItem == 6)
    {
      defaults();
    }

    else if (page == 1 && menuItem < 3)
    {
      page = 2;
    }

    else if (page == 2)
    {
      page = 1;
    }
  }

}

void mode1()
{

}

void mode2()
{

}

void mode3()
{

}

void defaults()
{

}

void timerIsr() 
{
  encoder->service();
}

void displayIntMenuPage(String menuItem, int value)
{
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(15, 0);
    display.print(menuItem);
    display.drawFastHLine(0,10,83,BLACK);
    display.setCursor(5, 15);
    display.print("Value");
    display.setTextSize(2);
    display.setCursor(5, 25);
    display.print(value);
    display.setTextSize(2);
    display.display();
}

// might consider this one
void displayStringMenuPage(String menuItem, String value)
{
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(15, 0);
    display.print(menuItem);
    display.drawFastHLine(0,10,83,BLACK);
    display.setCursor(5, 15);
    display.print("Value");
    display.setTextSize(2);
    display.setCursor(5, 25);
    display.print(value);
    display.setTextSize(2);
    display.display();
}

void displayMenuItem(String item, int position, boolean selected)
{
    if(selected)
    {
      display.setTextColor(WHITE, BLACK);
    }
    else
    {
      display.setTextColor(BLACK, WHITE);
    }
    display.setCursor(0, position);
    display.print(">"+item);
}

  void setContrast()
  {
    display.setContrast(contrast);
    display.display();
  }

void readRotaryEncoder()
{
  value += encoder->getValue();
  
  if (value/2 > last) {
    last = value/2;
    down = true;
    delay(150);
  }else   if (value/2 < last) {
    last = value/2;
    up = true;
    delay(150);
  }
}