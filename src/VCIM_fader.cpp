/**
 * @file VCIM_fader.cpp
 * @author Bramble
 * @brief CPP file for VMI VCIM Module
 * @version 1.0
 * @date 2025-02-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "VCIM_fader.h"

vcim::vcim(String moduleName, uint8_t i2c_addr, int update_flag_gpio, uint16_t update_interval) : module(moduleName)
{
  this->update_flag_gpio = update_flag_gpio;
  this->i2c_addr = i2c_addr;
  this->update_interval = update_interval;
}

void vcim::init()
{
  pinMode(this->update_flag_gpio, INPUT_PULLDOWN);

  for(int i = 0; i < VCIM_REGISTER_ARRAY_SIZE; i++)   // set mask, to copy all values into registers on recieve
  {
    update_flag_in_mask[i] = 1;
  }
  
  // set override for all color value registers
  for(int i = 0; i < VCIM_AMMOUNT_BUTTONS; i++)   // buttons
  {
    uint8_t position = VCIM_REG_OFFSET_BUTTON_COLOURS + i*3;
    update_flag_in_mask[position] = 0;
    update_flag_in_mask[position + 1] = 0;
    update_flag_in_mask[position + 2] = 0;
    update_flag_out_mask[position] = 1;
    update_flag_out_mask[position + 1] = 1;
    update_flag_out_mask[position + 2] = 1;
  }

}

void vcim::get_update()
{
  Wire.beginTransmission(this->i2c_addr);
  Wire.write(0x00);
  uint8_t error = Wire.endTransmission();
  Serial.printf("Response test for recieve: %d\n", error);
  delay(10);
  uint8_t recieved_ammount = Wire.requestFrom(this->i2c_addr, sizeof(this->vcim_register), true);
  for(uint8_t i = 0; (i < recieved_ammount && Wire.available()); i++)
  {
    uint8_t r_byte = Wire.read();
    if(r_byte != this->vcim_register.register_array[i] && update_flag_in_mask[i] > 0)
    {
      this->vcim_register.register_array[i] = r_byte;
      this->update_flags_in[i] = 1;
    }
    else
    {
      this->update_flags_in[i] = 0; 
    }
    
    Serial.printf("%d ", this->vcim_register.register_array[i]);
  }
  Serial.println();
  Serial.printf("Recieved ammount: %d\n", recieved_ammount);
  delay(10);
}

void vcim::send_update()
{
  uint8_t update_counter = 0;
  for(uint8_t i = 0; i < sizeof(this->vcim_register); i++)
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
        Wire.write(this->vcim_register.register_array[start_reg + n]);
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
    uint8_t start_reg = sizeof(this->vcim_register) - update_counter;
    Wire.beginTransmission(this->i2c_addr);
    Wire.write(start_reg);
    for(uint8_t n = 0; n < update_counter; n++)
    {
      Wire.write(this->vcim_register.register_array[start_reg + n]);
    }
    uint8_t error = Wire.endTransmission();
    delay(10);
    Serial.printf("Response test for send: %d\t start_reg: %d\t ammount: %d\n", error, start_reg, update_counter);
    update_counter = 0;
  }

  /*
  Wire.beginTransmission(this->i2c_addr);
  Wire.write(0x00);
  for(uint8_t i = 0; i < sizeof(this->vcim_register); i++)
  {
    Wire.write(this->vcim_register.register_array[i]);
  }
  uint8_t error = Wire.endTransmission();
  Serial.printf("Response test for send: %d\n", error);
  */
}

uint8_t vcim::update_available()
{
  if( (this->update_interval != (uint16_t)-1) && (this->update_time_prev + this->update_interval < millis()) )
  {
    this->update_time_prev = millis();
    return 1;
  }
  return digitalRead(this->update_flag_gpio);
}

/*
_vcim_fader_register * vcim::get_register_pointer()
{
  return &this->vcim_register;
}
*/

