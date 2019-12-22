#include <qppcad/ui/qinline_tool_window.hpp>
#include <qppcad/core/app_state.hpp>

using namespace qpp;
using namespace qpp::cad;

qinline_tool_window_t::qinline_tool_window_t(QWidget *parent) : qembed_window_t(parent) {

  m_btn_apply_tool = new QPushButton(tr("Apply"));
  m_btn_cancel_tool = new QPushButton(tr("Close"));

  header_lt->addWidget(m_btn_apply_tool);
  header_lt->addWidget(m_btn_cancel_tool);
  header_lt->addStretch(1);
  header_lt->setContentsMargins(9, 6, 4, 6);

  main_lt->addStretch(0);

  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

}

void qinline_tool_window_t::mark_as_horizontal() {

  app_state_t *astate = app_state_t::get_inst();
  setMaximumHeight(astate->size_guide.inline_tool_horizontal_max_height());
  setMinimumHeight(astate->size_guide.inline_tool_horizontal_min_height());

}

void qinline_tool_window_t::mark_as_vertical() {

  app_state_t *astate = app_state_t::get_inst();
  setFixedWidth(astate->size_guide.inline_tool_vertical_max_width());

}
