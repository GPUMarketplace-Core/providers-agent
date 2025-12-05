#ifdef _WIN32
#include <gtest/gtest.h>
#include "gpu_probe/gpu_probe.h"

void get_gpu_info();

TEST(GPUProbeTest, InitializeAdapters) {
    EXPECT_TRUE(nvre::initialize_adapters());
}

TEST(GPUProbeTest, GetGPUInfo) {
    ASSERT_TRUE(nvre::initialize_adapters());
    EXPECT_NO_THROW(get_gpu_info());
}

TEST(GPUProbeTest, GetGPUVRAM) {
    ASSERT_TRUE(nvre::initialize_adapters());

    for (unsigned i = 0; i < nvre::adapters_count; i++)
    {
        nvre::NvHandle root = { 0 };
        ASSERT_EQ(nvre::alloc_root_handle(nvre::adapters[i], &root), NV_OK);

        nvre::NvHandle device = { 0 };
        ASSERT_EQ(nvre::alloc_nv0080_handle(nvre::adapters[i], root, { i }), NV_OK);

        nvre::NvHandle sub_device = { 0 };
        ASSERT_EQ(nvre::alloc_nv2080_handle(nvre::adapters[i], root, { i }, { i + 1 }), NV_OK);

        nvre::NV2080_CTRL_GPU_GET_PARTITIONS_PARAMS partitions_params = { 0 };
        EXPECT_EQ(nvre::nv2080_ctrl_gpu_get_partitions(nvre::adapters[i], root, { i + 1 }, partitions_params), NV_OK);

        nvre::free_handle(nvre::adapters[i], root, { i }, { i + 1 });
        nvre::free_handle(nvre::adapters[i], root, root, { i });
        nvre::free_handle(nvre::adapters[i], root, { 0 }, root);
    }
}
#endif // _WIN32
