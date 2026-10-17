#pragma once

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QRectF>
#include <QString>
#include <QWidget>
#include <QtGlobal>
#include <cmath>
#include <stdint.h>
#include "expo_deadzone.h"

// Graphical description of the *current settings*, not a second control path.
// Green is sampled using the firmware's exact function; black is a continuous
// hypothetical extension outside the configured output limits, NOT a signal
// commanded to the servo. The physical HeadTracker GUI itself is unchanged.
class ExpoCurvePreview : public QWidget {
public:
    explicit ExpoCurvePreview(QWidget *parent=nullptr) : QWidget(parent) {
        setObjectName("expoCurvePreview");
        setMinimumSize(390,275);
        setToolTip(tr("Green: active Expo curve; black: unused total travel. "
                      "Blue: configured min/max; red: center; amber: deadzone; "
                      "yellow diamond: measured output."));
    }

    void setCurve(int expo,uint16_t center,uint16_t minv,uint16_t maxv,
                  uint16_t deadLeft,uint16_t deadRight) {
        percentage=qBound(0,expo,100);
        mid=center; low=minv; high=maxv;
        dzLeft=deadLeft; dzRight=deadRight;
        update();
    }
    // Only actual data received over the connected HeadTracker serial protocol
    // enables the yellow diamond; it never appears as a fabricated sample.
    void setActualOutput(int output) {
        const int bounded=qBound(TrackerMin,output,TrackerMax);
        if (!hasOutput || actualOutput!=bounded) {
            actualOutput=bounded; hasOutput=true;update();
        }
    }
    void clearActualOutput() {hasOutput=false;update();}
    bool hasActualOutput() const {return hasOutput;}
    int currentActualOutput() const {return actualOutput;}
    int currentExpo() const {return percentage;}
    uint16_t currentDeadLeft() const {return dzLeft;}
    uint16_t currentDeadRight() const {return dzRight;}
    uint16_t outputFor(float delta) const {
        return expoDeadzoneChannel(delta,mid,low,high,
           static_cast<uint8_t>(percentage),dzLeft,dzRight);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(),palette().base().color());
        const QRectF plot(60,16,qMax(140,width()-84),qMax(120,height()-68));
        p.setPen(QPen(palette().mid().color(),1));p.drawRect(plot);
        if (low>=high || mid<=low || mid>=high) return;

        const double total=TrackerMax-TrackerMin;
        const auto px=[&](double in) {
            return plot.left()+(in-TrackerMin)/total*plot.width();
        };
        const auto py=[&](double out) {
            return plot.bottom()-(out-TrackerMin)/total*plot.height();
        };
        p.setClipRect(plot);
        p.setPen(QPen(QColor(218,218,218),1,Qt::DotLine));
        for(int i=1;i<5;++i) {
            const double x=plot.left()+i*plot.width()/5.;
            const double y=plot.top()+i*plot.height()/5.;
            p.drawLine(QPointF(x,plot.top()),QPointF(x,plot.bottom()));
            p.drawLine(QPointF(plot.left(),y),QPointF(plot.right(),y));
        }

        const double dleft=qMin(double(dzLeft),double(mid-low-1));
        const double dright=qMin(double(dzRight),double(high-mid-1));
        const double xleft=mid-dleft, xright=mid+dright;
        p.fillRect(QRectF(px(xleft),plot.top(),
                          qMax(0.0,px(xright)-px(xleft)),plot.height()),
                   QColor(240,171,43,54));

        // Boundaries are deliberately SOLID BLUE, red at center, amber
        // dashed at the deadzone edges. No black dashed reference lines.
        p.setPen(QPen(QColor(20,85,241),2));
        p.drawLine(QPointF(px(low),plot.top()),QPointF(px(low),plot.bottom()));
        p.drawLine(QPointF(px(high),plot.top()),QPointF(px(high),plot.bottom()));
        p.setPen(QPen(QColor(230,38,38),2));
        p.drawLine(QPointF(px(mid),plot.top()),QPointF(px(mid),plot.bottom()));
        p.setPen(QPen(QColor(219,142,20),1.25,Qt::DashLine));
        p.drawLine(QPointF(px(xleft),plot.top()),QPointF(px(xleft),plot.bottom()));
        p.drawLine(QPointF(px(xright),plot.top()),QPointF(px(xright),plot.bottom()));

