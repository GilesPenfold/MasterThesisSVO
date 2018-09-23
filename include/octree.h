#ifndef OCTREE_H
#define OCTREE_H
#include <stdint.h>
#include <algorithm>
#include <node.h>
#include <vector>
#include <morton.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cube.h>
#include <snappy.h>
//#include "zip_file.hpp"
//#include "zip.h"
#include "miniz.h"

enum COMPRESSIONTYPE{NONE,SNAPPY,MINIZ};


// Based on: https://github.com/Forceflow/ooc_svo_builder/blob/master/src/svo_builder/OctreeBuilder.h

class Octree
{
public:

    Octree(){}
    Octree(std::string _name, size_t _GridSize, float _UnitLength);
    ///
    /// \brief AddVoxel
    /// \param _m
    /// Adds a voxel of given Morton code _m to the current octree
    void AddVoxel(Morton _m);
    ///
    /// \brief AddEmptyVoxel
    /// \param _b
    /// Fills the octree with a number of empty voxels equal to _b
    void AddEmptyVoxel(size_t _b);
    ///
    /// \brief FastAddEmptyVoxel
    /// \param _b
    /// Quickly fills the octree with a number of empty voxels equal to _b
    void FastAddEmptyVoxel(size_t _b);
    ///
    /// \brief Refine
    /// \param _b
    /// Refines the octree leaf nodes and readjusts the positions of the childred nodes after adding
    /// a lot of empty nodes at once
    void Refine(size_t _b);
    ///
    /// \brief Begin
    /// \param _compress
    /// Starts the octree construction process, indicating compression if it is to be applied
    void Begin(COMPRESSIONTYPE _compress);
    ///
    /// \brief Finalize
    /// Finishes the octree creation and writes the final nodes out to file
    void Finalize();
    ///
    /// \brief FindBestFillChild
    /// \param _b
    /// \return
    /// Given the number of voxels to fill _b, this will give the number of the child that
    /// should be allocated the voxels
    int FindBestFillChild(size_t _b);
    ///
    /// \brief FindHighestNonEmptyChild
    /// \return
    /// Locates the child node of highest order in the morton code that is not empty
    int FindHighestNonEmptyChild();
    ///
    /// \brief ChildEmpty
    /// \param _n
    /// \return
    /// Checks if the children nodes are empty
    bool ChildEmpty(std::vector<Node> &_n);
    ///
    /// \brief GroupNodes
    /// \param _n
    /// \return
    /// Groups nodes together into the same space within the octree
    Node GroupNodes(std::vector<Node> &_n);
    ///
    /// \brief FindPowerOfEight
    /// \param _b
    /// \return
    /// Gives the highest power of 8 that cna be found within _b
    unsigned int FindPowerOfEight(size_t _b);

    // IO
    ///
    /// \brief WriteNode
    /// \param _n
    /// Writes a node to file
    void WriteNode(Node &_n);
    ///
    /// \brief ReadNode
    /// \param _n
    /// Reads a node from file
    void ReadNode(Node &_n);
    ///
    /// \brief WriteData
    /// \param _m
    /// Writes the morton data to file
    void WriteData(Morton &_m);
    ///
    /// \brief ReadData
    /// Reads the morton data from file
    void ReadData();
    ///
    /// \brief WriteHeader
    /// Writes the octree header to file
    void WriteHeader();
    ///
    /// \brief ReadHeader
    /// Reads the octree header from file
    void ReadHeader();
    ///
    /// \brief ReadOctree
    /// Reads an octree in from file
    void ReadOctree();
    ///
    /// \brief CompressFile
    /// Defunct. Compresses an entire octree file.
    void CompressFile();
    ///
    /// \brief CompressFileIncremental
    /// \param _final
    /// Incrementally compresses a series of nodes to file during the construction process.
    void CompressFileIncremental(bool _final);
    ///
    /// \brief ReadCompressedFile
    /// Reads a compressed octree file.
    void ReadCompressedFile();
    ///
    /// \brief WriteNodeCompress
    /// \param _n
    /// Writes a node to file and allocates it for compression.
    void WriteNodeCompress(Node &_n);


    ///
    /// \brief Draw
    /// Defuct. Draws the octree.
    void Draw();


    std::vector<std::vector<Node>> m_Children;

    int m_MaxDepth;
    Morton m_CurrentMorton;
    Morton m_MaxMorton;
    size_t m_DataPosition;
    size_t m_NodePosition;
    size_t m_GridSize;

    COMPRESSIONTYPE m_Compression;

    FILE* m_File;
    std::string m_FileName;
    Cube _cube;

    std::vector<Node*> m_Nodes;

    std::ofstream m_outStream;
    std::ifstream m_inStream;

    std::vector<std::string*> m_Data;
    std::vector<std::string*> m_DebugData;
    int counter = 0;


};

#endif // OCTREE_H
