/*
 * Bermudan Swaption Pricing module.
 */

#ifndef BERMUDAN_SWAPTION_H
#define BERMUDAN_SWAPTION_H

#include <QString>

#include <string>

void bootstrapIrTermStructure(int nOis, Period *oisTenor, double *oisRates,
            Period depositTenor, double depositRate,
            int nFuturesPrices, Date *futuresMaturities, double *futuresPrices,
            int nSwapQuotes, Period *swapTenors, double *swapQuotes,
            int settlementDays, Calendar calendar, Date settlementDate,
            DayCounter dayCounter, ext::shared_ptr<IborIndex> liborIndex,
            bool endOfMonth, bool useDualCurve,
            RelinkableHandle<YieldTermStructure> &discountTermStructure,
            RelinkableHandle<YieldTermStructure> &forecastTermStructure);

double priceSwaption(double notional,
        QString currency, std::string effectiveDate, std::string maturityDate, bool changeFirstExerciseDate, std::string firstExerciseDate,
        QString fixedDirection, double fixedCoupon, QString fixedPayFreq, std::string fixedDayCounter,
        QString floatDirection, QString floatIndex, QString floatPayFreq, std::string floatDayCounter,
        QString style, QString position, QString callFreq,
        std::string today, QString model, QString engine,
        QString complexity, QString curve, bool useExternalVolSurface,
        std::vector<std::vector<double> > &volSurface);

#endif
