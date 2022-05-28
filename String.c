#ifndef STRING_C
#define STRING_C
#include <stdint.h>

// Written by Evan Beachly
// Public domain

// Utility function
int64_t countCharsOfNulTerminatedString(const char *nulTerminatedString)
{
    int64_t offset = 0;
    char c = nulTerminatedString[offset];
    while (c != '\0')
    {
        ++offset;
        c = nulTerminatedString[offset];
    }
    return offset;
}

struct String
{
    int64_t length;
    int64_t capacity;
    int attemptedToWriteMoreThanCapacity;
    char *bytes;
};

static inline struct String String(char *bytes, int64_t capacity)
{
    struct String ret;
    ret.length = 0;
    ret.attemptedToWriteMoreThanCapacity = 0;
    ret.capacity = capacity;
    ret.bytes = bytes;
    return ret;
}

#define S_STRING                                                     \
    char s_buffer[260];                                              \
    struct String s_string = String(s_buffer, sizeof(s_buffer) - 4); \
    struct String *s = &s_string;

// If you pass in a string with NULL bytes, the function won't write any characters, but it will still update the string's length as if it were. You do still need to pass in a capacity.
// This is useful to do a first pass to count how long of a buffer you need before actually allocating the memory

static inline void writeChar(char value, struct String *s)
{
    if (s->length < s->capacity)
    {
        if (s->bytes != NULL)
        {
            s->bytes[s->length] = value;
        }
        s->length++;
    }
    else
    {
        s->attemptedToWriteMoreThanCapacity = 1;
    }
    return;
}

void writeSpaces(int64_t numSpaces, struct String *s)
{
    if (s->bytes != NULL)
    {
        // If we should write the string
        int64_t i;
        for (i = 0; i < numSpaces && s->length < s->capacity; ++i)
        {
            s->bytes[s->length] = ' ';
            s->length++;
        }
        if (i < numSpaces)
        {
            s->attemptedToWriteMoreThanCapacity = 1;
        }
        return;
    }
    else
    {
        // If we should just count characters
        int64_t finalLength = s->length + numSpaces;
        if (finalLength <= s->capacity)
        {
            s->length = finalLength;
        }
        else
        {
            s->length = s->capacity;
            s->attemptedToWriteMoreThanCapacity = 1;
        }
    }
}

void writeZeros(int64_t numZeros, struct String *s)
{
    if (s->bytes != NULL)
    {
        // If we should write the string
        int64_t i;
        for (i = 0; i < numZeros && s->length < s->capacity; ++i)
        {
            s->bytes[s->length] = '0';
            s->length++;
        }
        if (i < numZeros)
        {
            s->attemptedToWriteMoreThanCapacity = 1;
        }
        return;
    }
    else
    {
        // If we should just count characters
        int64_t finalLength = s->length + numZeros;
        if (finalLength <= s->capacity)
        {
            s->length = finalLength;
        }
        else
        {
            s->length = s->capacity;
            s->attemptedToWriteMoreThanCapacity = 1;
        }
    }
}

void writeNulTerminatedString(const char *value, struct String *s)
{
    if (s->bytes != NULL)
    {
        // If we should write the string
        int64_t inputOffset = 0;
        char c = value[inputOffset];
        while (c != 0)
        {
            if (s->length < s->capacity)
            {
                s->bytes[s->length] = c;
                ++inputOffset;
                ++s->length;
                c = value[inputOffset];
            }
            else
            {
                s->attemptedToWriteMoreThanCapacity = 1;
                return;
            }
        }
        return;
    }
    else
    {
        // If we should just count characters
        int64_t finalLength = s->length + countCharsOfNulTerminatedString(value);
        if (finalLength <= s->capacity)
        {
            s->length = finalLength;
        }
        else
        {
            s->length = s->capacity;
            s->attemptedToWriteMoreThanCapacity = 1;
        }
    }
}

void writeStringNoNullTerminator(const char *value, struct String *s)
{
    if (s->bytes != NULL)
    {
        // If we should write the string
        int64_t inputOffset = 0;
        char c = value[inputOffset];
        while (c != 0 && c != '\0')
        {
            if (s->length < s->capacity)
            {
                s->bytes[s->length] = c;
                ++inputOffset;
                ++s->length;
                c = value[inputOffset];
            }
            else
            {
                s->attemptedToWriteMoreThanCapacity = 1;
                return;
            }
        }
        return;
    }
    else
    {
        // If we should just count characters
        int64_t finalLength = s->length + countCharsOfNulTerminatedString(value);
        if (finalLength <= s->capacity)
        {
            s->length = finalLength;
        }
        else
        {
            s->length = s->capacity;
            s->attemptedToWriteMoreThanCapacity = 1;
        }
    }
}

