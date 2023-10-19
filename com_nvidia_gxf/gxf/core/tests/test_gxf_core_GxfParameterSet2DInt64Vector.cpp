/*
Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "common/assert.hpp"
#include "gtest/gtest.h"
#include "gxf/core/gxf.h"

namespace {

constexpr const char* kGxeManifestFilename = "gxf/gxe/manifest.yaml";

}  // namespace

class GxfParameterSet2DInt64Vector_Test : public ::testing::Test {
 public:
  void SetUp() {
    GXF_ASSERT_SUCCESS(GxfContextCreate(&context));
    GXF_ASSERT_SUCCESS(GxfLoadExtensions(context, &info));
    GXF_ASSERT_SUCCESS(GxfCreateEntity(context, &entity_create_info, &eid));
    GXF_ASSERT_SUCCESS(GxfComponentTypeId(context, "nvidia::gxf::BlockMemoryPool", &tid));
    GXF_ASSERT_SUCCESS(GxfComponentAdd(context, eid, tid, "test", &cid));
  }

  void TearDown() {
    GXF_ASSERT_SUCCESS(GxfEntityDestroy(context, eid));
    GXF_ASSERT_SUCCESS(GxfContextDestroy(context));
  }

 protected:
  gxf_context_t context = kNullContext;
  const GxfLoadExtensionsInfo info{nullptr, 0, &kGxeManifestFilename, 1, nullptr};
  gxf_uid_t eid = kNullUid;
  const GxfEntityCreateInfo entity_create_info = {0};
  gxf_tid_t tid = GxfTidNull();
  gxf_uid_t cid = kNullUid;
  uint64_t height;
  uint64_t* height_ptr = &height;
  uint64_t width;
  uint64_t* width_ptr = &width;
};

TEST_F(GxfParameterSet2DInt64Vector_Test, ValidParameter) {
  int64_t int64_1d_vector_1[] = {0, 5, -8, 55, -94};
  int64_t int64_1d_vector_2[] = {26, -71, -1, 666, 3600};
  int64_t* int64_2d_vector[] = {int64_1d_vector_1, int64_1d_vector_2};
  GXF_ASSERT_SUCCESS(
      GxfParameterSet2DInt64Vector(context, cid, "int64_2d_vector", int64_2d_vector, 2, 5));
  GXF_ASSERT_SUCCESS(
      GxfParameterGet2DInt64VectorInfo(context, cid, "int64_2d_vector", height_ptr, width_ptr));
  int64_t vector_1[width];
  int64_t vector_2[width];
  int64_t* value[] = {vector_1, vector_2};
  GXF_ASSERT_SUCCESS(GxfParameterGet2DInt64Vector(context, cid, "int64_2d_vector", value,
                                                    height_ptr, width_ptr));
  for (uint32_t i = 0; i < height; i++) {
    for (uint32_t j = 0; j < width; j++) { GXF_ASSERT_EQ(value[i][j], int64_2d_vector[i][j]); }
  }
}

TEST_F(GxfParameterSet2DInt64Vector_Test, InvalidContext) {
  int64_t int64_1d_vector_1[] = {0, 5, -8, 55, -94};
  int64_t int64_1d_vector_2[] = {26, -71, -5, 666, 3600};
  int64_t* int64_2d_vector[] = {int64_1d_vector_1, int64_1d_vector_2};
  GXF_ASSERT_EQ(
      GxfParameterSet2DInt64Vector(kNullContext, cid, "int64_2d_vector", int64_2d_vector, 2, 5),
      GXF_CONTEXT_INVALID);
}

TEST_F(GxfParameterSet2DInt64Vector_Test, NullValue) {
  GXF_ASSERT_EQ(GxfParameterSet2DInt64Vector(context, cid, "int64_1d_vector", nullptr, 2, 5),
                GXF_ARGUMENT_NULL);
}

TEST_F(GxfParameterSet2DInt64Vector_Test, OverwritingValidParameterType) {
  int64_t int64_1d_vector_1_1[] = {0, 5, -8, 55, -96};
  int64_t int64_1d_vector_1_2[] = {89, -57, -5, 666, 3500};
  int64_t* int64_2d_vector_1[] = {int64_1d_vector_1_1, int64_1d_vector_1_2};
  GXF_ASSERT_SUCCESS(
      GxfParameterSet2DInt64Vector(context, cid, "int64_2d_vector", int64_2d_vector_1, 2, 5));
  GXF_ASSERT_SUCCESS(
      GxfParameterGet2DInt64VectorInfo(context, cid, "int64_2d_vector", height_ptr, width_ptr));
  int64_t vector_1_1[width];
  int64_t vector_1_2[width];
  int64_t* value1[] = {vector_1_1, vector_1_2};
  GXF_ASSERT_SUCCESS(GxfParameterGet2DInt64Vector(context, cid, "int64_2d_vector", value1,
                                                    height_ptr, width_ptr));
  for (uint32_t i = 0; i < height; i++) {
    for (uint32_t j = 0; j < width; j++) { GXF_ASSERT_EQ(value1[i][j], int64_2d_vector_1[i][j]); }
  }

  int64_t int64_1d_vector_2_1[] = {5, -76, -83, 55, -638};
  int64_t int64_1d_vector_2_2[] = {893, -527, -25, 66634, 350340};
  int64_t* int64_2d_vector_2[] = {int64_1d_vector_2_1, int64_1d_vector_2_2};
  GXF_ASSERT_SUCCESS(
      GxfParameterSet2DInt64Vector(context, cid, "int64_2d_vector", int64_2d_vector_2, 2, 5));
  GXF_ASSERT_SUCCESS(
      GxfParameterGet2DInt64VectorInfo(context, cid, "int64_2d_vector", height_ptr, width_ptr));
  int64_t vector_2_1[width];
  int64_t vector_2_2[width];
  int64_t* value2[] = {vector_2_1, vector_2_2};
  GXF_ASSERT_SUCCESS(GxfParameterGet2DInt64Vector(context, cid, "int64_2d_vector", value2,
                                                    height_ptr, width_ptr));
  for (uint32_t i = 0; i < height; i++) {
    for (uint32_t j = 0; j < width; j++) { GXF_ASSERT_EQ(value2[i][j], int64_2d_vector_2[i][j]); }
  }
}

TEST_F(GxfParameterSet2DInt64Vector_Test, SettingInvalidParameterType) {
  double float_1d_vector_1[] = {0.01, 5.6, -8.3, 55, -96.99};
  double float_1d_vector_2[] = {89, -57, -0.5, 666.6, 3500};
  double* float_2d_vector[] = {float_1d_vector_1, float_1d_vector_2};
  GXF_ASSERT_SUCCESS(
      GxfParameterSet2DFloat64Vector(context, cid, "int64_2d_vector", float_2d_vector, 2, 5));
  int64_t int64_1d_vector_1[] = {0, 5, -8, 55, -96};
  int64_t int64_1d_vector_2[] = {89, -57, -5, 666, 3500};
  int64_t* int64_2d_vector[] = {int64_1d_vector_1, int64_1d_vector_2};
  GXF_ASSERT_EQ(
      GxfParameterSet2DInt64Vector(context, cid, "int64_2d_vector", int64_2d_vector, 2, 5),
      GXF_PARAMETER_INVALID_TYPE);
}

TEST_F(GxfParameterSet2DInt64Vector_Test, BoundaryValue) {
  int64_t int64_1d_vector_1[] = {INT64_MAX, INT64_MIN};
  int64_t int64_1d_vector_2[] = {INT64_MIN, INT64_MAX};
  int64_t* int64_2d_vector[] = {int64_1d_vector_1, int64_1d_vector_2};
  GXF_ASSERT_SUCCESS(
      GxfParameterSet2DInt64Vector(context, cid, "int64_2d_vector", int64_2d_vector, 2, 2));
  GXF_ASSERT_SUCCESS(
      GxfParameterGet2DInt64VectorInfo(context, cid, "int64_2d_vector", height_ptr, width_ptr));
  int64_t vector_1[width];
  int64_t vector_2[width];
  int64_t* value[] = {vector_1, vector_2};
  GXF_ASSERT_SUCCESS(GxfParameterGet2DInt64Vector(context, cid, "int64_2d_vector", value,
                                                    height_ptr, width_ptr));
  for (uint32_t i = 0; i < height; i++) {
    for (uint32_t j = 0; j < width; j++) { GXF_ASSERT_EQ(value[i][j], int64_2d_vector[i][j]); }
  }
}
