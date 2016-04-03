#pragma once

#include <deque>

#include "model.hpp"
#include "data.hpp"

namespace IOSKJ {

/**
 * Abstract base class for all management procedures
 */
class Procedure {
 public:
    virtual void reset(void){};
    virtual void operate(uint time, Model& model) = 0;
    virtual void read(std::istream& stream){};
    virtual void write(std::ostream& stream) = 0;
};

/**
 * Class for lagging management control as in the real world
 *
 * e.g. usually outputs need be be lagged by two years from data: 
 *  data from `year`, MP operated in `year+1`, to set management control in `year+2`
 *
 * Because the procedure is operated in `year`, before the model is updated, it is
 * using "data" (ie. model state) from `year-1`, so an actual two year lag is represented 
 * here using lag = 1.
 */
class Lagger : public std::deque<double> {
 public:

    /**
     * Set the number of years in lag
     */
    void set(int lag){
        // Fill with NANs
        clear();
        resize(lag,NAN);
    }

    /**
     * Push a new value on to the lag queue and pop
     * one off. This should be called in every year and will
     * return `NAN` for first `lag` years.
     */
    double push_pop(double current){
        push_back(current);
        auto lagged = front();
        pop_front();
        return lagged;
    }
};


/**
 * `DoNothing` management procedure
 *
 * A management procedure that does nothing; used for testing
 */
class DoNothing : public Procedure, public Structure<DoNothing> {
public:

    virtual void write(std::ostream& stream){
        stream
            <<"DoNothing"<<"\t\t\t\t\t\t\t\t\t\t\n";
    }

    virtual void operate(uint time, Model& model){
    }
};


/**
 * `HistCatch` management procedure
 *
 * A management procedure based on the historical catch
 */
class HistCatch : public Procedure, public Structure<HistCatch> {
public:

    Array<Variable<Fixed>,Year,Quarter,Region,Method> catches;

    HistCatch(void){
        // Read in historical catches (borrowed from parameters)
        catches.read("parameters/input/catches.tsv",true);
    }

    virtual void write(std::ostream& stream){
        stream
            <<"HistCatch"<<"\t\t\t\t\t\t\t\t\t\t\n";
    }

    virtual void operate(uint time, Model& model){
        // Apply the actual quarterly catch history
        // using 2012 catch distribution by quarter, region, method
        // etc for years in the future
        uint year = IOSKJ::year(time);
        if(year>2014) year = 2014;
        uint quarter = IOSKJ::quarter(time);
        model.exploit = model.exploit_catch;
        for(auto region : regions){
            for(auto method : methods){
                model.catches(region,method) = catches(year,quarter,region,method);
            }
        }
    }
};


/**
 * Constant catch management procedure
 *
 * Used as a illustrative reference case
 */
class ConstCatch : public Procedure, public Structure<ConstCatch> {
public:

    /**
     * Total allowable catch (quarterly)
     *
     * This is the mean catch over last 5 years (t) (2009–2013) from Table 7
     * of IOTC–2014–WPTT16
     */
    double tac = 429564.0/4;

    virtual void read(std::istream& stream){
        stream
            >>tac;
    }

    virtual void write(std::ostream& stream){
        stream
            <<"ConstCatch\t"<<tac<<"\t\t\t\t\t\t\t\t\t\n";
    }

    virtual void operate(uint time, Model& model){
        model.catches_set(tac);
    }
};

/**
 * Constant effort management procedure
 *
 * Used as a illustrative reference case
 */
class ConstEffort : public Procedure, public Structure<ConstEffort> {
public:

    /**
     * Total allowable effort (quarterly)
     *
     * Nominal number of effort units for each region/method
     */
    double tae = 100;

    virtual void write(std::ostream& stream){
        stream
            <<"ConstEffort\t"<<tae<<"\t\t\t\t\t\t\t\t\t\n";
    }

