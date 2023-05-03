#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "DebounceIn.h"
#include "HTU21D.h"
#include "LSM9DS1.h"
#include "MPL3115A2.h"
#include "MPL3115A2/MPL3115A2.h"
#include "PinDetect.h"
#include "SDFileSystem.h"
#include "mbed.h"
#include "uLCD_4DGL.h"
#include "wave_player.h"


#include<time.h>
#include<math.h>



#define MPL3115A2_I2C_ADDRESS (0x60);
#define PI 3.14159
#define DECLINATION -4.94  // Declination (degrees) in Atlanta,GA.

SDFileSystem sd(p5, p6, p7, p8, "sd");  // SD card

AnalogOut DACout(p18);

wave_player waver(&DACout);

// connected modules, pc included to help with debugging purposes for any device
// upgrades
Serial pc(USBTX, USBRX);
Serial blue(p28, p27);
uLCD_4DGL uLCD(p13, p14, p11);

// Hardware Sensors
HTU21D temphumid(p9, p10);
LSM9DS1 IMU(p9, p10, 0xD6, 0x3C);
MPL3115A2 mpl(p9, p10, (0x60));
//ensor(p20);

// mbed led output
BusOut myled(LED1, LED2, LED3, LED4);
/*
PinDetect Up(p16);
PinDetect Down(p17);
PinDetect Left(p5);
PinDetect Right(p15);
PinDetect ScreenSwitch(p14);
*/

//Thread timingThread;
int y;
int mo;
int d;
int h;
int m;
Timer secondsT;

int selectedValue = 0;

// global sensor data saved
int selectDate = 0;
int sample_ftemp;
int sample_ctemp;
int sample_ktemp;
int sample_humid;

// conditions for screen
int backColor;
float outClockS = 0;
float outClockM = 0;
float outClockH = 0;

const char *returnDay(int year, int month, int day) {
    // 4/26/23 is base date - Wednesday
    int baseyear = 23;
    int basemonth = 4;
    int baseday = 26;

    year = (year - baseyear) * 365;
    month = month - basemonth;
    day = day - baseday;

    if (mo == 4 || mo == 6 || mo == 9 || mo == 11) {
        month = month * 30;
    } else if (mo == 2) {
        month = month * 28;
    } else {
        month = month * 31;
    }
    day = day + month + year;
    day = abs(day);
    day = day % 7;
    if (day == 0) {
        return "Wedn";
    }
    if (day == 1) {
        return "Thrus";
    }
    if (day == 2) {
        return "Fri";
    }
    if (day == 3) {
        return "Sat";
    }
    if (day == 4) {
        return "Sun";
    }
    if (day == 5) {
        return "Mon";
    }
    if (day == 6) {
        return "Tues";
    }
    return "Fun";
}
void minPassed() {
    secondsT.reset();
    m++;
    if (m == 60) {
        m = 0;
        h++;
    }
}

void headerSecUpdate() {
    string dayOfWeek = returnDay(y, mo, d);

    int secs = secondsT.read();
    uLCD.color(WHITE);
    if (secondsT.read() >= 60) {
        minPassed();
    }
}


int returnDayColor() {
    int red, green, blue;
    int totalC;
    if (h >= 12) {
        blue = h - 24;
    }

    blue = 19 * abs(blue);
    if (blue >= 255) {
        blue = 255;
    }
    red = blue / 2;
    green = blue / 2;
    red = red << 16;
    green = green << 8;
    totalC = red + green + blue;
    return totalC;
}

string toLower(string in) {
    for (int i = 0; i < in.length(); i++) {
        in[i] = tolower(in[i]);
    }
    return in;
}

