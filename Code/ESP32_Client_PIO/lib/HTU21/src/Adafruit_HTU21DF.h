/*!
 * @file Adafruit_HTU21DF.h
 */

 #ifndef _ADAFRUIT_HTU21DF_H
 #define _ADAFRUIT_HTU21DF_H
 
 #include <Adafruit_BusIO_Register.h>
 #include <Adafruit_I2CDevice.h>
 
 /** Default I2C address for the HTU21D. */
 #define HTU21DF_I2CADDR (0x40)
 
 /** Read temperature register. */
 #define HTU21DF_READTEMP (0xE3)
 
 /** Read humidity register. */
 #define HTU21DF_READHUM (0xE5)
 
 /** Write register command. */
 #define HTU21DF_WRITEREG (0xE6)
 
 /** Read register command. */
 #define HTU21DF_READREG (0xE7)
 
 /** Reset command. */
 #define HTU21DF_RESET (0xFE)
 
 /**
  * Driver for the Adafruit HTU21DF breakout board.
  */
 class Adafruit_HTU21DF {
 public:
   Adafruit_HTU21DF();
 
   bool begin(TwoWire *theWire = &Wire);
   float readTemperature(void);
   float readHumidity(void);
   void reset(void);
 
 private:
   Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface
   float _last_humidity, _last_temp;
 };
 
 #endif /* _ADAFRUIT_HTU21DF_H */