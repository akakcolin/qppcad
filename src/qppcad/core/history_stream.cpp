#include <qppcad/core/history_stream.hpp>
#include <cassert>

namespace qpp {

namespace cad {

hist_doc_base_t::hist_doc_base_t() {

}

hist_doc_base_t::~hist_doc_base_t() {

}

void hist_doc_base_t::begin_recording(bool init_as_base_commit) {

  hist_doc_base_t *parent = p_super_parent ? p_super_parent : this;
  //must be false when we start recording
  assert(!parent->p_is_recording);
  parent->p_is_recording = true;
  parent->p_init_as_base_commit = init_as_base_commit;
  parent->p_commit_exclusive_on_change_old = parent->p_commit_exclusive_on_change;
  parent->p_commit_exclusive_on_change = false;

}

void hist_doc_base_t::end_recording() {

  hist_doc_base_t *parent = p_super_parent ? p_super_parent : this;
  //must be true when we stop recording
  assert(parent->p_is_recording);

  parent->record_impl(parent->p_init_as_base_commit);

  parent->p_is_recording = false;
  parent->p_init_as_base_commit = false;
  parent->p_commit_exclusive_on_change = parent->p_commit_exclusive_on_change_old;

}

hist_doc_base_t::epoch_t hist_doc_base_t::get_cur_epoch() {
  return p_cur_epoch;
}

hs_result_e hist_doc_base_t::set_cur_epoch(hist_doc_base_t::epoch_t cur_epoch, bool emit_event) {

  auto it_ce = std::find(std::begin(p_hist_line), std::end(p_hist_line), cur_epoch);
  if (it_ce != std::end(p_hist_line)) {
    epoch_t prev_epoch = p_cur_epoch;
    p_cur_epoch = cur_epoch;
    return emit_event ? on_epoch_changed(prev_epoch) : hs_result_e::hs_success;
  }

  return hs_result_e::hs_invalid_epoch;

}

hs_result_e hist_doc_base_t::on_epoch_changed(hist_doc_base_t::epoch_t prev_epoch) {
  return hs_result_e::hs_success;
}

hs_result_e hist_doc_base_t::on_epoch_removed(hist_doc_base_t::epoch_t target_epoch){
  return hs_result_e::hs_success;
}

hs_result_e hist_doc_base_t::commit_exclusive(hist_doc_base_t *child,
                                              std::optional<epoch_t> child_epoch) {

  auto [push_result, new_epoch] = push_epoch(std::nullopt, true);
  if (!push_result || !new_epoch) return hs_result_e::hs_error;
  on_commit_exclusive();

  if (child && child_epoch)
    this->augment_epoch(*new_epoch, child, *child_epoch);

  if (p_parent) {
    return p_parent->commit_exclusive(this, *new_epoch);
  } else {
    return hs_result_e::hs_success;
  }

}

void hist_doc_base_t::on_commit_exclusive() {

}

bool hist_doc_base_t::get_commit_exclusive_on_change() {
  return p_commit_exclusive_on_change;
}

void hist_doc_base_t::set_commit_exclusive_on_change(bool value) {
  p_commit_exclusive_on_change = value;
}

void hist_doc_base_t::set_delta_state_type(hist_doc_delta_state_e new_dstate) {
  p_dstate = new_dstate;
}

bool hist_doc_base_t::is_unmodified() {

  epoch_t cur_epoch = get_cur_epoch();

  std::vector<bool> childs_are_modified;

  for (auto child : p_childs)
    if (child) {

      auto epoch_it = p_childs_states.find(cur_epoch);
      if (epoch_it == end(p_childs_states)) {
        childs_are_modified.push_back(false);
        continue;
      }

      auto child_it = epoch_it->second.find(child);
      if (child_it == end(epoch_it->second)) {
        childs_are_modified.push_back(false);
        continue;
      }

      childs_are_modified.push_back(child->is_unmodified()
                                    && child->get_cur_epoch() == child_it->second.m_child_epoch);

    }

  bool all_childs_are_unmodified =
      p_childs.empty() ? true : std::all_of(begin(childs_are_modified),
                                            end(childs_are_modified),
                                            [](bool value) {return value;});

  return all_childs_are_unmodified && is_unmodified_impl();

}

bool hist_doc_base_t::is_unmodified_impl() {
  return true;
}

void hist_doc_base_t::update_super_root(hist_doc_base_t::self_t *new_super_root) {
  p_super_parent = new_super_root;
  for (auto child : p_childs)
    if (child) child->update_super_root(new_super_root);
}

hs_result_e hist_doc_base_t::reset() {

  std::vector<bool> child_reseted;
  for (auto child: p_childs)
    if (child) child_reseted.push_back(child->reset());

  hs_result_e self_rt = reset_impl();
  bool all_child_succeded =
      p_childs.empty() ? true : std::all_of(begin(child_reseted),
                                            end(child_reseted),
                                            [](bool value) {return value;});

  return self_rt == hs_result_e::hs_success && all_child_succeded ? hs_result_e::hs_success :
                                                                    hs_result_e::hs_error;
}

void hist_doc_base_t::record_impl(bool init_as_base_commit) {

  if (init_as_base_commit) {
    p_hist_line = {0};
    p_cur_epoch = 0;
  }

  for (auto child : p_childs)
    if (child) child->record_impl(init_as_base_commit);

}

hs_result_e hist_doc_base_t::reset_impl() {
  return hs_result_e::hs_success;
}

std::tuple<hs_result_e, std::optional<hist_doc_base_t::epoch_t> > hist_doc_base_t::push_epoch(
    std::optional<hist_doc_base_t::epoch_t> new_epoch_ex,
    bool checkout_to_new_epoch) {

  epoch_t cur_epoch = get_cur_epoch();

  epoch_t new_epoch = (new_epoch_ex) ?
                                     *new_epoch_ex :
                                     *std::max_element(p_hist_line.begin(), p_hist_line.end()) + 1;

  if (p_hist_line.empty()) {
    p_hist_line.push_back(new_epoch);
    if (checkout_to_new_epoch) checkout_to_epoch(new_epoch);
    return {hs_result_e::hs_success, new_epoch};
  }

  auto last_it = p_hist_line.rbegin();
  auto last_epoch = *last_it;

  if (new_epoch < last_epoch) {
    p_is_bad = true;
    return {hs_result_e::hs_invalid_epoch, std::nullopt};
  } else {

    auto cur_epoch_iter = std::find(begin(p_hist_line), end(p_hist_line), cur_epoch);
    auto cur_epoch_idx = std::distance(begin(p_hist_line), cur_epoch_iter);

    p_hist_line.insert(cur_epoch_iter + 1, new_epoch);

    cur_epoch_idx++;
    for (size_t i = cur_epoch_idx; i < p_hist_line.size(); i++)
      on_epoch_removed(p_hist_line[i]);

    p_hist_line.resize(cur_epoch_idx + 1);

    if (checkout_to_new_epoch) checkout_to_epoch(new_epoch);

    //copy child augmetns from previous epoch
    p_childs_states[new_epoch] = p_childs_states[cur_epoch];
    return {hs_result_e::hs_success, new_epoch};

  }

}

size_t hist_doc_base_t::get_history_size() const {
  return p_hist_line.size();
}

std::vector<hist_doc_base_t::epoch_t> hist_doc_base_t::get_history() const {
  return p_hist_line;
}

hs_result_e hist_doc_base_t::augment_epoch(hist_doc_base_t::epoch_t target_epoch,
                                           hist_doc_base_t::self_t *child,
                                           hist_doc_base_t::epoch_t child_epoch,
                                           bool alive) {

  if (!child) {
    return hs_result_e::hs_invalid_child;
  }

  if (find_hl(target_epoch) == std::end(p_hist_line)) {
    return hs_result_e::hs_invalid_epoch;
  }

  if (child->find_hl(child_epoch) == end(child->p_hist_line)) {
    return hs_result_e::hs_invalid_child_epoch;
  }

  p_childs_states[target_epoch][child] = {child_epoch, alive};

  return hs_result_e::hs_success;

}

size_t hist_doc_base_t::get_augmented_count(hist_doc_base_t::epoch_t target_epoch) const {
  auto aug_elist_it = p_childs_states.find(target_epoch);
  if (aug_elist_it == end(p_childs_states)) return 0;
  return aug_elist_it->second.size();
}

bool hist_doc_base_t::is_augmented_by(hist_doc_base_t::epoch_t target_epoch,
                                      hist_doc_base_t *child) {

  auto aug_elist_it = p_childs_states.find(target_epoch);
  if (aug_elist_it == end(p_childs_states)) return false;

  auto aug_it = aug_elist_it->second.find(child);
  if (aug_it == end(aug_elist_it->second)) return false;

  return true;

}

bool hist_doc_base_t::has_epoch(hist_doc_base_t::epoch_t target_epoch) {
  return find_hl(target_epoch) != end(p_hist_line);
}

hs_result_e hist_doc_base_t::remove_augment_from_epoch(hist_doc_base_t::self_t *child,
                                                       hist_doc_base_t::epoch_t target_epoch) {

  auto epoch_it = p_childs_states.find(target_epoch);
  if (epoch_it == end(p_childs_states)) return hs_result_e::hs_invalid_epoch;

  auto ch_epoch_it = epoch_it->second.find(child);
  if (ch_epoch_it == end(epoch_it->second)) return hs_result_e::hs_invalid_epoch;

  epoch_it->second.erase(ch_epoch_it);
  return hs_result_e::hs_success;

}

hs_result_e hist_doc_base_t::is_child_alive(epoch_t target_epoch, self_t* child) const {

  auto epoch_it = p_childs_states.find(target_epoch);
  if (epoch_it == end(p_childs_states)) return hs_result_e::hs_invalid_epoch;

  auto child_it = epoch_it->second.find(child);
  if (child_it == end(epoch_it->second)) return hs_result_e::hs_invalid_child;

  return child_it->second.m_is_alive ? hs_result_e::hs_true : hs_result_e::hs_false;

}

hs_result_e hist_doc_base_t::checkout_to_epoch(hist_doc_base_t::epoch_t target_epoch) {

  if (p_cur_epoch == target_epoch) {
    return hs_result_e::hs_success;
  }

  epoch_t prev_epoch = get_cur_epoch();
  hs_result_e cur_res = set_cur_epoch(target_epoch, false);

  if (cur_res != hs_result_e::hs_success) {
    return hs_result_e::hs_invalid_epoch;
  }

  epoch_t cur_epoch = get_cur_epoch();

  auto epoch_it = p_childs_states.find(cur_epoch);
  if (epoch_it != end(p_childs_states)) {

    auto &epoch_aug_vec = epoch_it->second;

    //check that augmented data is valid
    size_t valid_childs{0};
    for (auto &elem : epoch_aug_vec) {

      auto child = std::get<0>(elem);
      auto child_epoch_meta = std::get<1>(elem);

      if (child && child->has_epoch(child_epoch_meta.m_child_epoch)) {
        valid_childs++;
      } else {
        return hs_result_e::hs_invalid_epoch;
      }

    }

    if (valid_childs != epoch_aug_vec.size()) {
      return hs_result_e::hs_invalid_epoch;
    }

    for (auto &elem : epoch_aug_vec) {
      auto child = std::get<0>(elem);
      auto child_epoch_meta = std::get<1>(elem);
      child->checkout_to_epoch(child_epoch_meta.m_child_epoch);
    }

  }

  on_epoch_changed(prev_epoch);
  return hs_result_e::hs_success;

}

hs_result_e hist_doc_base_t::checkout_by_dist(int dist) {

  if (can_checkout_by_dist(dist)) {
    auto cur_it = find_hl(get_cur_epoch());
    std::advance(cur_it, dist);
    checkout_to_epoch(*cur_it);
    return hs_result_e::hs_success;
  } else {
    return hs_result_e::hs_error;
  }

}

bool hist_doc_base_t::can_checkout_by_dist(int dist) {

  auto cur_it = find_hl(get_cur_epoch());
  if (cur_it == end(p_hist_line)) return false;
  int cur_idx = std::distance(begin(p_hist_line), cur_it);
  cur_idx += dist;
  return cur_idx >= 0 && cur_idx < static_cast<int>(p_hist_line.size());

}

hs_result_e hist_doc_base_t::add_hs_child(hist_doc_base_t::self_t *child) {

  if (!child) return hs_result_e::hs_invalid_child;

  child->p_parent = this;
  p_childs.push_back(child);
  child->update_super_root(this);

  epoch_t cur_epoch = get_cur_epoch();
  auto epoch_it = find_hl(cur_epoch);
  auto epoch_dist = static_cast<size_t>(std::distance(begin(p_hist_line), epoch_it));

  // instead of adding augment to 0..cur_epoch mark 0..cur_epoch - 1 alive status
  // opposite to cur_epoch
  //augment_epoch(cur_epoch, child, get_cur_epoch());
  for (size_t i = 0; i <= epoch_dist; i++)
    augment_epoch(i, child, child->get_cur_epoch(), i == epoch_dist ? true : false);

  return hs_result_e::hs_success;

}

hist_doc_base_t::self_t *hist_doc_base_t::get_parent() {
  if (p_parent)
    return p_parent->get_parent();
  return this;
}

hist_doc_base_t::self_t *hist_doc_base_t::get_super_parent() {
  return p_super_parent;
}

hist_doc_base_t::self_t *hist_doc_base_t::get_child(size_t idx) const {
  if (idx < p_childs.size())
    return p_childs[idx];
  return nullptr;
}

std::optional<size_t> hist_doc_base_t::is_child(hist_doc_base_t::self_t *child) const {
  if (!child) return std::nullopt;
  auto it1 = std::find(begin(p_childs), end(p_childs), child);
  return (it1 != end(p_childs) ? std::optional<size_t>{std::distance(begin(p_childs), it1)} :
                                 std::nullopt);
}

hs_result_e hist_doc_base_t::remove_child(size_t child_id) {
  if (child_id < p_childs.size())
    return remove_child(p_childs[child_id]);
  return hs_result_e::hs_error;
}

hs_result_e hist_doc_base_t::remove_child(self_t *child) {

  auto it1 = std::find(begin(p_childs), end(p_childs), child);
  if (it1 != std::end(p_childs)) {

    for (auto &[k, v] : p_childs_states) {
      auto chld_epoch_it = v.find(child);
      if (chld_epoch_it != end(v)) v.erase(chld_epoch_it);
    }

    p_childs.erase(it1);
    return hs_result_e::hs_success;

  } else {
    return hs_result_e::hs_error;
  }

}

size_t hist_doc_base_t::get_children_count() const { return p_childs.size(); }

} // namespace qpp::cad

} // namespace qpp