void setHeader(bool clear, bool init) {
    if (clear) {
        uLCD.cls();
        backColor = returnDayColor();
        uLCD.filled_rectangle(0, 0, 150, 150, backColor);
    }

    uLCD.filled_rectangle(0, 0, 150, 17, BLACK);
    uLCD.filled_rectangle(0, 17, 150, 18, WHITE);
    uLCD.color(WHITE);
    uLCD.locate(0, 0);
    uLCD.printf("4180 Weather App\n");

    if (!init) {
        string dayOfWeek = returnDay(y, mo, d);
        int secs = secondsT.read();
        uLCD.printf("%d/%d/%d %s-%d:%d\n", d, mo, y, dayOfWeek, h, m);
    }
}

void secHand() {
    int sec;
    uLCD.line(65, 70, 65 + 40 * cos(outClockS), 70 + 40 * sin(outClockS), BLACK);
    sec = secondsT.read();
    outClockS = sec * (PI / 30) -(PI /2); 
    uLCD.color(RED);
    uLCD.line(65, 70, 65 + 40 * cos(outClockS), 70 + 40 * sin(outClockS), RED);
}

void hrHand() {
    int hr, min;
    
    hr = h;
    min = m;
    uLCD.line(65, 70, 65 + 20 * cos(outClockH), 70 + 20 * sin(outClockH), BLACK);

    if (hr <= 12){
        outClockH = (hr * (PI / 6) - (PI / 2)) + ((min / 12) * (PI / 30));
    } 
    else if (hr > 12){
        outClockH = ((hr - 12) * (PI / 6) - (PI / 2)) + ((min / 12) * (PI / 30));
    }
    uLCD.color(BLUE);
    uLCD.line(65, 70, 65 + 20 * cos(outClockH), 70 + 20 * sin(outClockH), BLUE);
}

void minHand() {
    int min;
   
    min = m;
    min = 50;
    uLCD.line(65, 70, 65 + 30 * cos(outClockM), 70 + 30 * sin(outClockM), BLACK);

    outClockM = (min * (PI / 30) - (PI /2)); 
    uLCD.color(GREEN);
    uLCD.line(65, 70, 65 + 30 * cos(outClockM), 70 + 30 * sin(outClockM), GREEN);
}

void clockLCD() {
    myled = '1';
    setHeader(true,false);
     uLCD.filled_rectangle(0, 15, 40, 35, WHITE);
    uLCD.filled_rectangle(0, 17, 38, 33, BLACK);
    uLCD.locate(0,3);
    uLCD.printf("Clock\n");

    uLCD.filled_circle(65, 70, 40, WHITE);
    uLCD.filled_circle(65, 70, 38, BLACK);
    uLCD.filled_circle(65, 70, 2, WHITE);
    uLCD.filled_rectangle(0, 114, 200, 200, BLACK);

    uLCD.locate(0,15);
    uLCD.color(RED);
    uLCD.printf("             Secs");
    uLCD.locate(0,15);
    uLCD.color(GREEN);
    uLCD.printf("       Mins");
    uLCD.locate(0,15);
    uLCD.color(BLUE);
    uLCD.printf(" Hours");
    uLCD.baudrate(16800);
    while (1) {
        secHand();
        minHand();
        hrHand();
        headerSecUpdate();
    }
}


