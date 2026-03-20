/**
 * @file TinyLLMInference.cpp
 * @brief Pure C inference kernels ported from dosllam2.c / llama2.c.
 */

#include "TinyLLMInference.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

void tinyllm_rmsnorm(float* o, float* x, float* weight, int size)
{
    float ss = 0.0f;
    for (int j = 0; j < size; j++) {
        ss += x[j] * x[j];
    }
    ss /= size;
    ss += 1e-5f;
    ss = 1.0f / sqrtf(ss);

    for (int j = 0; j < size; j++) {
        o[j] = weight[j] * (ss * x[j]);
    }
}

void tinyllm_softmax(float* x, int size)
{
    float max_val = x[0];
    for (int i = 1; i < size; i++) {
        if (x[i] > max_val) {
            max_val = x[i];
        }
    }

    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        x[i] = expf(x[i] - max_val);
        sum += x[i];
    }

    for (int i = 0; i < size; i++) {
        x[i] /= sum;
    }
}

void tinyllm_matmul(float* xout, float* x, float* w, int n, int d)
{
    for (int i = 0; i < d; i++) {
        float val = 0.0f;
        for (int j = 0; j < n; j++) {
            val += w[i * n + j] * x[j];
        }
        xout[i] = val;
    }
}

float* tinyllm_forward(
    float* x, float* xb, float* xb2, float* hb, float* hb2,
    float* q, float* key_cache, float* value_cache, float* att, float* logits,
    float* token_emb, float* rms_att, float* wq, float* wk, float* wv, float* wo,
    float* rms_ffn, float* w1, float* w2, float* w3, float* rms_final, float* wcls,
    const TinyLLMConfig* config,
    int token, int pos)
{
    int dim = config->dim;
    int kv_dim = (config->dim * config->n_kv_heads) / config->n_heads;
    int kv_mul = config->n_heads / config->n_kv_heads;
    int hidden_dim = config->hidden_dim;
    int head_size = dim / config->n_heads;
    int seq_len = config->seq_len;

    // Copy the token embedding into x
    float* content_row = token_emb + token * dim;
    memcpy(x, content_row, dim * sizeof(float));

    // Forward all layers
    for (int l = 0; l < config->n_layers; l++) {
        // Attention rmsnorm
        tinyllm_rmsnorm(xb, x, rms_att + l * dim, dim);

        // Key and value point to the kv cache
        int loff = l * seq_len * kv_dim;
        float* k = key_cache + loff + pos * kv_dim;
        float* v = value_cache + loff + pos * kv_dim;

        // QKV matmuls for this position
        tinyllm_matmul(q, xb, wq + l * dim * dim, dim, dim);
        tinyllm_matmul(k, xb, wk + l * dim * kv_dim, dim, kv_dim);
        tinyllm_matmul(v, xb, wv + l * dim * kv_dim, dim, kv_dim);

        // RoPE relative positional encoding
        for (int i = 0; i < dim; i += 2) {
            int head_dim = i % head_size;
            float freq = 1.0f / powf(10000.0f, head_dim / (float)head_size);
            float val = pos * freq;
            float fcr = cosf(val);
            float fci = sinf(val);
            int rotn = i < kv_dim ? 2 : 1;
            for (int vn = 0; vn < rotn; vn++) {
                float* vec = vn == 0 ? q : k;
                float v0 = vec[i];
                float v1 = vec[i + 1];
                vec[i] = v0 * fcr - v1 * fci;
                vec[i + 1] = v0 * fci + v1 * fcr;
            }
        }

        // Multihead attention
        for (int h = 0; h < config->n_heads; h++) {
            float* qh = q + h * head_size;
            float* atth = att + h * seq_len;

            // Compute attention scores
            for (int t = 0; t <= pos; t++) {
                float* kh = key_cache + loff + t * kv_dim + (h / kv_mul) * head_size;
                float score = 0.0f;
                for (int i = 0; i < head_size; i++) {
                    score += qh[i] * kh[i];
                }
                score /= sqrtf((float)head_size);
                atth[t] = score;
            }

            // Softmax the scores
            tinyllm_softmax(atth, pos + 1);

            // Weighted sum of values
            float* xbh = xb + h * head_size;
            memset(xbh, 0, head_size * sizeof(float));
            for (int t = 0; t <= pos; t++) {
                float* vh = value_cache + loff + t * kv_dim + (h / kv_mul) * head_size;
                float a = atth[t];
                for (int i = 0; i < head_size; i++) {
                    xbh[i] += a * vh[i];
                }
            }
        }

        // Final matmul to get the output of attention
        tinyllm_matmul(xb2, xb, wo + l * dim * dim, dim, dim);

        // Residual connection
        for (int i = 0; i < dim; i++) {
            x[i] += xb2[i];
        }

        // FFN rmsnorm
        tinyllm_rmsnorm(xb, x, rms_ffn + l * dim, dim);

        // FFN: self.w2(F.silu(self.w1(x)) * self.w3(x))
        tinyllm_matmul(hb, xb, w1 + l * dim * hidden_dim, dim, hidden_dim);
        tinyllm_matmul(hb2, xb, w3 + l * dim * hidden_dim, dim, hidden_dim);

        // SwiGLU non-linearity
        for (int i = 0; i < hidden_dim; i++) {
            float val = hb[i];
            val *= (1.0f / (1.0f + expf(-val)));
            val *= hb2[i];
            hb[i] = val;
        }

        // Final matmul
        tinyllm_matmul(xb, hb, w2 + l * hidden_dim * dim, hidden_dim, dim);

        // Residual connection
        for (int i = 0; i < dim; i++) {
            x[i] += xb[i];
        }
    }

    // Final rmsnorm
    tinyllm_rmsnorm(x, x, rms_final, dim);

    // Classifier into logits
    tinyllm_matmul(logits, x, wcls, dim, config->vocab_size);

    return logits;
}