    virtual void operate(uint time, Model& model){
        model.effort_set(tae);
    }
};


/**
 * `Mald2016` management procedure
 *
 * The "HCR" propose by the Maldives for the 2016 commission meeting.
 *
 * This is similar to `BRule` but instead of having a target F, uses a 
 * target exploitation rate, has a maximum catch, and applies a catch limit
 * rather than a F to the model.
 */
class Mald2016 : public Procedure, public Structure<Mald2016> {
public:

    // The following values for the control parameters are 
    // "reference case" only.

    /**
     * Frequency of stock status estimates
     */
    int frequency = 3;

    /**
     * Precision with which stock status is estimated
     */
    double precision = 0.2;

    /**
     * Maximum exploitation rate
     */
    double emax = 1;

    /**
     * Threshold stock status (below which exploitation rate is reduced)
     */
    double thresh = 0.4;

    /**
     * Closure stock status (below which catch is zero)
     */
    double closure = 0.1;

    /**
     * Maximum catch level
     */
    double cmax = 600;

    /**
     * Implementation lag (years from data to implementation)
     */
    int lag = 2;
    Lagger lagger;

    /**
     * Reflection for input/output
     */
    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(frequency,"frequency")
            .data(precision,"precision")
            .data(emax,"emax")
            .data(thresh,"thresh")
            .data(closure,"closure")
            .data(cmax,"cmax")
        ;
    }

    /**
     * Read from input stream
     */
    void read(std::istream& stream){
        stream
            >>frequency
            >>precision
            >>emax
            >>thresh
            >>closure
            >>cmax;
    }

    /**
     * Write to output stream
     */
    void write(std::ostream& stream){
        stream
            <<"Mald2016"<<"\t"
            <<frequency<<"\t"
            <<precision<<"\t"
            <<emax<<"\t"
            <<thresh<<"\t"
            <<closure<<"\t"
            <<cmax<<"\t\t\t\t\n";
    }

    /**
     * Reset this management procedure
     */
    virtual void reset(void){
        lagger.set(lag);
        last_ = -1;
    }

    /**
     * Operate this management procedure
     */
    virtual void operate(uint time, Model& model){
        int year = IOSKJ::year(time);
        int quarter = IOSKJ::quarter(time);
        if(quarter==0){
            double catches;

            if(last_<0 or year-last_>=frequency) {
                // Get b_curr and b_0 (sum over all regions)
                double bcurr = sum(model.biomass_spawners);
                double b0 = sum(model.biomass_spawners_unfished);
                
                // Apply imprecision to simulate stock
                // assessment estimation
                Lognormal imprecision(1,precision);
                bcurr *= imprecision.random();
                b0 *= imprecision.random();
                            
                // Calculate recommended exploitation rate
                double status = bcurr/b0;
                double exprate;
                if(status<closure) exprate = 0;
                else if(status>thresh) exprate = emax;
                else exprate = emax/(thresh-closure)*(status-closure);
                
                // Calculate and apply catch limit...
                // ...convert annual exp rate to a quarterly exp rate
                auto f_annual = - std::log(1-exprate);
                auto exprate_quarterly = 1 - std::exp(-f_annual/4);
                // ...calculate quarterly catches and ensure not
                // above the annual limit
                catches = exprate_quarterly * bcurr;
                if (catches*4 > cmax) {
                    catches = cmax/4;
                }

                // Store year so know when to do this again
                last_ = year;
            } else {
                catches = NAN;
            }

            // Move a long the lag queue
            double catches_lagged = lagger.push_pop(catches);
            if(not std::isnan(catches_lagged)) {
                model.catches_set(catches_lagged);
            }
        }
    }

private:
    /**
     * Last time that the status estimate was made
     */
    int last_ = -1;

};


/**
 * `BRule` management procedure
 */
class BRule : public Procedure, public Structure<BRule> {
public:

    /**
     * Frequency of stock status estimates
     */
    int frequency = 2;

    /**
     * Precision with which B (stock status) is estimated
     */
    double precision = 0.1;

    /**
     * Target (maximum) F (fishing mortality)
     */
    double target;

    /**
     * Threshold B (stock status) below which F is reduced
     */
    double thresh;

