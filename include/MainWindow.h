#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "NGLScene.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    ///
    /// \brief Browse
    /// Browses the local file system for an obj file
    void Browse();
    ///
    /// \brief BrowseOct
    /// Browses the local file system for an octree or octreecompress file
    void BrowseOct();
    ///
    /// \brief Voxelize
    /// Runs the voxelization process on the selected obj file
    void Voxelize();
    ///
    /// \brief UpdateOctreeDraw
    /// Defunct. Draws the octree in the scene.
    void UpdateOctreeDraw();


private:
    Ui::MainWindow *m_ui;
		/// @brief our openGL widget
		NGLScene *m_gl;
};

#endif // MAINWINDOW_H
