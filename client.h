struct SensorData {
    int methane;          // 甲烷浓度 0-99
    int temperature;      // 温度 0-55
    int smokeDetected;     // 烟雾状态  0-1
    int powerStatus;       // 电源状态  0-1
    int mainFanStatus;     // 主风扇状态 0-1
    int backupFanStatus;   // 备风扇状态 0-1
    int buzzersStatus;     // 蜂鸣器状态 0-1
    SensorData()
        : methane(0), temperature(0), smokeDetected(0),
          powerStatus(1), mainFanStatus(1), backupFanStatus(0),
          buzzersStatus(0) {}
};
