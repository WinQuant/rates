/* Reproduce the BBG deal */

#include <ql/qldefines.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/pricingengines/swaption/fdhullwhiteswaptionengine.hpp>
#include <ql/pricingengines/swaption/treeswaptionengine.hpp>
#include <ql/pricingengines/swaption/jamshidianswaptionengine.hpp>
#include <ql/math/array.hpp>
#include <ql/math/interpolations/forwardflatinterpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/solvers1d/bisection.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/indexes/ibor/libor.hpp>
#include <ql/currencies/america.hpp>

#include <ql/cashflows/coupon.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/forwardcurve.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/utilities/dataparsers.hpp>

#include <ql/experimental/shortrate/generalizedhullwhite.hpp>

#include <iostream>
#include <iomanip>

#include "model/bermudanSwaption.h"

using namespace QuantLib;

#if defined(QL_ENABLE_SESSONS)
namespace QuantLib {
    Integer sessionId() { return 0; }
}
#endif


Date ghwDates[] = { Date(16, July,    2019),
                    Date(16, August,  2019),
                    Date(15, October, 2019),
                    Date(15, January, 2020),
                    Date(16, April,   2020),
                    Date(16, July,    2020),
                    Date(16, July,    2021),
                    Date(17, July,    2022),
                    Date(17, July,    2023),
                    Date(16, July,    2024)};
Real ghwVols[] = { 0.0061, 0.0066, 0.0066, 
                   0.0084, 0.0080, 0.0085,
                   0.0062, 0.0065, 0.0123,
                   0.00001 };
ext::shared_ptr<GeneralizedHullWhite> makeGhw(
            const Handle<YieldTermStructure> &yt, Real speed) {
    std::vector<Date> vd = std::vector<Date>(std::begin(ghwDates),
                                             std::end(ghwDates));
    std::vector<Real> vv = std::vector<Real>(std::begin(ghwVols),
                                             std::end(ghwVols));
    std::vector<Real> vr = std::vector<Real>(vv.size(), speed);
    return ext::make_shared<GeneralizedHullWhite>(yt, vd, vd, vr, vv);
};

VanillaSwap::Type getQuantLibPayDirection(QString direction) {
    VanillaSwap::Type type = VanillaSwap::Receiver;

    if (direction == QString::fromUtf8("收款(Receive)")) {
        type = VanillaSwap::Receiver;
    } else if (direction == QString::fromUtf8("付款(Pay)")){
        type = VanillaSwap::Payer;
    }

    return type;
}

Frequency getQuantLibPayFreq(QString payFreq) {
    Frequency freq = Annual;
    if (payFreq == QString::fromUtf8("季度支付(Quarter)")) {
        freq = Quarterly;
    } else if (payFreq == QString::fromUtf8("半年支付(Semi-annual)")) {
        freq = Semiannual;
    } else if (payFreq == QString::fromUtf8("年度支付(Annual)")) {
        freq = Annual;
    }

    return freq;
}

DayCounter getQuantLibDayCounter(std::string dayCounter) {
    DayCounter dc = ActualActual();

    if (dayCounter == "30 / 360") {
        dc = Thirty360(Thirty360::USA);
    } else if (dayCounter == "Act / 360") {
        dc = Actual360();
    } else if (dayCounter == "Act / Act") {
        dc = ActualActual();
    }

    return dc;
}

IborIndex *getQuantLibIndex(QString floatIndex,
        Calendar calendar, Handle<YieldTermStructure> &fwdCurve) {
    floatIndex = floatIndex;
    return new Libor("US0003M", Period(3, Months), 2,
                        USDCurrency(), calendar, Actual360(), fwdCurve);
}

ext::shared_ptr<Exercise> getQuantLibOptionExercise(QString style,
            ext::shared_ptr<VanillaSwap> swap, Date startDate){
    if (style == QString::fromUtf8("百慕大期权(Bermudan)")) {
        // Bermudan swaption
        // construct coupon dates
        std::vector<Date> bbgBermudanDates;
        const std::vector<ext::shared_ptr<CashFlow> >& bbgLeg =
                swap->floatingLeg();
        for (Size i=0; i<bbgLeg.size(); i++) {
            ext::shared_ptr<Coupon> coupon =
                    ext::dynamic_pointer_cast<Coupon>(bbgLeg[i]);
            bbgBermudanDates.push_back(coupon->accrualStartDate());
        }

        return ext::shared_ptr<Exercise>(
                         new BermudanExercise(bbgBermudanDates));
    } else if (style == QString::fromUtf8("欧式期权(European)")) {
        return ext::shared_ptr<Exercise>(
                    new EuropeanExercise(startDate));
    }

    return ext::shared_ptr<Exercise>(NULL);
}