void printAttitude(float ax, float ay, float az, float mx, float my, float mz) {
    float roll = atan2(ay, az);
    float pitch = atan2(-ax, sqrt(ay * ay + az * az));

    mx = -mx;
    float heading;

    if (my == 0.0)
        heading = (mx < 0.0) ? 180.0 : 0.0;
    else
        heading = atan2(mx, my) * 360.0 / (2.0 * PI);

    heading -= DECLINATION;

    if (heading > 180.0)
        heading = heading - 360.0;
    else if (heading < -180.0)
        heading = 360.0 + heading;
    else if (heading < 0.0)
        heading = 360.0 + heading;

    pitch *= 180.0 / PI;
    roll *= 180.0 / PI;

    uLCD.printf("Pitch: %.4f\nRoll: %.4f degrees \n", pitch, roll);
    uLCD.printf("Magnetic Heading:\n %.3f degrees\n", heading);
}
void generalDataScreen() {
    myled = '0';
    uLCD.cls();
    setHeader(false,false);

    uLCD.filled_rectangle(0, 17, 90, 35, WHITE);
    uLCD.filled_rectangle(0, 16, 88, 33, BLACK);
    uLCD.locate(0,3);
    uLCD.printf("General Data\n");

    double alt, bar, temp;
    if (!IMU.begin()) {
        pc.printf("Failed to communicate with LSM9DS1.\n");
    }
        IMU.readTemp();
        IMU.readMag();
        IMU.readGyro();

        sample_ftemp = temphumid.sample_ftemp();
        sample_ctemp = temphumid.sample_ctemp();
        sample_ktemp = temphumid.sample_ktemp();
        sample_humid = temphumid.sample_humid();

        alt = mpl.getAltitude();
        bar = mpl.getPressure();
        temp = mpl.getTemperature();

        while (!IMU.tempAvailable()) {
            IMU.readTemp();
        }
        while (!IMU.magAvailable(X_AXIS)){
          IMU.readMag();
        }
        while (!IMU.accelAvailable()){
            IMU.readAccel();
        }
        while (!IMU.gyroAvailable()){
            IMU.readGyro();
        }
        selectDate++;
        uLCD.locate(0,5);
        uLCD.color(GREEN);
        if(selectDate == 1){
            uLCD.printf("Temp One: %d F\n", sample_ftemp);
            uLCD.printf("Temp Two: %.2f C%\n", temp);
            uLCD.printf("IMU Temp: %.2f C\n", 25.0 + IMU.temperature / 16.0);
            uLCD.printf("Humidity: %d%%\n", sample_humid);
        }
        else if (selectDate ==2){////////////////
            uLCD.printf("Xaxis  Yaxis  Zaxis\n\r");
            uLCD.printf("gyro: %.2f %.2f %.2f\n", IMU.gx, IMU.gy, IMU.gz);
            uLCD.printf("accel: %.2f %.2f %.2f\n", IMU.ax, IMU.ay, IMU.az);
            uLCD.printf("mag: %.2f %.2f %.2f\n", IMU.mx, IMU.my, IMU.mz);
        }
        else if(selectDate ==3){
        printAttitude(IMU.calcAccel(IMU.ax), IMU.calcAccel(IMU.ay),
                    IMU.calcAccel(IMU.az), IMU.calcMag(IMU.mx),
                    IMU.calcMag(IMU.my), IMU.calcMag(IMU.mz));
        selectDate=0;
        }
    
}

void moonCycle() {
    setHeader(true, false);
    uLCD.filled_rectangle(0, 17, 75, 35, WHITE);
    uLCD.filled_rectangle(0, 16, 73, 33, BLACK);
    uLCD.locate(0,3);
    uLCD.printf("Moon Cycle\n");

    double moonstage = (double) d;
    moonstage = ceil(moonstage / 3);
    int out = (int) moonstage;
    switch (out) {
        case 1:  // fat right
            uLCD.filled_circle(60, 80, 30, WHITE);
            uLCD.filled_circle(61, 80, 28, BLACK);
            uLCD.filled_circle(73, 80, 28, WHITE);
            break;
        case 2:  // full
            uLCD.filled_circle(60, 80, 30, WHITE);
            break;
        case 3:  // full
            uLCD.filled_circle(60, 80, 30, WHITE);
            break;
        case 4:  // fat left
            uLCD.filled_circle(70, 80, 30, WHITE);
            uLCD.filled_circle(69, 80, 28, BLACK);
            uLCD.filled_circle(57, 80, 28, WHITE);
            break;
        case 5:  // third q left
            uLCD.filled_circle(60, 80, 30, WHITE);
            uLCD.filled_rectangle(60, 18, 200, 200, BLACK);
            break;
        case 6:  // tiny left
            uLCD.filled_circle(60, 80, 30, WHITE);
            uLCD.filled_circle(77, 80, 29, BLACK);
            break;
        case 7:  // new moon
            uLCD.filled_circle(60, 80, 30, WHITE);
            uLCD.filled_circle(60, 80, 28, BLACK);
            break;
        case 8:  // new moon
            uLCD.filled_circle(60, 80, 30, WHITE);
            uLCD.filled_circle(60, 80, 28, BLACK);
            break;
        case 9:  // tiny right
            uLCD.filled_circle(65, 80, 30, WHITE);
            uLCD.filled_circle(52, 80, 29, BLACK);
            break;
        case 10:  // first q right
            uLCD.filled_circle(60, 80, 30, WHITE);
            uLCD.filled_rectangle(0, 18, 60, 200, BLACK);
            break;
        default:
            uLCD.printf("Screen cannot currently print due to LCD fault error");
    }
}

