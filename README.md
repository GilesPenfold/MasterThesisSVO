# In-Core Sparse Voxel Octrees - Giles Penfold, MSc CAVE Thesis 2017/18

#### This software compresses and decompresses SVO datasets given a .obj mesh, storing the voxels as a .octreecompress and .octreeheader file.

This is based on ForceFlow's OOC SVO builder (https://github.com/Forceflow/ooc_svo_builder) using NCCA's NGL library and additional compression functionality.

## Requirements:

- NCCA NGL (https://github.com/NCCA/NGL)
- OpenGL (https://www.opengl.org/sdk/)
- Snappy (https://github.com/google/snappy)
  Place snappy.h, snappy-internal.h, snappy-sinksource.h, snappy-stubs-internal.h, and snappy-stubs-public.h into the include folder.
- Miniz (https://github.com/richgel999/miniz)
  Place miniz.h into the include folder.


## Running the system:

Place the specified requirements into their folders.
Build the project using QTCreator.
Run the project file.

## Using the system:

Use the "Browse Obj" button to locate a .obj mesh to load in. The displayed mesh is not representative of the final mesh, however the bounding box is. This is due to issues between NGL and a specific QTCreator functionality.

Select the grid size of the octree, recommended to use <1024 for speed.

Select the compression type to be used.

Click Voxelize and wait until the process is complete. This can take some time.

To load in an octree, click the "Browse Octree" button and select an octree to load in. It will simply decompress it and load it into memory, proving that there was no data loss in the compression stage.