    /**
     * Limit B (stock status) below which F is 0
     */
    double limit;

    /**
     * Reflection
     */
    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(frequency,"frequency")
            .data(precision,"precision")
            .data(target,"target")
            .data(thresh,"thresh")
            .data(limit,"limit")
        ;
    }

    void read(std::istream& stream){
        stream
            >>frequency
            >>precision
            >>target
            >>thresh
            >>limit;
    }

    void write(std::ostream& stream){
        stream
            <<"BRule"<<"\t"
            <<frequency<<"\t"
            <<precision<<"\t"
            <<target<<"\t"
            <<thresh<<"\t"
            <<limit<<"\t\t\t\t\t\n";
    }

    virtual void reset(void){
        last_ = -1;
    }

    virtual void operate(uint time, Model& model){
        int year = IOSKJ::year(time);
        int quarter = IOSKJ::quarter(time);
        if(quarter==0 and (last_<0 or year-last_>=frequency)){
            // Get stock status
            double b = model.biomass_status();
            // Add imprecision
            Lognormal imprecision(1,precision);
            b *= imprecision.random();
            // Calculate F
            double f;
            if(b<limit) f = 0;
            else if(b>thresh) f = target;
            else f = target/(thresh-limit)*(b-limit);
            // Apply F
            model.fishing_mortality_set(f);
            // Update last
            last_ = year;
        }
    }

private:
    /**
     * Last time that the status estimate was made
     */
    int last_ = -1;
};

/**
 * `FRange` management procedure
 */
class FRange : public Procedure, public Structure<FRange> {
public:

    /**
     * Frequency of exp. rate estimates
     */
    int frequency = 2;

    /**
     * Precision of exp. rate estimate
     */
    double precision = 0.1;

    /**
     * Target exp. rate
     */
    double target;

    /**
     * Buffer around target exp. rate
     */
    double buffer;

    /**
     * Restriction on multiplicative changes
     * in effort
     */
    double change_max = 0.3;

    /**
     * Reflection
     */
    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(frequency,"frequency")
            .data(precision,"precision")
            .data(target,"target")
            .data(buffer,"buffer")
            .data(change_max,"change_max")
        ;
    }

    void write(std::ostream& stream){
        stream
            <<"FRange"<<"\t"
            <<frequency<<"\t"
            <<precision<<"\t"
            <<target<<"\t"
            <<buffer<<"\t"
            <<change_max<<"\t\t\t\t\t\n";
    }

    virtual void reset(void){
        last_ = -1;
        effort_ = 100;
    }

    virtual void operate(uint time, Model& model){
        int year = IOSKJ::year(time);
        int quarter = IOSKJ::quarter(time);
        if(quarter==0 and (last_<0 or year-last_>=frequency)){
            // Get an estimate of exploitation rate
            double f = model.exploitation_rate_get();
            // Add imprecision
            Lognormal imprecision(1,precision);
            f *= imprecision.random();
            // Check to see if F is outside of range
            if(f<target-buffer or f>target+buffer){
                // Calculate ratio between current estimated F and target
                double adjust = target/f;
                if(adjust>(1+change_max)) adjust = 1+change_max;
                else if(adjust<1/(1+change_max)) adjust = 1/(1+change_max);
                // Adjust effort
                effort_ *= adjust;
                model.effort_set(effort_);
            }
            // Update last
            last_ = year;
        }
    }

private:

    /**
     * Time that the last exp. rate estimate was made
     */
    int last_;

    /**
     * Total allowable effort
     */
    double effort_;
};

/**
 * `IRate` management procedure
 */
class IRate : public Procedure, public Structure<IRate> {
public:

    /**
     * Precision of CPUE in reflecting vulnerable biomass
     */
    double precision = 0.2;

    /**
     * Degree of smoothing of biomass index
     */
    double responsiveness = 1;

    /**
     * Target harvest rate
     */
    double multiplier = 400;

    /**
     * Threshold biomass index
     */
    double threshold = 0.3;

