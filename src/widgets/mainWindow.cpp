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

#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/timeunit.hpp>
#include <ql/utilities/dataparsers.hpp>

#include <iostream>
#include <fstream>

using namespace OpenXLSX;

#define FORWARD_CURVE_TERM_IDX 0
#define FORWARD_CURVE_UNIT_IDX 1
#define FORWARD_CURVE_BID_IDX  8
#define FORWARD_CURVE_ASK_IDX  9
#define OIS_CURVE_TERM_IDX  0
#define OIS_CURVE_UNIT_IDX  1
#define OIS_CURVE_BID_IDX   3
#define OIS_CURVE_ASK_IDX   4

Period OIS_TENORS[] = {
    Period( 1, Days ),  Period( 1, Weeks ),   Period( 2, Weeks ),
    Period( 3, Weeks ), Period( 1, Months ),  Period( 2, Months ),
    Period( 3, Months ), Period( 4, Months ), Period( 5, Months ),
    Period( 6, Months ), Period( 9, Months ), Period( 12, Months ),
    Period( 18, Months ), Period( 2, Years ), Period( 3, Years ),
    Period( 4, Years ),  Period( 5, Years ),  Period( 7, Years ),
    Period( 10, Years ), Period( 12, Years ), Period( 15, Years ),
    Period( 20, Years ), Period( 25, Years ), Period( 30, Years ),
    Period( 40, Years ), Period( 50, Years ) };

// forecast forward rate
double OIS_RATES[] = {
    0.0241, 0.023879, 0.023885, 0.022890,
    0.022140, 0.021530, 0.02760, 0.020210, 0.019770, 0.019310, 0.018420, 0.017790,
    0.016760, 0.016200, 0.015820, 0.015840, 0.016040, 0.019211, 0.020390, 0.021035,
    0.021717, 0.022360, 0.022591, 0.022665, 0.022498, 0.022188 };

Period DEPOSIT_TENOR(3, Months);
double DEPOSIT_RATE = 0.0229963;

Date FUTURES_MATURITIES[] = {
    Date(18, September, 2019), Date(18, December, 2019),
    Date(18, March, 2020),     Date(17, June, 2020),
    Date(16, September, 2020), Date(16, December, 2020) };

double FUTURES_PRICES[] = {
    97.92, 97.99, 98.17, 98.255, 98.315, 98.305 };

Period SWAP_TENORS[] = {
    Period( 2, Years ),  Period( 3, Years ),  Period( 4, Years ),
    Period( 5, Years ),  Period( 6, Years ),  Period( 7, Years ),
    Period( 8, Years ),  Period( 9, Years ),  Period( 10, Years ),
    Period( 11, Years ), Period( 12, Years ), Period( 15, Years ),
    Period( 20, Years ), Period( 25, Years ), Period( 30, Years ),
    Period( 40, Years ), Period( 50, Years ) };

double SWAP_QUOTES[] = {
    0.018799, 0.018327, 0.018291, 0.018500, 0.018840, 0.019211,
    0.019608, 0.020005, 0.020390, 0.020730, 0.021035, 0.021717,
    0.022360, 0.022591, 0.022665, 0.022498, 0.022188 };

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

int date2string(char *buffer, Date d) {
    return sprintf(buffer, "%d-%02d-%02d", d.year(), d.month(),
                d.dayOfMonth());
}

RatesMainWindow::~RatesMainWindow() {}

