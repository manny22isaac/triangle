#include "userosc.h"
#include "float_math.h"
#include "osc_api.h"

typedef struct State {

float w0;
float phase;
float lfo;
float lfoz;
float idx;
float shiftshape;

} State;

static State s_state;

void OSC_INIT(uint32_t platform, uint32_t api) {

s_state.w0    = 0.f; 
s_state.phase = 0.f;  
s_state.lfo = 0.f;
s_state.lfoz = 0.f;
s_state.idx = 0.f;
s_state.shiftshape = 0.f;
}

#define k_wt_sqr_size_exp      7
#define k_wt_sqr_size          (1U<<k_wt_sqr_size_exp)
#define k_wt_sqr_u32shift      (24)
#define k_wt_sqr_frrecip       (5.96046447753906e-008f) // 1/(1<<24)    
#define k_wt_sqr_mask          (k_wt_sqr_size-1)
#define k_wt_sqr_lut_size      (k_wt_sqr_size+1)
#define k_wt_sqr_notes_cnt     7
#define k_wt_sqr_lut_tsize     (k_wt_sqr_notes_cnt * k_wt_sqr_lut_size)

  extern const uint8_t wt_sqr_notes[k_wt_sqr_notes_cnt];
  extern const float wt_sqr_lut_f[k_wt_sqr_lut_tsize];


//this is a band-limited sawtooth wave from the api documentation.
__fast_inline float triangle(float x, float idx){
const float p = x - (uint32_t)x;

const float x0f = 2.f * p * k_wt_sqr_size;
const uint32_t x0p = (uint32_t)x0f;

uint32_t x0 = x0p, x1 = x0p+1;
float sign = 1.f;
if (x0p >= k_wt_sqr_size) {
  x0 = k_wt_sqr_size - (x0p & k_wt_sqr_mask);
  x1 = x0 - 1;
  sign = -1.f;
}
const float *wt = &wt_saw_lut_f[(uint16_t)s_state.idx*k_wt_saw_lut_size];
const float y0 = linintf(x0f - x0p, wt[x0], wt[x1]);
  return sign*y0;
}

void OSC_CYCLE(const user_osc_param_t *const params, int32_t *yn, const uint32_t frames){
  const float w0 = s_state.w0 = osc_w0f_for_note((params->pitch)>>8, params->pitch & 0xFF);

  s_state.lfo = q31_to_f32(params->shape_lfo);
  float lfoz = s_state.lfoz;
  const float lfo_inc = (s_state.lfo - lfoz) / frames;
  float phase = s_state.phase;

  q31_t * __restrict y = (q31_t *)yn;
  const q31_t * y_e = y + frames;

  for (; y != y_e; ) {
    // calculate current signal value

    float p = osc_softclipf(0.05f, triangle(phase, s_state.idx + lfoz));

    p = (p <= 0) ? 1.f - p : p - (uint32_t)p;

    const float sig  = osc_softclipf(0.05f, s_state.shiftshape * osc_sinf(p));
    // convert Floating-Point[-1, 1] to Fixed-Point[-0x7FFFFFFF, 0x7FFFFFFF] and update signal
    *(y++) = f32_to_q31(sig);
    // inclement wave phase
    phase += w0;
    // reset wave phase if it overlap
    phase -= (uint32_t)phase;
    lfoz += lfo_inc;
  }
  s_state.phase = phase;
  s_state.lfoz = lfoz;
  }


void OSC_NOTEON(const user_osc_param_t *const params) {
  
  // Note on triggers here (Not essential)
  (void)params;
  
}

void OSC_NOTEOFF(const user_osc_param_t *const params) {
  (void)params;
  
  // Note off triggers here (Not essential)
  
}

void OSC_PARAM(uint16_t index, uint16_t value) { 

  const float valf = param_val_to_f32(value);
  // Update parameters and dependent variables here
  
  switch (index) {
    case k_user_osc_param_id1: 
      break;
      
    case k_user_osc_param_id2: 
      break;
      
    case k_user_osc_param_id3: 
      break;
      
    case k_user_osc_param_id4: 
      break;
      
    case k_user_osc_param_id5: 
      break;

    case k_user_osc_param_id6: 
      break;
      
    case k_user_osc_param_shape:    // A knob
      s_state.idx = 0.2f * valf;
      break;

    case k_user_osc_param_shiftshape:   // B knob
      s_state.shiftshape = 1.f + valf;
      break;

    default:
      break; 
  }
}
