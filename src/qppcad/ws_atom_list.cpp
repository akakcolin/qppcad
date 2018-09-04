#include <qppcad/ws_atom_list.hpp>
#include <qppcad/app.hpp>
#include <io/geomio.hpp>
#include <io/vasp_io.hpp>

using namespace qpp;

ws_atom_list_t::ws_atom_list_t(workspace_t* parent):ws_item_t(parent){

  bNeedToRebuildNBT = true;
  iDim = 3;
  //cell = new periodic_cell<float>({20.0, 0.0, 0.0}, {0.0, 20.0, 0.0}, {0.0, 0.0, 20.0});

  geom = new xgeometry<float, periodic_cell<float> >(3,"rg1");
  geom->set_format({"atom", "number", "charge", "x", "y", "z", "show", "sel"},

  {type_string, type_int, type_real, type_real, type_real, type_real,
   type_bool, type_bool});

  geom->DIM = 0;
  geom->cell.DIM = 0;

  ext_obs = new extents_observer<float>(*geom);
  tws_tr = new tws_tree<float>(*geom);
  tws_tr->bAutoBonding = true;

  parent->add_item_to_workspace(this);

}

void ws_atom_list_t::vote_for_view_vectors(vector3<float> &vOutLookPos,
                                         vector3<float> &vOutLookAt){
  if(geom->nat() > 2){
      vOutLookAt += (ext_obs->aabb.max + ext_obs->aabb.min) / 2.0;
      vector3<float> vSize = ext_obs->aabb.max - ext_obs->aabb.min;
      float fSize = vSize.norm();

      vOutLookPos  +=
          ext_obs->aabb.max.normalized() * clamp<float>(fSize, 10.0, 100.0);
    }
  else vOutLookPos += vector3<float>(0.0, 0.0, -5.0);

}

void ws_atom_list_t::geometry_changed(){
  aabb = ext_obs->aabb;
}

void ws_atom_list_t::render(){
  ws_item_t::render();
  //we need it for lambda fn

  app_state_t* astate = &(c_app::get_state());
  vector3<float> _pos = pos;
  if (app_state_c->dp != nullptr){

      if (astate->bDebugDrawRTree){
          astate->dp->begin_render_aabb();
          tws_tr->apply_visitor( [astate, _pos](tws_node<float> *inNode, int deepLevel){
            astate->dp->render_aabb(clr_maroon, inNode->bb.min+_pos, inNode->bb.max+_pos);});
          astate->dp->end_render_aabb();
        }

      if (geom->DIM == 3 && b_show && b_draw_cell){
          astate->dp->begin_render_line();
          vector3<float> cell_clr = clr_black;
          if (bSelected){
              if(parent_ws->cur_edit_type == ws_edit_type::EDIT_WS_ITEM)  cell_clr = clr_red;
              if(parent_ws->cur_edit_type == ws_edit_type::EDIT_WS_ITEM_CONTENT) cell_clr =clr_lime;
            }

            app_state_c->dp->render_cell_3d(
                  cell_clr, geom->cell.v[0], geom->cell.v[1],geom->cell.v[2], pos, 1.1f);
          astate->dp->end_render_line();
        }

      if (!b_show) return;


      // atom render start
      astate->dp->begin_atom_render();

      // draw {0,..} atoms
      for (int i = 0; i < geom->nat(); i++) render_atom(i, index::D(geom->DIM).all(0));

      // draw imaginary atoms that appear due to periodic
      if (geom->DIM > 0) for (uint16_t i = 0; i < tws_tr->imgAtoms.size(); i++)
            render_atom(tws_tr->imgAtoms[i]->atm, tws_tr->imgAtoms[i]->idx);

      astate->dp->end_atom_render();
      // atom render end

      // bond render
      astate->dp->begin_render_bond();


      for (uint16_t i = 0; i < geom->nat(); i++)
        for (uint16_t j = 0; j < tws_tr->n(i); j++){

            uint16_t id1 = i;
            uint16_t id2 = tws_tr->table_atm(i,j);
            index idx2 = tws_tr->table_idx(i,j);

            render_bond(id1, index::D(geom->DIM).all(0), id2, idx2);

            if(idx2 != index::D(geom->DIM).all(0))
              render_bond(id2, idx2, id1, index::D(geom->DIM).all(0));

          }

      if (geom->DIM > 0)
        for (uint16_t i = 0; i < tws_tr->imgAtoms.size(); i++)
          for (uint16_t j = 0; j < tws_tr->imgAtoms[i]->imBonds.size(); j++){

              uint16_t id1 = tws_tr->imgAtoms[i]->atm;
              uint16_t id2 = tws_tr->imgAtoms[i]->imBonds[j]->atm;

              index idx1 = tws_tr->imgAtoms[i]->idx;
              index idx2 = tws_tr->imgAtoms[i]->imBonds[j]->idx;

              render_bond(id1, idx1, id2, idx2);
            }


      astate->dp->end_render_bond();

    }
}

