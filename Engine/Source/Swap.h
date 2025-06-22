#pragma once

template<typename T>
void Swap32(T& dst)
{
    OCT_ASSERT(sizeof(T) == 4);

    uint8_t* charArray = reinterpret_cast<uint8_t*>(&dst);
    uint8_t c0 = charArray[0];
    uint8_t c1 = charArray[1];
    uint8_t c2 = charArray[2];
    uint8_t c3 = charArray[3];

    charArray[0] = c3;
    charArray[1] = c2;
    charArray[2] = c1;
    charArray[3] = c0;
}

template<typename T>
void Swap16(T& dst)
{
    OCT_ASSERT(sizeof(T) == 2);

    uint8_t* charArray = reinterpret_cast<uint8_t*>(&dst);
    uint8_t c0 = charArray[0];
    uint8_t c1 = charArray[1];

    charArray[0] = c1;
    charArray[1] = c0;
}
