#pragma once

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>
#include "trackersettings.h"
#include "expo_curve_preview.h"

// Separate view model: all changes remain local until OK is accepted.
// MainWindow synchronizes its existing range bars before calling updateFromUI().
class ExpoDialog : public QDialog {
public:
    explicit ExpoDialog(TrackerSettings &settings, QWidget *parent=nullptr)
        : QDialog(parent)
    {
        setObjectName("expoDialog");
        setWindowTitle(tr("Expo / Curves"));
        setMinimumSize(850,480);
        resize(960,530);

        original[0] = {settings.getPanExpo(),settings.getPanDZLow(),
            settings.getPanDZHigh(),settings.getPan_Cnt(),
            settings.getPan_Min(),settings.getPan_Max()};
        original[1] = {settings.getTltExpo(),settings.getTltDZLow(),
            settings.getTltDZHigh(),settings.getTlt_Cnt(),
            settings.getTlt_Min(),settings.getTlt_Max()};
        original[2] = {settings.getRllExpo(),settings.getRllDZLow(),
            settings.getRllDZHigh(),settings.getRll_Cnt(),
            settings.getRll_Min(),settings.getRll_Max()};
        for (int i=0;i<3;++i) drafts[i]=original[i];

        auto *root=new QVBoxLayout(this);
        auto *body=new QHBoxLayout();
        root->addLayout(body,1);
        auto *controls=new QGroupBox(tr("Curve settings"),this);
        controls->setMinimumWidth(235);
        controls->setMaximumWidth(280);
        auto *form=new QFormLayout(controls);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        axis=new QComboBox(controls);
        axis->setObjectName("curveAxis");
        axis->addItems({tr("Pan"),tr("Tilt"),tr("Roll")});
        form->addRow(tr("Axis"),axis);

        expo=new QSpinBox(controls);
        expo->setObjectName("expoPercent");
        expo->setRange(0,100);
        expo->setSuffix(" %");
        form->addRow(tr("Expo"),expo);

        deadLow=new QSpinBox(controls);
        deadLow->setObjectName("deadzoneLeft");
        deadLow->setRange(0,250);
        deadLow->setSuffix(" us");
        form->addRow(tr("Deadzone Left"),deadLow);

        deadHigh=new QSpinBox(controls);
        deadHigh->setObjectName("deadzoneRight");
        deadHigh->setRange(0,250);
        deadHigh->setSuffix(" us");
        form->addRow(tr("Deadzone Right"),deadHigh);

        center=new QSpinBox(controls);
        center->setObjectName("curveCenter");
        center->setSuffix(" us");
        center->setRange(TrackerSettings::MIN_CNT,TrackerSettings::MAX_CNT);
        form->addRow(tr("Center"),center);

        minimum=new QSpinBox(controls);
        minimum->setObjectName("curveMin");
        minimum->setRange(TrackerSettings::MIN_PWM,
                          TrackerSettings::MAX_PWM-TrackerSettings::MINMAX_RNG);
        minimum->setSuffix(" us");
        form->addRow(tr("Min"),minimum);

        maximum=new QSpinBox(controls);
        maximum->setObjectName("curveMax");
        maximum->setRange(TrackerSettings::MIN_PWM+TrackerSettings::MINMAX_RNG,
                          TrackerSettings::MAX_PWM);
        maximum->setSuffix(" us");
        form->addRow(tr("Max"),maximum);

        auto *resetAxis=new QPushButton(tr("Restore Axis Values"),controls);
        resetAxis->setObjectName("resetAxis");
        form->addRow(resetAxis);
        auto *hint=new QLabel(tr("Changes are previewed immediately.\n"
            "Click OK, then Save Settings in the main window."),controls);
        hint->setWordWrap(true);
        form->addRow(hint);
        body->addWidget(controls,0);

        auto *chartBox=new QGroupBox(tr("Output vs. Head Input"),this);
        auto *chartLayout=new QVBoxLayout(chartBox);
        chartTitle=new QLabel(chartBox);
        chartTitle->setObjectName("curveGraphTitle");
        chartTitle->setAlignment(Qt::AlignCenter);
        chartLayout->addWidget(chartTitle);
        preview=new ExpoCurvePreview(chartBox);
        chartLayout->addWidget(preview,1);
        chartLayout->addWidget(new QLabel(
            tr("Blue: Expo + deadzone   ·   Gray dashed: linear reference   ·   "
               "Amber: deadzone"),chartBox));
        body->addWidget(chartBox,1);

        auto *bottom=new QHBoxLayout();
        root->addLayout(bottom);
        bottom->addWidget(new QLabel(tr("Live mathematical preview — not sensor telemetry."),this));
        bottom->addStretch();
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Ok |
                                           QDialogButtonBox::Cancel,this);
        buttons->setObjectName("expoButtons");
        bottom->addWidget(buttons);

