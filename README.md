# 🌱 Soil Doctor
### Embedded AI-Based Soil Analysis Device for Crop & Fertilizer Recommendation

> *Real-time soil intelligence — no lab, no internet, no waiting.*

---

##  Overview

**Soil Doctor** is a portable, offline-capable embedded AI device that analyzes soil health in real time and instantly recommends the best crops and fertilizers — right in the field. Built on an ESP32 microcontroller with an industrial 7-in-1 soil sensor and a trained Decision Tree model, it eliminates the traditional "Information Lag" of laboratory-based testing.

**Developed by Group 05 — Department of Applied Electronics & Instrumentation Engineering, GEC Kozhikode (April 2026)**

| | |
|---|---|
| **Authors** | Gopika Suresh · Megha C · Midhunrag N V · Shreya S N |
| **Guide** | Dr. Jesy P |
| **University** | APJ Abdul Kalam Technological University |
| **Degree** | B.Tech — Electronics & Communication Engineering |

---

##  The Problem

Traditional soil testing is broken for smallholder farmers:

-  **Weeks of delay** waiting for lab results
-  **High cost** per test, impractical for frequent monitoring  
-  **No connectivity** in remote rural areas
-  **Guesswork-based** fertilizer application → soil degradation + chemical runoff

---

##  The Solution

Soil Doctor delivers **< 10 second** field-to-recommendation latency with **95% classification accuracy** — fully offline.

```
Insert probe → Read sensors → AI inference → See recommendation
       < 10 seconds, no internet required
```

---

##  Hardware Components

| Component | Role |
|---|---|
| **ESP32 Dev Board** | Central processing unit; runs Decision Tree inference |
| **7-in-1 Soil Sensor** (ZTS-3002-TR-ECTHNPKPH-N0) | Measures N, P, K, pH, EC, moisture, temperature |
| **MAX485 Module** | RS485 ↔ TTL signal conversion (Modbus-RTU bridge) |
| **16×2 I2C LCD** | Displays soil readings and AI recommendations |
| **5V Rechargeable Power Bank** | Portable standalone operation |
| **Zero PCB Board** | Permanent, durable component mounting |
| **Push Button + Rocker Switch** | Navigation and power control |
| **1kΩ / 2.2kΩ Resistors** | Voltage divider for 5V → 3.3V signal conditioning |

---

##  System Architecture

```
┌─────────────────┐    RS485     ┌──────────┐    TTL     ┌─────────┐    I2C    ┌─────────────┐
│  7-in-1  Soil   │ ──────────▶  │  MAX485  │ ─────────▶ │  ESP32  │ ────────▶ │ LCD Display │
│     Sensor      │              │Converter │            │  (MCU)  │           │   16×2      │
└─────────────────┘              └──────────┘            └────┬────┘           └─────────────┘
                                                              │
                                                     ┌────────▼────────┐
                                                     │  Decision Tree  │
                                                     │  AI Inference   │
                                                     │  (Offline C++)  │
                                                     └─────────────────┘
        └─────────────────────────────────────────────────────────────────────────────────────────┘
                                        5V Rechargeable Power Bank
```

---

##  Processing Pipeline

```
Stage 1 — DATA ACQUISITION
  7-in-1 sensor measures NPK, pH, EC, moisture, temperature
  Encodes readings into Modbus-RTU frames over RS485

Stage 2 — SIGNAL CONVERSION
  MAX485 converts RS485 differential signals → TTL serial logic
  1k/2.2k voltage divider: 5V → 3.3V (ESP32 safe)

Stage 3 — PROCESSING & INFERENCE
  ESP32 parses Modbus frames with 16-bit CRC verification
  Applies scaling factors (e.g., raw ÷ 10.0 = real pH)
  Runs Decision Tree → crop classification + gap analysis

Stage 4 — VISUALIZATION
  Results shown on 16×2 LCD
  User navigates between soil readings and AI recommendations via push button
```

---

##  AI Model — Decision Tree Classifier

Trained in Python (Scikit-learn / Spyder IDE) and extracted as hard-coded C++ `if-then` rules for fully offline, edge-based inference on the ESP32.

**Supported crops:** Tomato · Chilli · Okra · Red Amaranthus · Cowpea

**Classification accuracy: 95%**

