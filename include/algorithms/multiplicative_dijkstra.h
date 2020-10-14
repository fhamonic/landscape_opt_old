#ifndef MULTIPLICATIVE_DIJKSTRA_H
#define MULTIPLICATIVE_DIJKSTRA_H

#include <lemon/dijkstra.h>

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

} //END OF NAMESPACE LEMON



namespace lemon {
    /**
     * @ingroup shortest_path
     * @brief A minimalist Dijkstra algorithm class based on \ref Dijkstra
     * 
     * @tparam GR The type of the digraph the algorithm runs on.
     * @tparam LEN \ref concepts::ReadMap "readable" arc map that specifies the lengths of the arcs.
     * @tparam TR The traits class that defines various types used by the 
     * algorithm. By default, it is \ref DijkstraDefaultTraits "DijkstraDefaultTraits<GR, LEN>"
     */
    template <typename GR=ListDigraph,
                typename LEN=typename GR::template ArcMap<int>,
                typename TR=DijkstraDefaultTraits<GR,LEN> >
    class SimplerDijkstra {
    public:
        typedef typename TR::Digraph Digraph;
        typedef typename TR::Value Value;
        typedef typename TR::LengthMap LengthMap;
        
        typedef typename TR::HeapCrossRef HeapCrossRef;
        typedef typename TR::Heap Heap;
        typedef typename TR::OperationTraits OperationTraits;

        typedef TR Traits;

    private:
        typedef typename Digraph::Node Node;
        typedef typename Digraph::NodeIt NodeIt;
        typedef typename Digraph::Arc Arc;
        typedef typename Digraph::OutArcIt OutArcIt;

        const Digraph *G;    
        const LengthMap *_length;
        
        HeapCrossRef *_heap_cross_ref;
        Heap *_heap;

    public:
        SimplerDijkstra(const Digraph& g, const LengthMap& length) :
                G(&g), _length(&length),
                _heap_cross_ref(Traits::createHeapCrossRef(*G)),
                _heap(Traits::createHeap(*_heap_cross_ref)) { }

        ~SimplerDijkstra() {
            delete _heap_cross_ref;
            delete _heap;
        }

    public:
        void init(Node s) {
            _heap->clear();
            for ( NodeIt u(*G) ; u!=INVALID ; ++u )
                _heap_cross_ref->set(u,Heap::PRE_HEAP);
            if(_heap->state(s) != Heap::IN_HEAP)
                _heap->push(s,OperationTraits::zero());
        }

        bool emptyQueue() const { return _heap->empty(); }

        std::pair<typename GR::Node, double> processNextNode() {
            Node v=_heap->top();
            Value oldvalue=_heap->prio();
            _heap->pop();
            for(OutArcIt e(*G,v); e!=INVALID; ++e) {
                Node w=G->target(e);
                switch(_heap->state(w)) {
                case Heap::PRE_HEAP:
                    _heap->push(w,OperationTraits::plus(oldvalue, (*_length)[e]));
                    break;
                case Heap::IN_HEAP: {
                    Value newvalue = OperationTraits::plus(oldvalue, (*_length)[e]);
                    if ( OperationTraits::less(newvalue, (*_heap)[w]) )
                        _heap->decrease(w, newvalue);
                    }
                    break;
                case Heap::POST_HEAP:
                    break;
                }
            }
            return std::make_pair(v, oldvalue);
        }
    };

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
