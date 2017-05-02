 #include <Wire.h>
#include <StopWatch.h>
#include <Math.h>

//required for tft lcd
#include <SPI.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeSerif12pt7b.h>

#define pin_s_tachometer 6

#define pin_buttonFlag 36
#define pin_STR_STP 24
#define pin_RST 26
#define pin_LAP 28
#define pin_SELECT 32 
#define pin_CANCEL 31

#define ms_min 60000
#define ms_sec 1000

#define lcd_width 320
#define lcd_height 240

#define third_x 107
#define third_y 80

#define pin_tft_cs 10
#define pin_tft_dc 9

#define lcd_background ILI9341_BLACK
#define lcd_border ILI9341_WHITE
#define lcd_text_title ILI9341_WHITE, ILI9341_BLACK
#define lcd_text_variable ILI9341_YELLOW, ILI9341_BLACK
#define lcd_text_danger ILI9341_RED, ILI9341_BLACK

#define memAddr_odo_total 0

#define asciiCode_degree 167
#define asciiCode_percent 37

enum OPERATING_MODE {IDLE, RACE, TEST};
enum SCREEN {STARTUP, MAIN, SIMP_TEST};

class QSM_button
{
  public:
  QSM_button(int IO_pin);
  bool pressed();
  
  private:
  int pin;
};

class QSM_processor
{
  public:
  int calcInclinePercent(int accel_output[3]);
  int calcAcceleration(int accel_output[3]);
  int calcMetersPerHour(int ms_per_rev);
  unsigned int calcMeters(unsigned int revolutions);
  unsigned int calcAvgLapTime(unsigned int lapTimesArray[10], int lapCount);
  int* convert_ms_clock(unsigned int msCount);
};

struct QSM_rawDataPacket
{
  //updated by functions
  int lapCount;
  unsigned int lapTimes[10];
  unsigned int currentLap_startTime;

  //updated by updateSystemData function
  unsigned int totalTime;
  unsigned int currentLapTime;
  unsigned int speed_mph;
  int* accel_out;

  unsigned int dist_m_total;
  unsigned int dist_m_race;
  unsigned int dist_m_lap;
};

struct QSM_lcdDataPacket : public QSM_rawDataPacket
{
  float kmph;
  int incline_percent;
  unsigned int time_total[2];
  unsigned int lapTime_average;
};

class QSM_lcd_prefSet
{
  public:
  uint16_t background;
  uint16_t border;
  uint16_t text_title[2];
  uint16_t text_variable[2];
  uint16_t text_danger[2];
  QSM_lcd_prefSet(uint16_t c_background,
                  uint16_t c_border,
                  uint16_t c_text_title,
                  uint16_t c_text_variable,
                  uint16_t c_text_danger)
  {
    background = c_background;
    border = c_border;
    text_title[0] = c_text_title;     
    text_variable[0] = c_text_variable;
    text_danger[0] = c_text_danger;
    text_title[1] = c_background;
    text_variable[1] = c_background;
    text_danger[1] = c_background;
  }
};


class QSM_lcd : public Adafruit_ILI9341
{
  public:
   QSM_lcd(int cs_pin, int dc_pin):Adafruit_ILI9341(cs_pin, dc_pin){};
  
  void attachData(QSM_rawDataPacket *rawDataMemAddress, QSM_lcd_prefSet *set_prefSet);
  QSM_rawDataPacket *rawDataPointer;
  void rawDataHandler();
  QSM_processor proc;
  QSM_lcdDataPacket procData;
  
  enum SCREEN activeScreen;
  QSM_lcd_prefSet* prefSet;
  
  void init(enum SCREEN screenLayout);
  void startupScreen();
  void setScreen(enum SCREEN screenLayout);
  void screen_main_load();
  void screen_test_load();

  void refresh();

  void drawSpeedometer(unsigned int speed_mph);
  void printTime_ClockFormat(unsigned int ms);

  void print_header_message(char* message, uint16_t colour, uint8_t tmp_text_size,char* message2 = "");
  
};

//class odometer_binary
//{
//  odometer_binary(int pin_reset, int pin_Q0, int pin_Q1, int pin_Q2, int pin_Q3)
//  {
//    pinMode(pin_Q0, INPUT);
//    pinMode(pin_Q1, INPUT);
//    pinMode(pin_Q2, INPUT);
//    pinMode(pin_Q3, INPUT);
//    pinMode(pin_reset, OUTPUT);
//  }
//  byte getRevolutions();
//}
//class QSM_raceData
//{
//  QSM_processor proc;
//  unsigned int dist_total;
//  unsigned int dist_lap;
//  unsigned int lapTimes[10];
//  unsigned int time_total;
//  unsigned int speed_average;
//  QSM_raceData(QSM_rawDataPacket rawData)
//  {
//    dist_total = rawData.dist_m_total;
//  }
//}

class QSM_sys
{
  public:
  StopWatch globalTimer;
  QSM_processor proc;
  QSM_rawDataPacket data_inst;
  QSM_rawDataPacket* data_inst_ptr;
  QSM_rawDataPacket data_prev;
  enum OPERATING_MODE currentOpMode;
  
  QSM_sys(enum OPERATING_MODE initMode = IDLE);
  begin();
  
  private:
  void InitializeData();
  void AttachDevices();
  void InitiateTimers();
  
  QSM_lcd* lcd;
  QSM_rawDataPacket updateSystemData();
  void MPU_setup();  //done
  
  //devices
  int runTachometer();//done
  StopWatch tachResetTimer;
  
  int runOdometer();
  int* runMPU();      //done
  
  StopWatch raceTimer;
  StopWatch lapTimer;
  
  //operating loops
  void mode_idle();
  void mode_race();
  void mode_test();

  //buttons
  QSM_button* STRT_STP;
  QSM_button* RST;
  QSM_button* LAP;
  QSM_button* SELECT;
  QSM_button* CANCEL;
  
  void runButtonHandler();//done
  int getSpeedDelta();
  
};

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  
  QSM_sys sys(IDLE);
  
  while(true);
  
}
