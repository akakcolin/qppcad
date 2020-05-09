#include <qppcad/core/history_stream.hpp>

namespace qpp {

namespace cad {

hist_doc_base_t::hist_doc_base_t() {

}

hist_doc_base_t::~hist_doc_base_t() {

}

hist_doc_base_t::epoch_t hist_doc_base_t::get_cur_epoch() {
  return p_cur_epoch;
}

hr_result_e hist_doc_base_t::set_cur_epoch(hist_doc_base_t::epoch_t cur_epoch) {

  auto it_ce = std::find(std::begin(p_hist_line), std::end(p_hist_line), cur_epoch);
  if (it_ce != std::end(p_hist_line)) {
    epoch_t prev_epoch = p_cur_epoch;
    p_cur_epoch = cur_epoch;
    return on_epoch_changed(prev_epoch);
  }

  return hr_result_e::hr_invalid_epoch;

}

hr_result_e hist_doc_base_t::on_epoch_changed(hist_doc_base_t::epoch_t prev_epoch) {
  return hr_result_e::hr_success;
}

hr_result_e hist_doc_base_t::on_epoch_removed(hist_doc_base_t::epoch_t target_epoch){
  return hr_result_e::hr_success;
}

hr_result_e hist_doc_base_t::commit_exclusive(hist_doc_base_t *child,
                                              std::optional<epoch_t> child_epoch) {

  auto [push_result, new_epoch] = push_epoch(std::nullopt, true);
  if (!push_result || !new_epoch) return hr_result_e::hr_error;
  on_commit_exclusive();

  if (child && child_epoch)
    this->augment_epoch(*new_epoch, child, *child_epoch);

  if (p_parent) {
    return p_parent->commit_exclusive(this, *new_epoch);
  } else {
    return hr_result_e::hr_success;
  }

}

void hist_doc_base_t::on_commit_exclusive() {

}

hist_doc_delta_state_e hist_doc_base_t::get_delta_state_type() {
  return p_dstate;
}

void hist_doc_base_t::set_delta_state_type(hist_doc_delta_state_e new_dstate) {
  p_dstate = new_dstate;
}

bool hist_doc_base_t::is_dirty() {
  return p_is_dirty;
}

void hist_doc_base_t::mark_as_dirty() {
  p_is_dirty = true;
}

std::tuple<hr_result_e, std::optional<hist_doc_base_t::epoch_t> > hist_doc_base_t::push_epoch(
    std::optional<hist_doc_base_t::epoch_t> new_epoch_ex,
    bool checkout_to_new_epoch) {

  epoch_t cur_epoch = get_cur_epoch();

  epoch_t new_epoch = (new_epoch_ex) ?
                                     *new_epoch_ex :
                                     *std::max_element(p_hist_line.begin(), p_hist_line.end()) + 1;

  if (p_hist_line.empty()) {
    p_hist_line.push_back(new_epoch);
    if (checkout_to_new_epoch) checkout_to_epoch(new_epoch);
    return {hr_result_e::hr_success, new_epoch};
  }

  auto last = p_hist_line.rbegin();
  if (new_epoch < *last) {
    p_is_bad = true;
    return {hr_result_e::hr_invalid_epoch, std::nullopt};
  } else {

    p_hist_line.insert(begin(p_hist_line) + cur_epoch + 1, new_epoch);
    auto er_start = begin(p_hist_line) + cur_epoch + 2;
    auto er_end = end(p_hist_line);

    std::vector<epoch_t> epochs_to_delete{er_start, er_end};
    for (auto epoch_to_delete : epochs_to_delete)
      on_epoch_removed(epoch_to_delete);

    p_hist_line.erase(er_start, er_end);
    if (checkout_to_new_epoch) checkout_to_epoch(new_epoch);

    return {hr_result_e::hr_success, new_epoch};

  }

}

size_t hist_doc_base_t::get_history_size() {
  return p_hist_line.size();
}

std::vector<hist_doc_base_t::epoch_t> hist_doc_base_t::get_history() {
  return p_hist_line;
}

hr_result_e hist_doc_base_t::augment_epoch(hist_doc_base_t::epoch_t target_epoch,
                                           hist_doc_base_t::self_t *child,
                                           hist_doc_base_t::epoch_t child_epoch) {

  if (!child) {
    return hr_result_e::hr_invalid_child;
  }

  if (find_hl(target_epoch) == std::end(p_hist_line)) {
    return hr_result_e::hr_invalid_epoch;
  }

  if (child->find_hl(child_epoch) == end(child->p_hist_line)) {
    return hr_result_e::hr_invalid_child_epoch;
  }

  auto &aug_elist = p_childs_states[target_epoch];
  auto find_fnc = [child](std::tuple<self_t*, epoch_t> &elem) {
    return std::get<0>(elem) == child;
  };

  auto it1 = std::find_if(begin(aug_elist), end(aug_elist), find_fnc);
  if (it1 != std::end(aug_elist)) {
    size_t aug_idx = static_cast<size_t>(std::distance(begin(aug_elist), it1));
    aug_elist[aug_idx] = {child, child_epoch};
  } else {
    aug_elist.push_back({child, child_epoch});
  }

  return hr_result_e::hr_success;

}

size_t hist_doc_base_t::get_augmented_count(hist_doc_base_t::epoch_t target_epoch) {
  auto &aug_elist = p_childs_states[target_epoch];
  return aug_elist.size();
}

bool hist_doc_base_t::has_epoch(hist_doc_base_t::epoch_t target_epoch) {
  return find_hl(target_epoch) != end(p_hist_line);
}

hr_result_e hist_doc_base_t::remove_augment_from_epoch(hist_doc_base_t::self_t *child,
                                                       hist_doc_base_t::epoch_t child_epoch,
                                                       hist_doc_base_t::epoch_t target_epoch) {

  auto epoch_it = p_childs_states.find(target_epoch);
  if (epoch_it == end(p_childs_states)) return hr_result_e::hr_invalid_epoch;

  auto find_fnc = [child, child_epoch](std::tuple<self_t*, epoch_t> &elem) {
    return std::get<0>(elem) == child && std::get<1>(elem) == child_epoch;
  };

  auto ch_epoch_it =
      std::find_if(begin(epoch_it->second), end(epoch_it->second), find_fnc);

  if (ch_epoch_it == end(epoch_it->second)) return hr_result_e::hr_invalid_epoch;

  epoch_it->second.erase(ch_epoch_it);
  return hr_result_e::hr_success;

}

hr_result_e hist_doc_base_t::checkout_to_epoch(hist_doc_base_t::epoch_t target_epoch) {

  if (p_cur_epoch == target_epoch) {
    return hr_result_e::hr_success;
  }

  hr_result_e cur_res = set_cur_epoch(target_epoch);
  if (cur_res != hr_result_e::hr_success) {
    return hr_result_e::hr_invalid_epoch;
  }

  epoch_t cur_epoch = get_cur_epoch();
  auto epoch_it = p_childs_states.find(cur_epoch);
  if (epoch_it != end(p_childs_states)) {

    auto &epoch_aug_vec = epoch_it->second;

    //check that augmented data is valid
    size_t valid_childs{0};
    for (auto &elem : epoch_aug_vec) {

      auto child = std::get<0>(elem);
      auto child_epoch = std::get<1>(elem);

      if (child && child->has_epoch(child_epoch)) {
        valid_childs++;
      } else {
        return hr_result_e::hr_invalid_epoch;
      }

    }

    if (valid_childs != epoch_aug_vec.size()) {
      return hr_result_e::hr_invalid_epoch;
    }

    for (auto &elem : epoch_aug_vec) {
      auto child = std::get<0>(elem);
      auto child_epoch = std::get<1>(elem);
      child->checkout_to_epoch(child_epoch);
    }

  }

  return hr_result_e::hr_success;

}

hr_result_e hist_doc_base_t::checkout_by_dist(int dist) {
  if (can_checkout_by_dist(dist)) {
    auto cur_it = find_hl(get_cur_epoch());
    std::advance(cur_it, dist);
    checkout_to_epoch(*cur_it);
    return hr_result_e::hr_success;
  } else {
    return hr_result_e::hr_error;
  }
}

hr_result_e hist_doc_base_t::checkout_forward() {
 return checkout_by_dist(1);
}

hr_result_e hist_doc_base_t::checkout_backward() {
  return checkout_by_dist(-1);
}

bool hist_doc_base_t::can_checkout_by_dist(int dist) {
  auto cur_it = find_hl(get_cur_epoch());
  if (cur_it == end(p_hist_line)) return false;
  int cur_idx = std::distance(begin(p_hist_line), cur_it);
  cur_idx += dist;
  return cur_idx >= 0 && cur_idx < static_cast<int>(p_hist_line.size());
}

bool hist_doc_base_t::can_checkout_forward() {
  return can_checkout_by_dist(1);
}

bool hist_doc_base_t::can_checkout_backward() {
  return can_checkout_by_dist(-1);
}

hr_result_e hist_doc_base_t::add_hs_child(hist_doc_base_t::self_t *child) {

  if (!child) return hr_result_e::hr_invalid_child;

  child->p_parent = this;
  p_childs.push_back(child);
  epoch_t cur_epoch = get_cur_epoch();
  auto epoch_it = find_hl(cur_epoch);
  auto epoch_dist = static_cast<size_t>(std::distance(begin(p_hist_line), epoch_it));
  for (size_t i = 0; i <= epoch_dist; i++)
    augment_epoch(i, child, child->get_cur_epoch());

  return hr_result_e::hr_success;

}

hist_doc_base_t::self_t *hist_doc_base_t::get_root() {
  if (p_parent)
    return p_parent->get_root();
  return this;
}

hist_doc_base_t::self_t *hist_doc_base_t::get_children(size_t idx) {
  if (idx < p_childs.size())
    return p_childs[idx];
  return nullptr;
}

std::optional<size_t> hist_doc_base_t::is_children(hist_doc_base_t::self_t *child) {
  if (!child) return std::nullopt;
  auto it1 = std::find(begin(p_childs), end(p_childs), child);
  return (it1 != end(p_childs) ? std::optional<size_t>{std::distance(begin(p_childs), it1)} :
                               std::nullopt);
}

hr_result_e hist_doc_base_t::remove_child(size_t child_id) {
  if (child_id < p_childs.size())
    return remove_child(p_childs[child_id]);
  return hr_result_e::hr_error;
}

hr_result_e hist_doc_base_t::remove_child(self_t *child) {

  auto it1 = std::find(begin(p_childs), end(p_childs), child);
  if (it1 != std::end(p_childs)) {

    for (auto &[k, v] : p_childs_states) {
      //std::cout << "@@@a " << k << " " << v.size() << std::endl;
      auto new_end_v =
          std::remove_if(begin(v), end(v), [child](std::tuple<self_t*, epoch_t> &elem) {
            return std::get<0>(elem) == child;
          });
      v.erase(new_end_v, v.end());
      //std::cout << "@@@b " << k << " " << v.size() << std::endl;
    }

    p_childs.erase(it1);
    return hr_result_e::hr_success;

  } else {
    return hr_result_e::hr_error;
  }

}

size_t hist_doc_base_t::get_children_count() { return p_childs.size(); }

} // namespace qpp::cad

} // namespace qpp
