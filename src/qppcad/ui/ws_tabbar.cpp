#include <qppcad/ui/ws_tabbar.hpp>
#include <qppcad/core/app_state.hpp>

#include <QInputDialog>
#include <QLineEdit>

using namespace qpp;
using namespace qpp::cad;

ws_tabbar_t::ws_tabbar_t(QWidget *parent) : QTabBar (parent) {
  app_state_t *astate = app_state_t::get_inst();
  //setFixedHeight(28);
  setDrawBase(false);
  setExpanding(false);
  setTabsClosable(true);
  //setMovable(true);

  update_tabs();
  cur_ws_changed();

  connect(astate->astate_evd,
          &app_state_event_disp_t::wss_changed_signal,
          this,
          &ws_tabbar_t::wss_changed_slot);

  connect(astate->astate_evd,
          &app_state_event_disp_t::cur_ws_changed_signal,
          this,
          &ws_tabbar_t::cur_ws_changed);

  connect(this,
          &ws_tabbar_t::currentChanged,
          this,
          &ws_tabbar_t::current_changed);

  connect(this,
          &ws_tabbar_t::tabCloseRequested,
          this,
          &ws_tabbar_t::tabs_closed);

  connect(this,
          &ws_tabbar_t::tabBarDoubleClicked,
          this,
          &ws_tabbar_t::tab_double_clicked);
}

void ws_tabbar_t::update_tabs() {
  app_state_t *astate = app_state_t::get_inst();
  blockSignals(true);
  while(count() != 0)
    removeTab(0);
  for (size_t i = 0; i < astate->ws_mgr->m_ws.size(); i++)
    addTab(tr("[%1] %2")
           .arg(i)
           .arg(QString::fromStdString(astate->ws_mgr->m_ws[i]->m_ws_name)));
  blockSignals(false);
}

void ws_tabbar_t::tabs_closed(int index) {
  app_state_t* astate = app_state_t::get_inst();

  if (!(astate->ws_mgr->has_wss() && index >=0 && index < astate->ws_mgr->m_ws.size()))
    return;

  auto cls_ws = astate->ws_mgr->m_ws[index];
  if (!cls_ws)
    return;

  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, tr("Workspace -> Close"),
                                tr("Do you really want to close the workspace?"),
                                QMessageBox::Yes | QMessageBox::No);

  if (reply == QMessageBox::Yes)
    cls_ws->m_marked_for_deletion = true;
}

void ws_tabbar_t::cur_ws_changed() {
  //update_tabs();
  app_state_t *astate = app_state_t::get_inst();
  blockSignals(true);
  auto cur_ws_idx = astate->ws_mgr->get_cur_id();
  if (cur_ws_idx)
    setCurrentIndex(*cur_ws_idx);
  else
    setCurrentIndex(-1);
  blockSignals(false);
}

void ws_tabbar_t::wss_changed_slot() {
  app_state_t *astate = app_state_t::get_inst();
  //astate->tlog("@@@ DEBUG: ws_tabbar_t::wss_changed_slot()");
  update_tabs();
  blockSignals(true);
  auto cur_ws_idx = astate->ws_mgr->get_cur_id();
  if (cur_ws_idx)
    setCurrentIndex(*cur_ws_idx);
  else
    setCurrentIndex(-1);
  blockSignals(false);
}

void ws_tabbar_t::current_changed(int current) {
  app_state_t *astate = app_state_t::get_inst();
  if (current >= 0 && current < astate->ws_mgr->m_ws.size())
    astate->ws_mgr->set_cur_id(std::optional<size_t>(current));
}

void ws_tabbar_t::tab_double_clicked(int index) {
  app_state_t* astate = app_state_t::get_inst();
  if (!(astate->ws_mgr->has_wss() && index >=0 && index < astate->ws_mgr->m_ws.size()))
    return;
  auto target_ws = astate->ws_mgr->m_ws[index];
  if (!target_ws)
    return;
  bool ok;
  QString text = QInputDialog::getText(this, tr("Workspace -> Rename"),
                                       tr("New workspace name:"), QLineEdit::Normal,
                                       QString::fromStdString(target_ws->m_ws_name), &ok);
  if (ok && text != "") {
    target_ws->m_ws_name = text.toStdString();
    astate->astate_evd->wss_changed();
  }
}
