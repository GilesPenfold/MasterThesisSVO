# This specifies the exe name
TARGET=SVOThesis
# where to put the .o files
OBJECTS_DIR=obj
# core Qt Libs to use add more here if needed.
QT+=gui opengl core

# as I want to support 4.8 and 5 this will set a flag for some of the mac stuff
# mainly in the types.h file for the setMacVisual which is native in Qt5
isEqual(QT_MAJOR_VERSION, 5) {
	cache()
	DEFINES +=QT5BUILD
}

# where to put moc auto generated files
MOC_DIR=moc
# on a mac we don't create a .app bundle file ( for ease of multiplatform use)
CONFIG-=app_bundle
# Auto include all .cpp files in the project src directory (can specifiy individually if required)
SOURCES+= src/NGLScene.cpp \
          src/NGLSceneMouseControls.cpp \
          src/MainWindow.cpp \
          src/main.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/morton.cpp \
    src/NGLScene.cpp \
    src/NGLSceneMouseControls.cpp \
    src/octree.cpp \
    src/snappy.cc \
    src/snappy-sinksource.cc \
    src/snappy-stubs-internal.cc \
    src/voxelizer.cpp \
    src/miniz.c \

# same for the .h files
HEADERS+= include/NGLScene.h \
          include/MainWindow.h \
          include/WindowParams.h \
            include/bboxwrapper.h \
            include/cube.h \
            include/MainWindow.h \
            include/morton.h \
            include/mortoncoords.h \
            include/NGLScene.h \
            include/node.h \
            include/octree.h \
            include/snappy.h \
            include/snappy-internal.h \
            include/snappy-sinksource.h \
            include/snappy-stubs-internal.h \
            include/snappy-stubs-public.h \
            include/Voxelizer.h \
            include/WindowParams.h \
            include/miniz.h \
            include/tri.h



FORMS+= $$PWD/ui/MainWindow.ui
# and add the include dir into the search path for Qt and make
INCLUDEPATH +=./include
# where our exe is going to live (root of project)
DESTDIR=./
# add the glsl shader files
OTHER_FILES+= README.md \
              shaders/*.glsl
# were are going to default to a console app
CONFIG += console
NGLPATH=$$(NGLDIR)
isEmpty(NGLPATH){ # note brace must be here
	message("including $HOME/NGL")
	include($(HOME)/NGL/UseNGL.pri)
}
else{ # note brace must be here
	message("Using custom NGL location")
	include($(NGLDIR)/UseNGL.pri)
}


