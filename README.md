# ESP32S3 Sunton 7" RTC Clock

[Polski opis poniżej](#wersja-polska)

An elegant retro/modern digital clock based on the **Sunton ESP32-S3 development board with a 7-inch touchscreen (800x480)**. The project integrates an external high-precision **DS3231 Real-Time Clock (RTC)** module and renders anti-aliased digital **TTF (TrueType Fonts)** directly on the screen.

The project is optimized for **PlatformIO** with a pinned environment version (utilizing the proven Arduino Core 2.x), ensuring a stable build and eliminating API breaking changes introduced in the newer ESP-IDF 5.x releases.

---

## 🚀 Key Features

*   **7-Inch Display:** Full utilization of the Sunton native screen resolution (800x480) for clock presentation.
*   **Precision Hardware Timing:** Integration with the external DS3231 RTC chip, keeping accurate time even when completely powered off.
*   **Smooth Typography (Anti-aliasing):** High-quality dynamic rendering of digital TTF fonts.
*   **Rock-Solid Build Architecture:** Pinned platform version configuration prevents dependency hell caused by recent framework updates.

---

## 🛠️ Hardware Requirements

1.  **Development Board:** Sunton 7" ESP32-S3 (e.g., ESP32-8048S070).
2.  **RTC Module:** DS3231 connected via the I2C bus.
3.  **Backup Battery:** CR1220 or CR2032 inside the RTC module to preserve time.

---

## 💻 Installation & Setup

This repository is pre-configured for **VS Code** with the **PlatformIO** extension.

### 1. Clone the Repository
```bash
git clone https://github.com
cd ESP32S3-SUNTON7-CLOCK
```

### 2. PlatformIO Configuration (`platformio.ini`)
To avoid compilation errors with deprecated APIs on newer frameworks, the configuration forces the stable `6.x` platform series:

```ini
[env:esp32dev]
platform = espressif32@6.7.0
framework = arduino
# Remaining Sunton 7" specific display setup...
```

### 3. Build & Flash
1. Open the project folder in **VS Code**.
2. Wait for PlatformIO to download all tools and toolchains.
3. Click the **Clean** icon (trash can) on the bottom status bar to purge any cached configurations.
4. Click the **Build** icon (checkmark) to compile.
5. Connect your Sunton board via USB and click **Upload** (right arrow).

---

## 📂 Project Structure

*   `src/` - Main application source code (`main.cpp`).
*   `include/` - Screen configurations and pin mapping header files.
*   `lib/` - Local custom drivers for the Sunton display and TTF engine.
*   `platformio.ini` - Project workspace configuration and dependencies.

---

<a id="wersja-polska"></a>
# Wersja Polska

Elegancki, cyfrowy zegar retro/modern oparty na module **Sunton ESP32-S3 z 7-calowym ekranem dotykowym (800x480)**. Projekt wykorzystuje zewnętrzny, precyzyjny moduł czasu rzeczywistego **RTC DS3231** oraz renderuje wygładzone cyfrowe czcionki **TTF (TrueType Fonts)**.

Projekt został zoptymalizowany dla **PlatformIO** z zamrożonym środowiskiem (wykorzystującym sprawdzony rdzeń Arduino 2.x), gwarantując stabilną kompilację i brak konfliktów z nowszymi wersjami ESP-IDF 5.x.

---

## 🚀 Główne Funkcje

*   **7-calowy Wyświetlacz:** Pełne wykorzystanie rozdzielczości ekranu Sunton (800x480) do prezentacji zegara.
*   **Precyzyjny Czas RTC:** Integracja z zewnętrznym układem DS3231, który podtrzymuje czas nawet po odłączeniu zasilania.
*   **Wygładzone Czcionki (Anti-aliasing):** Dynamiczne i gładkie renderowanie cyfrowych czcionek TTF bezpośrednio na ekranie.
*   **Stabilna Architektura:** Zamrożona konfiguracja środowiska zapobiega problemom ze zmianami API w najnowszych wersjach frameworków.

---

## 🛠️ Wymagania Sprzętowe

1.  **Płytka deweloperska:** Sunton 7" ESP32-S3 (np. ESP32-8048S070).
2.  **Moduł RTC:** DS3231 (podłączony pod magistralę I2C).
3.  **Bateria CR1220/CR2032:** Do podtrzymania zasilania w module RTC.

---

## 💻 Instalacja i Konfiguracja

Projekt jest w pełni przygotowany do uruchomienia w środowisku **VS Code** z zainstalowanym rozszerzeniem **PlatformIO**.

### 1. Klonowanie repozytorium
```bash
git clone https://github.com
cd ESP32S3-SUNTON7-CLOCK
```

### 2. Konfiguracja PlatformIO (`platformio.ini`)
W celu uniknięcia problemów z kompatybilnością ze starym rdzeniem Arduino, plik konfiguracyjny wymusza stabilną wersję platformy z serii `6.x`:

```ini
[env:esp32dev]
platform = espressif32@6.7.0
framework = arduino
# Pozostała konfiguracja specyficzna dla wyświetlacza Sunton 7"...
```

### 3. Kompilacja i Wgrywanie
1. Otwórz folder projektu w **VS Code**.
2. Poczekaj, aż PlatformIO automatycznie pobierze wymagane biblioteki i toolchain.
3. Kliknij ikonę **Clean** (śmietnik na dolnym pasku), aby wyczyścić ewentualne pozostałości.
4. Kliknij ikonę **Build** (ptaszek), aby skompilować projekt.
5. Podłącz płytkę Sunton kablem USB i kliknij **Upload** (strzałka w prawo).

---

## 📂 Struktura Projektu

*   `src/` - Główny kod źródłowy aplikacji (`main.cpp`).
*   `include/` - Pliki nagłówkowe konfiguracji ekranu i pinów.
*   `lib/` - Lokalne biblioteki i sterowniki dla wyświetlacza Sunton oraz czcionek TTF.
*   `platformio.ini` - Plik konfiguracyjny środowiska i zależności.

---

## 📄 Licencja

Projekt udostępniony jako backup kodu o charakterze Open Source. Możesz go dowolnie modyfikować i dostosowywać do własnych potrzeb.
