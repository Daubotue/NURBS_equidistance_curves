#include "palette.h"
#include <QDebug>
#include <cmath>
#include <QTextStream>
#include <QMessageBox>
#include <io.h>
#include <direct.h>

Palette::Palette(QWidget *parent) :
    QWidget(parent)
    ,Cutting_curves(true)
    ,Smoothing_of_equidistant_curves(true)
    ,Isseal(true)
    ,showFairingCurve(true)
    ,step_width(0.01)
    ,deltaCurve(1)
{
    init();
    currentK = 3;
    offset  = -40;

    showCtrlNode     = true;
    showCtrlNodeLine = true;
    showCtrlNodeText = false;
    showCurveNode    = false;

    setMouseTracking(true);

    //测试方便
    paintModelChoice(4);
}

Palette::~Palette()
{
}

void Palette::init()
{
    b_model3  = false;
    PaintMenu = false;
    ctrlPoints.clear();
    curvePoints.clear();
    offsetPoints.clear();
    db_ctrlPoints.clear();
    db_curvePoints.clear();
    db_offsetPoints.clear();
    db_fairingPoints.clear();
    dis_CrossPoints.clear();
}

void Palette::mousePressEvent(QMouseEvent* event)
{
    if (PaintMenu)
    {
        if(event->button() == Qt::LeftButton)
        {
            setCursor(QCursor(Qt::CrossCursor));

            int i = getCurrentNode(event->pos());
            if(i >= 0)
            {
                currentNode = i;
                return;
            }
            structPoint tmp;
            tmp.point = event->pos();;
            tmp.weight = 1.0;
            ctrlPoints.push_back(tmp);
            currentNode = ctrlPoints.size()-1;
            generateCurve(Isseal = false);
            update();
        }
    }
    else
    {
        setCursor(QCursor(Qt::ArrowCursor));
    }
}

void Palette::paintEvent(QPaintEvent *event)
{
    QPainter* painter = new QPainter(this);

    // draw Contorl Points
    if (showCtrlNode)
    {
        QPen ctrlPen1(QColor(0,0,255));
        ctrlPen1.setWidth(7);
        painter->setPen(ctrlPen1);
        QBrush bruch(Qt::SolidPattern);//画刷
        bruch.setColor(Qt::gray);
        painter->setBrush(bruch);//设置画刷
        for(int i=0; i<ctrlPoints.size(); i++)
        {
            painter->drawEllipse(static_cast<int>(ctrlPoints[i].point.x())-1,
                                 static_cast<int>(ctrlPoints[i].point.y())-1,
                                 2, 2);
        }
    }

    // draw Control Lines;
    if(showCtrlNodeLine)
    {
        QPen ctrlPen(QColor(0,255,0));
        ctrlPen.setWidth(2);
        ctrlPen.setStyle(Qt::DashDotDotLine);
        painter->setPen(ctrlPen);
        for(int i=0; i<ctrlPoints.size()-1; i++)
        {
            painter->drawLine(ctrlPoints[i].point,ctrlPoints[i+1].point);
        }
    }

    // draw NURBS Curve
    QPen curvePen(QColor(0,0,0));
    curvePen.setWidth(2);
    painter->setPen(curvePen);

    for(int i=0; i<curvePoints.size()-1; i++)
    {
        painter->drawLine(curvePoints[i],curvePoints[i+1]);
    }

    // draw OFFSET Curve
    QPen offsetPen(QColor(255,0,0));
    offsetPen.setWidth(2);
    painter->setPen(offsetPen);

    for (int i=0; i<db_offsetPoints.size(); i++)
    {
        for (int j=0; j<db_offsetPoints.at(i).size()-1; j++)
        {
            painter->drawLine(db_offsetPoints.at(i)[j], db_offsetPoints.at(i)[j+1]);
        }
    }

    for (int i=0; i<offsetPoints.size()-1; i++)
    {
        painter->drawLine(offsetPoints.at(i), offsetPoints.at(i+1));
    }

    //draw fairing curve
    if (showFairingCurve)
    {
        QPen offsetPen(QColor(0,0,255));
        offsetPen.setWidth(2);
        painter->setPen(offsetPen);
        for (int i=0; i<db_fairingPoints.size(); i++)
        {
            for (int j=0; j<db_fairingPoints.at(i).size()-1; j++)
            {
                painter->drawLine(db_fairingPoints.at(i)[j], db_fairingPoints.at(i)[j+1]);
            }
        }
    }

    //show Control Node Text
    if (showCtrlNodeText)
    {
        QPen pen1(QColor(0,0,255));
        painter->setPen(pen1);
        painter->setFont(QFont("Consolas",18));
        for(int i=0; i< ctrlPoints.size(); i++)
        {
            QPointF t(ctrlPoints[i].point.x()-40,ctrlPoints[i].point.y()+35);
            painter->drawText(t,QString("P%1").arg(i));
        }
    }

    // (Optional) show some hint text on Curve
    if(showCurveNode)
    {
        QPen pen2(QColor(10,200,50));
        pen2.setWidth(5);
        painter->setPen(pen2);
        painter->setFont(QFont("Consolas",12));
        for(double u=currentK; u<=ctrlPoints.size(); u+=1)
        {
            QPointF tmp(10,20);
            for(int i=0; i<ctrlPoints.size();i++)
            {
                QPointF t = ctrlPoints[i].point;

                t*=N(currentK,i,u);

                tmp+=t;
            }
            painter->drawText(tmp,QString("u=%1").arg(u));
            painter->drawPoint(static_cast<int>(tmp.x()-10), static_cast<int>(tmp.y()-20));
        }
    }
    //画中心线model3
    if (b_model3)
    {
        QPen medianPen1(QColor(0,0,255));
        medianPen1.setWidth(2);
        medianPen1.setStyle(Qt::DashDotDotLine);
        painter->setPen(medianPen1);
        painter->drawLine(150,400, 970,400);
    }
    delete painter;
    painter = nullptr;
}

