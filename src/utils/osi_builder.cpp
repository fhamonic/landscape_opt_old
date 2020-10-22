#include "utils/osi_builder.hpp"

OSI_Builder::OSI_Builder() : nb_vars{0}, matrix(nullptr) {}
OSI_Builder::~OSI_Builder() {
    delete[] objective;
    delete[] col_lb;
    delete[] col_ub;
    delete matrix;
}

OSI_Builder & OSI_Builder::addVarType(VarType * var_type) {
    var_type->setOffset(this->nb_vars);
    varTypes.push_back(var_type);
    this->nb_vars += var_type->getNumber();
    return *this;
}
void OSI_Builder::init() {
    objective = new double[nb_vars];
    col_lb = new double[nb_vars];
    col_ub = new double[nb_vars];

    for(VarType * varType : varTypes) {
        const int offset = varType->getOffset();
        const int last_id = varType->getOffset() + varType->getNumber() - 1; 
        for(int i=offset; i<=last_id; i++) {
            assert(0<= i && i <= nb_vars);
            setObjective(i, 0);
            setBounds(i, varType->getDefaultLB(), varType->getDefaultUB());
        }
    }

    colNames.resize(nb_vars);

    matrix =  new CoinPackedMatrix(false, 2, 0);
    matrix->setDimensions(0, nb_vars);
}
OSI_Builder & OSI_Builder::setObjective(int var_id, double coef) {
    assert(coef == coef);
    objective[var_id] = coef;
    return *this;
}
OSI_Builder & OSI_Builder::setBounds(int var_id, double lb, double ub) {
    assert(lb == lb);
    assert(ub == ub);
    col_lb[var_id] = lb;
    col_ub[var_id] = ub;
    return *this;
}
OSI_Builder & OSI_Builder::buffEntry(int var_id, double coef) {
    assert(0<= var_id && var_id <= nb_vars);
    assert(coef == coef);
    row_indices_buffer.push_back(var_id);
    row_coeffs_buffer.push_back(coef);
    return *this;
}
OSI_Builder & OSI_Builder::popEntryBuffer() {
    row_indices_buffer.pop_back();
    row_coeffs_buffer.pop_back();
    return *this;
}
OSI_Builder & OSI_Builder::clearEntryBuffer() {
    row_indices_buffer.clear();
    row_coeffs_buffer.clear();
    return *this;
}
OSI_Builder & OSI_Builder::pushRowWithoutClearing(double lb, double ub) {
    assert(lb == lb);
    assert(ub == ub);
    matrix->appendRow(row_indices_buffer.size(), row_indices_buffer.data(), row_coeffs_buffer.data());
    row_lb.push_back(lb);
    row_ub.push_back(ub);
    return *this;
}
OSI_Builder & OSI_Builder::pushRow(double lb, double ub) {
    pushRowWithoutClearing(lb, ub);
    clearEntryBuffer();
    return *this;
}
OSI_Builder & OSI_Builder::setColName(int var_id, std::string name) {
    colNames[var_id] = name;
    return *this;
}
OSI_Builder & OSI_Builder::setContinuous(int var_id) {
    std::remove(integers_variables.begin(), integers_variables.end(), var_id);
    return *this;
}
OSI_Builder & OSI_Builder::setInteger(int var_id) {
    integers_variables.push_back(var_id);
    return *this;
}