void ws_atom_list_t::render_atom(const uint16_t atNum, const index &atIndex){

  int ap_idx = ptable::number_by_symbol(geom->atom(atNum));
  float fDrawRad = 0.4f;
  vector3<float> color(0.0, 0.0, 1.0);
  if(ap_idx != -1){
      fDrawRad = ptable::get_inst()->arecs[ap_idx-1].aRadius * app_state_c->fAtomRadiusScaleFactor;
      color = ptable::get_inst()->arecs[ap_idx-1].aColorJmol;
    }

  if((parent_ws->cur_edit_type == ws_edit_type::EDIT_WS_ITEM_CONTENT) &&
     (geom->xfield<bool>("sel", atNum)) && bSelected)
    color = vector3<float>(0.43f, 0.55f, 0.12f);

  app_state_c->dp->render_atom(color, geom->pos(atNum, atIndex) + pos, fDrawRad);
}

void ws_atom_list_t::render_bond(const uint16_t atNum1, const index &atIndex1,
                               const uint16_t atNum2, const index &atIndex2){
  int ap_idx = ptable::number_by_symbol(geom->atom(atNum1));
  vector3<float> bcolor(0.0, 0.0, 1.0);
  if(ap_idx != -1){bcolor = ptable::get_inst()->arecs[ap_idx-1].aColorJmol;}
  app_state_c->dp->render_bond(bcolor, geom->pos(atNum1, atIndex1)+ pos,
                               geom->pos(atNum2, atIndex2)+ pos,
                               app_state_c->fBondScaleFactor);
}

void ws_atom_list_t::render_ui(){
  ws_item_t::render_ui();
  if (geom->DIM > 0) ImGui::Checkbox("Draw periodic cell", &b_draw_cell);
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Text("Summary:");
  ImGui::Separator();
  ImGui::Columns(2);
  ImGui::Text("Total atoms:");
  ImGui::Text("Atom types:");
  ImGui::NextColumn();
  ImGui::Text(fmt::format("{}", geom->nat()).c_str());
  ImGui::Text(fmt::format("{}", geom->n_atom_types()).c_str());
  ImGui::Columns(1);

  ImGui::Spacing();
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Columns(3);
  ImGui::Text("Atom type");
  ImGui::NextColumn();
  ImGui::Text("Count");
  ImGui::NextColumn();
  ImGui::Text("Color");
  ImGui::NextColumn();
  ImGui::Separator();
  for (uint8_t i = 0; i < geom->n_types(); i++){
      ImGui::Text(geom->atom_of_type(i).c_str());
    }
  ImGui::NextColumn();

  for (uint8_t i = 0; i < geom->n_types(); i++){
      ImGui::Text(fmt::format("{}",geom->get_atom_count_by_type(i)).c_str());
    }
  ImGui::NextColumn();

  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  for (uint8_t i = 0; i < geom->n_types(); i++){
      const ImVec2 p = ImGui::GetCursorScreenPos();
      float p_x = p.x + 8 + 6;
      float p_y = p.y + 8;
      ImVec2 p_n(p_x, p_y);
      int ap_idx = ptable::number_by_symbol(geom->atom_of_type(i));
      vector3<float> bc(0.0, 0.0, 1.0);
      if(ap_idx != -1) {bc = ptable::get_inst()->arecs[ap_idx-1].aColorJmol;}
      draw_list->AddCircleFilled(p_n, 8, ImColor(ImVec4(bc[0], bc[1], bc[2], 1.0f)));
      ImGui::Dummy(ImVec2(16, 16));
    }
  ImGui::NextColumn();

  ImGui::Columns(1);
}