void Palette::generateCurve(bool bIsseal)
{
    curvePoints.clear();
    db_fairingPoints.clear();
    for (double u=1; u<ctrlPoints.size()+2; u+=step_width)
    {
        QPointF tmp(0,0);
        double tmp1 = 0;
        for(int i=0; i<ctrlPoints.size();i++)
        {
            QPointF t = ctrlPoints[i].point;
            t *= N(currentK,i,u) * ctrlPoints.at(i).weight;
            tmp += t;
        }
        for(int i=0; i<ctrlPoints.size();i++)
        {
            double t = 1;
            t = N(currentK,i,u) * ctrlPoints.at(i).weight;
            tmp1 += t;
        }
        curvePoints.push_back(tmp / tmp1);
    }
    //验证原始NURBS点
    QFile data("..\\..\\processData\\output_OriginalctrlPoints.txt");
    if (data.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&data);
        for (int i=0; i<curvePoints.size(); i++)
        {
            out<<qSetFieldWidth(8)<<curvePoints.at(i).x()<<"  "
              <<curvePoints.at(i).y()<<endl;
        }
    }
    data.close();

    //是否初始化光顺
    if (bIsseal)
    {
        initFairing();
    }
    db_curvePoints.push_back(curvePoints);
}

void Palette::generateOffset(double offdis)
{
    //清空
    offsetPoints.clear();
    db_fairingPoints.clear();
    //计算一阶导数
    calDerivative(curvePoints);
    //计算二阶导数
    cal2Derivative(derivative_x, derivative_y);
    //得到offset曲线的点
    calOffsetCurve(curvePoints, normal_vector, radCurvity, static_cast<int>(offdis));

    //保存offset点到offset曲线上的点集合
    db_offsetPoints.push_back(offsetPoints);
}

void Palette::paintModelChoice(int n)
{
    switch(n)
    {
    case 1: paintModel_1(ctrlPoints, true);break;
    case 2: paintModel_2(ctrlPoints, true);break;
    case 3: paintModel_3(ctrlPoints, false);break;
    case 4: paintModel_4(ctrlPoints, false);break;
    case 5: paintModel_5(ctrlPoints, false);break;
    default: break;
    }
}

void Palette::paintModel_1(QVector<structPoint> &mctrlPoints, bool isseal)
{
    mctrlPoints.clear(); //先清空
    QFile file("..\\..\\demo\\S_Curve.txt");
    //已读写方式打开文件,如果文件不存在会自动创建文件
    if(!file.open(QIODevice::ReadWrite))
    {
        qDebug()<<"打开失败";
    }
    QTextStream in(&file);
    while (! in.atEnd())
    {
        structPoint sP;
        in>>sP.point.rx();
        in>>sP.point.ry();
        in>>sP.weight;
        mctrlPoints.push_back(sP);
    }
    file.close();
    db_ctrlPoints.push_back(mctrlPoints);
    //控制顶点求NURBS曲线
    generateCurve(isseal);
    offset = 30;
    generateOffset(offset);
}

void Palette::paintModel_2(QVector<structPoint> &mctrlPoints, bool isseal)
{
    mctrlPoints.clear(); //先清空
    QFile file("..\\..\\demo\\Butterfly_Curve.txt");
    //已读写方式打开文件,如果文件不存在会自动创建文件
    if(!file.open(QIODevice::ReadWrite))
    {
        qDebug()<<"打开失败";
    }
    QTextStream in(&file);
    while (! in.atEnd())
    {
        structPoint sP;
        in>>sP.point.rx();
        in>>sP.point.ry();
        in>>sP.weight;
        mctrlPoints.push_back(sP);
    }
    file.close();
    db_ctrlPoints.push_back(mctrlPoints);
    //控制顶点求NURBS曲线
    generateCurve(isseal);
    offset = 8;
    generateOffset(offset);
}

void Palette::paintModel_3(QVector<structPoint> &mctrlPoints, bool isseal)
{
    b_model3 = true;
    mctrlPoints.clear(); //先清空
    QFile file("..\\..\\demo\\Santanyingyue_Curve.txt");
    //已读写方式打开文件,如果文件不存在会自动创建文件
    if(!file.open(QIODevice::ReadWrite))
    {
        qDebug()<<"打开失败";
    }
    QTextStream in(&file);
    int index = 0;
    while (index++ < 26)
    {
        structPoint sP;
        in>>sP.point.rx();
        in>>sP.point.ry();
        in>>sP.weight;
        mctrlPoints.push_back(sP);
    }
    file.close();
    db_ctrlPoints.push_back(mctrlPoints);
    //控制顶点求NURBS曲线
    generateCurve(isseal);
    offset = 8;
    generateOffset(offset);
    QPointF pBegin(200, 400);
    QPointF pEnd(925, 400);
    curvePoints.push_front(pBegin);
    curvePoints.push_back(pEnd);
    QPointF pOffBegin(208, 400);
    QPointF pOffEnd(917, 400);
    offsetPoints.push_front(pOffBegin);
    offsetPoints.push_back(pOffEnd);
}

