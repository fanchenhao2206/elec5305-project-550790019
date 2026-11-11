//
// Tim Fan <fanchenhao2206@icloud.com>

#pragma once

#include "IPlugPlatform.h"
#include "Filter.h"

BEGIN_IPLUG_NAMESPACE

template <typename T>
class picoDAWFilter : public IFilter<T>
{
public:
  picoDAWFilter()
  : IFilter<T>()
  {
  }
};

END_IPLUG_NAMESPACE
