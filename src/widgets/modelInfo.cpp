/*
 * Model info.
 */

#include <QLocale>

#include "widgets/modelInfo.h"

ModelInfo::ModelInfo() : QWidget() {
    labelToday_ = new QLabel(QString::fromUtf8("计价日期"));
    labelModel_ = new QLabel(QString::fromUtf8("短期利率模型"));
    labelEngine_ = new QLabel(QString::fromUtf8("定价模型"));
    labelComplexity_ = new QLabel(QString::fromUtf8("Hull-White参数"));
    labelCurves_ = new QLabel(QString::fromUtf8("收益率曲线"));
    labelPricePerc_ = new QLabel(QString::fromUtf8("价格(%)"));
    labelPrice_ = new QLabel(QString::fromUtf8("价格"));


    QDate today = QDate::currentDate();
    today_ = new QDateEdit();
    today_->setDate(today);

    model_ = new QComboBox();
    engine_ = new QComboBox();
    complexity_ = new QComboBox();
    curves_ = new QComboBox();
    pricePerc_ = new QLabel();
    price_ = new QLabel();

    // supported short-rate model
    model_->addItem(QString::fromUtf8("Hull-White One Factor"));

    // default engine
    engine_->addItem(QString::fromUtf8("有限差分(FD)"));
    engine_->addItem(QString::fromUtf8("Black方法"));

    // complexity
    complexity_->addItem(QString::fromUtf8("常函数"));
    complexity_->addItem(QString::fromUtf8("阶梯函数"));

    // curves
    curves_->addItem(QString::fromUtf8("单一曲线"));
    curves_->addItem(QString::fromUtf8("双重曲线"));

    // layout
    layout = new QGridLayout(this);

    layout->addWidget(labelToday_, 1, 1, Qt::AlignLeft);
    layout->addWidget(today_, 1, 2, Qt::AlignLeft);

    layout->addWidget(labelComplexity_, 1, 3, Qt::AlignLeft);
    layout->addWidget(complexity_, 1, 4, Qt::AlignLeft);

    layout->addWidget(labelModel_, 2, 1, Qt::AlignLeft);
    layout->addWidget(model_, 2, 2, Qt::AlignLeft);

    layout->addWidget(labelCurves_, 2, 3, Qt::AlignLeft);
    layout->addWidget(curves_, 2, 4, Qt::AlignLeft);

    layout->addWidget(labelEngine_, 3, 1, Qt::AlignLeft);
    layout->addWidget(engine_, 3, 2, Qt::AlignLeft);

    layout->addWidget(labelPricePerc_, 3, 3, Qt::AlignLeft);
    layout->addWidget(pricePerc_, 3, 4, Qt::AlignLeft);

    layout->addWidget(labelPrice_, 4, 3, Qt::AlignLeft);
    layout->addWidget(price_, 4, 4, Qt::AlignLeft);

    this->setLayout(layout);
}

ModelInfo::~ModelInfo() {
    delete labelToday_;
    delete labelModel_;
    delete labelEngine_;
    delete labelComplexity_;
    delete labelCurves_;
    delete labelPricePerc_;
    delete labelPrice_;

    delete today_;
    delete model_;
    delete engine_;
    delete complexity_;
    delete curves_;
    delete pricePerc_;
    delete price_;

    delete layout;
}

QDate ModelInfo::pricingDate() {
    return today_->date();
}

QString ModelInfo::model() {
    return model_->currentText();
}

QString ModelInfo::engine() {
    return engine_->currentText();
}

QString ModelInfo::complexity() {
    return complexity_->currentText();
}

QString ModelInfo::curve() {
    return curves_->currentText();
}

void ModelInfo::setPrice(double returnRate, double price) {
    QLocale cLocale = QLocale::c();
    pricePerc_->setText(cLocale.toString(returnRate * 100, 'f', 3));
    price_->setText(cLocale.toString(price, 'f', 2));
}
