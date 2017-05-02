QSM_lcd_prefSet prefSet_DARK(ILI9341_BLACK, ILI9341_WHITE, ILI9341_WHITE, ILI9341_YELLOW, ILI9341_RED);
QSM_lcd_prefSet prefSet_LITE(ILI9341_WHITE, ILI9341_NAVY, ILI9341_BLACK, ILI9341_DARKGREEN, ILI9341_RED);
QSM_lcd_prefSet prefSet_CONT(ILI9341_CYAN, ILI9341_BLACK, ILI9341_MAROON, ILI9341_PURPLE, ILI9341_RED);

void QSM_lcd::attachData(QSM_rawDataPacket *rawDataMemAddress, QSM_lcd_prefSet *set_prefSet)
{
  Serial.println("Attaching LCD to System data");
  rawDataPointer = rawDataMemAddress;
  Serial.println("Aquiring LCD preference set");
  prefSet = set_prefSet;
}

void QSM_lcd::init(enum SCREEN screenLayout = MAIN)
{
  Serial.println("Initializing screen");
  begin();
  invertDisplay(false);
  fillScreen(ILI9341_BLACK);
  setRotation(1);
  //procData = {0};
  startupScreen();
  setScreen(MAIN);
}

void QSM_lcd::setScreen(enum SCREEN screenLayout)
{
  activeScreen = screenLayout;
  
  switch(screenLayout)
  {
    case MAIN : screen_main_load();
    case TEST : screen_test_load();
  }
}

void QSM_lcd::screen_main_load()
{
  //initiator
  Serial.println("LCD: Main Screen: Starting");
  setTextSize(2);
  setTextColor(ILI9341_WHITE);
  fillScreen(prefSet->background);
  //print_header_message("LCD INITITED", ILI9341_RED, 4);

  //title
  Serial.println("LCD: Main Screen: Loading header...");
  setCursor(third_x/3, third_y/6);
  //print("QSM VEHICLE");

  //timers
  Serial.println("LCD: Main Screen: Loading timers...");
  setCursor(third_x/6-10, 2*third_y/3-5);
  print("LAP");
  drawLine(0, 2*third_y/3+14, lcd_width/3, 2*third_y/3+14, lcd_border);
  setCursor(third_x/6-10, 2*third_y/3+20);
  print("TOT");
  drawLine(0, 2*third_y/3+39, lcd_width/3, 2*third_y/3+39, lcd_border);
  setCursor(third_x/6-10, 2*third_y/3+45);
  print("AVG");

  
  //speedometer 
  Serial.println("LCD: Main Screen: Loading speedometer...");
  drawRect(0, lcd_height/2, lcd_width/2+1, third_y+20, lcd_border);
  setCursor(105, 180);
  print("km.h");

  
  //inclinometer, lap odometer
  Serial.println("LCD: Main Screen: Loading inclinometer...");
  drawRect(lcd_width/2, lcd_height/2, 2*third_x-1, third_y, lcd_border);
  drawLine(lcd_width/2, (lcd_height+third_y)/2, lcd_width-1, (lcd_height+third_y)/2, lcd_border);

  drawLine(0, 41, lcd_width, 41, lcd_border);
  setCursor(third_x+60, lcd_height/2 + 15);
  print("INCL.");
  setCursor(third_x+60, lcd_height/2 + 55);
  print("L-ODO"); 

  //lapCounter
  Serial.println("LCD: Main Screen: Loading lap counter..");
  drawRect(240, 41, 80, 80, lcd_border);
  setCursor(lcd_width-40, third_y+22);
  print("/10");

  setTextColor(lcd_text_title);
  
  drawSpeedometer(20000);
  Serial.println("Main Screen Load complete");
}

