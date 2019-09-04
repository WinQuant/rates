/* Reproduce the BBG deal */

#include <ql/qldefines.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/indexes/ibor/fedfunds.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/schedule.hpp>
#include <ql/math/interpolations/forwardflatinterpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>

#include <ql/instruments/swaption.hpp>
#include <ql/pricingengines/swaption/fdhullwhiteswaptionengine.hpp>
#include <ql/pricingengines/swaption/treeswaptionengine.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/math/array.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/solvers1d/bisection.hpp>
#include <ql/math/solvers1d/ridder.hpp>

#include <ql/cashflows/coupon.hpp>
#include <ql/time/calendars/target.hpp>

#include <ql/experimental/shortrate/generalizedhullwhite.hpp>

#include <vector>
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

Period maturities[] = {
    Period( 1, Months ), Period( 3, Months ), Period( 6, Months ),
    Period( 9, Months ), Period( 1, Years ),  Period( 2, Years ),
    Period( 3, Years ),  Period( 4, Years ),  Period( 5, Years ),
    Period( 6, Years ) };

Period lengths[] = {
    Period( 6, Years ), Period( 6, Years ), Period( 5, Years ),
    Period( 5, Years ), Period( 5, Years ), Period( 4, Years ),
    Period( 3, Years ), Period( 2, Years ), Period( 1, Years ),
    Period( 1, Years ) };

// dual curve ois discounting
double oisDiscountingVols[] = {
    0.3619, 0.3558, 0.3665,
    0.3634, 0.3588, 0.3528,
    0.3408, 0.3312, 0.3219,
    0.3063 };

double liborDiscountingVols[] = {
    0.3654, 0.3594, 0.3700,
    0.3671, 0.3627, 0.3572,
    0.3454, 0.3361, 0.3265,
    0.3115 };

// forecast forward rate
double oisRates[] = {
    2.4100, 2.3879, 2.3885, 2.2890,
    2.2140, 2.1530, 2.0760, 2.0210, 1.9770, 1.9310, 1.8420, 1.7790,
    1.6760, 1.6200, 1.5820, 1.5840, 1.6040, 1.9211, 2.0390, 2.1035,
    2.1717, 2.2360, 2.2591, 2.2665, 2.2498, 2.2188 };

Period depositTenor(3, Months);
double depositRate = 0.0229963;

Date futuresMats[] = {
    Date(18, September, 2019), Date(18, December, 2019),
    Date(18, March, 2020),     Date(17, June, 2020),
    Date(16, September, 2020), Date(16, December, 2020) };

double futuresPrices[] = {
    97.92, 97.99, 98.17, 98.255, 98.315, 98.305 };

Period swapTenors[] = {
    Period( 2, Years ),  Period( 3, Years ),  Period( 4, Years ),
    Period( 5, Years ),  Period( 6, Years ),  Period( 7, Years ),
    Period( 8, Years ),  Period( 9, Years ),  Period( 10, Years ),
    Period( 11, Years ), Period( 12, Years ), Period( 15, Years ),
    Period( 20, Years ), Period( 25, Years ), Period( 30, Years ),
    Period( 40, Years ), Period( 50, Years ) };

double swapQuotes[] = {
    0.018799, 0.018327, 0.018291, 0.018500, 0.018840, 0.019211,
    0.019608, 0.020005, 0.020390, 0.020730, 0.021035, 0.021717,
    0.022360, 0.022591, 0.022665, 0.022498, 0.022188 };