void dayCycle() {
    uLCD.filled_rectangle(0, 100, 200, 200, 0x023020);
    setHeader(true, false);
    if (h > 7 && h < 18) {
        if (h == 7 || h == 17) {
            uLCD.filled_circle(65, 170, 40, 0xfdd835);
        }
        if (h == 8 || h == 16) {
            uLCD.filled_circle(65, 130, 40, 0xfdd835);
        }
        if (h == 9 || h == 15) {
            uLCD.filled_circle(65, 90, 40, 0xfdd835);
        }
        if (h == 10 || h == 14) {
            uLCD.filled_circle(65, 50, 40, 0xfdd835);
        }
        if (h == 11 || h == 13) {
            uLCD.filled_circle(65, 30, 40, 0xfdd835);
        }
        if (h == 12) {
            uLCD.filled_circle(65, 0, 40, 0xfdd835);
        }
        uLCD.filled_rectangle(0, 100, 200, 200, 0x90EE90);
    } 
    else {
        if (h == 19 || h == 5) {
            uLCD.filled_circle(65, 150, 40, 0xfdd835);
        }
        if (h == 20 || h == 4) {
            uLCD.filled_circle(65, 110, 40, 0xfdd835);
        }
        if (h == 21 || h == 3) {
            uLCD.filled_circle(65, 80, 40, 0xfdd835);
        }
        if (h == 22 || h == 2) {
            uLCD.filled_circle(65, 50, 40, 0xfdd835);
        }
        if (h == 23 || h == 1) {
            uLCD.filled_circle(65, 30, 40, 0xfdd835);
        }
        if (h == 24) {
            uLCD.filled_circle(65, 0, 40, 0xfdd835);
        }
        uLCD.filled_rectangle(0, 100, 200, 200, 0x023020);
    }
    setHeader(false, false);
    uLCD.filled_rectangle(0, 17, 70, 35, WHITE);
    uLCD.filled_rectangle(0, 16, 68, 33, BLACK);
    uLCD.locate(0,3);
    uLCD.printf("Day Cycle\n");
}

