# MistMinds: Hyperlocal Air Quality Monitoring & Automated Mist Control System

MistMinds is an end-to-end IoT, Cloud, and Web-based environmental telemetry and automated mitigation platform. The system ingests hyperlocal air quality data (PM2.5, PM10, temperature, humidity) from ESP32 edge nodes, synchronizes data in real-time via Firebase Realtime Database, visualizes colony-wide telemetry on an interactive map, and orchestrates misting/sprinkler actuators to suppress particulate pollution.

---

## 📑 Table of Contents
1. [Project Overview & Architecture](#project-overview--architecture)
2. [Key Features & Dashboard Sections](#key-features--dashboard-sections)
3. [Component Breakdown](#component-breakdown)
   - [A. Hardware & Firmware (ESP32 Edge Node)](#a-hardware--firmware-esp32-edge-node)
   - [B. Cloud & Firebase Database](#b-cloud--firebase-database)
   - [C. Web Dashboard (React + Vite + TypeScript)](#c-web-dashboard-react--vite--typescript)
4. [Firebase Setup & Configuration](#firebase-setup--configuration)
5. [Complete Deployment Guide for Ubuntu 22.04.5 LTS](#complete-deployment-guide-for-ubuntu-22045-lts)
6. [Hardware Flashing & Pinout Guide](#hardware-flashing--pinout-guide)
7. [Environment Variables Reference](#environment-variables-reference)

---

## 1. Project Overview & Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32 Edge Node (Node-1)                 │
│  - DHT22 (Temp & Humidity)                                  │
│  - Particulate Sensor / Potentiometer / Simulation Engine   │
│  - 16x2 I2C LCD Display                                     │
│  - Relay / Solenoid Valve (Sprinkler Actuation)             │
│  - Tactile Switch (Mode Toggle)                             │
└──────────────────────────────┬──────────────────────────────┘
                               │ WiFi (HTTPS / WebSockets)
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                Firebase Realtime Database                   │
│  - `/Node1` (Real-time AQI, Temp, Humidity, Relay State)    │
│  - `/sprinkler_history` (Historical activation events)      │
│  - `/history/Node1...Node4` (Historical telemetry logs)     │
└──────────────────────────────┬──────────────────────────────┘
                               │ Real-time WebSockets
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                 MistMinds Web Dashboard                     │
│  (Nginx / Node.js on Ubuntu 22.04.5 LTS Server)             │
│  - Real-time Colony Averaging & Telemetry Cards             │
│  - Automatic & Manual Sprinkler Actuator Controls           │
│  - Full Activation History Logs & Modal Analytics           │
│  - Interactive Leaflet Geospatial Colony Map                │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Key Features & Dashboard Sections

### 1. **Header & Global Colony Overview**
- **Brand Status & Live Clock**: 24-hour real-time system clock synchronized with browser time.
- **Colony Average Metric**: Real-time aggregation of AQI metrics across all active spatial nodes.

### 2. **Node Selector & Connectivity Heartbeat**
- **Node Selection Dropdown**: Switch between individual physical or simulated IoT nodes (Node-1, Node-2, Node-3, Node-4, and the DPCC/CPCB Official Reference Station).
- **Live Connection Guard & Heartbeat**: Pulsing indicator detecting actual live WebSocket packets from the physical ESP32 node.

### 3. **Hyperlocal Node Telemetry & Spatial Analytics**
- **Prominent Color-Coded AQI Scorecard**: Categorized according to National Air Quality Index (NAQI) standards (Good, Satisfactory, Moderate, Poor, Very Poor, Severe).
- **Environmental Telemetry Matrix**:
  - **PM2.5** ($\mu\text{g/m}^3$) and **PM10** ($\mu\text{g/m}^3$).
  - **Ambient Temperature** ($^\circ\text{C}$) & **Relative Humidity** ($\%$).
  - **Sprinkler State Indicator** (`ACTIVE` vs `STANDBY`).
- **Official Station Comparison & Distance**: Haversine distance calculator measuring distance in kilometers between the node and the regulatory station.
- **Last Treatment Tracking**: Elapsed time since the selected node was last misted/treated.

### 4. **Sprinkler Control & Actuation**
- **Dual Operational Modes**:
  - **Automatic Mode**: Triggers misting automatically based on real-time pollution and humidity thresholds.
  - **Manual Mode**: Direct user override for fixed 10-minute misting sessions.
- **Emergency Stop / Start**: Direct actuation toggles with automatic state synchronization to Firebase.
- **Activation History & Modal**: Logs every misting session, duration, and AQI reduction delta (`-X%`), with full history view.

### 5. **Interactive Hyperlocal Leaflet Map**
- **CartoDB Voyager Geospatial Basemap**: High-contrast, responsive mapping canvas.
- **Colony Boundary Geofence**: Bounded colony polygon overlay.
- **Dynamic Node Markers**: Color-coded markers mapped to real-time AQI values with interactive selection synchronizing all dashboard widgets.

---

## 3. Component Breakdown

### A. Hardware & Firmware (ESP32 Edge Node)
- **Directory**: `firmware/esp32_aqi_node/`
- **Source Files**:
  - `esp32_aqi_node.ino`: Main firmware code handling sensor sampling, local LCD rendering, relay toggling, and Firebase synchronization.
  - `secrets.h`: WiFi and Firebase credentials.

### B. Cloud & Firebase Database
- Stores current node state under `/Node1`:
  ```json
  {
    "aqi": 74,
    "pm25": 45.2,
    "humidity": 58,
    "temperature": 27.4,
    "sprinklerActive": false,
    "timestamp": 1727700000000
  }
  ```
- Stores actuation logs under `/sprinkler_history` and sensor time-series under `/history`.

### C. Web Dashboard (Frontend)
- **Directory**: `dashboard/`
- **Tech Stack**: React 18, TypeScript, Vite, Tailwind CSS, Leaflet.js, Firebase JS SDK.
- **Key Modules**:
  - `App.tsx`: Main application shell, state management, and Firebase real-time listeners.
  - `components/AQIMap.tsx`: Interactive Leaflet map with colony geofence and dynamic AQI markers.
  - `components/SprinklerControl.tsx`: Manual/Auto control panel, threshold tuner, and misting history tables.
  - `services/aqiService.ts`: Firebase RTDB connector, NAQI calculation algorithms, and mock generation fallbacks.

---

## 4. Firebase Setup & Configuration

### Step 1: Create a Firebase Project
1. Go to the [Firebase Console](https://console.firebase.google.com/).
2. Click **Add project** and name it (e.g. `mistminds-aqi`).
3. Disable Google Analytics (optional) and click **Create Project**.

### Step 2: Set up Realtime Database (RTDB)
1. In the Firebase console sidebar, navigate to **Build** > **Realtime Database**.
2. Click **Create Database**.
3. Choose a location closest to your deployment (e.g. `United States (us-central1)` or `Singapore (asia-southeast1)`).
4. Start in **Test Mode** (or configure security rules below).
5. Set the **Security Rules** under the *Rules* tab:
   ```json
   {
     "rules": {
       ".read": true,
       ".write": true
     }
   }
   ```
   *(For production, you can configure Email/Password auth rules).*

### Step 3: Enable Email/Password Authentication (for ESP32)
1. Go to **Build** > **Authentication** > **Sign-in method**.
2. Enable **Email/Password**.
3. Go to the **Users** tab and click **Add user**.
4. Create credentials (e.g., `iothackathon@gmail.com` and your password).

### Step 4: Obtain Web App Config Keys
1. Go to **Project Settings** (gear icon) > **General**.
2. Under *Your apps*, click the **Web icon (`</>`)** to register a web application.
3. Copy the `firebaseConfig` object values.

---

## 5. Complete Deployment Guide for Ubuntu 22.04.5 LTS

### Prerequisites
- Ubuntu 22.04.5 LTS Server (x86_64 or ARM64) with `sudo` access.
- Domain name or public IP address.

---

### Step 1: Update System & Install Core Dependencies
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl git ufw nginx build-essential
```

### Step 2: Install Node.js 20 LTS (NodeSource)
```bash
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt install -y nodejs

# Verify installation
node -v # Should output v20.x.x
npm -v  # Should output v10.x.x
```

---

### Step 3: Clone & Setup the Project
```bash
# Navigate to web root directory
cd /var/www

# Clone your repository (or copy the project directory)
sudo git clone https://github.com/your-username/MistMinds.git mistminds
sudo chown -R $USER:$USER /var/www/mistminds

# Navigate to the dashboard directory
cd /var/www/mistminds/dashboard
```

---

### Step 4: Configure Production Environment Variables
Create the `.env.local` or `.env` file in `/var/www/mistminds/dashboard/`:

```bash
nano .env.local
```

Paste your actual Firebase and API credentials:
```env
# World Air Quality Index (WAQI) API Key (optional)
AQI_API_KEY=YOUR_WAQI_API_KEY

# Firebase Client SDK Configuration
VITE_FIREBASE_API_KEY=AIzaSyC8gUDOQ1Db0RpeSlr0cx2JxcpMIGripoM
VITE_FIREBASE_AUTH_DOMAIN=your-project.firebaseapp.com
VITE_FIREBASE_DATABASE_URL=https://your-project-default-rtdb.firebaseio.com
VITE_FIREBASE_PROJECT_ID=your-project-id
VITE_FIREBASE_STORAGE_BUCKET=your-project.firebasestorage.app
VITE_FIREBASE_MESSAGING_SENDER_ID=your_messaging_sender_id
VITE_FIREBASE_APP_ID=your_app_id
VITE_FIREBASE_MEASUREMENT_ID=your_measurement_id
```
*Save with `Ctrl + O`, `Enter`, and exit with `Ctrl + X`.*

---

### Step 5: Install Dependencies & Build Frontend
```bash
cd /var/www/mistminds/dashboard
npm install
npm run build
```
This compiles optimized static assets into `/var/www/mistminds/dashboard/dist`.

---

### Step 6: Configure Nginx as Web Server
1. Create an Nginx site configuration file:
   ```bash
   sudo nano /etc/nginx/sites-available/mistminds
   ```

2. Paste the following configuration (replace `your_domain_or_ip` with your server's domain or public IP):
   ```nginx
   server {
       listen 80;
       listen [::]:80;

       server_name your_domain_or_ip;

       root /var/www/mistminds/dashboard/dist;
       index index.html;

       location / {
           try_files $uri $uri/ /index.html;
       }

       # Enable gzip compression
       gzip on;
       gzip_vary on;
       gzip_min_length 1024;
       gzip_proxied expired no-cache no-store private auth;
       gzip_types text/plain text/css text/xml text/javascript application/x-javascript application/xml application/javascript application/json;
       gzip_disable "MSIE [1-6]\.";

       # Cache static assets
       location ~* \.(js|css|png|jpg|jpeg|gif|ico|svg|woff|woff2|ttf|eot)$ {
           expires 1y;
           add_header Cache-Control "public, no-transform";
       }

       error_page 404 /index.html;
   }
   ```

3. Enable the site and test Nginx configuration:
   ```bash
   # Enable configuration
   sudo ln -s /etc/nginx/sites-available/mistminds /etc/nginx/sites-enabled/

   # Remove default configuration if conflicting
   sudo rm -f /etc/nginx/sites-enabled/default

   # Test syntax
   sudo nginx -t

   # Restart Nginx
   sudo systemctl restart nginx
   sudo systemctl enable nginx
   ```

---

### Step 7: Configure Firewall (UFW)
```bash
sudo ufw allow OpenSSH
sudo ufw allow 'Nginx Full'
sudo ufw enable
```

---

### Step 8 (Optional): Setup Free SSL with Let's Encrypt (Certbot)
If you have a public domain pointed to your Ubuntu server:
```bash
sudo apt install -y certbot python3-certbot-nginx
sudo certbot --nginx -d your_domain.com -d www.your_domain.com
```
Certbot will configure SSL automatically and setup automatic certificate renewals.

---

## 6. Hardware Flashing & Pinout Guide

### ESP32 Pin Connections
| Component | ESP32 Pin | Notes |
|---|---|---|
| **DHT22 Data Pin** | GPIO 4 | Pull-up resistor (4.7kΩ / 10kΩ) recommended |
| **Relay / Solenoid Valve** | GPIO 23 | Active HIGH signal to toggle misting nozzle |
| **Tactile Push Button** | GPIO 27 | Mode toggle (INPUT_PULLUP) |
| **Potentiometer (Simulation)** | GPIO 35 | Analog simulation of AQI level |
| **LCD 16x2 I2C SDA** | GPIO 21 | I2C Data (0x27) |
| **LCD 16x2 I2C SCL** | GPIO 22 | I2C Clock |

### Configuring `secrets.h` in Arduino IDE
1. Open `firmware/esp32_aqi_node/secrets.h`.
2. Configure your local WiFi network and Firebase credentials:
   ```cpp
   #ifndef SECRETS_H
   #define SECRETS_H

   // WiFi Configuration
   #define WIFI_SSID "YOUR_WIFI_SSID"
   #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

   // Firebase Configuration
   #define API_KEY "YOUR_FIREBASE_API_KEY"
   #define DATABASE_URL "your-project-default-rtdb.firebaseio.com"

   // Firebase Auth Credentials
   #define USER_EMAIL "your_user@example.com"
   #define USER_PASSWORD "your_password"

   #endif
   ```
3. Install required Arduino libraries:
   - `Firebase ESP Client` by Mobizt
   - `DHT sensor library` by Adafruit
   - `LiquidCrystal I2C` by Frank de Brabander
4. Select board `ESP32 Dev Module` and flash the firmware.

---

## 7. Environment Variables Reference

| Variable | Description | Example / Default |
|---|---|---|
| `VITE_FIREBASE_API_KEY` | Firebase Web API Key | `AIzaSy...` |
| `VITE_FIREBASE_AUTH_DOMAIN` | Firebase Auth Domain | `mistminds.firebaseapp.com` |
| `VITE_FIREBASE_DATABASE_URL` | Realtime Database URL | `https://mistminds-rtdb.firebaseio.com` |
| `VITE_FIREBASE_PROJECT_ID` | Firebase Project ID | `mistminds` |
| `VITE_FIREBASE_STORAGE_BUCKET`| Firebase Storage Bucket | `mistminds.firebasestorage.app` |
| `VITE_FIREBASE_MESSAGING_SENDER_ID` | Messaging Sender ID | `123456789012` |
| `VITE_FIREBASE_APP_ID` | Firebase App ID | `1:123456789012:web:abcdef` |
| `AQI_API_KEY` | WAQI Public Token (optional)| `demo` |
