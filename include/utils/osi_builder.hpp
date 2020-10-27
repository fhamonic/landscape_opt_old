/**
 * @file osi_builder.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief OSI_Builder class declaration
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 */
#define OSI_BUILDER_HPP

#include<functional>
#include<vector>

#include "coin/OsiClpSolverInterface.hpp"
#include "coin/OsiCbcSolverInterface.hpp" // deprecated
#include "coin/OsiGrbSolverInterface.hpp"

#include "coin/CbcModel.hpp" // deprecated
#include "coin/CbcSolver.hpp"

#include "coin/CoinPackedMatrix.hpp"

/**
 * @brief A practical class for building OsiSolver instances 
 */
class OSI_Builder {
    public:
        static const int MIN = 1;
        static const int MAX = -1;
        static constexpr double INFTY = std::numeric_limits<double>::max();

        class VarType {
            protected:
                int _number;
                int _offset;
                double _default_lb;
                double _default_ub;
                bool _integer;
                VarType(int number, double lb=0, double ub=INFTY, bool integer=false) : _number(number), _offset(0), _default_lb(lb), _default_ub(ub), _integer(integer) {}
                VarType() : VarType(0) {}
            public:
                void setOffset(int offset) { _offset = offset; }
                int getOffset() { return _offset; }
                int getNumber() const { return _number; }
                double getDefaultLB() { return _default_lb; }
                double getDefaultUB() { return _default_ub; }
                bool isInteger() { return _integer; }
        };
    private:
        int nb_vars;
        
        std::vector<VarType*> varTypes;

        double * objective;
        double * col_lb;
        double * col_ub;

        std::vector<int> row_indices_buffer;
        std::vector<double> row_coeffs_buffer;
        std::vector<double> row_lb;
        std::vector<double> row_ub;

        std::vector<int> integers_variables;

        OsiSolverInterface::OsiNameVec colNames;

        CoinPackedMatrix * matrix;
    public:
        OSI_Builder();
        ~OSI_Builder();

        int getNbVars() const { return nb_vars; };
        int getNbConstraints() const { return matrix->getNumRows(); };

        OSI_Builder & addVarType(VarType * var_type);
        void init();
        OSI_Builder & setObjective(int  var_id, double coef);
        OSI_Builder & setBounds(int  var_id, double lb, double ub);
        OSI_Builder & buffEntry(int  var_id, double coef);
        OSI_Builder & popEntryBuffer();
        OSI_Builder & clearEntryBuffer();
        OSI_Builder & pushRowWithoutClearing(double lb, double ub);
        OSI_Builder & pushRow(double lb, double ub);
        OSI_Builder & setColName(int var_id, std::string name);

        OSI_Builder & setContinuous(int var_id);
        OSI_Builder & setInteger(int var_id);
        
        template <class OsiSolver>
        OsiSolver * buildSolver(int sense, bool relaxed=false) {
            OsiSolver * solver = new OsiSolver();
            solver->loadProblem(*matrix, col_lb, col_ub, objective, row_lb.data(), row_ub.data());
            solver->setObjSense(sense);
            if(relaxed)
                return solver;
            for(int i : integers_variables) 
                solver->setInteger(i);
            solver->setColNames(colNames, 0, nb_vars, 0);
            return solver;
        }

        static int nb_pairs(int n) {
            return n*(n-1)/2;
        };
        static int nb_couples(int n) {
            return 2*nb_pairs(n);
        };

        static int compose_pair(int i, int j) {
            assert(i != j);
            if(i > j) std::swap(i,j);
            return nb_pairs(j)+i;
        };
        static std::pair<int,int> retrieve_pair(int id) {
            const int j = std::round(std::sqrt(2*id+1));
            const int i = id - nb_pairs(j);
            return std::pair<int,int>(i,j);
        };
        static int compose_couple(int i, int j) {
            return 2*compose_pair(i, j) + (i < j ? 0 : 1);
        };
        static std::pair<int,int> retrieve_couple(int id) {
            std::pair<int,int> p = retrieve_pair(id / 2);
            if(id % 2 == 1) std::swap(p.first, p.second);
            return p;
        };


};

#endif //OSICLPSOLVER_BUILDER_HPP