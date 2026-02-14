#include "includes.h"

void DecToStr (uint8_t value, char * result)
{
	uint8_t power10;
	word compareValue;

	compareValue = 1;

	for (power10 = 0; compareValue <= value; power10 ++)
	{
		compareValue *= 10;
	}

	if (value == 0) {power10 = 1;}
	
	result[power10] = '\0';

	for (power10; power10 > 0; power10 --)
	{
		result[power10 - 1] = (value % 10) + '0';
		value /= 10;
	}
}

char HexToAscii (uint8_t Value)
{
	char character;

	if (Value < 10)
	{
		character = Value + 0x30;
	}
	else
	{
		character = Value - 9 + 0x40;
	}

	return character;
}
