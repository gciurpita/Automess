### Description ###

An Arduino .ino file news two functions: <i>setup()</i> and <i>loop()</i>.
Setup() is invoked once when the program starts and
loop() is invoked repeatedly.

**Setup()** configure both a hardware and software serial port
for 9600 and 4800 bit-per-second (bps).
It prints the name of the program and a version string.

**Receive()** checks for serial data from the Automess
on the software serial interface, <i>mySerial</i>.
It reads the receive value into "c".
It will copy "c" into the receive buff, <i>recBuf[]</i>
indexed by <i>idx</i>.
But before doing this, when the <i>idx == 0</i>,
it verifies that the first value received is the STX character,
<a href=https://en.wikipedia.org/wiki/ASCII#/media/File:USASCII_code_chart.png>
ASCII</a> value 2
indicating the start of a measurement report.

When the final value is received, when <i>idx == MSG_SIZE</i>,
is calculated.
An error is reported if incorrect,
otherwise a value 1 is returned indicating a
complete and verified sequence of values is received.

**Decode()** converts the received data into a floating point value.
It first combines the two mantissa bytes and then
scales that value by the value of the exponent minus 15.

The mantissa holds the non-zero bits of the measurement.
The exponent indicates scaling (power of 2) of the mantissa bits.
Shifting a 1 to the left by the exponent results in that
<a href=https://en.wikipedia.org/wiki/Power_of_two>
power of 2</a>.
A negative exponent indicates that the mantissa is a value less than one.

**Convert()** displays the received data bytes,
calls <i>decode()</i> to translate the received bytes into
a floating point value and
displays it.

**Loop()** simply calls <i>receive()</i> and
when it returns a one indicating a complete message
