/*
 * RatesMainWindow
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTableWidget>

#include <vector>
#include <string>

#include "widgets/dealInfo.h"
#include "widgets/fixedLegSpec.h"
#include "widgets/floatLegSpec.h"
#include "widgets/optionality.h"
#include "widgets/modelInfo.h"

#include <ql/handle.hpp>
#include <ql/time/date.hpp>
#include <ql/time/period.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>

using namespace QuantLib;

class RatesMainWindow : public QMainWindow {
    Q_OBJECT
public:
    virtual ~RatesMainWindow();
    void setupMenu();

    void setDealInfoWidget(DealInfo *dealInfo);
    void setFixedLegSpecWidget(FixedLegSpec *fixedLegSpec);
    void setFloatLegSpecWidget(FloatLegSpec *floatLegSpec);
    void setOptionalityWidget(Optionality *optionality);
    void setModelInfoWidget(ModelInfo *modelInfo);

    void setVolTableWidget(QTableWidget *volTable);
    void setOisTableWidget(QTableWidget *oisTable);
    void setForwardTableWidget(QTableWidget *forwardTable);

    std::vector<std::string> getRowIndex();
    std::vector<std::string> getColIndex();
    std::vector<std::vector<double>> getValue();

    void getOisQuoteData(std::vector<Period> &oisTenors,
                         std::vector<double> &oisRates);
    void getForwardQuoteData(Period &depositTenor, double &depositRate,
                             std::vector<Date> &futuresMats,
                             std::vector<double> &futuresPrices,
                             std::vector<Period> &swapTenors,
                             std::vector<double> &swapQuotes);

private slots:
    void openBbg();
    void saveOis();
    void saveForecast();
    void calculate();

private:
    void updateVolTable();
    void updateOisTable(Date startDate, Calendar calendar,
            const std::vector<Period> &oisTerms,
            const RelinkableHandle<YieldTermStructure> &discountTermStructure);
    void updateForwardTable(Date startDate, Calendar calendar,
            Period depositTenor,
            const std::vector<Date> &futuresMaturities,
            const std::vector<Period> &swapTenors,
            const RelinkableHandle<YieldTermStructure> &forecastTermStructure);

    std::vector<std::string> volRowIndex_;
    std::vector<std::string> volColIndex_;
    std::vector<std::vector<double>> vol_;

    // forward rate curves
    std::vector<int> forwardTerm_;
    std::vector<std::string> forwardUnit_;
    std::vector<double> forwardBid_;
    std::vector<double> forwardAsk_;

    // ois rate curves
    std::vector<int> oisTerm_;
    std::vector<std::string> oisUnit_;
    std::vector<double> oisBid_;
    std::vector<double> oisAsk_;

    // widgets
    DealInfo *dealInfo_;
    FixedLegSpec *fixedLegSpec_;
    FloatLegSpec *floatLegSpec_;
    Optionality *optionality_;
    ModelInfo *modelInfo_;

    QTableWidget *volTable_;
    QTableWidget *oisCurveTable_;
    QTableWidget *forwardCurveTable_;
};

#endif
