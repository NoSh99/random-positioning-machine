// Adjust this before wiring

#define LCD_RS 3
#define LCD_CE 4
#define LCD_DC 5 // ****
#define LCD_Din 12 // ***
#define LCD_CLK 13
#define LCD_BL 7

#define Large_Dir 10
#define Large_Pul 11 // ***
#define Small_Dir 8 // ****
#define Small_Pul 9

#define pulse_per_rev 400 //

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <AccelStepper.h>

// ======================== MENU VARIABLE =====================
int menuItem = 1;
int lastMenuItem = 1;
int frame = 1;
int page = 1;
int contrast = 60;

String menuItem1 = "Speed (RPM)";
String menuItem2 = "Time (sec)";
String menuItem3 = "2D - Clinostat";
String menuItem4 = "3D - Clinostat";
String menuItem5 = "Random Mode";
String menuItem6 = "Reset";

int adjustable_speed = 0; // RPM, negative means CCW, only for Clinostat Mode
int time_for_random = 1;  // second, only for random mode, SHOULD CONSIDER AT LEAST 3 or 4 SECOND

bool turn_ccw = false;
bool turn_cw = false;
bool press_button = false;

// ======================= MOTOR OPERATING VARIABLE =============
const float one_rpm = pulse_per_rev / 60;
const int min_rpm_random_mode = 20;
const int max_rpm_random_mode = 120;

bool start_random_large = true;
bool start_random_small = true;

int random_time_large, random_time_small;
int speed_large, speed_small;
int random_negative_large, random_negative_small; // use to indicate the direction of the motor, (-) means ccw and (+) means cw
int time_range_for_random_mode;

long random_num_large, random_num_small;
unsigned long current_time, delta_time_large, delta_time_small;

// CALL //
ClickEncoder *encoder;
int16_t last, value;

Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_DC, LCD_CE, LCD_RS);

AccelStepper stepperLarge(1, Large_Dir, Large_Pul);
AccelStepper stepperSmall(1, Small_Dir, Small_Pul);

// ==================== SETUP ==================
void setup()
{
  //
  pinMode(LCD_BL, OUTPUT);

  encoder = new ClickEncoder(A1, A0, A2);
  encoder->setAccelerationEnabled(false);

  display.begin();
  display.clearDisplay();
  setContrast();

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  //   Serial.begin(9600);

  last = encoder->getValue();

  stepperLarge.setMaxSpeed(2000);
  stepperLarge.setAcceleration(2000);

  stepperSmall.setMaxSpeed(2000);
  stepperSmall.setAcceleration(2000);
}

// ===================== LOOP ====================
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
          press_button = true;
          Serial.print('B');
        break;
      }
    }
    Serial.print(menuItem); 
//    Serial.print(' '); Serial.print(frame); 
    Serial.println(page);
    operateMenu();
}

// ===================== FOR MOTOR AND MODE ===============
// ========================================================

void randomMode()
{

  // might need to do something to reset the speed when switching between modes
  time_range_for_random_mode = time_for_random; // time belongs to what we input through the encoder.
  current_time = millis();
  if (start_random_large == true)
  {
    random_negative_large = -1 + rand() % 3; // random between 3 number starting from -1, means -1, 0, 1
    random_num_large = random_negative_large * rand();
    random_time_large = (abs(random_num_large % time_range_for_random_mode) + 1); // plus 1 in case it return 0;

        speed_large = (random_num_large % max_rpm_random_mode);

            if (speed_large > -1 * min_rpm_random_mode && speed_large <= 0)
                speed_large = -1 * min_rpm_random_mode;
    if (speed_large > 0 && speed_large < min_rpm_random_mode)
      speed_large = min_rpm_random_mode;

    stepperLarge.setSpeed(speed_large * one_rpm);
  }

  if (start_random_small == true)
  {
    random_negative_small = -1 + rand() % 3; // random between 3 number starting from -1, means -1, 0, 1
    random_num_small = random_negative_small * rand();
    random_time_small = (abs(random_num_small % time_range_for_random_mode) + 1); // plus 1 in case it return 0;

        speed_small = (random_num_small % max_rpm_random_mode); 
        if (speed_small > -1 * min_rpm_random_mode && speed_small < 0)
            speed_small = -1 * min_rpm_random_mode;
    if (speed_small >= 0 && speed_small < min_rpm_random_mode)
      speed_small = min_rpm_random_mode;

    stepperSmall.setSpeed(speed_small * one_rpm);
  }

  stepperLarge.runSpeed();
  stepperSmall.runSpeed();

  if (current_time - delta_time_large > random_time_large * 1000) // milisec to sec
  {
    start_random_large = true;
    delta_time_large = current_time;
  }
  else
    start_random_large = false;

  if (current_time - delta_time_small > random_time_small * 1000)
  {
    start_random_small = true;
    delta_time_small = current_time;
  }
  else
    start_random_small = false;
}

