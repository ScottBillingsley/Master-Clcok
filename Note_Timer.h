#ifndef MY_NOTE_TIMER_H
#define MY_NOTE_TIMER_H

#include <Arduino.h>

class NoteTimer{

  private:
    int counter;
    int compare_count;
    
  public:
    NoteTimer();
    void init();
    void count();
    void setCompare(int compare_count);

    bool check();
  
};

#endif
