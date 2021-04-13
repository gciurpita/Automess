// monitor Automess Dose Rate Meter
// convert Automess output to floating pt value string

const char version [] = "ver: 210407a greg ciurpita";

#include <SoftwareSerial.h>

// connect Automess to pins 2 and GND
#define RX  2
#define TX  3

SoftwareSerial mySerial (RX, TX, true);

#define STX         02
#define MSG_SIZE    6
byte recBuf [MSG_SIZE] = { 02, 0x14, 0xd6, 0x6d, 0xfa, 0x55 };  // test data

int  idx = 0;

char s [80];

// -----------------------------------------------------------------------------
void setup() {
    Serial.begin   (9600);
    mySerial.begin (4800);

    Serial.println ("<Automess Ready>");
    Serial.println (version);
}

// -----------------------------------------------------------------------------
// check for input, return true (1) when complete and checksum correct
int
receive (void) {
    if (mySerial.available())  {
        byte  c = mySerial.read();

        if (0 == idx && STX != c)  {
            sprintf (s, "  %s: not STX %02x", __func__, c);
            Serial.println (s);
            return 0;
        }

        recBuf [idx++] = c;

        // process complete message
        if (MSG_SIZE == idx)  {
            idx = 0;

            // check checksum
            byte chkSum = 0;
            for (int i = 1; i < MSG_SIZE; i++)
                chkSum ^= recBuf [i];

            if (chkSum)  {
                sprintf (s, "  %s: chksum error %02x", __func__, chkSum);
                Serial.println (s);
            }
            else
                return 1;   // message with correct checksum
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------
// translate Automess output to floating pt value
#define LSB 2
#define MSB 3
#define EXP 4

float
decode (
    uint8_t *buf )
{
#define EXP_OFF     -15
    float   fVal = ((buf [MSB] << 8) | buf [LSB]);
    int8_t  exp  = buf [EXP] + EXP_OFF;

    if (exp < 0)
        fVal /=  (1L << -exp);
    else
        fVal *=  (1L << exp);

    return fVal;
}

// -----------------------------------------------------------------------------
// check for input and processes when complete and valid
void convert () {
    for (int i = 0; i < MSG_SIZE; i++) {
        sprintf (s, " %02x", recBuf [i]);
        Serial.print (s);
    }

    Serial.print ("  ");
    dtostrf (decode (recBuf), 6, 4, s);
    Serial.println (s);
}

// -----------------------------------------------------------------------------
// check for input and processes when complete and valid
void loop() {
    if (receive ())
        convert();
}
