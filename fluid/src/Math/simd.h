#include <immintrin.h>

#define LANE_WIDTH 8

// AVX512
#if (LANE_WIDTH==16)


//======
// AVX2
//======
#elif (LANE_WIDTH==8)

// https://www.youtube.com/watch?v=qejTqnxQRcw&t=7s
// https://github.com/CppCon/CppCon2020/blob/main/Presentations/adventures_in_simd_thinking_part_1/adventures_in_simd_thinking_part_1__bob_steagall__cppcon_2020.pdf
// typedef __m256 wfloat;
// typedef __m256i wint;
// wfloat load_value(float);
// wfloat load_values(float a, float b, float c, float d, float e, float f, float g, float h);
// wfloat load_from(float*);
// wfloat masked_load_from(float*, float fill, wint mask);
// wfloat masked_load_from(float*, wfloat fill, wint mask);
// wfloat store_to(float*, wfloat v);
// wfloat masked_store_to(float*, wfloat v, wint mask);
// wint make_bit_mask();
// wfloat blend(wfloat a, wfloat b, wint mask);
// wfloat permute(wfloat a, wint perm);
// // wfloat masked_permute(wfloat a, wfloat b, wint perm, wint mask);
// wfloat make_perm_map();             // template<A...H>
// wfloat rotate(wfloat a);        // template <int R>
// wfloat rotate_down(wfloat a);
// wfloat rotate_up(wfloat a);
// wfloat shift(wfloat a);
// wfloat shift_up(wfloat a);
// wfloat shift_down(wfloat a);
// wfloat shift_with_carry(wfloat a, wfloat b);
// wfloat shift_up_with_carry(wfloat a, wfloat b);
// wfloat shift_down_with_carry(wfloat a,  wfloat b);
// void inplace_shift_with_carry(wfloat& a, wfloat& b);
// wfloat fmadd(wfloat a, wfloat b, wfloat c);
// wfloat min(wfloat a, wfloat b);
// wfloat max(wfloat a, wfloat b);

struct wide_float {
    __m256 V;
    wide_float(){};
    wide_float(float);
    wide_float(float*);
    wide_float& operator=(float);
};
struct wide_int {
    __m256i V;
    wide_int(){};
    wide_int(int);
    wide_int(int*);
    wide_int& operator=(int);
};


//-----------
// MAKE WIDE
inline wide_float
WideFloatFromFloat(float A){
    wide_float ret;
    ret.V = _mm256_set1_ps(A);
    return ret;
}
inline wide_int
WideIntFromInt(int A){
    wide_int ret;
    ret.V = _mm256_set1_epi32(A);
    return ret;
}
inline wide_float
WideFloatFromWideInt(wide_int A) {
    wide_float ret;
    ret.V = _mm256_cvtepi32_ps(A.V);
    return ret;
}
inline wide_float
WideFloatFromInt(int A) {
    wide_float ret = WideFloatFromFloat((float) A);
    return ret;
}

//--------------
// CONSTRUCTORS
wide_int::wide_int(int A) {
    this->V = WideIntFromInt(A).V;
}
wide_int::wide_int(int* A) {
    this->V = _mm256_loadu_si256((__m256i*)A);
}
wide_int& wide_int::operator=(int A) {
    this->V = WideIntFromInt(A).V;
    return *this;
}
wide_float::wide_float(float A) {
    this->V = WideFloatFromFloat(A).V;
}
wide_float::wide_float(float* A) {
    this->V = _mm256_loadu_ps(A);
}
wide_float& wide_float::operator=(float A) {
    this->V = WideFloatFromFloat(A).V;
    return *this;
}

