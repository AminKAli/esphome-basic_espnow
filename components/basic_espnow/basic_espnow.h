#pragma once

#include "esphome/core/component.h"
#include "esp_now.h"
#include <vector>
#include <functional>
#include <array>

namespace esphome {
namespace basic_espnow {

typedef std::function<void(const std::string &, const std::array<uint8_t, 6> &)> espnow_cb_t;

class BasicESPNow : public Component {
 public:
  BasicESPNow();

  void setup() override;
  void loop() override {};

  void add_peer(const std::array<uint8_t, 6> &mac);

  void send_message(const std::string &message, const std::array<uint8_t, 6> &mac_address);
  void send_broadcast(const std::string &message);

  void on_message(espnow_cb_t callback);

 protected:
  static void on_espnow_recv_(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len);
  static BasicESPNow *instance_;

  std::vector<espnow_cb_t> callbacks_;
  std::vector<std::array<uint8_t, 6>> pending_peers_;
};

}  // namespace basic_espnow
}  // namespace esphome
