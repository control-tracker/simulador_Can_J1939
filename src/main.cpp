#include <Arduino.h>
#include "driver/twai.h"

#define TX_PIN GPIO_NUM_5
#define RX_PIN GPIO_NUM_4

// Estrutura com prioridade como uint32_t
struct CanMessageConfig {
  uint32_t pgn;
  uint32_t priority; // CRÍTICO: precisa ser uint32_t
  uint8_t source_address;
  const uint8_t *data;
  uint32_t interval_ms;
  unsigned long last_sent;
};

// Mensagens
const uint8_t data_FE32[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const uint8_t data_FE33[8] = {0x00, 0x25, 0x00, 0x25, 0x00, 0x25, 0x00, 0x25};//00//combustivel//00//temperatura//00//pressao ar//00//PRESSAP OLEO
const uint8_t data_FE34[8] = {0x7E, 0xC4, 0x4C, 0x7F, 0x9D, 0x73, 0x61, 0x0F};
const uint8_t data_FF35[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const uint8_t data_FEC1[8] = {0x60, 0xD0, 0xFF, 0x02, 0xBC, 0x34, 0x00, 0x00};
//const uint8_t data_FF38[8] = {0XFD, 0x43, 0x4F, 0x4E, 0x54, 0x52, 0x4F, 0XFF}; //PRESS 1 E 2
const uint8_t data_FE6C[8] = {0x7E, 0x3F, 0xFF, 0xC4, 0x00, 0x70, 0x50, 0X50};
//72508494,0CFE6CEE,true,Rx,0,8,00,3F,FF,C4,00,00,00,00,
//71985111,18FF3821,true,Rx,0,8,FD,30,30,30,30,30,30,FF,
CanMessageConfig messages[] = {
  {0xFF32, 6, 0x21, data_FE32, 150, 0},
  {0xFF33, 6, 0x21, data_FE33, 150, 0},
  {0xFF34, 6, 0x21, data_FE34, 150, 0},
  {0xFF35, 6, 0x21, data_FF35, 150, 0},
 // {0xFF38, 6, 0x21, data_FF38, 150, 0},
  {0xFEC1, 6, 0xEE, data_FEC1, 150, 0},
  {0xFE6C, 3, 0xEE, data_FE6C, 150, 0}
};

const size_t num_messages = sizeof(messages) / sizeof(messages[0]);

// === CÁLCULO DO ID J1939 SEM SHIFT ===
uint32_t calc_j1939_id(uint32_t priority, uint32_t pgn, uint8_t sa) {
  return (priority * 0x04000000UL) + (pgn << 8) + sa;
}

// Envia frame CAN
bool sendFrame(const CanMessageConfig &msgConf) {
  twai_message_t msg = {};
  msg.identifier = calc_j1939_id(msgConf.priority, msgConf.pgn, msgConf.source_address);
  msg.extd = 1;
  msg.rtr = 0;
  msg.data_length_code = 8;
  memcpy(msg.data, msgConf.data, 8);

  esp_err_t result = twai_transmit(&msg, pdMS_TO_TICKS(10));
  if (result != ESP_OK) {
    Serial.printf("❌ Erro ao enviar PGN 0x%05X | Código: %d\n", msgConf.pgn, result);
    return false;
  }

  Serial.printf("✅ Enviado PGN 0x%05X com ID 0x%08lX | Prioridade %lu | SA 0x%02X\n",
                msgConf.pgn, msg.identifier, msgConf.priority, msgConf.source_address);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("🚐 Inicializando CAN...");

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_PIN, RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS(); // ou 500KBITS
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("❌ Erro ao instalar TWAI");
    while (true);
  }

  if (twai_start() != ESP_OK) {
    Serial.println("❌ Erro ao iniciar TWAI");
    while (true);
  }

  Serial.println("✅ TWAI inicializado com sucesso");
}

void loop() {
  unsigned long now = millis();

  for (size_t i = 0; i < num_messages; i++) {
    if (now - messages[i].last_sent >= messages[i].interval_ms) {
      sendFrame(messages[i]);
      messages[i].last_sent = now;
    }
  }
}
