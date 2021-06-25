#include <qppcad/ws_item/volume_view/volume_view.hpp>
#include <qppcad/render/volume_tools.hpp>
#include <qppcad/core/app_state.hpp>

using namespace qpp;
using namespace qpp::cad;

volume_view_t::volume_view_t() : ws_item_t () {

  set_default_flags(ws_item_flags_default
                    | ws_item_flags_support_tr
                    | ws_item_flags_support_moveto
                    | ws_item_flags_support_rendering);

}

void volume_view_t::mc_polygonise(float _isolevel) {
  //volume_helper::polygonise_volume_mc_naive(*(m_first_mesh), m_volume, _isolevel, 100);
}

void volume_view_t::vote_for_view_vectors(vector3<float> &v_out_look_pos,
                                          vector3<float> &v_out_look_at) {

}

void volume_view_t::render() {

  ws_item_t::render();
  app_state_t* astate = app_state_t::get_inst();

  auto pos = m_pos.get_value();

  for (size_t i = 0; i < m_volumes.size(); i++) {



    if (m_volumes[i]->m_ready_to_render && m_is_visible.get_value()
        && (i == m_current_vol || m_volumes[i]->m_render_permanent.get_value())) {

      shader_program_t *custom_sp =
          m_volumes[i]->m_transparent_volume.get_value() ? astate->sp_mvap_ssl : nullptr;
      astate->dp->begin_render_general_mesh(custom_sp);
      vector3<float> scale{1,1,1};
      vector3<float> rot{0};
      vector3<float> color{0.5f};

      //astate->glapi->glDisable(GL_CULL_FACE);
      if (m_volumes[i]->m_volume_type == ws_volume_t::volume_mo) {
        astate->dp->render_general_mesh(pos,
                                        scale,
                                        rot,
                                        m_volumes[i]->m_color_pos.get_value(),
                                        m_volumes[i]->m_first_mesh,
                                        m_volumes[i]->m_alpha.get_value(),
                                        custom_sp);

        astate->dp->render_general_mesh(pos,
                                        scale,
                                        rot,
                                        m_volumes[i]->m_color_neg.get_value(),
                                        m_volumes[i]->m_second_mesh,
                                        m_volumes[i]->m_alpha.get_value(),
                                        custom_sp);
      }

      if (m_volumes[i]->m_volume_type == ws_volume_t::volume_density) {
        astate->dp->render_general_mesh(pos,
                                        scale,
                                        rot,
                                        m_volumes[i]->m_color_vol.get_value(),
                                        m_volumes[i]->m_first_mesh,
                                        m_volumes[i]->m_alpha.get_value(),
                                        custom_sp);
      }
      //astate->glapi->glEnable(GL_CULL_FACE);
      astate->dp->end_render_general_mesh(custom_sp);

    }

    if (m_volumes[i]->m_need_to_regenerate) {

      if (!m_volumes[i]->m_first_mesh) m_volumes[i]->m_first_mesh = new mesh_t();
      if (!m_volumes[i]->m_second_mesh) m_volumes[i]->m_second_mesh = new mesh_t();

      if (m_volumes[i]->m_volume_type == ws_volume_t::volume_mo) {
        volume_helper::polygonise_vol_mc(*(m_volumes[i]->m_first_mesh),
                                         m_volumes[i]->m_volume,
                                         m_volumes[i]->m_isolevel.get_value(),
                                         100,
                                         m_volumes[i]->m_volume.m_addr_mode);
        volume_helper::polygonise_vol_mc(*(m_volumes[i]->m_second_mesh),
                                         m_volumes[i]->m_volume,
                                         -m_volumes[i]->m_isolevel.get_value(),
                                         100,
                                         m_volumes[i]->m_volume.m_addr_mode);
      }

      if (m_volumes[i]->m_volume_type == ws_volume_t::volume_density) {
        volume_helper::polygonise_vol_mc(*(m_volumes[i]->m_first_mesh),
                                         m_volumes[i]->m_volume,
                                         m_volumes[i]->m_isolevel.get_value(),
                                         100,
                                         m_volumes[i]->m_volume.m_addr_mode);
      }

      m_volumes[i]->m_ready_to_render = true;
      m_volumes[i]->m_need_to_regenerate = false;
      astate->make_viewport_dirty();

    }

  }

}

