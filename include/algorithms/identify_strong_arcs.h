#ifndef IDENTIFY_STRONG_H
#define IDENTIFY_STRONG_H

///\ingroup shortest_path
///\file
///\brief Dijkstra algorithm.

#include "algorithms/multiplicative_dijkstra.h"


namespace lemon {

    template <typename TR>
    class LabeledValue {
        typedef TR OperationTraits;
        typedef OperationTraits::Value Value;

        public:
            Value value;
            bool label;
            LabeledValue() : value{OperationTraits::zero()}, label{false} {}
            LabeledValue(Value value, bool label=false) : value{value}, label{label} {}
            bool operator<(const LabeledValue<TR> &o) const { return OperationTraits::less(value, o.value) || (value == o.value && label && !o.label); }
    };


    template<typename GR, typename LEN>
    struct IdentifyDefaultTraits {
        typedef GR Digraph;

        typedef LEN LengthMap;
        typedef typename LEN::Value Value;

        typedef DijkstraDefaultOperationTraits<Value> OperationTraits;
        typedef LabeledValue<OperationTraits> LabeledDist;

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
        
        typedef DijkstraMultiplicativeOperationTraits<Value> OperationTraits;
        typedef LabeledValue<OperationTraits> LabeledDist;

        typedef typename Digraph::template NodeMap<int> HeapCrossRef;
        static HeapCrossRef *createHeapCrossRef(const Digraph &g) { return new HeapCrossRef(g); }

        typedef BinHeap<LabeledDist, HeapCrossRef, std::less<LabeledDist>> Heap;
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
        int nb_strong_candidates;

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
        void init(Arc &uv) {
            create_local();
            _heap->clear();
            for(NodeIt w(*G) ; w!=INVALID ; ++w)
                _heap_cross_ref->set(w,Heap::PRE_HEAP);

            Node u=G->source(uv);
            assert(_heap->state(u) != Heap::IN_HEAP);
            _heap->push(u, LabeledDist());
            nb_strong_candidates = 0;
        }


        void updateStrong(Value &u_oldvalue, Arc &uw) {
            Node w=G->target(uw);
            LabeledDist w_newvalue(OperationTraits::plus(u_oldvalue, (*_worst_length)[uw]), true);
            switch(_heap->state(w)) {
            case Heap::PRE_HEAP:
                _heap->push(w, w_newvalue);
                nb_strong_candidates++;
                break;
            case Heap::IN_HEAP: {
                LabeledDist w_oldvalue = (*_heap)[w];
                if(w_newvalue < w_oldvalue) {
                    if(!w_oldvalue.label)
                        nb_strong_candidates++;
                    _heap->decrease(w, w_newvalue);
                }
            }
                break;
            case Heap::POST_HEAP:
                break;
            }
        }

        void updateNonStrong(Value &u_oldvalue, Arc &uw) {
            Node w=G->target(uw);
            LabeledDist w_newvalue(OperationTraits::plus(u_oldvalue, (*_best_length)[uw]), false);
            switch(_heap->state(w)) {
            case Heap::PRE_HEAP:
                _heap->push(w, w_newvalue);
                break;
            case Heap::IN_HEAP: {
                LabeledDist w_oldvalue = (*_heap)[w];
                if(w_newvalue < w_oldvalue) {
                    if(w_oldvalue.label)
                        nb_strong_candidates--;
                    _heap->decrease(w, w_newvalue);
                }
            }
                break;
            case Heap::POST_HEAP:
                break;
            }
        }

        void processNextNode() {
            Node u=_heap->top();
            LabeledDist oldvalue=_heap->prio();
            _heap->pop();
            
            if(oldvalue.label) {
                Traits::addNode(*_labeledNodesList, u);
                nb_strong_candidates--;
                for(OutArcIt a(*G,u); a!=INVALID; ++a)
                    updateStrong(oldvalue.value, a);
            } else {
                for(OutArcIt a(*G,u); a!=INVALID; ++a)
                    updateNonStrong(oldvalue.value, a);                
            }
        }

