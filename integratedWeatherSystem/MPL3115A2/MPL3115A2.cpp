/*
 * File description here
 */
#include "MPL3115A2.h"
#include <cstdint>

/* some definitions here */
#define MPL_STATUS          0x00
#define MPL_OUT_P_MSB       0x01
#define MPL_OUT_P_CSB       0x02
#define MPL_OUT_P_LSB       0x03
#define MPL_OUT_T_MSB       0x04
#define MPL_OUT_T_LSB       0x05
#define MPL_DR_STATUS       0x06
#define MPL_OUT_P_DELTA_MSB 0x07
#define MPL_OUT_P_DELTA_CSB 0x08
#define MPL_OUT_P_DELTA_LSB 0x09
#define MPL_OUT_T_DELTA_MSB 0x0A
#define MPL_OUT_T_DELTA_LSB 0x0B
#define MPL_WHO_AM_I        0x0C
#define MPL_F_STATUS        0x0D
#define MPL_F_DATA          0x0E
#define MPL_F_SETUP         0x0F
#define MPL_TIME_DLY        0x10
#define MPL_SYSMOD          0x11
#define MPL_INT_SOURCE      0x12
#define MPL_PT_DATA_CFG     0x13
#define MPL_BAR_IN_MSB      0x14
#define MPL_BAR_IN_LSB      0x15
#define MPL_P_TGT_MSB       0x16
#define MPL_P_TGT_LSB       0x17
#define MPL_T_TGT           0x18
#define MPL_P_WND_MSB       0x19
#define MPL_P_WND_LSB       0x1A
#define MPL_T_WND           0x1B
#define MPL_P_MIN_MSB       0x1C
#define MPL_P_MIN_CSB       0x1D
#define MPL_P_MIN_LSB       0x1E
#define MPL_T_MIN_MSB       0x1F
#define MPL_T_MIN_LSB       0x20
#define MPL_P_MAX_MSB       0x21
#define MPL_P_MAX_CSB       0x22
#define MPL_P_MAX_LSB       0x23
#define MPL_T_MAX_MSB       0x24
#define MPL_T_MAX_LSB       0x25
#define MPL_CTRL_REG1       0x26
#define MPL_CTRL_REG2       0x27
#define MPL_CTRL_REG3       0x28
#define MPL_CTRL_REG4       0x29
#define MPL_CTRL_REG5       0x2A
#define MPL_OFF_P           0x2B
#define MPL_OFF_T           0x2C
#define MPL_OFF_H           0x2D
/*
 * If F_MODE = 0, FIFO is disabled
 *
 * 0x00/0x06 Sensor Status Register (DR_STATUS)
 * 0x01 Pressure Data Out MSB (OUT_P_MSB)
 * 0x02 Pressure Data Out CSB (OUT_P_CSB)
 * 0x03 Pressure Data Out LSB (OUT_P_LSB)
 * 0x04 Temperature Data Out MSB (OUT_T_MSB)
 * 0x05 Temperature Data out LSB (OUT_T_LSB)
 *
 * If F_Mode > 0, FIFO is circular buffer or full stop mode
 *
 * 0x00/0x0D Sensor Status Register (F_STATUS)
 * 0x01 FIFO 8-bit Data Access (F_DATA)
 * 0x02 Read to Reserved Area returns 00
 * 0x03 Read to Reserved Area returns 00
 * 0x04 Read to Reserved Area returns 00
 * 0x05 Read to Reserved Area returns 00
 *
 * F_MODE = 00, FIFO is disabled
 * F_MODE = 01, FIFO is circulated buffer
 * F_MODE = 10, FIFO is full stop mode
 */

    
MPL3115A2::MPL3115A2(PinName sda, PinName scl, int addr) : m_i2c(sda, scl), m_addr(addr<<1) {
    // activate the peripheral
    standby() ;
    setCTRL_REG1( 0x38 ) ; /* oversample 32 */
    activate() ;
}

MPL3115A2::~MPL3115A2() { }

void MPL3115A2::readRegs(int addr, uint8_t * data, int len) {
    char t[1] = {addr} ;
    m_i2c.write(m_addr, t, 1, true) ;
    m_i2c.read(m_addr, (char*)data, len) ;
}

void MPL3115A2::writeRegs(uint8_t * data, int len) {
   m_i2c.write(m_addr, (char *)data, len) ;
}

uint8_t MPL3115A2::getStatus(void) {
    uint8_t data[1] ;
    readRegs(MPL_DR_STATUS, data, 1) ;
    return( data[0] ) ;
}

/*
 * getAltitude returns the altitude in meters times 65536
 */
