#include "solvers/concept/restoration_plan_2.hpp"

RestorationPlan2::RestorationPlan2(const Landscape & l) {  }
RestorationPlan2::~RestorationPlan2() {  }
        
const Landscape & RestorationPlan2::getLandscape() const {  }


int RestorationPlan2::getNbNodes(Option i) const {  }
int RestorationPlan2::getNbArcs(Option i) const {  }
bool RestorationPlan2::contains(Option i, Graph_t::Node v) const {  }
bool RestorationPlan2::contains(Option i, Graph_t::Arc a) const {  }

bool RestorationPlan2::contains(Graph_t::Node v) const {  }
bool RestorationPlan2::contains(Graph_t::Arc a) const {  }

void RestorationPlan2::addNode(Option i, Graph_t::Node v, double quality_gain) {  }
void RestorationPlan2::addLink(Option i, Graph_t::Arc a, double restored_probability) {  }

void RestorationPlan2::removePatch(Option i, Graph_t::Node v) {  }
void RestorationPlan2::removeLink(Option i, Graph_t::Arc a) {  }

void RestorationPlan2::removePatch(Graph_t::Node v) {  }
void RestorationPlan2::removeLink(Graph_t::Arc a) {  }

double RestorationPlan2::getQualityGain(Option i, Graph_t::Node v) const {  }
double RestorationPlan2::getRestoredProbability(Option i, Graph_t::Arc a) const {  }

double RestorationPlan2::id(Option i, Graph_t::Node v) const {  }
double RestorationPlan2::id(Option i, Graph_t::Arc a) const {  }


RestorationPlan2::Option RestorationPlan2::addOption(double cost) {  }
void RestorationPlan2::setCost(Option i, double cost) {  }
double RestorationPlan2::getCost(Option i) const {  }

bool RestorationPlan2::containsOption(Option i) const {  }
void RestorationPlan2::removeOption(Option i) {  }
int RestorationPlan2::getNbOptions() const {  }

const std::map<RestorationPlan2::Option, double> & RestorationPlan2::getOptions(Graph_t::Node v) const {  }
const std::map<RestorationPlan2::Option, double> & RestorationPlan2::getOptions(Graph_t::Arc a) const {  }

void RestorationPlan2::cleanInvalidElements() {  }