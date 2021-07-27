#ifndef MULTIPLICATIVE_DIJKSTRA_H
#define MULTIPLICATIVE_DIJKSTRA_H

#include <algorithms/simpler_dijkstra.hpp>

namespace lemon {
    /**
     * @brief Multiplicative operation traits for the Dijkstra algorithm class.
     * 
     * This operation traits class defines all computational operations and 
     * constants which are used in the multiplicative version of Dijkstra algorithm.
     * 
     * @tparam V 
     */
    template <typename V>
    struct DijkstraMultiplicativeOperationTraits {
        using Value = V;
        
        static Value zero() { return static_cast<Value>(1);}
        static Value plus(const Value& left, const Value& right) { return left * right; }
        static bool less(const Value& left, const Value& right) { return left > right; }
    };

    /**
     * @brief Multiplicative traits class of Dijkstra class.
     * 
     * @tparam GR The type of the digraph.
     * @tparam LEN The type of the length map.
     */
    template<typename GR, typename LEN>
    struct DijkstraMultiplicativeTraits {
        using Digraph = GR;

        using LengthMap = LEN;
        using Value = typename LEN::Value;

        using OperationTraits = DijkstraMultiplicativeOperationTraits<Value>;

        using HeapCrossRef = typename Digraph::template NodeMap<int>;
        static HeapCrossRef *createHeapCrossRef(const Digraph &g) { return new HeapCrossRef(g); }

        using Heap = BinHeap<typename LEN::Value, HeapCrossRef, std::greater<Value> >;
        static Heap *createHeap(HeapCrossRef& r) { return new Heap(r); }

        using PredMap = typename Digraph::template NodeMap<typename Digraph::Arc>;
        static PredMap *createPredMap(const Digraph &g) { return new PredMap(g); }

        using ProcessedMap = NullMap<typename Digraph::Node,bool>;
        static ProcessedMap *createProcessedMap(const Digraph &) { return new ProcessedMap(); }

        using DistMap = typename Digraph::template NodeMap<typename LEN::Value>;
        static DistMap *createDistMap(const Digraph &g) { return new DistMap(g); }
    };

    /**
     * @ingroup shortest_path
     * @brief Template alias for "Dijkstra<GR,LEN,DijkstraMultiplicativeTraits<GR,LEN>>"
     * 
     * @tparam GR 
     * @tparam LEN 
     */
    template <typename GR=ListDigraph,
            typename LEN=typename GR::template ArcMap<double>>
    using MultiplicativeDijkstra = Dijkstra<GR, LEN, DijkstraMultiplicativeTraits<GR, LEN>>;

    /**
     * @ingroup shortest_path
     * @brief Template alias for "SimplerDijkstra<GR,LEN,DijkstraMultiplicativeTraits<GR,LEN>>"
     * 
     * @tparam GR 
     * @tparam LEN 
     */
    template <typename GR=ListDigraph,
            typename LEN=typename GR::template ArcMap<double>>
    using MultiplicativeSimplerDijkstra = SimplerDijkstra<GR, LEN, DijkstraMultiplicativeTraits<GR, LEN>>;
} //END OF NAMESPACE LEMON

#endif