void Palette::paintModel_4(QVector<structPoint> &mctrlPoints, bool isseal)
{
    mctrlPoints.clear(); //先清空
    QFile file("..\\..\\demo\\demo1_Curve.txt");
    //已读写方式打开文件,如果文件不存在会自动创建文件
    if(!file.open(QIODevice::ReadWrite))
    {
        qDebug()<<"打开失败";
    }
    QTextStream in(&file);
    while (! in.atEnd())
    {
        structPoint sP;
        in>>sP.point.rx();
        in>>sP.point.ry();
        in>>sP.weight;
        mctrlPoints.push_back(sP);
    }
    file.close();
    db_ctrlPoints.push_back(mctrlPoints);
    //控制顶点求NURBS曲线
    generateCurve(isseal);
    offset = 40;
    generateOffset(offset);
}

void Palette::paintModel_5(QVector<structPoint> &mctrlPoints, bool isseal)
{
    mctrlPoints.clear(); //先清空
    QFile file("..\\..\\demo\\demo2_Curve.txt");
    //已读写方式打开文件,如果文件不存在会自动创建文件
    if(!file.open(QIODevice::ReadWrite))
    {
        qDebug()<<"打开失败";
    }
    QTextStream in(&file);
    while (! in.atEnd())
    {
        structPoint sP;
        in>>sP.point.rx();
        in>>sP.point.ry();
        in>>sP.weight;
        mctrlPoints.push_back(sP);
    }
    file.close();
    db_ctrlPoints.push_back(mctrlPoints);
    //控制顶点求NURBS曲线
    generateCurve(isseal);
    offset = -8;
    generateOffset(offset);
}

double Palette::N(int k, int i, double u)
{
    assert(k>=1 && k<=3);
    switch(k)
    {
    case 1:
        return N1(i,u);
    case 2:
        return N2(i,u);
    case 3:
        return N3(i,u);
    }
    return 0;
}

double Palette::N1(int i, double u)
{
    double t = u-i;

    if(0<=t && t<1)
        return t;
    if(1<=t && t<2)
        return 2-t;
    return 0;
}

double Palette::N2(int i, double u)
{
    double t = u-i;

    if(0<=t && t<1)
        return 0.5*t*t;
    if(1<=t && t<2)
        return 3*t-t*t-1.5;
    if(2<=t && t<3)
        return 0.5*pow(3-t,2);
    return 0;
}

double Palette::N3(int i, double u)
{
    double t = u-i;
    double a = 1.0/6.0;

    if(0<=t && t<1)
        return a*t*t*t;
    if(1<=t && t<2)
        return a*(-3*pow(t-1,3) + 3*pow(t-1,2) + 3*(t-1) +1);
    if(2<=t && t<3)
        return a*(3*pow(t-2,3) - 6*pow(t-2,2) +4);
    if(3<=t && t<4)
        return a*pow(4-t,3);
    return 0;
}

int Palette::getCurrentNode(QPointF t)
{
    for(int i=0; i<ctrlPoints.size(); i++)
    {
        double dx = ctrlPoints[i].point.x() - t.x();
        double dy = ctrlPoints[i].point.y() - t.y();

        double length = sqrt(dx*dx+dy*dy);

        if(length<10)
        {
            return i;
        }
    }
    return -1;
}

void Palette::calDerivative(QVector<QPointF> &mcurvePoints)
{
    //先清空一阶导数, 法矢
    derivative_x.clear();
    derivative_y.clear();
    normal_vector.clear();
    //3点求导
    QVector<QPointF> curvePoints_dup(mcurvePoints);
    curvePoints_dup.push_front(mcurvePoints.front());
    curvePoints_dup.push_back(mcurvePoints.last());
    if (mcurvePoints.size() > 1)
    {
        double tmp_x=0., tmp_y=0., theta=0.;
        int size = curvePoints_dup.size();
        QPointF tmp(0,0);
        for (int i=1; i<size-1; i=i+1)
        {
            tmp_x = (curvePoints_dup.at(i+1).x()-curvePoints_dup.at(i-1).x())/(2*step_width);
            tmp_y = (curvePoints_dup.at(i+1).y()-curvePoints_dup.at(i-1).y())/(2*step_width);
            if (qAbs(tmp_x) < DMIN)
                tmp_x = 0;
            if (qAbs(tmp_y) < DMIN)
                tmp_y = 0;
            if (tmp_x > DMAX)
                tmp_x = DMAX;
            if (tmp_y > DMAX)
                tmp_y = DMAX;
            if (tmp_x < -DMAX)
                tmp_x = -DMAX;
            if (tmp_y < -DMAX)
                tmp_y = -DMAX;
            derivative_x.push_back(tmp_x);
            derivative_y.push_back(tmp_y);
        }
        for (int i=0; i<derivative_x.size(); i++)
        {
            theta = sqrt(derivative_x.at(i)*derivative_x.at(i)
                         +derivative_y.at(i)*derivative_y.at(i));

            //判断分母是否为0
            if (theta < DMIN)
            {
                theta = DMIN;
                tmp_x = -1.0;
                tmp_y = 1.0;
            }
            else
            {
                tmp_x = -derivative_y.at(i)/theta;
                tmp_y = derivative_x.at(i)/theta;
            }

            tmp.rx() = tmp_x;
            tmp.ry() = tmp_y;
            normal_vector.push_back(tmp);
        }
    }
    //验证一阶导数
    QFile data("..\\..\\processData\\output_derivative.txt");
    if (data.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&data);
        for (int i=0; i<derivative_x.size(); i++)
        {
            out<<qSetFieldWidth(6)<<derivative_x.at(i)<<"  "
              <<derivative_y.at(i)<<"  "<<normal_vector[i].rx()<<"  "<<normal_vector[i].ry()<<endl;
        }
    }
    data.close();
}

