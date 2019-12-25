#ifndef QPPCAD_TOOL_SEL_VIS
#define QPPCAD_TOOL_SEL_VIS
#include <qppcad/core/qppcad.hpp>
#include <qppcad/ws_item/ws_item_behaviour_manager.hpp>

namespace qpp {

  namespace cad {

    class sel_vis_tool_t : public ws_item_tool_t {
    
      public:
      
        void exec(ws_item_t *item, uint32_t _error_ctx) override;
        
    };
    
  } // namespace qpp::cad
 
} // namespace qpp

#endif
