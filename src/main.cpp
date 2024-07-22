#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>

CAN_device_t CAN_cfg;               // CAN Config
const int tx_queue_size = 10;       // Transmit Queue size
CAN_speed_t CAN_speed = CAN_SPEED_250KBPS;  // CAN speed

void setup() {
  Serial.begin(115200);
  Serial.println("Basic Demo - ESP32-Arduino-CAN");

  // Configure CAN parameters
  CAN_cfg.speed = CAN_speed;
  CAN_cfg.tx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_pin_id = GPIO_NUM_4;
  CAN_cfg.rx_queue = xQueueCreate(tx_queue_size, sizeof(CAN_frame_t));
  
  // Initialize CAN Module
  ESP32Can.CANInit();
}

void sendCANFrame(CAN_frame_t frame) {
    delay(10);

  if (ESP32Can.CANWriteFrame(&frame) == 0){
    Serial.println("Frame sent successfully");
  } else {
    Serial.println("Error sending frame");
  }
}

void loop() {
  // Simulate odometer data (PGN 65217 - High Resolution Vehicle Distance)
  uint32_t odometerPGN = 65217;
  CAN_frame_t odometerFrame;
  odometerFrame.FIR.B.FF = CAN_frame_ext;
  odometerFrame.MsgID = (0 << 26) | (odometerPGN << 8) | 0x00; // Priority 3, PGN 65217, source address 0
  odometerFrame.FIR.B.DLC = 8;
  uint32_t totalDistance = 855; // Total distance in bits (12345.6 km)
  uint32_t tripDistance = 654300/5;   // Trip distance in bits (6543.21 km)
  odometerFrame.data.u8[0] = totalDistance & 0xFF;
  odometerFrame.data.u8[1] = (totalDistance >> 8) & 0xFF;
  odometerFrame.data.u8[2] = (totalDistance >> 16) & 0xFF;
  odometerFrame.data.u8[3] = (totalDistance >> 24) & 0xFF;
  odometerFrame.data.u8[4] = tripDistance & 0xFF;
  odometerFrame.data.u8[5] = (tripDistance >> 8) & 0xFF;
  odometerFrame.data.u8[6] = (tripDistance >> 16) & 0xFF;
  odometerFrame.data.u8[7] = (tripDistance >> 24) & 0xFF;
  sendCANFrame(odometerFrame);

  // Simulate RPM data (PGN 61444 - Electronic Engine Controller 1)
  uint32_t rpmPGN = 61444;
  CAN_frame_t rpmFrame;
  rpmFrame.FIR.B.FF = CAN_frame_ext;
  rpmFrame.MsgID = (0 << 26) | (rpmPGN << 8) | 0x00; // Priority 3, PGN 61444, source address 0
  rpmFrame.FIR.B.DLC = 8;
  rpmFrame.data.u8[0] = 0;    // Other data
  rpmFrame.data.u8[1] = 0;    // Other data
  rpmFrame.data.u8[2] = 0;    // Other data
  rpmFrame.data.u8[3] = 0;    // Other data
  rpmFrame.data.u8[4] = 0x4B; // RPM lower byte (1500 RPM / 0.125 = 12000)
  rpmFrame.data.u8[5] = 0x30; // RPM upper byte
  rpmFrame.data.u8[6] = 0;    // Other data
  rpmFrame.data.u8[7] = 0;    // Other data
  sendCANFrame(rpmFrame);

  // Simulate speed data (PGN 65256 - Vehicle Speed)
  uint32_t speedPGN = 65256;
  CAN_frame_t speedFrame;
  speedFrame.FIR.B.FF = CAN_frame_ext;
  speedFrame.MsgID = (3 << 26) | (speedPGN << 8) | 0x00; // Priority 3, PGN 65256, source address 0
  speedFrame.FIR.B.DLC = 8;
  uint16_t speedInBits = 60000/256; // Speed in bits (60 km/h / 0.001)
  speedFrame.data.u8[0] = speedInBits & 0xFF; // Lower byte of speed
  speedFrame.data.u8[1] = (speedInBits >> 8) & 0xFF; // Upper byte of speed
  speedFrame.data.u8[2] = 0; // Other data
  speedFrame.data.u8[3] = 0; // Other data
  speedFrame.data.u8[4] = 0; // Other data
  speedFrame.data.u8[5] = 0; // Other data
  speedFrame.data.u8[6] = 0; // Other data
  speedFrame.data.u8[7] = 0; // Other data
  sendCANFrame(speedFrame);

  // Simulate hourmeter data (PGN 65253 - Hourmeter)
  uint32_t hourmeterPGN = 65253;
  CAN_frame_t hourmeterFrame;
  hourmeterFrame.FIR.B.FF = CAN_frame_ext;
  hourmeterFrame.MsgID = (3 << 26) | (hourmeterPGN << 8) | 0x00; // Priority 3, PGN 65253, source address 0
  hourmeterFrame.FIR.B.DLC = 8;
  hourmeterFrame.data.u32[0] = 5000; // Total engine hours
  hourmeterFrame.data.u8[4] = 0;    // Other data
  hourmeterFrame.data.u8[5] = 0;    // Other data
  hourmeterFrame.data.u8[6] = 0;    // Other data
  hourmeterFrame.data.u8[7] = 0;    // Other data
  sendCANFrame(hourmeterFrame);

  // Simulate tachograph data (PGN 65132 - TCO1)
  uint32_t tcoPGN = 65132;
  CAN_frame_t tcoFrame;
  tcoFrame.FIR.B.FF = CAN_frame_ext;
  tcoFrame.MsgID = (3 << 26) | (tcoPGN << 8) | 0x00; // Priority 3, PGN 65132, source address 0
  tcoFrame.FIR.B.DLC = 8;
  tcoFrame.data.u8[0] = (0x5 << 5) | (0x3 << 2) | 0x1; // Driver 1 working state, Driver 2 working state, Vehicle motion
  tcoFrame.data.u8[1] = (0xA << 4) | (0x2 << 2) | 0x1; // Driver 1 Time Related States, Driver card driver 1, Vehicle Overspeed
  tcoFrame.data.u8[2] = (0x5 << 4) | (0x2 << 2);       // Driver 2 Time Related States, Driver card driver 2
  tcoFrame.data.u8[3] = (0x1 << 6) | (0x1 << 4) | (0x1 << 2) | 0x1; // System event, Handling information, Tachograph performance, Direction indicator
  uint16_t tachographSpeed = 6000; // Example speed in RPM
  tcoFrame.data.u8[4] = tachographSpeed & 0xFF;
  tcoFrame.data.u8[5] = (tachographSpeed >> 8) & 0xFF;
  tcoFrame.data.u8[6] = 0x82; // Other data
  tcoFrame.data.u8[7] = 0x82; // Other data
  sendCANFrame(tcoFrame);

  // Simulate engine oil temperature data (PGN 65262 - Engine Temperature)
  uint32_t engineTempPGN = 65262;
  CAN_frame_t engineTempFrame;
  engineTempFrame.FIR.B.FF = CAN_frame_ext;
  engineTempFrame.MsgID = (3 << 26) | (engineTempPGN << 8) | 0x00; // Priority 3, PGN 65262, source address 0
  engineTempFrame.FIR.B.DLC = 8;
  uint16_t oilTemp = 85; // Example oil temperature in degrees Celsius
  engineTempFrame.data.u8[0] = oilTemp & 0xFF;
  engineTempFrame.data.u8[1] = (oilTemp >> 8) & 0xFF;
  engineTempFrame.data.u8[2] = 0; // Other data
  engineTempFrame.data.u8[3] = 0; // Other data
  engineTempFrame.data.u8[4] = 0; // Other data
  engineTempFrame.data.u8[5] = 0; // Other data
  engineTempFrame.data.u8[6] = 0; // Other data
  engineTempFrame.data.u8[7] = 0; // Other data
  sendCANFrame(engineTempFrame);

  // Simulate engine oil pressure data (PGN 65263 - Engine Fluid Pressure)
  uint32_t enginePressurePGN = 65263;
  CAN_frame_t enginePressureFrame;
  enginePressureFrame.FIR.B.FF = CAN_frame_ext;
  enginePressureFrame.MsgID = (3 << 26) | (enginePressurePGN << 8) | 0x00; // Priority 3, PGN 65263, source address 0
  enginePressureFrame.FIR.B.DLC = 8;
  uint16_t oilPressure = 150; // Example oil pressure in kPa
  enginePressureFrame.data.u8[0] = oilPressure & 0xFF;
  enginePressureFrame.data.u8[1] = (oilPressure >> 8) & 0xFF;
  enginePressureFrame.data.u8[2] = 0; // Other data
  enginePressureFrame.data.u8[3] = 0; // Other data
  enginePressureFrame.data.u8[4] = 0; // Other data
  enginePressureFrame.data.u8[5] = 0; // Other data
  enginePressureFrame.data.u8[6] = 0; // Other data
  enginePressureFrame.data.u8[7] = 0; // Other data
  sendCANFrame(enginePressureFrame);

  // Simulate accelerator pedal position (PGN 61443 - Electronic Engine Controller 2)
  uint32_t pedalPositionPGN = 61443;
  CAN_frame_t pedalPositionFrame;
  pedalPositionFrame.FIR.B.FF = CAN_frame_ext;
  pedalPositionFrame.MsgID = (3 << 26) | (pedalPositionPGN << 8) | 0x00; // Priority 3, PGN 61443, source address 0
  pedalPositionFrame.FIR.B.DLC = 8;
  pedalPositionFrame.data.u8[0] = 0; // Other data
  pedalPositionFrame.data.u8[1] = 0x10; // Pedal position lower byte (80% position)
  pedalPositionFrame.data.u8[2] = 0; // Pedal position upper byte
  pedalPositionFrame.data.u8[3] = 0; // Other data
  pedalPositionFrame.data.u8[4] = 0; // Other data
  pedalPositionFrame.data.u8[5] = 0; // Other data
  pedalPositionFrame.data.u8[6] = 0; // Other data
  pedalPositionFrame.data.u8[7] = 0; // Other data
  sendCANFrame(pedalPositionFrame);

  // Simulate total fuel consumption (PGN 65257 - Engine Fuel)
  uint32_t totalFuelPGN = 65257;
  CAN_frame_t totalFuelFrame;
  totalFuelFrame.FIR.B.FF = CAN_frame_ext;
  totalFuelFrame.MsgID = (3 << 26) | (totalFuelPGN << 8) | 0x00; // Priority 3, PGN 65257, source address 0
  totalFuelFrame.FIR.B.DLC = 8;
  uint32_t totalFuel = 5; // Total fuel consumed in liters * 0.5
  totalFuelFrame.data.u8[0] = totalFuel & 0xFF;
  totalFuelFrame.data.u8[1] = (totalFuel >> 8) & 0xFF;
  totalFuelFrame.data.u8[2] = (totalFuel >> 16) & 0xFF;
  totalFuelFrame.data.u8[3] = (totalFuel >> 24) & 0xFF;
  totalFuelFrame.data.u8[4] = 0; // Other data
  totalFuelFrame.data.u8[5] = 0; // Other data
  totalFuelFrame.data.u8[6] = 0; // Other data
  totalFuelFrame.data.u8[7] = 0; // Other data
  sendCANFrame(totalFuelFrame);

  // Simulate instantaneous fuel consumption (PGN 65266 - Fuel Economy)
  uint32_t instantFuelPGN = 65266;
  CAN_frame_t instantFuelFrame;
  instantFuelFrame.FIR.B.FF = CAN_frame_ext;
  instantFuelFrame.MsgID = (3 << 26) | (instantFuelPGN << 8) | 0x00; // Priority 3, PGN 65266, source address 0
  instantFuelFrame.FIR.B.DLC = 8;
  uint16_t instantFuel = 1; // Instantaneous fuel rate in liters per hour * 0.05
  instantFuelFrame.data.u8[0] = instantFuel & 0xFF;
  instantFuelFrame.data.u8[1] = (instantFuel >> 8) & 0xFF;
  instantFuelFrame.data.u8[2] = 0; // Other data
  instantFuelFrame.data.u8[3] = 0; // Other data
  instantFuelFrame.data.u8[4] = 0; // Other data
  instantFuelFrame.data.u8[5] = 0; // Other data
  instantFuelFrame.data.u8[6] = 0; // Other data
  instantFuelFrame.data.u8[7] = 0; // Other data
  sendCANFrame(instantFuelFrame);

  // Wait before sending the next set of data
  delay(10);
}
