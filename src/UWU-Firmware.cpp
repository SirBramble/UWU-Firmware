#include <Arduino.h>

#include "modules.h"
#include "PCIM_fader.h"
#include "VCIM_fader.h"
#include "ACOM_fader.h"
#include "hidInterface.h"
#include "lighting.h"
#include "i2cinterface.h"

//#define SERIAL_DEBUG


/////////////////////////////////////////////// Choose your base ///////////////////////////////////////////////
//#define IS_MACRUWU
//#define IS_KEYBOARD
//#define IS_VMI    // Voicemeeter Interface

#define MACRUWU_VERSION 2

#define ADDRESS_NUMPAD1 0x20
#define ADDRESS_Bluetooth 0x69

#define ADDRESS_PCIM1 0x11
#define PCIM1_UPDATE_GPIO 10
#define PCIM1_UPDATE_INTERVAL -1

#define ADDRESS_PCIM2 0x10
#define PCIM2_UPDATE_GPIO 9
#define PCIM2_UPDATE_INTERVAL -1

#define ADDRESS_PCIM3 0x12
#define PCIM3_UPDATE_GPIO 8
#define PCIM3_UPDATE_INTERVAL -1

#define ADDRESS_PCIM4 0x13
#define PCIM4_UPDATE_GPIO 7
#define PCIM4_UPDATE_INTERVAL -1

#define ADDRESS_PCIM5 0x14
#define PCIM5_UPDATE_GPIO 6
#define PCIM5_UPDATE_INTERVAL -1

#define ADDRESS_VCIM1 0x20
#define VCIM1_UPDATE_GPIO 11
#define VCIM1_UPDATE_INTERVAL -1

#define ADDRESS_ACOM1 0x30
#define ACOM1_UPDATE_GPIO 12
#define ACOM1_UPDATE_INTERVAL -1

uint8_t registered_I2C_addresses[] = {ADDRESS_NUMPAD1,ADDRESS_Bluetooth};

#ifdef IS_MACRUWU
_macruwu macruwu("macruwu", MACRUWU_VERSION);
RGB_LIGHTING lighting_macruwu(29, 32, 50);    // RGB_LIGHTING(uint8_t led_pin, uint16_t led_count, uint8_t led_max_brightness)
#endif

#ifdef IS_KEYBOARD
RGB_LIGHTING lighting_keyboard(1, 88, 50);    // RGB_LIGHTING(uint8_t led_pin, uint16_t led_count, uint8_t led_max_brightness)
_keyboard keyboard("keyboard");
_numpad numpad1("numpad1", ADDRESS_NUMPAD1);
#endif

#ifdef IS_VMI
pcim pcim1("pcim1", ADDRESS_PCIM1, PCIM1_UPDATE_GPIO, PCIM1_UPDATE_INTERVAL);
pcim pcim2("pcim2", ADDRESS_PCIM2, PCIM2_UPDATE_GPIO, PCIM2_UPDATE_INTERVAL);
pcim pcim3("pcim3", ADDRESS_PCIM3, PCIM3_UPDATE_GPIO, PCIM3_UPDATE_INTERVAL);
pcim pcim4("pcim4", ADDRESS_PCIM4, PCIM4_UPDATE_GPIO, PCIM4_UPDATE_INTERVAL);
pcim pcim5("pcim5", ADDRESS_PCIM5, PCIM5_UPDATE_GPIO, PCIM5_UPDATE_INTERVAL);

vcim vcim1("vcim1", ADDRESS_VCIM1, VCIM1_UPDATE_GPIO, VCIM1_UPDATE_INTERVAL);

acom acom1("acom1", ADDRESS_ACOM1, ACOM1_UPDATE_GPIO, ACOM1_UPDATE_INTERVAL);
#endif

i2cInterface i2c(registered_I2C_addresses, sizeof(registered_I2C_addresses), 4, 5);
hidInterface hid(1000, ADDRESS_Bluetooth);

uint64_t prev;
uint8_t error;




#ifdef IS_MACRUWU
inline void handleMacruwu();
#endif

#ifdef IS_KEYBOARD
inline void handleKeyboard();
inline void handleNumpad();
#endif

#ifdef IS_VMI
void handlePCIM(pcim *pcimx);
void handleVCIM(vcim *vcimx);
void handleACOM(acom *acomx);
#endif

inline void scanI2C();
inline void check_fs_update();


