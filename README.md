# 📡 Presenza | CampusOS Smart Attendance

**Presenza** is a plug-and-play institutional attendance tracking system powered by an edge-computing ESP32 microcontroller, engineered to eliminate manual paper rosters and prevent proxy check-ins using localized hardware validation.

---

### 🚀 How It Works
1. **Local Access:** When students enter the lecture hall, they connect to the device's localized, secure Wi-Fi network.
2. **Instant Check-In:** The captive portal routes them to a sleek, mobile-optimized webpage where they submit their unique **Register Number**.
3. **Hardware Validation:** The system filters inputs for pure integers, cross-references an active memory array to instantly block duplicate check-ins, and logs the timestamp.

---

### ✨ Core Features
* 🔒 **Anti-Proxy Constraints:** Restricts logging access strictly to individuals physically present within the microcontroller's local Wi-Fi radius.
* 🕒 **Dynamic Session Isolation:** Automatically generates an alphanumeric Session ID (e.g., `SESS-584921`) at boot to prevent records from overlapping across different classes.
* 👨‍🏫 **Secured Faculty Deck:** Roster data is locked behind an HTTP Basic Authentication firewall (`/list`), accessible only via verified teacher credentials.
* 📥 **One-Click Telemetry Export:** Enables teachers to instantly stream active session arrays directly into a clean `.csv` spreadsheet file for university database integration.