//--------------
// LOAD - STORE
inline wide_int
LoadPackedWideInt(int* A) {
    wide_int ret;
    ret.V = _mm256_loadu_si256((__m256i*)A);
    return ret;
}
inline wide_float
LoadPackedWideFloat(float* A) {
    wide_float ret;
    ret.V = _mm256_loadu_ps(A);
    return ret;
}
inline wide_int
LoadMaskedPackedWideInt(int* A, wide_int mask) {
    wide_int ret;
    ret.V = _mm256_maskload_epi32(A, mask.V);
    return ret;
}
inline wide_float
LoadMaskedPackedWideFloat(float* A, wide_int mask) {
    wide_float ret;
    ret.V = _mm256_maskload_ps(A, mask.V);
    return ret;
}
inline void
StoreWideInt(int* dest, wide_int source) {
    _mm256_storeu_si256((__m256i*)dest, source.V);
}
inline void
StoreWideFloat(float* dest, wide_float source) {
    _mm256_storeu_ps(dest, source.V);
}
inline void
StoreMaskedWideFloat(float* dest, wide_int mask, wide_float source){
    // if mask -> then save
    _mm256_maskstore_ps(dest, mask.V, source.V);
}
inline void
StoreMaskedWideFloat(int* dest, wide_int mask, wide_int source){
    // if mask -> then save
    _mm256_maskstore_epi32(dest, mask.V, source.V);
}

//----------
// WIDE_INT
inline wide_int
operator==(wide_int A, wide_int B) {
    wide_int ret;
    ret.V = _mm256_cmpeq_epi32(A.V, B.V);
    return ret;
}
inline wide_int
operator==(int A, wide_int B) {
    wide_int ret = (WideIntFromInt(A) == B);
    return ret;
}
inline wide_int
operator==(wide_int A, int B) {
    wide_int ret = (A == WideIntFromInt(B));
    return ret;
}

inline wide_int
operator!=(wide_int A, wide_int B) {
    wide_int ret;
    ret.V = _mm256_andnot_si256((A==B).V, _mm256_set1_epi32(0xFFFFFFFF));
    return ret;
}
inline wide_int
operator!=(int A, wide_int B) {
    wide_int ret = (WideIntFromInt(A)!=B);
    return ret;
}
inline wide_int
operator!=(wide_int A, int B) {
    wide_int ret = (A!=WideIntFromInt(B));
    return ret;
}

inline wide_int
operator|(wide_int A, wide_int B) {
    wide_int ret;
    ret.V = _mm256_or_si256(A.V, B.V);
    return ret;
}
inline wide_int
operator|(int A, wide_int B) {
    wide_int ret = WideIntFromInt(A) | B;
    return ret;
}
inline wide_int
operator|(wide_int A, int B) {
    wide_int ret = A | WideIntFromInt(B);
    return ret;
}
inline wide_int&
operator|=(wide_int& A, wide_int B) {
    A = A | B;
    return A;
}
inline wide_int&
operator|=(wide_int& A, int B) {
    A |= WideIntFromInt(B);
    return A;
}

inline wide_int
operator&(wide_int A, wide_int B) {
    wide_int ret;
    ret.V = _mm256_and_si256(A.V, B.V);
    return ret;
}
inline wide_int
operator&(int A, wide_int B) {
    wide_int ret = WideIntFromInt(A) & B;
    return ret;
}
inline wide_int
operator&(wide_int A, int B) {
    wide_int ret = A & WideIntFromInt(B);
    return ret;
}

inline wide_int
operator>(wide_int A, wide_int B) {
    wide_int ret;
    ret.V = _mm256_cmpgt_epi32(A.V, B.V);
    return ret;
}
inline wide_int
operator>(int A, wide_int B) {
    wide_int ret = WideIntFromInt(A)>B;
    return ret;
}
inline wide_int
operator>(wide_int A, int B) {
    wide_int ret = A>WideIntFromInt(B);
    return ret;
}

inline wide_int
operator>=(wide_int A, wide_int B) {
    wide_int ret = (A > B) | (A == B);
    return ret;
}
inline wide_int
operator>=(int A, wide_int B) {
    wide_int ret = (WideIntFromInt(A)>=B);
    return ret;
}
inline wide_int
operator>=(wide_int A, int B) {
    wide_int ret = (A>=WideIntFromInt(B));
    return ret;
}

