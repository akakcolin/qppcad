#ifndef QPPCAD_WS_ITEM_OBJ_INSP_H
#define QPPCAD_WS_ITEM_OBJ_INSP_H

#include <qppcad/core/qppcad.hpp>
#include <qppcad/ws_item/ws_item.hpp>
#include <qppcad/ui/qbinded_inputs.hpp>
#include <qppcad/ui/qspoiler_widget.hpp>
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>

namespace qpp {

  namespace cad {

    struct ws_item_tab_widget_t {

        QWidget *tab_top_wdgt;
        QVBoxLayout *tab_top_wdgt_lt;
        QScrollArea *tab_scroll;
        QFrame *tab_inner_widget;
        QVBoxLayout *tab_inner_widget_lt;
        QIcon *icon_enabled;
        QIcon *icon_disabled;
        int tab_id;

    };

    class ws_item_obj_insp_widget_t : public QTabWidget {

        Q_OBJECT

      public:

        ws_item_t *m_binded_item{nullptr};

        ws_item_tab_widget_t *tab_general;
        qspoiler_widget_t *sp_info_widget;

        QFormLayout *tg_form_layout;
        QLabel *ws_item_name;
        QLabel *ws_item_type;
        QLabel *ws_item_pos_label;
        qbinded_float3_input_t *ws_item_pos;

        qspoiler_widget_t *tg_actions;
        QGridLayout *tg_actions_layout;
        QPushButton *tg_actions_delete;
        QPushButton *tg_actions_rename;
        QPushButton *tg_actions_clone;

//        QLabel *ws_item_is_visible_label;
//        qbinded_checkbox_t *ws_item_is_visible;
//        QLabel *ws_item_bb_visible_label;
//        qbinded_checkbox_t *ws_item_bb_visible;

        qbinded_bool_named_vector_t *ws_item_show_item_bb;

        ws_item_tab_widget_t* def_tab(QString tab_name,
                                      QString icon_name_enabled,
                                      QString icon_name_disabled = "");

        virtual void bind_to_item(ws_item_t *_binding_item);
        virtual void unbind_item();
        virtual void update_from_ws_item();
        void set_tab_enabled(ws_item_tab_widget_t *tab,
                             bool v_enabled);
        void pre_init_gb(QGroupBox *gb, QFormLayout *gb_lt);
        void post_init_gb(QGroupBox *gb, QFormLayout *gb_lt);
        void init_form_lt(QFormLayout *frm_lt);
        void init_form_lt_lbl(QLabel *_label);

        ws_item_obj_insp_widget_t();

      public slots:

        void cur_ws_selected_item_position_changed();
        void rename_current_item();
        void delete_current_item();
        void cur_tab_changed(int index);

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
