
#pragma once

#include <Arduino.h>
#include <time.h>
#include <ESP32_SoftWire.h>

//=================================================================================================================================
class DS3231
{
#define DS3231_I2C_ADDRESS 0x68

public:
  DS3231(int SDA, int SCL)
  {
    // constructor
    swWire.begin(SDA, SCL, 100000);
  }
  ~DS3231()
  {
    // destructor
  }

  void CLOCK_WRITE_REG(uint8_t deviceaddress, uint32_t reg_no, uint32_t data)
  {
    uint32_t new_ad = (reg_no << 2);
    i2c_eeprom_write_word(deviceaddress, new_ad, data);
  }
  uint32_t CLOCK_READ_REG(uint8_t deviceaddress, uint32_t reg_no)
  {
    uint32_t res;
    uint32_t new_ad = (reg_no << 2);
    res = i2c_eeprom_read_word(deviceaddress, new_ad);
    //  Serial.printf("res = %d\n\r" , res);
    return res;
  }
  uint32_t i2c_eeprom_read_word(uint8_t deviceaddress, uint32_t eeaddress)
  {
    uint32_t rdata = 0xFF;
    uint32_t result = 0;
    uint32_t i;

    uint8_t MSB = (uint8_t)(eeaddress >> 8);
    uint8_t LSB = (uint8_t)(eeaddress & 0xFF);

    swWire.beginTransmission(deviceaddress);
    swWire.write((int)MSB);
    swWire.write((int)LSB);
    swWire.endTransmission();
    swWire.requestFrom(deviceaddress, 4);
    for (i = 0; i < 4; i++)
    {
      result = result >> 8;
      rdata = (uint32_t)swWire.read();
      rdata = (rdata << 24);
      result = result | rdata;
    }
    // Serial.printf("result = 0x%08X\n\r" , result);
    return result;
  }

  uint8_t i2c_eeprom_read_byte(uint8_t deviceaddress, uint32_t eeaddress)
  {
    uint8_t rdata = 0xFF;
    uint8_t MSB = (eeaddress >> 8);
    uint8_t LSB = (eeaddress & 0xFF);

    swWire.beginTransmission(deviceaddress);
    swWire.write((int)MSB);
    swWire.write((int)LSB);
    swWire.endTransmission();
    swWire.requestFrom(deviceaddress, 1);
    rdata = swWire.read();
    return rdata;
  }