void writeNulTerminatedStringRightJustify(const char *value, int64_t endLength, struct String *s)
{
    // If we should write the string
    int64_t valueLength = countCharsOfNulTerminatedString(value);
    int64_t minimumLength = endLength - s->length;
    int64_t numSpaces;

    if (valueLength < minimumLength)
    {
        numSpaces = minimumLength - valueLength;
    }
    else
    {
        numSpaces = 0;
    }

    if (s->bytes != NULL)
    {
        // If we should write the string
        writeSpaces(numSpaces, s);
        writeNulTerminatedString(value, s);
    }
    else
    {
        // If we should just count characters
        int64_t finalLength = s->length + numSpaces + valueLength;
        if (finalLength <= s->capacity)
        {
            s->length = finalLength;
        }
        else
        {
            s->length = s->capacity;
            s->attemptedToWriteMoreThanCapacity = 1;
        }
    }
}

void writeNumber(int64_t value, struct String *s)
{
    // Write negative sign
    if (value < 0)
    {
        writeChar('-', s);
        if (value == 0x8000000000000000ll)
        {
            value = 0x7FFFFFFFFFFFFFFFll;
        }
        else
        {
            value = -value;
        }
    }

    int64_t ten = 10;

    // Go down all of the possible digits
    int64_t magnitude = 1000000000000000000ll; // Max value of int64_t is about 9 quintillion
    {
        int foundHighestMagnitude = 0;
        for (; magnitude >= ten; magnitude = magnitude / 10)
        {
            // See if the value was greater than this magnitude
            foundHighestMagnitude = foundHighestMagnitude || value >= magnitude;
            if (foundHighestMagnitude)
            {
                int64_t digit = value / magnitude;
                writeChar((char)('0' + digit), s);
                value = value - digit * magnitude;
            }
        }
    }

    // Always write the one's place
    {
        int64_t digit = value / magnitude;
        writeChar((char)('0' + digit), s);
        value = value - digit * magnitude;
        magnitude = magnitude / 10;
    }

    return;
}

void writeNumberRightJustify(int64_t value, int64_t endLength, struct String *s)
{
    int64_t initialLength = s->length;
    // Try writing it out once without spaces to see how long the number will be
    writeNumber(value, s);

    int64_t numSpaces = endLength - s->length;
    if (numSpaces > 0)
    {
        // If we need spaces, reset the string and try again
        s->length = initialLength;
        writeSpaces(numSpaces, s);
        writeNumber(value, s);
    }
}

void writeNumberZeroPadding(int64_t value, int64_t endLength, struct String *s)
{
    int64_t initialLength = s->length;
    // Try writing it out once without spaces to see how long the number will be
    writeNumber(value, s);

    int64_t numZeros = endLength - s->length;
    if (numZeros > 0)
    {
        // If we need spaces, reset the string and try again
        s->length = initialLength;
        writeZeros(numZeros, s);
        writeNumber(value, s);
    }
}

void writeDecimalNumber(int64_t value, int64_t numDecimalPlacesToWrite, struct String *s)
{
    // Write negative sign
    if (value < 0)
    {
        writeChar('-', s);
        if (value == 0x8000000000000000ll)
        {
            value = 0x7FFFFFFFFFFFFFFFll;
        }
        else
        {
            value = -value;
        }
    }

    int64_t ten = 10;
    for (int d = 0; d < numDecimalPlacesToWrite; ++d)
    {
        ten = ten * 10;
    }

    // Go down all of the possible digits
    int64_t magnitude = 1000000000000000000ll; // Max value of int64_t is about 9 quintillion
    {
        int foundHighestMagnitude = 0;
        for (; magnitude >= ten; magnitude = magnitude / 10)
        {
            // See if the value was greater than this magnitude
            foundHighestMagnitude = foundHighestMagnitude || value >= magnitude;
            if (foundHighestMagnitude)
            {
                int64_t digit = value / magnitude;
                writeChar((char)('0' + digit), s);
                value = value - digit * magnitude;
            }
        }
    }

    // Always write the one's place
    {
        int64_t digit = value / magnitude;
        writeChar((char)('0' + digit), s);
        value = value - digit * magnitude;
        magnitude = magnitude / 10;
    }

    // Write the decimal point, if necessary
    if (numDecimalPlacesToWrite > 0)
    {
        writeChar('.', s);

        // Write the digits after the decimal
        for (; magnitude >= 1; magnitude = magnitude / 10)
        {
            int64_t digit = value / magnitude;
            writeChar((char)('0' + digit), s);
            value = value - digit * magnitude;
        }
    }

    return;
}

