/*
 * Bermudan Swaption Pricing module.
 */

#ifndef BERMUDAN_SWAPTION_H
#define BERMUDAN_SWAPTION_H

#include <QString>

#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/qldefines.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/date.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/period.hpp>

#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/zeroyieldstructure.hpp>

#include <string>
#include <vector>

using namespace QuantLib;

void bootstrapIrTermStructure(const std::vector<Period> &oisTenors, const std::vector<double> &oisRates,
            Period depositTenor, double depositRate,
            const std::vector<Date> &futuresMaturities, const std::vector<double> &futuresPrices,
            const std::vector<Period> &swapTenors, const std::vector<double> &swapQuotes,
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
        std::vector<std::vector<double> > &volSurface,
        std::vector<Period> &oisTenors, std::vector<double> &oisRates,
        Period depositTenor, double depositRate,
        std::vector<Date> &futuresMaturities, std::vector<double> &futuresPrices,
        std::vector<Period> &swapTenors, std::vector<double> &swapQuotes);

#endif
