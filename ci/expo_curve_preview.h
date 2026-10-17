#pragma once
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QtGlobal>
#include <QRectF>
#include <QString>
#include <cmath>
#include "expo_deadzone.h"

// Live preview uses precisely the same output function as the firmware.
// Input = signed delta in microseconds after gain/inversion;
// Output = absolute microsecond channel value after Expo + deadzone.
class ExpoCurvePreview : public QWidget {
public:
    explicit ExpoCurvePreview(QWidget *parent=nullptr) : QWidget(parent) {
        setObjectName("expoCurvePreview");
        setMinimumSize(460,260);
        setToolTip(tr("Trait bleu : courbe Expo + zone morte. Tirets gris : reponse lineaire. "
                      "Bande ambre : zone morte."));
    }
    void setCurve(int expo, uint16_t center, uint16_t minv, uint16_t maxv,
                  uint16_t deadLeft, uint16_t deadRight) {
        percentage = qBound(0,expo,100);
        mid=center; low=minv; high=maxv;
        dzLeft=deadLeft; dzRight=deadRight;
        update();
    }
    uint16_t outputFor(float delta) const {
        return expoDeadzoneChannel(delta,mid,low,high,
                                  static_cast<uint8_t>(percentage),dzLeft,dzRight);
    }
    int currentExpo() const { return percentage; }
    uint16_t currentDeadLeft() const { return dzLeft; }
    uint16_t currentDeadRight() const { return dzRight; }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(),palette().base().color());
        const QRectF plot(65.0,18.0,qMax(140,width()-90),qMax(110,height()-72));
        p.setPen(QPen(palette().mid().color(),1));
        p.drawRect(plot);
        if (low >= high || mid < low || mid > high) return;
        const double leftSpan=double(mid)-low, rightSpan=double(high)-mid;
        const double maxSpan=double(high)-low;
        auto xpos=[&](double x) {return plot.left() + (x+1.0)*0.5*plot.width();};
        auto ypos=[&](double out) {return plot.bottom()-(out-low)/maxSpan*plot.height();};
        const double negativeDz=leftSpan>1.0 ? qMin(double(dzLeft),leftSpan-1.0)/leftSpan : 0;
        const double positiveDz=rightSpan>1.0 ? qMin(double(dzRight),rightSpan-1.0)/rightSpan : 0;
        // Shaded zero-output band; visibly asymmetric if the two sides differ.
        const QRectF deadRect(xpos(-negativeDz),plot.top(),
                              (negativeDz+positiveDz)*plot.width()/2.0,plot.height());
        p.fillRect(deadRect,QColor(235,167,43,52));
        p.setPen(QPen(palette().mid().color(),1,Qt::DotLine));
        p.drawLine(QPointF(xpos(0),plot.top()),QPointF(xpos(0),plot.bottom()));
        p.drawLine(QPointF(plot.left(),ypos(mid)),QPointF(plot.right(),ypos(mid)));

        // Straight line is what the outputs would do without Expo or deadzone.
        QPainterPath linear,actual;
        for (int i=0;i<=300;i++) {
            const double normalized=-1.0+2.0*i/300.0;
            const float delta=static_cast<float>(
                normalized < 0.0 ? normalized*leftSpan : normalized*rightSpan);
            const QPointF original(xpos(normalized),
                ypos(expoChannel(delta,mid,low,high,0)));
            const QPointF real(xpos(normalized),ypos(outputFor(delta)));
            if (!i) {linear.moveTo(original);actual.moveTo(real);}
            else {linear.lineTo(original);actual.lineTo(real);}
        }
        p.setPen(QPen(QColor(145,145,145),2,Qt::DashLine)); p.drawPath(linear);
        p.setPen(QPen(QColor(20,123,211),3));p.drawPath(actual);
        p.setPen(palette().text().color());
        p.drawText(QRectF(2,plot.top()-8,61,22),Qt::AlignRight,tr("Max"));
        p.drawText(QRectF(2,ypos(mid)-10,61,22),Qt::AlignRight,tr("Centre"));
        p.drawText(QRectF(2,plot.bottom()-12,61,22),Qt::AlignRight,tr("Min"));
        p.drawText(QRectF(plot.left(),plot.bottom()+6,65,22),Qt::AlignLeft,tr("-100 %"));
        p.drawText(QRectF(xpos(0)-45,plot.bottom()+6,90,22),Qt::AlignCenter,tr("Centre"));
        p.drawText(QRectF(plot.right()-80,plot.bottom()+6,80,22),Qt::AlignRight,tr("+100 %"));
        p.drawText(QRectF(plot.left(),plot.bottom()+27,plot.width(),18),
                   Qt::AlignCenter,tr("Mouvement de la tete (apres gain)"));
    }
private:
    int percentage=0;
    uint16_t mid=1500,low=1050,high=1950,dzLeft=0,dzRight=0;
};
