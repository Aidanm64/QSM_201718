#define tach_repCount 1
#define tach_timeStep 1
#define tach_reset_delay 500

QSM_sys::QSM_sys(enum OPERATING_MODE initMode = IDLE)
{
  currentOpMode = initMode;
  Serial.println("QSM_sys object created"); //debug

  InitializeData();
  AttachDevices();
  InitiateTimers();
  
  lcd->print_header_message("", ILI9341_BLACK, 1);
  while (1)
  {  
    runButtonHandler();
    //Serial.println(currentOpMode);
    //Serial.println(runTachometer());
    lcd->refresh();
    
    switch(currentOpMode)
    {
      case RACE : mode_race(); break;
      case IDLE : mode_idle(); break;
      case TEST : mode_test(); break;
    }
  }
}

void QSM_sys::InitializeData()
{
  data_inst = {0};
  data_inst_ptr = &data_inst;
}

void QSM_sys::AttachDevices()
{
  //Attach lcd
  lcd = new QSM_lcd(pin_tft_cs, pin_tft_dc);
  
  Serial.println("QSM_lcd object created");
  
  lcd->attachData(data_inst_ptr, &prefSet_DARK);
  lcd->init();
  lcd->print_header_message("LCD INITIATED", ILI9341_NAVY, 2);
  delay(200);

  setup_tachometer(pin_s_tachometer);
  lcd->print_header_message("TACH INITIATED", ILI9341_NAVY, 2);
  MPU_setup();
  lcd->print_header_message("MPU INITIATED", ILI9341_NAVY, 2);

  //Attach Buttons
  STRT_STP = new QSM_button(pin_STR_STP);
  lcd->print_header_message("Start/Stop button attached", ILI9341_DARKGREEN, 2);
  delay(100);
  RST = new QSM_button(pin_RST);
  lcd->print_header_message("Reset button attached", ILI9341_DARKGREEN, 2);
  delay(100);
  LAP = new QSM_button(pin_LAP);
  lcd->print_header_message("Lap Increment button attached", ILI9341_DARKGREEN, 2);
  delay(100);
  SELECT = new QSM_button(pin_SELECT);
  lcd->print_header_message("Select button attached", ILI9341_DARKGREEN, 2);
  delay(100);
  CANCEL = new QSM_button(pin_CANCEL);
  lcd->print_header_message("Cancel button attached", ILI9341_DARKGREEN, 2);
  delay(100);
}



void QSM_sys::InitiateTimers()
{
  globalTimer.start();
  raceTimer.reset();
  tachResetTimer.reset();
}
//void QSM_sys::set_opMode(enum OPERATING_MODE newOpMode)
//{
//  switch(newOpMode)
//  {
//    case RACE  
//  }
//}

void QSM_sys::mode_race()
{
  
}

void QSM_sys::mode_idle()
{
  
  data_inst.speed_mph = runTachometer();
  data_inst.totalTime = raceTimer.value();
  
  Serial.println(data_inst.speed_mph);
  Serial.println(lcd->rawDataPointer->speed_mph);
  //data_inst.accel_out = runMPU();
}

void QSM_sys::mode_test()
{
}

QSM_rawDataPacket QSM_sys::updateSystemData()
{
  Serial.println("Getting total time");
  data_inst.totalTime = raceTimer.value();
  Serial.println("Getting lap time");
  data_inst.currentLapTime = lapTimer.value();
  Serial.println("Running Tachometer");
  data_inst.speed_mph = runTachometer();
  Serial.println("Getting incline");
  data_inst.accel_out[3] = runMPU();
}

