#include "MPL3115A2/MPL3115A2.h"
#include "mbed.h"
#include "HTU21D.h"
#include "MPL3115A2.h"
#include "LSM9DS1.h"

#define MPL3115A2_I2C_ADDRESS (0x60);

Serial pc(USBTX, USBRX);

HTU21D temphumid(p9, p10);

LSM9DS1 IMU(p9, p10, 0xD6, 0x3C);

AnalogIn lightSensor(p20); 

int sample_ftemp;
int sample_ctemp;
int sample_ktemp;
int sample_humid;

float lightLevel = 0;

#define PI 3.14159

#define DECLINATION -4.94 // Declination (degrees) in Atlanta,GA.

void printAttitude(float ax, float ay, float az, float mx, float my, float mz)
{
    float roll = atan2(ay, az);
    float pitch = atan2(-ax, sqrt(ay * ay + az * az));

    mx = -mx;
    float heading;
    
    if (my == 0.0)
        heading = (mx < 0.0) ? 180.0 : 0.0;
    else
        heading = atan2(mx, my)*360.0/(2.0*PI);
    
    heading -= DECLINATION;
    
    if(heading>180.0) heading = heading - 360.0;
    else if(heading<-180.0) heading = 360.0 + heading;
    else if(heading<0.0) heading = 360.0  + heading;

    pitch *= 180.0 / PI;
    roll  *= 180.0 / PI;

    pc.printf("Pitch: %f,    Roll: %f degress\n\r",pitch,roll);
    pc.printf("Magnetic Heading: %f degress\n\r",heading);
}

int main()
{
    double alt, bar, temp ;
    
    MPL3115A2 mpl(p28, p27, (0x60)) ;
    
    pc.printf("Beginning...");
    pc.printf("\n\r");

    IMU.begin();
    if (!IMU.begin()) {
        pc.printf("Failed to communicate with LSM9DS1.\n");
    }

    wait(1);

    while(1) 
    {
        sample_ftemp = temphumid.sample_ftemp();
        sample_ctemp = temphumid.sample_ctemp();
        sample_ktemp = temphumid.sample_ktemp();
        sample_humid = temphumid.sample_humid();

        alt = mpl.getAltitude() ;
        bar = mpl.getPressure() ;
        temp = mpl.getTemperature() ;

        lightLevel = lightSensor;

        while(!IMU.tempAvailable());
        IMU.readTemp();
        while(!IMU.magAvailable(X_AXIS));
        IMU.readMag();
        while(!IMU.accelAvailable());
        IMU.readAccel();
        while(!IMU.gyroAvailable());
        IMU.readGyro();

        pc.printf("Temperature: %d F\n\r", sample_ftemp);
        pc.printf("Temperature: %d C\n\r", sample_ctemp);
        pc.printf("Temperature: %d K\n\r", sample_ktemp);
        pc.printf("Humidity: %d %%\n\r", sample_humid);
        pc.printf("Light level %.2f %%\n\r", lightLevel);
        pc.printf("Altitude: %.2f %\n\r", alt);
        pc.printf("Barometric: %.2f %\n\r", bar);
        pc.printf("Temperature Two: %.2f %\n\r", temp);   
        pc.printf("\nIMU Temperature = %f C\n\r",25.0 + IMU.temperature/16.0);
        pc.printf("        X axis    Y axis    Z axis\n\r");
        pc.printf("gyro:  %9f %9f %9f in deg/s\n\r", IMU.calcGyro(IMU.gx), IMU.calcGyro(IMU.gy), IMU.calcGyro(IMU.gz));
        pc.printf("accel: %9f %9f %9f in Gs\n\r", IMU.calcAccel(IMU.ax), IMU.calcAccel(IMU.ay), IMU.calcAccel(IMU.az));
        pc.printf("mag:   %9f %9f %9f in gauss\n\r", IMU.calcMag(IMU.mx), IMU.calcMag(IMU.my), IMU.calcMag(IMU.mz));
        
        printAttitude(IMU.calcAccel(IMU.ax), IMU.calcAccel(IMU.ay), IMU.calcAccel(IMU.az), IMU.calcMag(IMU.mx), IMU.calcMag(IMU.my), IMU.calcMag(IMU.mz));    
        
        pc.printf("\n\r");
        
        wait(2);
    }
}