void setup() {
  Serial.begin(115200);
  Serial.println("uwu Setup 1");

  filesystemSetup();
  Serial.println("uwu Setup 2");

  filesystemCreateConfig();
  Serial.println("uwu Setup 3");

  hid.init();

  Serial.println("uwu Setup 4");

  Serial.println("uwu Setup 5");

  #ifndef IS_VMI
  i2c.init();     // REMEMBER: if you update the files, you need to disable the setting of the i2C pins. This will cause the controller to crash
  #else
  Wire.begin();
  Wire.setClock(100000);
  #endif

  #ifdef IS_KEYBOARD
  lighting_keyboard.setup();
  lighting_keyboard.set_led_remap(keyboard.get_led_remap(), keyboard.get_ammount_keys());
  keyboard.init();
  numpad1.init(&i2c);
  #endif

  #ifdef IS_MACRUWU
  lighting_macruwu.setup();
  lighting_macruwu.set_led_remap(macruwu.get_led_remap(), macruwu.get_ammount_keys());
  macruwu.init(); // Dependant on I2C!! Call after i2c.init()!!
  macruwu.updateKeymapsFromFile();
  #endif

  #ifdef IS_VMI
  pcim1.init();
  pcim1.set_fader_pid_max_cycles(20);
  pcim1.send_update();
  pcim2.init();
  pcim2.set_fader_pid_max_cycles(20);
  pcim2.send_update();
  pcim3.init();
  pcim3.set_fader_pid_max_cycles(20);
  pcim3.send_update();
  pcim4.init();
  pcim4.set_fader_pid_max_cycles(20);
  pcim4.send_update();
  pcim5.init();
  pcim5.set_fader_pid_max_cycles(20);
  pcim5.send_update();

  vcim1.init();
  vcim1.set_fader_pid_max_cycles(20);
  vcim1.send_update();

  acom1.init();
  acom1.set_fader_pid_max_cycles(20);
  acom1.send_update();
  #endif

  if(hid.bluetooth_mode == 0){
    error = i2c.disableESP(ADDRESS_Bluetooth);
  }

  prev = 0;
}



/////////////////////////////////////////////////////////////// Loop ///////////////////////////////////////////////////////////////

void loop() {

  check_fs_update();    // remember to activate the rest of the modules   edit: I did

  #ifndef IS_VMI
  scanI2C();
  #endif

  #ifdef IS_KEYBOARD
  lighting_keyboard.update();
  lighting_keyboard.set_effect(keyboard.getLayerLightingEffect());

  handleKeyboard();
  handleNumpad();
  #endif

  #ifdef IS_MACRUWU
  lighting_macruwu.update();
  lighting_macruwu.set_effect(macruwu.getLayerLightingEffect());

  handleMacruwu();
  #endif

  #ifdef IS_VMI
  handlePCIM(&pcim1);
  handlePCIM(&pcim2);
  handlePCIM(&pcim3);
  handlePCIM(&pcim4);
  handlePCIM(&pcim5);
  handleVCIM(&vcim1);
  handleACOM(&acom1);
  #endif

  hid.clear_midi_CC_update_available();

  for(int i = 0; i < 50; i++)    // get the FRESHEST midi data in town! It's not a bug, IT'S A FEATURE! Get it while it's hot!
  {
    hid.readMidi();
  }
  delay(1);
}

/////////////////////////////////////////////////////////////// Macruwu Handler ///////////////////////////////////////////////////////////////
#ifdef IS_MACRUWU
inline void handleMacruwu(){
  macruwu.update();

  for(int i = 0; i < 32; i++){
    key *currentKey = macruwu.getKeyPointer(i + 1);

    if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey)){
      if(hid.get_midi_CC_state(currentKey) ){
        lighting_macruwu.enable_override(i, currentKey->color);
        hid.set_midi_CC_update_handled(currentKey);
      }
      else {
        lighting_macruwu.disable_override(i);
        hid.set_midi_CC_update_handled(currentKey);
      }
    }

    if(macruwu.isPressed(i)){
      if(currentKey->hasLayerChange) macruwu.setLayer(currentKey->changeToLayer);
      
      if(currentKey->isMIDI) hid.sendMidi_Analog(macruwu.getKeyPointer(i + 1), 255);
      
      else Serial.printf("pressing index: %d\n", i); hid.press(currentKey);
      
      switch(currentKey->color_mode){
        case no_override: break;
        case midi_bound: break;
        case pressed: lighting_macruwu.enable_override(i, currentKey->color); break;
        case not_pressed: lighting_macruwu.disable_override(i); break;
        case toggle: lighting_macruwu.toggle_override(i, currentKey->color); break;
        case disabled: lighting_macruwu.disable_override(i); break;
      }
    }
    else if(macruwu.isReleased(i)){
      if(currentKey->isMIDI) hid.sendMidi_Analog(macruwu.getKeyPointer(i + 1), 0);
      
      else Serial.println("releasing"); hid.release(macruwu.getKeyPointer(i + 1));
      

      switch(currentKey->color_mode){
        case no_override: break;
        case midi_bound: break;
        case pressed: lighting_macruwu.disable_override(i); break;
        case not_pressed: lighting_macruwu.enable_override(i, currentKey->color); break;
        case toggle: break;
        case disabled: lighting_macruwu.disable_override(i); break;
      }
    }
  }
}
#endif

