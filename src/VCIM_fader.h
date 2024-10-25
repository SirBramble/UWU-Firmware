#ifndef VCIM_FADER_H
#define VCIM_FADER_H

#include "filesystem.h"
#include "i2cinterface.h"

#define VCIM_REGISTER_ARRAY_SIZE 0x9D + 1

#define VCIM_REG_OFFSET_FADER_KP                      0x00
#define VCIM_REG_OFFSET_FADER_KI                      0x01
#define VCIM_REG_OFFSET_FADER_KD                      0x02
#define VCIM_REG_OFFSET_FADER_PID_MAX_CYCLE           0x03
#define VCIM_REG_OFFSET_FADER_POSITION                0x04
#define VCIM_REG_OFFSET_BUTTON_STATES                 0x07
#define VCIM_REG_OFFSET_BUTTON_COLOURS                0x28
#define VCIM_REG_OFFSET_ENCODER_POSITIONS             0x8B
#define VCIM_REG_OFFSET_ENCODER_BUTTON_STATES         0x94
#define VCIM_REG_OFFSET_RING_LIGHT_MAX_BRIGHTNESS     0x9D

#define VCIM_AMMOUNT_FADERS 3
#define VCIM_AMMOUNT_BUTTONS 33
#define VCIM_AMMOUNT_ENCODERS 9

#define VCIM_FADER_TOP_LIMMIT 200

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
    uint8_t fader_positions[VCIM_AMMOUNT_FADERS];
    uint8_t button_states[VCIM_AMMOUNT_BUTTONS];
    colour button_colours[VCIM_AMMOUNT_BUTTONS];
    uint8_t encoder_postitions[VCIM_AMMOUNT_ENCODERS];
    uint8_t encoder_button_states[VCIM_AMMOUNT_ENCODERS];
    uint8_t ring_light_max_brightness;
	} registers;

	uint8_t register_array[VCIM_REGISTER_ARRAY_SIZE];

} _vcim_fader_register;

class vcim : public module
{
public:
  vcim(String moduleName, uint8_t i2c_addr, int update_flag_gpio, uint16_t update_interval = -1);
  void init();
  void get_update();    // sets update in flags
  void send_update();   // clears all update out flags
  uint8_t update_available();
  //_vcim_fader_register * get_register_pointer();

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
  _vcim_fader_register vcim_register;
  uint8_t update_flags_in[VCIM_REGISTER_ARRAY_SIZE];
  uint8_t update_flags_out[VCIM_REGISTER_ARRAY_SIZE];
  uint8_t update_flag_in_mask[VCIM_REGISTER_ARRAY_SIZE];   // only copy into registers, if 1 at position (Use to mask RGB vals)
  uint8_t update_flag_out_mask[VCIM_REGISTER_ARRAY_SIZE];   // debug, will patch
  int update_flag_gpio;
  uint8_t i2c_addr;
  uint64_t update_time_prev = 0;
  uint16_t update_interval = 0;   // in ms
};

#endif // VCIM_FADER_H