inline wide_int
operator<(wide_int A, wide_int B) {
    return (B > A);
}
inline wide_int
operator<(int A, wide_int B) {
    return (B > WideIntFromInt(A));
}
inline wide_int
operator<(wide_int A, int B) {
    return (WideIntFromInt(B) > A);
}

inline wide_int
operator<=(wide_int A, wide_int B) {
    return (B >= A);
}
inline wide_int
operator<=(int A, wide_int B) {
    return (B >= WideIntFromInt(A));
}
inline wide_int
operator<=(wide_int A, int B) {
    return (WideIntFromInt(B) >= A);
}

inline wide_int
operator+(wide_int A, wide_int B) {
    wide_int ret;
    ret.V = _mm256_add_epi32(A.V, B.V);
    return ret;
}
inline wide_int
operator+(int A, wide_int B) {
    wide_int ret = WideIntFromInt(A) + B;
    return ret;
}
inline wide_int
operator+(wide_int A, int B) {
    wide_int ret = A + WideIntFromInt(B);
    return ret;
}

inline wide_int&
operator+=(wide_int& A, wide_int B) {
    A = A + B;
    return A;
}
inline wide_int&
operator+=(wide_int& A, int B) {
    A += WideIntFromInt(B);
    return A;
}

inline wide_int
operator-(wide_int A) {
    wide_int ret;
    ret.V = 0 - A.V;
    return ret;
}
inline wide_int
operator-(wide_int A, wide_int B) {
    wide_int ret;
    ret.V = _mm256_sub_epi32(A.V, B.V);
    return ret;
}
inline wide_int
operator-(int A, wide_int B) {
    wide_int ret = WideIntFromInt(A)-B;
    return ret;
}
inline wide_int
operator-(wide_int A, int B) {
    wide_int ret = A-WideIntFromInt(B);
    return ret;
}

inline wide_int&
operator-=(wide_int& A, wide_int B) {
    A = A - B;
    return A;
}
inline wide_int&
operator-=(wide_int& A, int B) {
    A -= WideIntFromInt(B);
    return A;
}

// NOTE(alex): This is darn slow, maybe we should never even do this on int
inline wide_int
operator*(wide_int A, wide_int B) {
    wide_int ret;
    ret.V = _mm256_mullo_epi32(A.V, B.V);
    return ret;
}
inline wide_int
operator*(int A, wide_int B) {
    wide_int ret = WideIntFromInt(A)*B;
    return ret;
}
inline wide_int
operator*(wide_int A, int B) {
    wide_int ret = A*WideIntFromInt(B);
    return ret;
}

inline wide_int&
operator*=(wide_int& A, wide_int B) {
    A = A * B;
    return A;
}
inline wide_int&
operator*=(wide_int& A, int B) {
    A *= WideIntFromInt(B);
    return A;
}


//------------
// WIDE_FLOAT
inline wide_int
operator>(wide_float A, wide_float B) {
    wide_int ret;
    ret.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_GT_OQ));
    return ret;
}
inline wide_int
operator>(wide_float A, float B) {
    return (A > WideFloatFromFloat(B));
}
inline wide_int
operator>(float A, wide_float B) {
    return (WideFloatFromFloat(A) > B);
}

inline wide_int
operator>=(wide_float A, wide_float B) {
    wide_int ret;
    ret.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_GE_OQ));
    return ret;
}
inline wide_int
operator>=(wide_float A, float B) {
    return (A >= WideFloatFromFloat(B));
}
inline wide_int
operator>=(float A, wide_float B) {
    return (WideFloatFromFloat(A) >= B);
}

inline wide_int
operator<(wide_float A, wide_float B) {
    wide_int ret;
    ret.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_LT_OQ));
    return ret;
}
inline wide_int
operator<(wide_float A, float B) {
    return (A < WideFloatFromFloat(B));
}
inline wide_int
operator<(float A, wide_float B) {
    return (WideFloatFromFloat(A) < B);
}

inline wide_int
operator<=(wide_float A, wide_float B) {
    wide_int ret;
    ret.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_LE_OQ));
    return ret;
}
inline wide_int
operator<=(wide_float A, float B) {
    return (A <= WideFloatFromFloat(B));
}
inline wide_int
operator<=(float A, wide_float B) {
    return (WideFloatFromFloat(A) <= B);
}

