#ifndef PALETTE_H
#define PALETTE_H

/***************************************************************************************************
*
* 版权信息：版权所有 (c) 2019, 浙江大学
*
* 文件名称：palette.h
* 摘    要：算法具体实现，并实现绘图
*
* 当前版本：0.1.0
* 作    者：林旭军
* 日    期：2019-09-03
* 备    注：创建并完成迁移
***************************************************************************************************/
#include <QWidget>
#include <QVector>
#include <QPainter>
#include <QPaintEvent>
#include <cmath>
#include <QCursor>

//最大值, 最小值
const double DMAX = 1.e12;
const double DMIN = 1.e-12;

//控制点结构体
struct structPoint
{
    QPointF point;
    double weight;
};

class Palette : public QWidget
{
    Q_OBJECT
public:
    explicit Palette(QWidget *parent = nullptr);
    ~Palette();

protected:
    void mousePressEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);

public:
    void init();//初始化
    void generateCurve(bool bIsseal);                // 生成NURBS样条曲线
    void generateOffset(double offdis=10.0);         // 生成NURBS-offset样条曲线

    void paintModelChoice(int n=1);                  //n代表选择哪一幅图画
    void paintModel_1(QVector<structPoint> &mctrlPoints, bool isseal=true);
    void paintModel_2(QVector<structPoint> &mctrlPoints, bool isseal=true);
    void paintModel_3(QVector<structPoint> &mctrlPoints, bool isseal=true);
    void paintModel_4(QVector<structPoint> &mctrlPoints, bool isseal=false);
    void paintModel_5(QVector<structPoint> &mctrlPoints, bool isseal=false);

private:
    double N(int k, int i, double u);
    double N1(int i, double u);
    double N2(int i, double u);
    double N3(int i, double u);   // 三次B样条的基函数
    int    getCurrentNode(QPointF t);

    //计算一阶导数
    void   calDerivative(QVector<QPointF> &mcurvePoints);
    //计算二阶导数
    void   cal2Derivative(QVector<double> &mderivative_x, QVector<double> &mderivative_y);
    //等距线，offdistance为偏移距离
    void   calOffsetCurve(QVector<QPointF> &mcurvePoints, QVector<QPointF> &mnormal_vector,
                        QVector<double> &mradCurvity, int offdistance);
    //曲线光顺
    void   calOffsetDerivative(const QVector<QPointF> &moffsetPoints,const QVector<int> &mtargetLocation,
                             int begin=30, int end=30);//计算AB点一阶和二阶,begin和end为偏移点,保存光顺点
    //计算曲线凸凹性
    double calConvexHull(QVector<QPointF> &mcurvePoints, const int &index);

    //回溯求解交点
    void   backuponCrossPoint(QVector<int> &mtargetLocation, const QVector<QPointF> &moffsetPoints);

    //计算偏差
    void   calBias(QVector<QPointF> &mdis_CrossPoints,
                 QVector< QVector<QPointF> > &mdb_fairingPoints);

    //初始化连续
    void   initFairing(); //初始化曲线首尾光顺

public:
    bool   b_model3;                                //demo3 的中心线
    bool   showCtrlNode;                            //控制点节点
    bool   showCtrlNodeLine;                        //控制点节点连线
    bool   showCtrlNodeText;                        //控制点节点文本
    bool   showCurveNode;                           //曲线节点
    bool   Cutting_curves;                          //是否裁剪
    bool   Smoothing_of_equidistant_curves;         //是否处理自交
    bool   Isseal;                                  //图形是否封闭
    bool   PaintMenu;                               //paint按钮
    bool   showFairingCurve;                        //光顺曲线是否高亮
    double step_width;                              //步长
    int    currentK;                                //
    double offset;                                  //偏移距离
    int    deltaCurve;                              //曲线取点间隔
    int    currentNode;

    //控制顶点，  NURBS曲线上的离散点， NURBS-offset曲线上的离散点
    QVector<structPoint>          ctrlPoints;          // 控制点
    QVector<QPointF>              curvePoints;         // NURBS曲线上的点
    QVector<QPointF>              offsetPoints;        // NURBS-offset曲线上的点
    QVector<QPointF>              fairingPoints;       // fairing曲线上的点
    QVector<QVector<structPoint>> db_ctrlPoints;       //控制点集合
    QVector<QVector<QPointF>>     db_curvePoints;      //NURBS曲线上的点集合
    QVector<QVector<QPointF>>     db_offsetPoints;     //NURBS-offset曲线上的点集合
    QVector<QVector<QPointF>>     db_fairingPoints;    //fairing曲线上的点集合

    //过渡曲线偏差
    QVector<QPointF> dis_CrossPoints;     // 自交点

    //一阶偏导， 二阶偏导
    QVector<double>  derivative_x;        //x导数
    QVector<double>  derivative_y;        //y导数
    QVector<QPointF> normal_vector;       //单位法向量
    QVector<double>  derivative_x2;       //x的2阶导数
    QVector<double>  derivative_y2;       //y的2阶导数
    QVector<double>  radCurvity;          //曲率半径
};

#endif // PALETTE_H
