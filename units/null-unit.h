// these are exposed from a unit

// called when the unit is loaded, returns the number of params it accepts
unsigned int init();

// process a single value, in a 0-255 position frame, return output
float process(unsigned char position, float input, unsigned char channel);

// called when you plugin is unloaded
void destroy();

// set a parameter
void param_set(unsigned int param, unsigned int value);

// get the current value of a parameter
unsigned int param_get(unsigned int param);

// returns the name of the unit (32 characters, max)
char* get_name_unit();

// returns the name of the parameter (32 characters, max)
char* get_name_param(unsigned int param);

// returns number of channels
unsigned char get_channel_count();


// these are exposed from host

// get current unix time in ms (wraps around at 4.294967296e^9)
unsigned int now();

// get a pseudo-random number -3.4e^38 to 3.4e^38
float random();

// get some named bytes (sample, etc) from host
unsigned int get_bytes(char* name, unsigned int offset, unsigned int length);

// Reduced-precision math functions, exposed from host

// Returns the arc cosine of x in radians.
float acos(float x);

// Returns the arc sine of x in radians.
float asin(float x);

// Returns the arc tangent of x in radians.
float atan(float x);

// Returns the arc tangent in radians of y/x based on the signs of both values to determine the correct quadrant.
float atan2(float y, float x);

// Returns the cosine of a radian angle x.
float cos(float x);

// Returns the hyperbolic cosine of x.
float cosh(float x);

// Returns the sine of a radian angle x.
float sin(float x);

// Returns the hyperbolic sine of x.
float sinh(float x);

// Returns the tangent of a given angle(x).
float tan(float x);

// Returns the hyperbolic tangent of x.
float tanh(float x);

// Returns the value of e raised to the xth power.
float exp(float x);

// The returned value is the mantissa and the integer pointed to by exponent is the exponent. The resultant value is x = mantissa * 2 ^ exponent.
float frexp(float x, int *exponent);

// Returns x multiplied by 2 raised to the power of exponent.
float ldexp(float x, int exponent);

// Returns the natural logarithm (base-e logarithm) of x.
float log(float x);

// Returns the common logarithm (base-10 logarithm) of x.
float log10(float x);

// The returned value is the fraction component (part after the decimal), and sets integer to the integer component.
float modf(float x, float *integer);

// Returns x raised to the power of y.
float pow(float x, float y);

// Returns the square root of x.
float sqrt(float x);

// Returns the smallest integer value greater than or equal to x.
float ceil(float x);

// Returns the absolute value of x.
float fabs(float x);

// Returns the largest integer value less than or equal to x.
float floor(float x);

// Returns the remainder of x divided by y.
float fmod(float x, float y);

// Returns the nearest integer value of x(rounded off values).
float round(float x);

