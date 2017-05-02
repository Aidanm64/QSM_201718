QSM_button::QSM_button(int IO_pin)
{
  pin = IO_pin;
  pinMode(IO_pin, INPUT);
}

bool QSM_button::pressed()
{
  
  if(return(digitalRead(pin)) == HIGH){
    return(true);
  }
}

