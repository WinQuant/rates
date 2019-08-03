/* dealInfo.cpp
 * Implmentation of the deal info widget.
 */

#include "widgets/dealInfo.h"

DealInfo::DealInfo() : QWidget() {
    labelNotional_ = new QLabel(QString::fromUtf8("名义价格"));
    labelCurrency_ = new QLabel(QString::fromUtf8("计价单位"));
    labelEffectiveDate_ = new QLabel(QString::fromUtf8("生效日"));
    labelMaturityDate_  = new QLabel(QString::fromUtf8("到期日"));

    notional_ = new QLineEdit();
    currency_ = new QComboBox();
    effectiveDate_ = new QDateEdit();
    maturityDate_  = new QDateEdit();

    // default notional
    notional_->setText("10000000");

    // supported currencies
    currency_->addItem("CNY");
    currency_->addItem("USD");

    // set default QDateEdit dates
    // effective date is today and the maturity date is one
    // year later
    QDate today = QDate::currentDate();
    effectiveDate_->setDate(today);
    maturityDate_->setDate(today.addDays(365));

    // layout
    layout = new QGridLayout(this);

    layout->addWidget(labelNotional_, 1, 1, Qt::AlignLeft);
    layout->addWidget(notional_, 1, 2, Qt::AlignLeft);

    layout->addWidget(labelCurrency_, 2, 1, Qt::AlignLeft);
    layout->addWidget(currency_, 2, 2, Qt::AlignLeft);

    layout->addWidget(labelEffectiveDate_, 3, 1, Qt::AlignLeft);
    layout->addWidget(effectiveDate_, 3, 2, Qt::AlignLeft);

    layout->addWidget(labelMaturityDate_, 4, 1, Qt::AlignLeft);
    layout->addWidget(maturityDate_, 4, 2, Qt::AlignLeft);

    this->setLayout(layout);
}

DealInfo::~DealInfo() {
    delete labelNotional_;
    delete labelCurrency_;
    delete labelEffectiveDate_;
    delete labelMaturityDate_;

    delete notional_;
    delete currency_;
    delete effectiveDate_;
    delete maturityDate_;

    delete layout;
}

QString DealInfo::notional() {
    return notional_->text();
}

QString DealInfo::currency() {
    return currency_->currentText();
}

QDate DealInfo::effectiveDate() {
    return effectiveDate_->date();
}

QDate DealInfo::maturityDate() {
    return maturityDate_->date();
}
