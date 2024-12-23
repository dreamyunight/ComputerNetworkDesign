struct SensorData {
    int methane;          // 甲烷浓度
    int temperature;      // 温度
    bool smokeDetected;     // 烟雾状态
    bool powerStatus;       // 电源状态
    bool mainFanStatus;     // 主风扇状态
    bool backupFanStatus;   // 备风扇状态
    bool buzzersStatus;     // 蜂鸣器状态
    SensorData()
        : methane(0), temperature(0), smokeDetected(false),
          powerStatus(true), mainFanStatus(false), backupFanStatus(false),
          buzzersStatus(false) {}
};
