
#pragma once

class Dots
{
  short _row;
  short _column;
  LiquidCrystal* _lcd;
  bool _running;

  char _current;

public:
  Dots(LiquidCrystal *lcd)
  {
    _current = 0;
    _lcd = lcd;
  }

  void location(short column, short row)
  {
    _column = column;
    _row = row;
  }

  void tick()
  {
    if(!_running)
    {
      return;
    }
    
    if(millis() % 250 != 0)
    {
      return;
    }
    
    if(_current >= 3)
    {
      _current = 0;
      _lcd->setCursor(_column, _row);
      _lcd->write(' ');
      _lcd->write(' ');
      _lcd->write(' ');
      return;
    }

    _lcd->setCursor(_column + _current, _row);
    _lcd->write('.');
    _current++;
  }
  
  void running(bool running)
  {
    _running = running;
  }
};

