/*
 * SPDX-FileCopyrightText: Copyright (c) 2021-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NVIDIA_GXF_BT_PARALLEL_BEHAVIOR_HPP_
#define NVIDIA_GXF_BT_PARALLEL_BEHAVIOR_HPP_

#include <vector>

#include "gxf/std/codelet.hpp"
#include "gxf/std/controller.hpp"
#include "gxf/std/scheduling_terms.hpp"

namespace nvidia {
namespace gxf {

// Runs its child entities in parallel. By default, succeeds when all child
// entities succeed and fails when all child entities fail.
class ParallelBehavior : public Codelet {
 public:
  virtual ~ParallelBehavior() = default;

  gxf_result_t registerInterface(Registrar* registrar) override;
  gxf_result_t initialize() override;
  gxf_result_t tick() override;

 private:
  // parent codelet points to children's scheduling terms to schedule child
  // entities
  Parameter<std::vector<Handle<nvidia::gxf::BTSchedulingTerm> > > children_;
  std::vector<Handle<nvidia::gxf::BTSchedulingTerm> > children;
  std::vector<gxf_uid_t> children_eid;
  // its own scheduling term to start/stop itself
  Parameter<Handle<nvidia::gxf::BTSchedulingTerm> > s_term_;
  Handle<nvidia::gxf::BTSchedulingTerm> s_term;
  size_t getNumChildren() const;
  entity_state_t GetChildStatus(size_t child_id);
  gxf_result_t startChild(size_t child_id);
  gxf_result_t stopAllChild();
  Parameter<int64_t> success_threshold_;
  Parameter<int64_t> failure_threshold_;
  SchedulingConditionType ready_conditions;
  SchedulingConditionType never_conditions;
};

}  // namespace gxf
}  // namespace nvidia

#endif  // NVIDIA_GXF_BT_PARALLEL_BEHAVIOR_HPP_
