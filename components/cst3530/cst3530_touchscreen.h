#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::cst3530 {

// The CST3530 uses 32-bit (4-byte) register addressing.
static const uint32_t CST3530_DATA_REG = 0xD0070000;        // main touch data block
static const uint32_t CST3530_END_READ_REG = 0xD00002AB;    // ack / end-of-read (clears IRQ)
static const uint32_t CST3530_COORD_NEXT_REG = 0xD0070900;  // 2nd+ touch points (unused: single-touch)

class CST3530Touchscreen : public touchscreen::Touchscreen, public i2c::I2CDevice {
 public:
  void setup() override;
  void update_touches() override;
  void dump_config() override;

  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  void set_reset_pin(GPIOPin *pin) { this->reset_pin_ = pin; }

 protected:
  void continue_setup_();
  bool read_reg32_(uint32_t reg, uint8_t *data, uint8_t len);
  bool write_reg32_(uint32_t reg);

  InternalGPIOPin *interrupt_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
};

}  // namespace esphome::cst3530