bool ws_atom_list_t::mouse_click(ray<float> *click_ray){
  if (click_ray){
      std::vector<tws_query_data<float>* > res;
      //we need to translate ray in world frame to local geometry frame
      ray<float> local_geom_ray;
      local_geom_ray.start = click_ray->start - pos;
      local_geom_ray.dir = click_ray->dir;
      tws_tr->query_ray<query_ray_add_all<float> >(&local_geom_ray, &res);
      //std::cout << "res_size = " << res.size() << std::endl;
      std::sort(res.begin(), res.end(), tws_query_data_sort_by_dist<float>);

      if (res.size() > 0){
          //std::cout << res[0]->atm << std::endl;
          if ((parent_ws->cur_edit_type == ws_edit_type::EDIT_WS_ITEM_CONTENT) && bSelected &&
              (res[0]->idx == index::D(geom->DIM).all(0)))
            geom->xfield<bool>("sel", res[0]->atm ) = !(geom->xfield<bool>("sel", res[0]->atm ));
          return true;
        }
    }
  return false;
}

bool ws_atom_list_t::support_translation(){return true;}

bool ws_atom_list_t::support_rotation(){return false;}

bool ws_atom_list_t::support_scaling(){return  false;}

bool ws_atom_list_t::support_content_editing(){return true;}

bool ws_atom_list_t::support_selection(){return true;}

bool ws_atom_list_t::support_rendering_bounding_box(){return geom->DIM > 0;}

std::string ws_atom_list_t::compose_item_name(){
  return fmt::format("Type = [atom list], DIM = [{}d]", geom->DIM);
}

void ws_atom_list_t::update(float delta_time){
  ws_item_t::update(delta_time);
}

float ws_atom_list_t::get_bb_prescaller(){
  if (geom->DIM == 3) return 1.5f;
  return 1.1f;
}

void ws_atom_list_t::shift(const vector3<float> vShift){
  tws_tr->bAutoBonding = false;
  tws_tr->bAutoBuild   = false;

  for (int i = 0; i < geom->nat(); i++)
    geom->coord(i) = vShift + geom->pos(i) ;

  ext_obs->aabb.min = vShift + ext_obs->aabb.min;
  ext_obs->aabb.max = vShift + ext_obs->aabb.max;
  tws_tr->apply_shift(vShift);

  tws_tr->bAutoBonding = true;
  tws_tr->bAutoBuild   = true;
  geometry_changed();
}

void ws_atom_list_t::load_from_file(qc_file_format eFileFormat,
                                  std::string sFileName,
                                  bool bAutoCenter){

  c_app::log(fmt::format("Loading geometry from file {} to ws_atom_list in workspace {}",
                         sFileName, parent_ws->ws_name));

  std::ifstream fQCData(sFileName);
  if (!(fQCData.good())) {
      c_app::log(fmt::format("Error in loading from file {}", sFileName));
      return;
    }

  //clean geom and tws-tree
  tws_tr->bAutoBonding = false;
  tws_tr->bAutoBuild   = false;
  tws_tr->clear_ntable();
  tws_tr->clear_tree();
  ext_obs->bFirstData = true;

  name = extract_base_name(sFileName);

  switch (eFileFormat) {
    case qc_file_format::format_standart_xyz:
      geom->DIM = 0;
      read_xyz(fQCData, *(geom));
      break;

    case qc_file_format::format_vasp_poscar:
      geom->DIM = 3;
      geom->cell.DIM = 3;
      read_vasp_poscar(fQCData, *(geom));
      break;

    default: c_app::log("File format not implemented");

    }

  fQCData.close();

  if(bAutoCenter){
      vector3<float> vCenter(0.0, 0.0, 0.0);
      for (int i = 0; i < geom->nat(); i++)
        vCenter += geom->pos(i);
      vCenter *= (1.0f / geom->nat());
      for (int i = 0; i < geom->nat(); i++)
        geom->coord(i) = -vCenter + geom->pos(i) ;

      ext_obs->aabb.min = -vCenter + ext_obs->aabb.min;
      ext_obs->aabb.max = -vCenter + ext_obs->aabb.max;
    }

  tws_tr->manual_build();
  tws_tr->find_all_neighbours();
  tws_tr->bAutoBonding = true;
  tws_tr->bAutoBuild   = true;

  geometry_changed();

  if (parent_ws) parent_ws->workspace_changed();

}

void ws_atom_list_t::rebuild_ngbt(){

  bNeedToRebuildNBT = false;
}
