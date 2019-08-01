#include <qppcad/python/python_comp_chem.hpp>
#include <qppcad/ws_item/geom_view/geom_view.hpp>
#include <pybind11/functional.h>

using namespace qpp;
using namespace qpp::cad;

PYBIND11_EMBEDDED_MODULE(cc, m) {

  auto module_cp2k = m.def_submodule("cp2k");
  module_cp2k.def("make_neb_calc", &cp2k_helper_t::save_neb_data,
                  py::arg("g_start"), py::arg("g_end"), py::arg("path") = "");
  module_cp2k.def("gen_fixed_atoms_section", &cp2k_helper_t::gen_fixed_atoms_section);

  auto module_orca = m.def_submodule("orca");
  module_orca.def("gen_multijob_from_anim", &orca_helper_t::gen_multijob_from_anim);

  m.def("get_raw_coords_section", &generic_qc_helper_t::get_raw_coords_section);
  m.def("gen_pair_dist_calc_data", &generic_qc_helper_t::gen_pair_dist_calc_data);

}
