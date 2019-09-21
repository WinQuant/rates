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
#include <ql/pricingengines/swaption/fdg2swaptionengine.hpp>
#include <ql/pricingengines/swaption/treeswaptionengine.hpp>
#include <ql/pricingengines/swaption/g2swaptionengine.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <ql/math/array.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/solvers1d/bisection.hpp>
#include <ql/math/solvers1d/ridder.hpp>

#include <ql/cashflows/coupon.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/utilities/dataparsers.hpp>

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

Period oisTenors[] = {
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

        std::cout << maturities[i] << "x"
                  << lengths[i]
                  << std::setprecision(5) << std::noshowpos
                  << ": model " << std::setw(7) << io::volatility(implied)
                  << ", market " << std::setw(7)
                  << io::volatility(bsImpliedVols[i])
                  << " (" << std::setw(7) << std::showpos
                  << io::volatility(diff) << std::noshowpos << ")\n";
    }
}

void calibrateG2Model(
          const double *bsImpliedVols,
          const ext::shared_ptr<ShortRateModel>& model,
          const std::vector<ext::shared_ptr<BlackCalibrationHelper> >& helpers,
          double simplex) {

    std::vector<bool> fixParameters;
    fixParameters.push_back( true );
    fixParameters.push_back( false );
    fixParameters.push_back( true );
    fixParameters.push_back( false );
    fixParameters.push_back( false );
    LevenbergMarquardt om(1e-8, 1e-8, 1e-8);
    model->calibrate(helpers, om, EndCriteria(1000, 250, 1e-6, 1e-8, 1e-8),
            Constraint(), std::vector<Real>(), fixParameters);

    // Output the implied Black volatilities
    for (Size i=0; i<helpers.size(); i++) {
        Real npv = helpers[i]->modelValue();
        Volatility implied = helpers[i]->impliedVolatility(npv, 1e-4,
                1000, 0.05, 0.50);
        std::cout << npv << std::endl;
        Volatility diff = implied - bsImpliedVols[i];

        std::cout << maturities[i] << "x"
                  << lengths[i]
                  << std::setprecision(5) << std::noshowpos
                  << ": model " << std::setw(7) << io::volatility(implied)
                  << ", market " << std::setw(7)
                  << io::volatility(bsImpliedVols[i])
                  << " (" << std::setw(7) << std::showpos
                  << io::volatility(diff) << std::noshowpos << ")\n";
    }
}

ext::shared_ptr<GeneralizedHullWhite> makeGhw(
            RelinkableHandle<YieldTermStructure> &yt, Real speed) {
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
            ext::shared_ptr<VanillaSwap> swap, Date startDate,
            bool changeFirstExerciseDate, Date firstExerciseDate){
    // Bermudan swaption
    // construct coupon dates
    std::vector<Date> bbgBermudanDates;
    Date europeanDate = startDate;
    int offset = 0;
    const std::vector<ext::shared_ptr<CashFlow> >& bbgLeg =
            swap->fixedLeg();
    if (changeFirstExerciseDate) {
        ext::shared_ptr<Coupon> coupon =
                ext::dynamic_pointer_cast<Coupon>(bbgLeg[0]);
        offset = firstExerciseDate - coupon->accrualStartDate();

        europeanDate = firstExerciseDate;
    }
    for (Size i=0; i<bbgLeg.size(); i++) {
        ext::shared_ptr<Coupon> coupon =
                ext::dynamic_pointer_cast<Coupon>(bbgLeg[i]);
        bbgBermudanDates.push_back(coupon->accrualStartDate() + offset);
        std::cout << "Bermudan exercise date: " << bbgBermudanDates[i] << std::endl;
    }

    if (style == QString::fromUtf8("百慕大期权(Bermudan)")) {
        return ext::shared_ptr<Exercise>(
                         new BermudanExercise(bbgBermudanDates));
    } else if (style == QString::fromUtf8("欧式期权(European)")) {
        // European option expires on the last day
        std::cout << "European exercise date: " << europeanDate << std::endl;
        return ext::shared_ptr<Exercise>(
                    new EuropeanExercise(europeanDate));
    }

    return ext::shared_ptr<Exercise>(NULL);
}

ext::shared_ptr<GeneralizedHullWhite> buildGhw(
            const Handle<YieldTermStructure> &yt, Real speed) {
    Date dates[] = { Date(16, July,    2019),
                     Date(16, August,  2019),
                     Date(15, October, 2019),
                     Date(15, January, 2020),
                     Date(16, April,   2020),
                     Date(16, July,    2020),
                     Date(16, July,    2021),
                     Date(18, July,    2022),
                     Date(17, July,    2023),
                     Date(16, July,    2024) };
    Real vols[] = { 0.0075, 0.0073, 0.0073, 
                    0.0072, 0.0073, 0.0072,
                    0.0070, 0.0072, 0.0070,
                    0.0075 };
    std::vector<Date> vd = std::vector<Date>(std::begin(dates),
                                             std::end(dates));
    std::vector<Real> vv = std::vector<Real>(std::begin(vols),
                                             std::end(vols));
    std::vector<Real> vr = std::vector<Real>(vv.size(), speed);
    return ext::make_shared<GeneralizedHullWhite>(yt, vd, vd, vr, vv);
};

