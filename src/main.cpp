#include "timerISR.h"
#include "helper.h"
#include "LCD.h"
#include "mp3_player.h"
#include "periph.h"

#define NUM_TASKS 6

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

const unsigned long RGB_PERIOD = 100;
const unsigned long PLAY_PAUSE_PERIOD = 5;
const unsigned long TOP_LCD_PERIOD = 100;
const unsigned long BOTTOM_LCD_PERIOD = 100;
const unsigned long TRACK_PERIOD = 5;
const unsigned long MP3_MANAGER_PERIOD = 10;

const unsigned long GCD_PERIOD = findGCD(RGB_PERIOD, 
  findGCD(PLAY_PAUSE_PERIOD, 
    findGCD(TOP_LCD_PERIOD, 
      findGCD(BOTTOM_LCD_PERIOD, 
        findGCD(TRACK_PERIOD, 
            MP3_MANAGER_PERIOD)))));

task tasks[NUM_TASKS];

void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {
		if ( tasks[i].elapsedTime >= tasks[i].period ) {
			tasks[i].state = tasks[i].TickFct(tasks[i].state); 
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += GCD_PERIOD;
	}
}

bool playing = false;
bool greenButtonPressed = false;
bool leftButtonPressed = false;
bool rightButtonPressed = false;
bool bottomLCDCleared = false;
bool topLCDCleared = false;

int8_t currTrack = 1;
int8_t lastTrack = currTrack;

enum PLAY_PAUSE_STATES {PLAY_PAUSE_INIT, PLAY_PAUSE_PLAY, PLAY_PAUSE_PAUSE};
enum RGB_STATES {RGB_READY, RGB_PLAYING, RGB_PAUSED};
enum TOP_LCD_STATES {TOP_LCD_INIT, TOP_LCD_DISPLAY};
enum BOTTOM_LCD_STATES {BOTTOM_LCD_READY, BOTTOM_LCD_PLAYING, BOTTOM_LCD_PAUSED};
enum TRACK_STATES {TRACK_STAY, TRACK_CHANGE_LEFT, TRACK_CHANGE_RIGHT};
enum MP3_MANAGER_STATES {MP3_READY, MP3_PLAY, MP3_PAUSE};

void set_rgb(int r, int g, int b){
  PORTB = SetBit(PORTB, 1, r);
  PORTB = SetBit(PORTB, 2, g);
  PORTB = SetBit(PORTB, 3, b);
}

int curr = 1;

int RGBTick(int state) {  
  set_rgb(0, 0, 0);

  switch(state){
    case RGB_READY:
      switch(curr){
        case 1:
          set_rgb(1, 0, 0);
          curr++;
          break;
        case 2:
          set_rgb(0, 1, 0);
          curr++;
          break;
        case 3:
          set_rgb(0, 0, 1);
          curr = 1;
          break;
      }

      if(playing){
        return RGB_PLAYING;
      }

      return RGB_READY;
    case RGB_PLAYING:
      set_rgb(0, 1, 0);

      if(!playing){
        return RGB_PAUSED;
      }

      return RGB_PLAYING;
    case RGB_PAUSED:
      set_rgb(1, 1, 0);

      if(playing){
        return RGB_PLAYING;
      }

      return RGB_PAUSED;
  }

  return RGB_READY;
}

// The play/pause button is the green button

int PlayPauseTick(int state) {
  bool currentlyPressed = !GetBit(PINC, 0);

  if(currentlyPressed && !greenButtonPressed){
    playing = !playing;
  }

  greenButtonPressed = currentlyPressed;

  switch(state){
    case PLAY_PAUSE_INIT:

      if(playing)
        return PLAY_PAUSE_PLAY;

      return PLAY_PAUSE_INIT;
    case PLAY_PAUSE_PLAY:
      if(!playing)
        return PLAY_PAUSE_PAUSE;
      
      return PLAY_PAUSE_PLAY;
    case PLAY_PAUSE_PAUSE:
      if(playing)
        return PLAY_PAUSE_PLAY;
      
      return PLAY_PAUSE_PAUSE;
  }

  return PLAY_PAUSE_INIT;
}

int TopLCDTick(int state) {
  switch(state){
    case TOP_LCD_INIT:
      return playing ? TOP_LCD_DISPLAY : TOP_LCD_INIT;
    case TOP_LCD_DISPLAY:
      lcd_write_str(0, 0, "TRACK ");
      lcd_write_character(currTrack + '0');
      return TOP_LCD_DISPLAY;
  }

  return TOP_LCD_INIT;
}

