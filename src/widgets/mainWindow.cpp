#include <QAction>
#include <QFileDialog>
#include <QString>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QTableWidgetItem>

#include <OpenXLSX/OpenXLSX.h>

#include "widgets/mainWindow.h"
#include "model/bermudanSwaption.h"

#include <ql/time/timeunit.hpp>

#include <iostream>

using namespace OpenXLSX;

#define FORWARD_CURVE_TERM_IDX 0
#define FORWARD_CURVE_UNIT_IDX 1
#define FORWARD_CURVE_BID_IDX  8
#define FORWARD_CURVE_ASK_IDX  9
#define OIS_CURVE_TERM_IDX  0
#define OIS_CURVE_UNIT_IDX  1
#define OIS_CURVE_VALUE_IDX 13

TimeUnit getTimeUnit(std::string tu) {
    TimeUnit t = Years;
    if (tu == "DY") {
        t = Days;
    } else if (tu == "WK") {
        t = Weeks;
    } else if (tu == "MO") {
        t = Months;
    }

    return t;
}

RatesMainWindow::~RatesMainWindow() {}

void RatesMainWindow::setupMenu() {
    QMenuBar *menuBar = this->menuBar();
    QAction *openAction = new QAction(QString::fromUtf8("打开数据文件"), this);
    connect(openAction, SIGNAL(triggered()), this, SLOT(openBbg()));

    QMenu *fileMenu = menuBar->addMenu(QString::fromUtf8("文件"));
    fileMenu->addAction(openAction);
}

void RatesMainWindow::setVolTableWidget(QTableWidget *volTable) {
    volTable_ = volTable;
}

std::vector<std::string> RatesMainWindow::getRowIndex() {
    return volRowIndex_;
}

std::vector<std::string> RatesMainWindow::getColIndex() {
    return volColIndex_;
}

std::vector<std::vector<double>> RatesMainWindow::getValue() {
    return vol_;
}

void RatesMainWindow::getOisQuoteData(std::vector<Period> &oisTenors,
            std::vector<double> &oisRates) {
    for (int i = 0; i < oisTerm_.size(); i++) {
        Period p(oisTerm_[i], getTimeUnit(oisUnit_[i]));
        oisTenors.push_back(p);
        oisRates.push_back(oisValue_[i]);
    }
}

void getForwardQuoteData(Period &depositTenor, double &depositRate,
                         std::vector<Date> &futuresMats,
                         std::vector<double> &futuresPrices,
                         std::vector<Period> &swapTenors,
                         std::vector<double> &swapQuotes) {
    for (int i = 0; i < forwardTerm_.size(); i++) {
        double value = 0.5 * (forwardBid_[i] + forwardAsk_[i]);
        if (i == 0) {
            depositTenor = Period(forwardTerm_[i], getTimeUnit(forwardUnit_[i]));
            depositRate = value;
        } else if (forwardUnit_[i] == "ACTDATE") {
            int dnum = forwardTerm_[i];
            Date d(dnum % 100, (dum % 10000) / 100, dum / 10000);
            futuresMats.push_back(d);
            futuresPrices.push_back(value);
        } else {
            Period p(forwardTerm_[i], forwardUnit_[i]);
            swapTenors.push_back(p);
            swapQuotes.push_back(value);
        }
    }
}

template<typename T> void readColumn(
        XLWorksheet &sheet, int columnIndex, int rowCount, std::vector<T> &data) {
    // empty current storage
    data.clear();

    // skip header
    for (int i = 1; i < rowCount; i++) {
        std::cout << i << " "
                  << columnIndex << " "
                  << sheet.Cell(i + 1, columnIndex + 1).Value().Get<T>()
                  << std::endl;
        data.push_back(sheet.Cell(i + 1, columnIndex + 1).Value().Get<T>());
    }
}

void RatesMainWindow::openBbg() {
    // select a file
    QString filename = QFileDialog::getOpenFileName(
            this, QString::fromUtf8("打开文件"), "doc", "Excel (*.xlsx)");
    XLDocument doc(filename.toUtf8().constData());
    XLWorkbook workbook = doc.Workbook();
    // need the VolSurface sheet
    XLWorksheet sheet = workbook.Worksheet("VolSurface");

    int rowCount = sheet.RowCount();
    int colCount = sheet.ColumnCount();

    // re-construct vector
    volRowIndex_.clear();
    volColIndex_.clear();
    vol_.clear();

    for (int i = 0; i < rowCount - 1; i++)
        vol_.push_back(std::vector<double>(colCount - 1));

    // the first column is the row index and the first row is the col index
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < colCount; j++) {
            if ((i > 0) || (j > 0)) {
                if ((i == 0) && (j > 0)) {
                    volColIndex_.push_back(
                            sheet.Cell(i + 1, j + 1).Value().Get<std::string>());
                } else if (( i > 0) && (j == 0)) {
                    volRowIndex_.push_back(
                            sheet.Cell(i + 1, j + 1).Value().Get<std::string>());
                } else {
                    try {
                        vol_[i - 1][j - 1] = sheet.Cell(i + 1, j + 1).Value().Get<double>();
                    } catch ( OpenXLSX::XLException e ) {
                        vol_[i - 1][j - 1] = sheet.Cell(i + 1, j + 1).Value().Get<int>();
                    }
                    std::cout << i << " " << j << std::endl;
                    std::cout << vol_[i - 1][j - 1] << std::endl;
                }
            }
        }
    }
    
    // load curve
    XLWorksheet forwards = workbook.Worksheet("Forward");
    rowCount = forwards.RowCount();
    std::cout << "Forward row count " << rowCount << std::endl;
    readColumn<int>(forwards, FORWARD_CURVE_TERM_IDX, rowCount, forwardTerm_);
    readColumn<std::string>(forwards, FORWARD_CURVE_UNIT_IDX, rowCount, forwardUnit_);
    readColumn<double>(forwards, FORWARD_CURVE_BID_IDX, rowCount, forwardBid_);
    readColumn<double>(forwards, FORWARD_CURVE_ASK_IDX, rowCount, forwardAsk_);

    // ois curve
    XLWorksheet ois = workbook.Worksheet("OIS");
    rowCount = ois.RowCount();
    readColumn<int>(ois, OIS_CURVE_TERM_IDX, rowCount, oisTerm_);
    readColumn<std::string>(ois, OIS_CURVE_UNIT_IDX, rowCount, oisUnit_);
    readColumn<double>(ois, OIS_CURVE_VALUE_IDX, rowCount, oisValue_);

    std::cout << "Read file done." << std::endl;
    // notify the vol table value changes
    updateVolTable();
}

