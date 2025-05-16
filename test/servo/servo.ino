#include <ESP32Servo.h>

#define SERVO_PIN 10 // Pino do servo motor

const int MAXPOS = 180, MINPOS = 0;

Servo ParachuteServo; // Objeto do servo

void setupServo()
{
    ParachuteServo.attach(SERVO_PIN);
    ParachuteServo.write(MINPOS);
}

void setup()
{
    Serial.begin(115200);
    setupServo();
}

void loop()
{
    for (int i = MINPOS; i <= MAXPOS; i++)
    {
        ParachuteServo.write(i);
        Serial.println(i);
        delay(15);
    }
    delay(1000);
    for (int i = MAXPOS; i >= MINPOS; i--)
    {
        ParachuteServo.write(i);
        Serial.println(i);
        delay(15);
    }
    delay(1000);
}
