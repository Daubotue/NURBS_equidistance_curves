/***************************************************************************************************
*
* 版权信息：版权所有 (c) 2019, 浙江大学
*
* 文件名称：mainwindow.h
* 摘    要：主窗口
*
* 当前版本：0.1.0
* 作    者：林旭军
* 日    期：2019-09-03
* 备    注：创建并完成迁移
***************************************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <palette.h>
#include <QDesktopServices>
#include <QAction>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void slot_fileEvent_savePic();

    void slot_editEvent_controlPoint();
    void slot_editEvent_controlLine();
    void slot_editEvent_controlPointLText();

    void slot_demoEvent_1();
    void slot_demoEvent_2();
    void slot_demoEvent_3();
    void slot_demoEvent_4();
    void slot_demoEvent_5();

    void slot_paintEvent_draw();

private:
    //Demo按钮的勾选
    void actionInitSetChecked();

private:
    Palette* palette;   // 画板

    QLabel *status_mousePos;

    //File
    QAction *file_savePic;

    //Edit
    QAction *edit_controlPoint;
    QAction *edit_controlLine;
    QAction *edit_controlPointLText;

    //Demo
    QAction *demo_1;
    QAction *demo_2;
    QAction *demo_3;
    QAction *demo_4;
    QAction *demo_5;

    //Paint
    QAction *paint_draw;

};

#endif // MAINWINDOW_H
