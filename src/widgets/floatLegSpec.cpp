/*
 * Float leg specification.
 */

#include "widgets/floatLegSpec.h"

FloatLegSpec::FloatLegSpec() : QWidget() {
    labelDirection_ = new QLabel(QString::fromUtf8("收付款方向"));
    labelIndex_ = new QLabel(QString::fromUtf8("掉期指数"));
    labelSpread_ = new QLabel(QString::fromUtf8("基差(Spread)"));
    labelLeverage_ = new QLabel(QString::fromUtf8("杠杆率"));
    labelPayFreq_ = new QLabel(QString::fromUtf8("付息频率"));
    labelDayCounter_ = new QLabel(QString::fromUtf8("日期计数"));

    direction_ = new QComboBox();
    index_ = new QComboBox();
    spread_ = new QLabel("0.00 bp");
    leverage_ = new QLabel("1");
    payFreq_ = new QComboBox();
    dayCounter_ = new QComboBox();

    // supported direction
    direction_->addItem(QString::fromUtf8("付款(Pay)"));
    direction_->addItem(QString::fromUtf8("收款(Receive)"));

    // default index
    index_->addItem("US0003M");

    // supported pay freq
    payFreq_->addItem(QString::fromUtf8("季度支付(Quarter)"));
    payFreq_->addItem(QString::fromUtf8("半年支付(Semi-annual)"));
    payFreq_->addItem(QString::fromUtf8("年度支付(Annual)"));

    // supported day count
    dayCounter_->addItem("Act / 360");
    dayCounter_->addItem("30 / 360");
    dayCounter_->addItem("Act / Act");

    // layout
    layout = new QGridLayout(this);

    layout->addWidget(labelDirection_, 1, 1, Qt::AlignLeft);
    layout->addWidget(direction_, 1, 2, Qt::AlignLeft);

    layout->addWidget(labelIndex_, 2, 1, Qt::AlignLeft);
    layout->addWidget(index_, 2, 2, Qt::AlignLeft);

    layout->addWidget(labelSpread_, 3, 1, Qt::AlignLeft);
    layout->addWidget(spread_, 3, 2, Qt::AlignLeft);

    layout->addWidget(labelLeverage_, 4, 1, Qt::AlignLeft);
    layout->addWidget(leverage_, 4, 2, Qt::AlignLeft);

    layout->addWidget(labelPayFreq_, 5, 1, Qt::AlignLeft);
    layout->addWidget(payFreq_, 5, 2, Qt::AlignLeft);

    layout->addWidget(labelDayCounter_, 6, 1, Qt::AlignLeft);
    layout->addWidget(dayCounter_, 6, 2, Qt::AlignLeft);

    this->setLayout(layout);
}

FloatLegSpec::~FloatLegSpec() {
    delete labelDirection_;
    delete labelIndex_;
    delete labelSpread_;
    delete labelLeverage_;
    delete labelPayFreq_;
    delete labelDayCounter_;

    delete direction_;
    delete index_;
    delete spread_;
    delete leverage_;
    delete payFreq_;
    delete dayCounter_;

    delete layout;
}

QString FloatLegSpec::direction() {
    return direction_->currentText();
}

QString FloatLegSpec::index() {
    return index_->currentText();
}

QString FloatLegSpec::payFreq() {
    return payFreq_->currentText();
}

QString FloatLegSpec::dayCounter() {
    return dayCounter_->currentText();
}