void realTimeBar() {
    myled = '5';
    setHeader(true,false);
    uLCD.filled_rectangle(0, 15, 40, 35, WHITE);
    uLCD.filled_rectangle(0, 17, 38, 33, BLACK);
    uLCD.locate(0,3);
    uLCD.printf("Gyro\n");

    uLCD.baudrate(16800);
    uLCD.color(WHITE);
    setHeader(true, false);
    uLCD.locate(0, 3);
    uLCD.text_width(2);
    uLCD.text_height(2);
    uLCD.textbackground_color(backColor);
    uLCD.printf("x   y   z");
        uLCD.filled_rectangle(0, 77, 200, 81, WHITE);
    while (1) {

        IMU.readMag();
        float inX = (IMU.mx / 50);
        float inY = (IMU.my / 50);
        float inZ = (IMU.mz / 50);
 
        if (inX > 0) {
            inX = inX/1.3;
            inX = 80-inX;
            uLCD.filled_rectangle(15, inX, 27, 80, GREEN);
            uLCD.filled_rectangle(15, 50, 27, inX, BLACK);
            uLCD.filled_rectangle(15, 81, 27, 200, BLACK);

        } else {
            inX = inX/2;
            inX = (80-inX);
            uLCD.filled_rectangle(15, 50, 27, 80, BLACK);
            uLCD.filled_rectangle(15, inX, 27, 200, BLACK);
            uLCD.filled_rectangle(15, 80, 27, inX, RED);
        }
        if (inY > 0) {
            inY = inY/2.3;
            inY = (80-inY);
            uLCD.filled_rectangle(58, inY, 70, 80, GREEN);
            uLCD.filled_rectangle(58, 55, 70, inY, BLACK);
            uLCD.filled_rectangle(58, 81, 70, 200, BLACK);

        } else {
            inY = (110 - inY);
            uLCD.filled_rectangle(58, 55, 70, 80, BLACK);
            uLCD.filled_rectangle(58, inY, 70, 200, BLACK);
            uLCD.filled_rectangle(58, 80, 70, inY, RED);
        }
        if (inZ > 0) {
            inZ = inZ/2.6;
            inZ = (60- inZ);
            uLCD.filled_rectangle(100, inZ, 112, 80, GREEN);
            uLCD.filled_rectangle(100, 50, 112, inZ, BLACK);
            uLCD.filled_rectangle(100, 81, 112, 200, BLACK);

        }
        else {
            inZ = (100- inZ);
            uLCD.filled_rectangle(100, 50, 112, 80, BLACK);
            uLCD.filled_rectangle(100, inZ, 112, 200, BLACK);
            uLCD.filled_rectangle(100, 80, 112, inZ, RED);
        }
        wait(.1);
    }
}

void tempChart() {
    myled = '4';

    uLCD.cls();
    setHeader(true, false);
    uLCD.baudrate(33600);
    uLCD.locate(0, 3);
    uLCD.text_width(2);
    uLCD.text_height(2);
    uLCD.textbackground_color(backColor);
    uLCD.printf("CurrTemp:");
    uLCD.text_width(1);
    uLCD.text_height(1);
    uLCD.locate(0, 9);
    uLCD.color(WHITE);
    uLCD.printf("100 85 70 55 40\n");
    uLCD.text_height(4);
    uLCD.printf(" |  |  |  |  |");

    uLCD.filled_circle(105, 105, 18, WHITE);
    uLCD.filled_rectangle(15, 95, 110, 115, WHITE);
    uLCD.filled_circle(15, 105, 10, WHITE);

    uLCD.filled_circle(15, 105, 5, BLACK);
    uLCD.filled_rectangle(15, 103, 110, 107, BLACK);

    uLCD.filled_circle(105, 105, 15, RED);
    // highest value is 15
    // 40-100 -> 15-110 pixel
    while(1){
        int temp = temphumid.sample_ftemp();
        uLCD.text_width(2);
        uLCD.text_height(2);
        uLCD.locate(0, 3);
        uLCD.printf("%d F", temp);

        int offset = temp;
        offset = offset * 1.175;
        offset = (int) 130 - offset ;
        uLCD.locate(0, 3);
        uLCD.locate(0,1);
          uLCD.filled_rectangle(20, 95, offset, 115, WHITE);
        uLCD.filled_rectangle(20, 103, offset, 107, BLACK);

        if (offset >= 100) {
            uLCD.filled_circle(15, 105, 8, RED);
        }
        uLCD.filled_rectangle(offset, 97, 110, 113, RED);
        wait(.05);
    }
}

void blackout(int line, int color1, int color2) {
    uLCD.color(color1);
    uLCD.locate(0, line);
    uLCD.printf("oooooooooooooooooo");
    uLCD.color(color2);
}

