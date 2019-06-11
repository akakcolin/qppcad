#pragma push_macro("slots")
#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#pragma pop_macro("slots")

#include <qppcad/ws_item/ws_item_trampoline.hpp>
#include <qppcad/workspace.hpp>
#include <symm/index.hpp>
#include <geom/lace3d.hpp>
#include <qppcad/ws_item/geom_view/python_geom_view.hpp>
#include <qppcad/ws_item/ccd_view/python_ccd_view.hpp>
#include <qppcad/ws_item/psg_view/psg_view.hpp>
#include <qppcad/ws_item/volume_view/volume_view.hpp>
#include <qppcad/ws_item/node_book/node_book.hpp>
#include <qppcad/ws_item/pdos_view/pdos_view.hpp>
#include <qppcad/app_state.hpp>

using namespace qpp;
using namespace qpp::cad;

namespace py = pybind11;

std::shared_ptr<ws_item_t> construct_from_geom(
    workspace_t &ws,
    std::shared_ptr<xgeometry<float, periodic_cell<float> > > geom,
    std::string name) {

    auto new_item = ws.m_owner->m_bhv_mgr->fbr_ws_item_by_type(geom_view_t::get_type_static());
    if (!new_item) return nullptr;

    auto as_gv = new_item->cast_as<geom_view_t>();
    if(!as_gv) return nullptr;

    if (as_gv->m_geom) {
        as_gv->m_geom->remove_observer(*as_gv->m_ext_obs);
        as_gv->m_geom->remove_observer(*as_gv->m_tws_tr);
      }

    as_gv->m_geom = geom;
    as_gv->m_ext_obs->geom = as_gv->m_geom.get();
    as_gv->m_tws_tr->geom = as_gv->m_geom.get();

    as_gv->m_tws_tr->do_action(act_lock | act_clear_all);
    as_gv->m_geom->add_observer(*as_gv->m_ext_obs);
    as_gv->m_geom->add_observer(*as_gv->m_tws_tr);
    as_gv->m_tws_tr->do_action(act_unlock | act_rebuild_tree);
    as_gv->m_tws_tr->do_action(act_rebuild_ntable);
    as_gv->m_name = name;
    ws.add_item_to_ws(new_item);

    return new_item;

}

std::shared_ptr<ws_item_t> construct_from_array_group(
    workspace_t &ws,
    std::shared_ptr<array_group<matrix3<float> > > ag,
    std::string name) {

    auto new_item = ws.m_owner->m_bhv_mgr->fbr_ws_item_by_type(psg_view_t::get_type_static());
    if (!new_item) return nullptr;

    auto as_psg = new_item->cast_as<psg_view_t>();
    if(!as_psg) return nullptr;

    as_psg->m_ag = ag;
    as_psg->m_name = name;
    ws.add_item_to_ws(new_item);
    as_psg->update_view();

    return new_item;

}

//void upd_oi(ws_item_t *_item) {

//  if (_item && _item->m_selected)
//    app_state_t::get_inst()->astate_evd->cur_ws_selected_item_need_to_update_obj_insp();

//}

void mvd() {

  app_state_t *astate = app_state_t::get_inst();
  astate->make_viewport_dirty();

}