void writeDecimalNumberRightJustify(int64_t value, int64_t numDecimalPlacesToWrite, int64_t endLength, struct String *s)
{
    int64_t initialLength = s->length;
    // Try writing it out once without spaces to see how long the number will be
    writeDecimalNumber(value, numDecimalPlacesToWrite, s);

    int64_t numSpaces = endLength - s->length;
    if (numSpaces > 0)
    {
        // If we need spaces, reset the string and try again
        s->length = initialLength;
        writeSpaces(numSpaces, s);
        writeDecimalNumber(value, numDecimalPlacesToWrite, s);
    }
    else
    {
        writeDecimalNumber(value, numDecimalPlacesToWrite, s);
    }
}

int64_t readNumber(const char *charArray, int64_t *offset, int64_t arrayLength, int64_t valueToReturnOnError)
{
    // Read past any spaces
    char c = 0; // Initialize to zero so it isn't '-' by chance.
    while (*offset < arrayLength)
    {
        c = charArray[*offset];
        if (c != ' ')
        {
            break;
        }
        ++*offset;
    }

    // Read in the - sign
    int64_t sign = 1;
    if (c == '-')
    {
        sign = -1;
        ++*offset;
        if (*offset < arrayLength)
        {
            c = charArray[*offset];
        }
        else
        {
            return valueToReturnOnError;
        }
    }

    int64_t number = 0;
    // If the character after the whitespace/- is a digit
    if (c >= '0' && c <= '9')
    {
        // Read in the digits
        while (c >= '0' && c <= '9')
        {
            number = 10 * number + (c - '0');
            ++*offset;
            if (*offset < arrayLength)
            {
                c = charArray[*offset];
            }
            else
            {
                return sign * number;
            }
        }
        return sign * number;
    }
    else
    {
        // Not a digit! error
        return valueToReturnOnError;
    }
}

int64_t readDecimalNumber(const char *charArray, int64_t *offset, int64_t arrayLength, int64_t numDecimalPlacesToReadIn, int64_t valueToReturnOnError)
{

    // Read past any spaces
    char c = 0; // Initialize to zero so it isn't '-' by chance.
    while (*offset < arrayLength)
    {
        c = charArray[*offset];
        if (c != ' ')
        {
            break;
        }
        ++*offset;
    }

    // Read in the - sign
    int64_t sign = 1;
    if (c == '-')
    {
        sign = -1;
        ++*offset;
        if (*offset < arrayLength)
        {
            c = charArray[*offset];
        }
        else
        {
            // Haven't seen any digits yet, but end of input
            return valueToReturnOnError;
        }
    }

    int64_t number = 0;
    if (c < '0' || c > '9')
    {
        // Error. Numbers must begin with a digit
        return valueToReturnOnError;
    }
    // Read in the digits
    while (c >= '0' && c <= '9')
    {
        number = 10 * number + (c - '0');
        ++*offset;
        if (*offset < arrayLength)
        {
            c = charArray[*offset];
        }
        else
        {
            // End of file
            // Add desired precision
            for (int i = 0; i < numDecimalPlacesToReadIn; ++i)
            {
                number = number * 10;
            }
            return sign * number;
        }
    }

    // See if there's a decimal point
    if (c != '.')
    {
        // Return the result
        // Add desired precision
        for (int64_t i = 0; i < numDecimalPlacesToReadIn; ++i)
        {
            number = number * 10;
        }
        return sign * number;
    }

    // Get the next character
    ++*offset;
    if (*offset < arrayLength)
    {
        c = charArray[*offset];
    }
    else
    {
        // End of input. return result
        // Add desired precision
        for (int64_t i = 0; i < numDecimalPlacesToReadIn; ++i)
        {
            number = number * 10;
        }
        return sign * number;
    }

    int64_t roundedNumber = number; // If there's more decimal places than we need, we'll use the next digit to round our answer and save it here. However, we'll keep using "number" to read in digits in case there's an exponent and we can use the precision.
    // Read the number beyond the decimal place
    while (c >= '0' && c <= '9')
    {
        if (numDecimalPlacesToReadIn > 0)
        {
            number = 10 * number + (c - '0');
            roundedNumber = number;
            --numDecimalPlacesToReadIn;
        }
        else if (numDecimalPlacesToReadIn == 0)
        {
            // If there are more digits, use them to round
            if (c >= '5')
            {
                roundedNumber = number + 1;
            }
            number = 10 * number + (c - '0');
            --numDecimalPlacesToReadIn;
        }
        else
        {
            number = 10 * number + (c - '0');
            --numDecimalPlacesToReadIn;
        }
        ++*offset;
        if (*offset < arrayLength)
        {
            c = charArray[*offset];
        }
        else
        {
            // End of input
            // Add desired precision
            for (int64_t i = 0; i < numDecimalPlacesToReadIn; ++i)
            {
                roundedNumber = roundedNumber * 10;
            }
            return sign * roundedNumber;
        }
    }

    // Check if there's an exponent after this
    if (c == 'e' || c == 'E')
    {
        // Go to the next character
        ++*offset;

        // Read in the exponent
        int64_t exponent = readNumber(charArray, offset, arrayLength, 1);

        // Figure out how many zeros we need to append to get the correct precision
        numDecimalPlacesToReadIn = numDecimalPlacesToReadIn + exponent; // Read in "exponent"-many more zeros

        // If we need more zeros
        if (numDecimalPlacesToReadIn >= 0)
        {
            for (int i = 0; i < numDecimalPlacesToReadIn; ++i)
            {
                number = number * 10;
            }
            return sign * number;
        }
        else
        {
            // We need to scale it down
            for (int64_t i = -1; i > numDecimalPlacesToReadIn; --i)
            { // Remove all the unnecessary decimal places except one
                number = number / 10;
            }
            number = (number + 5) / 10; // Use the last decimal place to round
            return sign * number;
        }
    }
    else
    {
        // No exponent
        // Return the result
        // Add desired precision
        for (int64_t i = 0; i < numDecimalPlacesToReadIn; ++i)
        {
            roundedNumber = roundedNumber * 10;
        }
        return sign * roundedNumber;
    }
}

