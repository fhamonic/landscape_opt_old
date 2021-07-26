#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <cassert> // assert
#include <string> // std::string
#include <list> // std::list
#include <map> // std::map
#include <vector> // std::vector

#include <algorithm> // std::sort , for_each ...
#include <execution> // execution::par

#include "landscape/mutable_landscape.hpp"

#include "solvers/concept/solution.hpp"
#include "solvers/concept/restoration_plan.hpp"

#include "utils/chrono.hpp"

namespace concepts {
    class Solver {
        protected:
            class Param {
                public:
                    virtual ~Param() {};
                    virtual void parse(const char * arg)=0;
                    virtual void set(bool v)=0;
                    virtual void set(int v)=0;
                    virtual void set(double v)=0;
                    virtual bool getBool() const=0;
                    virtual int getInt() const=0;
                    virtual double getDouble() const=0;
                    virtual std::string toString() const=0;
            };
            class IntParam : public Param {
                protected:
                    int value;
                public:
                    IntParam(int v) : value(v) {};
                    void parse(const char * arg) { value = std::atoi(arg); };
                    void set(bool v) { value = (v ? 1 : 0); };
                    void set(int v) { value = v; };
                    void set(double v) { value = v; };
                    bool getBool() const { return value != 0; };
                    int getInt() const { return value; };
                    double getDouble() const { return value; };
                    std::string toString() const { return std::to_string(value); }
            };
            class DoubleParam : public Param {
                protected:
                    double value;
                public:
                    DoubleParam(double v) : value(v) {};
                    void parse(const char * arg) { value = std::atof(arg); };
                    void set(bool v) { value = (v ? 1 : 0); };
                    void set(int v) { value = v; };
                    void set(double v) { value = v; };
                    bool getBool() const { return value != 0.0; };
                    int getInt() const { return value; };
                    double getDouble() const { return value; };
                    std::string toString() const { return std::to_string(value); }
            };

            std::map<std::string, Param*> params;
        public:
            virtual ~Solver() {
                for(std::pair<std::string, Param*> element : params)
                    delete element.second;
            };

            bool setParam(std::string name, const char * arg) {
                if(params.find(name) == params.end())
                    return false;
                params[name]->parse(arg);
                return true;
            };
            const std::map<std::string, Param*> & getParams() const {
                return params;
            };

            virtual Solution solve(const MutableLandscape & landscape, const RestorationPlan<MutableLandscape>& plan, const double B) const=0;

            virtual const std::string name() const=0;

            std::string toString() const { 
                std::string s = name(); 
                for(std::pair<std::string, Param*> element : params) {
                    s += "_" + element.first + "=" + element.second->toString();
                }
                return s;
            } 
    };
}

#include "helper.hpp"

#endif //SOLVER_HPP