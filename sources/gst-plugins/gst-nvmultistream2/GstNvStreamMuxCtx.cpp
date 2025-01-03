/*
 * SPDX-FileCopyrightText: Copyright (c) 2021-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#include "GstNvStreamMuxCtx.h"


GstNvStreamMuxCtx::GstNvStreamMuxCtx() : mutex(), audioParamsMap()
{
}


void GstNvStreamMuxCtx::SaveAudioParams(uint32_t padId, uint32_t sourceId, NvBufAudioParams audioParams)
{
    std::unique_lock<std::mutex> lck(mutex);
    audioParamsMap[padId] = audioParams;
    audioParamsMap[padId].sourceId = sourceId;
    audioParamsMap[padId].padId = padId;
}

NvBufAudioParams GstNvStreamMuxCtx::GetAudioParams(uint32_t padId)
{
    std::unique_lock<std::mutex> lck(mutex);
    return audioParamsMap[padId];
}

void GstNvStreamMuxCtx::SetMemTypeNVMM(uint32_t padId, bool isNVMM)
{
    std::unique_lock<std::mutex> lck(mutex);
    isNVMMMap[padId] = isNVMM;
}

bool GstNvStreamMuxCtx::IsMemTypeNVMM(uint32_t padId)
{
    std::unique_lock<std::mutex> lck(mutex);
    return isNVMMMap[padId];
}
