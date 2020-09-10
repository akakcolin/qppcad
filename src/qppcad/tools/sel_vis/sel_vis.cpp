#include <qppcad/tools/sel_vis/sel_vis.hpp>
#include <qppcad/ws_item/geom_view/geom_view.hpp>
#include <qppcad/core/app_state.hpp>

using namespace qpp;
using namespace qpp::cad;

void sel_vis_tool_t::exec(ws_item_t *item, uint32_t _error_ctx) {

  //app_state_t *astate = app_state_t::get_inst();

  if (!item) {
    QMessageBox::warning(nullptr, QObject::tr("Select by visibility"),
                         QObject::tr("ws_item == nullptr"));
    return;
  }

  auto al = item->cast_as<geom_view_t>();
  if (!al) {
    QMessageBox::warning(nullptr, QObject::tr("Select by visibility"),
                         QObject::tr("ws_item.cast<geom_view_t>() == nullptr"));
    return;
  }

  al->begin_recording(hs_doc_rec_type_e::hs_doc_rec_as_new_epoch);
  al->sel_visible();
  al->end_recording();

}
