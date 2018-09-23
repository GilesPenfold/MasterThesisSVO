#pragma once
#ifndef MORTON_H
#define MORTON_H
#include <stdint.h>
#include <mortoncoords.h>
//#include "zip_file.hpp"


// Based on this article http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
// Using the magic bits method

class Morton
{
public:

    Morton();
    Morton(uint_fast64_t _x);
    ~Morton();

    ///
    /// \brief pushByThree
    /// \param _a
    /// \return
    /// Spread an unsigned int _a along a uint64 so that it can be interleaved
    uint64_t pushByThree(unsigned int _a);
    ///
    /// \brief pullByThree
    /// \param _a
    /// \return
    /// Compress a unsigned int _a along a unint64 so that it can be uninterleaved
    uint64_t pullByThree(unsigned int _a);

    ///
    /// \brief encodeMortonMagic
    /// \param _m
    /// Encodes a series of 3 integers in _m into morton code
    ///
    void encodeMortonMagic(MortonCoords _m);
    ///
    /// \brief decodeMortonMagic
    /// \return
    /// Decodes the current morton of this morton object into a series of 3 ints stored as MortonCoords
    MortonCoords decodeMortonMagic();
    ///
    /// \brief decodeMortonMagic
    /// \param _m
    /// \return
    /// Decodes a morton _m into a series of 3 ints stored as MortonCoords
    MortonCoords decodeMortonMagic(Morton _m);

    uint_fast64_t M(){return m_Morton;}

    Morton& operator++ ()
    {
        m_Morton++;
        return *this;
    }

private:
    uint_fast64_t m_Morton;

};


#endif // MORTON_H
