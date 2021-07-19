#ifndef ECAREFF_HPP
#define ECAREFF_HPP

#include "indices/concept/connectivity_index.hpp"
#include "indices/eca.hpp"

#include "lemon/dijkstra.h"
#include "lemon/connectivity.h"

class ECAReff : public concepts::ConnectivityIndex {
    private:
        double exponent;
    public:
        ECAReff(double exponent=2) : exponent(exponent) {};
        ~ECAReff() {};

        template <typename GR, typename DM>
        typename GR::template NodeMap<typename GR::template NodeMap<double> *> * getReff(const GR & g, const DM & resistance);
   
        template <typename GR, typename QM, typename DM, typename CM>
        double eval(const concepts::AbstractLandscape<GR, QM, DM, CM> & landscape);
};

template <typename GR, typename DM>
typename GR::template NodeMap<typename GR::template NodeMap<double> *> * ECAReff::getReff(const GR & g, const DM & resistance) {
    
    typedef typename GR::template NodeMap<double> NodeResistancesMap;
    typedef typename GR::template NodeMap<NodeResistancesMap *> ResistancesMap;
    typedef GR Graph;
    typedef typename GR::Node Node;
    typedef typename GR::template NodeMap<int> IndiceMap;
    
    // identify nodes of connected components
    IndiceMap componentMap(g);
    const int nb_connected_components = lemon::connectedComponents(g, componentMap);


    //std::cout << "cc : " << nb_connected_components << std::endl;


    // enumerate and give indice to nodes of each connected components
    int * indices_count = new int[nb_connected_components];
    for(int i=0; i<nb_connected_components; i++)
        indices_count[i] = 0;

    IndiceMap indiceMap(g);
    for (typename Graph::NodeIt u(g); u != lemon::INVALID; ++u) {
        const int connected_component = componentMap[u];
        indiceMap[u] = indices_count[connected_component];
        indices_count[connected_component]++;
    }

    // laplacian of each connected component (conductances graph)
    Eigen::MatrixXd * L = new Eigen::MatrixXd[nb_connected_components];
    for(int i=0; i<nb_connected_components; i++) {
        L[i] = Eigen::MatrixXd(indices_count[i], indices_count[i]);
        L[i].setZero();
    }

    for (typename Graph::ArcIt e(g); e != lemon::INVALID; ++e) {
        const Node u = g.u(e);
        const Node v = g.v(e);

        const int component = componentMap[u];
        if(component != componentMap[v])
            continue;

        const int indice_u = indiceMap[u];
        const int indice_v = indiceMap[v];

        L[component](indice_u, indice_v) = L[component](indice_v, indice_u) = -1/resistance[e];
        L[component](indice_u, indice_u) += 1/resistance[e];
        L[component](indice_v, indice_v) += 1/resistance[e];
    }

    // laplacian pseudoinverse of each connected component
    Eigen::MatrixXd * L_plus = new Eigen::MatrixXd[nb_connected_components];
    for(int i=0; i<nb_connected_components; i++)
        L_plus[i] = L[i].completeOrthogonalDecomposition().pseudoInverse();

    // construct returned map
    ResistancesMap * effective_resistance = new ResistancesMap(g);

    for (typename Graph::NodeIt u(g); u != lemon::INVALID; ++u) {
        (*effective_resistance)[u] = new NodeResistancesMap(g);
        const int component = componentMap[u];
        for (typename Graph::NodeIt v(g); v != lemon::INVALID; ++v) {
            if(component != componentMap[v]) {
                (*(*effective_resistance)[u])[v] = std::numeric_limits<double>::max();
                continue;
            }
            const int indice_u = indiceMap[u];
            const int indice_v = indiceMap[v];

            (*(*effective_resistance)[u])[v] = L_plus[component](indice_u, indice_u) + L_plus[component](indice_v, indice_v) - 2 * L_plus[component](indice_u, indice_v);
        }
    }

    delete[] L_plus;
    delete[] L;
    delete[] indices_count;

    return effective_resistance;
}

template <typename GR, typename QM, typename DM, typename CM>
double ECAReff::eval(const concepts::AbstractLandscape<GR, QM, DM, CM> & landscape) {
    typedef GR Graph;
    typedef QM QualityMap;
    typedef DM DifficultyMap;
    typedef typename GR::template NodeMap<typename GR::template NodeMap<double> *> ResistancesMap;

    double sum = 0;

    const Graph & g = landscape.getNetwork();
    const QualityMap & quality = landscape.getQualityMap();
    const DifficultyMap & difficulty = landscape.getDifficultyMap();

    ResistancesMap * effective_resistance = getReff(g, difficulty);

    for (typename Graph::NodeIt s(g); s != lemon::INVALID; ++s) {
        for (typename Graph::NodeIt t(g); t != lemon::INVALID; ++t) {
            if(graph.id(s) >= graph.id(t))
                continue;

            const double r_st = (*(*effective_resistance)[s])[t];

            //std::cout << "r( " << g.id(s) << " , " << g.id(t) << " ) = " << r_st << std::endl;

            sum += quality[s] * quality[t] * p0*std::exp(-r_st/alpha);
        }
    }

    delete effective_resistance;

    return std::sqrt(sum);
}

#endif //ECAREFF_HPP