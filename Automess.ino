// monitor Automess Dose Rate Meter
// convert Automess output to floating pt value string

const char version [] = "ver: 210412a greg ciurpita";

#include <SoftwareSerial.h>

// connect Automess to pins 2 and GND
#define RX  2
#define TX  3

SoftwareSerial mySerial (RX, TX, true);

#define STX         02
#define MSG_SIZE    6
byte recBuf [MSG_SIZE] = { 02, 0x14, 0xd6, 0x6d, 0xfa, 0x55 };  // test data

int  idx = 0;

char s [40];
char t [10];

void convert (uint8_t * recBuf);

// -----------------------------------------------------------------------------
void setup() {
    Serial.begin   (9600);
    mySerial.begin (4800);

    Serial.println ("<Automess Ready>");
    Serial.println (version);

#if 1
    byte testBuf [][MSG_SIZE] = {
        { 0x02, 0x14, 0x70, 0x9d, 0x00, 0x55 },
        { 0x02, 0x14, 0x37, 0xa1, 0xf6, 0x55 },
        { 0x02, 0x14, 0x16, 0xa5, 0xec, 0x55 },
    };

    convert (& testBuf [0][0]);
    convert (& testBuf [1][0]);
    convert (& testBuf [2][0]);
#endif
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
    float   fVal = (unsigned) ((buf [MSB] << 8) | buf [LSB]);
    int8_t  exp  = buf [EXP] + EXP_OFF;

    if (exp < 0)
        fVal /=  (1LL << -exp);
    else
        fVal *=  (1LL << exp);

    return fVal;
}

// -----------------------------------------------------------------------------
// check for input and processes when complete and valid
void convert (
    uint8_t * recBuf )
{
#if 1       // potentially for debug
    for (int i = 0; i < MSG_SIZE; i++) {
        sprintf (s, " %02x", recBuf [i]);
        Serial.print (s);
    }
    Serial.print ("  ");
#endif

    const char *units;
    float val = decode (recBuf);

    if (1.0 <= val)
        units = "Sv/hr";

    else if (0.001 <= val)  {
        units = "mSv/hr";
        val  *= 1000;
    }
    
    else  {
        units = "uSv/hr";
        val  *= 1000000;
    }

    dtostrf (val, 3, 2, t);
    sprintf (s, "%s %s", t, units);
    Serial.println (s);
}

// -----------------------------------------------------------------------------
// check for input and processes when complete and valid
void loop() {
    if (receive ())
        convert (recBuf);
}
