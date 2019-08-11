/*
 * Model Info
 */

#ifndef MODEL_INFO_H
#define MODEL_INFO_H

#include <QWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QDate>

class ModelInfo : public QWidget {
public:
    ModelInfo();
    ~ModelInfo();

    QDate pricingDate();
    QString model();
    QString engine();

    void setPrice(double price);
private:
    // set of labels
    QLabel *labelToday_;
    QLabel *labelModel_;
    QLabel *labelEngine_;
    QLabel *labelPricePerc_;
    QLabel *labelPrice_;

    QDateEdit *today_;
    QComboBox *model_;
    QComboBox *engine_;
    QLabel *pricePerc_;
    QLabel *price_;

    // layout
    QGridLayout *layout;
};

#endif

