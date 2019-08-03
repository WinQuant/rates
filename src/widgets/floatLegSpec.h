/*
 * Float let specification.
 */

#ifndef FLOAT_LEG_SPEC_H
#define FLOAT_LEG_SPEC_H

#include <QWidget>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QString>

class FloatLegSpec : public QWidget {
public:
    FloatLegSpec();
    ~FloatLegSpec();

    QString direction();
    QString index();
    QString payFreq();
    QString dayCounter();
private:
    // set of labels
    QLabel *labelDirection_;
    QLabel *labelIndex_;
    QLabel *labelSpread_;
    QLabel *labelLeverage_;
    QLabel *labelPayFreq_;
    QLabel *labelDayCounter_;

    QComboBox *direction_;
    QComboBox *index_;
    QLabel *spread_;
    QLabel *leverage_;
    QComboBox *payFreq_;
    QComboBox *dayCounter_;

    // layout
    QGridLayout *layout;
};

#endif
