/**
 * @file ACOM_fader.h
 * @author Bramble
 * @brief Header file for VMI ACOM Module
 * @version 1.0
 * @date 2025-02-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef ACOM_FADER_H
#define ACOM_FADER_H

#include "filesystem.h"
#include "i2cinterface.h"

#define ACOM_REGISTER_ARRAY_SIZE 0xAC + 1

#define ACOM_REG_OFFSET_FADER_KP                      0x00
#define ACOM_REG_OFFSET_FADER_KI                      0x01
#define ACOM_REG_OFFSET_FADER_KD                      0x02
#define ACOM_REG_OFFSET_FADER_PID_MAX_CYCLE           0x03
#define ACOM_REG_OFFSET_FADER_POSITION                0x04
#define ACOM_REG_OFFSET_ENCODER_POSITIONS             0x0C
#define ACOM_REG_OFFSET_ENCODER_BUTTON_STATES         0x1C
#define ACOM_REG_OFFSET_BUTTON_STATES                 0x2C
#define ACOM_REG_OFFSET_BUTTON_COLOURS                0x4C
#define ACOM_REG_OFFSET_RING_LIGHT_MAX_BRIGHTNESS     0xAC

#define ACOM_AMMOUNT_FADERS 8
#define ACOM_AMMOUNT_ENCODERS 16
#define ACOM_AMMOUNT_BUTTONS 32

#define ACOM_FADER_TOP_LIMMIT 200

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
    uint8_t fader_positions[ACOM_AMMOUNT_FADERS];
    uint8_t encoder_postitions[ACOM_AMMOUNT_ENCODERS];
    uint8_t encoder_button_states[ACOM_AMMOUNT_ENCODERS];
    uint8_t button_states[ACOM_AMMOUNT_BUTTONS];
    colour button_colours[ACOM_AMMOUNT_BUTTONS];
    uint8_t ring_light_max_brightness;
	} registers;

	uint8_t register_array[ACOM_REGISTER_ARRAY_SIZE];

} _acom_fader_register;

class acom : public module
{
public:
  acom(String moduleName, uint8_t i2c_addr, int update_flag_gpio, uint16_t update_interval = -1);
  void init();
  void get_update();    // sets update in flags
  void send_update();   // clears all update out flags
  uint8_t update_available();
  //_acom_fader_register * get_register_pointer();

  // Sets update out flags
  void set_fader_kp(uint8_t val);
  void set_fader_ki(uint8_t val);
  void set_fader_kd(uint8_t val);
  void set_fader_pid_max_cycles(uint8_t val);
  void set_fader_positions(uint8_t position, uint8_t val);
  void set_button_states(uint8_t position, uint8_t val);
  void set_button_colours(uint8_t position, colour val);
  void set_encoder_postitions(uint8_t position, uint8_t val);
  void set_encoder_button_states(uint8_t position, uint8_t val);
  void set_ring_light_max_brightness(uint8_t val);

  // Checks clear update in flags
  uint8_t check_update_fader_positions(uint8_t position);
  uint8_t check_update_button_states(uint8_t position);
  uint8_t check_update_encoder_postitions(uint8_t position);
  uint8_t check_update_encoder_button_states(uint8_t position);

  uint8_t get_fader_positions(uint8_t position);
  uint8_t get_button_states(uint8_t position);
  uint8_t get_encoder_postitions(uint8_t position);
  uint8_t get_encoder_button_states(uint8_t position);
private:
  _acom_fader_register acom_register;
  uint8_t update_flags_in[ACOM_REGISTER_ARRAY_SIZE];
  uint8_t update_flags_out[ACOM_REGISTER_ARRAY_SIZE];
  uint8_t update_flag_in_mask[ACOM_REGISTER_ARRAY_SIZE];   // only copy into registers, if 1 at position (Use to mask RGB vals)
  uint8_t update_flag_out_mask[ACOM_REGISTER_ARRAY_SIZE];   // debug, will patch
  int update_flag_gpio;
  uint8_t i2c_addr;
  uint64_t update_time_prev = 0;
  uint16_t update_interval = 0;   // in ms
};

#endif // ACOM_FADER_H