double MPL3115A2::getAltitude(void)
{
    uint8_t tmp[3] ;
    uint16_t sample_time ;
    int32_t data ;
 
    standby() ;
    modeAlt() ;
    sample_time = getSampleTime() ;
    activate() ;
    oneShot() ;
    wait_ms(sample_time) ;
    readRegs(MPL_OUT_P_MSB, tmp, 3) ;
    data = (tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8) ;
    return( ((double)data)/65536.0 ) ;
}

/*
 * getPressure returns the pressure in Pascals times 64
 */
double MPL3115A2::getPressure(void)
{
    uint8_t tmp[3] ;
    uint32_t data ;
    uint16_t sample_time ;

    standby() ;
    modeBar() ;
    sample_time = getSampleTime() ;
    activate() ;
    oneShot() ;
    wait_ms(sample_time) ;
    readRegs(MPL_OUT_P_MSB, tmp, 3) ;
    data = ((tmp[0]<<16)|(tmp[1]<<8)|(tmp[2])) >> 6  ;
    return(((double)data) / 100.0 ) ;
} 

/*
 * getTemperature returns the temperature in c-degree times 256
 */
double MPL3115A2::getTemperature(void)
{
    uint8_t tmp[2] ;
    uint16_t data ;
    
    readRegs(MPL_OUT_T_MSB, tmp, 2) ;
    data = (tmp[0]<<8)|(tmp[1]) ;
    return( ((double)data)/256.0  ) ;
}

/*
 * getAltDelta returns delta of alt in meters times 65536
 */
int32_t MPL3115A2::getAltDelta(void)
{
    uint8_t tmp[3] ;
    int32_t data ;
    
    readRegs(MPL_OUT_P_DELTA_MSB, tmp, 3) ;
    data = (tmp[0]<<24)|(tmp[1]<<16)|((tmp[2]&0xF0)<<8) ;
    return( data ) ;
}

/*
 * getPrsDelta returns delta of pressure in Pascal times 16
 */
int32_t MPL3115A2::getPrsDelta(void)
{
    uint8_t tmp[3] ;
    int32_t data ;
    
    readRegs(MPL_OUT_P_DELTA_MSB, tmp, 3) ;
    data = (tmp[0]<<24)|(tmp[1]<<16)|((tmp[2]&0xF0)<<8) ;
    return( data ) ;
}

/*
 * getTempDelta returns delta of temperature in c-degree times 16
 */
int16_t MPL3115A2::getTempDelta(void)
{
    uint8_t tmp[2] ;
    int16_t data ;
    
    readRegs(MPL_OUT_T_DELTA_MSB, tmp, 2) ;
    data = (tmp[0]<<8) | (tmp[1]&0xF0) ;
    return( data ) ;
}

uint8_t MPL3115A2::getID(void)
{
    uint8_t tmp[1] ;
    readRegs(MPL_WHO_AM_I, tmp, 1) ;
    return( tmp[0] ) ;
}

uint8_t MPL3115A2::getFstatus(void)
{
    uint8_t tmp[1] ;
    readRegs(MPL_F_STATUS, tmp, 1) ;
    return(tmp[0]) ;
}

uint8_t MPL3115A2::getFdata(void)
{
    uint8_t tmp[1] ;
    readRegs(MPL_F_DATA, tmp, 1) ;
    return(tmp[0]) ;
}

uint8_t MPL3115A2::getFsetup(void)
{
    uint8_t tmp[1] ;
    readRegs(MPL_F_SETUP, tmp, 1) ;
    return(tmp[0]) ;
}

void MPL3115A2::setFsetup(uint8_t data)
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_F_SETUP ;
    tmp[1] = data ;
    writeRegs(tmp, 2) ;
}

uint8_t MPL3115A2::getTimeDelay(void)
{
    uint8_t tmp[1] ;
    readRegs(MPL_TIME_DLY, tmp, 1) ;
    return( tmp[0] ) ;
}

uint8_t MPL3115A2::getSysMod(void)
{
    uint8_t tmp[1] ;
    readRegs(MPL_SYSMOD, tmp, 1) ;
    return( tmp[0] ) ;
}

uint8_t MPL3115A2::getIntSource(void)
{
    uint8_t tmp[1] ;
    readRegs(MPL_INT_SOURCE, tmp, 1) ;
    return( tmp[0] ) ;
}

uint8_t MPL3115A2::getPtDataConfig(void)
{
    uint8_t tmp[1] ;
    readRegs(MPL_PT_DATA_CFG, tmp, 1) ;
    return( tmp[0] ) ;
}

void MPL3115A2::setPtDataConfig(uint8_t data)
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_PT_DATA_CFG ;
    tmp[1] = data ;
    writeRegs(tmp, 2) ;
}

