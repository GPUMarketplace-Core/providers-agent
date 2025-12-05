#include "gpu_probe/gpu_probe.h"
#include <iostream>
#include <vector>
#include <string>

static void get_gpu_info()
{
    if (!nvre::initialize_adapters())
    {
        return;
    }

    for (unsigned i = 0; i < nvre::adapters_count; i++)
    {
        nvre::NvHandle root = { 0 };
        if (nvre::alloc_root_handle(nvre::adapters[i], &root) != NV_OK)
        {
            continue;
        }

        nvre::NvHandle device = { 0 };
        if (nvre::alloc_nv0080_handle(nvre::adapters[i], root, { i }) != NV_OK)
        {
            nvre::free_handle(nvre::adapters[i], root, { 0 }, root);
            continue;
        }

        nvre::NvHandle sub_device = { 0 };
        if (nvre::alloc_nv2080_handle(nvre::adapters[i], root, { i }, { i + 1 }) != NV_OK)
        {
            nvre::free_handle(nvre::adapters[i], root, root, { i });
            nvre::free_handle(nvre::adapters[i], root, { 0 }, root);
            continue;
        }

        std::string name;
        if (nvre::nv2080_ctrl_gpu_get_name(nvre::adapters[i], root, { i + 1 }, name) == NV_OK)
        {
            std::cout << "Found GPU:" << name << std::endl;
        }

        nvre::NV2080_CTRL_GPU_GET_PARTITIONS_PARAMS partitions_params = { 0 };
        if (nvre::nv2080_ctrl_gpu_get_partitions(nvre::adapters[i], root, { i + 1 }, partitions_params) == NV_OK)
        {
            if (partitions_params.validPartitionCount > 0)
            {
                std::cout << "  VRAM: " << partitions_params.queryPartitionInfo[0].memSize << " bytes" << std::endl;
            }
        }

        std::string info;
        if (nvre::nv2080_ctrl_gpu_get_gid_info(nvre::adapters[i], root, { i + 1 }, info) == NV_OK)
        {
            // Do something with the info
        }

        nvre::free_handle(nvre::adapters[i], root, { i }, { i + 1 });
        nvre::free_handle(nvre::adapters[i], root, root, { i });
        nvre::free_handle(nvre::adapters[i], root, { 0 }, root);
    }
}
