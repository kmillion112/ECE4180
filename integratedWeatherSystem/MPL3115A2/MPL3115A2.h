/*
 * File description
 *
 */
#ifndef MPL3115A2_H
#define MPL3115A2_H

#include "mbed.h"

/**
 * MPL3115A2 I2C Precision Altimeter
 *
 */

class MPL3115A2
{
public:
 /**
  * MPL3115A2
  *
  * @param sda SDA pin
  * @param scl SCL pin
  * @param addr address of the I2C peripheral
  */
MPL3115A2(PinName sda, PinName scl, int addr) ;

~MPL3115A2() ;

/*
 * some member functions here)
 */
uint8_t getStatus(void) ;
/**
 * get Altitude (m)
 *
 * @param none
 * @returns double altitude in meter
 */
double getAltitude(void) ; 

/**
 * get Pressure
 *
 * @param none
 * @returns double pressure in hPa
 */
double getPressure(void) ; 

/**
 * get Temperature
 *
 * @param none
 * @returns double temperature in degree Celsius
 */
double getTemperature(void) ;

int32_t  getAltDelta(void) ; /* x65536 */
int32_t  getPrsDelta(void) ; /* x16 */
int16_t  getTempDelta(void) ; 
uint8_t  getID(void) ;
uint8_t  getFstatus(void) ;
uint8_t  getFdata(void) ;
uint8_t  getFsetup(void) ;
void     setFsetup(uint8_t data) ;
uint8_t  getTimeDelay(void) ;
uint8_t  getSysMod(void) ;
uint8_t  getIntSource(void) ;
uint8_t  getPtDataConfig(void) ;
void     setPtDataConfig(uint8_t data) ;
uint16_t getBarIn(void) ;
void     setBarIn(uint16_t data) ;
int16_t  getP_TGT(void) ;
void     setP_TGT(int16_t data) ;
int8_t   getT_TGT(void) ;
void     setT_TGT(int8_t data) ;
uint16_t getP_WND(void) ;
void     setP_WND(uint16_t data) ;
uint8_t  getT_WND(void) ;
void     setT_WND(uint8_t data) ;
int32_t  getP_MIN(void) ;
void     setT_MIN(int32_t data) ;
int32_t  getP_MAX(void) ;
void     setP_MAX(int32_t data) ;
int16_t  getT_MIN(void) ;
void     setT_MIN(int16_t data) ;
int16_t  getT_MAX(void) ;
void     setT_MAX(int16_t data) ;
uint8_t  getCTRL_REG1(void) ;
void     setCTRL_REG1(uint8_t data) ;
uint8_t  getCTRL_REG2(void) ;
void     setCTRL_REG2(uint8_t data) ;
uint8_t  getCTRL_REG3(void) ;
void     setCTRL_REG3(uint8_t data) ;
uint8_t  getCTRL_REG4(void) ;
void     setCTRL_REG4(uint8_t data) ;
uint8_t  getCTRL_REG5(void) ;
void     setCTRL_REG5(uint8_t data) ;
uint16_t getSampleTime(void) ;
void     oneShot(void) ;
void     standby(void) ;
void     activate(void) ;
void     modeAlt(void) ;
void     modeBar(void) ;
private:
  I2C m_i2c;
  int m_addr ;
  void readRegs(int addr, uint8_t *data, int len) ;
  void writeRegs(uint8_t *data, int len) ;
} ;
#endif /* MPL3115A2_H */