```
DECISION TREE RULES:
├── Soil Moisture ≤ 57.50  →  Cowpea
└── Soil Moisture > 57.50
    ├── Potassium ≤ 160.00
    │   ├── EC ≤ 1.60       →  Red Amaranthus
    │   └── EC > 1.60       →  Okra
    └── Potassium > 160.00
        ├── Soil Moisture ≤ 65.00  →  Chilli
        └── Soil Moisture > 65.00  →  Tomato
```

**Gap Analysis:** After crop identification, the ESP32 compares measured N, P, K values against TNAU Agritech benchmarks and generates specific fertilizer advice (e.g., *"Apply Urea"*, *"Apply Phosphorus"*, *"Reduce N fert"*).

---

##  Software Stack

| Tool | Purpose |
|---|---|
| **Arduino IDE** | ESP32 firmware development (C/C++), Modbus-RTU parsing, LCD control |
| **Python / Spyder IDE** | Decision Tree training with Scikit-learn; rule extraction |
| **EasyEDA** | Schematic capture and PCB pinout design |
| **Wokwi Simulator** | Firmware validation before physical assembly |

---

##  Results

### Test Case Validation

| TC | N (ppm) | P (ppm) | K (ppm) | pH | Moisture | Crop | Fertilizer |
|:--:|:-------:|:-------:|:-------:|:--:|:--------:|:----:|:----------:|
| 1 | 563 | 1347 | 1350 | 6.9 | 65.7% | Tomato | Apply Urea |
| 2 | 298 | 736 | 733 | 7.1 | 51.3% | Cowpea | Apply Phosphorus |
| 3 | 7 | 62 | 54 | 6.4 | 58.2% | Red Amaranthus | Apply Potassium |
| 4 | 218 | 551 | 548 | 9.0 | 61.5% | Chilli | Reduce N fert |

### Performance Metrics

| Metric | Expected | Achieved |
|--------|----------|----------|
| Latency | Real-time (eliminate lag) | **< 10 seconds** |
| Accuracy | High classification accuracy | **95%** |
| Reliability | Noise-resistant in field | Error-free via MAX485 + 16-bit CRC |
| Throughput | Multi-parameter acquisition | 7-in-1 parallel (N, P, K, pH, EC, Temp, Moisture) |
| Power | Portable, long-term field use | Stable via 5V rechargeable bank |
| Connectivity | Offline capable | **Fully offline** — no internet required |

---

##  Future Work

- **Smart Irrigation Integration** — LoRa/LPWAN communication for long-range autonomous water management
- **Cloud Dashboard** — Mobile app with real-time IoT monitoring and historical soil data logging
- **Expanded Crop Database** — Support for more crop varieties and regional agronomic benchmarks
- **Custom PCB** — Replace Zero Board with a purpose-designed PCB for weatherproofing and miniaturization

---

##  Key References

1. Khaliq et al., "AI-driven smart agriculture," *IEEE Access*, 2025.
2. Madhuri et al., "Prediction of crop yield using ML algorithms," *ICTACS*, IEEE, 2022.
3. Madhumathi et al., "Soil nutrient prediction and crop recommendation system," *ICCPCT*, IEEE, 2023.
4. Tamil Nadu Agricultural University (TNAU), "Fertilizer Schedule for Vegetable Crops," Agritech Portal, 2024.
5. Chowdam et al., "IoT-Enabled smart soil monitoring system," *ICVADV*, IEEE, 2025.

---

##  SDG Alignment

| Goal | Contribution |
|------|-------------|
| **SDG 2** — Zero Hunger | Improves crop productivity via precision soil analysis |
| **SDG 9** — Industry & Innovation | Low-cost embedded AI for agriculture |
| **SDG 12** — Responsible Consumption | Prevents over-fertilization and chemical runoff |
| **SDG 15** — Life on Land | Supports long-term soil health preservation |

---

##  License

This project was submitted as a Mini Project Report to APJ Abdul Kalam Technological University in partial fulfillment of the B.Tech degree requirements. April 2026.

---

<p align="center">
  <em>Government Engineering College — Kozhikode, Kerala 673005</em><br>
  <em>Department of Applied Electronics & Instrumentation Engineering</em>
</p>

##  Author

Shreya S N

---

## ⭐ If you like this project

Give it a ⭐ on GitHub!

