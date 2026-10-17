#pragma once
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QRectF>
#include <QString>
#include <QtGlobal>
#include "expo_deadzone.h"

// True mathematical response from firmware. No fake telemetry.
class ExpoCurvePreview : public QWidget {
public:
    explicit ExpoCurvePreview(QWidget *parent=nullptr) : QWidget(parent) {
        setObjectName("expoCurvePreview");
        setMinimumSize(390,275);
        setToolTip(tr("Blue: Expo with deadzone. Gray dashed: linear. "
                      "Amber: center deadzone."));
    }
    void setCurve(int expo, uint16_t center, uint16_t minv, uint16_t maxv,
                  uint16_t deadLeft, uint16_t deadRight) {
        percentage=qBound(0,expo,100);
        mid=center;low=minv;high=maxv;
        dzLeft=deadLeft; dzRight=deadRight;
        update();
    }
    uint16_t outputFor(float delta) const {
        return expoDeadzoneChannel(delta,mid,low,high,
             static_cast<uint8_t>(percentage),dzLeft,dzRight);
    }
    int currentExpo() const {return percentage;}
    uint16_t currentDeadLeft() const {return dzLeft;}
    uint16_t currentDeadRight() const {return dzRight;}

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(),palette().base().color());
        const QRectF plot(62,16,qMax(140,width()-86),qMax(120,height()-70));
        p.setPen(QPen(palette().mid().color(),1));
        p.drawRect(plot);
        if (low>=high || mid<low || mid>high) return;
        const double span=high-low;
        auto px=[&](double input){return plot.left()+((input-low)/span)*plot.width();};
        auto py=[&](double value){return plot.bottom()-((value-low)/span)*plot.height();};

        p.setPen(QPen(QColor(210,210,210),1,Qt::DotLine));
        for (int i=1;i<5;i++) {
            const double x=plot.left()+i*plot.width()/5.0;
            const double y=plot.top()+i*plot.height()/5.0;
            p.drawLine(QPointF(x,plot.top()),QPointF(x,plot.bottom()));
            p.drawLine(QPointF(plot.left(),y),QPointF(plot.right(),y));
        }
        const double left=mid-qMin(double(dzLeft),double(mid-low-1));
        const double right=mid+qMin(double(dzRight),double(high-mid-1));
        p.fillRect(QRectF(px(left),plot.top(),px(right)-px(left),plot.height()),
                   QColor(244,174,54,64));
        p.setPen(QPen(QColor(214,121,28),1.5,Qt::DashLine));
        p.drawLine(QPointF(px(left),plot.top()),QPointF(px(left),plot.bottom()));
        p.drawLine(QPointF(px(right),plot.top()),QPointF(px(right),plot.bottom()));
        p.setPen(QPen(QColor(191,67,58),1,Qt::DashLine));
        p.drawLine(QPointF(px(mid),plot.top()),QPointF(px(mid),plot.bottom()));
        p.drawLine(QPointF(plot.left(),py(mid)),QPointF(plot.right(),py(mid)));

        QPainterPath linear,actual;
        for (int i=0;i<=360;i++) {
            const double input=low+span*i/360.0;
            const float delta=static_cast<float>(input-mid);
            const QPointF ref(px(input),
                py(expoChannel(delta,mid,low,high,0)));
            const QPointF real(px(input),py(outputFor(delta)));
            if (!i){linear.moveTo(ref);actual.moveTo(real);}
            else {linear.lineTo(ref);actual.lineTo(real);}
        }
        p.setPen(QPen(QColor(149,149,149),2,Qt::DashLine));
        p.drawPath(linear);
        p.setPen(QPen(QColor(20,100,241),3));
        p.drawPath(actual);
        p.setPen(palette().text().color());
        p.drawText(QRectF(0,plot.top()-9,55,18),Qt::AlignRight,
                   QString::number(high));
        p.drawText(QRectF(0,py(mid)-9,55,18),Qt::AlignRight,
                   QString::number(mid));
        p.drawText(QRectF(0,plot.bottom()-9,55,18),Qt::AlignRight,
                   QString::number(low));
        p.drawText(QRectF(plot.left()-15,plot.bottom()+5,70,19),Qt::AlignLeft,
                   QString::number(low));
        p.drawText(QRectF(px(mid)-45,plot.bottom()+5,90,19),Qt::AlignCenter,
                   QString::number(mid));
        p.drawText(QRectF(plot.right()-65,plot.bottom()+5,72,19),Qt::AlignRight,
                   QString::number(high));
        p.drawText(QRectF(plot.left(),plot.bottom()+26,plot.width(),20),
                   Qt::AlignCenter,tr("Head Input (us)"));
        p.save();
        p.translate(4,plot.center().y());
        p.rotate(-90);
        p.drawText(QRectF(-70,-5,140,16),Qt::AlignCenter,tr("Output (us)"));
        p.restore();
    }
private:
    int percentage=0;
    uint16_t mid=1500,low=1050,high=1950,dzLeft=0,dzRight=0;
};
