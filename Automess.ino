// monitor Automess Dose Rate Meter
// convert Automess output to floating pt value string

const char version [] = "ver: 210424a greg ciurpita";   // version string

// use Software Serial library to communicate using pins 2,3
#include <SoftwareSerial.h>
#define RX  2
#define TX  3
SoftwareSerial mySerial (RX, TX, true);

// define a msg buffer, its size the start text value
#define MSG_SIZE    6
#define STX         02
byte recBuf [MSG_SIZE] = { 02, 0x14, 0xd6, 0x6d, 0xfa, 0x55 };  // test data

// index of the next available byte in recBuf
int  idx = 0;

// two ascii character strings (arrays) used by sprintf()
char s [40];
char t [10];

// forward declartion of convert() needed on laptop
void convert (uint8_t * recBuf);

// -----------------------------------------------------------------------------
// setup performed once on Arduino
void setup() {
    // initialize serial interfaces and their bit-rates
    Serial.begin   (9600);
    mySerial.begin (4800);

    // start-up messages displayed on serial monitor
    Serial.println ("<Automess Ready>");
    Serial.println (version);

    // optional code to test some possible Automess values
#if 1
    // values picked in Sv/hr, mSv/hr and uSv/hr ranges
    byte testBuf [][MSG_SIZE] = {
        { 0x02, 0x14, 0x70, 0x9d, 0x00, 0x55 },
        { 0x02, 0x14, 0x37, 0xa1, 0xf6, 0x55 },
        { 0x02, 0x14, 0x16, 0xa5, 0xec, 0x55 },
    };

    // pass pointer (&) to first byte of each test sequence
    convert (& testBuf [0][0]);
    convert (& testBuf [1][0]);
    convert (& testBuf [2][0]);
#endif
}

// -----------------------------------------------------------------------------
// check for input, return true (1) when complete and checksum correct
int
receive (void) {
    // check if a byte available on Software serial interface
    if (mySerial.available())  {
        // read byte in to varibale c
        byte  c = mySerial.read();

        // discard if waiting for first byte (idx == 0) and not STX
        if (0 == idx && STX != c)  {
            sprintf (s, "  %s: not STX %02x", __func__, c);
            Serial.println (s);
            return 0;       // indicate incomplete msg
        }

        // capture byte, incrementing index
        recBuf [idx++] = c;

        // check if a complete msg received and process it
        if (MSG_SIZE == idx)  {
            // reset index for next msg
            idx = 0;

            // calculate checksum - exclusive OR of bits in each byte
            byte chkSum = 0;
            for (int i = 1; i < MSG_SIZE; i++)
                chkSum ^= recBuf [i];

            // final checksum byte should make total checksum zero
            if (chkSum)  {
                sprintf (s, "  %s: chksum error %02x", __func__, chkSum);
                Serial.println (s);
            }
            else
                return 1;   // indicate complete and valid msg received
        }
    }
    return 0;       // indicate incomplete msg
}

// -----------------------------------------------------------------------------
// translate Automess output to floating pt value

//    indices of exponent and least/most significant mantissa bytes
#define LSB 2
#define MSB 3
#define EXP 4

float
decode (
    uint8_t *buf )
{
#define EXP_OFF     -15
    // shift MSB left 8 bits (*256) and OR with LSB creating 16-bit value
    float   fVal = (unsigned) ((buf [MSB] << 8) | buf [LSB]);

    // add EXP_OFF (-15) to value of power of 2 exponent
    int8_t  exp  = buf [EXP] + EXP_OFF;

    // calculate magnitude of exponent and divide/multiply mantissa
    //     shift 1 (long long) by value of exponent
    if (exp < 0)
        fVal /=  (1LL << -exp); // divide mantissa is exponent negative
    else
        fVal *=  (1LL << exp);  // multiply mantissa is exponent negative

    return fVal;
}

// -----------------------------------------------------------------------------
// check for input and processes when complete and valid
void convert (
    uint8_t * recBuf )
{
    // optionally display Automess bytes
#if 1       // potentially for debug
    for (int i = 0; i < MSG_SIZE; i++) {
        // format a substring for each byte as 2 digit hex string
        sprintf (s, " %02x", recBuf [i]);
        Serial.print (s);
    }
    // put a couple spaces after hex bytes
    Serial.print ("  ");
#endif

    const char *units;              // ptr to string for units
    float val = decode (recBuf);    // get value

    // sequencially check the magnitude of the values
    //     to determine the scale and re-scale the value
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

    // arduino doesn't support %f in sprintf()
    dtostrf (val, 3, 2, t);             // format value as a string
    sprintf (s, "%s %s", t, units);     // foramt value and units
    Serial.println (s);                 // output
}

// -----------------------------------------------------------------------------
// check for input and processes when complete and valid
void loop() {
    if (receive ())
        convert (recBuf);
}