void vcim::set_fader_kp(uint8_t val)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_FADER_KP;
  this->vcim_register.registers.fader_kp = val;
  this->update_flags_out[reg_offset] = 1;
}
void vcim::set_fader_ki(uint8_t val)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_FADER_KI;
  this->vcim_register.registers.fader_ki = val;
  this->update_flags_out[reg_offset] = 1;
}
void vcim::set_fader_kd(uint8_t val)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_FADER_KD;
  this->vcim_register.registers.fader_kd = val;
  this->update_flags_out[reg_offset] = 1;
}
void vcim::set_fader_pid_max_cycles(uint8_t val)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_FADER_PID_MAX_CYCLE;
  this->vcim_register.registers.fader_pid_max_cycles = val;
  this->update_flags_out[reg_offset] = 1;
}
void vcim::set_fader_positions(uint8_t position, uint8_t val)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_FADER_POSITION + position;
  this->vcim_register.registers.fader_positions[position] = val;
  this->update_flags_out[reg_offset] = 1;
}

void vcim::set_button_states(uint8_t position, uint8_t val)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_BUTTON_STATES;
  this->vcim_register.registers.button_states[position] = val;
  this->update_flags_out[reg_offset + position] = 1;
}
void vcim::set_button_colours(uint8_t position, colour val)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_BUTTON_COLOURS;
  this->vcim_register.registers.button_colours[position] = val;
  this->update_flags_out[reg_offset + position*3] = 1;
  this->update_flags_out[reg_offset + position*3 + 1] = 1;
  this->update_flags_out[reg_offset + position*3 + 2] = 1;
}
void vcim::set_encoder_postitions(uint8_t position, uint8_t val)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_ENCODER_POSITIONS;
  this->vcim_register.registers.encoder_postitions[position] = val;
  this->update_flags_out[reg_offset + position] = 1;
}
void vcim::set_encoder_button_states(uint8_t position, uint8_t val)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_ENCODER_BUTTON_STATES;
  this->vcim_register.registers.encoder_button_states[position] = val;
  this->update_flags_out[reg_offset + position] = 1;
}
void vcim::set_ring_light_max_brightness(uint8_t val)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_RING_LIGHT_MAX_BRIGHTNESS;
  this->vcim_register.registers.ring_light_max_brightness = val;
  this->update_flags_out[reg_offset] = 1;
}

uint8_t vcim::check_update_fader_positions(uint8_t position)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_FADER_POSITION + position;
  uint8_t return_val = this->update_flags_in[reg_offset];
  this->update_flags_in[reg_offset] = 0;
  return return_val;
}
uint8_t vcim::check_update_button_states(uint8_t position)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_BUTTON_STATES;
  uint8_t return_val = this->update_flags_in[reg_offset + position];
  this->update_flags_in[reg_offset + position] = 0;
  return return_val;
}
uint8_t vcim::check_update_encoder_postitions(uint8_t position)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_ENCODER_POSITIONS;
  uint8_t return_val = this->update_flags_in[reg_offset + position];
  this->update_flags_in[reg_offset + position] = 0;
  return return_val;
}
uint8_t vcim::check_update_encoder_button_states(uint8_t position)
{
  uint8_t reg_offset = VCIM_REG_OFFSET_ENCODER_BUTTON_STATES;
  uint8_t return_val = this->update_flags_in[reg_offset + position];
  this->update_flags_in[reg_offset + position] = 0;
  return return_val;
}

uint8_t vcim::get_fader_positions(uint8_t position)
{
  return this->vcim_register.registers.fader_positions[position];
}
uint8_t vcim::get_button_states(uint8_t position)
{
  return this->vcim_register.registers.button_states[position];
}
uint8_t vcim::get_encoder_postitions(uint8_t position)
{
  return this->vcim_register.registers.encoder_postitions[position];
}
uint8_t vcim::get_encoder_button_states(uint8_t position)
{
  return this->vcim_register.registers.encoder_button_states[position];
}