/////////////////////////////////////////////////////////////// Keyboard Handler ///////////////////////////////////////////////////////////////
#ifdef IS_KEYBOARD
inline void handleKeyboard(){
  keyboard.update();

  for(int i = 0; i < AMMOUNT_KEYS; i++){
    key *currentKey = keyboard.getKeyPointer(i + 1);

    if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey)){
      if(hid.get_midi_CC_state(currentKey)){
        lighting_keyboard.enable_override(i, currentKey->color);
        hid.set_midi_CC_update_handled(currentKey);
      }
      else {
        hid.set_midi_CC_update_handled(currentKey);
        lighting_keyboard.disable_override(i);
      }
    }

    if(currentKey->color_mode == const_color) lighting_keyboard.enable_override(i, currentKey->color);

    if(currentKey->color_mode == disabled) lighting_keyboard.enable_override(i, 0);

    if(keyboard.isPressed(i)){
      if(currentKey->hasLayerChange) keyboard.setLayer(currentKey->changeToLayer);

      if(currentKey->isMIDI) hid.sendMidi_Analog(keyboard.getKeyPointer(i + 1), 255);
      
      else Serial.println("pressing"); hid.press(currentKey);
      
      switch(currentKey->color_mode){
        case no_override: break;
        case midi_bound: break;
        case pressed: lighting_keyboard.enable_override(i, currentKey->color); break;
        case not_pressed: lighting_keyboard.disable_override(i); break;
        case toggle: lighting_keyboard.toggle_override(i, currentKey->color); break;
        case disabled: lighting_keyboard.disable_override(i); break;
      }
    }
    else if(keyboard.isReleased(i)){
      if(currentKey->isMIDI) hid.sendMidi_Analog(keyboard.getKeyPointer(i + 1), 0);
      
      else Serial.println("releasing"); hid.release(keyboard.getKeyPointer(i + 1));

      switch(currentKey->color_mode){
        case no_override: break;
        case midi_bound: break;
        case pressed: lighting_keyboard.disable_override(i); break;
        case not_pressed: lighting_keyboard.enable_override(i, currentKey->color); break;
        case toggle: break;
        case disabled: lighting_keyboard.disable_override(i);
      }
    }
  }
}
#endif

/////////////////////////////////////////////////////////////// Numpad Handler ///////////////////////////////////////////////////////////////

