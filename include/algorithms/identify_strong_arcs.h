#ifndef IDENTIFY_STRONG_H
#define IDENTIFY_STRONG_H

///\ingroup shortest_path
///\file
///\brief Dijkstra algorithm.

#include "algorithms/multiplicative_dijkstra.h"


namespace lemon {

    template <typename V>
    class LabeledValue {
        typedef V Value;

        public:
            Value value;
            bool label;

            LabeledValue() : value{1}, label{false} {}
            LabeledValue(const Value &value, bool label=false) : value{value}, label{label} {}
            LabeledValue(const LabeledValue<Value> &o) : value{o.value}, label{o.label} {}
            
            bool operator<(const LabeledValue<Value> &o) const { return value < o.value || (value == o.value && label && !o.label); }
            bool operator>(const LabeledValue<Value> &o) const { return o < *this; }

            bool operator==(const LabeledValue<Value> &o) const { return value == o.value && label == o.label; }

            void operator=(const LabeledValue<Value> &o) { value = o.value; label = o.label; }
            void operator=(const Value &o) { value = o.value; label = false; }

            LabeledValue<Value> operator+(const LabeledValue<Value> &o) const { return LabeledValue<Value>(value + o.value, label || o.label); }
            LabeledValue<Value> operator*(const LabeledValue<Value> &o) const { return LabeledValue<Value>(value * o.value, label || o.label); }
    };


    template<typename GR, typename LEN>
    struct IdentifyDefaultTraits {
        typedef GR Digraph;

        typedef LEN LengthMap;
        typedef typename LEN::Value Value;
        typedef LabeledValue<Value> LabeledDist;

        typedef DijkstraDefaultOperationTraits<LabeledDist> OperationTraits;

        typedef typename Digraph::template NodeMap<int> HeapCrossRef;
        static HeapCrossRef *createHeapCrossRef(const Digraph &g) { return new HeapCrossRef(g); }

        typedef BinHeap<LabeledDist, HeapCrossRef, std::less<LabeledDist> > Heap;
        static Heap *createHeap(HeapCrossRef& r) { return new Heap(r); }

        typedef typename Digraph::Node Node;
        typedef std::vector<Node> NodeList;
        static NodeList * createNodeList(const Digraph &) { return new std::vector<Node>(); }
        static void addNode(NodeList &n, Node u) { n.push_back(u); }
    };

    template<typename GR, typename LEN>
    struct IdentifyMultiplicativeTraits {
        typedef GR Digraph;

        typedef LEN LengthMap;
        typedef typename LEN::Value Value;
        typedef LabeledValue<Value> LabeledDist;

        typedef DijkstraMultiplicativeOperationTraits<LabeledDist> OperationTraits;

        typedef typename Digraph::template NodeMap<int> HeapCrossRef;
        static HeapCrossRef *createHeapCrossRef(const Digraph &g) { return new HeapCrossRef(g); }

        typedef BinHeap<LabeledDist, HeapCrossRef, std::greater<LabeledDist>> Heap;
        static Heap *createHeap(HeapCrossRef& r) { return new Heap(r); }

        typedef typename Digraph::Node Node;
        typedef std::vector<Node> NodeList;
        static NodeList * createNodeList(const Digraph &) { return new std::vector<Node>(); }
        static void addNode(NodeList &n, Node u) { n.push_back(u); }
    };



    template <typename GR=ListDigraph,
                typename LEN=typename GR::template ArcMap<int>,
                typename TR=IdentifyDefaultTraits<GR,LEN> >
    class IdentifyStrong {
    public:
        typedef typename TR::Digraph Digraph;
        typedef typename TR::Value Value;
        typedef typename TR::LabeledDist LabeledDist;
        typedef typename TR::LengthMap LengthMap;
        
        typedef typename TR::HeapCrossRef HeapCrossRef;
        typedef typename TR::Heap Heap;
        typedef typename TR::NodeList NodeList;
        typedef typename TR::OperationTraits OperationTraits;

        typedef TR Traits;

    private:
        typedef typename Digraph::Node Node;
        typedef typename Digraph::NodeIt NodeIt;
        typedef typename Digraph::Arc Arc;
        typedef typename Digraph::OutArcIt OutArcIt;

        const Digraph *G;    
        const LengthMap *_worst_length;
        const LengthMap *_best_length;

        
        HeapCrossRef *_heap_cross_ref;
        Heap *_heap;
        int nb_candidates;

        NodeList *_labeledNodesList;
        bool local_labeledNodesList;

        void create_local() {
            if(!_labeledNodesList) {
                local_labeledNodesList = true;
                _labeledNodesList = TR::createNodeList(*G);
            }
        }

    public:
        IdentifyStrong(const Digraph &g, const LengthMap &worst_length, const LengthMap &best_length) : 
                G(&g), _worst_length(&worst_length), _best_length(&best_length),
                _heap_cross_ref(Traits::createHeapCrossRef(*G)),
                _heap(Traits::createHeap(*_heap_cross_ref)),
                _labeledNodesList(nullptr), local_labeledNodesList(false) { }