int BottomLCDTick(int state) {
  switch(state){
    case BOTTOM_LCD_READY:
      lcd_write_str(1, 0, "READY");

      if(playing){
        bottomLCDCleared = false;
        return BOTTOM_LCD_PLAYING;
      }
      
      return BOTTOM_LCD_READY;
    case BOTTOM_LCD_PLAYING:
      if(!bottomLCDCleared){
        bottomLCDCleared = true;
        lcd_clear_row(1);
      }

      lcd_write_str(1, 0, "PLAYING");

      if(!playing){
        bottomLCDCleared = false;
        return BOTTOM_LCD_PAUSED;
      }

      return BOTTOM_LCD_PLAYING;
    case BOTTOM_LCD_PAUSED:
      if(!bottomLCDCleared){
        bottomLCDCleared = true;
        lcd_clear_row(1);
      }

      lcd_write_str(1, 0, "PAUSED");

      if(playing){
        bottomLCDCleared = false;
        return BOTTOM_LCD_PLAYING;
      }
      
      return BOTTOM_LCD_PAUSED;
  }

  return BOTTOM_LCD_READY;
}

int TrackTick(int state) {
  bool leftPressed = !GetBit(PINC, 1);
  bool rightPressed = !GetBit(PINC, 2);

  switch(state){
    case TRACK_STAY:
      if(leftPressed){
        return TRACK_CHANGE_LEFT;
      }

      if(rightPressed){
        return TRACK_CHANGE_RIGHT;
      }

      return TRACK_STAY;
    case TRACK_CHANGE_LEFT:
      if(!leftButtonPressed){
        currTrack--;
        if(currTrack < 1) 
          currTrack = 5;

        leftButtonPressed = true;
      }

      if(!leftPressed){
        leftButtonPressed = false;
        return TRACK_STAY;
      }

      return TRACK_CHANGE_LEFT;
    case TRACK_CHANGE_RIGHT:
      if(!rightButtonPressed){
        currTrack++;
        if(currTrack > 5)
          currTrack = 1;
        
        rightButtonPressed = true;
      }

      if(!rightPressed){
        rightButtonPressed = false;
        return TRACK_STAY;
      }

      return TRACK_CHANGE_RIGHT;
  }

  return TRACK_STAY;
}

int MP3ManagerTick(int state) {
  switch(state){
    case MP3_READY:
      if(playing){
        mp3_play_track();
        return MP3_PLAY;
      }
      
      return MP3_READY;
    case MP3_PLAY:
      if(currTrack != lastTrack){
        mp3_play_track_index(currTrack);
        lastTrack = currTrack;
      }

      return playing ? MP3_PLAY : MP3_PAUSE;
    case MP3_PAUSE:
      mp3_pause_track();

      if(playing){
        mp3_play_track_index(currTrack);
      }

      return playing ? MP3_PLAY : MP3_PAUSE;
  }

  return MP3_READY;
}

int main(void) {
    DDRC = 0x00;
    PORTC = 0xFF;

    DDRB = 0x0E; // 00001110
    PORTB = 0x00;

    mp3_init();
    lcd_init();
    lcd_clear();
    set_rgb(0, 0, 0);

    tasks[0].period = RGB_PERIOD;
    tasks[0].state = RGB_READY;
    tasks[0].elapsedTime = RGB_PERIOD;
    tasks[0].TickFct = &RGBTick;

    tasks[1].period = PLAY_PAUSE_PERIOD;
    tasks[1].state = PLAY_PAUSE_INIT;
    tasks[1].elapsedTime = PLAY_PAUSE_PERIOD;
    tasks[1].TickFct = &PlayPauseTick;

    tasks[2].period = TOP_LCD_PERIOD;
    tasks[2].state = TOP_LCD_DISPLAY;
    tasks[2].elapsedTime = TOP_LCD_PERIOD;
    tasks[2].TickFct = &TopLCDTick;

    tasks[3].period = BOTTOM_LCD_PERIOD;
    tasks[3].state = BOTTOM_LCD_READY;
    tasks[3].elapsedTime = BOTTOM_LCD_PERIOD;
    tasks[3].TickFct = &BottomLCDTick;

    tasks[4].period = TRACK_PERIOD;
    tasks[4].state = TRACK_STAY;
    tasks[4].elapsedTime = TRACK_PERIOD;
    tasks[4].TickFct = &TrackTick;
    
    tasks[5].period = MP3_MANAGER_PERIOD;
    tasks[5].state = MP3_READY;
    tasks[5].elapsedTime = MP3_MANAGER_PERIOD;
    tasks[5].TickFct = &MP3ManagerTick;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}