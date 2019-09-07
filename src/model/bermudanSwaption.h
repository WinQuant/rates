/*
 * Bermudan Swaption Pricing module.
 */

#ifndef BERMUDAN_SWAPTION_H
#define BERMUDAN_SWAPTION_H

#include <QString>

#include <string>

double priceSwaption(double notional,
        QString currency, std::string effectiveDate, std::string maturityDate,
        QString fixedDirection, double fixedCoupon, QString fixedPayFreq, std::string fixedDayCounter,
        QString floatDirection, QString floatIndex, QString floatPayFreq, std::string floatDayCounter,
        QString style, QString position, QString callFreq,
        std::string today, QString model, QString engine,
        QString complexity, QString curve, bool useExternalVolSurface,
        std::vector<std::vector<double> > &volSurface);

#endif