// functor for equation solver
struct ghwBlackSolverImpl {
    ghwBlackSolverImpl(ext::shared_ptr<GeneralizedHullWhite> &model,
            ext::shared_ptr<BlackCalibrationHelper> &helper,
            Size index, bool fillUncalibrated):model_(model),
                helper_(helper), index_(index),
                size_(model_->FixedReversion().size() / 2),
                fillUncalibrated_(fillUncalibrated) {
    }

    Real operator()(Real vol) const {
        // size of the model
        Disposable<Array> params = model_->params();
        if (fillUncalibrated_) {
            for (Size i = index_; i < size_; i++)
                params[size_ + i] = vol;
        } else {
            params[size_ + index_] = vol;
        }
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
    bool fillUncalibrated_;
};

void calibrateGhw( ext::shared_ptr<GeneralizedHullWhite> &model,
        std::vector<ext::shared_ptr<BlackCalibrationHelper> >& helpers,
        bool fillUncalibrated) {
    // need to build a series of calibration helper.
    // when calibrate the spot vol, the european swaption is assumed.
    std::cout << "In calibrateGhw" << std::endl;

    for (Size i = 0; i < helpers.size(); i++) {
        ghwBlackSolverImpl solver(model, helpers[i], i, fillUncalibrated);
        Bisection bsolver;
        std::cout << "solve " << i << std::endl;
        Real root = bsolver.solve(solver, 1e-5, 0.0015, 0.001, 0.02);
    }

    std::cout << "GHW calibrated." << std::endl;
};


void printParams( ext::shared_ptr<GeneralizedHullWhite> &model ) {
    Disposable<Array> params = model->params();
    for (Size i = 0; i < params.size(); i++)
        std::cout << params[ i ] << std::endl;
}

ext::shared_ptr<PricingEngine> getQuantLibPricingEngine (
            QString model, QString engine,
            QString complexity, Size nHelpers,
            ext::shared_ptr<IborIndex> &liborIndex, double *bsVols,
            RelinkableHandle<YieldTermStructure> &fwdTermStructure,
            RelinkableHandle<YieldTermStructure> &discountTermStructure) {
    // setup calibration helpers
    std::vector<ext::shared_ptr<BlackCalibrationHelper> > bbgCalibrateSwaptions;
    for (Size i=0; i<nHelpers; i++) {
        ext::shared_ptr<Quote> vol(new SimpleQuote(bsVols[i]));
        bbgCalibrateSwaptions.push_back(
                    ext::shared_ptr<BlackCalibrationHelper>(new
                            SwaptionHelper(maturities[i],
                                           lengths[i],
                                           Handle<Quote>(vol),
                                           liborIndex,
                                           Period(6, Months),
                                           Thirty360(Thirty360::USA),
                                           Actual360(),
                                           discountTermStructure)));
    }

    if (model == "Hull-White One Factor") {
        if (complexity == QString::fromUtf8( "常函数" )) {
            ext::shared_ptr<HullWhite> bbgHW(
                        new HullWhite(fwdTermStructure, 0.03, 0.00727));

            std::vector<bool> bbgFixParam;
            bbgFixParam.push_back(true);
            bbgFixParam.push_back(false);
            for (Size i = 0; i < nHelpers; i++ ) {
                // set pricing engine
                bbgCalibrateSwaptions[i]->setPricingEngine(
                       ext::shared_ptr<PricingEngine>(
                            new FdHullWhiteSwaptionEngine(bbgHW)));
            }

            bbgCalibrateModel(
                    bsVols, bbgHW, bbgCalibrateSwaptions, bbgFixParam);
            std::cout << "Calibrated (with BBG vol) results: "
                      << "a = " << bbgHW->params()[0] << ", "
                      << "sigma = " << bbgHW->params()[1] << std::endl;

            return ext::shared_ptr<PricingEngine>(
                        new FdHullWhiteSwaptionEngine(bbgHW));
        } else {
            // calibrate piecewise Hull-White one factor
            // named generalized Hull White.
            ext::shared_ptr<GeneralizedHullWhite> bbgPiecewiseHW(buildGhw(
                            fwdTermStructure, 0.03));

            std::cout << "Calibrate piecewise Hull-White model..." << std::endl;
            for (Size i = 0; i < nHelpers; i++) {
                bbgCalibrateSwaptions[i]->setPricingEngine(ext::shared_ptr<PricingEngine>(
                            new TreeSwaptionEngine(bbgPiecewiseHW, 150,
                                    discountTermStructure)));
            }

            calibrateGhw(bbgPiecewiseHW, bbgCalibrateSwaptions, true);
            calibrateGhw(bbgPiecewiseHW, bbgCalibrateSwaptions, false);
            calibrateGhw(bbgPiecewiseHW, bbgCalibrateSwaptions, false);

            return ext::shared_ptr<PricingEngine>(
                    new TreeSwaptionEngine(
                            bbgPiecewiseHW, 500, discountTermStructure));
        }
} else {
        ext::shared_ptr<G2> g2(
                    new G2(fwdTermStructure, 0.049235,
                             0.00278221, 0.049235, 0.00916386, -0.650439));

        for (Size i=0; i<nHelpers; i++) {
            // set pricing engine
            bbgCalibrateSwaptions[i]->setPricingEngine(
                   ext::shared_ptr<PricingEngine>(
                        new G2SwaptionEngine(g2, 6, 100)));
        }
        calibrateG2Model(
                bsVols, g2, bbgCalibrateSwaptions, 0.05);
        std::cout << "Calibrated (with BBG vol) results: "
            << g2->params() << std::endl;
        return ext::shared_ptr<PricingEngine>(
                    new FdG2SwaptionEngine(g2, 500));
    }
}

double *extractExternalVols(std::vector<std::vector<double> > &volSurface) {
    double *vols = new double[10];
    vols[ 0 ] = volSurface[ 0 ][ 5 ] / 100;
    vols[ 1 ] = volSurface[ 1 ][ 5 ] / 100;
    vols[ 2 ] = volSurface[ 2 ][ 4 ] / 100;
    vols[ 3 ] = volSurface[ 3 ][ 4 ] / 100;
    vols[ 4 ] = volSurface[ 4 ][ 4 ] / 100;
    vols[ 5 ] = volSurface[ 5 ][ 3 ] / 100;
    vols[ 6 ] = volSurface[ 6 ][ 2 ] / 100;
    vols[ 7 ] = volSurface[ 7 ][ 1 ] / 100;
    vols[ 8 ] = volSurface[ 8 ][ 0 ] / 100;
    vols[ 9 ] = volSurface[ 9 ][ 0 ] / 100;

    for (Size i = 0; i < 10; i++) {
        std::cout << i << " " << vols[ i ] << std::endl;
    }

    return vols;
}

double priceSwaption(double notional,
        QString currency, std::string effectiveDate, std::string maturityDate, bool changeFirstExerciseDate, std::string firstExerciseDate,
        QString fixedDirection, double fixedCoupon, QString fixedPayFreq, std::string fixedDayCounter,
        QString floatDirection, QString floatIndex, QString floatPayFreq, std::string floatDayCounter,
        QString style, QString position, QString callFreq,
        std::string today, QString model, QString engine,
        QString complexity, QString curve, bool useExternalVolSurface,
        std::vector<std::vector<double> > &volSurface) {
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
    for (unsigned long i = 1; i < sizeof( oisRates ) / sizeof( oisRates[ 0 ] ); i++) {
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
    for (unsigned long i = 0; i < sizeof( futuresPrices ) / sizeof( futuresPrices[ 0 ] ); i++) {
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
    for (unsigned long i = 0; i < sizeof( swapQuotes ) / sizeof( swapQuotes[ 0 ] ); i++) {
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

    // if use external vol surface, read from user input.
    if (useExternalVolSurface) bsVols = extractExternalVols(volSurface);

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
    Date firstDate = DateParser::parseFormatted(firstExerciseDate, "%Y/%m/%d");

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

    // at the money swap
    Rate fixedATMRate = swap->fairRate();
    std::cout << fixedATMRate << std::endl;
    std::cout << swap->NPV() << std::endl;

    // construct swaption exercise
    ext::shared_ptr<Exercise> exercise = getQuantLibOptionExercise(style,
                swap, startDate, changeFirstExerciseDate, firstDate);
    Swaption swaption(swap, exercise);

    // pricing with generalized hull white for piece-wise term structure fit
    ext::shared_ptr<PricingEngine> pricingEngine = getQuantLibPricingEngine(
                model, engine, complexity,
                sizeof(oisDiscountingVols) / sizeof(oisDiscountingVols[0]),
                liborIndex, bsVols,
                forecastTermStructure,
                discountTermStructure);
    swaption.setPricingEngine(pricingEngine);

    std::cout << "Model price at " << swaption.NPV() << std::endl;

    // release memory if useExternalVolSurface
    if (useExternalVolSurface) delete [] bsVols;

    return swaption.NPV();
}
