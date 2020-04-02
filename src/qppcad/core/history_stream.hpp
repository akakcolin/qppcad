﻿#ifndef QPPCAD_HISTORY_STREAM
#define QPPCAD_HISTORY_STREAM

#include <qppcad/core/qppcad.hpp>

namespace qpp {

  namespace cad {

    class hist_doc_base_t;
    using epoch_t = std::size_t;

    enum hist_doc_delta_state_e {
      delta_instant,
      delta_incremental
    };

    enum hr_result_e {
      hr_error = 0,
      hr_success = 1,
      hr_invalid_epoch = 2,
      hr_epoch_ill_defined = 3,
      hr_invalid_child = 4,
      hr_invalid_child_epoch = 5
    };

    /**
     * @brief The hist_doc_base_t class
     */
    class hist_doc_base_t {

    public:

      using epoch_t = std::size_t;
      using self_t = hist_doc_base_t;

    private:

      epoch_t p_cur_epoch{0};
      self_t *p_parent{nullptr};
      std::vector<self_t*> p_childs;
      std::map<epoch_t, std::vector<std::tuple<self_t*, epoch_t>>> p_childs_states;
      std::set<epoch_t> p_hist_line{0};
      bool p_is_bad{false};
      bool p_is_dirty{false};
      hist_doc_delta_state_e p_dstate{hist_doc_delta_state_e::delta_instant};

    public:

      hist_doc_base_t ();
      virtual ~hist_doc_base_t();

      /**
       * @brief get_cur_epoch
       * @return
       */
      epoch_t get_cur_epoch();

      /**
       * @brief set_cur_epoch
       * @param cur_epoch
       * @return
       */
      hr_result_e set_cur_epoch(epoch_t cur_epoch);

      virtual hr_result_e on_epoch_changed(epoch_t prev_epoch);

      /**
       * @brief get_delta_state_type - not implemented yet
       * @return
       */
      hist_doc_delta_state_e get_delta_state_type();

      /**
       * @brief set_delta_state_type - not implemented yet
       * @param new_dstate
       */
      void set_delta_state_type(hist_doc_delta_state_e new_dstate);

      /**
       * @brief is_dirty - not implemented yet
       * @return
       */
      bool is_dirty();

      /**
       * @brief mark_as_dirty - not implemented yet
       */
      void mark_as_dirty();

      /**
       * @brief push_epoch
       * @param new_epoch
       * @return
       */
      std::tuple<hr_result_e, std::optional<epoch_t>> push_epoch(
          std::optional<epoch_t> new_epoch_ex = std::nullopt);

      /**
       * @brief get_history_size
       */
      size_t get_history_size();

      /**
       * @brief augment_epoch
       * @param child pointer to child
       * @param child_epoch
       * @param target_epoch
       */
      hr_result_e augment_epoch(epoch_t target_epoch, self_t* child, epoch_t child_epoch);

      /**
       * @brief get_augmented_count
       * @param target_epoch
       * @return
       */
      size_t get_augmented_count(epoch_t target_epoch);

      /**
       * @brief has_epoch
       * @param target_epoch
       * @return
       */
      bool has_epoch(epoch_t target_epoch);

      /**
       * @brief remove_augment_from_epoch
       * @param child
       * @param child_epoch
       * @param target_epoch
       * @return
       */
      hr_result_e remove_augment_from_epoch(self_t* child, epoch_t child_epoch,
                                            epoch_t target_epoch);

      /**
       * @brief checkout_to_epoch
       * @param target_epoch
       * @return
       */
      hr_result_e checkout_to_epoch(epoch_t target_epoch);

      /**
       * @brief add_child
       * @param child
       */
      void add_child(self_t *child);

      /**
       * @brief get_root
       * @return
       */
      self_t *get_root();

      /**
       * @brief get_children
       * @param idx
       * @return
       */
      self_t *get_children(size_t idx);

      /**
       * @brief is_children
       * @param child
       * @return
       */
      std::optional<size_t> is_children(self_t *child);

      /**
       * @brief remove_child
       * @param child_id
       */
      void remove_child(size_t child_id);

      /**
       * @brief remove_child
       * @param child
       */
      void remove_child(self_t *child);

      /**
       * @brief get_children_count
       */
      size_t get_children_count();
      //end of children stuff

    };

    template<typename STORED_TYPE>
    class hist_doc_t : public hist_doc_base_t {

    private:

      STORED_TYPE p_cur_value;
      std::map<epoch_t, STORED_TYPE> p_stored_values;

    public:

      hr_result_e push_epoch_ex(STORED_TYPE &&new_val,
                                std::optional<epoch_t> new_epoch = std::nullopt);

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
