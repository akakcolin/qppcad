#include <qppcad/ws_item/ws_item_extended_editor.hpp>

using namespace qpp;
using namespace qpp::cad;

ext_editor_geom_policy_e ws_item_extended_editor_t::get_geom_policy() {

  return ext_editor_geom_policy_e::always_min;

}

bool ws_item_extended_editor_t::can_be_binded_to(ws_item_t *_item_to_bind) {

  return false;

}

int ws_item_extended_editor_t::get_minimum_width() {

  return 300;

}

void ws_item_extended_editor_t::bind_to_item(ws_item_t *_binding_item) {

  m_binded_item = _binding_item;
  update_from_ws_item();

}

void ws_item_extended_editor_t::unbind_item() {

  m_binded_item = nullptr;

}

void ws_item_extended_editor_t::update_from_ws_item() {

}

int ws_item_extended_editor_t::first_time_width_percentage() {
  return 50;
}

ws_item_extended_editor_t::ws_item_extended_editor_t() : QWidget() {

}

