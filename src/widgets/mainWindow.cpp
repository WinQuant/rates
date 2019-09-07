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

#include <iostream>

using namespace OpenXLSX;

#define CURVE_TERM_IDX 0
#define CURVE_MARKET_RATE_IDX 1
#define CURVE_SHIFT_IDX 2
#define CURVE_SHIFTED_RATE_IDX 3
#define CURVE_ZERO_RATE_IDX 4
#define CURVE_DISCOUNT_FACTOR_IDX 5

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
    return rowIndex_;
}

std::vector<std::string> RatesMainWindow::getColIndex() {
    return colIndex_;
}

std::vector<std::vector<double>> RatesMainWindow::getValue() {
    return value_;
}

template<typename T> void readColumn(
        XLWorksheet &sheet, int columnIndex, int rowCount, std::vector<T> &data) {
    // empty current storage
    data.clear();

    // skip header
    for (int i = 1; i < rowCount; i++) {
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
    rowIndex_.clear();
    colIndex_.clear();
    value_.clear();

    for (int i = 0; i < rowCount - 1; i++)
        value_.push_back(std::vector<double>(colCount - 1));

    // the first column is the row index and the first row is the col index
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < colCount; j++) {
            if ((i > 0) || (j > 0)) {
                if ((i == 0) && (j > 0)) {
                    colIndex_.push_back(
                            sheet.Cell(i + 1, j + 1).Value().Get<std::string>());
                } else if (( i > 0) && (j == 0)) {
                    rowIndex_.push_back(
                            sheet.Cell(i + 1, j + 1).Value().Get<std::string>());
                } else {
                    try {
                        value_[i - 1][j - 1] = sheet.Cell(i + 1, j + 1).Value().Get<double>();
                    } catch ( OpenXLSX::XLException e ) {
                        value_[i - 1][j - 1] = sheet.Cell(i + 1, j + 1).Value().Get<int>();
                    }
                    std::cout << i << " " << j << std::endl;
                    std::cout << value_[i - 1][j - 1] << std::endl;
                }
            }
        }
    }
    
    /*
    // load curve
    XLWorksheet curves = workbook.Worksheet("Curve");
    rowCount = curves.RowCount();
    readColumn<std::string>(curves, CURVE_TERM_IDX, rowCount, term_);
    readColumn<double>(curves, CURVE_MARKET_RATE_IDX, rowCount, marketRate_);
    // readColumn<double>(curves, CURVE_SHIFT_IDX, rowCount, shift_);
    readColumn<double>(curves, CURVE_SHIFTED_RATE_IDX, rowCount, shiftedRate_);
    readColumn<double>(curves, CURVE_ZERO_RATE_IDX, rowCount, zeroRate_);
    readColumn<double>(curves, CURVE_DISCOUNT_FACTOR_IDX, rowCount, discount_);
    */
    std::cout << "Read file done." << std::endl;
    // notify the vol table value changes
    updateVolTable();
}

void RatesMainWindow::updateVolTable() {
    // erase old data
    volTable_->setRowCount(0);
    volTable_->setColumnCount(0);

    volTable_->setRowCount(rowIndex_.size());
    volTable_->setColumnCount(colIndex_.size());

    // update new contents

    for (unsigned long i = 0; i < colIndex_.size(); i++) {
        volTable_->setHorizontalHeaderItem(i,
                new QTableWidgetItem(
                        QString::fromUtf8(colIndex_[i].c_str())));
    }
    for (unsigned long i = 0; i < rowIndex_.size(); i++) {
        volTable_->setVerticalHeaderItem(i,
                new QTableWidgetItem(
                    QString::fromUtf8(rowIndex_[i].c_str())));
    }

    for (unsigned long i = 0; i < rowIndex_.size(); i++) {
        for (unsigned long j = 0; j < colIndex_.size(); j++) {
            volTable_->setItem(i, j, new QTableWidgetItem(
                        QString::number(value_[i][j], 'g', 4)));
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

    if (!useExternalVolSurface || value_.size() > 0) {
        double price = priceSwaption(notional,
                currency, effectiveDate, maturityDate,
                fixedDirection, fixedCoupon, fixedPayFreq, fixedDayCounter,
                floatDirection, floatIndex, floatPayFreq, floatDayCounter,
                style, position, callFreq,
                pricingDate, model, engine, complexity, curve,
                useExternalVolSurface, value_);
        modelInfo_->setPrice(price / notional, price);
    } else {
        // no vol surface loaded, alert.
        QMessageBox::critical(this, QString::fromUtf8("未加载波动率曲面"),
                    QString::fromUtf8("请先加载波动率曲面！"),
                               QMessageBox::Ok);
    }
}