        // The actual green part precisely matches the firmware, including
        // center plateau, left/right deadzone, expo and active min/max.
        auto drawSegment=[&](double start,double stop,bool used) {
            QPainterPath path;
            for(int i=0;i<=300;++i) {
                const double input=start+(stop-start)*i/300.;
                const double y=used
                    ? outputFor(static_cast<float>(input-mid))
                    : extrapolateUnused(input);
                const QPointF point(px(input),py(y));
                if(i==0) path.moveTo(point);
                else path.lineTo(point);
            }
            return path;
        };
        const QPainterPath green=drawSegment(low,high,true);
        const QPainterPath blackBefore=drawSegment(TrackerMin,low,false);
        const QPainterPath blackAfter=drawSegment(high,TrackerMax,false);
        p.setPen(QPen(Qt::black,2));
        p.drawPath(blackBefore);p.drawPath(blackAfter);
        p.setPen(QPen(QColor(0,139,35),3));
        p.drawPath(green);

        if(hasOutput) {
            // Output telemetry gives Y only, not the raw input angle. Invert
            // the monotonic green curve to locate the corresponding X.
            double input=mid;
            if (actualOutput<low) input=low;
            else if (actualOutput>high) input=high;
            else if (actualOutput!=mid) {
                double a=low,b=high;
                for(int i=0;i<28;++i) {
                    double m=(a+b)/2.;
                    if(outputFor(float(m-mid))<actualOutput) a=m;
                    else b=m;
                }
                input=(a+b)/2.;
            }
            const QPointF point(px(input),py(qBound(double(low),
                                   double(actualOutput),double(high))));
            QPolygonF diamond;
            diamond<<QPointF(point.x(),point.y()-7)
                   <<QPointF(point.x()+7,point.y())
                   <<QPointF(point.x(),point.y()+7)
                   <<QPointF(point.x()-7,point.y());
            p.setPen(QPen(Qt::black,1.5));
            p.setBrush(QColor(255,236,0));
            p.drawPolygon(diamond);
            p.setBrush(Qt::NoBrush);
        }
        p.setClipping(false);
        p.setPen(palette().text().color());
        const auto tick=[&](int value) {
            return QString::number(value);
        };
        p.drawText(QRectF(1,plot.top()-6,53,17),Qt::AlignRight,tick(TrackerMax));
        p.drawText(QRectF(1,py(mid)-8,53,18),Qt::AlignRight,tick(mid));
        p.drawText(QRectF(1,plot.bottom()-12,53,19),Qt::AlignRight,tick(TrackerMin));
        p.drawText(QRectF(plot.left(),plot.bottom()+5,65,20),Qt::AlignLeft,tick(TrackerMin));
        p.drawText(QRectF(px(mid)-45,plot.bottom()+5,90,20),Qt::AlignCenter,tick(mid));
        p.drawText(QRectF(plot.right()-70,plot.bottom()+5,70,20),Qt::AlignRight,tick(TrackerMax));
        p.drawText(QRectF(plot.left(),plot.bottom()+25,plot.width(),20),
                   Qt::AlignCenter,tr("Head Input (us)"));
        p.save();p.translate(2,plot.center().y());p.rotate(-90);
        p.drawText(QRectF(-70,-6,140,16),Qt::AlignCenter,tr("Output (us)"));
        p.restore();
    }

private:
    static constexpr int TrackerMin=988,TrackerMax=2012;
    int percentage=0;
    uint16_t mid=1500,low=1050,high=1950,dzLeft=0,dzRight=0;
    bool hasOutput=false;
    int actualOutput=1500;
    double extrapolateUnused(double input) const {
        // Mathematically continue the selected green curve outside each
        // configured end stop. This black line is illustrative only.
        const bool neg=input<mid;
        const double span=neg?mid-low:high-mid;
        const double dead=qMin(neg?double(dzLeft):double(dzRight),span-1.);
        const double magnitude=std::fabs(input-mid);
        const double normalized=qMax(0.0,magnitude-dead)/(span-dead);
        const double e=percentage/100.;
        const double shaped=normalized*((1.-e)+e*normalized*normalized);
        return mid+(neg?-1.:1.)*span*shaped;
    }
};
