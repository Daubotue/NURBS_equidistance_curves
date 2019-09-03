#include "mainwindow.h"

#include <QStatusBar>
#include <QMenuBar>
#include <QMouseEvent>
#include <QFileDialog>
#include <QApplication>
#include <QPixmap>
#include <QDesktopWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    palette = new Palette(this);

    setWindowTitle("NURBS曲线模拟程序: NURBS Curve - By Robin Lin");
    setWindowState(Qt::WindowMaximized);

    //控件
    //mouse位置点
    status_mousePos = new QLabel(this);
    status_mousePos->setFrameStyle(QFrame::Box | QFrame::Sunken);
    statusBar()->addWidget(status_mousePos);
//    centralWidget()->setMouseTracking(true);
    setMouseTracking(true);

    //Menu
    QMenu *File  = this->menuBar()->addMenu(tr("&文件"));
    QMenu *Edit  = this->menuBar()->addMenu(tr("&编辑"));
    QMenu *Demo  = this->menuBar()->addMenu(tr("&演示"));
    QMenu *Paint = this->menuBar()->addMenu(tr("&绘图"));

    //File
    file_savePic = new QAction(tr("Save Picture"), this);
    file_savePic->setStatusTip(tr("Save picture in file of SAVE."));

    File->addAction(file_savePic);

    //Edit
    edit_controlPoint      = new QAction(tr("Control Points"), this);
    edit_controlLine       = new QAction(tr("Control Points Line"), this);
    edit_controlPointLText = new QAction(tr("Control Points Text"), this);
    edit_controlPoint->setCheckable(true);
    edit_controlLine->setCheckable(true);
    edit_controlPointLText->setCheckable(true);
    edit_controlPoint->setChecked(true);
    edit_controlLine->setChecked(true);

    Edit->addAction(edit_controlPoint);
    Edit->addAction(edit_controlLine);
    Edit->addAction(edit_controlPointLText);

    //Demo
    demo_1 = new QAction(tr("Demo1: S-Curve"), this);
    demo_1->setToolTip("S-Curve");
    demo_1->setStatusTip("S-Curve: 2 offset curves");
    demo_1->setCheckable(true);
    demo_2 = new QAction(tr("Demo2: Butterfly"), this);
    demo_2->setToolTip("Butterfly");
    demo_2->setStatusTip("Butterfly: 2 offset curves");
    demo_2->setCheckable(true);
    demo_3 = new QAction(tr("Demo3: Part"), this);
    demo_3->setToolTip("Part");
    demo_3->setStatusTip("Part: 2 offset curves");
    demo_3->setCheckable(true);
    demo_4 = new QAction(tr("Demo4: Curve1"), this);
    demo_4->setToolTip("Curve1");
    demo_4->setStatusTip("Curve1: 2 offset curves");
    demo_4->setCheckable(true);
    demo_5 = new QAction(tr("Demo5: Curve2"), this);
    demo_5->setToolTip("Curve2");
    demo_5->setStatusTip("Curve2: 2 offset curves");
    demo_5->setCheckable(true);

    Demo->addAction(demo_1);
    Demo->addAction(demo_2);
    Demo->addAction(demo_3);
    Demo->addAction(demo_4);
    Demo->addAction(demo_5);

    //Paint
    paint_draw = new QAction(tr("Draw"), this);
    paint_draw->setStatusTip("Draw Nurbs Curve.");
    paint_draw->setCheckable(true);

    Paint->addAction(paint_draw);

    //connect
    connect(file_savePic, &QAction::triggered, this, &MainWindow::slot_fileEvent_savePic);

    connect(edit_controlPoint, &QAction::triggered, this, &MainWindow::slot_editEvent_controlPoint);
    connect(edit_controlLine, &QAction::triggered, this, &MainWindow::slot_editEvent_controlLine);
    connect(edit_controlPointLText, &QAction::triggered, this, &MainWindow::slot_editEvent_controlPointLText);

    connect(demo_1, &QAction::triggered, this, &MainWindow::slot_demoEvent_1);
    connect(demo_2, &QAction::triggered, this, &MainWindow::slot_demoEvent_2);
    connect(demo_3, &QAction::triggered, this, &MainWindow::slot_demoEvent_3);
    connect(demo_4, &QAction::triggered, this, &MainWindow::slot_demoEvent_4);
    connect(demo_5, &QAction::triggered, this, &MainWindow::slot_demoEvent_5);

    connect(paint_draw, &QAction::triggered, this, &MainWindow::slot_paintEvent_draw);
}

