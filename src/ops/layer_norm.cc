#include "ctranslate2/ops/layer_norm.h"

#define EPSILON 0.000001f

namespace ctranslate2 {
  namespace ops {

    template <Device D, typename T>
    void LayerNorm::compute(const StorageView& beta,
                            const StorageView& gamma,
                            const StorageView& input,
                            StorageView& output) const {
      static thread_local StorageView tmp(input.device(), input.dtype());
      size_t depth = input.dim(-1);
      size_t batch_size = input.size() / depth;
      tmp.resize({depth});
      for (size_t i = 0; i < batch_size; ++i) {
        const auto* x = input.data<T>() + i * depth;
        auto* y = output.data<T>() + i * depth;
        auto mean = primitives<>::mean(x, depth);
        primitives<>::sub(mean, x, y, depth);
        primitives<>::pow(y, tmp.data<T>(), static_cast<T>(2), depth);
        auto variance = primitives<>::mean(tmp.data<T>(), depth);
        primitives<>::mul(static_cast<T>(1.0 / sqrt(variance + EPSILON)), y, depth);
        primitives<>::mul(gamma.data<T>(), y, depth);
        primitives<>::add(beta.data<T>(), y, depth);
      }
    }

#define DECLARE_IMPL(T)                                                 \
    template void                                                       \
    LayerNorm::compute<Device::CPU, T>(const StorageView& beta,         \
                                       const StorageView& gamma,        \
                                       const StorageView& input,        \
                                       StorageView& output) const;

    DECLARE_IMPL(float)

  }
}