#ifdef IS_KEYBOARD
inline void handleNumpad(){
  
  numpad1.update();

  if(numpad1.registered()){
    for(int i = 0; i < I2C_BUFFER_SIZE; i++){
      key *currentKey = numpad1.getKeyPointer(i + 1);
      if(currentKey == nullptr) continue;

      // Key color handler
      if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey)){
        if (hid.get_midi_CC_state(currentKey)) {
          i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), true);
          hid.set_midi_CC_update_handled(currentKey);
        }
        else {
          i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false);
          hid.set_midi_CC_update_handled(currentKey);
        }
      }

      // Update Fader values
      if(currentKey->isMIDI && currentKey->isAnalog){         //don't forget to compare val with previous val when setting. Yes mum...
        hid.sendMidi_Analog(numpad1.getKeyPointer(i + 1), map(i2c.getVal(i + 1, numpad1.address()), 0, 255, 0, 127));
      }

      else if(numpad1.isPressed_hold(i)){
        if(currentKey->hasLayerChange){
          numpad1.setLayer(currentKey->changeToLayer);
        }
        if(currentKey->isMIDI){
          hid.sendMidi_Digital(numpad1.getKeyPointer(i + 1), true);
        }
        else{
          Serial.println("pressing");
          hid.press(numpad1.getKeyPointer(i + 1));
        }
        switch(currentKey->color_mode){
          case no_override: break;
          case midi_bound: break;
          case pressed: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), true); break;
          case not_pressed: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false); break;
          case toggle: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false); break;
          case disabled: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false); break;
        }
      }
      else if(numpad1.isReleased_hold(i)){
        if(currentKey->isMIDI) hid.sendMidi_Digital(numpad1.getKeyPointer(i + 1), false);

        else Serial.println("releasing"); hid.release(numpad1.getKeyPointer(i + 1));

        switch(currentKey->color_mode){
          case no_override: break;
          case midi_bound: break;
          case pressed: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false); break;
          case not_pressed: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), true); break;
          case toggle: break;
          case disabled: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false); break;
        }
      }
      
    }
  }
}
#endif
/////////////////////////////////////////////////////////////// PCIM handler ///////////////////////////////////////////////////////////////
#ifdef IS_VMI
void handlePCIM(pcim *pcimx)
{
  if(pcimx == NULL)
  {
    return;
  }
  //-----------------Update From PC-----------------------

  bool send_update_flag = 0;

  // fader handler
  for(int i = 0; i < 1; i++)
  {
    key *currentKey = pcimx->getKeyPointer(i + 1);
    if(currentKey == NULL) continue;

    if((currentKey->isMIDI && currentKey->isAnalog) && hid.get_midi_CC_update_available(currentKey))
    {
      Serial.printf("got MIDI update with update available = %d\n", hid.get_midi_CC_update_available(currentKey));
      uint8_t fader_send_val = hid.get_midi_CC_state(currentKey);     
      fader_send_val = map(fader_send_val, 0, 127, 0, PCIM_FADER_TOP_LIMMIT);
      pcimx->set_fader_position(fader_send_val);
      Serial.printf("set fader to: %d\n", fader_send_val);
      send_update_flag = 1;
      //hid.set_midi_CC_update_handled(currentKey);
      hid.force_clear_midi_CC_update_available(currentKey);
    }
  }

  // encoder handler
  for(int i = 0; i < 7; i++)
  {
    key *currentKey = pcimx->getKeyPointer(i + 2);
    if(currentKey == NULL) continue;

    if((currentKey->isMIDI && currentKey->isAnalog) && hid.get_midi_CC_update_available(currentKey))
    {
      Serial.printf("got MIDI update with update available = %d\n", hid.get_midi_CC_update_available(currentKey));
      pcimx->set_encoder_postitions(i, map(hid.get_midi_CC_state(currentKey), 0, 127, 0, 255));
      Serial.printf("set encoder to: %d\n", map(hid.get_midi_CC_state(currentKey), 0, 127, 0, 255));
      send_update_flag = 1;
      //hid.set_midi_CC_update_handled(currentKey);
      hid.force_clear_midi_CC_update_available(currentKey);
    }
  }

  // encoder post button handler
  for(int i = 0; i < 4; i++)
  {
    key *currentKey = pcimx->getKeyPointer(i + 16);
    if(currentKey == NULL) continue;

    // Key color handler
    if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey))
    {
      
      if (hid.get_midi_CC_state(currentKey))
      {
        Serial.printf("Setting post button colour to: %d\n", currentKey->color);
        colour tmp;
        tmp.r = currentKey->color >> 16;
        tmp.g = currentKey->color >> 8;
        tmp.b = currentKey->color;
        pcimx->set_encoder_post_button_colours(i, tmp);
        hid.force_clear_midi_CC_update_available(currentKey);
      }
      else
      {
        colour tmp;
        tmp.r = 0;
        tmp.g = 0;
        tmp.b = 0;
        pcimx->set_encoder_post_button_colours(i, tmp);
        hid.force_clear_midi_CC_update_available(currentKey);
      }
      send_update_flag = 1;
    }
  }

  // encoder eq button handler
  for(int i = 0; i < 1; i++)
  {
    key *currentKey = pcimx->getKeyPointer(i + 20);
    if(currentKey == NULL) continue;

    // Key color handler
    if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey))
    {
      
      if (hid.get_midi_CC_state(currentKey))
      {
        Serial.printf("Setting eq button colour to: %d\n", currentKey->color);
        colour tmp;
        tmp.r = currentKey->color >> 16;
        tmp.g = currentKey->color >> 8;
        tmp.b = currentKey->color;
        pcimx->set_encoder_eq_button_colour(tmp);
        hid.force_clear_midi_CC_update_available(currentKey);
      }
      else
      {
        colour tmp;
        tmp.r = 0;
        tmp.g = 0;
        tmp.b = 0;
        pcimx->set_encoder_eq_button_colour(tmp);
        hid.force_clear_midi_CC_update_available(currentKey);
      }
      send_update_flag = 1;
    }
  }

  // button handler
  for(int i = 0; i < 11; i++)
  {
    key *currentKey = pcimx->getKeyPointer(i + 21);
    if(currentKey == NULL) continue;

    // Key color handler
    if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey))
    {
      
      if (hid.get_midi_CC_state(currentKey))
      {
        Serial.printf("Setting button colour to: %d\n", currentKey->color);
        colour tmp;
        tmp.r = currentKey->color >> 16;
        tmp.g = currentKey->color >> 8;
        tmp.b = currentKey->color;
        pcimx->set_button_colours(10-i, tmp);
        hid.force_clear_midi_CC_update_available(currentKey);
      }
      else
      {
        colour tmp;
        tmp.r = 0;
        tmp.g = 0;
        tmp.b = 0;
        pcimx->set_button_colours(10-i, tmp);
        hid.force_clear_midi_CC_update_available(currentKey);
      }
      send_update_flag = 1;
    }
  }

  if(send_update_flag)
  {
    pcimx->send_update();
    delay(1);
    send_update_flag = 0;
  }


  //-----------------Update From PCIM-----------------------

  if(pcimx->update_available())
  {
    pcimx->get_update();
    Serial.printf("got PCIM update\n");

    //fader handler
    for(int i = 0; i < 1; i++)
    {
      key *currentKey = pcimx->getKeyPointer(i + 1);
      if(currentKey == NULL) continue;

      if(currentKey->isMIDI && currentKey->isAnalog && pcimx->check_update_fader_position())
      {
        uint8_t midi_send_val = pcimx->get_fader_position();
        midi_send_val = (midi_send_val > PCIM_FADER_TOP_LIMMIT) ? PCIM_FADER_TOP_LIMMIT : midi_send_val;   // set top bound
        midi_send_val = map(midi_send_val, 0, PCIM_FADER_TOP_LIMMIT, 0, 127);
        hid.sendMidi_Analog(currentKey, midi_send_val);
      }
    }

    // encoder handler
    for(int i = 0; i < 7; i++)
    {
      key *currentKey = pcimx->getKeyPointer(i + 2);
      if(currentKey == NULL) continue;

      if(currentKey->isMIDI && currentKey->isAnalog && pcimx->check_update_encoder_postitions(i))
      {
        hid.sendMidi_Analog(currentKey, map(pcimx->get_encoder_postitions(i), 0, 255, 0, 127));
      }
    }

    // encoder button handler
    for(int i = 0; i < 7; i++)
    {
      key *currentKey = pcimx->getKeyPointer(i + 9);
      if(currentKey == NULL) continue;

      if(pcimx->check_update_encoder_button_states(i))
      {
        Serial.printf("got encoder Button update\n");

        if(pcimx->get_encoder_button_states(i))
        {
          if(currentKey->hasLayerChange)
          {
            pcimx->setLayer(currentKey->changeToLayer);
          }
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, true);
          }
          else
          {
            Serial.println("pressing");
            hid.press(currentKey);
          }
        }
        else if(!pcimx->get_encoder_button_states(i))
        {
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, false);
          }
          else 
          {
            Serial.println("releasing");
            hid.release(currentKey);
          }
        }
      }
    }


    // post button handler
    for(int i = 0; i < 4; i++)
    {
      key *currentKey = pcimx->getKeyPointer(i + 16);
      if(currentKey == NULL) continue;

      if(pcimx->check_update_encoder_post_button_states(i))
      {
        Serial.printf("got encoder post Button update\n");

        if(pcimx->get_encoder_post_button_states(i))
        {
          if(currentKey->hasLayerChange)
          {
            pcimx->setLayer(currentKey->changeToLayer);
          }
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, true);
          }
          else{
            Serial.println("pressing");
            hid.press(currentKey);
          }
        }
        else if(!pcimx->get_encoder_post_button_states(i))
        {
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, false);
          }

          else
          {
            Serial.println("releasing");
            hid.release(currentKey);
          }
        }
      }
    }

    // eq button handler
    for(int i = 0; i < 1; i++)
    {
      key *currentKey = pcimx->getKeyPointer(i + 20);
      if(currentKey == NULL) continue;

      if(pcimx->check_update_encoder_eq_button_state())
      {
        Serial.printf("got encoder eq Button update\n");

        if(pcimx->get_encoder_eq_button_state())
        {
          if(currentKey->hasLayerChange)
          {
            pcimx->setLayer(currentKey->changeToLayer);
          }
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, true);
          }
          else{
            Serial.println("pressing");
            hid.press(currentKey);
          }
        }
        else if(!pcimx->get_encoder_eq_button_state())
        {
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, false);
          }

          else
          {
            Serial.println("releasing");
            hid.release(currentKey);
          }
        }
      }
    }

    // button handler
    for(int i = 0; i < 11; i++)
    {
      key *currentKey = pcimx->getKeyPointer(i + 21);
      if(currentKey == NULL) continue;

      if(pcimx->check_update_button_states(10-i))
      {
        Serial.printf("got Button update\n");

        if(pcimx->get_button_states(10-i))
        {
          if(currentKey->hasLayerChange)
          {
            pcimx->setLayer(currentKey->changeToLayer);
          }
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, true);
          }
          else{
            Serial.println("pressing");
            hid.press(currentKey);
          }
        }
        else if(!pcimx->get_button_states(10-i))
        {
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, false);
          }

          else
          {
            Serial.println("releasing");
            hid.release(currentKey);
          }
        }
      }
    }

    hid.clear_midi_CC_update_available();
  }
}

