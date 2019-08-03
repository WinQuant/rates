/*
 * RatesMainWindow
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTableWidget>

#include <vector>
#include <string>

class RatesMainWindow : public QMainWindow {
    Q_OBJECT
public:
    virtual ~RatesMainWindow();
    void setupMenu();

    void setVolTableWidget(QTableWidget *volTable);

    std::vector<std::string> getRowIndex();
    std::vector<std::string> getColIndex();
    std::vector<std::vector<double>> getValue();

private slots:
    void openBbg();

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

    QTableWidget *volTable_;
};

#endif
