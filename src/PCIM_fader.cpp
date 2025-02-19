/**
 * @file PCIM_fader.cpp
 * @author Bramble
 * @brief CPP file for VMI PCIM Module
 * @version 1.0
 * @date 2025-02-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "PCIM_fader.h"

pcim::pcim(String moduleName, uint8_t i2c_addr, int update_flag_gpio, uint16_t update_interval) : module(moduleName)
{
  this->update_flag_gpio = update_flag_gpio;
  this->i2c_addr = i2c_addr;
  this->update_interval = update_interval;
}

void pcim::init()
{
  pinMode(this->update_flag_gpio, INPUT_PULLDOWN);

  for(int i = 0; i < PCIM_REGISTER_ARRAY_SIZE; i++)   // set mask, to copy all values into registers on recieve
  {
    update_flag_in_mask[i] = 1;
  }
  
  // set override for all color value registers
  for(int i = 0; i < PCIM_AMMOUNT_BUTTONS; i++)   // buttons
  {
    uint8_t position = PCIM_REG_OFFSET_BUTTON_COLOURS + i*3;
    update_flag_in_mask[position] = 0;
    update_flag_in_mask[position + 1] = 0;
    update_flag_in_mask[position + 2] = 0;
    update_flag_out_mask[position] = 1;
    update_flag_out_mask[position + 1] = 1;
    update_flag_out_mask[position + 2] = 1;
  }

  for(int i = 0; i < PCIM_AMMOUNT_ENCODER_POST_BUTTONS; i++)   // encoders post button
  {
    uint8_t position = PCIM_REG_OFFSET_ENCODER_POST_BUTTON_COLOURS + i*3;
    update_flag_in_mask[position] = 0;
    update_flag_in_mask[position + 1] = 0;
    update_flag_in_mask[position + 2] = 0;
  }

  update_flag_in_mask[PCIM_REG_OFFSET_ENCODER_EQ_BUTTON_COLOUR] = 0; // encoders eq button
  update_flag_in_mask[PCIM_REG_OFFSET_ENCODER_EQ_BUTTON_COLOUR + 1] = 0;
  update_flag_in_mask[PCIM_REG_OFFSET_ENCODER_EQ_BUTTON_COLOUR + 2] = 0;

  for(int i = 0; i < PCIM_AMMOUNT_LEVEL_INDICATOR_LEDS; i++)   // Level Indicator left
  {
    uint8_t position = PCIM_REG_OFFSET_LEVEL_INDICATOR_COLOUR_LEFT + i*3;
    update_flag_in_mask[position] = 0;
    update_flag_in_mask[position + 1] = 0;
    update_flag_in_mask[position + 2] = 0;
  }

  for(int i = 0; i < PCIM_AMMOUNT_LEVEL_INDICATOR_LEDS; i++)   // Level Indicator right
  {
    uint8_t position = PCIM_REG_OFFSET_LEVEL_INDICATOR_COLOUR_RIGHT + i*3;
    update_flag_in_mask[position] = 0;
    update_flag_in_mask[position + 1] = 0;
    update_flag_in_mask[position + 2] = 0;
  }

}

void pcim::get_update()
{
  Wire.beginTransmission(this->i2c_addr);
  Wire.write(0x00);
  uint8_t error = Wire.endTransmission();
  Serial.printf("Response test for recieve: %d\n", error);
  delay(10);
  uint8_t recieved_ammount = Wire.requestFrom(this->i2c_addr, sizeof(this->pcim_register), true);
  for(uint8_t i = 0; (i < recieved_ammount && Wire.available()); i++)
  {
    uint8_t r_byte = Wire.read();
    if(r_byte != this->pcim_register.register_array[i] && update_flag_in_mask[i] > 0)
    {
      this->pcim_register.register_array[i] = r_byte;
      this->update_flags_in[i] = 1;
    }
    else
    {
      this->update_flags_in[i] = 0; 
    }
    
    Serial.printf("%d ", this->pcim_register.register_array[i]);
  }
  Serial.println();
  Serial.printf("Recieved ammount: %d\n", recieved_ammount);
  delay(10);
}

void pcim::send_update()
{
  uint8_t update_counter = 0;
  for(uint8_t i = 0; i < sizeof(this->pcim_register); i++)
  {
    if(this->update_flags_out[i] > 0 || update_flag_out_mask[i] > 0)
    {
      update_counter++;
    }
    else if (update_counter > 0)
    {
      uint8_t start_reg = i - update_counter;
      Wire.beginTransmission(this->i2c_addr);
      Wire.write(start_reg);
      for(uint8_t n = 0; n < update_counter; n++)
      {
        Wire.write(this->pcim_register.register_array[start_reg + n]);
      }
      uint8_t error = Wire.endTransmission();
      delay(10);
      Serial.printf("Response test for send: %d\t start_reg: %d \t ammount: %d \t address: %d\n", error, start_reg, update_counter, this->i2c_addr);
      update_counter = 0;
    }

    this->update_flags_out[i] = 0;
  }

  if (update_counter > 0)
  {
    uint8_t start_reg = sizeof(this->pcim_register) - update_counter;
    Wire.beginTransmission(this->i2c_addr);
    Wire.write(start_reg);
    for(uint8_t n = 0; n < update_counter; n++)
    {
      Wire.write(this->pcim_register.register_array[start_reg + n]);
    }
    uint8_t error = Wire.endTransmission();
    delay(10);
    Serial.printf("Response test for send: %d\t start_reg: %d\t ammount: %d\n", error, start_reg, update_counter);
    update_counter = 0;
  }

  /*
  Wire.beginTransmission(this->i2c_addr);
  Wire.write(0x00);
  for(uint8_t i = 0; i < sizeof(this->pcim_register); i++)
  {
    Wire.write(this->pcim_register.register_array[i]);
  }
  uint8_t error = Wire.endTransmission();
  Serial.printf("Response test for send: %d\n", error);
  */
}