bool volume_view_t::mouse_click(ray_t<float> *click_ray) {
  return false;
}

std::string volume_view_t::compose_type_descr() {
  return "3d volume data";
}

void volume_view_t::update(float delta_time) {

}

float volume_view_t::get_bb_prescaller() {
  return 1.0f;
}

uint32_t volume_view_t::get_num_cnt_selected() {
  return 0;
}

size_t volume_view_t::content_count() {
  return 0;
}

void volume_view_t::updated_externally(uint32_t update_reason) {

  ws_item_t::updated_externally(update_reason);

  //regenerate meshes
  if (update_reason & ws_item_updf_regenerate_content) {
    for (auto &vol : m_volumes) {
      vol->m_need_to_regenerate = true;
      vol->m_ready_to_render = false;
    }
  }

}

void volume_view_t::save_to_json(json &data) {

}

void volume_view_t::load_from_json(json &data, repair_connection_info_t &rep_info) {

}

size_t volume_view_t::clone_volume(size_t volume_id) {

  app_state_t *astate = app_state_t::get_inst();

  if (volume_id >= m_volumes.size()) return 0;

  std::shared_ptr<ws_volume_record_t> new_vol_rec = std::make_shared<ws_volume_record_t>();
  new_vol_rec->copy_from(*m_volumes[volume_id]);
  new_vol_rec->m_volume.m_name = fmt::format("{}{}",
                                             m_volumes[volume_id]->m_volume.m_name,
                                             m_volumes.size());

  m_volumes.push_back(new_vol_rec);

  if (m_selected) astate->astate_evd->cur_ws_selected_item_need_to_update_obj_insp();

  return m_volumes.size()-1;

}


void ws_volume_record_t::copy_from(ws_volume_record_t &other, bool clear_volume) {

  if (clear_volume) {

    m_volume.m_axis = other.m_volume.m_axis;
    m_volume.m_steps = other.m_volume.m_steps;
    m_volume.m_offset = other.m_volume.m_offset;
    m_volume.m_addr_mode = other.m_volume.m_addr_mode;
    m_volume.m_tot_atoms = other.m_volume.m_tot_atoms;
    m_volume.m_name = other.m_volume.m_name;
    m_volume.m_has_negative_values = other.m_volume.m_has_negative_values;
    m_volume.m_field.resize(other.m_volume.m_field.size(), 0);

  } else {
    m_volume             = other.m_volume;
  }

  m_alpha              = other.m_alpha;
  m_isolevel           = other.m_isolevel;

  m_color_vol          = other.m_color_vol;
  m_color_pos          = other.m_color_pos;
  m_color_neg          = other.m_color_neg;

  m_transparent_volume = other.m_transparent_volume;
  m_volume_type        = other.m_volume_type;

  m_ready_to_render    = false;
  m_need_to_regenerate = true;

}

ws_volume_record_t::ws_volume_record_t() {

  m_transparent_volume.set_value(false); add_hs_child(&m_transparent_volume);
  m_render_permanent.set_value(false); add_hs_child(&m_render_permanent);
  m_alpha.set_value(0.75f); add_hs_child(&m_alpha);
  m_isolevel.set_value(false); add_hs_child(&m_isolevel);
  m_transparent_volume.set_value(def_isovalue_dens); add_hs_child(&m_transparent_volume);

  m_color_pos.set_cvalue(clr_red); add_hs_child(&m_color_pos);
  m_color_neg.set_cvalue(clr_navy); add_hs_child(&m_color_neg);
  m_color_vol.set_cvalue(clr_yellow); add_hs_child(&m_color_vol);


}
