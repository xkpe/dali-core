#ifndef __DALI_INTEGRATION_PROFILING_H__
#define __DALI_INTEGRATION_PROFILING_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>

namespace Dali
{

namespace Integration
{

enum ProfilingType
{
  PROFILING_TYPE_PAN_GESTURE,

  PROFILING_TYPE_END
};

/**
 * Called by adaptor to provide profiling information.
 *
 * @pre Should be called after Core creation.
 * @param type The type of profiling information to output.
 */
DALI_IMPORT_API void EnableProfiling( ProfilingType type );

} // namespace Integration

} // namespace Dali

#endif // __DALI_INTEGRATION_PROFILING_H__