uint8_t pcim::update_available()
{
  if( (this->update_interval != (uint16_t)-1) && (this->update_time_prev + this->update_interval < millis()) )
  {
    this->update_time_prev = millis();
    return 1;
  }
  return digitalRead(this->update_flag_gpio);
}

/*
_pcim_fader_register * pcim::get_register_pointer()
{
  return &this->pcim_register;
}
*/

void pcim::set_fader_kp(uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_FADER_KP;
  this->pcim_register.registers.fader_kp = val;
  this->update_flags_out[reg_offset] = 1;
}
void pcim::set_fader_ki(uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_FADER_KI;
  this->pcim_register.registers.fader_ki = val;
  this->update_flags_out[reg_offset] = 1;
}
void pcim::set_fader_kd(uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_FADER_KD;
  this->pcim_register.registers.fader_kd = val;
  this->update_flags_out[reg_offset] = 1;
}
void pcim::set_fader_pid_max_cycles(uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_FADER_PID_MAX_CYCLE;
  this->pcim_register.registers.fader_pid_max_cycles = val;
  this->update_flags_out[reg_offset] = 1;
}
void pcim::set_fader_position(uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_FADER_POSITION;
  this->pcim_register.registers.fader_position = val;
  this->update_flags_out[reg_offset] = 1;
}