inline wide_int
operator==(wide_float A, wide_float B) {
    wide_int ret;
    ret.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_EQ_OQ));
    return ret;
}
inline wide_int
operator==(wide_float A, float B) {
    return (A == WideFloatFromFloat(B));
}
inline wide_int
operator==(float A, wide_float B) {
    return (WideFloatFromFloat(A) == B);
}

inline wide_int
operator!=(wide_float A, wide_float B) {
    wide_int ret;
    ret.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_NEQ_OQ));
    return ret;
}
inline wide_int
operator!=(wide_float A, float B) {
    return (A != WideFloatFromFloat(B));
}
inline wide_int
operator!=(float A, wide_float B) {
    return (WideFloatFromFloat(A) != B);
}

inline wide_float
operator+(wide_float A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_add_ps(A.V, B.V);
    return ret;
}
inline wide_float
operator+(wide_float A, float B) {
    return A + WideFloatFromFloat(B);
}
inline wide_float
operator+(float A, wide_float B) {
    return WideFloatFromFloat(A) + B;
}

inline wide_float&
operator+=(wide_float& A, wide_float B) {
    A.V = _mm256_add_ps(A.V, B.V);
    return A;
}
inline wide_float&
operator+=(wide_float& A, float B) {
    A += WideFloatFromFloat(B);
    return A;
}

inline wide_float
operator-(wide_float A) {
    wide_float ret;
    ret.V = 0 - A.V;
    return ret;
}

inline wide_float
operator-(wide_float A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_sub_ps(A.V, B.V);
    return ret;
}
inline wide_float
operator-(float A, wide_float B) {
    return WideFloatFromFloat(A) - B;
}
inline wide_float
operator-(wide_float A, float B) {
    return A - WideFloatFromFloat(B);
}

inline wide_float&
operator-=(wide_float& A, wide_float B) {
    A.V = _mm256_sub_ps(A.V, B.V);
    return A;
}
inline wide_float&
operator-=(wide_float& A, float B) {
    A -= WideFloatFromFloat(B);
    return A;
}

inline wide_float
operator*(wide_float A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_mul_ps(A.V, B.V);
    return ret;
}
inline wide_float
operator*(float A, wide_float B) {
    wide_float ret = WideFloatFromFloat(A) * B;
    return ret;
}
inline wide_float
operator*(wide_float A, float B) {
    wide_float ret = A * WideFloatFromFloat(B);
    return ret;
}

inline wide_float&
operator*=(wide_float& A, wide_float B) {
    A.V = _mm256_mul_ps(A.V, B.V);
    return A;
}
inline wide_float&
operator*=(wide_float& A, float B) {
    A *= WideFloatFromFloat(B);
    return A;
}

inline wide_float
operator/(wide_float A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_div_ps(A.V, B.V);
    return ret;
}
inline wide_float
operator/(float A, wide_float B) {
    wide_float ret = WideFloatFromFloat(A) / B;
    return ret;
}
inline wide_float
operator/(wide_float A, float B) {
    wide_float ret = A / WideFloatFromFloat(B);
    return ret;
}

inline wide_float&
operator/=(wide_float& A, wide_float B) {
    A.V = _mm256_div_ps(A.V, B.V);
    return A;
}
inline wide_float&
operator/=(wide_float& A, float B) {
    A /= WideFloatFromFloat(B);
    return A;
}

inline wide_float
operator|(wide_float A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_or_ps(A.V, B.V);
    return ret;
}
inline wide_float
operator|(float A, wide_float B) {
    wide_float ret = WideFloatFromFloat(A) | B;
    return ret;
}
inline wide_float
operator|(wide_float A, float B) {
    wide_float ret = A | WideFloatFromFloat(B);
    return ret;
}
inline wide_float&
operator|=(wide_float& A, wide_float B) {
    A = A | B;
    return A;
}
inline wide_float&
operator|=(wide_float& A, float B) {
    A |= WideFloatFromFloat(B);
    return A;
}

