#ifndef CONNECTIVITY_INDEX_HPP
#define CONNECTIVITY_INDEX_HPP

#include "landscape/landscape.hpp"

namespace concepts {
    class ConnectivityIndex {
        public:
            ConnectivityIndex() {};
            ~ConnectivityIndex() {};
    
            template <typename GR, typename QM, typename DM>
            double eval(const concepts::AbstractLandscape<GR, QM, DM> & landscape) {
                (void) landscape;
                return 0.0;
            }
    };
}

#endif //CONNECTIVITY_INDEX_HPP