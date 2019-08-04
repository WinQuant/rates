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

#include <ql/experimental/shortrate/generalizedhullwhite.hpp>

#include <iostream>
#include <iomanip>

using namespace QuantLib;

#if defined(QL_ENABLE_SESSONS)
namespace QuantLib {
    Integer sessionId() { return 0; }
}
#endif

#define NOTIONAL 10000000

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
    /*
    Date forwardDates[] = { 
        Date( 16, July, 2019 ),
        Date( 16, July, 2020 ), Date( 12, October, 2020 ),
        Date( 12, January, 2021 ), Date( 12, April, 2021 ),
        Date( 12, July, 2021 ),    Date( 12, October, 2021 ),
        Date( 12, January, 2022 ), Date( 12, April, 2022 ),
        Date( 12, July, 2022 ),    Date( 12, October, 2022 ),
        Date( 12, January, 2023 ), Date( 12, April, 2023 ),
        Date( 12, July, 2023 ),    Date( 12, October, 2023 ),
        Date( 12, January, 2024 ), Date( 12, April, 2024 ),
        Date( 12, July, 2024 ),    Date( 12, October, 2024 ),
        Date( 12, January, 2025 ), Date( 12, April, 2025 ),
        Date( 15, July, 2025 ),
        Date( 15, July, 2026 ), Date( 15, July, 2027 ),
        Date( 15, July, 2028 ), Date( 15, July, 2029 ),
        Date( 15, July, 2030 ), Date( 15, July, 2031 ),
        Date( 15, July, 2034 ), Date( 15, July, 2039 ),
        Date( 15, July, 2044 ), Date( 15, July, 2049 ),
        Date( 15, July, 2059 ), Date( 15, July, 2069 )
    };

    Rate forwardRates[] = {
        0.22222222,
        0.01722118, 0.01681929,
        0.01680328, 0.01642874,
        0.01736672, 0.01722198,
        0.01700362, 0.01679132,
        0.01789090, 0.01793672,
        0.01792175, 0.01791040,
        0.01887402, 0.01901328,
        0.01910846, 0.01920218,
        0.02010704, 0.02028457,
        0.02043685, 0.02059422,
        0.01872399,
        0.01910549, 0.01950800, 0.01991010,
        0.02030175, 0.02066999, 0.02096700,
        0.02166650, 0.02233445, 0.02257999,
        0.02266150, 0.02248000, 0.02216500
    };
    */

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
    /*
    Date discountingDates[] = { Date( 16, July, 2019 ),
        Date( 14, October, 2020 ),
        Date( 14, January, 2021 ), Date( 14, April, 2021 ),
        Date( 14, July, 2021 ),    Date( 14, October, 2021 ),
        Date( 14, January, 2022 ), Date( 14, April, 2022 ),
        Date( 14, July, 2022 ),    Date( 14, October, 2022 ),
        Date( 14, January, 2023 ), Date( 14, April, 2023 ),
        Date( 14, July, 2023 ),    Date( 14, October, 2023 ),
        Date( 14, January, 2024 ), Date( 14, April, 2024 ),
        Date( 14, July, 2024 ),    Date( 14, October, 2024 ),
        Date( 14, January, 2025 ), Date( 14, April, 2025 ),
        Date( 14, July, 2025 ),
        Date( 15, July, 2025 ),
        Date( 15, July, 2026 ), Date( 15, July, 2027 ),
        Date( 15, July, 2028 ), Date( 15, July, 2029 ),
        Date( 15, July, 2030 ), Date( 15, July, 2031 ),
        Date( 15, July, 2034 ), Date( 15, July, 2039 ),
        Date( 15, July, 2044 ), Date( 15, July, 2049 ),
        Date( 15, July, 2059 ), Date( 15, July, 2069 )
    };

    Rate discountingRates[] = { 1.0,
        0.975577,
        0.971405, 0.967346,
        0.963351, 0.959100,
        0.954904, 0.950869,
        0.946858, 0.942557,
        0.938266, 0.934090,
        0.929890, 0.925438,
        0.920975, 0.916560,
        0.912145, 0.907500,
        0.902834, 0.898258,
        0.893621,
        0.894150,
        0.875090, 0.855718, 0.836047, 0.816152, 0.796257, 0.776939,
        0.721127, 0.637235, 0.565823, 0.504040, 0.406721, 0.333739
    };
    */
    /*
Date discountingDates[] = {
    Date(26, July, 2019),    Date(28, October, 2019),
    Date(28, January, 2020), Date(28, April, 2020),
    Date(28, July, 2020),    Date(28, October, 2020),
    Date(28, January, 2021), Date(28, April, 2021),
    Date(28, July, 2021),    Date(28, October, 2021),
    Date(28, January, 2022), Date(28, April, 2022),
    Date(28, July, 2022),    Date(28, October, 2022),
    Date(28, January, 2023), Date(28, April, 2023),
    Date(28, July, 2023),    Date(28, October, 2023),
    Date(28, January, 2024), Date(28, April, 2024),
    Date(28, July, 2024),    Date(28, October, 2024),
    Date(28, January, 2025), Date(28, April, 2025),
    Date(28, July, 2025),
    Date(28, July, 2026),
    Date(28, July, 2027),
    Date(28, July, 2028),
    Date(28, July, 2029),
};

DiscountFactor discountingRates[] = {
    1.000000, 0.994115,
    0.988806, 0.983876,
    0.979440, 0.975200,
    0.971079, 0.967075,
    0.963124, 0.958951,
    0.954863, 0.950951,
    0.947081, 0.942912,
    0.938776, 0.934765,
    0.930746, 0.926450,
    0.922154, 0.917913,
    0.913680, 0.909189,
    0.904685, 0.900272,
    0.895803,
    0.877607,
    0.858793,
    0.839621,
    0.820294
};
    */

    std::vector<Date> dcd(std::begin(discountingDates), std::end(discountingDates));
    std::vector<Rate> dcr(std::begin(discountingRates), std::end(discountingRates));
    ext::shared_ptr<InterpolatedDiscountCurve<Cubic> > dcc =
            ext::make_shared<InterpolatedDiscountCurve<Cubic> >( dcd, dcr, Actual360());
    // show discount factor
    Size size = sizeof(discountingRates) / sizeof(discountingRates[0]);
    for (Size i=0; i<size; i++)
        std::cout << discountingDates[i] << ", " << dcc->zeroRate( discountingDates[i], Thirty360(), Simple ) << std::endl;

    return dcc;
}