ext::shared_ptr<GeneralizedHullWhite> getQuantLibModel(QString model,
            Handle<YieldTermStructure> &fwdCurve) {
    model = model;
    return ext::shared_ptr<GeneralizedHullWhite>(makeGhw(
                    fwdCurve, 0.03));
}

ext::shared_ptr<PricingEngine> getQuantLibPricingEngine(QString engine,
            ext::shared_ptr<GeneralizedHullWhite> model,
            Handle<YieldTermStructure> &discountCurve) {
    engine = engine;
    return ext::shared_ptr<PricingEngine>(
                new TreeSwaptionEngine(model, 500, discountCurve));
}

ext::shared_ptr<YieldTermStructure> makeForwardCurve() {
    // forward curve
    Date forwardDates[] = { Date( 15, July, 2019 ), Date( 15, September, 2019 ),
                     Date( 15, December, 2019 ), Date( 15, March, 2020 ),
                     Date( 15, June, 2020 ), Date( 15, September, 2020 ),
                     Date( 15, December, 2020 ), Date( 15, July, 2021 ),
                     Date( 15, July, 2022 ), Date( 15, July, 2023 ),
                     Date( 15, July, 2024 ), Date( 15, July, 2025 ),
                     Date( 15, July, 2026 ), Date( 15, July, 2027 ),
                     Date( 15, July, 2028 ), Date( 15, July, 2029 ),
                     Date( 15, July, 2030 ), Date( 15, July, 2031 ),
                     Date( 15, July, 2034 ), Date( 15, July, 2039 ),
                     Date( 15, July, 2044 ), Date( 15, July, 2049 ),
                     Date( 15, July, 2059 ), Date( 15, July, 2069 ) };
    Rate forwardRates[] = { 0.02322250, 0.02069755, 0.01999178,
                            0.01808343, 0.01712259, 0.01650936,
                            0.01659374, 0.01870500, 0.01822999,
                            0.01818000, 0.01838299, 0.01872399,
                            0.01910549, 0.01950800, 0.01991010,
                            0.02030175, 0.02066999, 0.02096700,
                            0.02166650, 0.02233445, 0.02257999,
                            0.02266150, 0.02248000, 0.02216500 };

    std::vector<Date> fcd(std::begin(forwardDates), std::end(forwardDates));
    std::vector<Rate> fcr(std::begin(forwardRates), std::end(forwardRates));
    ext::shared_ptr<InterpolatedForwardCurve<ForwardFlat> > fcc =
            ext::make_shared<InterpolatedForwardCurve<ForwardFlat> >( fcd, fcr, Actual360());

    return fcc;
}

ext::shared_ptr<YieldTermStructure> makeTermStructures() {
    // discount curve directly
    Date discountingDates[] = { Date(15, July, 2019 ), Date( 18, July, 2019 ),
                     Date( 15, September, 2019 ), Date( 15, December, 2019 ),
                     Date( 15, March, 2020 ), Date( 15, June, 2020 ),
                     Date( 15, September, 2020 ), Date( 15, December, 2020 ),
                     Date( 15, July, 2021 ),
                     Date( 15, July, 2022 ), Date( 15, July, 2023 ),
                     Date( 15, July, 2024 ), Date( 15, July, 2025 ),
                     Date( 15, July, 2026 ), Date( 15, July, 2027 ),
                     Date( 15, July, 2028 ), Date( 15, July, 2029 ),
                     Date( 15, July, 2030 ), Date( 15, July, 2031 ),
                     Date( 15, July, 2034 ), Date( 15, July, 2039 ),
                     Date( 15, July, 2044 ), Date( 15, July, 2049 ),
                     Date( 15, July, 2059 ), Date( 15, July, 2069 ) };
    Rate discountingRates[] = { 1.0,
        0.994100, 0.990833, 0.985851, 0.981365, 0.977135, 0.973075,
        0.969010, 0.963438, 0.947077, 0.930235, 0.912599, 0.894150,
        0.875090, 0.855718, 0.836047, 0.816152, 0.796257, 0.776939,
        0.721127, 0.637235, 0.565823, 0.504040, 0.406721, 0.333739 };

    std::vector<Date> dcd(std::begin(discountingDates), std::end(discountingDates));
    std::vector<Rate> dcr(std::begin(discountingRates), std::end(discountingRates));
    ext::shared_ptr<InterpolatedDiscountCurve<Cubic> > dcc =
            ext::make_shared<InterpolatedDiscountCurve<Cubic> >( dcd, dcr, Actual360());

    return dcc;
}

