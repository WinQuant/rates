#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QWidget>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>

#include <iostream>

#include "mainWindow.moc"
#include "widgets/dealInfo.h"
#include "widgets/fixedLegSpec.h"
#include "widgets/floatLegSpec.h"
#include "widgets/optionality.h"
#include "widgets/modelInfo.h"

QWidget *buildPricingPanel(QWidget *parent, RatesMainWindow *window) {
    QScrollArea *scroll = new QScrollArea(parent);

    QWidget *panel = new QWidget(scroll);
    QVBoxLayout *layout = new QVBoxLayout(panel);

    // deal specification
    QLabel *dealInfoLabel = new QLabel(QString::fromUtf8("交易信息"));
    dealInfoLabel->setStyleSheet("font-weight: bold");
    DealInfo *dealInfo = new DealInfo();

    // fixed leg specification
    QLabel *fixedLegLabel = new QLabel(QString::fromUtf8("固定利率端规范"));
    fixedLegLabel->setStyleSheet("font-weight: bold");
    FixedLegSpec *fixedLeg = new FixedLegSpec();
    
    // float leg specification
    QLabel *floatLegLabel = new QLabel(QString::fromUtf8("浮动利率端规范"));
    floatLegLabel->setStyleSheet("font-weight: bold");
    FloatLegSpec *floatLeg = new FloatLegSpec();

    // optionality
    QLabel *optionalityLabel = new QLabel(QString::fromUtf8("期权性"));
    optionalityLabel->setStyleSheet("font-weight: bold");
    Optionality *optionality = new Optionality();

    // model information
    QLabel *modelInfoLabel = new QLabel(QString::fromUtf8("模型参数"));
    modelInfoLabel->setStyleSheet("font-weight: bold");
    ModelInfo *modelInfo = new ModelInfo();

    // pass-in model
    window->setDealInfoWidget(dealInfo);
    window->setFixedLegSpecWidget(fixedLeg);
    window->setFloatLegSpecWidget(floatLeg);
    window->setOptionalityWidget(optionality);
    window->setModelInfoWidget(modelInfo);

    QPushButton *calcButton = new QPushButton(QString::fromUtf8("计算"));
    // trigger for calculation
    window->connect(calcButton, SIGNAL(clicked()), window, SLOT(calculate()));

    layout->addWidget(dealInfoLabel);
    layout->addWidget(dealInfo);

    layout->addWidget(fixedLegLabel);
    layout->addWidget(fixedLeg);

    layout->addWidget(floatLegLabel);
    layout->addWidget(floatLeg);

    layout->addWidget(optionalityLabel);
    layout->addWidget(optionality);

    layout->addWidget(modelInfoLabel);
    layout->addWidget(modelInfo);

    layout->addWidget(calcButton);

    panel->setLayout(layout);

    scroll->setWidget(panel);
    scroll->setWidgetResizable(true);
    scroll->setFixedHeight(700);

    return scroll;
}

QTableWidget *buildMarketPanel(QWidget *parent) {
    QTableWidget *table = new QTableWidget(parent);
    table->setRowCount(5);
    table->setColumnCount(5);

    // fill-in dummy numbers
    for (int i=0; i < 5; i++) {
        for (int j=0; j < 5; j++) {
            table->setItem(i, j, new QTableWidgetItem());
        }
    }

    return table;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    RatesMainWindow *window = new RatesMainWindow();
    window->setWindowTitle(QString::fromUtf8("Swaption Pricing"));
    window->resize(640, 720);

    // window menu
    window->setupMenu();

    QWidget *centralWidget = new QWidget(window);
    QTabWidget *tabs = new QTabWidget(centralWidget);

    QWidget *pricingPanel = buildPricingPanel(tabs, window);
    QTableWidget *marketPanel  = buildMarketPanel(tabs);

    tabs->setFixedSize(640, 720);
    tabs->addTab(pricingPanel, "Pricing");
    tabs->addTab(marketPanel,  "Market Volatility");

    window->setVolTableWidget(marketPanel);
    window->setCentralWidget(centralWidget);
    window->show();

    return app.exec();
}
