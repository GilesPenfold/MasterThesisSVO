#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <qfiledialog.h>
#include <qDebug>

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), m_ui(new Ui::MainWindow)
{
  m_ui->setupUi(this);

  qDebug() << "project path : " << QDir::currentPath().toUtf8();

  std::string oname("models/Helix_Old.obj");
  std::string tname("textures/helix_base.tif");

  m_gl=new NGLScene(this);

  m_ui->s_mainWindowGridLayout->addWidget(m_gl,0,0,2,1);
  m_ui->m_GridSize->setCurrentIndex(1);
  m_ui->m_CompressionTypes->setCurrentIndex(1);

  connect(m_ui->m_Browse, &QAbstractButton::clicked, this, &MainWindow::Browse);
  connect(m_ui->m_Voxelize, &QAbstractButton::clicked, this, &MainWindow::Voxelize);
  connect(m_ui->m_BrowseOct, &QAbstractButton::clicked, this, &MainWindow::BrowseOct);
  //connect(m_ui->m_ShowOctree, &QAbstractButton::clicked, this, &MainWindow::UpdateOctreeDraw);

}

void MainWindow::Voxelize()
{

    COMPRESSIONTYPE C;

    if(m_ui->m_CompressionTypes->currentIndex() == 0)
        C = NONE;
    if(m_ui->m_CompressionTypes->currentIndex() == 1)
        C = SNAPPY;
    if(m_ui->m_CompressionTypes->currentIndex() == 2)
        C = MINIZ;

    m_gl->Voxelize(m_ui->m_DirectoryComboBox->text().toStdString(), C, m_ui->m_GridSize->currentText().toInt());
}

//void MainWindow::UpdateOctreeDraw()
//{
//    m_gl->m_DrawOctree = m_ui->m_ShowOctree->isChecked();
//    std::cout << "I was clicked!\n";
//}

void MainWindow::Browse()
{

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open .obj file"), QDir::currentPath()+"/models", tr("OBJ (*.obj) ;; All Files (*)"));

        if (fileName.isEmpty())
                return;
        else
        {
            qDebug() << "selected file path : " << fileName.toUtf8();

            std::string path(QDir::currentPath().toStdString());
            std::string file(fileName.toStdString());

            file = file.substr(path.length()+1,file.size());

            m_gl->LoadOBJ(file);

            m_ui->m_DirectoryComboBox->setText(QString::fromStdString(file));
        }

}

void MainWindow::BrowseOct()
{

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open octree file"), QDir::currentPath(), tr("octree (*.octree) ;; octreecompress (*.octreecompress) ;; All Files (*)"));

        if (fileName.isEmpty())
                return;
        else
        {
            qDebug() << "selected file path : " << fileName.toUtf8();

            std::string path(QDir::currentPath().toStdString());
            std::string file(fileName.toStdString());

            file = file.substr(path.length()+1,file.size());

            //m_gl->LoadOBJ(file);

            m_gl->LoadOctree(file);

            m_ui->m_OctCombo->setText(QString::fromStdString(file));
        }

}

MainWindow::~MainWindow()
{
    delete m_ui;
}
