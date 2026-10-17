#pragma once
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include "trackersettings.h"

// The model is changed only after acceptance; Cancel has no side effects.
class ExpoDialog : public QDialog {
public:
    explicit ExpoDialog(TrackerSettings &settings, QWidget *parent = nullptr)
        : QDialog(parent) {
        setObjectName("expoDialog");
        setWindowTitle(tr("Courbes Expo - Pan / Tilt / Roll"));
        auto *form = new QFormLayout(this);
        const char *names[] = {"panExpo", "tiltExpo", "rollExpo"};
        const QString labels[] = {tr("Pan (horizontal)"), tr("Tilt (vertical)"),
                                  tr("Roll (inclinaison)")};
        const int values[] = {settings.getPanExpo(), settings.getTltExpo(), settings.getRllExpo()};
        for (int i = 0; i < 3; ++i) {
            spins[i] = new QSpinBox(this);
            spins[i]->setObjectName(names[i]);
            spins[i]->setRange(0, 100);
            spins[i]->setSuffix(" %");
            spins[i]->setSingleStep(1);
            spins[i]->setValue(values[i]);
            form->addRow(labels[i], spins[i]);
        }
        auto *help = new QLabel(tr("0 % = lineaire ; 100 % = cubique.\n"
                                   "Apres OK, cliquer Save Settings pour conserver les valeurs."), this);
        form->addRow(help);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        form->addRow(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
    bool applyTo(TrackerSettings &settings) const {
        if (result() != QDialog::Accepted) return false;
        settings.setPanExpo(static_cast<uint8_t>(spins[0]->value()));
        settings.setTltExpo(static_cast<uint8_t>(spins[1]->value()));
        settings.setRllExpo(static_cast<uint8_t>(spins[2]->value()));
        return true;
    }
private:
    QSpinBox *spins[3];
};
