#ifndef QPPCAD_SFLOW_BASE_NODES_WS_ITEM
#define QPPCAD_SFLOW_BASE_NODES_WS_ITEM

#include <qppcad/sflow/sflow_node.hpp>

namespace qpp {

  namespace cad {

    class sf_claim_xgeom_node_t final : public sflow_node_t {

      public:

        sf_claim_xgeom_node_t();
        bool execute_ex() override;
        bool is_single_node() override;

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
