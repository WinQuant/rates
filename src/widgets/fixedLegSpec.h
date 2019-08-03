/*
 * Fixed leg specification.
 */

#ifndef FIXED_LEG_SPEC_H
#define FIXED_LEG_SPEC_H

#include <QWidget>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QString>

class FixedLegSpec : public QWidget {
public:
    FixedLegSpec();
    ~FixedLegSpec();

    QString direction();
    QString coupon();
    QString payFreq();
    QString dayCounter();
private:
    // set of labels
    QLabel *labelDirection_;
    QLabel *labelCoupon_;
    QLabel *labelPayFreq_;
    QLabel *labelDayCounter_;

    QComboBox *direction_;
    QLineEdit *coupon_;
    QComboBox *payFreq_;
    QComboBox *dayCounter_;

    // layout
    QGridLayout *layout;
};

#endif