/*
Date ghwDates[] = { Date(26, July,    2019),
                    Date(26, August,  2019),
                    Date(28, October, 2019),
                    Date(27, January, 2020),
                    Date(27, April,   2020),
                    Date(27, July,    2020),
                    Date(26, July,    2021),
                    Date(26, July,    2022),
                    Date(26, July,    2023),
                    Date(26, July,    2024),
                    Date(28, July,    2025)};
Real ghwVols[] = { 0.0072, 0.0070, 0.0072, 
                   0.0072, 0.0073, 0.0074,
                   0.0073, 0.0074, 0.0075,
                   0.0077, 0.0077 };
                   */
Date ghwDates[] = { Date(16, July,    2019),
                    Date(16, August,  2019),
                    Date(18, October, 2019),
                    Date(17, January, 2020),
                    Date(17, April,   2020),
                    Date(17, July,    2020),
                    Date(16, July,    2021),
                    Date(16, July,    2022),
                    Date(16, July,    2023),
                    Date(16, July,    2024)};
Real ghwVols[] = { 0.0075, 0.0073, 0.0073, 
                   0.0072, 0.0073, 0.0072,
                   0.0070, 0.0072, 0.0070,
                   0.0075 };
ext::shared_ptr<GeneralizedHullWhite> makeGhw(
            const Handle<YieldTermStructure> &yt, Real speed) {
    std::vector<Date> vd = std::vector<Date>(std::begin(ghwDates),
                                             std::end(ghwDates));
    std::vector<Real> vv = std::vector<Real>(std::begin(ghwVols),
                                             std::end(ghwVols));
    std::vector<Real> vr = std::vector<Real>(vv.size(), speed);
    return ext::make_shared<GeneralizedHullWhite>(yt, vd, vd, vr, vv);
};

// the vol is calibrated based on the bbg vol surface
Volatility bbgCalibrationDiagnalVols[] = {
    0.3654, 0.3594, 0.3700, 0.3671, 0.3627,
    0.3572, 0.3454, 0.3361, 0.3265, 0.3115
    // 0.2936, 0.3106, 0.3369, 0.3531, 0.3622,
    // 0.3855, 0.3926, 0.3859, 0.3884, 0.3109
    
    // 0.6878, 0.6766, 0.6828, 0.6765, 0.6747,
    // 0.6685, 0.6604, 0.6605, 0.6570, 0.6540
};
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
Period bbgCalibrationExpiries[] = {
    Period(1, Months), Period(3, Months), Period(6, Months),
    Period(9, Months), Period(1, Years),  Period(2, Years),
    Period(3, Years),  Period(4, Years),  Period(5, Years),
    Period(6, Years) };