size_t tinyllm_calc_runstate_size(const TinyLLMConfig* config, int max_seq_len)
{
    int dim = config->dim;
    int hidden_dim = config->hidden_dim;
    int n_layers = config->n_layers;
    int n_heads = config->n_heads;
    int kv_dim = (dim * config->n_kv_heads) / n_heads;
    int vocab_size = config->vocab_size;

    size_t size = 0;
    size += dim * sizeof(float);                           // x
    size += dim * sizeof(float);                           // xb
    size += dim * sizeof(float);                           // xb2
    size += hidden_dim * sizeof(float);                    // hb
    size += hidden_dim * sizeof(float);                    // hb2
    size += dim * sizeof(float);                           // q
    size += n_layers * max_seq_len * kv_dim * sizeof(float); // key_cache
    size += n_layers * max_seq_len * kv_dim * sizeof(float); // value_cache
    size += n_heads * max_seq_len * sizeof(float);         // att
    size += vocab_size * sizeof(float);                    // logits

    return size;
}

size_t tinyllm_calc_weights_size(const TinyLLMConfig* config)
{
    int dim = config->dim;
    int hidden_dim = config->hidden_dim;
    int n_layers = config->n_layers;
    int n_heads = config->n_heads;
    int n_kv_heads = config->n_kv_heads;
    int vocab_size = config->vocab_size;
    int head_size = dim / n_heads;
    int kv_dim = n_kv_heads * head_size;

    size_t size = 0;
    size += vocab_size * dim;
    size += n_layers * dim;
    size += n_layers * dim * (n_heads * head_size);
    size += n_layers * dim * (n_kv_heads * head_size);
    size += n_layers * dim * (n_kv_heads * head_size);
    size += n_layers * (n_heads * head_size) * dim;
    size += n_layers * dim;
    size += n_layers * dim * hidden_dim;
    size += n_layers * hidden_dim * dim;
    size += n_layers * dim * hidden_dim;
    size += dim;
    size += dim * vocab_size;

    return size * sizeof(float);
}
