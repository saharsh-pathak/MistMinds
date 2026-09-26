# MistMinds: Hyperlocal Air Quality Monitoring & Active Mitigation System

MistMinds is an end-to-end IoT, Machine Learning, and web-based environmental monitoring platform designed for real-time Air Quality Index (AQI) tracking, predictive forecasting, and automated mist-sprinkler mitigation.

---

## 📁 Repository Structure

```text
MistMinds/
├── dashboard/              # React + Vite TypeScript web dashboard
│   ├── src/                # Components, services, and types
│   ├── public/             # Static assets
│   ├── index.html          # Application entry point
│   ├── package.json        # Frontend dependencies & scripts
│   └── .env.example        # Environment variable templates
│
├── firmware/               # Hardware microcontroller firmware
│   └── esp32_aqi_node/     # ESP32 sensor node & Firebase client
│       ├── esp32_aqi_node.ino
│       └── secrets.h.example
│
├── ml_model/               # Machine learning models & training pipeline
│   ├── train.py            # Model training & weight export script
│   ├── dataset.csv         # Sensor training dataset
│   ├── model.h / model.json
│   └── scaler.h / scaler.json
│
├── data/                   # Sensor data exports and credential templates
│   ├── esp32_sensor_data.csv
│   └── credentials/
│
├── docs/                   # System documentation & PRD
│   ├── PRD.md
│   └── presentation_brief.md
│
└── tools/                  # Hardware tools & USB driver packages
    └── drivers/            # CH340 / CP210x USB-to-UART drivers
```

---

## 🚀 Quick Start

### 1. Web Dashboard (`dashboard/`)

```bash
cd dashboard
npm install
cp .env.example .env.local
# Configure your Firebase and API keys in .env.local
npm run dev
```

### 2. Firmware (`firmware/esp32_aqi_node/`)

1. Open `firmware/esp32_aqi_node/esp32_aqi_node.ino` in Arduino IDE or VSCode with PlatformIO.
2. Copy `secrets.h.example` to `secrets.h` and update your WiFi and Firebase Realtime Database credentials.
3. Select your ESP32 board port and upload.

### 3. Machine Learning Model (`ml_model/`)

```bash
cd ml_model
python train.py
```

---

## 🛡️ Key Features

- **Hyperlocal Sensing**: Real-time PM2.5, temperature, and humidity data collection via ESP32.
- **Triangular Verification**: Cross-validates local sensor readings against official reference stations.
- **Predictive AQI Forecasting**: Uses ML regression & Gemini AI modeling to forecast air quality 30 minutes in advance.
- **Automated Mitigation**: Automated relay activation for mist sprinklers when hazardous AQI thresholds are detected.

---

## 📜 License

MIT License. See [LICENSE](LICENSE) for details.