void handleVCIM(vcim *vcimx)
{
  if(vcimx == NULL)
  {
    return;
  }
  //-----------------Update From PC-----------------------

  bool send_update_flag = 0;

  // fader handler
  for(int i = 0; i < 3; i++)
  {
    key *currentKey = vcimx->getKeyPointer(i + 1);
    if(currentKey == NULL) continue;

    if((currentKey->isMIDI && currentKey->isAnalog) && hid.get_midi_CC_update_available(currentKey))
    {
      Serial.printf("got MIDI update with update available = %d\n", hid.get_midi_CC_update_available(currentKey));
      uint8_t fader_send_val = hid.get_midi_CC_state(currentKey);     
      fader_send_val = map(fader_send_val, 0, 127, 0, VCIM_FADER_TOP_LIMMIT);
      vcimx->set_fader_positions(i,fader_send_val);
      Serial.printf("set fader to: %d\n", fader_send_val);
      send_update_flag = 1;
      //hid.set_midi_CC_update_handled(currentKey);
      hid.force_clear_midi_CC_update_available(currentKey);
    }
  }

  // encoder handler
  for(int i = 0; i < 9; i++)
  {
    key *currentKey = vcimx->getKeyPointer(i + 4);
    if(currentKey == NULL) continue;

    if((currentKey->isMIDI && currentKey->isAnalog) && hid.get_midi_CC_update_available(currentKey))
    {
      Serial.printf("got MIDI update with update available = %d\n", hid.get_midi_CC_update_available(currentKey));
      vcimx->set_encoder_postitions(i, map(hid.get_midi_CC_state(currentKey), 0, 127, 0, 255));
      Serial.printf("set encoder to: %d\n", map(hid.get_midi_CC_state(currentKey), 0, 127, 0, 255));
      send_update_flag = 1;
      //hid.set_midi_CC_update_handled(currentKey);
      hid.force_clear_midi_CC_update_available(currentKey);
    }
  }

  // button handler
  for(int a = 0; a < 3; a++)
  {
    for(int i = 0; i < 11; i++)
    {
      key *currentKey = vcimx->getKeyPointer(i + 22 + (a*11));
      if(currentKey == NULL) continue;

      // Key color handler
      if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey))
      {
        
        if (hid.get_midi_CC_state(currentKey))
        {
          Serial.printf("Setting button colour to: %d\n", currentKey->color);
          colour tmp;
          tmp.r = currentKey->color >> 16;
          tmp.g = currentKey->color >> 8;
          tmp.b = currentKey->color;
          vcimx->set_button_colours(10-i + (a*11), tmp);
          hid.force_clear_midi_CC_update_available(currentKey);
        }
        else
        {
          colour tmp;
          tmp.r = 0;
          tmp.g = 0;
          tmp.b = 0;
          vcimx->set_button_colours(10-i + (a*11), tmp);
          hid.force_clear_midi_CC_update_available(currentKey);
        }
        send_update_flag = 1;
      }
    }
  }
  

  if(send_update_flag)
  {
    vcimx->send_update();
    delay(1);
    send_update_flag = 0;
  }


  //-----------------Update From VCIM-----------------------

  if(vcimx->update_available())
  {
    vcimx->get_update();
    Serial.printf("got VCIM update\n");

    //fader handler
    for(int i = 0; i < 3; i++)
    {
      key *currentKey = vcimx->getKeyPointer(i + 1);
      if(currentKey == NULL) continue;

      if(currentKey->isMIDI && currentKey->isAnalog && vcimx->check_update_fader_positions(i))
      {
        uint8_t midi_send_val = vcimx->get_fader_positions(i);
        midi_send_val = (midi_send_val > VCIM_FADER_TOP_LIMMIT) ? VCIM_FADER_TOP_LIMMIT : midi_send_val;   // set top bound
        midi_send_val = map(midi_send_val, 0, VCIM_FADER_TOP_LIMMIT, 0, 127);
        hid.sendMidi_Analog(currentKey, midi_send_val);
      }
    }

    // encoder handler
    for(int i = 0; i < 9; i++)
    {
      key *currentKey = vcimx->getKeyPointer(i + 4);
      if(currentKey == NULL) continue;

      if(currentKey->isMIDI && currentKey->isAnalog && vcimx->check_update_encoder_postitions(i))
      {
        hid.sendMidi_Analog(currentKey, map(vcimx->get_encoder_postitions(i), 0, 255, 0, 127));
      }
    }

    // encoder button handler
    for(int i = 0; i < 9; i++)
    {
      key *currentKey = vcimx->getKeyPointer(i + 13);
      if(currentKey == NULL) continue;

      if(vcimx->check_update_encoder_button_states(i))
      {
        Serial.printf("got encoder Button update\n");

        if(vcimx->get_encoder_button_states(i))
        {
          if(currentKey->hasLayerChange)
          {
            vcimx->setLayer(currentKey->changeToLayer);
          }
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, true);
          }
          else
          {
            Serial.println("pressing");
            hid.press(currentKey);
          }
        }
        else if(!vcimx->get_encoder_button_states(i))
        {
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, false);
          }
          else 
          {
            Serial.println("releasing");
            hid.release(currentKey);
          }
        }
      }
    }

    // button handler
    for(int a = 0; a < 3; a++)
    {
      for(int i = 0; i < 11; i++)
      {
        key *currentKey = vcimx->getKeyPointer(i + 22 + (11*a));
        if(currentKey == NULL) continue;

        if(vcimx->check_update_button_states(10-i + (11*a)))
        {
          Serial.printf("got Button update\n");

          if(vcimx->get_button_states(10-i + (11*a)))
          {
            if(currentKey->hasLayerChange)
            {
              vcimx->setLayer(currentKey->changeToLayer);
            }
            if(currentKey->isMIDI)
            {
              hid.sendMidi_Digital(currentKey, true);
            }
            else{
              Serial.println("pressing");
              hid.press(currentKey);
            }
          }
          else if(!vcimx->get_button_states(10-i + (11*a)))
          {
            if(currentKey->isMIDI)
            {
              hid.sendMidi_Digital(currentKey, false);
            }

            else
            {
              Serial.println("releasing");
              hid.release(currentKey);
            }
          }
        }
      }
    }

    hid.clear_midi_CC_update_available();
  }
}

