#include <morton.h>

Morton::Morton()
{
    m_Morton=0;
}

Morton::Morton(uint64_t _x)
{
    m_Morton=_x;
}

Morton::~Morton(){}

// Code based on: http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
// And: http://graphics.stanford.edu/~seander/bithacks.html#InterleaveBMN
uint64_t Morton::pushByThree(unsigned int _a)
{
    uint64_t x = _a & 0x1fffff;
    x = (x | x << 32) & 0x1f00000000ffff;
    x = (x | x << 16) & 0x1f0000ff0000ff;
    x = (x | x << 8) & 0x100f00f00f00f00f;
    x = (x | x << 4) & 0x10c30c30c30c30c3;
    x = (x | x << 2) & 0x1249249249249249;
    return x;
}

uint64_t Morton::pullByThree(unsigned int _a)
{
    uint64_t x = _a  & 0x1249249249249249;
    x = (x ^ (x >> 2)) & 0x10c30c30c30c30c3;
    x = (x ^ (x >> 4)) & 0x100f00f00f00f00f;
    x = (x ^ (x >> 8)) & 0x1f0000ff0000ff;
    x = (x ^ (x >> 16)) & 0x1f00000000ffff;
    x = (x ^ (x >> 32)) & 0x1fffff;
    return x;
}


void Morton::encodeMortonMagic(MortonCoords _m)
{
    m_Morton = 0;
    m_Morton |= pushByThree(_m.m_x) | pushByThree(_m.m_y) << 1 | pushByThree(_m.m_z) << 2;
}

MortonCoords Morton::decodeMortonMagic()
{
    MortonCoords retVal = MortonCoords();
    retVal.m_x = pullByThree(m_Morton);
    retVal.m_y = pullByThree(m_Morton>>1);
    retVal.m_z = pullByThree(m_Morton>>2);
    return retVal;
}

MortonCoords Morton::decodeMortonMagic(Morton _m)
{
    MortonCoords retVal = MortonCoords();
    retVal.m_x = pullByThree(_m.m_Morton);
    retVal.m_y = pullByThree(_m.m_Morton>>1);
    retVal.m_z = pullByThree(_m.m_Morton>>2);
    return retVal;
}
