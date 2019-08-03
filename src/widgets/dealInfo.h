/* dealInfo.h
 * Deal information widget.
 */

#ifndef DEAL_INFO_H
#define DEAL_INFO_H

#include <QWidget>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QString>

class DealInfo : public QWidget {
public:
    DealInfo();
    ~DealInfo();

    // deal notional
    QString notional();
    QString currency();
    QDate effectiveDate();
    QDate maturityDate();
private:
    // set of labels
    QLabel *labelNotional_;
    QLabel *labelCurrency_;
    QLabel *labelEffectiveDate_;
    QLabel *labelMaturityDate_;

    QLineEdit *notional_;
    QComboBox *currency_;
    QDateEdit *effectiveDate_;
    QDateEdit *maturityDate_;

    // layout
    QGridLayout *layout;
};

#endif
