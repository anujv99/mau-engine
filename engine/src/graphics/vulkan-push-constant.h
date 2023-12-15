#pragma once

#include <engine/types.h>
#include <engine/utils/handle.h>
#include "common.h"

namespace mau {

  class CommandBuffer;
  class Pipeline;
  class RTPipeline;

  class PushConstantBase: public HandledObject {
  public:
    PushConstantBase(TUint64 size);
    virtual ~PushConstantBase();

  public:
    VkPushConstantRange GetRange() const;
    void                Bind(Handle<CommandBuffer> cmd, Handle<Pipeline> pipeline) const;
    void                Bind(Handle<CommandBuffer> cmd, Handle<RTPipeline> pipeline) const;

  protected:
    void SetData(const void *const data, TUint64 size);
    void SetData(const void *const data);

  protected:
    void   *m_Data = nullptr;
    TUint64 m_Size = 0u;
  };

  template <typename T> class PushConstant: public PushConstantBase {
  public:
    PushConstant(const T &init);
    ~PushConstant();

  public:
    void Update(const T &data);
    T    GetData() const;
  };

  template <typename T> inline PushConstant<T>::PushConstant(const T &init): PushConstantBase(sizeof(T)) { SetData(reinterpret_cast<const void *>(&init)); }

  template <typename T> inline PushConstant<T>::~PushConstant() { }

  template <typename T> inline void PushConstant<T>::Update(const T &data) { SetData(reinterpret_cast<const void *>(&data)); }

  template <typename T> inline T PushConstant<T>::GetData() const {
    ASSERT(m_Data != nullptr);

    return *(reinterpret_cast<T *>(m_Data));
  }

} // namespace mau
