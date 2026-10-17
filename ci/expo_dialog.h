#pragma once
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QVBoxLayout>
#include "trackersettings.h"
#include "expo_curve_preview.h"

// All values are provisional until OK; Cancel leaves TrackerSettings unchanged.
// The graph is a live *prediction* using the firmware's actual curve function.
// No extra settings or flashing is required compared with Deadzone v2.40.
class ExpoDialog : public QDialog {
public:
    explicit ExpoDialog(TrackerSettings &settings, QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setObjectName("expoDialog");
        setWindowTitle(tr("Courbes Expo - Pan / Tilt / Roll"));
        setMinimumWidth(570);
        auto *form = new QFormLayout(this);
        const char *names[] = {"panExpo", "tiltExpo", "rollExpo"};
        const QString labels[] = {tr("Pan (horizontal)"),tr("Tilt (vertical)"),
                                  tr("Roll (inclinaison)")};
        const int values[] = {settings.getPanExpo(),settings.getTltExpo(),
                              settings.getRllExpo()};
        curves[0] = {settings.getPan_Cnt(), settings.getPan_Min(),settings.getPan_Max(),
                     settings.getPanDZLow(),settings.getPanDZHigh()};
        curves[1] = {settings.getTlt_Cnt(), settings.getTlt_Min(),settings.getTlt_Max(),
                     settings.getTltDZLow(),settings.getTltDZHigh()};
        curves[2] = {settings.getRll_Cnt(), settings.getRll_Min(),settings.getRll_Max(),
                     settings.getRllDZLow(),settings.getRllDZHigh()};

        for (int i = 0; i < 3; ++i) {
            spins[i] = new QSpinBox(this);
            spins[i]->setObjectName(names[i]);
            spins[i]->setRange(0, 100);
            spins[i]->setSuffix(" %");
            spins[i]->setSingleStep(1);
            spins[i]->setValue(values[i]);
            form->addRow(labels[i], spins[i]);
            connect(spins[i], qOverload<int>(&QSpinBox::valueChanged),
                    this, [this](int) { updatePreview(); });
        }
        auto *panel = new QWidget(this);
        auto *panelLayout = new QVBoxLayout(panel);
        panelLayout->setContentsMargins(0,0,0,0);
        axis = new QComboBox(panel);
        axis->setObjectName("curveAxis");
        axis->addItems({tr("Pan"),tr("Tilt"),tr("Roll")});
        panelLayout->addWidget(axis);
        preview = new ExpoCurvePreview(panel);
        panelLayout->addWidget(preview);
        description = new QLabel(panel);
        description->setObjectName("curveDescription");
        description->setWordWrap(true);
        panelLayout->addWidget(description);
        form->addRow(tr("Apercu de la courbe"),panel);
        connect(axis,qOverload<int>(&QComboBox::currentIndexChanged),
                this,[this](int){ updatePreview(); });
        auto *help = new QLabel(
            tr("Trait bleu : courbe reelle (Expo + zone morte). "
               "Tirets gris : mouvement lineaire. "
               "Zone ambre : aucun mouvement.\n"
               "Les changements dans cette fenetre sont temporaires jusqu'a OK ; "
               "cliquer ensuite Save Settings pour les memoriser."),this);
        help->setWordWrap(true);
        form->addRow(help);
        auto *buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,this);
        form->addRow(buttons);
        connect(buttons,&QDialogButtonBox::accepted,this,&QDialog::accept);
        connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
        updatePreview();
    }
    bool applyTo(TrackerSettings &settings) const {
        if (result() != QDialog::Accepted) return false;
        settings.setPanExpo(static_cast<uint8_t>(spins[0]->value()));
        settings.setTltExpo(static_cast<uint8_t>(spins[1]->value()));
        settings.setRllExpo(static_cast<uint8_t>(spins[2]->value()));
        return true;
    }
    void updatePreview() {
        if (!axis || !preview) return;
        const int i = axis->currentIndex();
        if (i<0 || i>2) return;
        const Curve &c=curves[i];
        preview->setCurve(spins[i]->value(),c.center,c.min,c.max,c.deadLow,c.deadHigh);
        description->setText(
            tr("Expo : %1 % | Zone morte : -%2 / +%3 us | Centre : %4 us | Butées : %5-%6 us")
               .arg(spins[i]->value()).arg(c.deadLow).arg(c.deadHigh)
               .arg(c.center).arg(c.min).arg(c.max));
    }
private:
    struct Curve { uint16_t center=1500,min=1050,max=1950,deadLow=0,deadHigh=0; };
    Curve curves[3];
    QSpinBox *spins[3]={nullptr,nullptr,nullptr};
    QComboBox *axis=nullptr;
    ExpoCurvePreview *preview=nullptr;
    QLabel *description=nullptr;
};
