#ifndef HISTORY_STREAM_ARRAY_PROXY
#define HISTORY_STREAM_ARRAY_PROXY

#include <qppcad/core/history_stream.hpp>
#include <cassert>

namespace qpp {

namespace cad {

template<typename T>
struct promote_to_value_type {
  using type =
      typename std::conditional<std::is_pointer<T>::value, std::remove_pointer<T>, T>::type;
};

template<typename STYPE>
struct hs_arr_rptr_policy {

  using holder_type_t =
      typename std::add_pointer<typename promote_to_value_type<STYPE>::type>::type;

  static void delete_holder(holder_type_t &elem) {
    delete elem;
    elem = nullptr;
  }

  static hs_doc_base_t *cast_from_holder(holder_type_t holder) {
    return reinterpret_cast<hs_doc_base_t*>(holder);
  }

};

template<typename STYPE>
struct hs_arr_sptr_policy {

  using holder_type_t = std::shared_ptr<typename promote_to_value_type<STYPE>::type>;

  static void delete_holder(holder_type_t elem) {

  }

  static hs_doc_base_t *cast_from_holder(holder_type_t &holder) {
    return static_cast<hs_doc_base_t*>(holder.get());
  }

};

template<typename STYPE, typename STYPE_STRG_POL = hs_arr_rptr_policy<STYPE>>
class hist_doc_array_proxy_t : public hs_doc_base_t {

public:

  using holder_type_t = typename STYPE_STRG_POL::holder_type_t;

private:

  std::vector<holder_type_t> p_array_data;
  std::map<hs_doc_base_t*, holder_type_t> p_map_hs_to_array;

public:

  static_assert(std::is_base_of<hs_doc_base_t, STYPE>::value);

  holder_type_t holder_lookup(hs_doc_base_t *rawh) {
    auto reti = p_map_hs_to_array.find(rawh);
    return reti != end(p_map_hs_to_array) ? p_map_hs_to_array[rawh] : nullptr;
  }

  hs_result_e add_hs_child_as_array(holder_type_t &new_arr_element, bool add_new_epoch = true) {

    hs_doc_base_t *as_hsd = STYPE_STRG_POL::cast_from_holder(new_arr_element);
    if (!as_hsd)
      return hs_result_e::hs_error;

    //check if exists
    auto ad_itr = std::find(begin(p_array_data), end(p_array_data), new_arr_element);

    if (ad_itr == end(p_array_data))
      p_array_data.push_back(new_arr_element);

    p_map_hs_to_array[as_hsd] = new_arr_element;

    return add_hs_child(as_hsd, add_new_epoch);

  }

  holder_type_t get_hs_child_as_array(size_t chl_idx) {

    hs_doc_base_t *elem = get_hs_child(chl_idx);
    if (!elem)
      return nullptr;

    auto map_itr = p_map_hs_to_array.find(elem);
    return map_itr != end(p_map_hs_to_array) ? map_itr->second : nullptr;

  }

  void request_child_deletion(hs_doc_base_t *child) override {

    auto map_itr = p_map_hs_to_array.find(child);
    if (map_itr == end(p_map_hs_to_array))
      return;

    holder_type_t childw = map_itr->second;
    p_map_hs_to_array.erase(map_itr);

    auto arr_data_it = std::find(begin(p_array_data), end(p_array_data), childw);
    if (arr_data_it != end(p_array_data))
      p_array_data.erase(arr_data_it);

    STYPE_STRG_POL::delete_holder(childw);

  }

  hs_result_e hs_remove_child_force(holder_type_t child) {
    return child ? hs_remove_child_force_rawptr(child.get()) : hs_result_e::hs_invalid_epoch;
  }

  hs_result_e hs_remove_child_force_rawptr(STYPE *child) {

    if (!child)
      return hs_result_e::hs_invalid_child;

    hs_result_e rem_res = hs_remove_child(child, false);
    if (rem_res != hs_result_e::hs_success)
      return hs_result_e::hs_error;

    request_child_deletion(child);

    return hs_result_e::hs_success;

  }

  typename std::enable_if<!std::is_same<holder_type_t, STYPE>::value, bool>::type
  is_child_unused(holder_type_t &child) {
    return hs_doc_base_t::is_child_unused(STYPE_STRG_POL::cast_from_holder(child));
  }

  typename std::enable_if<!std::is_same<holder_type_t, STYPE>::value, hs_result_e>::type
  is_child_alive(epoch_t target_epoch, holder_type_t &child) const {
    return hs_doc_base_t::is_child_alive(target_epoch, STYPE_STRG_POL::cast_from_holder(child));
  }

  typename std::enable_if<!std::is_same<holder_type_t, STYPE>::value, hs_result_e>::type
  set_alive_hs_child(holder_type_t &child, bool alive) {
    return hs_doc_base_t::set_alive_hs_child(STYPE_STRG_POL::cast_from_holder(child), alive);
  }

};

}

}

#endif