void bbgCalibrateModel(
          const double *bsImpliedVols,
          const ext::shared_ptr<ShortRateModel>& model,
          const std::vector<ext::shared_ptr<BlackCalibrationHelper> >& helpers,
          const std::vector<bool>& fixParameters=std::vector<bool>()) {
    LevenbergMarquardt om;
    model->calibrate(helpers, om,
                     EndCriteria(400, 100, 1.0e-8, 1.0e-8, 1.0e-8),
                     Constraint(), std::vector<Real>(), fixParameters);

    // Output the implied Black volatilities
    for (Size i=0; i<helpers.size(); i++) {
        Real npv = helpers[i]->modelValue();
        Volatility implied = helpers[i]->impliedVolatility(npv, 1e-4,
                1000, 0.00, 1.00);
        Volatility diff = implied - bsImpliedVols[i];

        std::cout << bbgCalibrateMaturities[i] << "x"
                  << bbgCalibrateTenors[i] << "Y"
                  << std::setprecision(5) << std::noshowpos
                  << ": model " << std::setw(7) << io::volatility(implied)
                  << ", market " << std::setw(7)
                  << io::volatility(bsImpliedVols[i])
                  << " (" << std::setw(7) << std::showpos
                  << io::volatility(diff) << std::noshowpos << ")\n";
    }
}

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

bool isDualCurve(QString curve) {
    return curve == QString::fromUtf8("双重曲线");
}

bool isConstantModel(QString complexity) {
    return complexity == QString::fromUtf8("常函数");
}