void Palette::cal2Derivative(QVector<double> &mderivative_x, QVector<double> &mderivative_y)
{
    //先清空二阶导数, 曲率半径
    derivative_x2.clear();
    derivative_y2.clear();
    radCurvity.clear();
    //3点求导
    QVector<double> derivative_x_dup(mderivative_x);
    QVector<double> derivative_y_dup(mderivative_y);
    derivative_x_dup.push_front(mderivative_x.front());
    derivative_x_dup.push_back(mderivative_x.last());
    derivative_y_dup.push_front(mderivative_y.front());
    derivative_y_dup.push_back(mderivative_y.last());
    if (mderivative_x.size() > 1)
    {
        double tmp_x=0., tmp_y=0.;
        int size = derivative_x_dup.size();;
        for (int i=1; i<size-1; i=i+1)
        {
            tmp_x = (derivative_x_dup.at(i+1)-derivative_x_dup.at(i-1))/(2*step_width);
            tmp_y = (derivative_y_dup.at(i+1)-derivative_y_dup.at(i-1))/(2*step_width);
            if (qAbs(tmp_x) < DMIN)
                tmp_x = 0;
            if (qAbs(tmp_y) < DMIN)
                tmp_y = 0;
            if (tmp_x > DMAX)
                tmp_x = DMAX;
            if (tmp_y > DMAX)
                tmp_y = DMAX;
            if (tmp_x < -DMAX)
                tmp_x = -DMAX;
            if (tmp_y < -DMAX)
                tmp_y = -DMAX;
            derivative_x2.push_back(tmp_x);
            derivative_y2.push_back(tmp_y);
        }
    }
    int size = mderivative_x.size();
    double d = 0., num = 0., den = 0.;
    for (int i=0; i<size; i++)
    {
        num = pow((mderivative_x.at(i)*mderivative_x.at(i)
                   + mderivative_y.at(i)*mderivative_y.at(i)), 1.5);
        den = qAbs(mderivative_x.at(i)*derivative_y2.at(i)
                   -derivative_x2.at(i)*mderivative_y.at(i));
        if (den < DMIN)
        {
            den = DMIN;
            d = DMAX;
        }
        else
        {
            d = num / den;
            if (d > DMAX)
                d = DMAX;
        }
        radCurvity.push_back(d);
    }
    //输出曲率半径
    QFile file("..\\..\\processData\\output_radCurvity.txt");
    if (file.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        QTextStream out(&file);
        for (int i=0; i<radCurvity.size(); i++)
        {
            out<<radCurvity.at(i)<<"  "<<curvePoints.at(i).x()<<"  "<<curvePoints.at(i).y()<<endl;
        }
    }
    file.close();
}