        ~IdentifyStrong() {
            if(local_labeledNodesList) delete _labeledNodesList;
            delete _heap_cross_ref;
            delete _heap;
        }

        IdentifyStrong & labeledNodesList(NodeList &n) {
            _labeledNodesList = &n;
            return *this;
        }

        const NodeList & getLabeledNodesList() { return *_labeledNodesList; }
    public:
        void init(Arc uv) {
            create_local();
            _heap->clear();
            for ( NodeIt w(*G) ; w!=INVALID ; ++w )
                _heap_cross_ref->set(w,Heap::PRE_HEAP);

            Node u=G->source(uv);
            Node v=G->target(uv);
            assert(_heap->state(u) != Heap::IN_HEAP);
            _heap->push(u,OperationTraits::zero());
            assert(_heap->state(v) != Heap::IN_HEAP);
            _heap->push(v,LabeledDist((*_worst_length)[uv], true));

            nb_candidates = 1;
        }

        void updateStrong(LabeledDist &oldvalue, Arc &e) {
            Node w=G->target(e);
            switch(_heap->state(w)) {
            case Heap::PRE_HEAP:
                _heap->push(w,OperationTraits::plus(oldvalue, (*_worst_length)[e]));
                nb_candidates++;
                break;
            case Heap::IN_HEAP: {
                const LabeledDist w_newvalue = OperationTraits::plus(oldvalue, (*_worst_length)[e]);
                const LabeledDist w_oldvalue = (*_heap)[w];
                if(OperationTraits::less(w_newvalue, w_oldvalue)) {
                    if(!w_oldvalue.label)
                        nb_candidates++;
                    _heap->decrease(w, w_newvalue);
                }
            }
                break;
            case Heap::POST_HEAP:
                break;
            }
        }

        void updateNonStrong(LabeledDist &oldvalue, Arc &e) {
            Node w=G->target(e);
            switch(_heap->state(w)) {
            case Heap::PRE_HEAP:
                _heap->push(w,OperationTraits::plus(oldvalue, (*_best_length)[e]));
                break;
            case Heap::IN_HEAP: {
                const LabeledDist w_newvalue = OperationTraits::plus(oldvalue, (*_best_length)[e]);
                const LabeledDist w_oldvalue = (*_heap)[w];
                if(OperationTraits::less(w_newvalue, w_oldvalue)) {
                    if(w_oldvalue.label)
                        nb_candidates--;
                    _heap->decrease(w, w_newvalue);
                }
            }
                break;
            case Heap::POST_HEAP:
                break;
            }
        }

        void processFirstNode(Arc &uv) {
            Node u=_heap->top();
            LabeledDist oldvalue=_heap->prio();
            _heap->pop();
            
            if(oldvalue.label) {
                Traits::addNode(*_labeledNodesList, u);
                nb_candidates--;
                for(OutArcIt a(*G,u); a!=INVALID; ++a) {
                    if(a == uv) continue;
                    updateStrong(oldvalue, a);
                }
            } else {
                for(OutArcIt a(*G,u); a!=INVALID; ++a) {
                    if(a == uv) continue;
                    updateNonStrong(oldvalue, a);
                }             
            }
        }

        void processNextNode() {
            Node v=_heap->top();
            LabeledDist oldvalue=_heap->prio();
            _heap->pop();
            
            if(oldvalue.label) {
                Traits::addNode(*_labeledNodesList, v);
                nb_candidates--;
                for(OutArcIt e(*G,v); e!=INVALID; ++e)
                    updateStrong(oldvalue, e);
            } else {
                for(OutArcIt e(*G,v); e!=INVALID; ++e)
                    updateNonStrong(oldvalue, e);                
            }
        }

        void run(Arc uv) {
            init(uv);
            if(!_heap->empty()) processFirstNode(uv);
            while(nb_candidates > 0) processNextNode();
            // while(!_heap->empty()) processNextNode();
        }
    };

    template <typename GR=ListDigraph,
                typename LEN=typename GR::template ArcMap<double>>
    using MultiplicativeIdentifyStrong = IdentifyStrong<GR, LEN, IdentifyMultiplicativeTraits<GR, LEN>>;




    template <typename GR=ListDigraph,
                typename LEN=typename GR::template ArcMap<int>,
                typename TR=IdentifyDefaultTraits<GR,LEN> >
    class IdentifyUseless {
    public:
        typedef typename TR::Digraph Digraph;
        typedef typename TR::Value Value;
        typedef typename TR::LabeledDist LabeledDist;
        typedef typename TR::LengthMap LengthMap;
        
        typedef typename TR::HeapCrossRef HeapCrossRef;
        typedef typename TR::Heap Heap;
        typedef typename TR::NodeList NodeList;
        typedef typename TR::OperationTraits OperationTraits;

        typedef TR Traits;

    private:
        typedef typename Digraph::Node Node;
        typedef typename Digraph::NodeIt NodeIt;
        typedef typename Digraph::Arc Arc;
        typedef typename Digraph::OutArcIt OutArcIt;

        const Digraph *G;    
        const LengthMap *_worst_length;
        const LengthMap *_best_length;

        
        HeapCrossRef *_heap_cross_ref;
        Heap *_heap;
        int nb_candidates;