void handleACOM(acom *acomx)
{
  if(acomx == NULL)
  {
    return;
  }
  //-----------------Update From PC-----------------------

  bool send_update_flag = 0;

  // fader handler
  for(int i = 0; i < 8; i++)
  {
    key *currentKey = acomx->getKeyPointer(i + 1);
    if(currentKey == NULL) continue;

    if((currentKey->isMIDI && currentKey->isAnalog) && hid.get_midi_CC_update_available(currentKey))
    {
      Serial.printf("got MIDI update with update available = %d\n", hid.get_midi_CC_update_available(currentKey));
      uint8_t fader_send_val = hid.get_midi_CC_state(currentKey);     
      fader_send_val = map(fader_send_val, 0, 127, 0, ACOM_FADER_TOP_LIMMIT);
      acomx->set_fader_positions(i,fader_send_val);
      Serial.printf("set fader to: %d\n", fader_send_val);
      send_update_flag = 1;
      //hid.set_midi_CC_update_handled(currentKey);
      hid.force_clear_midi_CC_update_available(currentKey);
    }
  }

  // encoder handler
  for(int i = 0; i < 16; i++)
  {
    key *currentKey = acomx->getKeyPointer(i + 9);
    if(currentKey == NULL) continue;

    if((currentKey->isMIDI && currentKey->isAnalog) && hid.get_midi_CC_update_available(currentKey))
    {
      Serial.printf("got MIDI update with update available = %d\n", hid.get_midi_CC_update_available(currentKey));
      acomx->set_encoder_postitions(i, map(hid.get_midi_CC_state(currentKey), 0, 127, 0, 255));
      Serial.printf("set encoder to: %d\n", map(hid.get_midi_CC_state(currentKey), 0, 127, 0, 255));
      send_update_flag = 1;
      //hid.set_midi_CC_update_handled(currentKey);
      hid.force_clear_midi_CC_update_available(currentKey);
    }
  }

  // button handler
  for(int i = 0; i < 32; i++)
  {
    key *currentKey = acomx->getKeyPointer(i + 41);
    if(currentKey == NULL) continue;

    // Key color handler
    if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey))
    {
      
      if (hid.get_midi_CC_state(currentKey))
      {
        Serial.printf("Setting button colour to: %d\n", currentKey->color);
        colour tmp;
        tmp.r = currentKey->color >> 16;
        tmp.g = currentKey->color >> 8;
        tmp.b = currentKey->color;
        acomx->set_button_colours(i, tmp);
        hid.force_clear_midi_CC_update_available(currentKey);
      }
      else
      {
        colour tmp;
        tmp.r = 0;
        tmp.g = 0;
        tmp.b = 0;
        acomx->set_button_colours(i, tmp);
        hid.force_clear_midi_CC_update_available(currentKey);
      }
      send_update_flag = 1;
    }
  }
  

  if(send_update_flag)
  {
    acomx->send_update();
    delay(1);
    send_update_flag = 0;
  }


  //-----------------Update From ACOM-----------------------

  if(acomx->update_available())
  {
    acomx->get_update();
    Serial.printf("got ACOM update\n");

    //fader handler
    for(int i = 0; i < 8; i++)
    {
      key *currentKey = acomx->getKeyPointer(i + 1);
      if(currentKey == NULL) continue;

      if(currentKey->isMIDI && currentKey->isAnalog && acomx->check_update_fader_positions(i))
      {
        uint8_t midi_send_val = acomx->get_fader_positions(i);
        midi_send_val = (midi_send_val > ACOM_FADER_TOP_LIMMIT) ? ACOM_FADER_TOP_LIMMIT : midi_send_val;   // set top bound
        midi_send_val = map(midi_send_val, 0, ACOM_FADER_TOP_LIMMIT, 0, 127);
        hid.sendMidi_Analog(currentKey, midi_send_val);
      }
    }

    // encoder handler
    for(int i = 0; i < 16; i++)
    {
      key *currentKey = acomx->getKeyPointer(i + 9);
      if(currentKey == NULL) continue;

      if(currentKey->isMIDI && currentKey->isAnalog && acomx->check_update_encoder_postitions(i))
      {
        hid.sendMidi_Analog(currentKey, map(acomx->get_encoder_postitions(i), 0, 255, 0, 127));
      }
    }

    // encoder button handler
    for(int i = 0; i < 16; i++)
    {
      key *currentKey = acomx->getKeyPointer(i + 25);
      if(currentKey == NULL) continue;

      if(acomx->check_update_encoder_button_states(i))
      {
        Serial.printf("got encoder Button update\n");

        if(acomx->get_encoder_button_states(i))
        {
          if(currentKey->hasLayerChange)
          {
            acomx->setLayer(currentKey->changeToLayer);
          }
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, true);
          }
          else
          {
            Serial.println("pressing");
            hid.press(currentKey);
          }
        }
        else if(!acomx->get_encoder_button_states(i))
        {
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, false);
          }
          else 
          {
            Serial.println("releasing");
            hid.release(currentKey);
          }
        }
      }
    }

    // button handler
    for(int i = 0; i < 32; i++)
    {
      key *currentKey = acomx->getKeyPointer(i + 41);
      if(currentKey == NULL) continue;

      if(acomx->check_update_button_states(i))
      {
        Serial.printf("got Button update\n");

        if(acomx->get_button_states(i))
        {
          if(currentKey->hasLayerChange)
          {
            acomx->setLayer(currentKey->changeToLayer);
          }
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, true);
          }
          else{
            Serial.println("pressing");
            hid.press(currentKey);
          }
        }
        else if(!acomx->get_button_states(i))
        {
          if(currentKey->isMIDI)
          {
            hid.sendMidi_Digital(currentKey, false);
          }

          else
          {
            Serial.println("releasing");
            hid.release(currentKey);
          }
        }
      }
    }

    hid.clear_midi_CC_update_available();
  }
}

