/*
 * Fixed leg specification.
 */

#include "widgets/fixedLegSpec.h"

FixedLegSpec::FixedLegSpec() : QWidget() {
    labelDirection_ = new QLabel(QString::fromUtf8("收付款方向"));
    labelCoupon_ = new QLabel(QString::fromUtf8("息费利率"));
    labelPayFreq_ = new QLabel(QString::fromUtf8("付息频率"));
    labelDayCounter_ = new QLabel(QString::fromUtf8("日期计数"));

    direction_ = new QComboBox();
    coupon_ = new QLineEdit();
    payFreq_ = new QComboBox();
    dayCounter_ = new QComboBox();

    // supported direction
    direction_->addItem(QString::fromUtf8("收款(Pay)"));
    direction_->addItem(QString::fromUtf8("付款(Receive)"));

    // default coupon
    coupon_->setText("2.00%");

    // supported pay freq
    payFreq_->addItem(QString::fromUtf8("季度支付(Quarter)"));
    payFreq_->addItem(QString::fromUtf8("半年支付(Semi-annual)"));
    payFreq_->addItem(QString::fromUtf8("年度支付(Annual)"));

    // supported day count
    dayCounter_->addItem("30 / 360");
    dayCounter_->addItem("Act / 360");
    dayCounter_->addItem("Act / Act");

    // layout
    layout = new QGridLayout(this);

    layout->addWidget(labelDirection_, 1, 1, Qt::AlignLeft);
    layout->addWidget(direction_, 1, 2, Qt::AlignLeft);

    layout->addWidget(labelCoupon_, 2, 1, Qt::AlignLeft);
    layout->addWidget(coupon_, 2, 2, Qt::AlignLeft);

    layout->addWidget(labelPayFreq_, 3, 1, Qt::AlignLeft);
    layout->addWidget(payFreq_, 3, 2, Qt::AlignLeft);

    layout->addWidget(labelDayCounter_, 4, 1, Qt::AlignLeft);
    layout->addWidget(dayCounter_, 4, 2, Qt::AlignLeft);

    this->setLayout(layout);
}

FixedLegSpec::~FixedLegSpec() {
    delete labelDirection_;
    delete labelCoupon_;
    delete labelPayFreq_;
    delete labelDayCounter_;

    delete direction_;
    delete coupon_;
    delete payFreq_;
    delete dayCounter_;

    delete layout;
}

QString FixedLegSpec::direction() {
    return direction_->currentText();
}

QString FixedLegSpec::coupon() {
    return coupon_->text();
}

QString FixedLegSpec::payFreq() {
    return payFreq_->currentText();
}

QString FixedLegSpec::dayCounter() {
    return dayCounter_->currentText();
}