void QSM_sys::runButtonHandler()
{
  //if(!digitalRead(pin_buttonFlag)) return(false);
  
  if(STRT_STP->pressed())
  {
    if(raceTimer.state() == StopWatch::RESET)
    {
      raceTimer.start();
      data_inst.lapCount = 1;
      data_inst.dist_m_total = 0;
    }
  }
  if(LAP->pressed())
  {
    data_inst.lapTimes[data_inst.lapCount] = data_inst.currentLapTime;
    data_inst.lapCount++;

    //EEPROM.write(memAddr_odo_total, data_inst.dist_m_lap+data_inst.dist_m_total); //adds to the total odometer of the vehicle
    data_inst.dist_m_lap = 0;
    
  }
  if(RST->pressed())
  {
    if (raceTimer.state() == StopWatch::RESET) return(false);
    raceTimer.reset();
    //EEPROM.write(memAddr_odo_total, data_inst.dist_m_lap+data_inst.dist_m_total);
  }
}
void setup_tachometer(int tach_pin)
{
  pinMode(tach_pin, INPUT);
}
int QSM_sys::runTachometer()
{
  int multiplier = 1;
  if(data_inst.speed_mph<10000)
  {
    multiplier = 3;   
  }
  if(tachResetTimer.value() < tach_reset_delay*multiplier)
  {
    return(data_inst.speed_mph);
  }
  else tachResetTimer.reset();   //resets the tachometer reset timer so the tachometer will not be used again until tach_reset_delay afer start()

  //`lcd->print_header_message("Running Tachometer", ILI9341_BLUE, 2);
  Serial.println("Run tachometer function called");
  unsigned int msCounter = 0; //measures wheel revolution time in m

  
  for (int i = 0; i < tach_repCount; i++) //repeats the tachometer operation as many times as tach_repCount
  {
    //Serial.println("First Loop");
    //runs until the leading edge of the magnet passes the hall effect sensor
    while (digitalRead(pin_s_tachometer) == 1)
    {
      delay(tach_timeStep);
    }
    //data_inst.Dist_cm_total+=wheelCircumference_mm/1000;
    //Serial.println("Second Loop"); //debug
    //triggered when the leading edge of the magnet passes the hall effect sensor
    while (digitalRead(pin_s_tachometer) == 0)
    {
      msCounter++;
      //Serial.println(msCounter); //debug
      delay(tach_timeStep);
    }
    //Serial.println("Third Loop"); //debug
    //triggered when the magnet has fully passed the sensor
    //ends when the sensor sees the magnet again
    while (digitalRead(pin_s_tachometer) == 1)
    {
      msCounter++;
      //Serial.println(msCounter); //debug
      delay(tach_timeStep);
    }
  }
    msCounter = msCounter / tach_repCount;
    //Serial.println(1000*(3600*1.6/msCounter));

    tachResetTimer.start();
    return(1000*(3600*1.6/msCounter));
    
  //Serial.println("Complete"); //debug
//  msCounter = msCounter / tach_repCount; //calculates the average time per revolution
//
//  //calculate instantaneous and average velocity
//  data_inst.Speed() = 1.6/(msCounter*tach_speed_coeff);

//  data_inst.Speed_Avg_Total = alpha * data_inst.Speed() + (1 - alpha)*data_inst.Speed_Avg_Total;
//
//  data_inst.Speed_Avg_Lap = alpha * data_inst.Speed() + (1- alpha)*data_inst.Speed_Avg_Lap;

//  unsigned long usCounter = 0; //measures wheel revolution time in m
//  unsigned int break_flag_counter = 0;
//  StopWatch tachTimer(StopWatch::MICROS);
//  
//  for (int i = 0; i < tach_repCount; i++) //repeats the tachometer operation as many times as tach_repCount
//  {
//    Serial.println("First Loop");
//    //runs until the leading edge of the magnet passes the hall effect sensor
//    while (digitalRead(pin_s_tachometer) == 1)
//    {
//      break_flag_counter++;
//      Serial.println(break_flag_counter);
//      if(break_flag_counter == 150000) return(0);
//    }
//    //data_inst.Dist_cm_total+=wheelCircumference_mm/1000;
//    tachTimer.start();
//    //Serial.println("Second Loop"); //debug
//    //triggered when the leading edge of the magnet passes the hall effect sensor
//    while (digitalRead(pin_s_tachometer) == 0)
//    {
//      //Serial.println(msCounter); //debug
//      //msCounter++;
//    }
//    //Serial.println("Third Loop"); //debug
//    //triggered when the magnet has fully passed the sensor
//    //ends when the sensor sees the magnet again
//    while (digitalRead(pin_s_tachometer) == 1)
//    {
//      //msCounter++;
//      //Serial.println(msCounter); //debug
//      //delay(tach_timeStep);
//    }
//    tachTimer.stop();
//  }
//  Serial.print("us Counter = ");
//  Serial.println(usCounter);
//  usCounter = tachTimer.value() / tach_repCount;
//  
//  return(1000*(1000*(3600*1.6/usCounter)));
//  //Serial.println("Complete"); //debug
////  msCounter = msCounter / tach_repCount; //calculates the average time per revolution
////
////  //calculate instantaneous and average velocity
////  data_inst.Speed() = 1.6/(msCounter*tach_speed_coeff);
//
////  data_inst.Speed_Avg_Total = alpha * data_inst.Speed() + (1 - alpha)*data_inst.Speed_Avg_Total;
////
////  data_inst.Speed_Avg_Lap = alpha * data_inst.Speed() + (1- alpha)*data_inst.Speed_Avg_Lap;
//

}

int QSM_sys::runOdometer()
{
  
}

void QSM_sys::MPU_setup()
{
  Wire.begin();
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();  
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s 
  Wire.endTransmission(); 
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission(); 
}

int* QSM_sys::runMPU()
{
  int* accel_outputs;
  //Serial.println("Beginning Transmission");
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  //Serial.println("Transmission complete, requestion data");
  Wire.requestFrom(0b1101000, 6); //Request Accel Registers (3B - 40)
  while (Wire.available() < 6);
  accel_outputs[0] = Wire.read() << 8 | Wire.read(); //Store first two bytes into accelX
  accel_outputs[1] = Wire.read() << 8 | Wire.read(); //Store middle two bytes into accelY
  accel_outputs[2] = Wire.read() << 8 | Wire.read(); //Store last two bytes into accelZ
  return(accel_outputs);
}

int QSM_sys::getSpeedDelta()
{
 return(data_inst.speed_mph-data_prev.speed_mph); 
}