// maturity grid
/*
Period bbgCalibrationMaturities[] = {
    Period(6, Years), Period(6, Years), Period(6, Years),
    Period(5, Years), Period(5, Years), Period(4, Years),
    Period(3, Years), Period(2, Years), Period(1, Years),
    Period(1, Years), Period(1, Years)
};
*/
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

void printParams( ext::shared_ptr<GeneralizedHullWhite> &model ) {
    Disposable<Array> params = model->params();
    for (Size i = 0; i < params.size(); i++)
        std::cout << params[ i ] << std::endl;
}

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

double priceSwaption() {
    Date todaysDate(16, July, 2019);
    Calendar calendar = TARGET();
    // Date settlementDate(18, July, 2019);
    Settings::instance().evaluationDate() = todaysDate;

    // discounting curve
    Handle<YieldTermStructure> bbgDiscountingCurve(
            makeTermStructures());
    Handle<YieldTermStructure> bbgFwdCurve(
            makeForwardCurve());

    // define the deal
    // deal property
    VanillaSwap::Type type = VanillaSwap::Receiver;

    // fixed convention
    Frequency fixedLegFrequency = Semiannual;
    BusinessDayConvention fixedLegConvention = ModifiedFollowing;
    // float convetion
    Frequency floatLegFrequency = Quarterly;
    BusinessDayConvention floatLegConvention = ModifiedFollowing;

    /*
    Date startDate = calendar.advance(todaysDate, 10, Days,
                            floatLegConvention);
    Date startDate = calendar.advance(settlementDate, 1, Years,
                            floatLegConvention);
    */
    Date startDate = Date( 15, July, 2020 );
    /*
    Date maturity  = calendar.advance(startDate, 5, Years,
                            floatLegConvention);
    */
    Date maturity = Date( 14, July, 2025 );

    // fixed leg property
    DayCounter fixedLegDayCounter = Thirty360(Thirty360::USA);
    Rate fixedRate = 0.02;
    Schedule fixedSchedule(startDate, maturity, Period(fixedLegFrequency),
                           calendar, fixedLegConvention, fixedLegConvention,
                           DateGeneration::Forward, false);

    // float leg property
    ext::shared_ptr<IborIndex> bbgIndex3M(
        new Libor("US0003M", Period(3, Months), 2, USDCurrency(), calendar,
                Actual360(), bbgFwdCurve));
    /*
    bbgIndex3M->addFixing(Date(15, July, 2019), 0.02266630, true);
    bbgIndex3M->addFixing(Date(12, July, 2019), 0.02266630, true);
    bbgIndex3M->addFixing(Date(11, July, 2019), 0.02266630, true);
    bbgIndex3M->addFixing(Date(10, July, 2019), 0.02266630, true);
    */
    Schedule floatSchedule(startDate, maturity, Period(floatLegFrequency),
                           calendar, floatLegConvention, floatLegConvention,
                           DateGeneration::Forward, false);

    // TODO discount factor consistent?
    // No...

    ext::shared_ptr<VanillaSwap> swap(new VanillaSwap(
        type, NOTIONAL,
        fixedSchedule, fixedRate, fixedLegDayCounter,
        floatSchedule, bbgIndex3M, 0.0, /* spread */
        bbgIndex3M->dayCounter()));

    // build the at the money swap
    swap->setPricingEngine(ext::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(bbgDiscountingCurve)));
    // at the money rate
    Rate fixedAtmRate = swap->fairRate();
    std::cout << "ATM rate: " << std::setw(7) << fixedAtmRate << std::endl;
        ext::shared_ptr<VanillaSwap> atmSwap(new VanillaSwap(
            type, 10000000.0,
            fixedSchedule, fixedAtmRate, fixedLegDayCounter,
            floatSchedule, bbgIndex3M, 0.0,
            bbgIndex3M->dayCounter()));

    // Bermudan swaption
    // construct coupon dates
    std::vector<Date> bbgBermudanDates;
    const std::vector<ext::shared_ptr<CashFlow> >& bbgLeg =
        swap->floatingLeg();
    // bbgBermudanDates.push_back(Date( 16, October, 2019 ));
    // bbgBermudanDates.push_back(Date( 16, January, 2020 ));
    // bbgBermudanDates.push_back(Date( 16, April, 2020 ));
    // bbgBermudanDates.push_back(Date( 16, July, 2020 ));
    for (Size i=0; i<bbgLeg.size(); i++) {
        ext::shared_ptr<Coupon> coupon =
            ext::dynamic_pointer_cast<Coupon>(bbgLeg[i]);
        bbgBermudanDates.push_back(coupon->accrualStartDate());
        std::cout << coupon->accrualStartDate() << std::endl;
    }

    ext::shared_ptr<Exercise> bbgBermudanExercise(
                         new BermudanExercise(bbgBermudanDates));
    ext::shared_ptr<Exercise> europeanExercise(
            new EuropeanExercise(startDate));
    Swaption bbgBermudanSwaption(swap, bbgBermudanExercise);
    // Swaption bbgBermudanSwaption(atmSwap, europeanExercise);

    // set Hull-white one factor model
    ext::shared_ptr<HullWhite> bbgHW(new HullWhite(bbgDiscountingCurve,
                0.03, 0.0073));
    bbgBermudanSwaption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new FdHullWhiteSwaptionEngine(bbgHW)));

    std::cout << std::endl;
    std::cout << "Pricing with Hull-white 1 factor: "
              << "a = " << bbgHW->params()[0] << ", "
              << "sigma = " << bbgHW->params()[1] << std::endl
              << "Price at " << bbgBermudanSwaption.NPV() << std::endl;

    /*
    // pricing with generalized hull white for piece-wise term structure fit
    ext::shared_ptr<GeneralizedHullWhite> bbgPiecewiseHW(makeGhw(
                    bbgFwdCurve, 0.03));
    */
    /*
    bbgBermudanSwaption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new JamshidianSwaptionEngine(bbgPiecewiseHW, bbgDiscountingCurve)));
    std::cout << "Jamshidian price at " << bbgBermudanSwaption.NPV() << std::endl;
    */

    /*
    bbgBermudanSwaption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(bbgPiecewiseHW, 500, bbgDiscountingCurve)));
    std::cout << "Piecewise Hull-White price at " << bbgBermudanSwaption.NPV() << std::endl;

    std::cout << std::endl
              << "Calibrate piecewise Hull-white..." << std::endl;

    std::vector<ext::shared_ptr<BlackCalibrationHelper> > calibrationSwaptions;
    Size calibrationSize =
        sizeof(bbgCalibrationExpiries) / sizeof(bbgCalibrationExpiries[0]);
    for (Size i = 0; i < calibrationSize; i++) {
        ext::shared_ptr<Quote> vol(
                    new SimpleQuote(bbgCalibrationDiagnalVols[i]));
        calibrationSwaptions.push_back(ext::shared_ptr<BlackCalibrationHelper>(new
                SwaptionHelper(bbgCalibrationExpiries[i],
                               bbgCalibrationMaturities[i],
                               Handle<Quote>(vol),
                               bbgIndex3M,
                               bbgIndex3M->tenor(),
                               fixedLegDayCounter,
                               bbgIndex3M->dayCounter(),
                               bbgFwdCurve,
                               BlackCalibrationHelper::RelativePriceError,
                               fixedAtmRate)));
        calibrationSwaptions[i]->setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new TreeSwaptionEngine(bbgPiecewiseHW, 500,
                            bbgDiscountingCurve)));
    }

    // calibrateGhw(bbgPiecewiseHW, calibrationSwaptions);
    std::cout << "GHW calibrated." << std::endl;
    // Output the implied Black volatilities
    for (Size i=0; i<calibrationSwaptions.size(); i++) {
        Real npv = calibrationSwaptions[i]->modelValue();
        Volatility implied = calibrationSwaptions[i]->impliedVolatility(npv, 1e-4, 1000, 1e-8, 100);
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

    // output calibrated vol
    Size n = calibrationSwaptions.size();
    std::cout << n << " " << bbgPiecewiseHW->params().size() << std::endl;
    for (Size i = 0; i < n; i++) {
        Real diff = bbgPiecewiseHW->params()[n + i] - ghwVols[i];
        std::cout << ghwDates[ i ]
                  << ", a = " << bbgPiecewiseHW->params()[i]
                  << ", vol = " << std::setw(7)
                  << io::volatility(bbgPiecewiseHW->params()[n + i])
                  << ", bbgVol = " << std::setw(7)
                  << io::volatility(ghwVols[i])
                  << " (" << std::setw(7) << std::showpos
                  << io::volatility(diff) << std::noshowpos
                  << ")" << std::endl;
    }

    std::cout << "Piecewise Hull-White price after calibration at " << bbgBermudanSwaption.NPV() << std::endl;
    */

    return bbgBermudanSwaption.NPV();
}
