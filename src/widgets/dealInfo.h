/* dealInfo.h
 * Deal information widget.
 */

#ifndef DEAL_INFO_H
#define DEAL_INFO_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QDate>
#include <QDateEdit>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QString>

class DealInfo : public QWidget {
    Q_OBJECT
public:
    DealInfo();
    ~DealInfo();

    // deal notional
    QString notional();
    QString currency();
    QDate effectiveDate();
    QDate maturityDate();
    bool changeFirstExerciseDate();
    QDate firstExerciseDate();

private slots:
    void checkFirstExerciseDate(int state);

private:
    // set of labels
    QLabel *labelNotional_;
    QLabel *labelCurrency_;
    QLabel *labelEffectiveDate_;
    QLabel *labelMaturityDate_;
    QCheckBox *checkFirstExerciseDate_;

    QLineEdit *notional_;
    QComboBox *currency_;
    QDateEdit *effectiveDate_;
    QDateEdit *maturityDate_;
    QDateEdit *firstExerciseDate_;

    // layout
    QGridLayout *layout;
};

#endif