double readDouble(const char *charArray, int64_t *offset, int64_t arrayLength, double valueToReturnOnError)
{
    // Read past any spaces
    char c = 0; // Initialize to zero so it isn't '-' by chance.
    while (*offset < arrayLength)
    {
        c = charArray[*offset];
        if (c != ' ')
        {
            break;
        }
        ++*offset;
    }

    // Read in the - sign
    int sign = 1;
    if (c == '-')
    {
        sign = -1;
        ++*offset;
        if (*offset < arrayLength)
        {
            c = charArray[*offset];
        }
        else
        {
            // Out of input, but didn't see a digit
            return valueToReturnOnError;
        }
    }

    double number = 0;
    if (c < '0' || c > '9')
    {
        // First character of a number must be a digit
        return valueToReturnOnError;
    }
    // Read in the digits
    while (c >= '0' && c <= '9')
    {
        number = 10 * number + (c - '0');
        ++*offset;
        if (*offset < arrayLength)
        {
            c = charArray[*offset];
        }
        else
        {
            // End of file
            return sign * number;
        }
    }

    double divisor = 1;

    // See if there's a decimal point
    if (c == '.')
    {
        // Get the next character
        ++*offset;
        if (*offset < arrayLength)
        {
            c = charArray[*offset];
        }
        else
        {
            // End of file. return result
            return sign * (number / divisor);
        }

        // Read the number beyond the decimal place
        while (c >= '0' && c <= '9')
        {
            number = 10 * number + (c - '0');
            divisor = 10 * divisor;
            ++*offset;
            if (*offset < arrayLength)
            {
                c = charArray[*offset];
            }
            else
            {
                // End of file
                return sign * (number / divisor);
            }
        }
    }

    // See if there's an exponent
    if (c == 'e')
    {
        // Get the next character
        ++*offset;
        if (*offset < arrayLength)
        {
            c = charArray[*offset];
        }
        else
        {
            // End of file. return result
            return sign * (number / divisor);
        }

        // Read in the exponent
        int exponentSign = 1;
        int exponent = 0;
        if (c == '-')
        {
            exponentSign = -1;
            // Get the next character
            ++*offset;
            if (*offset < arrayLength)
            {
                c = charArray[*offset];
            }
            else
            {
                // End of file. return result
                return sign * (number / divisor);
            }

            // Read in the exponent
            while (c >= '0' && c <= '9')
            {
                exponent = 10 * exponent + (c - '0');
                ++*offset;
                if (*offset < arrayLength)
                {
                    c = charArray[*offset];
                }
                else
                {
                    // End of file
                    break;
                }
            }
        }

        if (exponentSign == 1)
        {
            while (exponent > 0)
            {
                divisor = divisor / 10;
                exponent--;
            }
        }
        else
        {
            while (exponent > 0)
            {
                divisor = divisor * 10;
                exponent--;
            }
        }
    }

    // Return the result
    return sign * (number / divisor);
}

float readFloat(const char *charArray, int64_t *offset, int64_t arrayLength, float valueToReturnOnError)
{
    return (float)readDouble(charArray, offset, arrayLength, valueToReturnOnError);
}

#endif