/**
 * @file PCIM_fader.h
 * @author Bramble
 * @brief Header file for VMI PCIM Module
 * @version 1.0
 * @date 2025-02-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef PCIM_FADER_H
#define PCIM_FADER_H

#include "filesystem.h"
#include "i2cinterface.h"

#define PCIM_REGISTER_ARRAY_SIZE 0xA1 + 1

#define PCIM_REG_OFFSET_FADER_KP                      0x00
#define PCIM_REG_OFFSET_FADER_KI                      0x01
#define PCIM_REG_OFFSET_FADER_KD                      0x02
#define PCIM_REG_OFFSET_FADER_PID_MAX_CYCLE           0x03
#define PCIM_REG_OFFSET_FADER_POSITION                0x04
#define PCIM_REG_OFFSET_BUTTON_STATES                 0x05
#define PCIM_REG_OFFSET_BUTTON_COLOURS                0x10
#define PCIM_REG_OFFSET_ENCODER_POSITIONS             0x31
#define PCIM_REG_OFFSET_ENCODER_BUTTON_STATES         0x38
#define PCIM_REG_OFFSET_ENCODER_POST_BUTTON_STATES    0x3F
#define PCIM_REG_OFFSET_ENCODER_EQ_BUTTON_STATE       0x43
#define PCIM_REG_OFFSET_ENCODER_POST_BUTTON_COLOURS   0x44
#define PCIM_REG_OFFSET_ENCODER_EQ_BUTTON_COLOUR      0x50
#define PCIM_REG_OFFSET_RING_LIGHT_MAX_BRIGHTNESS     0x53
#define PCIM_REG_OFFSET_LEVEL_INDICATOR_COLOUR_LEFT   0x54
#define PCIM_REG_OFFSET_LEVEL_INDICATOR_COLOUR_RIGHT  0x7B

#define PCIM_AMMOUNT_BUTTONS 11
#define PCIM_AMMOUNT_ENCODERS 7
#define PCIM_AMMOUNT_ENCODER_POST_BUTTONS 4
#define PCIM_AMMOUNT_LEVEL_INDICATOR_LEDS 13

#define PCIM_FADER_TOP_LIMMIT 200

// moved to filesystem.h
/*
typedef struct{
  uint8_t r;
  uint8_t g;
  uint8_t b;
}colour;
*/

typedef union {

	struct {	
		uint8_t fader_kp;
    uint8_t fader_ki;
    uint8_t fader_kd;
    uint8_t fader_pid_max_cycles;
    uint8_t fader_position;
    uint8_t button_states[PCIM_AMMOUNT_BUTTONS];
    colour button_colours[PCIM_AMMOUNT_BUTTONS];
    uint8_t encoder_postitions[PCIM_AMMOUNT_ENCODERS];
    uint8_t encoder_button_states[PCIM_AMMOUNT_ENCODERS];
    uint8_t encoder_post_button_states[PCIM_AMMOUNT_ENCODER_POST_BUTTONS];
    uint8_t encoder_eq_button_state;
    colour encoder_post_button_colours[PCIM_AMMOUNT_ENCODER_POST_BUTTONS];
    colour encoder_eq_button_colour;
    uint8_t ring_light_max_brightness;
    colour level_indicator_colour_left[PCIM_AMMOUNT_LEVEL_INDICATOR_LEDS];
    colour level_indicator_colour_right[PCIM_AMMOUNT_LEVEL_INDICATOR_LEDS];
	} registers;

	uint8_t register_array[PCIM_REGISTER_ARRAY_SIZE];

} _pcim_fader_register;

class pcim : public module
{
public:
  pcim(String moduleName, uint8_t i2c_addr, int update_flag_gpio, uint16_t update_interval = -1);
  void init();
  void get_update();    // sets update in flags
  void send_update();   // clears all update out flags
  uint8_t update_available();
  //_pcim_fader_register * get_register_pointer();

  // Sets update out flags
  void set_fader_kp(uint8_t val);
  void set_fader_ki(uint8_t val);
  void set_fader_kd(uint8_t val);
  void set_fader_pid_max_cycles(uint8_t val);
  void set_fader_position(uint8_t val);
  void set_button_states(uint8_t position, uint8_t val);
  void set_button_colours(uint8_t position, colour val);
  void set_encoder_postitions(uint8_t position, uint8_t val);
  void set_encoder_button_states(uint8_t position, uint8_t val);
  void set_encoder_post_button_states(uint8_t position, uint8_t val);
  void set_encoder_eq_button_state(uint8_t val);
  void set_encoder_post_button_colours(uint8_t position, colour val);
  void set_encoder_eq_button_colour(colour val);
  void set_ring_light_max_brightness(uint8_t val);
  void set_level_indicator_colour_left(uint8_t position, colour val);
  void set_level_indicator_colour_right(uint8_t position, colour val);

  // Checks clear update in flags
  uint8_t check_update_fader_position();
  uint8_t check_update_button_states(uint8_t position);
  uint8_t check_update_encoder_postitions(uint8_t position);
  uint8_t check_update_encoder_button_states(uint8_t position);
  uint8_t check_update_encoder_post_button_states(uint8_t position);
  uint8_t check_update_encoder_eq_button_state();

  uint8_t get_fader_position();
  uint8_t get_button_states(uint8_t position);
  uint8_t get_encoder_postitions(uint8_t position);
  uint8_t get_encoder_button_states(uint8_t position);
  uint8_t get_encoder_post_button_states(uint8_t position);
  uint8_t get_encoder_eq_button_state();
private:
  _pcim_fader_register pcim_register;
  uint8_t update_flags_in[PCIM_REGISTER_ARRAY_SIZE];
  uint8_t update_flags_out[PCIM_REGISTER_ARRAY_SIZE];
  uint8_t update_flag_in_mask[PCIM_REGISTER_ARRAY_SIZE];   // only copy into registers, if 1 at position (Use to mask RGB vals)
  uint8_t update_flag_out_mask[PCIM_REGISTER_ARRAY_SIZE];   // debug, will patch
  int update_flag_gpio;
  uint8_t i2c_addr;
  uint64_t update_time_prev = 0;
  uint16_t update_interval = 0;   // in ms
};

#endif // PCIM_FADER_H