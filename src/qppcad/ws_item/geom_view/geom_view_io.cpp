#include <qppcad/ws_item/geom_view/geom_view_io.hpp>
#include <qppcad/ws_item/geom_view/geom_view.hpp>
#include <io/molcas.hpp>
#include <io/vasp_chgcar.hpp>
#include <qppcad/core/app_state.hpp>

using namespace qpp;
using namespace qpp::cad;

void geom_view_io_cube_t::load_from_stream_ex(std::basic_istream<char, TRAITS> &stream,
                                              geom_view_t *_item,
                                              workspace_t *ws) {

  std::shared_ptr<volume_view_t> vold = std::make_shared<volume_view_t>();

  _item->m_geom->DIM = 0;
  _item->m_geom->cell.DIM = 0;

  vold->load_from_stream(stream, *(_item->m_geom.get()), _item->m_name);
  _item->m_parent_ws->add_item_to_ws(vold);

}

void geom_view_molcas_grid_t::load_from_stream_ex(std::basic_istream<char, TRAITS> &stream,
                                                  geom_view_t *_item,
                                                  workspace_t *ws) {

  std::shared_ptr<volume_view_t> vold = std::make_shared<volume_view_t>();
  _item->m_geom->DIM = 0;
  _item->m_geom->cell.DIM = 0;

  std::vector<scalar_volume_t<float> > tmp_volumes;
  load_grid_ascii(stream, *(_item->m_geom.get()), tmp_volumes);
  _item->m_parent_ws->add_item_to_ws(vold);
  vold->m_name = fmt::format("{}.volume", _item->m_name);

  //process volumes
  vold->m_volumes.reserve(tmp_volumes.size());

  for (size_t i = 0; i < tmp_volumes.size(); i++) {

      ws_volume_record_t new_vol_rec;

      new_vol_rec.m_volume = std::move(tmp_volumes[i]);
      new_vol_rec.m_ready_to_render = false;
      new_vol_rec.m_need_to_regenerate = true;

      if (new_vol_rec.m_volume.m_has_negative_values) {
          new_vol_rec.m_volume_type = ws_volume_t::volume_mo;
          new_vol_rec.m_isolevel = qpp::def_isovalue_mo;
        } else {
          new_vol_rec.m_volume_type = ws_volume_t::volume_density;
          new_vol_rec.m_isolevel = qpp::def_isovalue_dens;
        }

      auto new_vol_rec_ptr = std::make_shared<ws_volume_record_t>(std::move(new_vol_rec));

      vold->m_volumes.push_back(new_vol_rec_ptr);

    } // end for

}

void geom_view_vasp_chgcar_t::load_from_stream_ex(std::basic_istream<char, TRAITS> &stream,
                                                  geom_view_t *_item,
                                                  workspace_t *ws) {

  std::shared_ptr<volume_view_t> vold = std::make_shared<volume_view_t>();
  _item->m_geom->DIM = 3;
  _item->m_geom->cell.DIM = 3;

  std::vector<scalar_volume_t<float> > tmp_volumes;
  read_vasp_chgcar(stream, *(_item->m_geom.get()), tmp_volumes);
  _item->m_parent_ws->add_item_to_ws(vold);
  vold->m_name = fmt::format("{}.volume", _item->m_name);

  vold->m_volumes.reserve(tmp_volumes.size());

  for (size_t i = 0; i < tmp_volumes.size(); i++) {

      ws_volume_record_t new_vol_rec;

      new_vol_rec.m_volume = std::move(tmp_volumes[i]);
      new_vol_rec.m_ready_to_render = false;
      new_vol_rec.m_need_to_regenerate = true;

      if (new_vol_rec.m_volume.m_has_negative_values) {
          new_vol_rec.m_volume_type = ws_volume_t::volume_mo;
          new_vol_rec.m_isolevel = qpp::def_isovalue_mo;
        } else {
          new_vol_rec.m_volume_type = ws_volume_t::volume_density;
          new_vol_rec.m_isolevel = qpp::def_isovalue_dens;
        }

      auto new_vol_rec_ptr = std::make_shared<ws_volume_record_t>(std::move(new_vol_rec));

      vold->m_volumes.push_back(new_vol_rec_ptr);

    } // end for

}