void pcim::set_button_states(uint8_t position, uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_BUTTON_STATES;
  this->pcim_register.registers.button_states[position] = val;
  this->update_flags_out[reg_offset + position] = 1;
}
void pcim::set_button_colours(uint8_t position, colour val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_BUTTON_COLOURS;
  this->pcim_register.registers.button_colours[position] = val;
  this->update_flags_out[reg_offset + position*3] = 1;
  this->update_flags_out[reg_offset + position*3 + 1] = 1;
  this->update_flags_out[reg_offset + position*3 + 2] = 1;
}
void pcim::set_encoder_postitions(uint8_t position, uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_ENCODER_POSITIONS;
  this->pcim_register.registers.encoder_postitions[position] = val;
  this->update_flags_out[reg_offset + position] = 1;
}
void pcim::set_encoder_button_states(uint8_t position, uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_ENCODER_BUTTON_STATES;
  this->pcim_register.registers.encoder_button_states[position] = val;
  this->update_flags_out[reg_offset + position] = 1;
}
void pcim::set_encoder_post_button_states(uint8_t position, uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_ENCODER_POST_BUTTON_STATES;
  this->pcim_register.registers.encoder_post_button_states[position] = val;
  this->update_flags_out[reg_offset + position] = 1;
}
void pcim::set_encoder_eq_button_state(uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_ENCODER_EQ_BUTTON_STATE;
  this->pcim_register.registers.encoder_eq_button_state = val;
  this->update_flags_out[reg_offset] = 1;
}
void pcim::set_encoder_post_button_colours(uint8_t position, colour val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_ENCODER_POST_BUTTON_COLOURS;
  this->pcim_register.registers.encoder_post_button_colours[position] = val;
  this->update_flags_out[reg_offset + position*3] = 1;
  this->update_flags_out[reg_offset + position*3 + 1] = 1;
  this->update_flags_out[reg_offset + position*3 + 2] = 1;
}
void pcim::set_encoder_eq_button_colour(colour val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_ENCODER_EQ_BUTTON_COLOUR;
  this->pcim_register.registers.encoder_eq_button_colour = val;
  this->update_flags_out[reg_offset] = 1;
  this->update_flags_out[reg_offset + 1] = 1;
  this->update_flags_out[reg_offset + 2] = 1;
}
void pcim::set_ring_light_max_brightness(uint8_t val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_RING_LIGHT_MAX_BRIGHTNESS;
  this->pcim_register.registers.ring_light_max_brightness = val;
  this->update_flags_out[reg_offset] = 1;
}
void pcim::set_level_indicator_colour_left(uint8_t position, colour val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_LEVEL_INDICATOR_COLOUR_LEFT;
  this->pcim_register.registers.level_indicator_colour_left[position] = val;
  this->update_flags_out[reg_offset + position*3] = 1;
  this->update_flags_out[reg_offset + position*3 + 1] = 1;
  this->update_flags_out[reg_offset + position*3 + 2] = 1;
}
void pcim::set_level_indicator_colour_right(uint8_t position, colour val)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_LEVEL_INDICATOR_COLOUR_RIGHT;
  this->pcim_register.registers.level_indicator_colour_right[position] = val;
  this->update_flags_out[reg_offset + position*3] = 1;
  this->update_flags_out[reg_offset + position*3 + 1] = 1;
  this->update_flags_out[reg_offset + position*3 + 2] = 1;
}

uint8_t pcim::check_update_fader_position()
{
  uint8_t reg_offset = PCIM_REG_OFFSET_FADER_POSITION;
  uint8_t return_val = this->update_flags_in[reg_offset];
  this->update_flags_in[reg_offset] = 0;
  return return_val;
}
uint8_t pcim::check_update_button_states(uint8_t position)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_BUTTON_STATES;
  uint8_t return_val = this->update_flags_in[reg_offset + position];
  this->update_flags_in[reg_offset + position] = 0;
  return return_val;
}
uint8_t pcim::check_update_encoder_postitions(uint8_t position)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_ENCODER_POSITIONS;
  uint8_t return_val = this->update_flags_in[reg_offset + position];
  this->update_flags_in[reg_offset + position] = 0;
  return return_val;
}
uint8_t pcim::check_update_encoder_button_states(uint8_t position)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_ENCODER_BUTTON_STATES;
  uint8_t return_val = this->update_flags_in[reg_offset + position];
  this->update_flags_in[reg_offset + position] = 0;
  return return_val;
}
uint8_t pcim::check_update_encoder_post_button_states(uint8_t position)
{
  uint8_t reg_offset = PCIM_REG_OFFSET_ENCODER_POST_BUTTON_STATES;
  uint8_t return_val = this->update_flags_in[reg_offset + position];
  this->update_flags_in[reg_offset + position] = 0;
  return return_val;
}
uint8_t pcim::check_update_encoder_eq_button_state()
{
  uint8_t reg_offset = PCIM_REG_OFFSET_ENCODER_EQ_BUTTON_STATE;
  uint8_t return_val = this->update_flags_in[reg_offset];
  this->update_flags_in[reg_offset] = 0;
  return return_val;
}

uint8_t pcim::get_fader_position()
{
  return this->pcim_register.registers.fader_position;
}
uint8_t pcim::get_button_states(uint8_t position)
{
  return this->pcim_register.registers.button_states[position];
}
uint8_t pcim::get_encoder_postitions(uint8_t position)
{
  return this->pcim_register.registers.encoder_postitions[position];
}
uint8_t pcim::get_encoder_button_states(uint8_t position)
{
  return this->pcim_register.registers.encoder_button_states[position];
}
uint8_t pcim::get_encoder_post_button_states(uint8_t position)
{
  return this->pcim_register.registers.encoder_post_button_states[position];
}
uint8_t pcim::get_encoder_eq_button_state()
{
  return this->pcim_register.registers.encoder_eq_button_state;
}

