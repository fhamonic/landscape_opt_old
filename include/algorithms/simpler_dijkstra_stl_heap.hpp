#ifndef SIMPLER_DIJKSTRA_STL_HEAP_H
#define SIMPLER_DIJKSTRA_STL_HEAP_H

#include <lemon/dijkstra.h>

namespace lemon {
/**
 * @ingroup shortest_path
 * @brief A minimalist Dijkstra algorithm class based on \ref Dijkstra
 *
 * @tparam GR The type of the digraph the algorithm runs on.
 * @tparam LEN \ref concepts::ReadMap "readable" arc map that specifies the
 * lengths of the arcs.
 * @tparam TR The traits class that defines various types used by the
 * algorithm. By default, it is \ref DijkstraDefaultTraits
 * "DijkstraDefaultTraits<GR, LEN>"
 */
template <typename GR = ListDigraph,
          typename LEN = typename GR::template ArcMap<int>,
          typename TR = DijkstraDefaultTraits<GR, LEN> >
class SimplerDijkstraSTLHeap {
public:
    using Digraph = typename TR::Digraph;
    using Value = typename TR::Value;
    using LengthMap = typename TR::LengthMap;

    using HeapCrossRef = typename TR::HeapCrossRef;
    using Heap = typename TR::Heap;
    using OperationTraits = typename TR::OperationTraits;

    using Traits = TR;

private:
    using Node = typename Digraph::Node;
    using NodeIt = typename Digraph::NodeIt;
    using Arc = typename Digraph::Arc;
    using OutArcIt = typename Digraph::OutArcIt;

    const Digraph * G;
    const LengthMap * _length;

    HeapCrossRef * _heap_cross_ref;
    Heap * _heap;

public:
    SimplerDijkstra(const Digraph & g, const LengthMap & length)
        : G(&g)
        , _length(&length)
        , _heap_cross_ref(Traits::createHeapCrossRef(*G))
        , _heap(Traits::createHeap(*_heap_cross_ref)) {}

    ~SimplerDijkstra() {
        delete _heap_cross_ref;
        delete _heap;
    }

public:
    void init(Node s) {
        _heap->clear();
        for(NodeIt u(*G); u != INVALID; ++u)
            _heap_cross_ref->set(u, Heap::PRE_HEAP);
        if(_heap->state(s) != Heap::IN_HEAP)
            _heap->push(s, OperationTraits::zero());
    }

    bool emptyQueue() const { return _heap->empty(); }

    std::pair<typename GR::Node, double> processNextNode() {
        Node v = _heap->top();
        Value oldvalue = _heap->prio();
        _heap->pop();
        for(OutArcIt e(*G, v); e != INVALID; ++e) {
            Node w = G->target(e);
            switch(_heap->state(w)) {
                case Heap::PRE_HEAP:
                    _heap->push(w,
                                OperationTraits::plus(oldvalue, (*_length)[e]));
                    break;
                case Heap::IN_HEAP: {
                    Value newvalue =
                        OperationTraits::plus(oldvalue, (*_length)[e]);
                    if(OperationTraits::less(newvalue, (*_heap)[w]))
                        _heap->decrease(w, newvalue);
                } break;
                case Heap::POST_HEAP:
                    break;
            }
        }
        return std::make_pair(v, oldvalue);
    }
};
}  // namespace lemon

#endif  // SIMPLER_DIJKSTRA_H
