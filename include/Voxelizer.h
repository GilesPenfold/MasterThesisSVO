#ifndef VOXELIZER_H
#define VOXELIZER_H
#include <ngl/BBox.h>
#include <ngl/Obj.h>
#include <memory.h>
#include <morton.h>
#include <tri.h>
#include <bboxwrapper.h>
#include <octree.h>


#define EMPTYVOXEL 0
#define FILLEDVOXEL 1

struct VOXELLIST
{
    public:
        bool* m_voxels;
};



class Voxelizer
{
public:

    ///
    /// \brief Voxelizer
    /// Initializes the voxelizer object
    Voxelizer(){m_FullVoxels=0; m_Done = false;}

    ///
    /// \brief Run
    /// \param _outname
    /// \param _compress
    /// \param _gridSize
    /// Begins the process of voxelization on the file _outname with compression type _compress and grid size of _gridSize
    void Run(std::string _outname, COMPRESSIONTYPE _compress, int _gridSize);
    ///
    /// \brief MemoryCalculations
    /// \param _gridSize
    /// \return
    /// Performs memory calculations for the current file (mainly used for out of core, which we did not implement in the end)
    size_t MemoryCalculations(size_t _gridSize);
    ///
    /// \brief SubGrid
    /// \param _tris
    /// \param _grid
    /// \param _numPartitions
    /// \param _gridSize
    /// \param _unitLength
    /// Performs the subgrid operations, preprocessing the mesh and creating a morton ordered subgrid to work with for our voxelization
    void SubGrid(std::vector<Tri*> &_tris, std::vector<BBoxWrapper*> &_grid, size_t _numPartitions, size_t _gridSize, float _unitLength);
    ///
    /// \brief Voxelize
    /// \param _numPartitions
    /// \param _gridSize
    /// \param _unitLength
    /// Our main voxelization method using Schwarz's improved surface voxelization
    void Voxelize(size_t _numPartitions, size_t _gridSize, float _unitLength);
    ///
    /// \brief RunOctree
    /// \param _numPartitions
    /// \param _gridSize
    /// Generate our octree using Baert's method of octree construction and child queues
    void RunOctree(size_t _numPartitions, size_t _gridSize);
    ///
    /// \brief SaveToFile
    /// Save the current octree to file
    void SaveToFile();
    ///
    /// \brief LoadOctree
    /// \param _fileName
    /// Load an octree in from file
    void LoadOctree(std::string _fileName);

    ///
    /// \brief Cubify
    /// \param _mesh
    /// \return
    /// Expands a bounding box to become a cube instead of a rectangle
    ngl::BBox* Cubify(ngl::Obj* _mesh);

    ///
    /// \brief GetGrid
    /// \return
    /// Returns the current morton ordered grid
    std::vector<BBoxWrapper*> GetGrid(){return m_Grid;}

    ///
    /// \brief GetMesh
    /// \return
    /// Defunct. Returns the current mesh.
    ngl::Obj* GetMesh(){return m_Mesh.get();}
    ///
    /// \brief GetBBox
    /// \return
    /// Defunct. Returns the current bounding box.
    ngl::BBox* GetBBox(){return m_BoundBox;}

    bool m_Done;



private:

    std::unique_ptr<ngl::Obj> m_Mesh;
    ngl::BBox* m_BoundBox;
    const Morton m_Start;
    const Morton m_End;
    float m_Size;
    bool* m_Voxels;
    int m_FullVoxels;

    // Allocate 4GB of memory for calculations (defunct, not using out of core)
    int m_MemoryLimit = 4096;

    std::vector<BBoxWrapper*> m_Grid;

    Octree m_Octree;
    std::string m_FileName;



};

#endif // VOXELIZER_H
