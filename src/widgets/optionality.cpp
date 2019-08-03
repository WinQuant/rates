/*
 * Optionality
 */

#include "widgets/optionality.h"

Optionality::Optionality() : QWidget() {
    labelStyle_ = new QLabel(QString::fromUtf8("行权方式"));
    labelPosition_ = new QLabel(QString::fromUtf8("买卖方向"));
    labelCallFreq_ = new QLabel(QString::fromUtf8("行权周期"));
    labelNotificationDays_ = new QLabel(QString::fromUtf8("通知日期"));
    labelBusiDayAdj_ = new QLabel(QString::fromUtf8("工作日调整"));
    labelRolling_ = new QLabel(QString::fromUtf8("换月约定"));

    style_ = new QComboBox();
    position_ = new QComboBox();
    callFreq_ = new QComboBox();
    notificationDays_ = new QLabel(QString::fromUtf8("2天"));
    busiDayAdj_ = new QLabel(QString::fromUtf8("顺延下一日(ModifiedFollowing)"));
    rolling_ = new QLabel(QString::fromUtf8("月底顺延"));

    // supported style
    style_->addItem(QString::fromUtf8("欧式期权(European)"));
    style_->addItem(QString::fromUtf8("百慕大期权(Bermudan)"));
    style_->addItem(QString::fromUtf8("美式期权(American)"));

    // support position
    position_->addItem(QString::fromUtf8("多头(Long)"));
    position_->addItem(QString::fromUtf8("空头(Short)"));

    // supported call freq
    callFreq_->addItem(QString::fromUtf8("季度支付(Quarter)"));
    callFreq_->addItem(QString::fromUtf8("半年支付(Semi-annual)"));
    callFreq_->addItem(QString::fromUtf8("年度支付(Annual)"));

    // layout
    layout = new QGridLayout(this);

    layout->addWidget(labelStyle_, 1, 1, Qt::AlignLeft);
    layout->addWidget(style_, 1, 2, Qt::AlignLeft);

    layout->addWidget(labelPosition_, 2, 1, Qt::AlignLeft);
    layout->addWidget(position_, 2, 2, Qt::AlignLeft);

    layout->addWidget(labelCallFreq_, 3, 1, Qt::AlignLeft);
    layout->addWidget(callFreq_, 3, 2, Qt::AlignLeft);

    layout->addWidget(labelNotificationDays_, 4, 1, Qt::AlignLeft);
    layout->addWidget(notificationDays_, 4, 2, Qt::AlignLeft);

    layout->addWidget(labelBusiDayAdj_, 5, 1, Qt::AlignLeft);
    layout->addWidget(busiDayAdj_, 5, 2, Qt::AlignLeft);

    layout->addWidget(labelRolling_, 6, 1, Qt::AlignLeft);
    layout->addWidget(rolling_, 6, 2, Qt::AlignLeft);

    this->setLayout(layout);
}

Optionality::~Optionality() {
    delete labelStyle_;
    delete labelPosition_;
    delete labelCallFreq_;
    delete labelNotificationDays_;
    delete labelBusiDayAdj_;
    delete labelRolling_;

    delete style_;
    delete position_;
    delete callFreq_;
    delete notificationDays_;
    delete busiDayAdj_;
    delete rolling_;

    delete layout;
}

QString Optionality::style() {
    return style_->currentText();
}

QString Optionality::position() {
    return position_->currentText();
}

QString Optionality::callFreq() {
    return callFreq_->currentText();
}
