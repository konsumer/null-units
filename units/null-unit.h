// use this header in all C null-units

#include "walloc.c"

// these are exposed from a unit

// called when the unit is loaded, returns the number of params it accepts
__attribute__((export_name("init")))
void init(unsigned int* initialParams);

// process a single value, in a 0-255 position frame, return output
__attribute__((export_name("process")))
float process(unsigned char position, float input, unsigned char channel);

// called when you plugin is unloaded
__attribute__((export_name("destroy")))
void destroy();

// get param count
__attribute__((export_name("get_param_count")))
unsigned int get_param_count();

// set a parameter
__attribute__((export_name("param_set")))
void param_set(unsigned int param, unsigned int value);

// get the current value of a parameter
__attribute__((export_name("param_get")))
unsigned int param_get(unsigned int param);

// returns the name of the unit (32 characters, max)
__attribute__((export_name("get_name_unit")))
char* get_name_unit();

// returns the name of the parameter (32 characters, max)
__attribute__((export_name("get_param_name")))
char* get_name_param(unsigned int param);

// returns number of channels
__attribute__((export_name("get_channel_count")))
unsigned char get_channel_count();


// these are exposed from host
__attribute__((import_module("env"), import_name("trace")))
void trace(char* message);

// get current unix time in ms (wraps around at 4.294967296e^9)
__attribute__((import_module("env"), import_name("now")))
unsigned int now();

// get a pseudo-random number -3.4e^38 to 3.4e^38
__attribute__((import_module("env"), import_name("random")))
float random();

// get some named bytes (sample, etc) from host
__attribute__((import_module("env"), import_name("get_bytes")))
void get_bytes(unsigned int id, unsigned int offset, unsigned int length, unsigned int* out);

// Reduced-precision math functions, exposed from host

// Returns the arc cosine of x in radians.
__attribute__((import_module("env"), import_name("acos")))
double acos(double x);

// Returns the arc sine of x in radians.
__attribute__((import_module("env"), import_name("asin")))
double asin(double x);

// Returns the arc tangent of x in radians.
__attribute__((import_module("env"), import_name("atan")))
double atan(double x);

// Returns the arc tangent in radians of y/x based on the signs of both values to determine the correct quadrant.
__attribute__((import_module("env"), import_name("atan2")))
double atan2(double y, double x);

// Returns the cosine of a radian angle x.
__attribute__((import_module("env"), import_name("cos")))
double cos(double x);

// Returns the hyperbolic cosine of x.
__attribute__((import_module("env"), import_name("cosh")))
double cosh(double x);

// Returns the sine of a radian angle x.
__attribute__((import_module("env"), import_name("sin")))
double sin(double x);

// Returns the hyperbolic sine of x.
__attribute__((import_module("env"), import_name("sinh")))
double sinh(double x);

// Returns the tangent of a given angle(x).
__attribute__((import_module("env"), import_name("tan")))
double tan(double x);

// Returns the hyperbolic tangent of x.
__attribute__((import_module("env"), import_name("tanh")))
double tanh(double x);

// Returns the value of e raised to the xth power.
__attribute__((import_module("env"), import_name("exp")))
double exp(double x);

// The returned value is the mantissa and the integer pointed to by exponent is the exponent. The resultant value is x = mantissa * 2 ^ exponent.
__attribute__((import_module("env"), import_name("frexp")))
double frexp(double x, int *exponent);

// Returns x multiplied by 2 raised to the power of exponent.
__attribute__((import_module("env"), import_name("ldexp")))
double ldexp(double x, int exponent);

// Returns the natural logarithm (base-e logarithm) of x.
__attribute__((import_module("env"), import_name("log")))
double log(double x);

// Returns the common logarithm (base-10 logarithm) of x.
__attribute__((import_module("env"), import_name("log10")))
double log10(double x);

// The returned value is the fraction component (part after the decimal), and sets integer to the integer component.
__attribute__((import_module("env"), import_name("modf")))
double modf(double x, double *integer);

// Returns x raised to the power of y.
__attribute__((import_module("env"), import_name("pow")))
double pow(double x, double y);

// Returns the square root of x.
__attribute__((import_module("env"), import_name("sqrt")))
double sqrt(double x);

// Returns the smallest integer value greater than or equal to x.
__attribute__((import_module("env"), import_name("ceil")))
double ceil(double x);

// Returns the absolute value of x.
__attribute__((import_module("env"), import_name("fabs")))
double fabs(double x);

// Returns the largest integer value less than or equal to x.
__attribute__((import_module("env"), import_name("floor")))
double floor(double x);

// Returns the remainder of x divided by y.
__attribute__((import_module("env"), import_name("fmod")))
double fmod(double x, double y);

// Returns the nearest integer value of x(rounded off values).
__attribute__((import_module("env"), import_name("round")))
double round(double x);

// simple utils

void itoa(int num, char* str) {
    char temp[20];
    int i = 0, j = 0;

    // Handle negative numbers
    if (num < 0) {
        str[j++] = '-';
        num = -num;
    }

    // Handle 0 explicitly
    if (num == 0) {
        str[j++] = '0';
        str[j] = '\0';
        return;
    }

    // Store digits in reverse order
    while (num > 0) {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }

    // Copy in correct order
    while (i > 0)
        str[j++] = temp[--i];

    str[j] = '\0';
}

void ftoa(float num, char* str, int precision) {
    // Extract integer part
    int whole = (int)num;

    // Extract decimal part
    float decimal = num - whole;
    if (decimal < 0) {
        decimal = -decimal;
    }

    // Handle negative numbers
    int i = 0;
    if (num < 0 && whole == 0) {
        str[i++] = '-';
    }

    // Convert integer part to string
    if (whole < 0) {
        str[i++] = '-';
        whole = -whole;
    }

    if (whole == 0) {
        str[i++] = '0';
    } else {
        char temp[20];
        int temp_i = 0;

        while (whole > 0) {
            temp[temp_i++] = (whole % 10) + '0';
            whole /= 10;
        }

        while (temp_i > 0) {
            str[i++] = temp[--temp_i];
        }
    }

    // Check if precision is needed
    if (precision > 0) {
        str[i++] = '.';

        // Implement rounding
        float rounding = 0.5;
        for (int j = 0; j < precision; j++) {
            rounding /= 10.0;
        }
        decimal += rounding;

        // Convert decimal part
        while (precision > 0) {
            decimal *= 10;
            int digit = (int)decimal;
            if (digit > 9) {
                digit = 9;
            }
            str[i++] = digit + '0';
            decimal -= digit;
            precision--;
        }
    }

    str[i] = '\0';
}

void join_strings(char *dest, const char *src) {
    int i, j;

    // Find end of dest string
    for(i = 0; dest[i] != '\0'; i++);

    // Copy src to end of dest
    for(j = 0; src[j] != '\0'; j++) {
        dest[i + j] = src[j];
    }
    dest[i + j] = '\0';  // Add null terminator
}


void* memset(void* ptr, int value, size_t num) {
  unsigned char* byte_ptr = (unsigned char*)ptr;
  for(size_t i = 0; i < num; i++) {
    byte_ptr[i] = (unsigned char)value;
  }
  return ptr;
}