MainWindow::~MainWindow()
{
    delete palette;
    delete status_mousePos;
    delete file_savePic;
    delete edit_controlPoint;
    delete edit_controlLine;
    delete edit_controlPointLText;
    delete demo_1;
    delete demo_2;
    delete demo_3;
    delete demo_4;
    delete demo_5;
    delete paint_draw;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    palette->setGeometry(0, 21, width(), height()-21);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QString pStr("%1 , %2");
    pStr = pStr.arg(event->globalPos().x()).arg(event->globalPos().y()); //pos是坐标
    status_mousePos->setText(pStr);
}

void MainWindow::slot_fileEvent_savePic()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("保存图片"),
                                                    "",
                                                    tr("Config Files (*.bmp)"));
    QPixmap pix, bmp;
    pix =bmp.grabWindow(QApplication::desktop()->winId(),
                        0,70,frameGeometry().width()-100,frameGeometry().height()-100);
    if (pix.isNull())
    {
        QMessageBox::information(this, "Error", "Picture is Null.", QMessageBox::Ok);
    }
    else
    {
        if(!pix.save(fileName,"BMP"))
        {
            QMessageBox::information(this, "Right", tr("保存错误 !"), QMessageBox::Ok);
        }
        else
        {
            QMessageBox::information(this, "Save", tr("保存成功!"),QMessageBox::Ok);
        }
    }
}

void MainWindow::slot_editEvent_controlPoint()
{
    //按钮勾选
    palette->showCtrlNode = !palette->showCtrlNode;
    edit_controlPoint->setChecked(palette->showCtrlNode);
    palette->update();
}

void MainWindow::slot_editEvent_controlLine()
{
    //按钮勾选
    palette->showCtrlNodeLine = !palette->showCtrlNodeLine;
    edit_controlPoint->setChecked(palette->showCtrlNodeLine);
    palette->update();
}

void MainWindow::slot_editEvent_controlPointLText()
{
    //按钮勾选
    palette->showCtrlNodeText = !palette->showCtrlNodeText;
    edit_controlPoint->setChecked(palette->showCtrlNodeText);
    palette->update();
}

void MainWindow::slot_demoEvent_1()
{
    //画曲线
    palette->init();
    actionInitSetChecked();
    demo_1->setChecked(true);
    palette->paintModelChoice(1);
    palette->update();
}

void MainWindow::slot_demoEvent_2()
{
    //画曲线
    palette->init();
    actionInitSetChecked();
    demo_2->setChecked(true);
    palette->paintModelChoice(2);
    palette->update();
}
void MainWindow::slot_demoEvent_3()
{
    //画曲线
    palette->init();
    actionInitSetChecked();
    demo_3->setChecked(true);
    palette->paintModelChoice(3);
    palette->update();
}

void MainWindow::slot_demoEvent_4()
{
    //画曲线
    palette->init();
    actionInitSetChecked();
    demo_4->setChecked(true);
    palette->paintModelChoice(4);
    palette->update();
}

void MainWindow::slot_demoEvent_5()
{
    //画曲线
    palette->init();
    actionInitSetChecked();
    demo_5->setChecked(true);
    palette->paintModelChoice(5);
    palette->update();
}

void MainWindow::slot_paintEvent_draw()
{
    //手动划线
    palette->init();
    actionInitSetChecked();
    palette->PaintMenu = true;
    paint_draw->setChecked(palette->PaintMenu);
    palette->update();
}

void MainWindow::actionInitSetChecked()
{
    demo_1->setChecked(false);
    demo_2->setChecked(false);
    demo_3->setChecked(false);
    demo_4->setChecked(false);
    demo_5->setChecked(false);

    paint_draw->setChecked(false);
}

