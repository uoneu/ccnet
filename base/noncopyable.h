#ifndef CCNET_BASE_NONCOPYABLE_H
#define CCNET_BASE_NONCOPYABLE_H

namespace ccnet
{

// 表示不可copy    
// 阻止对象的copy行 effective c++6
//
//
class noncopyable
{
 protected:
  noncopyable() = default;
  ~noncopyable() = default;

 private:
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;
};


}

#endif  // CCNET_BASE_NONCOPYABLE_H
