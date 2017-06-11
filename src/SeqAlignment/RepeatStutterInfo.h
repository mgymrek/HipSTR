#ifndef REPEAT_STUTTER_INFO_H_
#define REPEAT_STUTTER_INFO_H_

#include <algorithm>
#include <string>

#include "../error.h"
#include "../stutter_model.h"

// Maximum number of repeats that PCR stutter can add or remove from block's sequence
const int    MAX_STUTTER_REPEAT_INS = 3;
const int    MAX_STUTTER_REPEAT_DEL = -3;
const double LARGE_NEGATIVE = -10e6;

class RepeatStutterInfo {
 private:
  int period_, max_ins_, max_del_;
  StutterModel* stutter_model_;
  std::vector<int> allele_sizes_;

  RepeatStutterInfo(){
    period_        = -1;
    max_ins_       = -1;
    max_del_       = 1;
    stutter_model_ = NULL;
  }

 public:
  RepeatStutterInfo(int period, const std::string& ref_allele, const StutterModel* stutter_model){
    period_        = period;
    max_ins_       = MAX_STUTTER_REPEAT_INS*period_;
    max_del_       = MAX_STUTTER_REPEAT_DEL*period_;
    stutter_model_ = stutter_model->copy();
    allele_sizes_.push_back(ref_allele.size());
  }

  RepeatStutterInfo(const RepeatStutterInfo& other){
    period_        = other.period_;
    max_ins_       = other.max_ins_;
    max_del_       = other.max_del_;
    stutter_model_ = (other.stutter_model_ == NULL ? NULL : other.stutter_model_->copy());
    allele_sizes_  = other.allele_sizes_;
  }

  RepeatStutterInfo& operator=(const RepeatStutterInfo& other){
    if (&other != this){
      if (stutter_model_ != NULL) delete stutter_model_;
      period_        = other.period_;
      max_ins_       = other.max_ins_;
      max_del_       = other.max_del_;
      stutter_model_ = (other.stutter_model_ == NULL ? NULL : other.stutter_model_->copy());
      allele_sizes_  = other.allele_sizes_;
    }
    return *this;
  }

  ~RepeatStutterInfo(){
    if (stutter_model_ != NULL)
      delete stutter_model_;
  }

  void set_stutter_model(const StutterModel* model){
    if (stutter_model_ != NULL)
      delete stutter_model_;
    stutter_model_ = model->copy();
  }

  inline StutterModel* get_stutter_model() const  { return stutter_model_;  }
  inline int get_period()                  const  { return period_;         }
  inline int max_insertion()               const  { return max_ins_;        }
  inline int max_deletion()                const  { return max_del_;        }

  void add_alternate_allele(const std::string& alt_allele){
    allele_sizes_.push_back((int)alt_allele.size());
  }

  inline double log_prob_pcr_artifact(int seq_index, int artifact_size) const {
    int read_size = allele_sizes_.at(seq_index)+artifact_size;
    if (artifact_size == 0)
      return stutter_model_->log_stutter_pmf(allele_sizes_[seq_index], read_size);
    else if (artifact_size > 0)
      return (artifact_size > max_ins_ ? LARGE_NEGATIVE : stutter_model_->log_stutter_pmf(allele_sizes_[seq_index], read_size));
    else
      return (artifact_size < max_del_ || read_size < 0 ? LARGE_NEGATIVE : stutter_model_->log_stutter_pmf(allele_sizes_[seq_index], read_size));
  }
};
#endif
