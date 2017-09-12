#include "Utility.h"

ByteArray toByteArray(const BigInteger& value)
{
    return toByteArray(value.raw_data());
}

int fromByteArray(const Byte* data, BigInteger& value)
{
    std::vector<BigInteger::unit_t> rawData;
    auto bytesRead = fromByteArray(data, rawData);
    value.set_raw_data(rawData);
    return bytesRead;
}