#endif

/////////////////////////////////////////////////////////////// I2C scanner ///////////////////////////////////////////////////////////////

inline void scanI2C(){
  if(millis() > prev + 1000){
    prev = millis();
    #ifdef IS_KEYBOARD
    i2c.probe();
    #endif
    #ifdef SERIAL_DEBUG
      Serial.println("uwu");
      if(error == 0) Serial.println("ESP disabled successfully");
      else Serial.print("ESP has given error on disable: ");Serial.println(error);
    #endif
  }
}


/////////////////////////////////////////////////////////////// Filesystem Update Handler ///////////////////////////////////////////////////////////////

inline void check_fs_update(){
  if(check_fs_changed()){
    #ifdef IS_KEYBOARD
    keyboard.updateKeymapsFromFile();
    numpad1.updateKeymapsFromFile();
    for(int i = 0; i < numpad1.ammountKeys; i++){
      key *currentKey = numpad1.getKeyPointer(i);
      if(currentKey->color_mode != no_override){
        i2c.set_led_override(numpad1.address(), numpad1.remap_led_key(i), currentKey->color);
      }
    }
    #endif

    #ifdef IS_MACRUWU
    macruwu.updateKeymapsFromFile();
    Serial.printf("current Layer: %d\n", macruwu.current_layer);
    #endif

    #ifdef IS_VMI
    pcim1.updateKeymapsFromFile();
    pcim2.updateKeymapsFromFile();
    pcim3.updateKeymapsFromFile();
    pcim4.updateKeymapsFromFile();
    pcim5.updateKeymapsFromFile();
    vcim1.updateKeymapsFromFile();
    acom1.updateKeymapsFromFile();
    #endif

    
    
    set_fs_changed(false);
  }
}