void clinostatThreeD()
{
  speed_large = adjustable_speed;
  speed_small = adjustable_speed;

  stepperLarge.runSpeed();
  stepperSmall.runSpeed();
}

void clinostatTwoD()
{
  // might need to have to drive Large Motor to suitable position (parrallel with horizontal axis/the floor)
  speed_small = adjustable_speed;

  stepperSmall.runSpeed();
}

void defaults()
{

}

// ======================= FOR MENU =========================
// ==========================================================

void drawMenu()
{
  if (page == 1)
  {
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(15, 0);
    display.print("MAIN MENU");
    display.drawFastHLine(0, 10, 83, BLACK);

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
    displayIntMenuPage(menuItem1, adjustable_speed);
  }
  else if (page == 2 && menuItem == 2)
  {
    displayIntMenuPage(menuItem2, time_for_random);
  }
  //  else if (page == 2 && menuItem == 3)
  //  {
  //    // consider adding what type of string????
  //  }
  //  else if (page == 2 && menuItem == 4)
  //  {
  //
  //  }
   else if (page == 2 && menuItem == 5)
   {
     randomMode();
     displayStringMenuPage(menuItem5, speed_large);
   }
  //  else if (page == 2 && menuItem == 6)
  //  {
  //
  //  }
}

void operateMenu()
{

  // this part is to rotate counter clockwise
  if (turn_ccw && page == 1)
  {
    turn_ccw = false;
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

  else if (turn_ccw && page == 2 && menuItem == 1)
  {
    turn_ccw = false;
    adjustable_speed -= 5; // note, consider negative num cases, remember to account for the set maximum speed.
  }

  else if (turn_ccw && page == 2 && menuItem == 2)
  {
    turn_ccw = false;
    time_for_random--;
    if (time_for_random <= 0)
    {
      time_for_random = 1;
    }
  }

  //
  if (turn_cw && page == 1)
  {
    turn_cw = false;
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

  else if (turn_cw && page == 2 && menuItem == 1)
  {
    turn_cw = false;
    adjustable_speed += 5; // note, consider negative num cases
  }

  else if (turn_cw && page == 2 && menuItem == 2)
  {
    turn_cw = false;
    time_for_random++;
    // - add a max variable of time, maybe no need.
    // if (time_for_random >= )
    // {
    //   time_for_random = ;
    // }
    
  }

  // when pressing the press_button button, if you are in page 1, you will go direct to page 2
  // and when you are in page 2, you will return to page 1
  if (press_button)
  {
    // Serial.print("here");
    press_button = false;

    if (page == 1 && menuItem == 3)
    {
      clinostatTwoD();
      page = 2;
    }
    else if (page == 2 && menuItem == 3)
      page = 1;

    if (page == 1 && menuItem == 4)
    {
      clinostatThreeD();
      page = 2;
    }
    else if (page == 2 && menuItem == 4)
      page = 1;

    if (page == 1 && menuItem == 5)
    {
      randomMode();
      page = 2;
    }
    else if (page == 2 && menuItem == 5)
      page = 1;

    if (page == 1 && menuItem == 6)
    {
      defaults();
    }
    else if (page == 1 && menuItem < 3)
    {
      page = 2;
    }
    else if (page == 2 && menuItem < 3)
    {
      page = 1;
    }
  }
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
  display.drawFastHLine(0, 10, 83, BLACK);
  display.setCursor(5, 15);
  display.print("Value");
  display.setTextSize(2);
  display.setCursor(5, 25);
  display.print(value);
  display.setTextSize(2);
  display.display();
}

void displayStringMenuPage(String menuItem, String value)
{
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(15, 0);
    display.print(menuItem);
    display.drawFastHLine(0,10,83,BLACK);
    display.setCursor(0, 15);
    display.print("Large RPM:");
    display.setTextSize(1);
    display.setCursor(25, 15);
    display.print(value);
    display.setTextSize(1);
    display.display();
}

void displayMenuItem(String item, int position, boolean selected)
{
  if (selected)
  {
    display.setTextColor(WHITE, BLACK);
  }
  else
  {
    display.setTextColor(BLACK, WHITE);
  }
  display.setCursor(0, position);
  display.print(">" + item);
}

void setContrast()
{
  display.setContrast(contrast);
  display.display();
}

void readRotaryEncoder()
{
  value += encoder->getValue();

  if (value / 2 > last)
  {
    last = value / 2;
    turn_cw = true;
    delay(150);
  }
  else if (value / 2 < last)
  {
    last = value / 2;
    turn_ccw = true;
    delay(150);
  }
}