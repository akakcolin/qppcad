#include <qppcad/ws_item/arrow_primitive/arrow_primitive.hpp>
#include <qppcad/core/app_state.hpp>
#include <qppcad/core/json_helpers.hpp>

using namespace qpp;
using namespace qpp::cad;

arrow_primitive_t::arrow_primitive_t() : ws_item_t() {

  set_default_flags(ws_item_flags_default
                    | ws_item_flags_support_rendering
                    | ws_item_flags_support_moveto
                    | ws_item_flags_support_tr
                    | ws_item_flags_translate_emit_upd_event);

  m_arrow_to.set_value({1, 1, 1}); add_hs_child(&m_arrow_to);
  m_color.set_value({1, 0, 0}); add_hs_child(&m_color);
  m_arrow_cap_len.set_value(0.15f); add_hs_child(&m_arrow_cap_len);
  m_arrow_scale.set_value(0.05f); add_hs_child(&m_arrow_scale);
  m_arrow_cap_scale.set_value(0.12f); add_hs_child(&m_arrow_cap_scale);

}

void arrow_primitive_t::vote_for_view_vectors(vector3<float> &out_look_pos,
                                                 vector3<float> &out_look_at) {
  //do nothing
}

void arrow_primitive_t::render() {

  if (!m_is_visible.get_value()) return;

  app_state_t* astate = app_state_t::get_inst();

  if (m_need_to_recalc_render_data) {
      recalc_render_data();
      m_need_to_recalc_render_data = false;
    }
  //draw axes
  astate->dp->begin_render_general_mesh();
  astate->dp->render_general_mesh(m_model_body, m_color.get_value(), astate->mesh_cylinder);
  astate->dp->render_general_mesh(m_model_arrow, m_color.get_value(), astate->mesh_unit_cone);
  astate->dp->end_render_general_mesh();

}

bool arrow_primitive_t::mouse_click(ray_t<float> *click_ray) {
  return false;
}

std::string arrow_primitive_t::compose_type_descr() {
  return "arrow_primitive";
}

void arrow_primitive_t::update(float delta_time) {
  //do nothing
}

float arrow_primitive_t::get_bb_prescaller() {
  return 1.0f;
}

uint32_t arrow_primitive_t::get_num_cnt_selected() {
  return 0;
}

size_t arrow_primitive_t::get_content_count() {
  return 0;
}

bool arrow_primitive_t::can_be_written_to_json() {
  return true;
}

void arrow_primitive_t::save_to_json(json &data) {

  ws_item_t::save_to_json(data);
  json_io::hs_save_vec3(JSON_ARROW_PR_ARROW_TO, m_arrow_to, data);
  json_io::hs_save_vec3(JSON_ARROW_PR_COLOR, m_color, data);
  json_io::hs_save_var(JSON_ARROW_PR_ARROW_SCALE, m_arrow_scale, data);
  json_io::hs_save_var(JSON_ARROW_PR_CAP_SCALE, m_arrow_cap_scale, data);
  json_io::hs_save_var(JSON_ARROW_PR_CAP_LEN, m_arrow_cap_len, data);

}

void arrow_primitive_t::load_from_json(json &data, repair_connection_info_t &rep_info) {

  ws_item_t::load_from_json(data, rep_info);
  json_io::hs_load_vec3(JSON_ARROW_PR_ARROW_TO, m_arrow_to, data);
  json_io::hs_load_vec3(JSON_ARROW_PR_COLOR, m_color, data);
  json_io::hs_load_var(JSON_ARROW_PR_ARROW_SCALE, m_arrow_scale, data);
  json_io::hs_load_var(JSON_ARROW_PR_CAP_SCALE, m_arrow_cap_scale, data);
  json_io::hs_load_var(JSON_ARROW_PR_CAP_LEN, m_arrow_cap_len, data);

}

void arrow_primitive_t::recalc_render_data() {

  //body
  matrix4<float> mat_model = matrix4<float>::Identity();

  vector3<float> dir = (m_arrow_to.get_value() - m_pos.get_value()).normalized();

  mat_model.block<3,1>(0,3) = m_pos.get_value();
  mat_model.block<3,1>(0,2) = m_arrow_to.get_value() - m_pos.get_value();

  vector3<float> vec_axis_norm = mat_model.block<3,1>(0,2).normalized();

  mat_model.block<3,1>(0,0) = vec_axis_norm.unitOrthogonal() * m_arrow_scale.get_value();
  mat_model.block<3,1>(0,1) = vec_axis_norm.cross(mat_model.block<3,1>(0,0));
  mat_model.block<3,1>(0,3) = m_pos.get_value() ;

  m_model_body = mat_model;

  //cap
  vector3<float> start_aux = m_arrow_to.get_value() ;
  vector3<float> end_aux = m_arrow_to.get_value() + dir * m_arrow_cap_len.get_value();

  matrix4<float> mat_model_aux = matrix4<float>::Identity();
  mat_model_aux.block<3,1>(0,3) = start_aux;
  mat_model_aux.block<3,1>(0,2) = end_aux - start_aux;

  vector3<float> vec_axis_norm_aux = mat_model_aux.block<3,1>(0,2).normalized();
  mat_model_aux.block<3,1>(0,0) = vec_axis_norm_aux.unitOrthogonal() * m_arrow_cap_scale.get_value();
  mat_model_aux.block<3,1>(0,1) = vec_axis_norm_aux.cross(mat_model_aux.block<3,1>(0,0));
  mat_model_aux.block<3,1>(0,3) = start_aux;

  m_model_arrow = mat_model_aux;

}

void arrow_primitive_t::updated_externally(uint32_t update_reason) {

  ws_item_t::updated_externally(update_reason);
  recalc_render_data();

}