void QSM_lcd::refresh()
{
  //print_header_message("Screen Refresh Called", ILI9341_BLACK, 2);
  rawDataHandler();
  setTextColor(ILI9341_WHITE, ILI9341_BLACK);

  
  if (activeScreen == MAIN)
  {
    //timers
    setTextSize(2);
//    setCursor(third_x/2+10, 2*third_y/3-5);
//    printTime_ClockFormat(1000);
//    setCursor(third_x/2+10, 2*third_y/3+20);
//    printTime_ClockFormat(5600);
//    setCursor(third_x/2+10, 2*third_y/3+45);
//    printTime_ClockFormat(2500);

    setCursor(third_x/2+10, 2*third_y/3-5);
    printTime_ClockFormat(rawDataPointer->currentLapTime);
    setCursor(third_x/2+10, 2*third_y/3+20);
    printTime_ClockFormat((rawDataPointer->totalTime)/1000);
    setCursor(third_x/2+10, 2*third_y/3+45);
    print(procData.lapTime_average);

    //Speedometer
    setCursor(third_x/3-28, 6*third_y/3-25);
    setTextSize(5);
    print((float)(rawDataPointer->speed_mph)/1000);

    //Incline
    setCursor(2*third_x+20, lcd_height/2 + 15);
    setTextSize(2);
    //print(procData.incline_percent);
    print(3);
    print(char(asciiCode_degree));

    //Lap Odometer
    setCursor(2*third_x+20, lcd_height/2 + 55);
    print(3000);
    //print(rawDataPointer->dist_m_lap);
    print("m");

    //Lap Counter
    setCursor(lcd_width-60, 50);
    setTextSize(6);
    //print(2);
    print(rawDataPointer->lapCount);

    drawSpeedometer(rawDataPointer->speed_mph);
  }
  
}

void QSM_lcd::screen_test_load()
{
  
}

void QSM_lcd::startupScreen()
{
  Serial.println("Running startup screen");
  fillScreen(ILI9341_BLACK);
  fillRect(0, 0, lcd_width/3, lcd_height, ILI9341_BLUE);
  fillRect(lcd_width/3, 0, 2*lcd_width/3, lcd_height, ILI9341_YELLOW);
  fillRect(2*lcd_width/3, 0, lcd_width, lcd_height, ILI9341_RED);

  setTextColor(ILI9341_BLACK);
  setTextSize(8);
  //setFont(&FreeSerif12pt7b);
  setCursor(third_x/2-15, lcd_height/2-20);
  print("Q");
  setCursor(lcd_width/2-15, lcd_height/2-20);
  print("S");
  setCursor(5*third_x/2-15, lcd_height/2-20); 
  print("M");
  delay(2500);
  setFont();
}

void QSM_lcd::drawSpeedometer(unsigned int speed_mph)
{
  int width_max = 314;              //total width of gas guage
  int width_norm = 250; //width of gas guage before it turns red
  int pos_X = 0;    //rect top left x pixel coords
  int pos_Y = 220;  //rect top left y pixel coords   
  
  int height = 20;  //height of gas guage rectange

   int STEP = map(speed_mph, 0, 60000, 0, 314);  //maps 0 to 35 km/h
  
  
  //fillRect(pos_X, pos_Y, width_max, height, ILI9341_BLACK);
  
  if (STEP<width_norm)
    fillRect(pos_X, pos_Y, STEP, height, ILI9341_GREEN);
  else if (STEP>width_norm)
  {
    fillRect(pos_X, pos_Y, width_norm, height, ILI9341_GREEN);
    fillRect(width_norm+1, pos_Y, STEP-width_norm, height, ILI9341_RED);
  }
  
  fillRect(STEP+2, pos_Y, width_max-STEP, height, ILI9341_BLACK);
}

void QSM_lcd::rawDataHandler()
{
  //print_header_message("LCD data Handler Called", ILI9341_BLACK, 2);
  procData.lapTime_average = proc.calcAvgLapTime(rawDataPointer->lapTimes, rawDataPointer->lapCount);
  procData.kmph = (float)rawDataPointer->speed_mph/1000;
  procData.incline_percent = proc.calcInclinePercent(rawDataPointer->accel_out);
}

void QSM_lcd::printTime_ClockFormat(unsigned int ms)
{
   
  int* clk;
  clk = proc.convert_ms_clock(ms);
  
  print(clk[0]);
  print(":");
  print(clk[1]);
  print(":");
  print(clk[2]);
  
}

void QSM_lcd::print_header_message(char* message, uint16_t background_colour, uint8_t tmp_text_size, char* message2 = "")
{
  uint8_t init_text_size = textsize;
  uint16_t init_text_colour = textcolor;
  
  setTextSize(tmp_text_size);
  setTextColor(ILI9341_WHITE);
  
  fillRect(0, 0, lcd_width-1, 40, background_colour);
  setCursor((lcd_width/2) - (3*tmp_text_size*strlen(message)), 12); //to center the message, 4 bytes to char divided by 2
  print(message);
  //check if there is a second message
  if(message2 != "")
  {
    delay(100);
    print(message2);
    delay(100);
  }
  
  setTextSize(init_text_size);
  setTextColor(init_text_colour);
}
