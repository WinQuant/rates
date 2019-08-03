/*
 * Optionality
 */

#ifndef OPTIONALITY_H
#define OPTIONALITY_H

#include <QWidget>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QString>

class Optionality : public QWidget {
public:
    Optionality();
    ~Optionality();

    QString style();
    QString position();
    QString callFreq();
private:
    // set of labels
    QLabel *labelStyle_;
    QLabel *labelPosition_;
    QLabel *labelCallFreq_;
    QLabel *labelNotificationDays_;
    QLabel *labelBusiDayAdj_;
    QLabel *labelRolling_;

    QComboBox *style_;
    QComboBox *position_;
    QComboBox *callFreq_;
    QLabel *notificationDays_;
    QLabel *busiDayAdj_;
    QLabel *rolling_;

    // layout
    QGridLayout *layout;
};

#endif
