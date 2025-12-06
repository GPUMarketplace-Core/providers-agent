#ifdef _WIN32
#include <gtest/gtest.h>
#include "gpu_probe/gpu_probe.h"

using namespace providers_agent;

TEST(GPUProbeTest, Initialize) {
    gpu_probe::GPUProbe probe;
    EXPECT_EQ(probe.initialize(), common::Status::OK);
}

TEST(GPUProbeTest, LogToFile) {
    gpu_probe::GPUProbe probe;
    EXPECT_EQ(probe.initialize(), common::Status::OK);
    EXPECT_EQ(probe.logToFile("test_gpu_info.json"), common::Status::OK);
}
#endif // _WIN32