        connect(buttons,&QDialogButtonBox::accepted,this,&QDialog::accept);
        connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
        connect(axis,qOverload<int>(&QComboBox::currentIndexChanged),
                this,[this](int index){ selectAxis(index); });
        for (QSpinBox *spin : {expo,deadLow,deadHigh,center,minimum,maximum}) {
            connect(spin,qOverload<int>(&QSpinBox::valueChanged),this,
                    [this](int){ changed(); });
        }
        connect(resetAxis,&QPushButton::clicked,this,[this](){
            drafts[selected]=original[selected];
            loadFields();
        });
        loadFields();
    }

    bool applyTo(TrackerSettings &settings) const {
        if (result()!=QDialog::Accepted) return false;
        // Do not touch unsupported settings. The main window is responsible
        // for synchronizing its independent range bars on dialog acceptance.
        if (settings.supportsExpo()) {
            settings.setPanExpo(static_cast<uint8_t>(drafts[0].expo));
            settings.setTltExpo(static_cast<uint8_t>(drafts[1].expo));
            settings.setRllExpo(static_cast<uint8_t>(drafts[2].expo));
        }
        if (settings.supportsDeadzone()) {
            settings.setPanDZLow(static_cast<uint16_t>(drafts[0].deadLow));
            settings.setPanDZHigh(static_cast<uint16_t>(drafts[0].deadHigh));
            settings.setTltDZLow(static_cast<uint16_t>(drafts[1].deadLow));
            settings.setTltDZHigh(static_cast<uint16_t>(drafts[1].deadHigh));
            settings.setRllDZLow(static_cast<uint16_t>(drafts[2].deadLow));
            settings.setRllDZHigh(static_cast<uint16_t>(drafts[2].deadHigh));
        }
        settings.setPan_Min(static_cast<uint16_t>(drafts[0].minimum));
        settings.setPan_Max(static_cast<uint16_t>(drafts[0].maximum));
        settings.setPan_Cnt(static_cast<uint16_t>(drafts[0].center));
        settings.setTlt_Min(static_cast<uint16_t>(drafts[1].minimum));
        settings.setTlt_Max(static_cast<uint16_t>(drafts[1].maximum));
        settings.setTlt_Cnt(static_cast<uint16_t>(drafts[1].center));
        settings.setRll_Min(static_cast<uint16_t>(drafts[2].minimum));
        settings.setRll_Max(static_cast<uint16_t>(drafts[2].maximum));
        settings.setRll_Cnt(static_cast<uint16_t>(drafts[2].center));
        return true;
    }

private:
    struct AxisDraft { int expo,deadLow,deadHigh,center,minimum,maximum; };
    AxisDraft original[3],drafts[3];
    QComboBox *axis=nullptr;
    QSpinBox *expo=nullptr,*deadLow=nullptr,*deadHigh=nullptr,
             *center=nullptr,*minimum=nullptr,*maximum=nullptr;
    ExpoCurvePreview *preview=nullptr;
    QLabel *chartTitle=nullptr;
    int selected=0;
    bool loading=false;

    void selectAxis(int index) {
        if (index<0||index>2||loading) return;
        selected=index;
        loadFields();
    }
    void loadFields() {
        loading=true;
        const AxisDraft &d=drafts[selected];
        expo->setValue(d.expo);
        minimum->setValue(d.minimum);
        maximum->setValue(d.maximum);
        center->setValue(d.center);
        deadLow->setValue(d.deadLow);
        deadHigh->setValue(d.deadHigh);
        loading=false;
        changed();
    }
    void changed() {
        if (loading) return;
        loading=true;
        AxisDraft &d=drafts[selected];
        d.expo=expo->value();
        d.minimum=minimum->value();
        d.maximum=maximum->value();

        // Keep fields consistent with the existing main-range widget.
        const int centerMin=qMax(d.minimum+1,int(TrackerSettings::MIN_CNT));
        const int centerMax=qMin(d.maximum-1,int(TrackerSettings::MAX_CNT));
        center->setRange(centerMin,qMax(centerMin,centerMax));
        d.center=center->value();

        deadLow->setMaximum(qMax(0,qMin(250,d.center-d.minimum-1)));
        deadHigh->setMaximum(qMax(0,qMin(250,d.maximum-d.center-1)));
        d.deadLow=deadLow->value();
        d.deadHigh=deadHigh->value();
        loading=false;
        chartTitle->setText(tr("%1 — Expo: %2%  ·  Deadzone: -%3 / +%4 us")
             .arg(axis->currentText()).arg(d.expo).arg(d.deadLow).arg(d.deadHigh));
        preview->setCurve(d.expo,static_cast<uint16_t>(d.center),
            static_cast<uint16_t>(d.minimum),static_cast<uint16_t>(d.maximum),
            static_cast<uint16_t>(d.deadLow),static_cast<uint16_t>(d.deadHigh));
    }
};
