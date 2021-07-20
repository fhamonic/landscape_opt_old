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
        typedef V Value;
        
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
        typedef GR Digraph;

        typedef LEN LengthMap;
        typedef typename LEN::Value Value;

        typedef DijkstraMultiplicativeOperationTraits<Value> OperationTraits;

        typedef typename Digraph::template NodeMap<int> HeapCrossRef;
        static HeapCrossRef *createHeapCrossRef(const Digraph &g) { return new HeapCrossRef(g); }

        typedef BinHeap<typename LEN::Value, HeapCrossRef, std::greater<Value> > Heap;
        static Heap *createHeap(HeapCrossRef& r) { return new Heap(r); }

        typedef typename Digraph::template NodeMap<typename Digraph::Arc> PredMap;
        static PredMap *createPredMap(const Digraph &g) { return new PredMap(g); }

        typedef NullMap<typename Digraph::Node,bool> ProcessedMap;
        static ProcessedMap *createProcessedMap(const Digraph &) { return new ProcessedMap(); }

        typedef typename Digraph::template NodeMap<typename LEN::Value> DistMap;
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
