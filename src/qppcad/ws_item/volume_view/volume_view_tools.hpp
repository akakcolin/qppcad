#ifndef QPPCAD_VOLUME_VIEW_TOOLS
#define QPPCAD_VOLUME_VIEW_TOOLS

#include <qppcad/core/qppcad.hpp>
#include <qppcad/ws_item/volume_view/volume_view.hpp>

namespace qpp {

  namespace cad {

    struct volume_view_tools_t {

      static void sum_volumes(volume_view_t *src1, size_t vol1_idx, float prefactor1,
                              volume_view_t *src2, size_t vol2_idx, float prefactor2,
                              volume_view_t *dst, std::optional<size_t> dst_id = std::nullopt);

      static void volume_cut_sph(volume_view_t *dst,
                                 size_t volume_id,
                                 vector3<float> sph_cnt,
                                 float sph_rad,
                                 bool cut_inner);

      static void volume_cut_fnc(volume_view_t *dst,
                                 size_t volume_id,
                                 std::function<bool(vector3<float>&)> cut_fnc);

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
