#ifndef CCNET_BASE_TYPES_H
#define CCNET_BASE_TYPES_H

#include <stdint.h>
#include <string>

#ifndef NDEBUG
#include <assert.h>
#endif

#include "./noncopyable.h"

///
/// The most common stuffs.
///
namespace ccnet
{

using std::string;


// 能力比static_cast要弱,down_cast会报错
// use like this implicit_cast<superFoo>(foo)
template<typename To, typename From>
inline To implicit_cast(From const &f)
{
  return f;
}


// use like this: down_cast<T*>(foo)
// 函数模板类型除了从实参推导,也可以明确指定类型
// so we only accept pointers
//

template<typename To, typename From>     
inline To down_cast(From* f)                    
{
  if (false)
  {
    implicit_cast<From*, To>(0);
  }

#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
  assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif
  return static_cast<To>(f);
}

}

#endif
