int* QSM_processor::convert_ms_clock(unsigned int msCount)
{
   int* clkVals;
   int y = 60*60*1000;
   int h = msCount/y;
   clkVals[0] = (msCount-(h*y))/(y/60);
   clkVals[1] = (msCount-(h*y)-(clkVals[0]*(y/60)))/1000;
   clkVals[2] = msCount-(h*y)-(clkVals[0]*(y/60))-(clkVals[1]*1000);
//  clkVals[0] = (char)(msCount/ms_min);
//  clkVals[1] = (char)((msCount % ms_min)/ms_sec);
//  clkVals[2] = (char)((msCount % ms_min) % ms_sec);
  return(clkVals);
}

unsigned int QSM_processor::calcAvgLapTime(unsigned int lapTimesArray[10], int lapCount)
{
  unsigned int avgLapTime;
  
  for(int i = 0; i<lapCount; i++)
  {
    avgLapTime+=lapTimesArray[i];
  }
  return(avgLapTime/lapCount);
}

int QSM_processor::calcInclinePercent(int accel_out[3])
{ 
  int gForceX = (accel_out[0]) / 16384.0;
//  int gForceY = (accel_out[1]) / 16384.0;
//  int gForceZ = (accel_out[2] + 1302) / 16384.0;

  int degs_X = 360 * (asin(gForceX)) / (2 * PI);
//  int degs_Y = 360 * (asin(gForceY)) / (2 * PI);
//  int degs_Z = 360 * (asin(gForceZ)) / (2 * PI);

  return(degs_X);
}

int calcMetersPerHour(int ms_per_rev)
{
  float rev_per_ms = 1/ms_per_rev;
  
  //1000 ms per s
  //3600 s  per h
  //1.6  m  per rev
  int mph = rev_per_ms*1000*3600*1.6;
  return(mph);
}

