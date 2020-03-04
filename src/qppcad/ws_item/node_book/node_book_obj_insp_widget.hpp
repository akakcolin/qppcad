#ifndef QPPCAD_NODE_BOOK_OBJECT_INSP
#define QPPCAD_NODE_BOOK_OBJECT_INSP

#include <qppcad/ws_item/ws_item_obj_insp_widget.hpp>
#include <qppcad/ws_item/node_book/node_book.hpp>

namespace qpp {

  namespace cad {

    class node_book_obj_insp_widget_t final: public ws_item_obj_insp_widget_t {

        Q_OBJECT

      public:

        node_book_t *b_nb{nullptr};

        qspoiler_widget_t *m_gb_nb_settings;
        QFormLayout *m_gb_nb_settings_lt;
        qbinded_checkbox_t *m_nb_highlight_dirty_nodes;
        qbinded_checkbox_t *m_nb_auto_recompute;

        node_book_obj_insp_widget_t();

        void bind_to_item(ws_item_t *_binding_item) override;
        void update_from_ws_item() override;
        void unbind_item() override;

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