void dateIncrement() {
    switch (selectedValue) {
        case 0:
            blackout(11, RED, GREEN);
            uLCD.locate(0, 11);
            // uLCD.printf("(%d/%d/%d)\n", baseDates->years,
            // baseDates->months,baseDates->days);
            y++;
            uLCD.printf("Date: (%d/%d/%d)\n", d, mo, y);
            break;

        case 1:
            blackout(11, RED, GREEN);
            uLCD.locate(0, 11);
            if (mo < 12) {
                if (d >= 28) {
                    d = 28;
                }
                mo++;
            }
            uLCD.printf("Date: (%d/%d/%d)\n", d, mo, y);
            break;

        case 2:
            blackout(11, RED, GREEN);
            uLCD.locate(0, 11);
            if (d < 31) {
                if (mo == 4 || mo == 6 || mo == 9 || mo == 11) {
                    if (d < 30) {
                        d++;
                    }
                } else if (mo == 2) {
                    if (d < 28) {
                        d++;
                    }
                } else {
                    d++;
                }
            }
            uLCD.printf("Date: (%d/%d/%d)\n", d, mo, y);
            break;

        case 3:
            blackout(12, RED, GREEN);
            uLCD.locate(0, 12);
            if (h < 24) {
                h++;
            }
            uLCD.printf("Hour: %d\n", h);
            break;

        case 4:
            blackout(13, RED, GREEN);
            uLCD.locate(0, 13);
            if (m < 60) {
                m++;
            }
            uLCD.printf("Min: %d\n", m);
            break;

        default:
            uLCD.printf("Screen cannot currently print due to LCD fault error");
    }
}
void dateDeinc() {
    switch (selectedValue) {
        case 0:
            blackout(11, RED, GREEN);
            uLCD.locate(0, 11);
            if (y > 23) {
                y--;
            }
            uLCD.printf("Date: (%d/%d/%d)\n", d, mo, y);
            break;

        case 1:
            blackout(11, RED, GREEN);
            uLCD.locate(0, 11);
            if (mo > 1) {
                if (d >= 28) {
                    d = 28;
                }
                mo--;
            }
            uLCD.printf("Date: (%d/%d/%d)\n", d, mo, y);

            break;
        case 2:
            blackout(11, RED, GREEN);
            uLCD.locate(0, 11);
            if (d > 1) {
                d--;
            }
            uLCD.printf("Date: (%d/%d/%d)\n", d, mo, y);
            break;

        case 3:
            blackout(12, RED, GREEN);
            uLCD.locate(0, 12);
            if (h > 1) {
                h--;
            }
            uLCD.printf("Hour: %d\n", h);
            break;

        case 4:
            blackout(13, RED, GREEN);
            uLCD.locate(0, 13);
            if (m > 1) {
                m--;
            }
            uLCD.printf("Min: %d\n", m);
            break;

        default:
            uLCD.printf("Screen cannot currently print due to LCD fault error");
    }
}

void dateSet() {
    if (selectedValue < 6) {
        selectedValue++;
    }
    if (selectedValue == 3) {
        uLCD.locate(0, 12);
        uLCD.printf("Hour: 1");
        blackout(14, BLACK, GREEN);
        blackout(13, BLACK, GREEN);
    }
    if (selectedValue == 4) {
        uLCD.locate(0, 13);
        uLCD.printf("Mins: 0");
        blackout(14, BLACK, GREEN);
    }
    if (selectedValue == 5) {
        blackout(14, RED, GREEN);
        uLCD.locate(0, 14);
        uLCD.printf("Values Correct?");
    }
    if (selectedValue == 6) {
        secondsT.start();
        uLCD.color(WHITE);
        selectedValue++;
    }
}

void dateDeSet() {
    if (selectedValue > 0 && selectedValue < 6) {
        selectedValue--;
    }
}