uint16_t MPL3115A2::getBarIn(void)
{
    uint8_t tmp[2] ;
    readRegs(MPL_BAR_IN_MSB, tmp, 2) ;
    return( (uint16_t)tmp[0] ) ;
}

void MPL3115A2::setBarIn(uint16_t data)
{
    uint8_t tmp[3] ;
    tmp[0] = MPL_BAR_IN_MSB ;
    tmp[1] = (data >> 8)&0xFF ;
    tmp[2] = data & 0xFF ;
    writeRegs(tmp, 3) ;
}

int16_t MPL3115A2::getP_TGT(void)
{
    uint8_t tmp[2] ;
    int16_t data ;
    readRegs(MPL_P_TGT_MSB, tmp, 2) ;
    data = (tmp[0] << 8) | tmp[1] ;
    return( data ) ;
}

void MPL3115A2::setP_TGT(int16_t data)
{
    uint8_t tmp[3] ;
    tmp[0] = MPL_P_TGT_MSB ;
    tmp[1] = (data >> 8) & 0xFF ;
    tmp[2] = data & 0xFF ;
    writeRegs(tmp, 3) ;
}

int8_t MPL3115A2::getT_TGT(void)
{
    uint8_t tmp[1] ;
    readRegs(MPL_T_TGT, tmp, 1) ;
    return( (int8_t)tmp[0] ) ;
}

void MPL3115A2::setT_TGT(int8_t data)
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_PT_DATA_CFG ;
    tmp[1] = data ;
    writeRegs(tmp, 2) ;
}

uint16_t MPL3115A2::getP_WND(void) 
{
    uint8_t tmp[2] ;
    uint16_t data ;
    readRegs(MPL_P_WND_MSB, tmp, 2) ;
    data = (tmp[0]<<8) | tmp[1] ;
    return(data) ;
}

void     MPL3115A2::setP_WND(uint16_t data) 
{
    uint8_t tmp[3] ;
    tmp[0] = MPL_P_WND_MSB ;
    tmp[1] = (data >> 8)&0xFF ;
    tmp[2] = data & 0xFF ;
    writeRegs(tmp, 3) ;
}

uint8_t  MPL3115A2::getT_WND(void) 
{
    uint8_t tmp[1] ;
    readRegs(MPL_T_WND, tmp, 1) ;
    return(tmp[0]) ;
}

void     MPL3115A2::setT_WND(uint8_t data) 
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_T_WND ;
    tmp[1] = data ;
    writeRegs(tmp, 2) ;
}

int32_t  MPL3115A2::getP_MIN(void) 
{
    uint8_t tmp[3] ;
    int32_t data ;
    readRegs(MPL_P_MIN_MSB, tmp, 3) ;
    data = (tmp[0]<<24)|(tmp[1]<<16)|((tmp[2]&0xF0)<<8) ;
    return( data ) ;
}

void     MPL3115A2::setT_MIN(int32_t data) 
{
    uint8_t tmp[4] ;
    tmp[0] = MPL_P_MIN_MSB ;
    tmp[1] = (data >> 24) & 0xFF ;
    tmp[2] = (data >> 16) & 0xFF ;
    tmp[3] = (data >>  8) & 0xF0 ;
    writeRegs(tmp, 4) ;
}

int32_t  MPL3115A2::getP_MAX(void) 
{
    uint8_t tmp[3] ;
    int32_t data ;
    readRegs(MPL_P_MAX_MSB, tmp, 3) ;
    data = (tmp[0]<<24)|(tmp[1]<<16)|((tmp[2]&0xF0)<<8) ;
    return( data ) ;
}

void     MPL3115A2::setP_MAX(int32_t data) 
{
    uint8_t tmp[4] ;
    tmp[0] = MPL_P_MAX_MSB ;
    tmp[1] = (data >> 24) & 0xFF ;
    tmp[2] = (data >> 16) & 0xFF ;
    tmp[3] = (data >>  8) & 0xF0 ;
    writeRegs(tmp, 4) ;    
}

int16_t  MPL3115A2::getT_MIN(void) 
{
    uint8_t tmp[2] ;
    uint16_t data ;
    readRegs(MPL_T_MIN_MSB, tmp, 2) ;
    data = (tmp[0]<<8) | tmp[1] ;
    return(data) ;
}

void     MPL3115A2::setT_MIN(int16_t data) 
{
    uint8_t tmp[3] ;
    tmp[0] = MPL_T_MIN_MSB ;
    tmp[1] = (data >> 8) & 0xFF ;
    tmp[2] = data & 0xFF ;
    writeRegs(tmp, 3) ;
}

int16_t  MPL3115A2::getT_MAX(void) 
{
    uint8_t tmp[2] ;
    uint16_t data ;
    readRegs(MPL_T_MAX_MSB, tmp, 2) ;
    data = (tmp[0]<<8) | tmp[1] ;
    return( data ) ;
}

