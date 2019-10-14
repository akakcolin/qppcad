#ifndef QPPCAD_TOOL_COLORIZE_BY_DIST
#define QPPCAD_TOOL_COLORIZE_BY_DIST
#include <qppcad/core/qppcad.hpp>
#include <qppcad/ws_item/ws_item_behaviour_manager.hpp>
#include <qppcad/ws_item/geom_view/geom_view.hpp>
#include <qppcad/ui/qbinded_inputs.hpp>
#include <qppcad/ui/qspoiler_widget.hpp>

#include <QWidget>
#include <QDialog>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QApplication>

namespace qpp {

  namespace cad {

    class colorize_by_dist_tool_t : public ws_item_tool_t {

      public:

        void exec(ws_item_t *item, uint32_t _error_ctx) override;

    };

    class colorize_by_dist_widget_t : public QDialog {

        Q_OBJECT

      public:

        QVBoxLayout *main_lt;

        qspoiler_widget_t *main_gb;
        QFormLayout *main_gb_lt;

        QHBoxLayout *buttons_lt;
        QPushButton *button_apply;
        QPushButton *button_cancel;

        QComboBox *atom1_type;
        QComboBox *atom2_type;
        QDoubleSpinBox *atom_dist_max;
        qbinded_color3_input_t *clr_input;

        vector3<float> clr_low{0};

        geom_view_t *b_gv{nullptr};

        void init_data();
        colorize_by_dist_widget_t();

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
