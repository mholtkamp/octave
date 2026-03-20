/**
 * @file TinyLLMInference.h
 * @brief Pure C inference kernels for ultra-tiny LLM (llama2.c style).
 */

#pragma once

#include "Assets/TinyLLMAsset.h"  // For TinyLLMConfig
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RMS normalization.
 */
void tinyllm_rmsnorm(float* o, float* x, float* weight, int size);

/**
 * @brief In-place softmax.
 */
void tinyllm_softmax(float* x, int size);

/**
 * @brief Matrix-vector multiplication: W (d,n) @ x (n,) -> xout (d,)
 */
void tinyllm_matmul(float* xout, float* x, float* w, int n, int d);

/**
 * @brief Single-token forward pass through the transformer.
 */
float* tinyllm_forward(
    float* x, float* xb, float* xb2, float* hb, float* hb2,
    float* q, float* key_cache, float* value_cache, float* att, float* logits,
    float* token_emb, float* rms_att, float* wq, float* wk, float* wv, float* wo,
    float* rms_ffn, float* w1, float* w2, float* w3, float* rms_final, float* wcls,
    const TinyLLMConfig* config,
    int token, int pos
);

/**
 * @brief Calculate the size needed for RunState buffers.
 */
size_t tinyllm_calc_runstate_size(const TinyLLMConfig* config, int max_seq_len);

/**
 * @brief Calculate the size of model weights.
 */
size_t tinyllm_calc_weights_size(const TinyLLMConfig* config);

#ifdef __cplusplus
}
#endif
