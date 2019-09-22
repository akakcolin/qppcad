#ifndef QPPCAD_COMPL_LIST_VIEW_OBJECT_INSP
#define QPPCAD_COMPL_LIST_VIEW_OBJECT_INSP
#include <qppcad/ws_item/ws_item_obj_insp_widget.hpp>
#include <qppcad/ws_item/compl_list_view/compl_list_view.hpp>

namespace qpp {

  namespace cad {

    class compl_list_view_obj_insp_widget_t : public ws_item_obj_insp_widget_t {

        Q_OBJECT

      public:

        compl_list_view_obj_insp_widget_t();

        void bind_to_item(ws_item_t *_binding_item) override;
        void update_from_ws_item() override;
        void unbind_item() override;

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
