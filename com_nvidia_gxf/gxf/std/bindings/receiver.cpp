/*
Copyright (c) 2021-2022, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "gxf/std/receiver.hpp"
#include "gxf/std/double_buffer_receiver.hpp"
#include "gxf/core/component.hpp"

template <typename S>
nvidia::gxf::Expected<nvidia::gxf::Handle<S>> getHandle(gxf_context_t context, gxf_uid_t cid, const char* name) {
  gxf_uid_t eid;
  std::string component_name;
  const std::string tag = std::string(name);
  const size_t pos = tag.find('/');

  if (pos == std::string::npos) {
    // Get the entity of this component
    const gxf_result_t result_1 = GxfComponentEntity(context, cid, &eid);
    if (result_1 != GXF_SUCCESS) {
      GXF_LOG_ERROR("%s", GxfResultStr(result_1));
      throw std::runtime_error(GxfResultStr(result_1));
    }
    component_name = tag;
  } else {
    // Split the tag into entity and component name
    const std::string entity_name = tag.substr(0, pos);
    component_name = tag.substr(pos + 1);
    // Search for the entity
    const gxf_result_t result_1 = GxfEntityFind(context, entity_name.c_str(), &eid);
    if (result_1 != GXF_SUCCESS) {
      GXF_LOG_ERROR(
          "[E%05zu] Could not find entity '%s' while parsing parameter '%s' of component %zu",
          eid, entity_name.c_str(), tag.c_str(), cid);
      throw std::runtime_error(GxfResultStr(result_1));
    }
  }
  // Get the type id of the component we are are looking for.
  gxf_tid_t tid;
  const gxf_result_t result_1 = GxfComponentTypeId(context, nvidia::TypenameAsString<S>(), &tid);
  if (result_1 != GXF_SUCCESS) {
    GXF_LOG_ERROR("%s", GxfResultStr(result_1));
    throw std::runtime_error(GxfResultStr(result_1));
  }
  gxf_uid_t cid2;
  // Find the component in the indicated entity
  const gxf_result_t result_2 =
      GxfComponentFind(context, eid, tid, component_name.c_str(), nullptr, &cid2);
  if (result_2 != GXF_SUCCESS) {
    GXF_LOG_ERROR("%s", GxfResultStr(result_2));
    throw std::runtime_error(GxfResultStr(result_2));
  }


  auto handle = nvidia::gxf::Handle<S>::Create(context, cid2);
  return handle;
}

PYBIND11_MODULE(receiver_pybind, m) {
  pybind11::class_<nvidia::gxf::Receiver>(m, "Receiver")
      .def("receive",
           [](nvidia::gxf::Receiver& r) {
             auto message = r.receive();
             if (!message || message.value().is_null()) {
               if (!message) { GXF_LOG_ERROR("No Message"); }
               if (message.value().is_null()) { GXF_LOG_ERROR("Message Null"); }
               return nvidia::gxf::Entity();
             }
             return message.value();
           })
      .def("sync", [](nvidia::gxf::Receiver &r){
        auto result = r.sync();
        if(!result){
          GXF_LOG_ERROR("Sync Failed");
          std::runtime_error(GxfResultStr(result.error()));
        }
        return;
      })
      .def("back_size", &nvidia::gxf::Receiver::back_size)
      .def("size", &nvidia::gxf::Receiver::size)
      .def("capacity", &nvidia::gxf::Receiver::capacity)
      .def("get", [](gxf_context_t context, gxf_uid_t cid, const char* name) {
          std::vector<nvidia::gxf::Receiver*> result;
          auto maybe_receivers = getHandle<nvidia::gxf::Receiver>(context, cid, name);
          if (!maybe_receivers) {
            // GXF_LOG_ERROR("[E%05zu] Couldn't get receivers", this->eid());
            throw std::runtime_error(GxfResultStr(GXF_PARAMETER_NOT_INITIALIZED));
          }
          return maybe_receivers.value().get();
      },  pybind11::return_value_policy::reference);
  pybind11::class_<nvidia::gxf::DoubleBufferReceiver, nvidia::gxf::Receiver>(
      m, "DoubleBufferReceiver");
}
