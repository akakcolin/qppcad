#ifndef QPPCAD_JSON_HELPERS
#define QPPCAD_JSON_HELPERS

#include <qppcad/core/json_adapter.hpp>
#include <qppcad/core/history_stream.hpp>
#include <symm/index.hpp>

namespace qpp {

namespace cad {

class json_io {

public:

  static bool exists(const std::string &secname, json &data) {
    return data.find(secname) != data.end();
  }

  template<typename T>
  static void hs_save_var(const std::string &varname, hs_doc_t<T> &var, json &data) {
    data[varname] = var.get_value();
  }

  template<typename T>
  static void hs_load_var(const std::string &varname, hs_doc_t<T> &var, json &data) {
    auto it = data.find(varname);
    if (it != data.end()) var.set_value(it.value().get<T>());
  }

  template<typename T>
  static void hs_save_vec3(const std::string &varname, hs_doc_t<vector3<T>> &var, json &data) {
    vector3<T> vec = var.get_value();
    json var_vec = json::array({vec[0], vec[1], vec[2]});
    data[varname] = var_vec;
  }

  template<typename T>
  static void hs_load_vec3(const std::string &varname, hs_doc_t<vector3<T>> &var, json &data) {
    auto it = data.find(varname);
    vector3<T> vec;
    if (it != data.end())
      for(auto i = 0; i < 3; i++) vec[i] = it.value()[i].get<T>();
    var.set_value(std::move(vec));
  }

  template<typename T>
  static void save_var(const std::string &varname, T &var, json &data) {
    data[varname] = var;
  }

  template<typename T>
  static void save_vec3(const std::string &varname, vector3<T> &var, json &data) {
    json var_vec = json::array({var[0], var[1], var[2]});
    data[varname] = var_vec;
  }

  template<typename T>
  static void load_var(const std::string &varname, T &var, json &data) {
    auto it = data.find(varname);
    if (it != data.end()) var = it.value().get<T>();
  }

  template<typename T>
  static void load_vec3(const std::string &varname, vector3<T> &var, json &data) {
    auto it = data.find(varname);
    if (it != data.end())
      for(auto i = 0; i < 3; i++) var[i] = it.value()[i].get<T>();
  }

  static void save_index(const std::string &varname, const index idx, json &data) {

    json idx_array = json::array({});
    for (int i = 0; i < idx.DIM; i++)
      idx_array.push_back(idx(i));

    data[varname] = idx_array;

  }

  static index load_index(const std::string &varname, json &data) {

    if (data.find(varname) != data.end()) {
      json dt = data[varname];
      index ret = index::D(dt.size());
      for (int i = 0; i < ret.DIM; i++)
        ret(i) = dt[i].get<int>();
      return ret;
    }

    return index::D(0);

  }

};

} // namespace qpp::cad

} // namespace qpp

#endif