        NodeList *_labeledNodesList;
        bool local_labeledNodesList;

        void create_local() {
            if(!_labeledNodesList) {
                local_labeledNodesList = true;
                _labeledNodesList = TR::createNodeList(*G);
            }
        }

    public:
        IdentifyUseless(const Digraph &g, const LengthMap &worst_length, const LengthMap &best_length) : 
                G(&g), _worst_length(&worst_length), _best_length(&best_length),
                _heap_cross_ref(Traits::createHeapCrossRef(*G)),
                _heap(Traits::createHeap(*_heap_cross_ref)),
                _labeledNodesList(nullptr), local_labeledNodesList(false) { }

        ~IdentifyUseless() {
            if(local_labeledNodesList) delete _labeledNodesList;
            delete _heap_cross_ref;
            delete _heap;
        }

        IdentifyUseless & labeledNodesList(NodeList &n) {
            _labeledNodesList = &n;
            return *this;
        }

        const NodeList & getLabeledNodesList() { return *_labeledNodesList; }
    public:
        void init(Arc uv) {
            create_local();
            _heap->clear();
            for ( NodeIt w(*G) ; w!=INVALID ; ++w )
                _heap_cross_ref->set(w,Heap::PRE_HEAP);

            Node u=G->source(uv);
            Node v=G->target(uv);
            if(_heap->state(u) != Heap::IN_HEAP)
                _heap->push(u,OperationTraits::zero());
            if(_heap->state(v) != Heap::IN_HEAP)
                _heap->push(v,LabeledDist((*_best_length)[uv], true));

            nb_candidates = 1;
        }

        void updateUsefull(LabeledDist &oldvalue, Arc &e) {
            Node w=G->target(e);
            switch(_heap->state(w)) {
            case Heap::PRE_HEAP:
                _heap->push(w,OperationTraits::plus(oldvalue, (*_best_length)[e]));
                nb_candidates++;
                break;
            case Heap::IN_HEAP: {
                const LabeledDist w_newvalue = OperationTraits::plus(oldvalue, (*_best_length)[e]);
                const LabeledDist w_oldvalue = (*_heap)[w];
                if(OperationTraits::less(w_newvalue, w_oldvalue)) {
                    if(!w_oldvalue.label)
                        nb_candidates++;
                    _heap->decrease(w, w_newvalue);
                }
            }
                break;
            case Heap::POST_HEAP:
                break;
            }
        }
        
        void updateUseless(LabeledDist &oldvalue, Arc &e) {
            Node w=G->target(e);
            switch(_heap->state(w)) {
            case Heap::PRE_HEAP:
                _heap->push(w,OperationTraits::plus(oldvalue, (*_worst_length)[e]));
                break;
            case Heap::IN_HEAP: {
                const LabeledDist w_newvalue = OperationTraits::plus(oldvalue, (*_worst_length)[e]);
                const LabeledDist w_oldvalue = (*_heap)[w];
                if(OperationTraits::less(w_newvalue, w_oldvalue)) {
                    if(w_oldvalue.label)
                        nb_candidates--;
                    _heap->decrease(w, w_newvalue);
                }
            }
                break;
            case Heap::POST_HEAP:
                break;
            }
        }

        void processFirstNode(Arc &uv) {
            Node u=_heap->top();
            LabeledDist oldvalue=_heap->prio();
            _heap->pop();
            Traits::addNode(*_labeledNodesList, u);

            if(oldvalue.label) {
                Traits::addNode(*_labeledNodesList, u);
                nb_candidates--;
                for(OutArcIt e(*G,u); e!=INVALID; ++e) {
                    if(e == uv) continue;
                    updateUsefull(oldvalue, e);
                }
            } else {
                for(OutArcIt e(*G,u); e!=INVALID; ++e) {
                    if(e == uv) continue;
                    updateUseless(oldvalue, e);
                }             
            }
        }

        void processNextNode() {
            Node v=_heap->top();
            LabeledDist oldvalue=_heap->prio();
            _heap->pop();
            
            if(oldvalue.label) {
                nb_candidates--;
                for(OutArcIt e(*G,v); e!=INVALID; ++e)
                    updateUsefull(oldvalue, e);
            } else {
                Traits::addNode(*_labeledNodesList, v);
                for(OutArcIt e(*G,v); e!=INVALID; ++e)
                    updateUseless(oldvalue, e);                
            }
        }

        void run(Arc uv) {
            init(uv);
            if(!_heap->empty()) processFirstNode(uv);
            while(nb_candidates > 0) processNextNode();
            for(NodeIt w(*G); w!=INVALID; ++w) {
                if(_heap->state(w) == Heap::State::POST_HEAP) continue;
                Traits::addNode(*_labeledNodesList, w);
            }
            // while(!_heap->empty()) processNextNode();
        }
    };

    template <typename GR=ListDigraph,
                typename LEN=typename GR::template ArcMap<double>>
    using MultiplicativeIdentifyUseless = IdentifyUseless<GR, LEN, IdentifyMultiplicativeTraits<GR, LEN>>;


} //END OF NAMESPACE LEMON


#endif