    /**
     * Limit biomass index
     */
    double limit = 0.1;

    /**
     * Maximum change
     */
    double change_max = 0.3;

    /**
     * Maximum TAC
     */
    double maximum = 600;

    /**
     * Reflection
     */
    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(precision,"precision")
            .data(responsiveness,"responsiveness")
            .data(multiplier,"multiplier")
            .data(threshold,"threshold")
            .data(limit,"limit")
            .data(change_max,"change_max")
            .data(maximum,"maximum")
        ;
    }

    void read(std::istream& stream){
        stream
            >>precision
            >>responsiveness
            >>multiplier
            >>threshold
            >>limit
            >>change_max
            >>maximum;
    }

    void write(std::ostream& stream){
        stream
            <<"IRate"<<"\t"
            <<precision<<"\t"
            <<responsiveness<<"\t"
            <<multiplier<<"\t"
            <<threshold<<"\t"
            <<limit<<"\t"
            <<change_max<<"\t"
            <<maximum<<"\t\t\t\n";
    }

    virtual void reset(void){
        last_ = -1;
        index_ = -1;
    }

    virtual void operate(uint time, Model& model){
        int quarter = IOSKJ::quarter(time);
        // Operate once per year in the third quarter
        if(quarter==0){
            // Get CPUE as a combination of WE/PS and MA/PL
            GeometricMean combined;
            combined.append(model.cpue(WE,PS));
            combined.append(model.cpue(MA,PL));
            double cpue = combined;
            // Add observation error
            Lognormal imprecision(1,precision);
            cpue *= imprecision.random();
            // Update smoothed index
            if(index_==-1) index_ = cpue;
            else index_ = responsiveness*cpue + (1-responsiveness)*index_;
            // Calculate recommended harvest rate
            double rate;
            if(index_<limit) rate = 0;
            else if(index_>threshold) rate = multiplier;
            else rate = multiplier/(threshold-limit)*(index_-limit);
            // Calculate recommended TAC
            double tac = std::min(rate*cpue,maximum);
            // Restrict changes in TAC
            if(last_!=-1){
                double change = tac/last_;
                double max = 1 + change_max;
                if(change>max) change = max;
                else if(change<1/max) change = 1/max;
                tac = last_*change;
            }
            last_ = tac;
            // Apply recommended TAC
            model.catches_set(tac*1000/4);
        }
    }

private:

    /**
     * Smoothed biomass index
     */
    double index_;

    /**
     * Last TAC value
     */
    double last_;
};


class Procedures : public Array<Procedure*> {
public:

