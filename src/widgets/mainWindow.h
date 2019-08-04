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

    std::vector<std::string> getRowIndex();
    std::vector<std::string> getColIndex();
    std::vector<std::vector<double>> getValue();

private slots:
    void openBbg();
    void calculate();

private:
    void updateVolTable();

    std::vector<std::string> rowIndex_;
    std::vector<std::string> colIndex_;
    std::vector<std::vector<double>> value_;

    // rate curves
    std::vector<std::string> term_;
    std::vector<double> marketRate_;
    std::vector<double> shift_;
    // shifted rate = shift + market rate
    std::vector<double> shiftedRate_;
    std::vector<double> zeroRate_;
    std::vector<double> discount_;

    // widgets
    DealInfo *dealInfo_;
    FixedLegSpec *fixedLegSpec_;
    FloatLegSpec *floatLegSpec_;
    Optionality *optionality_;
    ModelInfo *modelInfo_;

    QTableWidget *volTable_;
};

#endif