void Palette::calOffsetCurve(QVector<QPointF> &mcurvePoints, QVector<QPointF> &mnormal_vector,
                    QVector<double> &mradCurvity, int offdistance)
{
    QPointF tmp(0,0);
    QVector<int> location;       //记录小于曲率半径的首末端点
    QVector<int> targetLocation; //记录裁剪的首末端点
    int offset = offdistance;
    double tmp_x=0., tmp_y=0.;
    int sizet = mnormal_vector.size();
    int j=0;

    //是否裁剪处理
    if (Cutting_curves)
    {
        while (j < sizet-2)
        {
            while (mradCurvity.at(j) > qAbs(offset) && j<sizet-1)
            {
                j++;
            }
            if (j == sizet-1)
            {
                break;
            }
            location.push_back(j);
            while (mradCurvity.at(j) <= qAbs(offset) && j<sizet-2)
            {
                j++;
            }
            location.push_back(j-1);
        }
    }

    //输出小于曲率半径的端点
	if (_access("..\\..\\processData", 0) == -1)
	{
		int flag = _mkdir("..\\..\\processData");
 
		if (flag != 0)
		{
            printf("make failed\n");
		}
	}
    QFile file("..\\..\\processData\\output_badpoint.txt");
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug()<<"打开失败";
    }
    QTextStream out(&file);
    for (int i=0; i<location.size(); i++)
    {
        out<<location.at(i)<<endl;
    }
    file.close();

    //等距线
    for (int i=0; i<sizet; i++)
    {
        tmp_x = mcurvePoints.at(deltaCurve*i).x() + offset * mnormal_vector.at(i).x();
        tmp_y = mcurvePoints.at(deltaCurve*i).y() + offset * mnormal_vector.at(i).y();
        tmp.rx() = tmp_x;
        tmp.ry() = tmp_y;
        offsetPoints.push_back(tmp);
    }

    //是否光顺处理
    if (1)
    {
        /*****暂时放这里*****/
        int begin=30;      /***targetLocation也用到了***/
        int end = 30;
        /*****end*****/

        //将location存放的曲率半径小于偏移距离的点剔除相同点放入targetLocation
        int lastToken = 0;
        for (int i=0; i<location.size(); i++)
        {
            if (i==0)
            {
                targetLocation.push_back(location.at(i));
                lastToken = targetLocation.back();
            }
            else
            {
                if (lastToken == location.at(i))
                {
                    targetLocation.pop_back();
                    lastToken = targetLocation.back();
                }
                else
                {
                    targetLocation.push_back(location.at(i));
                    lastToken = targetLocation.back();
                }
            }
        }

        //将targetLocation中不产生自交的点删除
        double con;
        QVector<int> tmp_target(targetLocation); //记录裁剪的首末端点
        targetLocation.clear();
        for (int i=0; i<tmp_target.size(); i++)
        {
            con = calConvexHull(curvePoints, tmp_target.at(i));
            if (offset > 0 && con > 0)
            {
                targetLocation.push_back(tmp_target.at(i));
            }
            else if (offset < 0 && con < 0)
            {
                targetLocation.push_back(tmp_target.at(i));
            }
        }

        //光滑过渡,存储光顺线段
        targetLocation.push_front(0);
        targetLocation.push_back(sizet);

        //输出光顺前的偏移曲线
        QFile file("..\\..\\processData\\output_preSmoothoffset.txt");
        if (file.open(QIODevice::Text | QIODevice::WriteOnly))
        {
            QTextStream out(&file);
            for (int i=0; i<offsetPoints.size(); i++)
            {
                out<<offsetPoints.at(i).x()<<" "<<offsetPoints.at(i).y()<<endl;
            }
        }
        file.close();

        //光顺处理
        if (Smoothing_of_equidistant_curves)
        {
            if (targetLocation.size() > 2 )
                calOffsetDerivative(offsetPoints, targetLocation);
            backuponCrossPoint(targetLocation, offsetPoints);

            //输出需要裁剪的端点
            QFile file("..\\..\\processData\\output_cutpoint.txt");
            if (file.open(QIODevice::Text | QIODevice::WriteOnly))
            {
                QTextStream out(&file);
                for (int i=0; i<targetLocation.size(); i++)
                {
                    out<<targetLocation.at(i)<<endl;
                }
            }
            file.close();

            QVector<QPointF> tmp_offsetPoints; //现将构造的光顺曲线临时放置
            int index = 0;
            int im;
            for (im=0; im<targetLocation.size()-2; im=im+2)
            {
                if (im==0)
                {
                    for (int ij=targetLocation.at(im); ij<targetLocation.at(im+1)-begin; ij++)
                    {
                        tmp_offsetPoints.push_back(offsetPoints.at(ij));
                    }
                }
                else
                {
                    for (int ij=targetLocation.at(im)+end; ij<targetLocation.at(im+1)-begin; ij++)
                    {
                        tmp_offsetPoints.push_back(offsetPoints.at(ij));
                    }
                }
                for (int ik=0; ik<db_fairingPoints.at(index).size(); ik++)
                {
                    tmp_offsetPoints.push_back(db_fairingPoints.at(index).at(ik));
                }
                index++;
            }
            for (int ij=targetLocation.at(im)+end; ij<targetLocation.at(im+1); ij++)
            {
                tmp_offsetPoints.push_back(offsetPoints.at(ij));
            }
            //将tmp_offsetPoints赋值给offsetPoints
            offsetPoints.clear();
            for (int i=0; i<tmp_offsetPoints.size(); i++)
            {
                offsetPoints.push_back(tmp_offsetPoints.at(i));
            }
        }
        else
        {
            backuponCrossPoint(targetLocation, offsetPoints);
            QVector<QPointF> tmp_offsetPoints; //现将构造的光顺曲线临时放置
            int im;
            for (im=0; im<targetLocation.size()-1; im=im+2)
            {
                for (int ij=targetLocation.at(im); ij<targetLocation.at(im+1); ij++)
                {
                    tmp_offsetPoints.push_back(offsetPoints.at(ij));
                }
            }

            //将tmp_offsetPoints赋值给offsetPoints
            offsetPoints.clear();
            for (int i=0; i<tmp_offsetPoints.size(); i++)
            {
                offsetPoints.push_back(tmp_offsetPoints.at(i));
            }
        }
        //计算偏差
        calBias(dis_CrossPoints, db_fairingPoints);
    }
}

