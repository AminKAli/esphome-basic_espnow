#include "basic_espnow.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include <cstring>


namespace esphome {
namespace basic_espnow {

static const char *const TAG = "basic_espnow";

BasicESPNow *BasicESPNow::instance_ = nullptr;

BasicESPNow::BasicESPNow() = default;

uint32_t counter = 0;

void BasicESPNow::setup() {
  printf("STARTING SETUP ESPNOW BASIC\n");
  instance_ = this;
  esp_log_level_set(TAG, ESP_LOG_VERBOSE);
  ESP_LOGI(TAG, "Deferring esp_now_init() with set_timeout");

  App.scheduler.set_timeout(this, "espnow_init", 100, [this]() {
    ESP_LOGI(TAG, "Initializing ESP-NOW");

    esp_err_t err = esp_now_init();
    if (err != ESP_OK && err != ESP_ERR_ESPNOW_EXIST) {
      ESP_LOGE(TAG, "esp_now_init failed: %s", esp_err_to_name(err));
      return;
    }

    esp_now_register_recv_cb(BasicESPNow::on_espnow_recv_);

    esp_now_peer_info_t broadcast_peer = {};
    memset(broadcast_peer.peer_addr, 0xFF, 6);
    broadcast_peer.channel = 0;
    broadcast_peer.ifidx = WIFI_IF_STA;
    broadcast_peer.encrypt = false;
    esp_now_add_peer(&broadcast_peer);

    for (const auto &mac : this->pending_peers_) {
      esp_now_peer_info_t peer = {};
      memcpy(peer.peer_addr, mac.data(), 6);
      peer.channel = 0;
      peer.ifidx = WIFI_IF_STA;
      peer.encrypt = false;

      if (!esp_now_is_peer_exist(peer.peer_addr)) {
        esp_err_t err = esp_now_add_peer(&peer);
        if (err == ESP_OK) {
          ESP_LOGI(TAG, "Added peer %02X:%02X:%02X:%02X:%02X:%02X",
                   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        } else {
          ESP_LOGW(TAG, "Failed to add peer: %s", esp_err_to_name(err));
        }
      }
    }

    ESP_LOGI(TAG, "ESP-NOW setup complete");
  });
}

void BasicESPNow::add_peer(const std::array<uint8_t, 6> &mac) {
  pending_peers_.push_back(mac);
}

void BasicESPNow::send_message(const std::string &message, const std::array<uint8_t, 6> &mac_address) {
  if (!esp_now_is_peer_exist(mac_address.data())) {
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, mac_address.data(), 6);
    peer.channel = 0;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
  } else {
  }

  esp_now_send(mac_address.data(), reinterpret_cast<const uint8_t *>(message.data()+counter), message.size()+4);
}

void BasicESPNow::send_broadcast(const std::string &message) {
  uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_send(broadcast, reinterpret_cast<const uint8_t *>(message.data()), message.size());
}

void BasicESPNow::on_message(espnow_cb_t callback) {
  callbacks_.push_back(callback);
}

void BasicESPNow::on_espnow_recv_(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  if (!instance_ || !recv_info || !data || len <= 0) return;

  std::array<uint8_t, 6> mac;
  memcpy(mac.data(), recv_info->src_addr, 6);
  std::string msg(reinterpret_cast<const char *>(data), len);

  ESP_LOGD("basic_espnow", "Recv from %02X:%02X:%02X:%02X:%02X:%02X RSSI: %d",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], recv_info->rssi);

  for (auto &cb : instance_->callbacks_) {
    cb(msg, mac);
  }
}


// void BasicESPNow::on_espnow_recv_(const uint8_t *mac_addr, const uint8_t *data, int len) {
//   ESP_LOGI(TAG, "Received ESP-NOW packet");
//   if (!instance_) {
//     ESP_LOGW(TAG, "Instance is null");
//     return;
//   }

//   std::string msg(reinterpret_cast<const char *>(data), len);
//   std::array<uint8_t, 6> mac;
//   memcpy(mac.data(), mac_addr, 6);

//   for (auto &cb : instance_->callbacks_) {
//     cb(msg, mac);
//   }
// }

}  // namespace basic_espnow
}  // namespace esphome