inline wide_float
operator&(wide_float A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_and_ps(A.V, B.V);
    return ret;
}
inline wide_float
operator&(float A, wide_float B) {
    wide_float ret = WideFloatFromFloat(A) & B;
    return ret;
}
inline wide_float
operator&(wide_float A, float B) {
    wide_float ret = A & WideFloatFromFloat(B);
    return ret;
}
inline wide_int
operator^(wide_int A, wide_int B){
    wide_int ret;
    ret.V = _mm256_xor_si256(A.V, B.V);
    return ret;
}
inline wide_float
operator^(wide_float A, wide_float B){
    wide_float ret;
    ret.V = _mm256_xor_ps(A.V, B.V);
    return ret;
}
inline wide_int
operator~(wide_int A){
    return A^wide_int(0xFFFFFFFF);
};

// MIXED
inline wide_float
operator-(wide_float A, wide_int B) {
    wide_float ret;
    ret.V = A.V - _mm256_cvtepi32_ps(B.V);
    return ret;
}
inline wide_float
operator-(wide_int A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_cvtepi32_ps(A.V) - B.V;
    return ret;
}

inline wide_float
operator+(wide_float A, wide_int B) {
    wide_float ret;
    ret.V = A.V + _mm256_cvtepi32_ps(B.V);
    return ret;
}
inline wide_float
operator+(wide_int A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_cvtepi32_ps(A.V) + B.V;
    return ret;
}

inline wide_float
operator*(wide_float A, wide_int B) {
    wide_float ret;
    ret.V = A.V * _mm256_cvtepi32_ps(B.V);
    return ret;
}
inline wide_float
operator*(wide_int A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_cvtepi32_ps(A.V) * B.V;
    return ret;
}

inline wide_float
operator/(wide_float A, wide_int B) {
    wide_float ret;
    ret.V = A.V / _mm256_cvtepi32_ps(B.V);
    return ret;
}
inline wide_float
operator/(wide_int A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_cvtepi32_ps(A.V) / B.V;
    return ret;
}

// ---------
// Functions
// ---------
inline wide_int
Min(wide_int A, wide_int B) {
    wide_int ret;
    ret.V = _mm256_min_epi32(A.V, B.V);
    return ret;
}
inline wide_float
Min(wide_float A, wide_float B) {
    wide_float ret;
    A.V = _mm256_min_ps(A.V, B.V);
    return ret;
}

inline wide_int
Max(wide_int A, wide_int B) {
    wide_int ret;
    ret.V = _mm256_max_epi32(A.V, B.V);
    return ret;
}
inline wide_float
Max(wide_float A, wide_float B) {
    wide_float ret;
    ret.V = _mm256_max_ps(A.V, B.V);
    return ret;
}

inline wide_float
Abs(wide_float A) {
    wide_float ret;
    ret.V = _mm256_andnot_ps(WideFloatFromFloat(-0.0f).V, A.V);
    return ret;
}
inline wide_int
Abs(wide_int A) {
    wide_int ret;
    ret.V = _mm256_abs_epi32(A.V);
    return ret;
}

inline wide_int
Square(wide_int A){
    wide_int ret = A*A;
    return ret;
}
inline wide_float
Square(wide_float A){
    wide_float ret = A*A;
    return ret;
}