        void processFirstNode(Arc &uv) {
            Node u=_heap->top();
            LabeledDist oldvalue=_heap->prio();
            _heap->pop();
            Traits::addNode(*_labeledNodesList, u);

            updateStrong(oldvalue.value, uv);
            for(OutArcIt a(*G,u); a!=INVALID; ++a) {
                if(a == uv) continue;
                updateNonStrong(oldvalue.value, a);
            }   
        }

        void run(Arc &uv) {
            init(uv);
            processFirstNode(uv);
            while(nb_strong_candidates > 0) processNextNode();
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
        int nb_usefull_candidates;

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
        void init(Arc &uv) {
            create_local();
            _heap->clear();
            for(NodeIt w(*G) ; w!=INVALID ; ++w)
                _heap_cross_ref->set(w,Heap::PRE_HEAP);
            Node u=G->source(uv);
            assert(_heap->state(u) != Heap::IN_HEAP);
            _heap->push(u, LabeledDist());
            nb_usefull_candidates = 0;  
        }


        void updateUsefull(Value &u_oldvalue, Arc &uw) {
            Node w=G->target(uw);
            LabeledDist w_newvalue(OperationTraits::plus(u_oldvalue, (*_best_length)[uw]), true);
            switch(_heap->state(w)) {
            case Heap::PRE_HEAP:
                _heap->push(w, w_newvalue);
                nb_usefull_candidates++;
                break;
            case Heap::IN_HEAP: {
                LabeledDist w_oldvalue = (*_heap)[w];
                if(w_newvalue < w_oldvalue) {
                    if(!w_oldvalue.label)
                        nb_usefull_candidates++;
                    _heap->decrease(w, w_newvalue);
                }
            }
                break;
            case Heap::POST_HEAP:
                break;
            }
        }
        
        void updateUseless(Value &u_oldvalue, Arc &uw) {
            Node w=G->target(uw);
            LabeledDist w_newvalue(OperationTraits::plus(u_oldvalue, (*_worst_length)[uw]), false);
            switch(_heap->state(w)) {
            case Heap::PRE_HEAP:
                _heap->push(w, w_newvalue);
                break;
            case Heap::IN_HEAP: {
                LabeledDist w_oldvalue = (*_heap)[w];
                if(w_newvalue < w_oldvalue) {
                    if(w_oldvalue.label)
                        nb_usefull_candidates--;
                    _heap->decrease(w, w_newvalue);
                }
            }
                break;
            case Heap::POST_HEAP:
                break;
            }
        }

        void processNextNode() {
            Node u=_heap->top();
            LabeledDist oldvalue=_heap->prio();
            _heap->pop();
            
            if(oldvalue.label) {
                nb_usefull_candidates--;
                for(OutArcIt a(*G,u); a!=INVALID; ++a)
                    updateUsefull(oldvalue.value, a);
            } else {
                Traits::addNode(*_labeledNodesList, u);
                for(OutArcIt a(*G,u); a!=INVALID; ++a)
                    updateUseless(oldvalue.value, a);                
            }
        }

        void processFirstNode(Arc &uv) {
            Node u=_heap->top();
            LabeledDist oldvalue=_heap->prio();
            _heap->pop();
            Traits::addNode(*_labeledNodesList, u);

            updateUsefull(oldvalue.value, uv);
            for(OutArcIt a(*G,u); a!=INVALID; ++a) {
                if(a == uv) continue;
                updateUseless(oldvalue.value, a);
            }   
        }
        
        void run(Arc &uv) {
            init(uv);
            processFirstNode(uv);
            while(nb_usefull_candidates > 0) processNextNode();
            for(NodeIt w(*G); w!=INVALID; ++w) {
                if(_heap->state(w) == Heap::State::POST_HEAP) continue;
                Traits::addNode(*_labeledNodesList, w);
            }
            //while(!_heap->empty()) processNextNode();
        }
    };

    template <typename GR=ListDigraph,
                typename LEN=typename GR::template ArcMap<double>>
    using MultiplicativeIdentifyUseless = IdentifyUseless<GR, LEN, IdentifyMultiplicativeTraits<GR, LEN>>;


} //END OF NAMESPACE LEMON


#endif
