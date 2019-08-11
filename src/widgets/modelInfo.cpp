/*
 * Model info.
 */

#include "widgets/modelInfo.h"

ModelInfo::ModelInfo() : QWidget() {
    labelToday_ = new QLabel(QString::fromUtf8("计价日期"));
    labelModel_ = new QLabel(QString::fromUtf8("短期利率模型"));
    labelEngine_ = new QLabel(QString::fromUtf8("定价模型"));
    labelPricePerc_ = new QLabel(QString::fromUtf8("价格(%)"));
    labelPrice_ = new QLabel(QString::fromUtf8("价格"));


    QDate today = QDate::currentDate();
    today_ = new QDateEdit();
    today_->setDate(today);

    model_ = new QComboBox();
    engine_ = new QComboBox();
    pricePerc_ = new QLabel();
    price_ = new QLabel();

    // supported short-rate model
    model_->addItem(QString::fromUtf8("Hull-White One Factor"));

    // default engine
    engine_->addItem(QString::fromUtf8("有限差分(FD)"));
    engine_->addItem(QString::fromUtf8("Black方法"));

    // layout
    layout = new QGridLayout(this);

    layout->addWidget(labelToday_, 1, 1, Qt::AlignLeft);
    layout->addWidget(today_, 1, 2, Qt::AlignLeft);

    layout->addWidget(labelModel_, 2, 1, Qt::AlignLeft);
    layout->addWidget(model_, 2, 2, Qt::AlignLeft);

    layout->addWidget(labelPricePerc_, 2, 3, Qt::AlignLeft);
    layout->addWidget(pricePerc_, 2, 4, Qt::AlignLeft);

    layout->addWidget(labelEngine_, 3, 1, Qt::AlignLeft);
    layout->addWidget(engine_, 3, 2, Qt::AlignLeft);

    layout->addWidget(labelPrice_, 3, 3, Qt::AlignLeft);
    layout->addWidget(price_, 3, 4, Qt::AlignLeft);

    this->setLayout(layout);
}

ModelInfo::~ModelInfo() {
    delete labelToday_;
    delete labelModel_;
    delete labelEngine_;
    delete labelPricePerc_;
    delete labelPrice_;

    delete today_;
    delete model_;
    delete engine_;
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

void ModelInfo::setPrice(double price) {
    pricePerc_->setText(QString::number(price / 1000000));
    price_->setText(QString::number(price));
}