void RatesMainWindow::updateVolTable() {
    // erase old data
    volTable_->setRowCount(0);
    volTable_->setColumnCount(0);

    volTable_->setRowCount(volRowIndex_.size());
    volTable_->setColumnCount(volColIndex_.size());

    // update new contents

    for (unsigned long i = 0; i < volColIndex_.size(); i++) {
        volTable_->setHorizontalHeaderItem(i,
                new QTableWidgetItem(
                        QString::fromUtf8(volColIndex_[i].c_str())));
    }
    for (unsigned long i = 0; i < volRowIndex_.size(); i++) {
        volTable_->setVerticalHeaderItem(i,
                new QTableWidgetItem(
                    QString::fromUtf8(volRowIndex_[i].c_str())));
    }

    for (unsigned long i = 0; i < volRowIndex_.size(); i++) {
        for (unsigned long j = 0; j < volColIndex_.size(); j++) {
            volTable_->setItem(i, j, new QTableWidgetItem(
                        QString::number(vol_[i][j], 'g', 4)));
        }
    }
}

void RatesMainWindow::setDealInfoWidget(DealInfo *dealInfo) {
    dealInfo_ = dealInfo;
}

void RatesMainWindow::setFixedLegSpecWidget(FixedLegSpec *fixedLegSpec) {
    fixedLegSpec_ = fixedLegSpec;
}

void RatesMainWindow::setFloatLegSpecWidget(FloatLegSpec *floatLegSpec) {
    floatLegSpec_ = floatLegSpec;
}

void RatesMainWindow::setOptionalityWidget(Optionality *optionality) {
    optionality_ = optionality;
}

void RatesMainWindow::setModelInfoWidget(ModelInfo *modelInfo) {
    modelInfo_ = modelInfo;
}

void RatesMainWindow::calculate() {
    std::cout << "In calculating..." << std::endl;
    // collect necessary parameters
    // deal related parameters
    double notional = dealInfo_->notional().toDouble();
    QString currency = dealInfo_->currency();
    std::string effectiveDate = dealInfo_->effectiveDate().toString(QString::fromUtf8("yyyy/MM/dd")).toUtf8().constData();
    std::string maturityDate  = dealInfo_->maturityDate().toString(QString::fromUtf8("yyyy/MM/dd")).toUtf8().constData();
    bool changeFirstExerciseDate = dealInfo_->changeFirstExerciseDate();
    std::string firstExerciseDate  = dealInfo_->firstExerciseDate().toString(QString::fromUtf8("yyyy/MM/dd")).toUtf8().constData();

    // fix leg related information
    QString fixedDirection = fixedLegSpec_->direction();
    double fixedCoupon = fixedLegSpec_->coupon().toDouble();
    QString fixedPayFreq = fixedLegSpec_->payFreq();
    std::string fixedDayCounter = fixedLegSpec_->dayCounter().toUtf8().constData();

    // float leg related information
    QString floatDirection = floatLegSpec_->direction();
    QString floatIndex = floatLegSpec_->index();
    QString floatPayFreq = floatLegSpec_->payFreq();
    std::string floatDayCounter = floatLegSpec_->dayCounter().toUtf8().constData();

    // optionality
    QString style = optionality_->style();
    QString position = optionality_->position();
    QString callFreq = optionality_->callFreq();

    // model
    std::string pricingDate = modelInfo_->pricingDate().toString(QString::fromUtf8("yyyy/MM/dd")).toUtf8().constData();
    QString model = modelInfo_->model();
    QString engine = modelInfo_->engine();
    QString complexity = modelInfo_->complexity();
    QString curve = modelInfo_->curve();

    bool useExternalVolSurface = modelInfo_->isExternalVolSurface();

    if (!useExternalVolSurface || vol_.size() > 0) {
        double price = priceSwaption(notional,
                currency, effectiveDate, maturityDate, changeFirstExerciseDate, firstExerciseDate,
                fixedDirection, fixedCoupon, fixedPayFreq, fixedDayCounter,
                floatDirection, floatIndex, floatPayFreq, floatDayCounter,
                style, position, callFreq,
                pricingDate, model, engine, complexity, curve,
                useExternalVolSurface, vol_);
        modelInfo_->setPrice(price / notional, price);
    } else {
        // no vol surface loaded, alert.
        QMessageBox::critical(this, QString::fromUtf8("未加载波动率曲面"),
                    QString::fromUtf8("请先加载波动率曲面！"),
                               QMessageBox::Ok);
    }
}