void Palette::calOffsetDerivative(const QVector<QPointF> &moffsetPoints,
                                  const QVector<int> &mtargetLocation, int begin, int end)
{
    //曲线光顺
    QPointF bP0, bP1, bP2, bP3, bP4, bP5;
    double k1_x,k1_y;                        //p0p1的斜率
    double k2_x,k2_y;                        //p5p4的斜率
    double ratio_a=0.06, ratio_b=0.06;       //系数
    double derivate1_1x, derivate1_2x, derivate1_3x;
    double derivate1_1y, derivate1_2y, derivate1_3y;
    double derivate2_1x, derivate2_1y;
    //构造BezierCurve(fairing)
    for (int ii=0; ii<mtargetLocation.size()-2; ii=ii+2)
    {
        //清空
        fairingPoints.clear();

        int i1 = mtargetLocation.at(ii+1)-begin;
        int i2 = mtargetLocation.at(ii+2)+end;
        if (i1 < 0)
            i1 = 0;
        if (i2 > moffsetPoints.size()-3)
            i2 = moffsetPoints.size()-3;
        bP0 = moffsetPoints.at(i1);
        bP5 = moffsetPoints.at(i2);

        //p0p1p2
        derivate1_1x = -(moffsetPoints.at(i1-2).x()-moffsetPoints.at(i1).x())/(2*step_width);
        derivate1_1y = -(moffsetPoints.at(i1-2).y()-moffsetPoints.at(i1).y())/(2*step_width);
        derivate1_2x = -(moffsetPoints.at(i1-1).x()-moffsetPoints.at(i1+1).x())/(2*step_width);
        derivate1_2y = -(moffsetPoints.at(i1-1).y()-moffsetPoints.at(i1+1).y())/(2*step_width);
        derivate1_3x = -(moffsetPoints.at(i1).x()-moffsetPoints.at(i1+2).x())/(2*step_width);
        derivate1_3y = -(moffsetPoints.at(i1).y()-moffsetPoints.at(i1+2).y())/(2*step_width);
        derivate2_1x = -(derivate1_1x-derivate1_3x)/(2*step_width);
        derivate2_1y = -(derivate1_1y-derivate1_3y)/(2*step_width);
        k1_x = derivate1_2x;
        k1_y = derivate1_2y;
        bP1.rx() = bP0.x() + ratio_a*k1_x;
        bP1.ry() = bP0.y() + ratio_b*k1_y;
        bP2.rx() = derivate2_1x * ratio_a*ratio_a/20 + 2*bP1.x() - bP0.x();
        bP2.ry() = derivate2_1y * ratio_b*ratio_b/20 + 2*bP1.y() - bP0.y();

        //p3p4p5
        derivate1_1x = -(moffsetPoints.at(i2-2).x()-moffsetPoints.at(i2).x())/(2*step_width);
        derivate1_1y = -(moffsetPoints.at(i2-2).y()-moffsetPoints.at(i2).y())/(2*step_width);
        derivate1_2x = -(moffsetPoints.at(i2-1).x()-moffsetPoints.at(i2+1).x())/(2*step_width);
        derivate1_2y = -(moffsetPoints.at(i2-1).y()-moffsetPoints.at(i2+1).y())/(2*step_width);
        derivate1_3x = -(moffsetPoints.at(i2).x()-moffsetPoints.at(i2+2).x())/(2*step_width);
        derivate1_3y = -(moffsetPoints.at(i2).y()-moffsetPoints.at(i2+2).y())/(2*step_width);
        derivate2_1x = -(derivate1_1x-derivate1_3x)/(2*step_width);
        derivate2_1y = -(derivate1_1y-derivate1_3y)/(2*step_width);
        k2_x = derivate1_2x;
        k2_y = derivate1_2y;
        bP4.rx() = bP5.x() - ratio_a*k2_x;
        bP4.ry() = bP5.y() - ratio_b*k2_y;
        bP3.rx() = derivate2_1x * ratio_a*ratio_a/20 + 2*bP4.x() - bP5.x();
        bP3.ry() = derivate2_1y * ratio_b*ratio_b/20 + 2*bP4.y() - bP5.y();
        QPointF tmp;
        for (double t=0; t<=1; t=t+0.01)
        {
            tmp.rx() = pow(1-t, 5)*bP0.x() + 5*t*pow(1-t, 4)*bP1.x() + 10*pow(t, 2)*pow(1-t, 3)*bP2.x()
                    + 10*pow(t, 3)*pow(1-t, 2)*bP3.x() + 5*pow(t, 4)*(1-t)*bP4.x() + pow(t, 5)*bP5.x();
            tmp.ry() = pow(1-t, 5)*bP0.y() + 5*t*pow(1-t, 4)*bP1.y() + 10*pow(t, 2)*pow(1-t, 3)*bP2.y()
                    + 10*pow(t, 3)*pow(1-t, 2)*bP3.y() + 5*pow(t, 4)*(1-t)*bP4.y() + pow(t, 5)*bP5.y();
            fairingPoints.push_back(tmp);
        }
        db_fairingPoints.push_back(fairingPoints);
    }
    //输出光顺曲线
    QFile file("..\\..\\processData\\output_fairingPoints.txt");
    if (file.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        QTextStream out(&file);
        for (int i=0; i<db_fairingPoints.size(); i++)
        {
            for (int j=0; j<db_fairingPoints.at(i).size(); j++)
                out<<db_fairingPoints.at(i).at(j).x()<<" "<<db_fairingPoints.at(i).at(j).y()<<endl;
            out<<endl;
        }
    }
    file.close();
}

