#pragma once

#include "main.h"

#include <array>
#include <cstdint>

#include "tutrcos/peripheral/gpio.hpp"
#include "tutrcos/peripheral/spi.hpp"

#include "encoder_base.hpp"

namespace tutrcos {
namespace module {

class AMT22 : public EncoderBase {
public:
  enum class Resolution : uint8_t {
    _12 = 12,
    _14 = 14,
  };

  enum class Type {
    SINGLE_TURN,
    MULTI_TURN,
  };

  AMT22(peripheral::SPI &spi, peripheral::GPIO &cs, Resolution resolution,
        Type type)
      : EncoderBase{1 << utility::to_underlying(resolution)}, spi_{spi},
        cs_{cs}, resolution_{resolution}, type_{type} {}

  bool update() {
    uint16_t cpr = 1 << utility::to_underlying(resolution_);
    switch (type_) {
    case Type::SINGLE_TURN: {
      std::array<uint8_t, 2> command{0x00, 0x00};
      uint16_t response;
      if (!send_command(command.data(), reinterpret_cast<uint8_t *>(&response),
                        command.size())) {
        return false;
      }

      int16_t count = response & (cpr - 1);
      int16_t delta = count - prev_count_;
      if (delta > (cpr / 2)) {
        delta -= cpr;
      } else if (delta < -(cpr / 2)) {
        delta += cpr;
      }
      set_count(get_count() + delta);
      prev_count_ = count;
      break;
    }
    case Type::MULTI_TURN: {
      std::array<uint8_t, 4> command{0x00, 0xA0, 0x00, 0x00};
      std::array<uint16_t, 2> response{};
      if (!send_command(command.data(),
                        reinterpret_cast<uint8_t *>(response.data()),
                        command.size())) {
        return false;
      }

      int16_t count = response[1] & (cpr - 1);
      int16_t rotation = response[0] & ((1 << 14) - 1);
      set_count(rotation * cpr + count);
      break;
    }
    }
    return true;
  }

  bool set_zero_point() {
    std::array<uint8_t, 2> command{0x00, 0x70};
    std::array<uint8_t, 2> response{};
    if (!send_command(command.data(), response.data(), command.size())) {
      return false;
    }
    prev_count_ = 0;
    set_count(0);
    return true;
  }

private:
  peripheral::SPI &spi_;
  peripheral::GPIO &cs_;
  Resolution resolution_;
  Type type_;
  int16_t prev_count_ = 0;

  bool send_command(const uint8_t *command, uint8_t *response, size_t size) {
    cs_.write(false);
    for (size_t i = 0; i < size; ++i) {
      if (!spi_.transmit_receive(&command[i], &response[size - i - 1], 1, 1)) {
        return false;
      }
    }
    cs_.write(true);
    for (size_t i = 0; i < size; i += 2) {
      if (!checksum(response[i], response[i + 1])) {
        return false;
      }
    }
    return true;
  }

  bool checksum(uint8_t l, uint8_t h) {
    bool k1 = !(bit(h, 5) ^ bit(h, 3) ^ bit(h, 1) ^ bit(l, 7) ^ bit(l, 5) ^
                bit(l, 3) ^ bit(l, 1));
    bool k0 = !(bit(h, 4) ^ bit(h, 2) ^ bit(h, 0) ^ bit(l, 6) ^ bit(l, 4) ^
                bit(l, 2) ^ bit(l, 0));
    return (k1 == bit(h, 7)) && (k0 == bit(h, 6));
  }

  bool bit(uint8_t x, uint8_t i) { return ((x >> i) & 1) == 1; }
};

} // namespace module
} // namespace tutrcos
