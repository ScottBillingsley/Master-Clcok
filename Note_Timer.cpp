#include "Note_Timer.h"

  NoteTimer::NoteTimer(){
    this->counter = counter;
    init();
  }

  /*Rest the counter */
  void NoteTimer::init(){
    counter = 0;
  }

  /*Increment the count */
  void NoteTimer::count(){
    counter ++;
  }

  /*Set the compare count */
  void NoteTimer::setCompare(int compare_count){
    this->compare_count = compare_count;
  }

  bool NoteTimer::check(){
    if(counter >= compare_count){
      return true;
    }else{
      return false;
    }
  }
