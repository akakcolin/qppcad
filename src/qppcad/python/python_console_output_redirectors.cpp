#include <qppcad/python/python_console_output_redirectors.hpp>
#include <qppcad/core/app_state.hpp>

using namespace qpp;
using namespace qpp::cad;

void python_std_output_redirector_t::write(pybind11::str data) {
  app_state_t::get_inst()->py_mgr->m_output_buffer += data.cast<std::string>();
}