double Palette::calConvexHull(QVector<QPointF> &mcurvePoints, const int &index)
{
    if (index < 1)
        return 0;
    QPointF p0, p1, p2;
    double denom1, denom2;
    double result = 0;
    p0 = mcurvePoints.at(index-1);
    p1 = mcurvePoints.at(index);
    p2 = mcurvePoints.at(index+1);
    denom1 = p2.x()-p1.x();
    denom2 = p1.y()-p0.y();
    if ((fabs(denom1) > 1e-5) && (fabs(denom2) > 1e-5))
        result = (p1.x()-p0.x())*(p2.y()-p1.y()) - denom1*denom2;
    return result;
}

void Palette::backuponCrossPoint(QVector<int> &mtargetLocation, const QVector<QPointF> &moffsetPoints)
{
    //
    QFile file("..\\..\\processData\\output_dis.txt");
    if (!file.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        qDebug()<<"output_dis.txt cannot open!";
    }
    QTextStream out(&file);
    if (!Smoothing_of_equidistant_curves)
    {
        int m_size = mtargetLocation.size();
        QVector<int> tmp_targetLocation;
        tmp_targetLocation.push_back(mtargetLocation.front());
        double delta_x=0., delta_y=0., dis;
        for (int k=1; k<m_size-1; k=k+2)
        {
            bool flag = true;
            for (int j=0, jl=mtargetLocation.at(k);
                 mtargetLocation.at(k-1)<jl && j<1/step_width; j++, jl--) //左端点
            {
                for (int i=0, ir=mtargetLocation.at(k+1);
                     ir<mtargetLocation.at(k+2) && i<1/step_width; i++, ir++) //右端点
                {
                    delta_x = moffsetPoints[jl].x() - moffsetPoints[ir].x();
                    delta_y = moffsetPoints[jl].y() - moffsetPoints[ir].y();
                    dis = sqrt(pow(delta_x, 2) + pow(delta_y, 2));
                    if (dis < 0.7)
                    {
                        tmp_targetLocation.push_back(jl);
                        tmp_targetLocation.push_back(ir);
                        flag = false;
                        QPointF tmp;
                        tmp.rx() = moffsetPoints.at(jl).x();
                        tmp.ry() = moffsetPoints.at(jl).y();
                        dis_CrossPoints.push_back(tmp);
                        out<<dis<<"  "<<jl<<"( "<<tmp.x()<<" , "<<tmp.y()<<") "
                          <<ir<<"( "<<moffsetPoints.at(ir).x()<<" , "<<moffsetPoints.at(ir).y()<<") "<<endl;
                        break;
                    }
                }
                if (!flag)
                    break;
            }
        }
        tmp_targetLocation.push_back(mtargetLocation.back());
        mtargetLocation.clear();
        for (int i=0; i<tmp_targetLocation.size(); i++)
        {
            mtargetLocation.push_back(tmp_targetLocation.at(i));
        }
    }
    else
    {
        int m_size = mtargetLocation.size();
        double delta_x=0., delta_y=0., dis;
        for (int k=1; k<m_size-1; k=k+2)
        {
            bool flag = true;
            for (int j=0, jl=mtargetLocation.at(k);
                 mtargetLocation.at(k-1)<jl && j<1/step_width; j++, jl--) //左端点
            {
                for (int i=0, ir=mtargetLocation.at(k+1);
                     ir<mtargetLocation.at(k+2) && i<1/step_width; i++, ir++) //右端点
                {
                    delta_x = moffsetPoints[jl].x() - moffsetPoints[ir].x();
                    delta_y = moffsetPoints[jl].y() - moffsetPoints[ir].y();
                    dis = sqrt(pow(delta_x, 2) + pow(delta_y, 2));
                    if (dis < 0.7)
                    {
                        flag = false;
                        QPointF tmp;
                        tmp.rx() = moffsetPoints.at(jl).x();
                        tmp.ry() = moffsetPoints.at(jl).y();
                        dis_CrossPoints.push_back(tmp);
                        out<<dis<<"  "<<jl<<"( "<<tmp.x()<<" , "<<tmp.y()<<") "
                          <<ir<<"( "<<moffsetPoints.at(ir).x()<<" , "<<moffsetPoints.at(ir).y()<<") "<<endl;
                        break;
                    }
                }
                if (!flag)
                    break;
            }
        }
    }
    file.close();
}

void Palette::calBias(QVector<QPointF> &mdis_CrossPoints,
                      QVector<QVector<QPointF> > &mdb_fairingPoints)
{
    double resultBias;
    double minBias = DMAX;
    QFile file("..\\..\\processData\\output_bias.txt");
    if (file.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        QTextStream out(&file);
        for (int i=0; i<mdb_fairingPoints.size(); i++)
        {
            QPointF crossPt = mdis_CrossPoints.at(i);
            for (int j=0; j<mdb_fairingPoints.at(i).size(); j++)
            {
                resultBias = sqrt(pow((mdb_fairingPoints[i][j].x()-crossPt.x()),2)
                                  + pow((mdb_fairingPoints[i][j].y()-crossPt.y()),2));
                if (resultBias < minBias)
                {
                    minBias = resultBias;
                }
                out<<resultBias<<endl;
            }
        }
        out<<"MIN Bias: "<<minBias<<endl;
    }
    file.close();
}