PYBIND11_EMBEDDED_MODULE(cad, m) {

  py::class_<workspace_manager_t,  std::shared_ptr<workspace_manager_t> >(m, "workspace_manager_t")
      .def("__len__", [](const workspace_manager_t &wsm){ return wsm.m_ws.size();})
      .def("__getitem__", [](const workspace_manager_t &wsm, size_t i) {
        if (i >= wsm.m_ws.size()) throw py::index_error();
        return wsm.m_ws[i];
      }, py::return_value_policy::reference_internal)
      .def("__getitem__", [](workspace_manager_t &wsm, std::string ws_name) {
        auto retv = wsm.get_by_name(ws_name);
        if (!retv) throw py::key_error();
        return retv;
       }, py::return_value_policy::reference_internal)
      .def("has_wss", &workspace_manager_t::has_wss)
      .def("set_cur", &workspace_manager_t::set_cur_id);

  py::class_<workspace_t,  std::shared_ptr<workspace_t> >(m, "workspace_t")
      .def_readwrite("name", &workspace_t::m_ws_name)
      .def("__len__", [](workspace_t &ws) {return ws.m_ws_items.size();})
      .def("__getitem__", [](workspace_t &ws, size_t i) {
        if (i >= ws.m_ws_items.size()) throw py::index_error();
        return ws.m_ws_items[i];
      }, py::return_value_policy::reference_internal, py::keep_alive<0,2>())
      .def("__getitem__", [](workspace_t &ws, std::string item_name) {
        auto retv = ws.get_by_name(item_name);
        if (!retv) throw py::key_error();
        return retv;
       }, py::return_value_policy::reference_internal, py::keep_alive<0,2>())
      .def("construct", &workspace_t::py_construct_item)
      .def("construct", &construct_from_geom)
      .def("construct", &construct_from_array_group)
      .def_property("bg",
                    [](workspace_t &src)
                    {return src.m_background_color;},
                    [](workspace_t &src, const vector3<float> value)
                    {src.m_background_color = value; mvd();})
      .def("__repr__", &workspace_t::py_get_repr)
      .def("__str__", &workspace_t::py_get_repr);

  py::class_<ws_item_t, std::shared_ptr<ws_item_t>, py_ws_item_t> py_ws_item_t(m, "ws_item_t");
  py_ws_item_t
          .def_readwrite("name", &ws_item_t::m_name)
          .def("get_cnt_count", &ws_item_t::get_content_count)
          .def("get_parent_ws", [](ws_item_t &wsi){return wsi.m_parent_ws;})
         // .def_readwrite("m_pos", &ws_item_t::get_pos, &ws_item_t::set_pos)
          .def_property("is_visible",
                        [](ws_item_t &src)
                        {return src.m_is_visible;},
                        [](ws_item_t &src, const bool value)
                        {src.m_is_visible = value; src.update_oi();})
          .def_property("show_bb",
                        [](ws_item_t &src)
                        {return src.m_show_bb;},
                        [](ws_item_t &src, const bool value)
                        {src.m_show_bb = value; src.update_oi();})
          .def_property("pos",
                        [](ws_item_t &src)
                        {return src.m_pos;},
                        [](ws_item_t &src, const vector3<float> value)
                        {src.m_pos = value; src.update_oi();})
          .def("__repr__", &ws_item_t::py_get_repr)
          .def("__str__", &ws_item_t::py_get_repr)
          .def_readwrite("offset", &ws_item_t::m_leader_offset);


  py_geom_view_reg_helper_t::reg(m, py_ws_item_t);
  py_ccd_view_reg_helper_t::reg(m, py_ws_item_t);

  py::class_<psg_view_t, std::shared_ptr<psg_view_t> >
  py_point_sym_group_t(m, "psg_view_t", py_ws_item_t);
  py_point_sym_group_t.def_readwrite("ag", &psg_view_t::m_ag, py::return_value_policy::reference);
  py_point_sym_group_t.def_readwrite("pg_axes", &psg_view_t::m_pg_axes);
  py_point_sym_group_t.def("update_view", &psg_view_t::update_view);

  py::class_<volume_view_t, std::shared_ptr<volume_view_t> >
  py_volume_view_t(m, "volume_view_t", py_ws_item_t);

  py::class_<node_book_t, std::shared_ptr<node_book_t> >
  py_node_book_t(m, "node_book_t", py_ws_item_t);

  py_node_book_t.def("num_nds", [](node_book_t &src){return src.m_scene->m_nodes.size();});
  py_node_book_t.def("num_sck", [](node_book_t &src){return src.m_scene->m_sockets.size();});
  py_node_book_t.def("num_con", [](node_book_t &src){return src.m_scene->m_connections.size();});
  py_node_book_t.def("execute", &node_book_t::execute);

  py::class_<pdos_view_t, std::shared_ptr<pdos_view_t> >
  py_pdos_view_t(m, "pdos_view_t", py_ws_item_t);
  py_pdos_view_t.def("load", &pdos_view_t::py_load_from_list);
  py_pdos_view_t.def("load_dir", &pdos_view_t::py_load_from_dir);
  py_pdos_view_t.def("rebuild", &pdos_view_t::rebuild_plots);
  py_pdos_view_t.def("scale_chn", &pdos_view_t::scale_channel);
  py_pdos_view_t.def("e_fermi", &pdos_view_t::py_get_efermi_for_channel);

}