/* TODO no calibration so far
// the vol is calibrated based on the bbg vol surface
Volatility bbgCalibrationDiagnalVols[] = {
    0.3654, 0.3594, 0.3700, 0.3671, 0.3627,
    0.3572, 0.3454, 0.3361, 0.3265, 0.3115
    // 0.2936, 0.3106, 0.3369, 0.3531, 0.3622,
    // 0.3855, 0.3926, 0.3859, 0.3884, 0.3109
    
    // 0.6878, 0.6766, 0.6828, 0.6765, 0.6747,
    // 0.6685, 0.6604, 0.6605, 0.6570, 0.6540
};
*/

/*
Volatility bbgCalibrationDiagnalVols[] = {
    0.3612, 0.3562, 0.3594, 0.3732, 0.3730,
    0.3747, 0.3665, 0.3580, 0.3497, 0.3338,
    0.3166
    // 0.6878, 0.6766, 0.6828, 0.6765, 0.6747,
    // 0.6685, 0.6604, 0.6605, 0.6570, 0.6540
};
*/

// expiry grid
/*
Period bbgCalibrationExpiries[] = {
    Period(1, Months), Period(3, Months), Period(6, Months),
    Period(9, Months), Period(1, Years),  Period(2, Years),
    Period(3, Years),  Period(4, Years),  Period(5, Years),
    Period(6, Years),  Period(7, Years)
};
*/
/* TODO no calibration so far
Period bbgCalibrationExpiries[] = {
    Period(1, Months), Period(3, Months), Period(6, Months),
    Period(9, Months), Period(1, Years),  Period(2, Years),
    Period(3, Years),  Period(4, Years),  Period(5, Years),
    Period(6, Years) };
*/

// maturity grid
/*
Period bbgCalibrationMaturities[] = {
    Period(6, Years), Period(6, Years), Period(6, Years),
    Period(5, Years), Period(5, Years), Period(4, Years),
    Period(3, Years), Period(2, Years), Period(1, Years),
    Period(1, Years), Period(1, Years)
};
*/

/* TODO no calibration so far
Period bbgCalibrationMaturities[] = {
    Period(6, Years), Period(6, Years), Period(5, Years),
    Period(5, Years), Period(5, Years), Period(4, Years),
    Period(3, Years), Period(2, Years), Period(1, Years),
    Period(1, Years) };

// functor for equation solver
struct ghwBlackSolverImpl {
    ghwBlackSolverImpl(ext::shared_ptr<GeneralizedHullWhite> &model,
            ext::shared_ptr<BlackCalibrationHelper> &helper,
            Size index):model_(model), helper_(helper), index_(index),
                size_(model_->FixedReversion().size() / 2) {
    }

    Real operator()(Real vol) const {
        // size of the model
        Disposable<Array> params = model_->params();
        for (Size i = index_; i < size_; i++)
            params[size_ + i] = vol;
        model_->setParams(params);
        Real npv = helper_->modelValue();
        Volatility implied = helper_->impliedVolatility(npv, 1e-4, 1000,
                    1e-8, 10);
        std::cout << "Implied vol: " << implied
                  << " black vol: " << helper_->volatility()->value()
                  << " spot vol: " << vol
                  << std::endl;
        return implied - helper_->volatility()->value();
    }

private:
    ext::shared_ptr<GeneralizedHullWhite> &model_;
    ext::shared_ptr<BlackCalibrationHelper> &helper_;
    Size index_;
    Size size_;
};
*/

void printParams( ext::shared_ptr<GeneralizedHullWhite> &model ) {
    Disposable<Array> params = model->params();
    for (Size i = 0; i < params.size(); i++)
        std::cout << params[ i ] << std::endl;
}

