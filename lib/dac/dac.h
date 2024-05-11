#include "MCP4728.h"
MCP4728 dac;

void printStatus()
{
  Serial.println("NAME     Vref  Gain  PowerDown  DACData");
  for (int i = 0; i < 4; ++i)
  {
    Serial.print("DAC");
    Serial.print(i, DEC);
    Serial.print("   ");
    Serial.print("    ");
    Serial.print(dac.getVref(i), BIN);
    Serial.print("     ");
    Serial.print(dac.getGain(i), BIN);
    Serial.print("       ");
    Serial.print(dac.getPowerDown(i), BIN);
    Serial.print("       ");
    Serial.println(dac.getDACData(i), DEC);

    Serial.print("EEPROM");
    Serial.print(i, DEC);
    Serial.print("    ");
    Serial.print(dac.getVref(i, true), BIN);
    Serial.print("     ");
    Serial.print(dac.getGain(i, true), BIN);
    Serial.print("       ");
    Serial.print(dac.getPowerDown(i, true), BIN);
    Serial.print("       ");
    Serial.println(dac.getDACData(i, true), DEC);
  }
  Serial.println(" ");
}

void setupDac()
{
  // DAC setup
  Wire.begin(D2, D1);
  Wire.setClock(400000L);
  dac.attatch(Wire, D5);
  dac.readRegisters();
  dac.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
  dac.selectPowerDown(MCP4728::PWR_DOWN::GND_500KOHM, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL);
  dac.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
  dac.enable(true);
}