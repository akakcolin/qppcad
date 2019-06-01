#ifndef QPPCAD_WS_COMP_CHEM_DATA_OBJ_INSP_H
#define QPPCAD_WS_COMP_CHEM_DATA_OBJ_INSP_H

#include <qppcad/ws_item/ws_item_obj_insp_widget.hpp>
#include <qppcad/ws_item/ccd_view/ccd_view_obj_insp_widget.hpp>
#include <qppcad/ws_item/ccd_view/ccd_view.hpp>
#include <QListWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

namespace qpp {

  namespace cad {

    class ccd_view_obj_insp_widget_t : public ws_item_obj_insp_widget_t {

        Q_OBJECT

      public:

        ccd_view_t *b_ccdv{nullptr};

        ws_item_tab_widget_t *tab_geo_opt;
        ws_item_tab_widget_t *tab_vibs;

        QLabel *tab_info_program;
        qspoiler_widget_t *gb_gen_ccd_info;
        QFormLayout *gb_gen_ccd_info_lt;
        QLabel *gen_info_prog_name;
        QLabel *gen_info_run_type;

        qspoiler_widget_t *tgo_select_step;
        QHBoxLayout *tgo_select_step_lt;
        QListWidget *tgo_steps_ex;
        qspoiler_widget_t *tgo_step_info;
        QFormLayout *tgo_step_info_lt;

        qspoiler_widget_t *gb_normal_modes;
        QVBoxLayout *gb_normal_modes_lt;

        void bind_to_item(ws_item_t *_binding_item) override;
        void update_from_ws_item() override;
        void unbind_item() override;

        void update_geo_opt();
        ccd_view_obj_insp_widget_t();

      public slots:

        void ui_cur_selected_step_item_changed();

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