void initDate() {
    setHeader(false, true);
    uLCD.locate(0, 3);
    uLCD.color(GREEN);
    uLCD.printf("Set date to \nuse application");
    y = 23;
    mo = 1;
    d = 1;
    h = 1;
    m = 0;

    uLCD.printf(
        "\n\nUp to incrament\nDown to decrement\nRight to enter\nLeft to "
        "cancel\n");
    uLCD.printf("\nDate: ");
}

void ScreenSelect() {
    char bnum = 0;
    char bhit = 0;
    bool realease = true;
    bool after = false;
    uLCD.baudrate(9600);

    while (blue.readable()) {
        if (blue.getc() == '!') {
            if (blue.getc() == 'B') {  // button data packet
                bnum = blue.getc();    // button number
                bhit = blue.getc();    // 1=hit, 0=release
                if (blue.getc() ==
                    char(~('!' + 'B' + bnum + bhit))) {  // checksum OK?
                    myled = bnum -
                            '0';  // current button number will appear on LEDs
                    switch (bnum) {
                        case '1':
                            // number button 1 for temp
                            if (bhit == '1') {
                                if (realease && selectedValue >= 6) {
                                    realease = false;
                                    myled = 15 - '0';
                                    clockLCD();
                                    wait(2);
                                }

                            } else {
                                realease = true;
                                myled = bnum - '0';
                            }
                            break;
                        case '2':  // number button 2
                            if (bhit == '1') {
                                if (realease && selectedValue >= 6) {
                                    realease = false;
                                    moonCycle();
                                    myled = 15 - '0';
                                    wait(2);
                                }
                            } else {
                                realease = true;
                                myled = bnum - '0';
                            }
                            break;
                        case '3':  // number button 3
                            if (bhit == '1') {
                                if (realease && selectedValue >= 6) {
                                    realease = false;
                                    myled = 15 - '0';
                                    dayCycle();
                                    wait(2);
                                }
                            } else {
                                realease = true;
                                myled = bnum - '0';
                            }
                            break;
                        case '4':  // number button 4
                            if (bhit == '1') {
                                if (realease && selectedValue >= 6) {
                                    realease = false;
                                    myled = 15 - '0';
                                    tempChart();
                                    wait(2);
                                }
                                // add hit code here
                            } else {
                                realease = true;

                                myled = bnum - '0';
                            }
                            break;
                        case '5':
                            // button 5 up arrow
                            if (bhit == '1') {
                                myled = 15 - '0';
                                if (selectedValue >= 6) {
                                    generalDataScreen();
                                } else {
                                    dateIncrement();
                                }

                                wait(.2);

                            } else {
                                myled = '0';
                            }
                            break;
                        case '6':  // button 6 down arrow
                            if (bhit == '1') {
                                myled = 15 - '0';
                                if (selectedValue >= 6) {
                                    realTimeBar();

                                } else {
                                    dateDeinc();
                                }

                                wait(.2);

                            } else {
                                myled = '0';
                            }
                            break;
                        case '7':  // button 7 left arrow
                            if (bhit == '1') {
                                myled = 15 - '0';
                                if (selectedValue >= 6) {
                                   generalDataScreen();

                                } else {
                                    dateDeSet();
                                }
                                wait(.2);

                            } else {
                                myled = '0';
                            }
                            break;
                        case '8':  // button 8 right arrow
                            if (bhit == '1') {
                                myled = 15 - '0';
                                if (selectedValue >= 6) {
                                    generalDataScreen();

                                } else {
                                    dateSet();
                                }
                                wait(.2);

                            } else {
                                myled = '0';
                            }
                            break;
                        default:
                            uLCD.printf("Screen cannot currently print due to LCD fault error");
                            break;
                    }
                }
            }
        }
    }
}

int main() {
    IMU.begin();
    IMU.calibrate();
    initDate();
    secondsT.start();
    blue.attach(&ScreenSelect, Serial::RxIrq);
}
