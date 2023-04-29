#include "MPL3115A2/MPL3115A2.h"
#include "mbed.h"
#include "HTU21D.h"
#include "MPL3115A2.h"

#define MPL3115A2_I2C_ADDRESS (0x60);

Serial pc(USBTX, USBRX);

HTU21D temphumid(p9, p10);

AnalogIn lightSensor(p20); 

int sample_ftemp;
int sample_ctemp;
int sample_ktemp;
int sample_humid;

float lightLevel = 0;

int main()
{
    double alt, bar, temp ;
    
    MPL3115A2 mpl(p28, p27, (0x60)) ;
    
    pc.printf("Beginning...");
    pc.printf("\n\r");

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

        pc.printf("Temperature: %d F\n\r", sample_ftemp);
        pc.printf("Temperature: %d C\n\r", sample_ctemp);
        pc.printf("Temperature: %d K\n\r", sample_ktemp);
        pc.printf("Humidity: %d %%\n\r", sample_humid);
        pc.printf("Light level %.2f %%\n\r", lightLevel);
        pc.printf("Altitude: %.2f %\n\r", alt);
        pc.printf("Barometric: %.2f %\n\r", bar);
        pc.printf("Temperature Two: %.2f %\n\r", temp);        
        pc.printf("\n\r");
        wait(.2);
    }
}