  void i2c_eeprom_write_word(uint8_t deviceaddress, uint32_t eeaddress, uint32_t data)
  {
    uint32_t to_send = data;
    uint32_t i;
    uint8_t sender;
    uint32_t ADDY = eeaddress;

    for (i = 0; i < 4; i++)
    {
      sender = (uint8_t)(to_send & 0xff);
      to_send = (to_send >> 8);
      i2c_eeprom_write_byte(deviceaddress, ADDY, sender);
      ADDY++;
    }
  }
  void i2c_eeprom_write_byte(uint8_t deviceaddress, uint32_t eeaddress, uint8_t data)
  {
    int rdata = data;
    uint8_t MSB = (eeaddress >> 8);
    uint8_t LSB = (eeaddress & 0xFF);

    swWire.beginTransmission(deviceaddress);
    swWire.write((int)MSB);
    swWire.write((int)LSB);
    swWire.write(data);
    swWire.endTransmission();
    delay(10);
  }
    //---
  void Set_Time(struct tm *s_time)
  {

    uint8_t secs = s_time->tm_sec % 60;
    uint8_t mins = s_time->tm_min % 60;
    uint8_t hh = s_time->tm_hour % 60;
    uint8_t wday = (s_time->tm_wday % 6) + 1;
    uint8_t mday = s_time->tm_mday;
    uint8_t mm = s_time->tm_mon + 1;
    uint8_t year = s_time->tm_year % 100;

    secs = BIN_to_BCD(secs);
    mins = BIN_to_BCD(mins);
    hh = BIN_to_BCD(hh);
    wday = BIN_to_BCD(wday);
    mday = BIN_to_BCD(mday);
    mm = BIN_to_BCD(mm);
    mm = mm + 128;

    year = BIN_to_BCD(year);

    swWire.beginTransmission(DS3231_I2C_ADDRESS);
    swWire.write(0);    // set next input to start at the seconds register
    swWire.write(secs); // set seconds (0 to 59)
    swWire.write(mins); // set minutes (0 to 59)
    swWire.write(hh);   // set hours   (0 to 23)
    swWire.write(wday); // set wday  (1 to 7) (1=Sunday, 7=Saturday)
    swWire.write(mday); // set date  (1 to 31)
    swWire.write(mm);   // set month (1 to 12)
    swWire.write(year); // set year  (0 to 99)
    swWire.endTransmission();
#if 0
    Serial.println("LIB_DS3231_WRITE_DATE_TIME :");
    Serial.printf("Secs. value : 0x%04X \n", secs);
    Serial.printf("Mins. value : 0x%04X \n", mins);
    Serial.printf("Hours value : 0x%04X \n", hh);
    Serial.printf("Wdays value : 0x%04X \n", wday);
    Serial.printf("Mdays value : 0x%04X \n", mday);
    Serial.printf("Month value : 0x%04X \n", (mm & 0x7F));
    Serial.printf("Year  value : 0x%04X \n\n", year);
#endif
  }
  //---
  void Read_Time(struct tm *r_time)
  {
    swWire.beginTransmission(DS3231_I2C_ADDRESS);
    swWire.write(0); // set DS3231 register pointer to 00h
    swWire.endTransmission();
    swWire.requestFrom(DS3231_I2C_ADDRESS, 7);
    // request seven bytes of data from DS3231 starting from register 00h
    r_time->tm_sec = BCD_to_BIN(swWire.read() & 0x7f);
    r_time->tm_min = BCD_to_BIN(swWire.read());
    r_time->tm_hour = BCD_to_BIN(swWire.read() & 0x3f);
    r_time->tm_wday = (BCD_to_BIN(swWire.read()) - 1);
    r_time->tm_mday = BCD_to_BIN(swWire.read());
    r_time->tm_mon = (BCD_to_BIN(swWire.read() & 0x1F) -1);
    r_time->tm_year = 100 + BCD_to_BIN(swWire.read());
  }
  //---
  void SECONDS_write(uint8_t secs)
  {
    uint8_t ss = secs % 60;
    ss = BIN_to_BCD(secs);
    WRITE_ONE_REG(0x00, ss);
  }
  //---
  void MINUTS_write(uint8_t mins)
  {
    mins = mins % 60;
    WRITE_ONE_REG(0x01, (BIN_to_BCD(mins)));
  }
  //---
  void HOURS_write(uint8_t hours)
  {
    hours = hours % 60;
    WRITE_ONE_REG(0x02, (BIN_to_BCD(hours)));
  }
  //---
  void MDAY_write(uint8_t m_day)
  {
    WRITE_ONE_REG(0x04, (BIN_to_BCD(m_day)));
  }
  //---
  void MONTH_write(uint8_t month)
  {
    uint8_t m_data = BIN_to_BCD(month);
    m_data += 128;
    WRITE_ONE_REG(0x05, m_data);
  }
  //---
  void YEAR_write(uint8_t rok)
  {
    WRITE_ONE_REG(0x06, (BIN_to_BCD(rok)));
    Serial.printf("Year  value : %d \n\n", rok);
  }
  //---
  void WRITE_ONE_REG(uint8_t addr, uint8_t data)
  {
    swWire.beginTransmission(DS3231_I2C_ADDRESS);
    swWire.write(addr);
    swWire.write(data);
    swWire.endTransmission();
  }
//---

private:
  SoftWire swWire;
  //-------------------------------------------------------
  // Convert normal decimal numbers to binary coded decimal
  byte BIN_to_BCD(byte val)
  {
    return (((val / 10) << 4) + (val % 10));
  }
  //---
  //--------------------------------------------------------------
  // Convert binary coded decimal to normal decimal numbers
  byte BCD_to_BIN(byte val)
  {
    return (((val >> 4) * 10) + (val & 0x0f));
  }
  //---
  //---------------------------------------------------------------
};

//=================================================================================================================================
