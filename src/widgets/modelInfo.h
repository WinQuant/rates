/*
 * Model Info
 */

#ifndef MODEL_INFO_H
#define MODEL_INFO_H

#include <QWidget>
#include <QCheckBox>
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
    QString complexity();
    QString curve();
    bool isExternalVolSurface();

    void setPrice(double returnRate, double price);
private:
    // set of labels
    QLabel *labelToday_;
    QLabel *labelModel_;
    QLabel *labelEngine_;
    QLabel *labelComplexity_;
    QLabel *labelCurves_;
    QLabel *labelPricePerc_;
    QLabel *labelPrice_;

    QDateEdit *today_;
    QComboBox *model_;
    QComboBox *engine_;
    QComboBox *complexity_;
    QComboBox *curves_;
    QCheckBox *externalVols_;
    QLabel *pricePerc_;
    QLabel *price_;

    // layout
    QGridLayout *layout;
};

#endif

