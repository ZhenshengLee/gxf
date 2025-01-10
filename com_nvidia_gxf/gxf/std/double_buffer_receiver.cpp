/*
Copyright (c) 2020-2023, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#include "gxf/std/double_buffer_receiver.hpp"

#include <memory>
#include <utility>

namespace nvidia {
namespace gxf {

gxf_result_t DoubleBufferReceiver::registerInterface(Registrar* registrar) {
  Expected<void> result;
  result &= registrar->parameter(capacity_, "capacity", "Capacity", "", 1UL);
  result &= registrar->parameter(policy_, "policy", "Policy", "0: pop, 1: reject, 2: fault", 2UL);
  return ToResultCode(result);
}

gxf_result_t DoubleBufferReceiver::initialize() {
  if (capacity_ == 0) { return GXF_ARGUMENT_OUT_OF_RANGE; }
  queue_ = std::make_unique<queue_t>(
      capacity_, (::gxf::staging_queue::OverflowBehavior)(policy_.get()), Entity{});
  return GXF_SUCCESS;
}

gxf_result_t DoubleBufferReceiver::deinitialize() {
  // Empty queue
  if (!queue_) {
    GXF_LOG_ERROR("Bad Queue in DoubleBufferReceiver with name '%s' and cid [C%05zu]", name(),
                  cid());
    return GXF_CONTRACT_INVALID_SEQUENCE;
  }
  if (size() != 0) {
    GXF_LOG_WARNING("Unprocessed num of message %lu in queue: %s:%s", size(), entity().name(),
                    name());
  }
  queue_->popAll();
  queue_->sync();
  queue_->popAll();
  return GXF_SUCCESS;
}

gxf_result_t DoubleBufferReceiver::pop_abi(gxf_uid_t* uid) {
  if (uid == nullptr) { return GXF_ARGUMENT_NULL; }
  if (!queue_) {
    GXF_LOG_ERROR("Bad Queue in DoubleBufferReceiver with name '%s' and cid [C%05zu]", name(),
                  cid());
    return GXF_FAILURE;
  }

  Entity entity = queue_->pop();
  if (entity.is_null()) {
    GXF_LOG_VERBOSE("Received null entity in DoubleBufferReceiver with name '%s' cid [C%05zu]",
                    name(), cid());
    return GXF_FAILURE;
  }

  // We do not want to decrement the ref count (which will happen in the Entity destructor)
  // as we expect the caller to do that.
  const gxf_result_t code = GxfEntityRefCountInc(context(), entity.eid());
  if (code != GXF_SUCCESS) { return code; }

  *uid = entity.eid();
  return GXF_SUCCESS;
}

gxf_result_t DoubleBufferReceiver::push_abi(gxf_uid_t other) {
  if (!queue_) { return GXF_FAILURE; }

  auto maybe = Entity::Shared(context(), other);
  if (!maybe) { return maybe.error(); }

  if (!queue_->push(std::move(maybe.value()))) {
    GXF_LOG_WARNING("Push failed on '%s'", name());
    return GXF_EXCEEDING_PREALLOCATED_SIZE;
  }

  return GXF_SUCCESS;
}

gxf_result_t DoubleBufferReceiver::peek_abi(gxf_uid_t* uid, int32_t index) {
  if (uid == nullptr) { return GXF_ARGUMENT_NULL; }
  if (!queue_) { return GXF_FAILURE; }

  *uid = queue_->peek(index).eid();
  if (*uid == kNullUid) { return GXF_FAILURE; }

  return GXF_SUCCESS;
}

gxf_result_t DoubleBufferReceiver::peek_back_abi(gxf_uid_t* uid, int32_t index) {
  if (uid == nullptr) { return GXF_ARGUMENT_NULL; }
  if (!queue_) { return GXF_FAILURE; }

  *uid = queue_->peek_backstage(index).eid();
  if (*uid == kNullUid) { return GXF_FAILURE; }

  return GXF_SUCCESS;
}

size_t DoubleBufferReceiver::capacity_abi() {
  return queue_ ? queue_->capacity() : 0;
}

size_t DoubleBufferReceiver::size_abi() {
  return queue_ ? queue_->size() : 0;
}

gxf_result_t DoubleBufferReceiver::receive_abi(gxf_uid_t* uid) {
  return pop_abi(uid);
}

size_t DoubleBufferReceiver::back_size_abi() {
  return queue_ ? queue_->back_size() : 0;
}

gxf_result_t DoubleBufferReceiver::sync_abi() {
  if (!queue_) { return GXF_FAILURE; }

  if (!queue_->sync()) {
    GXF_LOG_WARNING("Sync failed on '%s'", name());
    return GXF_EXCEEDING_PREALLOCATED_SIZE;
  }

  return GXF_SUCCESS;
}

gxf_result_t DoubleBufferReceiver::sync_io_abi() {
  return GXF_SUCCESS;
}

}  // namespace gxf
}  // namespace nvidia
