#include <qppcad/tools/purify_boundary_atoms/purify_boundary_atoms.hpp>
#include <qppcad/app_state.hpp>
#include <qppcad/ws_item/geom_view/geom_view.hpp>

using namespace qpp;
using namespace qpp::cad;

void purify_boundary_atoms_tool_t::exec(ws_item_t *item, uint32_t _error_ctx) {

  app_state_t *astate = app_state_t::get_inst();

  purify_boundary_atoms_widget_t paw;

  if (!item) return;

  auto as_gv = item->cast_as<geom_view_t>();
  if (!as_gv) return;

  paw.master_gv = as_gv;
  paw.rebuild_sub_gvs();
  int ret_code = paw.exec();

  if (ret_code == QDialog::Accepted)
    for (size_t i = 0; i < paw.sub_gvs_wdgt->count(); i++) {
        QListWidgetItem *item = paw.sub_gvs_wdgt->item(i);
        if (item->checkState() == Qt::Checked) {
            astate->tlog("@DEBUG: purify entered");
            paw.m_sub_gvs[i]->purify_boundary_atoms(paw.master_gv);
          }
      }

}

purify_boundary_atoms_widget_t::purify_boundary_atoms_widget_t() {

  app_state_t *astate = app_state_t::get_inst();

  main_lt = new QVBoxLayout;
  setLayout(main_lt);

  dialog_bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  for (auto btn : dialog_bb->buttons())
    btn->setFixedWidth(astate->size_guide.common_button_fixed_w());

  connect(dialog_bb,
          &QDialogButtonBox::accepted,
          this,
          &purify_boundary_atoms_widget_t::accept);

  connect(dialog_bb,
          &QDialogButtonBox::rejected,
          this,
          &purify_boundary_atoms_widget_t::reject);

  sub_gvs_wdgt = new QListWidget;

  actions_lt = new QHBoxLayout;
  action_select_all = new QPushButton(tr("Select all"));
  connect(action_select_all,
          &QPushButton::clicked,
          this,
          &purify_boundary_atoms_widget_t::select_all_clicked);

  actions_lt->addWidget(action_select_all);
  actions_lt->addStretch(1);

  main_lt->addLayout(actions_lt);
  main_lt->addWidget(sub_gvs_wdgt);
  main_lt->addWidget(dialog_bb);

}

void purify_boundary_atoms_widget_t::rebuild_sub_gvs() {

  app_state_t *astate = app_state_t::get_inst();

  sub_gvs_wdgt->clear();

  if (!master_gv) return;

  for (auto ws : astate->ws_mgr->m_ws)
    for (auto ws_item : ws->m_ws_items)
      if (ws_item->get_type() == geom_view_t::get_type_static())
        if (auto as_gv = ws_item->cast_as<geom_view_t>();
            as_gv != master_gv &&
            as_gv && master_gv->m_geom->DIM == as_gv->m_geom->DIM &&
            master_gv->m_geom->nat() == as_gv->m_geom->nat()) {

            astate->tlog("@DEBUG: purify_boundary_atoms_widget_t::rebuild_sub_gvs() -> in cycle");

            QListWidgetItem *list_item = new QListWidgetItem(sub_gvs_wdgt);

            std::string list_item_name = fmt::format("{}/{}",
                                                     as_gv->m_parent_ws->m_ws_name,
                                                     as_gv->m_name);

            list_item->setText(QString::fromStdString(list_item_name));
            list_item->setCheckState(Qt::Unchecked);

            m_sub_gvs.push_back(as_gv);

          }

}

void purify_boundary_atoms_widget_t::select_all_clicked() {

  for (size_t i = 0; i < sub_gvs_wdgt->count(); i++) {
      QListWidgetItem *item = sub_gvs_wdgt->item(i);
      item->setCheckState(Qt::Checked);
    }

}
