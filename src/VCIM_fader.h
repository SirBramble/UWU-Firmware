/**
 * @file VCIM_fader.h
 * @author Bramble
 * @brief Header file for VMI VCIM Module
 * @version 1.0
 * @date 2025-02-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
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

/**
 * @brief Fader ADC max val
 * Due to Faders being offset by Resistor, the ADC can only reach a cirtain maximum value. 
 * 
 */
#define VCIM_FADER_TOP_LIMMIT 200

// moved to filesystem.h
/*
typedef struct{
  uint8_t r;
  uint8_t g;
  uint8_t b;
}colour;
*/


/**
 * @brief VCIM buffered register union
 * Allows for register access via parameter names and register index.
 * 
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

/**
 * @brief VCIM module class
 * 
 */
class vcim : public module
{
public:
  /**
   * @brief VCIM constructor
   * 
   * @param moduleName Name ofd the module. Used as name in 'layout.txt' configuration file.
   * @param i2c_addr I2C Address of the module
   * @param update_flag_gpio GPIO connected to interrupt line of module
   * @param update_interval Update intervall in ms. Set to -1 if not needed
   */
  vcim(String moduleName, uint8_t i2c_addr, int update_flag_gpio, uint16_t update_interval = -1);

  /**
   * @brief Initialises the module and enables the interrupt pin
   * 
   */
  void init();

  /**
   * @brief Gets update from Module and sets 'update in'-flags
   * 
   */
  void get_update();

  /**
   * @brief Sends update to module and clears all 'update out'-flags
   * 
   */
  void send_update();

  /**
   * @brief Checks if Updates are available
   * 
   * @return uint8_t 
   */
  uint8_t update_available();


  //_vcim_fader_register * get_register_pointer();

  /**
   * @brief Sets KP value for fader PID controler and sets 'update out'-flag
   * 
   * @param val KP value
   */
  void set_fader_kp(uint8_t val);
  
  /**
   * @brief Sets KI value for fader PID controler and sets 'update out'-flag
   * 
   * @param val KI value
   */
  void set_fader_ki(uint8_t val);

  /**
   * @brief Sets KD value for fader PID controler and sets 'update out'-flag
   * 
   * @param val KD value
   */
  void set_fader_kd(uint8_t val);

  /**
   * @brief Sets maximum cycle count for fader controler and sets 'update out'-flag
   * 
   * @param val Maximum cycles
   */
  void set_fader_pid_max_cycles(uint8_t val);

  /**
   * @brief Sets fader position value and sets 'update out'-flag
   * 
   * @param position Index of the fader to be addressed
   * @param val Fader position to be set
   */
  void set_fader_positions(uint8_t position, uint8_t val);

  /**
   * @brief Sets button state at position and sets 'update out'-flag
   * 
   * @param position Index of the button to be addressed
   * @param val Button state to be set
   */
  void set_button_states(uint8_t position, uint8_t val);

  /**
   * @brief Sets button colour at position and sets 'update out'-flag
   * 
   * @param position Index of the button to be addressed
   * @param val Button colour to be set
   */
  void set_button_colours(uint8_t position, colour val);

  /**
   * @brief Sets encoder position value and sets 'update out'-flag
   * 
   * @param position Index of the encoder to be addressed
   * @param val Encoder position to be set
   */
  void set_encoder_postitions(uint8_t position, uint8_t val);

  /**
   * @brief Sets encoder button state at position and sets 'update out'-flag
   * 
   * @param position Index of the encdoer button to be addressed
   * @param val Button state to be set
   */
  void set_encoder_button_states(uint8_t position, uint8_t val);

  /**
   * @brief Sets maximum ring light bightness
   * 
   * @param val Maximum brightness
   */
  void set_ring_light_max_brightness(uint8_t val);

  // Checks clear update in flags

  /**
   * @brief Checks for updates in the fader position and clears 'update in'-flags
   * 
   * @param position Index to check
   * @return uint8_t 
   */
  uint8_t check_update_fader_positions(uint8_t position);

  /**
   * @brief Checks for updates in the button state and clears 'update in'-flags
   * 
   * @param position Index to check
   * @return uint8_t 
   */
  uint8_t check_update_button_states(uint8_t position);

  /**
   * @brief Checks for updates in the encoder position and clears 'update in'-flags
   * 
   * @param position Index to check
   * @return uint8_t 
   */
  uint8_t check_update_encoder_postitions(uint8_t position);

  /**
   * @brief Checks for updates in the encoder button states and clears 'update in'-flags
   * 
   * @param position Index to check
   * @return uint8_t 
   */
  uint8_t check_update_encoder_button_states(uint8_t position);

  /**
   * @brief Gets the currently buffered fader position
   * 
   * @param position Index of fader
   * @return uint8_t 
   */
  uint8_t get_fader_positions(uint8_t position);

  /**
   * @brief Gets the currently buffered button state
   * 
   * @param position Index of Button
   * @return uint8_t 
   */
  uint8_t get_button_states(uint8_t position);

  /**
   * @brief Gets the currently buffered encoder position
   * 
   * @param position Index of encoder
   * @return uint8_t 
   */
  uint8_t get_encoder_postitions(uint8_t position);

  /**
   * @brief Gets the currently buffered encoder button state
   * 
   * @param position Index of encoder button
   * @return uint8_t 
   */
  uint8_t get_encoder_button_states(uint8_t position);
private:
  /**
   * @brief Buffered copy of the modules registers
   * 
   */
  _vcim_fader_register vcim_register;

  /**
   * @brief 'Update in'-flag array
   * 
   */
  uint8_t update_flags_in[VCIM_REGISTER_ARRAY_SIZE];
  /**
   * @brief 'Update out'-flag array
   * 
   */
  uint8_t update_flags_out[VCIM_REGISTER_ARRAY_SIZE];
  /**
   * @brief 'Update in'-flag mask array
   * If 1 is set will copy into buffered registers. Used to mask RGB values
   * 
   */
  uint8_t update_flag_in_mask[VCIM_REGISTER_ARRAY_SIZE];

  /**
   * @brief 'Update out'-flag mask array
   * Debug. Will patch TM
   * 
   */
  uint8_t update_flag_out_mask[VCIM_REGISTER_ARRAY_SIZE];

  /**
   * @brief GPIO connected to interrupt line of module
   * 
   */
  int update_flag_gpio;

  /**
   * @brief I2C Address of module
   * 
   */
  uint8_t i2c_addr;

  /**
   * @brief Time of last update call 
   * 
   */
  uint64_t update_time_prev = 0;

  /**
   * @brief Update intervall in ms. Set to -1 if not needed
   * 
   */
  uint16_t update_interval = 0;
};

#endif // VCIM_FADER_H