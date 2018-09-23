#ifndef NGLSCENE_H_
#define NGLSCENE_H_

#include <ngl/Camera.h>
#include <ngl/Transformation.h>
#include <ngl/Vec3.h>
#include <ngl/Text.h>
#include "WindowParams.h"
#include <QEvent>
#include <QResizeEvent>
#include <QOpenGLWidget>
#include <memory>
#include <Voxelizer.h>
#include <morton.h>


/// @file NGLScene.h
/// @brief a basic Qt GL window class for ngl demos
/// @author Jonathan Macey
/// @version 1.0
/// @date 10/10/10
/// Revision History :
/// Initial Version 10/10/10 (Binary day ;-0 )
/// @class GLWindow
/// @brief our main glwindow widget for NGL applications all drawing elements are
/// put in this file
class NGLScene : public QOpenGLWidget
{
Q_OBJECT        // must include this if you use Qt signals/slots
public :
  /// @brief Constructor for GLWindow
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief Constructor for GLWindow
  /// @param [in] _parent the parent window to create the GL context in
  //----------------------------------------------------------------------------------------------------------------------
  NGLScene(QWidget *_parent);

		/// @brief dtor
	~NGLScene();

  void Voxelize(std::string _filename, COMPRESSIONTYPE _compress, int _gridSize);
  void LoadOBJ(std::string _filename);
  void LoadOctree(std::string _filename);
  bool m_DrawOctree;

protected:
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the windows params such as mouse and rotations etc
  //----------------------------------------------------------------------------------------------------------------------
  WinParams m_win;
  /// @brief  The following methods must be implimented in the sub class
  /// this is called when the window is created
  void initializeGL();

  /// @brief this is called whenever the window is re-sized
  /// @param[in] _w the width of the resized window
  /// @param[in] _h the height of the resized window
  void resizeGL(int _w , int _h);
  /// @brief this is the main gl drawing routine which is called whenever the window needs to
  // be re-drawn
  void paintGL();

private :


  //----------------------------------------------------------------------------------------------------------------------
  /// @brief used to store the global mouse transforms
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_mouseGlobalTX;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief Our Camera
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Camera m_cam;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief transformation stack for the gl transformations etc
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Transformation m_transform;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the model position for mouse movement
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Vec3 m_modelPos;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief our model
  //----------------------------------------------------------------------------------------------------------------------
  //std::unique_ptr<ngl::Obj> m_mesh;
  std::unique_ptr<ngl::Obj> m_mesh;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief flag to show bounding box
  //----------------------------------------------------------------------------------------------------------------------
  bool m_showBBox;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief flag to show bounding sphere
  //----------------------------------------------------------------------------------------------------------------------
  bool m_showBSphere;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief text for rendering
  //----------------------------------------------------------------------------------------------------------------------
  std::unique_ptr<ngl::Text> m_text;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief method to load transform matrices to the shader
  //----------------------------------------------------------------------------------------------------------------------
  void loadMatricesToShader();
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief Qt Event called when a key is pressed
  /// @param [in] _event the Qt event to query for size etc
  //----------------------------------------------------------------------------------------------------------------------
  void keyPressEvent(QKeyEvent *_event);
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called every time a mouse is moved
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mouseMoveEvent (QMouseEvent * _event );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse button is pressed
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mousePressEvent ( QMouseEvent *_event);
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse button is released
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mouseReleaseEvent ( QMouseEvent *_event );

  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse wheel is moved
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void wheelEvent( QWheelEvent *_event);

  std::string m_objFileName;
  std::string m_textureFileName;

  bool m_HasMesh;


  //----------------------------------------------------------------------------------------------------------------------
  /// @brief Our Voxelizer objects
  /// This processes everything to do with Voxelization
  //----------------------------------------------------------------------------------------------------------------------
  Voxelizer m_Voxelizer;







};

#endif