ext::shared_ptr<Exercise> getQuantLibOptionExercise(QString style,
            ext::shared_ptr<VanillaSwap> swap, Date startDate){
    // Bermudan swaption
    // construct coupon dates
    std::vector<Date> bbgBermudanDates;
    const std::vector<ext::shared_ptr<CashFlow> >& bbgLeg =
            swap->fixedLeg();
    for (Size i=0; i<bbgLeg.size(); i++) {
        ext::shared_ptr<Coupon> coupon =
                ext::dynamic_pointer_cast<Coupon>(bbgLeg[i]);
        bbgBermudanDates.push_back(coupon->accrualStartDate());
    }

    if (style == QString::fromUtf8("百慕大期权(Bermudan)")) {
        return ext::shared_ptr<Exercise>(
                         new BermudanExercise(bbgBermudanDates));
    } else if (style == QString::fromUtf8("欧式期权(European)")) {
        // European option expires on the last day
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

/ functor for equation solver
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
        // params[size_ + index_] = vol;
        model_->setParams(params);
        Real npv = helper_->modelValue();
        Volatility implied = helper_->impliedVolatility(npv, 1e-6, 1000,
                    1e-5, 1000);
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

// calibrate the given GeneralizedHullWhite model, the model is updated
// in place during the calibration.
void calibrateGhw( ext::shared_ptr<GeneralizedHullWhite> &model,
        std::vector<ext::shared_ptr<BlackCalibrationHelper> >& helpers) {
    // need to build a series of calibration helper.
    // when calibrate the spot vol, the european swaption is assumed.
    std::cout << "In calibrateGhw" << std::endl;
    Disposable<Array> params = model->params();
    for (Size i=0; i < helpers.size(); i++)
        params[helpers.size() + i] = 0.00001;
    model->setParams(params);

    for (Size i = 0; i < helpers.size(); i++) {
        ghwBlackSolverImpl solver(model, helpers[i], i);
        Bisection bsolver;
        std::cout << "solve " << i << std::endl;
        // try {
            Real root = bsolver.solve(solver, 1e-5, 0.0015, 0.001, 0.02);
        /*
        } catch (QuantLib::Error) {
            std::cout << "Error calibrating " << i << std::endl;
            Disposable<Array> params = model->params();
            params[helpers.size() + i] = 0.00001;
            model->setParams(params);
        }
        */
        /*
        if (i < helpers.size() - 1) {
            // initialize next period vol to current value.
            Disposable<Array> params = model->params();
            params[helpers.size() + i + 1] = root;
            model->setParams(params);
        }
        */
    }

    std::cout << "GHW calibrated." << std::endl;
    // Output the implied Black volatilities
    /*
    for (Size i=0; i<helpers.size(); i++) {
        printParams(model);
        Real npv = helpers[i]->modelValue();
        Volatility implied = helpers[i]->impliedVolatility(npv, 1e-4, 1000,
                    1e-16, 1000);
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
    */
};

void printParams( ext::shared_ptr<GeneralizedHullWhite> &model ) {
    Disposable<Array> params = model->params();
    for (Size i = 0; i < params.size(); i++)
        std::cout << params[ i ] << std::endl;
}

double priceSwaption(double notional,
        QString currency, std::string effectiveDate, std::string maturityDate,
        QString fixedDirection, double fixedCoupon, QString fixedPayFreq, std::string fixedDayCounter,
        QString floatDirection, QString floatIndex, QString floatPayFreq, std::string floatDayCounter,
        QString style, QString position, QString callFreq,
        std::string today, QString model, QString engine,
        QString complexity, QString curve) {
    // unused arguments
    currency = currency;
    floatDirection  = floatDirection;
    floatDayCounter = floatDayCounter;
    position = position;
    callFreq = callFreq;
    bool endOfMonth = true;
    
    Date todaysDate = DateParser::parseFormatted(today, "%Y/%m/%d");
    Calendar calendar = TARGET();
    int settlementDays  = 2;
    Date settlementDate = calendar.advance(todaysDate, settlementDays,
                Days, ModifiedFollowing);
    Settings::instance().evaluationDate() = todaysDate;

    std::cout << todaysDate << " " << settlementDate << std::endl;

    // OIS curve construction
    DayCounter oisDayCounter = Actual360();
    RelinkableHandle<YieldTermStructure> discountTermStructure;

    std::vector<ext::shared_ptr<ZeroYield::helper> > oisHelper;
    oisHelper.push_back( ext::shared_ptr<ZeroYield::helper>(
                                    new DepositRateHelper(
                                        Handle<Quote>(
                                                ext::shared_ptr<Quote>(
                                                    new SimpleQuote(oisRates[ 0 ] / 100))),
                                            Period(1, Days), settlementDays, calendar,
                                            ModifiedFollowing, endOfMonth, oisDayCounter ) ) );
    for (int i = 1; i < sizeof( oisRates ) / sizeof( oisRates[ 0 ] ); i++) {
        oisHelper.push_back( ext::shared_ptr<ZeroYield::helper>(
                        new OISRateHelper(
                                settlementDays, oisTenors[ i ],
                                Handle<Quote>(
                                    ext::shared_ptr<Quote>(
                                        new SimpleQuote( oisRates[ i ] / 100 )) ),
                                ext::shared_ptr<OvernightIndex>(new FedFunds()) ) ) );
    }

    // forward curve construction
    DayCounter cashDayCounter = Actual360();
    RelinkableHandle<YieldTermStructure> forecastTermStructure; 
    ext::shared_ptr<IborIndex> liborIndex( new USDLibor( Period(3, Months), forecastTermStructure ) );
    std::vector<ext::shared_ptr<ZeroYield::helper> > depositHelper;
    depositHelper.push_back( ext::shared_ptr<ZeroYield::helper >(
                                        new DepositRateHelper(
                                            Handle<Quote>(ext::shared_ptr<Quote>(
                                                    new SimpleQuote(depositRate) ) ),
                                                depositTenor, settlementDays, calendar,
                                                ModifiedFollowing, endOfMonth,
                                                cashDayCounter ) ) );
    // futures prices represent 3m-2y futures rate
    DayCounter futuresDayCounter = Actual360();
    for (int i = 0; i < sizeof( futuresPrices ) / sizeof( futuresPrices[ 0 ] ); i++) {
        depositHelper.push_back( ext::shared_ptr<ZeroYield::helper>(
                                        new FuturesRateHelper(
                                            Handle<Quote>(ext::shared_ptr<Quote>(
                                                    new SimpleQuote(futuresPrices[i]))),
                                                futuresMats[i], 3, calendar,
                                                ModifiedFollowing, endOfMonth,
                                                futuresDayCounter,
                                            Handle<Quote>(ext::shared_ptr<SimpleQuote>(new SimpleQuote(0.0))) ) ) );
    }

    // swap quotes
    DayCounter fixedLegDayCounter = Thirty360();
    for (int i = 0; i < sizeof( swapQuotes ) / sizeof( swapQuotes[ 0 ] ); i++) {
        depositHelper.push_back( ext::shared_ptr<ZeroYield::helper>(
                                    new SwapRateHelper(
                                        Handle<Quote>(ext::shared_ptr<Quote>(
                                                    new SimpleQuote(swapQuotes[ i ]))),
                                        swapTenors[ i ], calendar, Semiannual,
                                        ModifiedFollowing, fixedLegDayCounter,
                                        liborIndex, Handle<Quote>(), Period(0, Days),
                                        discountTermStructure, settlementDays ) ) );
    }

    ext::shared_ptr<PiecewiseYieldCurve<ZeroYield, Linear> > depoFuturesSwapCurve(
                new PiecewiseYieldCurve<ZeroYield, Linear>(
                        settlementDate, depositHelper, fixedLegDayCounter ) );
    ext::shared_ptr<PiecewiseYieldCurve<ZeroYield, Linear> > oisCurve(
                new PiecewiseYieldCurve<ZeroYield, Linear>(
                        settlementDate, oisHelper, fixedLegDayCounter ) );
    depoFuturesSwapCurve->enableExtrapolation();
    oisCurve->enableExtrapolation();

    double *bsVols = NULL;
    if (isDualCurve(curve)) {
        discountTermStructure.linkTo( oisCurve );
        bsVols = oisDiscountingVols;
    }
    else {
        discountTermStructure.linkTo( depoFuturesSwapCurve );
        bsVols = liborDiscountingVols;
    }
    forecastTermStructure.linkTo( depoFuturesSwapCurve );

    // define the deal
    // deal property
    VanillaSwap::Type type = getQuantLibPayDirection(fixedDirection);

    // fixed convention
    Frequency fixedLegFrequency = getQuantLibPayFreq(fixedPayFreq);
    BusinessDayConvention fixedLegConvention = ModifiedFollowing;
    // float convetion
    Frequency floatLegFrequency = getQuantLibPayFreq(floatPayFreq);
    BusinessDayConvention floatLegConvention = ModifiedFollowing;

    Date startDate = DateParser::parseFormatted(effectiveDate, "%Y/%m/%d");
    Date maturity = DateParser::parseFormatted(maturityDate, "%Y/%m/%d");

    // fixed leg property
    fixedLegDayCounter = getQuantLibDayCounter(fixedDayCounter);
    Rate fixedRate = fixedCoupon;
    Schedule fixedSchedule(startDate, maturity, Period(fixedLegFrequency),
                           calendar, fixedLegConvention, fixedLegConvention,
                           DateGeneration::Backward, endOfMonth);

    // float leg property
    DayCounter floatLegDayCounter = Actual360();
    Schedule floatSchedule(startDate, maturity, Period(floatLegFrequency),
                           calendar,
                           floatLegConvention, floatLegConvention,
                           DateGeneration::Backward, endOfMonth);

    ext::shared_ptr<VanillaSwap> swap(new VanillaSwap(
        type, notional,
        fixedSchedule, fixedRate, fixedLegDayCounter,
        floatSchedule, liborIndex, 0.0,
        floatLegDayCounter));

    // build the at the money swap
    swap->setPricingEngine(ext::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(discountTermStructure)));

    // construct swaption exercise
    ext::shared_ptr<Exercise> exercise = getQuantLibOptionExercise(style,
                swap, startDate);
    Swaption swaption(swap, exercise);

    // pricing with generalized hull white for piece-wise term structure fit
    ext::shared_ptr<GeneralizedHullWhite> piecewiseHw = getQuantLibModel(model, depoFuturesSwapCurve);
    ext::shared_ptr<PricingEngine> pricingEngine = getQuantLibPricingEngine(engine, piecewiseHw, discountTermStructure);
    swaption.setPricingEngine(pricingEngine);
    std::cout << "Piecewise Hull-White price at " << swaption.NPV() << std::endl;

    return swaption.NPV();
}