    void populate(void){

        append(new ConstCatch);
        append(new ConstEffort);

        // Mald2016
        {
            // Reference case 
            auto& ref = * new Mald2016;
            ref.frequency = 3;
            ref.precision = 0.1;
            ref.emax = 0.3;
            ref.thresh = 0.4;
            ref.closure = 0.1;
            ref.cmax = 700000;
            append(&ref);

            // 'Frontier case'
            // Every year, perfect estimates, not reduction
            // in exploitation rate.
            // To demonstrate the performance frontier.
            for(double emax=0.15; emax<0.7; emax+=0.05){
                auto& proc = * new Mald2016;
                proc.frequency = 1;
                proc.precision = 1e-6;
                proc.emax = emax;
                proc.thresh = 0;
                proc.closure = 0;
                proc.cmax = 1e6;
                append(&proc);
            }

            // Reference case with variation in `emax`
            for(double emax=0.15; emax<0.7; emax+=0.05){
                auto& proc = * new Mald2016(ref);
                proc.emax = emax;
                append(&proc);
            }
        }

        return;

        for(auto frequency : {3}){
            for(auto precision : {0.1}){
                for(auto emax : {0.05,0.1,0.15,0.2}){
                    for(auto thresh : {0.3, 0.4, 0.5}){
                        for(auto closure : {0.0, 0.1, 0.2}){
                            for(auto cmax : {600000, 700000}){
                                auto& proc = * new Mald2016;
                                proc.frequency = frequency;
                                proc.precision = precision;
                                proc.emax = emax;
                                proc.thresh = thresh;
                                proc.closure = closure;
                                proc.cmax = cmax;
                                append(&proc);
                            }
                        }
                    }
                }
            }
        }

        #if 0
        // BRule
        {
            auto& proc = * new BRule;
            proc.frequency = 2;
            proc.precision = 0.2;
            proc.target = 0.25;
            proc.thresh = 0.3;
            proc.limit = 0.05;
            append(&proc);
        }
        for(int frequency : {2}){
            for(double precision : {0.2}){
                for(auto target : {0.2,0.25,0.3}){
                    for(auto thresh : {0.2,0.3,0.4}){
                        for(auto limit : {0.025,0.05,0.1}){
                            auto& proc = * new BRule;
                            proc.frequency = frequency;
                            proc.precision = precision;
                            proc.target = target;
                            proc.thresh = thresh;
                            proc.limit = limit;
                            append(&proc);
                        }
                    }
                }
            }
        }
        #endif

        #if 0
        // FRange
        {
            auto& proc = * new FRange;
            proc.frequency = 3;
            proc.precision = 0.1;
            proc.target = 0.25;
            proc.buffer = 0.05;
            proc.change_max = 0.3;
            append(&proc);
        }
        for(int frequency : {2,5,7}){
            for(double precision : {0.2}){
                for(auto target : {0.2,0.25,0.3}){
                    for(auto buffer : {0.01,0.02,0.05}){
                        auto& proc = * new FRange;
                        proc.frequency = frequency;
                        proc.precision = precision;
                        proc.target = target;
                        proc.buffer = buffer;
                        proc.change_max = 0.4;
                        append(&proc);
                    }
                }
            }
        }
        #endif

        #if 0
        // IRate
        {
            auto& proc = * new IRate;
            proc.responsiveness = 0.6;
            proc.multiplier = 100000;
            proc.threshold = 0.4;
            proc.limit = 0.1;
            proc.change_max = 0.3;
            append(&proc);
        }
        for(double responsiveness : {0.5}){
            for(double multiplier : {100000,120000,140000}){
                for(auto threshold : {0.5, 0.6, 0.7}){
                    for(auto limit : {0.05, 0.1,0.2}){
                        auto& proc = * new IRate;
                        proc.precision = 0.2;
                        proc.responsiveness = responsiveness;
                        proc.multiplier = multiplier;
                        proc.threshold = threshold;
                        proc.limit = limit;
                        proc.change_max = 0.4;
                        proc.maximum = 150000;
                        append(&proc);
                    }
                }
            }
        }
        #endif
    }

    void reset(int procedure){
        operator[](procedure)->reset();
    }

    void operate(int procedure, uint time, Model& model){
        operator[](procedure)->operate(time,model);
    }

    void read(const std::string& path = "procedures/input/procedures.tsv"){
        std::ifstream file(path);
        std::string line;
        std::getline(file,line);//header
        while(std::getline(file,line)){
            std::istringstream stream(line);
            std::string clas;
            stream>>clas;
            if(clas=="HistCatch"){
                auto proc = new HistCatch;
                append(proc);
            } else if(clas=="ConstCatch"){
                auto proc = new ConstCatch;
                proc->read(stream);
                append(proc);
            } else if(clas=="BRule"){
                auto proc = new BRule;
                proc->read(stream);
                append(proc);
            } else if(clas=="IRate"){
                auto proc = new IRate;
                proc->read(stream);
                append(proc);
            } else {
                throw std::runtime_error("Unknown procedure class: "+clas);
            }
        }
    }

    void write(const std::string& path = "procedures/output/procedures.tsv"){
        std::ofstream file(path);
        file<<"procedure\tclass\tp1\tp2\tp3\tp4\tp5\tp6\tp7\tp8\tp9\tp10\n";
        int index = 0;
        for(Procedure* procedure : *this) {
            file<<index++<<"\t";
            procedure->write(file);
        }
    }
};

}