void     MPL3115A2::setT_MAX(int16_t data) 
{
    uint8_t tmp[3] ;
    tmp[0] = MPL_T_MAX_MSB ;
    tmp[1] = (data >> 8) & 0xFF ;
    tmp[2] = data & 0xFF ;
    writeRegs(tmp, 3) ;
}

uint8_t  MPL3115A2::getCTRL_REG1(void) 
{
    uint8_t tmp[1] ;
    readRegs(MPL_CTRL_REG1, tmp, 1) ;
    return( tmp[0] ) ;
}

void     MPL3115A2::setCTRL_REG1(uint8_t data) 
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_CTRL_REG1 ;
    tmp[1] = data ;
    writeRegs(tmp, 2) ;
}

uint8_t  MPL3115A2::getCTRL_REG2(void) 
{
    uint8_t tmp[1] ;
    readRegs(MPL_CTRL_REG2, tmp, 1) ;
    return( tmp[0] ) ;
}

void     MPL3115A2::setCTRL_REG2(uint8_t data) 
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_CTRL_REG2 ;
    tmp[1] = data ;
    writeRegs(tmp, 2) ;
}

uint8_t  MPL3115A2::getCTRL_REG3(void) 
{
    uint8_t tmp[1] ;
    readRegs(MPL_CTRL_REG3, tmp, 1) ;
    return( tmp[0] ) ;
}

void     MPL3115A2::setCTRL_REG3(uint8_t data) 
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_CTRL_REG3 ;
    tmp[1] = data ;
    writeRegs(tmp, 2) ;
}

uint8_t  MPL3115A2::getCTRL_REG4(void) 
{
    uint8_t tmp[1] ;
    readRegs(MPL_CTRL_REG4, tmp, 1) ;
    return( tmp[0] ) ;
}

void     MPL3115A2::setCTRL_REG4(uint8_t data) 
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_CTRL_REG4 ;
    tmp[1] = data ;
    writeRegs(tmp, 2) ;
}

uint8_t  MPL3115A2::getCTRL_REG5(void) 
{
    uint8_t tmp[1] ;
    readRegs(MPL_CTRL_REG5, tmp, 1) ;
    return( tmp[0] ) ;
}

void     MPL3115A2::setCTRL_REG5(uint8_t data) 
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_CTRL_REG5 ;
    tmp[1] = data ;
    writeRegs(tmp, 2) ;
}

void MPL3115A2::oneShot(void)
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_CTRL_REG1 ;
    readRegs(tmp[0], &tmp[1], 1) ;
    tmp[1] &= 0xFD ; /* clear OST */
    writeRegs(tmp, 2) ;
    tmp[1] |= 0x02 ; /* set OST */
    writeRegs(tmp, 2) ;
}

uint16_t MPL3115A2::getSampleTime(void) 
{
    uint8_t tmp[2] ;
    uint16_t sample_time = 6 ;
    tmp[0] = MPL_CTRL_REG1 ;
    readRegs(tmp[0], &tmp[1], 1) ;
    tmp[1] |= 0x02 ;
    writeRegs(tmp, 2) ;
    switch((tmp[1]>>3)&0x07) {
    case 0: sample_time = 6 ; break ;
    case 1: sample_time = 10 ; break ;
    case 2: sample_time = 18 ; break ;
    case 3: sample_time = 34 ; break ;
    case 4: sample_time = 66 ; break ;
    case 5: sample_time = 130 ; break ;
    case 6: sample_time = 258 ; break ;
    case 7: sample_time = 512 ; break ;
    default: sample_time = 6 ; break ;
    }
    return( sample_time ) ;
}

void MPL3115A2::standby(void)
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_CTRL_REG1 ;
    readRegs(tmp[0], &tmp[1], 1) ;
    tmp[1] &= 0xFE ;
    writeRegs(tmp, 2) ;
}

void MPL3115A2::activate(void)
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_CTRL_REG1 ;
    readRegs(tmp[0], &tmp[1], 1) ;
    tmp[1] |= 0x01 ;
    writeRegs(tmp, 2) ;
}

void MPL3115A2::modeAlt(void) 
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_CTRL_REG1 ;
    readRegs(tmp[0], &tmp[1], 1) ;
    tmp[1] |= 0x80 ;
    writeRegs(tmp, 2) ;
}

void MPL3115A2::modeBar(void)
{
    uint8_t tmp[2] ;
    tmp[0] = MPL_CTRL_REG1 ;
    readRegs(tmp[0], &tmp[1], 1) ;
    tmp[1] &= 0x7F ;
    writeRegs(tmp, 2) ;
}