void RatesMainWindow::setupMenu() {
    QMenuBar *menuBar = this->menuBar();
    // open file
    QAction *openAction = new QAction(QString::fromUtf8("打开数据文件"), this);
    connect(openAction, SIGNAL(triggered()), this, SLOT(openBbg()));

    // save OIS
    QAction *saveOisAction = new QAction(QString::fromUtf8("保存OIS曲线"), this);
    connect(saveOisAction, SIGNAL(triggered()), this, SLOT(saveOis()));

    // save forecast
    QAction *saveForecastAction = new QAction(QString::fromUtf8("保存远期曲线"), this);
    connect(saveForecastAction, SIGNAL(triggered()), this, SLOT(saveForecast()));

    QMenu *fileMenu = menuBar->addMenu(QString::fromUtf8("文件"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveOisAction);
    fileMenu->addAction(saveForecastAction);
}

void RatesMainWindow::setVolTableWidget(QTableWidget *volTable) {
    volTable_ = volTable;
}

void RatesMainWindow::setOisTableWidget(QTableWidget *oisTable) {
    oisCurveTable_ = oisTable;
}

void RatesMainWindow::setForwardTableWidget(QTableWidget *forwardTable) {
    forwardCurveTable_ = forwardTable;
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
    for (unsigned int i = 0; i < oisTerm_.size(); i++) {
        Period p(oisTerm_[i], getTimeUnit(oisUnit_[i]));
        oisTenors.push_back(p);
        oisRates.push_back(0.5 * (oisBid_[i] + oisAsk_[i]) * 0.01);
    }
}

void RatesMainWindow::getForwardQuoteData(
            Period &depositTenor, double &depositRate,
            std::vector<Date> &futuresMats,
            std::vector<double> &futuresPrices,
            std::vector<Period> &swapTenors,
            std::vector<double> &swapQuotes) {
    for (unsigned int i = 0; i < forwardTerm_.size(); i++) {
        double value = 0.5 * (forwardBid_[i] + forwardAsk_[i]) / 100;
        if (i == 0) {
            depositTenor = Period(forwardTerm_[i], getTimeUnit(forwardUnit_[i]));
            depositRate = value;
        } else if (forwardUnit_[i] == "ACTDATE") {
            int dnum = forwardTerm_[i];
            Date d(Day(dnum % 100),
                   Month((dnum % 10000) / 100),
                   Year(dnum / 10000));
            futuresMats.push_back(d);
            futuresPrices.push_back(100 - value * 100);
        } else {
            Period p(forwardTerm_[i], getTimeUnit(forwardUnit_[i]));
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
    readColumn<double>(ois, OIS_CURVE_BID_IDX, rowCount, oisBid_);
    readColumn<double>(ois, OIS_CURVE_ASK_IDX, rowCount, oisAsk_);

    std::cout << "Read file done." << std::endl;
    // notify the vol table value changes
    updateVolTable();

    std::cout << "Vol table updated." << std::endl;

    // prepare interest rate curve
    std::vector<Period> oisTenors;
    std::vector<double> oisRates;
    Period depositTenor;
    double depositRate;
    std::vector<Date> futuresMaturities;
    std::vector<double> futuresPrices;
    std::vector<Period> swapTenors;
    std::vector<double> swapQuotes;

    getOisQuoteData(oisTenors, oisRates);
    getForwardQuoteData(depositTenor, depositRate,
                futuresMaturities, futuresPrices,
                swapTenors, swapQuotes);

    std::string today = modelInfo_->pricingDate().toString(QString::fromUtf8("yyyy/MM/dd")).toUtf8().constData();
    Date todaysDate = DateParser::parseFormatted(today, "%Y/%m/%d");
    Calendar calendar = TARGET();
    int settlementDays  = 2;
    Date settlementDate = calendar.advance(todaysDate, settlementDays,
                Days, ModifiedFollowing);
    Settings::instance().evaluationDate() = todaysDate;

    std::cout << todaysDate << " " << settlementDate << std::endl;

    DayCounter fixedLegDayCounter = Thirty360();
    RelinkableHandle<YieldTermStructure> discountTermStructure;
    RelinkableHandle<YieldTermStructure> forecastTermStructure; 
    ext::shared_ptr<IborIndex> liborIndex( new USDLibor( Period(3, Months), forecastTermStructure ) );

    bootstrapIrTermStructure(oisTenors, oisRates,
            depositTenor, depositRate,
            futuresMaturities, futuresPrices,
            swapTenors, swapQuotes,
            settlementDays, calendar, settlementDate,
            fixedLegDayCounter, liborIndex,
            true, true, discountTermStructure, forecastTermStructure);
    std::cout << "IR term structure bootstrapped." << std::endl;

    updateOisTable(settlementDate, calendar,
                oisTenors, discountTermStructure);
    updateForwardTable(settlementDate, calendar,
                depositTenor, futuresMaturities, swapTenors,
                forecastTermStructure);
}

void RatesMainWindow::saveOis() {
    // select a file
    QString filename = QFileDialog::getSaveFileName(
            this, QString::fromUtf8("打开文件"), "doc", "CSV (*.csv)");
    std::ofstream output(filename.toUtf8().constData());
    output << "date,discount factor" << std::endl;
    unsigned int rowCount = oisCurveTable_->rowCount();
    unsigned int colCount = oisCurveTable_->columnCount();
    for (unsigned int i = 0; i < rowCount; i++) {
        for (unsigned int j = 0; j < colCount; j++) {
            output << oisCurveTable_->item(i, j)->text().toUtf8().constData();
            if (j < colCount - 1)
                output << ",";
            else
                output << std::endl;
        }
    }

    output.close();
}

void RatesMainWindow::saveForecast() {
    // select a file
    QString filename = QFileDialog::getSaveFileName(
            this, QString::fromUtf8("打开文件"), "doc", "CSV (*.csv)");
    std::ofstream output(filename.toUtf8().constData());
    output << "date,discount factor" << std::endl;
    unsigned int rowCount = forwardCurveTable_->rowCount();
    unsigned int colCount = forwardCurveTable_->columnCount();
    for (unsigned int i = 0; i < rowCount; i++) {
        for (unsigned int j = 0; j < colCount; j++) {
            output << forwardCurveTable_->item(i, j)->text().toUtf8().constData();
            if (j < colCount - 1)
                output << ",";
            else
                output << std::endl;
        }
    }

    output.close();
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

void RatesMainWindow::updateOisTable(Date startDate, Calendar calendar,
        const std::vector<Period> &oisTerms,
        const RelinkableHandle<YieldTermStructure> &discountTermStructure) {

    // erase old data
    oisCurveTable_->setRowCount(0);
    oisCurveTable_->setColumnCount(0);

    oisCurveTable_->setRowCount(oisTerms.size());
    oisCurveTable_->setColumnCount(2);

    // update new contents
    oisCurveTable_->setHorizontalHeaderItem(0,
            new QTableWidgetItem(
                    QString::fromUtf8("日期")));
    oisCurveTable_->setHorizontalHeaderItem(1,
            new QTableWidgetItem(
                    QString::fromUtf8("贴现率")));

    char buffer[64] = {'\0'};
    for (unsigned int i = 0; i < oisTerms.size(); i++) {
        double disc = 1.0;
        Date d = startDate;
        if (i > 0) {
            d = calendar.advance(startDate, oisTerms[i],
                            ModifiedFollowing);
            disc = discountTermStructure->discount(d);
        }
        date2string(buffer, d);
        oisCurveTable_->setItem(i, 0, new QTableWidgetItem(
                    QString::fromUtf8(buffer)));
        oisCurveTable_->setItem(i, 1, new QTableWidgetItem(
                    QString::number(disc, 'g', 4)));
    }
}

void RatesMainWindow::updateForwardTable(Date startDate,
            Calendar calendar, Period depositTenor,
            const std::vector<Date> &futuresMaturities,
            const std::vector<Period> &swapTenors,
            const RelinkableHandle<YieldTermStructure> &forecastTermStructure) {
    // erase old data
    forwardCurveTable_->setRowCount(0);
    forwardCurveTable_->setColumnCount(0);

    unsigned int futuresLength = futuresMaturities.size();
    unsigned int swapLength = swapTenors.size();
    forwardCurveTable_->setRowCount(2 + futuresLength + swapLength);
    forwardCurveTable_->setColumnCount(2);

    // update new contents
    forwardCurveTable_->setHorizontalHeaderItem(0,
            new QTableWidgetItem(
                    QString::fromUtf8("日期")));
    forwardCurveTable_->setHorizontalHeaderItem(1,
            new QTableWidgetItem(
                    QString::fromUtf8("贴现率")));

    char buffer[64] = {'\0'};
    // start date discount rate
    date2string(buffer, startDate);
    forwardCurveTable_->setItem(0, 0, new QTableWidgetItem(
                    QString::fromUtf8(buffer)));
    forwardCurveTable_->setItem(0, 1, new QTableWidgetItem(
                QString::number(1.0, 'g', 4)));

    // deposit discount rate
    Date d = calendar.advance(startDate, depositTenor,
                    ModifiedFollowing);
    double disc = forecastTermStructure->discount(d);
    date2string(buffer, d);
    forwardCurveTable_->setItem(1, 0, new QTableWidgetItem(
                    QString::fromUtf8(buffer)));
    forwardCurveTable_->setItem(1, 1, new QTableWidgetItem(
                QString::number(disc, 'g', 4)));
    // futures discount rate
    for (unsigned int i = 0; i < futuresLength; i++) {
        date2string(buffer, futuresMaturities[i]);
        forwardCurveTable_->setItem(i + 2, 0, new QTableWidgetItem(
                    QString::fromUtf8(buffer)));
        disc = forecastTermStructure->discount(futuresMaturities[i]);
        forwardCurveTable_->setItem(i + 2, 1, new QTableWidgetItem(
                    QString::number(disc, 'g', 4)));
    }
    // swap discount rate
    for (unsigned int i = 0; i < swapLength; i++) {
        d = calendar.advance(startDate, swapTenors[i],
                    ModifiedFollowing);
        date2string(buffer, d);
        forwardCurveTable_->setItem(i + 2 + futuresLength, 0,
                new QTableWidgetItem(QString::fromUtf8(buffer)));
        disc = forecastTermStructure->discount(d);
        forwardCurveTable_->setItem(i + 2 + futuresLength, 1,
                new QTableWidgetItem(QString::number(disc, 'g', 4)));
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

    // prepare interest rate curve
    std::vector<Period> oisTenors;
    std::vector<double> oisRates;
    Period depositTenor;
    double depositRate;
    std::vector<Date> futuresMaturities;
    std::vector<double> futuresPrices;
    std::vector<Period> swapTenors;
    std::vector<double> swapQuotes;
    
    bool useExternalVolSurface = modelInfo_->isExternalVolSurface();
    if (!useExternalVolSurface) {
        oisTenors.insert(oisTenors.begin(),
            std::begin(OIS_TENORS), std::end(OIS_TENORS));
        oisRates.insert(oisRates.begin(),
            std::begin(OIS_RATES), std::end(OIS_RATES));
        depositTenor = DEPOSIT_TENOR;
        depositRate  = DEPOSIT_RATE;
        futuresMaturities.insert(futuresMaturities.begin(),
            std::begin(FUTURES_MATURITIES), std::end(FUTURES_MATURITIES));
        futuresPrices.insert(futuresPrices.begin(),
            std::begin(FUTURES_PRICES), std::end(FUTURES_PRICES));
        swapTenors.insert(swapTenors.begin(),
            std::begin(SWAP_TENORS), std::end(SWAP_TENORS));
        swapQuotes.insert(swapQuotes.begin(),
            std::begin(SWAP_QUOTES), std::end(SWAP_QUOTES));
    } else if (vol_.size() > 0) {
        getOisQuoteData(oisTenors, oisRates);
        getForwardQuoteData(depositTenor, depositRate,
                    futuresMaturities, futuresPrices,
                    swapTenors, swapQuotes);
    } else {
        // no vol surface loaded, alert.
        QMessageBox::critical(this, QString::fromUtf8("未加载波动率曲面"),
                    QString::fromUtf8("请先加载波动率曲面！"),
                               QMessageBox::Ok);
    }

    if (!useExternalVolSurface || vol_.size() > 0) {
        double price = priceSwaption(notional,
                currency, effectiveDate, maturityDate, changeFirstExerciseDate, firstExerciseDate,
                fixedDirection, fixedCoupon, fixedPayFreq, fixedDayCounter,
                floatDirection, floatIndex, floatPayFreq, floatDayCounter,
                style, position, callFreq,
                pricingDate, model, engine, complexity, curve,
                useExternalVolSurface, vol_,
                oisTenors, oisRates,
                depositTenor, depositRate, futuresMaturities, futuresPrices, swapTenors, swapQuotes);
        modelInfo_->setPrice(price / notional, price);
    }
}
