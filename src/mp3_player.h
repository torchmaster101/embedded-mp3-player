#include "helper.h"
#include "serialATmega.h"

// Commands obtained from MP3 module documentation

#ifndef MP3_PLAYER_H
#define MP3_PLAYER_H

#define CMD_SEL_DEV 0x09 // To select microSD card
#define DEV_TF 0x02 // For microSD card

#define CMD_PLAY 0x0D
#define CMD_PLAY_INDEX 0x03
#define CMD_PAUSE 0x0E

// Sends command to MP3 module via UART pins TX and RX
void mp3_send_command(int8_t command, int16_t data){
    serial_char(0x7E); // start
    serial_char(0xFF);
    serial_char(0x06);
    serial_char(command);
    serial_char(0x00);
    serial_char((int8_t) (data >> 8));
    serial_char((int8_t) data);
    serial_char(0xEF); // end
}

void mp3_init(){
    serial_init(9600);
    _delay_ms(500);

    mp3_send_command(CMD_SEL_DEV, DEV_TF); // Select microSD card
    _delay_ms(200);
}

void mp3_play_track(){
    mp3_send_command(CMD_PLAY, 0x00);
}

void mp3_play_track_index(int8_t index){
    mp3_send_command(CMD_PLAY_INDEX, index);
}

void mp3_pause_track(){
    mp3_send_command(CMD_PAUSE, 0x00);
}

#endif