void Palette::initFairing()
{
    //曲线光顺
    QPointF bP0, bP1, bP2, bP3, bP4, bP5;
    double k1_x,k1_y;                        //p0p1的斜率
    double k2_x,k2_y;                        //p5p4的斜率
    double ratio_a=0.06, ratio_b=0.06;       //系数
    double derivate1_1x, derivate1_2x, derivate1_3x;
    double derivate1_1y, derivate1_2y, derivate1_3y;
    double derivate2_1x, derivate2_1y;

    //清空
    fairingPoints.clear();
    //第30个点和倒数第30个点
    int nn = 80;
    int i1 = curvePoints.size()-nn;
    int i2 = nn-1;
    bP0 = curvePoints.at(i1);
    bP5 = curvePoints.at(i2);

    //p0p1p2
    derivate1_1x = -(curvePoints.at(i1-2).x()-curvePoints.at(i1).x())/(2*step_width);
    derivate1_1y = -(curvePoints.at(i1-2).y()-curvePoints.at(i1).y())/(2*step_width);
    derivate1_2x = -(curvePoints.at(i1-1).x()-curvePoints.at(i1+1).x())/(2*step_width);
    derivate1_2y = -(curvePoints.at(i1-1).y()-curvePoints.at(i1+1).y())/(2*step_width);
    derivate1_3x = -(curvePoints.at(i1).x()-curvePoints.at(i1+2).x())/(2*step_width);
    derivate1_3y = -(curvePoints.at(i1).y()-curvePoints.at(i1+2).y())/(2*step_width);
    derivate2_1x = -(derivate1_1x-derivate1_3x)/(2*step_width);
    derivate2_1y = -(derivate1_1y-derivate1_3y)/(2*step_width);
    k1_x = derivate1_2x;
    k1_y = derivate1_2y;
    bP1.rx() = bP0.x() + ratio_a*k1_x;
    bP1.ry() = bP0.y() + ratio_b*k1_y;
    bP2.rx() = derivate2_1x * ratio_a*ratio_a/20 + 2*bP1.x() - bP0.x();
    bP2.ry() = derivate2_1y * ratio_b*ratio_b/20 + 2*bP1.y() - bP0.y();

    //p3p4p5
    derivate1_1x = -(curvePoints.at(i2-2).x()-curvePoints.at(i2).x())/(2*step_width);
    derivate1_1y = -(curvePoints.at(i2-2).y()-curvePoints.at(i2).y())/(2*step_width);
    derivate1_2x = -(curvePoints.at(i2-1).x()-curvePoints.at(i2+1).x())/(2*step_width);
    derivate1_2y = -(curvePoints.at(i2-1).y()-curvePoints.at(i2+1).y())/(2*step_width);
    derivate1_3x = -(curvePoints.at(i2).x()-curvePoints.at(i2+2).x())/(2*step_width);
    derivate1_3y = -(curvePoints.at(i2).y()-curvePoints.at(i2+2).y())/(2*step_width);
    derivate2_1x = -(derivate1_1x-derivate1_3x)/(2*step_width);
    derivate2_1y = -(derivate1_1y-derivate1_3y)/(2*step_width);
    k2_x = derivate1_2x;
    k2_y = derivate1_2y;
    bP4.rx() = bP5.x() - ratio_a*k2_x;
    bP4.ry() = bP5.y() - ratio_b*k2_y;
    bP3.rx() = derivate2_1x * ratio_a*ratio_a/20 + 2*bP4.x() - bP5.x();
    bP3.ry() = derivate2_1y * ratio_b*ratio_b/20 + 2*bP4.y() - bP5.y();
    QPointF tmp;
    for (double t=0; t<=1; t=t+0.01)
    {
        tmp.rx() = pow(1-t, 5)*bP0.x() + 5*t*pow(1-t, 4)*bP1.x() + 10*pow(t, 2)*pow(1-t, 3)*bP2.x()
                + 10*pow(t, 3)*pow(1-t, 2)*bP3.x() + 5*pow(t, 4)*(1-t)*bP4.x() + pow(t, 5)*bP5.x();
        tmp.ry() = pow(1-t, 5)*bP0.y() + 5*t*pow(1-t, 4)*bP1.y() + 10*pow(t, 2)*pow(1-t, 3)*bP2.y()
                + 10*pow(t, 3)*pow(1-t, 2)*bP3.y() + 5*pow(t, 4)*(1-t)*bP4.y() + pow(t, 5)*bP5.y();
        fairingPoints.push_back(tmp);
    }

    //重新构造curvePoints
    QVector<QPointF> tmp_curvePoints; //初始化的光顺曲线临时放置
    for (int i=nn; i<curvePoints.size()-nn; i++)
    {
        tmp_curvePoints.push_back(curvePoints.at(i));
    }
    for (int i=0; i<fairingPoints.size(); i++)
    {
        tmp_curvePoints.push_back(fairingPoints.at(i));
    }
    //将tmp_curvePoints赋值给curvePoints
    curvePoints.clear();
    for (int i=0; i<tmp_curvePoints.size(); i++)
    {
        curvePoints.push_back(tmp_curvePoints.at(i));
    }
    //输出小于曲率半径的端点
    QFile file("..\\..\\processData\\output_mcurvePoints.txt");
    if (file.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        QTextStream out(&file);
        for (int i=0; i<curvePoints.size(); i++)
        {
            out<<curvePoints.at(i).x()<<" "<<curvePoints.at(i).y()<<endl;
        }
    }
    file.close();
}