inline wide_int
WideIndex(int A) {
    wide_int ret;
    ret.V = _mm256_add_epi32(_mm256_set1_epi32(A), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
    return ret;
};

inline void
ConditionalAssign(wide_float* A, wide_int mask, wide_float B){
    A->V = _mm256_blendv_ps(A->V, B.V, _mm256_castsi256_ps(mask.V));
}

inline void
ConditionalAssign(wide_int* A, wide_int mask, wide_int B){
    A->V = _mm256_blendv_epi8(A->V, B.V, mask.V);
}

inline float
HorizontalMax (wide_float A) {
    float ret = *(float*)&A.V;
    for (int i=1; i<8; i++) {
        float v = *((float*)&A.V+i);
        ret = ret>v ? ret : v;
    }
    return ret;
}
inline float
HorizontalMin (wide_float A) {
    float ret = *(float*)&A.V;
    for (int i=1; i<8; i++) {
        float v = *((float*)&A.V+i);
        ret = ret<v ? ret : v;
    }
    return ret;
}
inline int
HorizontalMax (wide_int A) {
    int ret = *(int*)&A.V;
    for (int i=1; i<8; i++) {
        int v = *((int*)&A.V+i);
        ret = ret>v ? ret : v;
    }
    return ret;
}
inline int
HorizontalMin (wide_int A) {
    int ret = *(int*)&A.V;
    for (int i=1; i<8; i++) {
        int v = *((int*)&A.V+i);
        ret = ret<v ? ret : v;
    }
    return ret;
}

template <int R>
inline wide_int
Rotate(wide_int IN) {
    if constexpr ((R % 8) == 0)
        return IN;

    constexpr int S = (R > 0) ? (8 - (R % 8)) : -R;
    constexpr int A = (S + 0) % 8;
    constexpr int B = (S + 1) % 8;
    constexpr int C = (S + 2) % 8;
    constexpr int D = (S + 3) % 8;
    constexpr int E = (S + 4) % 8;
    constexpr int F = (S + 5) % 8;
    constexpr int G = (S + 6) % 8;
    constexpr int H = (S + 7) % 8;

    wide_int ret;
    ret.V = _mm256_permutevar8x32_epi32(IN.V, _mm256_setr_epi32(A,B,C,D,E,F,G,H));
    return ret;
}
template <int R>
inline wide_int
RotateRight(wide_int IN) {
    static_assert(R>=0);
    return Rotate<R>(IN);
}
template <int R>
inline wide_int
RotateLeft(wide_int IN) {
    static_assert(R>=0);
    return Rotate<-R>(IN);
}

template <int R>
inline wide_int
MakeShiftMask(){
    wide_int ret;
    const __m256i idx = _mm256_setr_epi32(0,1,2,3,4,5,6,7);
    if (R>=0) {
        ret.V = _mm256_cmpgt_epi32(_mm256_set1_epi32(R), idx);          // checks for R > idx
    } else {
        ret.V = _mm256_cmpgt_epi32(idx, _mm256_set1_epi32(R+7));
    }
    return ret;
}
template <int R>
inline wide_int
MakeRightShiftMask(){
    static_assert(R>=0);
    return MakeShiftMask<R>();
}
template <int R>
inline wide_int
MakeLeftShiftMask(){
    static_assert(R>=0);
    return MakeShiftMask<-R>();
}

template <int R>
inline wide_int
Shift(wide_int IN) {
    // rotates by R and set 0 instead of wrapping around
    wide_int rotated = Rotate<R>(IN);
    ConditionalAssign(&rotated, MakeShiftMask<R>(), 0);
    return rotated;
}
template <int R>
inline wide_int
LeftShift(wide_int IN) {
    static_assert(R>=0);
    return Shift<-R>(IN);
}
template <int R>
inline wide_int
RightShift(wide_int IN) {
    static_assert(R>=0);
    return Shift<R>(IN);
}

template <int R>
inline wide_int
ShiftWithCarry(wide_int A, wide_int B) {
    wide_int ret = Rotate<R>(A);
    ConditionalAssign(&ret, MakeShiftMask<R>(), Rotate<R>(B));
    return ret;
}
template <int R>
inline wide_int
ShiftLeftWithCarry(wide_int A, wide_int B) {            // Returns A with B shifted in it
    static_assert(R>=0);
    return ShiftWithCarry<-R>(A, B);
}
template <int R>
inline wide_int
ShiftRightWithCarry(wide_int A, wide_int B) {
    static_assert(R>=0);
    return ShiftWithCarry<R>(B, A);
}

// SSE
#elif (LANE_WIDTH==4)
// Single Scalar
#elif (LANE_WIDTH==1)
#else
#endif

#if (LANE_WIDTH!=1)
#endif
