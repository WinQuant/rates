#include <QAction>
#include <QFileDialog>
#include <QString>
#include <QMenuBar>
#include <QMenu>
#include <QTableWidgetItem>

#include <OpenXLSX/OpenXLSX.h>

#include "widgets/mainWindow.h"

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
                    value_[i - 1][j - 1] = sheet.Cell(i + 1, j + 1).Value().Get<double>();
                }
            }
        }
    }
    
    // load curve
    XLWorksheet curves = workbook.Worksheet("Curve");
    rowCount = curves.RowCount();
    readColumn<std::string>(curves, CURVE_TERM_IDX, rowCount, term_);
    readColumn<double>(curves, CURVE_MARKET_RATE_IDX, rowCount, marketRate_);
    // readColumn<double>(curves, CURVE_SHIFT_IDX, rowCount, shift_);
    readColumn<double>(curves, CURVE_SHIFTED_RATE_IDX, rowCount, shiftedRate_);
    readColumn<double>(curves, CURVE_ZERO_RATE_IDX, rowCount, zeroRate_);
    readColumn<double>(curves, CURVE_DISCOUNT_FACTOR_IDX, rowCount, discount_);

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