/* TODO no calibration so far
// calibrate the given GeneralizedHullWhite model, the model is updated
// in place during the calibration.
void calibrateGhw( ext::shared_ptr<GeneralizedHullWhite> &model,
        std::vector<ext::shared_ptr<BlackCalibrationHelper> >& helpers) {
    // need to build a series of calibration helper.
    // when calibrate the spot vol, the european swaption is assumed.
    std::cout << "In calibrateGhw" << std::endl;

    for (Size i = 0; i < helpers.size(); i++) {
        printParams(model);
        ghwBlackSolverImpl solver(model, helpers[i], i);
        Bisection bsolver;
        Real root = bsolver.solve(solver, 1e-5, 0.005, 0.00001, 0.02);
        if (i < helpers.size() - 1) {
            // initialize next period vol to current value.
            Disposable<Array> params = model->params();
            params[helpers.size() + i + 1] = root;
            model->setParams(params);
        }
    }

    std::cout << "GHW calibrated." << std::endl;
    // Output the implied Black volatilities
    for (Size i=0; i<helpers.size(); i++) {
        Real npv = helpers[i]->modelValue();
        Volatility implied = helpers[i]->impliedVolatility(npv, 1e-4, 1000,
                    1e-8, 10);
        Volatility diff = implied - bbgCalibrationDiagnalVols[i];

        std::cout << bbgCalibrationExpiries[i] << "x"
                  << bbgCalibrationMaturities[i]
                  << std::setprecision(5) << std::noshowpos
                  << ": model " << std::setw(7) << io::volatility(implied)
                  << ", market " << std::setw(7)
                  << io::volatility(bbgCalibrationDiagnalVols[i])
                  << " (" << std::setw(7) << std::showpos
                  << io::volatility(diff) << std::noshowpos << ")\n";
    }
};
*/

double priceSwaption(double notional,
        QString currency, std::string effectiveDate, std::string maturityDate,
        QString fixedDirection, double fixedCoupon, QString fixedPayFreq, std::string fixedDayCounter,
        QString floatDirection, QString floatIndex, QString floatPayFreq, std::string floatDayCounter,
        QString style, QString position, QString callFreq,
        std::string today, QString model, QString engine) {
    // unused arguments
    currency = currency;
    floatDirection  = floatDirection;
    floatDayCounter = floatDayCounter;
    position = position;
    callFreq = callFreq;
    
    Date todaysDate = DateParser::parseFormatted(today, "%Y/%m/%d");
    Calendar calendar = TARGET();
    Date settlementDate = DateParser::parseFormatted(effectiveDate, "%Y/%m/%d");
    Settings::instance().evaluationDate() = todaysDate;

    std::cout << todaysDate << std::endl;

    // discounting curve
    Handle<YieldTermStructure> bbgDiscountingCurve(
            makeTermStructures());
    Handle<YieldTermStructure> bbgFwdCurve(
            makeForwardCurve());

    // define the deal
    // deal property
    VanillaSwap::Type type = getQuantLibPayDirection(fixedDirection);

    // fixed convention
    Frequency fixedLegFrequency = getQuantLibPayFreq(fixedPayFreq);
    BusinessDayConvention fixedLegConvention = ModifiedFollowing;
    // float convetion
    Frequency floatLegFrequency = getQuantLibPayFreq(floatPayFreq);
    BusinessDayConvention floatLegConvention = ModifiedFollowing;

    Date startDate = settlementDate;
    Date maturity = DateParser::parseFormatted(maturityDate, "%Y/%m/%d");

    // fixed leg property
    DayCounter fixedLegDayCounter = getQuantLibDayCounter(fixedDayCounter);
    Rate fixedRate = fixedCoupon;
    Schedule fixedSchedule(startDate, maturity, Period(fixedLegFrequency),
                           calendar, fixedLegConvention, fixedLegConvention,
                           DateGeneration::Forward, false);

    // float leg property
    // TODO adapt to China index
    ext::shared_ptr<IborIndex> bbgIndex3M(getQuantLibIndex(floatIndex,
                    calendar, bbgFwdCurve));
    Schedule floatSchedule(startDate, maturity, Period(floatLegFrequency),
                           calendar,
                           floatLegConvention, floatLegConvention,
                           DateGeneration::Forward, false);

    ext::shared_ptr<VanillaSwap> swap(new VanillaSwap(
        type, notional,
        fixedSchedule, fixedRate, fixedLegDayCounter,
        floatSchedule, bbgIndex3M, 0.0,
        bbgIndex3M->dayCounter()));

    // build the at the money swap
    swap->setPricingEngine(ext::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(bbgDiscountingCurve)));

    // construct swaption exercise
    ext::shared_ptr<Exercise> exercise = getQuantLibOptionExercise(style,
                swap, startDate);
    Swaption swaption(swap, exercise);

    // pricing with generalized hull white for piece-wise term structure fit
    ext::shared_ptr<GeneralizedHullWhite> piecewiseHw = getQuantLibModel(model, bbgFwdCurve);
    ext::shared_ptr<PricingEngine> pricingEngine = getQuantLibPricingEngine(engine, piecewiseHw, bbgDiscountingCurve);
    swaption.setPricingEngine(pricingEngine);
    std::cout << "Piecewise Hull-White price at " << swaption.NPV() << std::endl;

    return swaption.NPV();
}
