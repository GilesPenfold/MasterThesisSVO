#include "NGLScene.h"
#include <iostream>
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Transformation.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <QColorDialog>
#include <ngl/Obj.h>
#include <ctime>

//----------------------------------------------------------------------------------------------------------------------
NGLScene::NGLScene( QWidget *_parent) : QOpenGLWidget( _parent )
{

  // set this widget to have the initial keyboard focus
  setFocus();
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  this->resize(_parent->size());
    m_showBBox=true;
    m_showBSphere=true;
    m_HasMesh = false;
    m_DrawOctree=false;

}

void NGLScene::LoadOBJ(std::string _fileName)
{

    m_mesh.reset(new ngl::Obj(_fileName, "textures/ratGrid.png"));
    m_mesh->createVAO();
    m_mesh->calcBoundingSphere();
    m_mesh->calcDimensions();

    m_HasMesh = true;

    update();
}

void NGLScene::LoadOctree(std::string _fileName)
{
    int start_s = clock();
    m_Voxelizer.LoadOctree(_fileName);
    int stop_s = clock();
    std::cout << "time: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 << std::endl;
}

// This virtual function is called once before the first call to paintGL() or resizeGL(),
//and then once whenever the widget has been assigned a new QGLContext.
// This function should set up any required OpenGL context rendering flags, defining display lists, etc.

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::initializeGL()
{
    // we must call this first before any other GL commands to load and link the
    // gl commands from the lib, if this is not done program will crash
    ngl::NGLInit::instance();

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);
    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);

    // Now we will create a basic Camera from the graphics library
    // This is a static camera so it only needs to be set once
    // First create Values for the camera position
    ngl::Vec3 from(0,4,8);
    ngl::Vec3 to(0,0,0);
    ngl::Vec3 up(0,1,0);
    m_cam.set(from,to,up);
    // set the shape using FOV 45 Aspect Ratio based on Width and Height
    // The final two are near and far clipping planes of 0.5 and 10
    m_cam.setShape(45.0f,720.0f/576.0f,0.05f,350.0f);
    // now to load the shader and set the values


    // grab an instance of shader manager
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    // load a frag and vert shaders

    shader->createShaderProgram("TextureShader");

    shader->attachShader("TextureVertex",ngl::ShaderType::VERTEX);
    shader->attachShader("TextureFragment",ngl::ShaderType::FRAGMENT);
    shader->loadShaderSource("TextureVertex","shaders/TextureVertex.glsl");
    shader->loadShaderSource("TextureFragment","shaders/TextureFragment.glsl");

    shader->compileShader("TextureVertex");
    shader->compileShader("TextureFragment");
    shader->attachShaderToProgram("TextureShader","TextureVertex");
    shader->attachShaderToProgram("TextureShader","TextureFragment");

    // link the shader no attributes are bound
    shader->linkProgramObject("TextureShader");
    (*shader)["TextureShader"]->use();

    (*shader)[ngl::nglColourShader]->use();

    shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);


    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->createSphere("sphere",1.0,20);
    // as re-size is not explicitly called we need to do this.
    glViewport(0,0,width(),height());


    //Voxelize("octree4", COMPRESSIONTYPE::SNAPPY, 1024);

    update();


}

void NGLScene::Voxelize(std::string _filename, COMPRESSIONTYPE _compress, int _gridSize)
{
    int start_s = clock();
    m_Voxelizer.Run(_filename, _compress, _gridSize);
    int stop_s=clock();
    std::cout << "time: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------
//This virtual function is called whenever the widget has been resized.
// The new size is passed in width and height.
void NGLScene::resizeGL( int _w, int _h )
{
    m_cam.setShape( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
    m_win.width  = static_cast<int>( _w * devicePixelRatio() );
    m_win.height = static_cast<int>( _h * devicePixelRatio() );
}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MVP=m_cam.getVPMatrix() *
                m_mouseGlobalTX*
                m_transform.getMatrix();

  shader->setUniform("MVP",MVP);
}

//----------------------------------------------------------------------------------------------------------------------
//This virtual function is called whenever the widget needs to be painted.
// this is our main drawing routine
void NGLScene::paintGL()
{
    // clear the screen and depth buffer
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glViewport(0,0,m_win.width,m_win.height);
     ngl::Mat4 rotX;
     ngl::Mat4 rotY;
     // create the rotation matrices
     rotX.rotateX(m_win.spinXFace);
     rotY.rotateY(m_win.spinYFace);
     // multiply the rotations
     m_mouseGlobalTX=rotY*rotX;
     // add the translations
     m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
     m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
     m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["TextureShader"]->use();
    m_transform.reset();
    loadMatricesToShader();

    if(m_HasMesh)
    {
        //m_mesh->bindVAO();
        // draw the mesh
        //m_Models[0]->draw();
        m_mesh->draw();
        //ngl::BBox BB = m_mesh->getBBox();
        //m_mesh->unbindVAO();
    }
    //m_bin->draw();
    // draw the mesh bounding box
    (*shader)["nglColourShader"]->use();

    if(m_showBBox==true && m_HasMesh)
    {
      loadMatricesToShader();
      shader->setUniform("Colour",0.0f,0.0f,1.0f,1.0f);
      //m_mesh->drawBBox();
      shader->setUniform("Colour",0.8f,0.1f,0.1f,1.0f);

      if(m_Voxelizer.m_Done && m_DrawOctree)
      {
          ngl::Vec3 bboxmin = ngl::Vec3(m_Voxelizer.GetBBox()->minX(),m_Voxelizer.GetBBox()->minY(),m_Voxelizer.GetBBox()->minZ());
          for(int i = 0; i < m_Voxelizer.GetGrid().size(); i++)
          {
              m_Voxelizer.GetGrid()[i]->m_BBoxGrid->draw();
              for(int j = 0; j < m_Voxelizer.GetGrid()[i]->m_NumTris(); j++)
                  m_Voxelizer.GetGrid()[i]->m_Tris[j]->m_BBox->draw();
          }
      }

    }

    if(m_showBSphere==true && m_HasMesh)
    {
      ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
      shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);
      m_transform.reset();
        glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
          m_transform.setPosition(m_mesh->getSphereCenter());
          m_transform.setScale(m_mesh->getSphereRadius(),m_mesh->getSphereRadius(),m_mesh->getSphereRadius());
          loadMatricesToShader();
          prim->draw("sphere");
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    }

    //m_text->renderText(10,18,"P toggle bounding Sphere B Toggle Bounding Box");
}

NGLScene::~NGLScene()
{
}

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  //case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  case Qt::Key_B : m_showBBox^=true; break;
  case Qt::Key_P : m_showBSphere^=true; break;
  case Qt::Key_Space :
        m_win.spinXFace=0;
        m_win.spinYFace=0;
        m_modelPos.set(ngl::Vec3::zero());
  break;
  default : break;
  }
  // finally update the GLWindow and re-draw
  update();
}
