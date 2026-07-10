#include "cst3530_touchscreen.h"

namespace esphome::cst3530 {

static const char *const TAG = "cst3530";

// 32-bit register read: write the four address bytes with no stop (repeated
// start), then read. Uses the bus directly because I2CDevice::write() has no
// stop parameter.
bool CST3530Touchscreen::read_reg32_(uint32_t reg, uint8_t *data, uint8_t len) {
  uint8_t addr[4] = {(uint8_t) (reg >> 24), (uint8_t) (reg >> 16), (uint8_t) (reg >> 8), (uint8_t) (reg & 0xFF)};
  if (this->bus_->write(this->address_, addr, 4, false) != i2c::ERROR_OK)
    return false;
  return this->bus_->read(this->address_, data, len) == i2c::ERROR_OK;
}

// 32-bit register write with no payload (used to ack / end a read).
bool CST3530Touchscreen::write_reg32_(uint32_t reg) {
  uint8_t addr[4] = {(uint8_t) (reg >> 24), (uint8_t) (reg >> 16), (uint8_t) (reg >> 8), (uint8_t) (reg & 0xFF)};
  return this->write(addr, 4) == i2c::ERROR_OK;
}

void CST3530Touchscreen::setup() {
  // CST3530 reset is active-low: hold low, release, then wait for boot.
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(false);  // assert reset
    delay(10);
    this->reset_pin_->digital_write(true);  // release
    this->set_timeout(200, [this] { this->continue_setup_(); });
  } else {
    this->continue_setup_();
  }
}

void CST3530Touchscreen::continue_setup_() {
  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->setup();
    this->attach_interrupt_(this->interrupt_pin_, gpio::INTERRUPT_FALLING_EDGE);
  }

  // Default raw range to the display's native size unless an explicit
  // calibration block was supplied (x_raw_min == x_raw_max means "unset").
  if (this->x_raw_max_ == this->x_raw_min_)
    this->x_raw_max_ = this->display_->get_native_width();
  if (this->y_raw_max_ == this->y_raw_min_)
    this->y_raw_max_ = this->display_->get_native_height();

  ESP_LOGCONFIG(TAG, "CST3530 touch initialised");
}

void CST3530Touchscreen::update_touches() {
  uint8_t buf[9] = {0};
  if (!this->read_reg32_(CST3530_DATA_REG, buf, 9)) {
    this->status_set_warning();
    return;
  }

  // A valid touch requires a non-zero point count (low nibble of buf[3]) AND
  // a set high nibble in buf[8] (per Waveshare's own driver check).
  uint8_t count = buf[3] & 0x0F;
  if (count == 0 || (buf[8] & 0xF0) == 0x00) {
    this->write_reg32_(CST3530_END_READ_REG);  // ack / release IRQ
    return;
  }

  this->write_reg32_(CST3530_END_READ_REG);  // ack / release IRQ

  // Decode first touch point (12-bit X/Y packed with the high nibbles in buf[7]).
  uint16_t x = ((uint16_t) (buf[7] & 0x0F) << 8) | buf[4];
  uint16_t y = ((uint16_t) (buf[7] & 0xF0) << 4) | buf[5];
  ESP_LOGD(TAG, "touch raw x=%u y=%u (points=%u)", x, y, count);
  this->add_raw_touch_position_(0, x, y);
}

void CST3530Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "CST3530 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  ESP_LOGCONFIG(TAG, "  X raw min/max: %d/%d, Y raw min/max: %d/%d", this->x_raw_min_, this->x_raw_max_,
                this->y_raw_min_, this->y_raw_max_);
}

}  // namespace esphome::cst3530
