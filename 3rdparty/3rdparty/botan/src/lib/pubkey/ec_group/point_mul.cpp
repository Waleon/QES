/*
* (C) 2015,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/internal/point_mul.h>
#include <botan/rng.h>
#include <botan/reducer.h>
#include <botan/internal/rounding.h>
#include <botan/internal/ct_utils.h>

namespace Botan {

PointGFp multi_exponentiate(const PointGFp& x, const BigInt& z1,
                            const PointGFp& y, const BigInt& z2)
   {
   PointGFp_Multi_Point_Precompute xy_mul(x, y);
   return xy_mul.multi_exp(z1, z2);
   }

Blinded_Point_Multiply::Blinded_Point_Multiply(const PointGFp& base,
                                               const BigInt& order,
                                               size_t h) :
   m_ws(PointGFp::WORKSPACE_SIZE),
   m_order(order)
   {
   BOTAN_UNUSED(h);
   Null_RNG null_rng;
   m_point_mul.reset(new PointGFp_Var_Point_Precompute(base, null_rng, m_ws));
   }

Blinded_Point_Multiply::~Blinded_Point_Multiply()
   {
   /* for ~unique_ptr */
   }

PointGFp Blinded_Point_Multiply::blinded_multiply(const BigInt& scalar,
                                                  RandomNumberGenerator& rng)
   {
   return m_point_mul->mul(scalar, rng, m_order, m_ws);
   }

PointGFp_Base_Point_Precompute::PointGFp_Base_Point_Precompute(const PointGFp& base,
                                                               const Modular_Reducer& mod_order) :
   m_base_point(base),
   m_mod_order(mod_order),
   m_p_words(base.get_curve().get_p().sig_words()),
   m_T_size(base.get_curve().get_p().bits() + PointGFp_SCALAR_BLINDING_BITS + 1)
   {
   std::vector<BigInt> ws(PointGFp::WORKSPACE_SIZE);

   const size_t p_bits = base.get_curve().get_p().bits();

   /*
   * Some of the curves (eg secp160k1) have an order slightly larger than
   * the size of the prime modulus. In all cases they are at most 1 bit
   * longer. The +1 compensates for this.
   */
   const size_t T_bits = round_up(p_bits + PointGFp_SCALAR_BLINDING_BITS + 1, 2) / 2;

   std::vector<PointGFp> T(3*T_bits);
   T.resize(3*T_bits);

   T[0] = base;
   T[1] = T[0];
   T[1].mult2(ws);
   T[2] = T[1];
   T[2].add(T[0], ws);

   for(size_t i = 1; i != T_bits; ++i)
      {
      T[3*i+0] = T[3*i - 2];
      T[3*i+0].mult2(ws);
      T[3*i+1] = T[3*i+0];
      T[3*i+1].mult2(ws);
      T[3*i+2] = T[3*i+1];
      T[3*i+2].add(T[3*i+0], ws);
      }

   PointGFp::force_all_affine(T, ws[0].get_word_vector());

   m_W.resize(T.size() * 2 * m_p_words);

   word* p = &m_W[0];
   for(size_t i = 0; i != T.size(); ++i)
      {
      T[i].get_x().encode_words(p, m_p_words);
      p += m_p_words;
      T[i].get_y().encode_words(p, m_p_words);
      p += m_p_words;
      }
   }

PointGFp PointGFp_Base_Point_Precompute::mul(const BigInt& k,
                                             RandomNumberGenerator& rng,
                                             const BigInt& group_order,
                                             std::vector<BigInt>& ws) const
   {
   if(k.is_negative())
      throw Invalid_Argument("PointGFp_Base_Point_Precompute scalar must be positive");

   // Choose a small mask m and use k' = k + m*order (Coron's 1st countermeasure)
   const BigInt mask(rng, PointGFp_SCALAR_BLINDING_BITS);

   // Instead of reducing k mod group order should we alter the mask size??
   const BigInt scalar = m_mod_order.reduce(k) + group_order * mask;

   const size_t windows = round_up(scalar.bits(), 2) / 2;

   const size_t elem_size = 2*m_p_words;

   BOTAN_ASSERT(windows <= m_W.size() / (3*elem_size),
                "Precomputed sufficient values for scalar mult");

   PointGFp R = m_base_point.zero();

   if(ws.size() < PointGFp::WORKSPACE_SIZE)
      ws.resize(PointGFp::WORKSPACE_SIZE);

   // the precomputed multiples are not secret so use std::vector
   std::vector<word> Wt(elem_size);

   for(size_t i = 0; i != windows; ++i)
      {
      const size_t window = windows - i - 1;
      const size_t base_addr = (3*window)*elem_size;

      const word w = scalar.get_substring(2*window, 2);

      const word w_is_1 = CT::is_equal<word>(w, 1);
      const word w_is_2 = CT::is_equal<word>(w, 2);
      const word w_is_3 = CT::is_equal<word>(w, 3);

      for(size_t j = 0; j != elem_size; ++j)
         {
         const word w1 = m_W[base_addr + 0*elem_size + j];
         const word w2 = m_W[base_addr + 1*elem_size + j];
         const word w3 = m_W[base_addr + 2*elem_size + j];

         Wt[j] = CT::select3<word>(w_is_1, w1, w_is_2, w2, w_is_3, w3, 0);
         }

      R.add_affine(&Wt[0], m_p_words, &Wt[m_p_words], m_p_words, ws);

      if(i == 0)
         {
         /*
         * Since we start with the top bit of the exponent we know the
         * first window must have a non-zero element, and thus R is
         * now a point other than the point at infinity.
         */
         BOTAN_DEBUG_ASSERT(w != 0);
         R.randomize_repr(rng, ws[0].get_word_vector());
         }
      }

   BOTAN_DEBUG_ASSERT(R.on_the_curve());

   return R;
   }

PointGFp_Var_Point_Precompute::PointGFp_Var_Point_Precompute(const PointGFp& point,
                                                             RandomNumberGenerator& rng,
                                                             std::vector<BigInt>& ws) :
   m_curve(point.get_curve()),
   m_p_words(m_curve.get_p().sig_words()),
   m_window_bits(4)
   {
   if(ws.size() < PointGFp::WORKSPACE_SIZE)
      ws.resize(PointGFp::WORKSPACE_SIZE);

   std::vector<PointGFp> U(static_cast<size_t>(1) << m_window_bits);
   U[0] = point.zero();
   U[1] = point;

   for(size_t i = 2; i < U.size(); i += 2)
      {
      U[i] = U[i/2].double_of(ws);
      U[i+1] = U[i].plus(point, ws);
      }

   // Hack to handle Blinded_Point_Multiply
   if(rng.is_seeded())
      {
      BigInt& mask = ws[0];
      BigInt& mask2 = ws[1];
      BigInt& mask3 = ws[2];
      BigInt& new_x = ws[3];
      BigInt& new_y = ws[4];
      BigInt& new_z = ws[5];
      secure_vector<word>& tmp = ws[6].get_word_vector();

      const CurveGFp& curve = U[0].get_curve();

      const size_t p_bits = curve.get_p().bits();

      // Skipping zero point since it can't be randomized
      for(size_t i = 1; i != U.size(); ++i)
         {
         mask.randomize(rng, p_bits - 1, false);
         // Easy way of ensuring mask != 0
         mask.set_bit(0);

         curve.sqr(mask2, mask, tmp);
         curve.mul(mask3, mask, mask2, tmp);

         curve.mul(new_x, U[i].get_x(), mask2, tmp);
         curve.mul(new_y, U[i].get_y(), mask3, tmp);
         curve.mul(new_z, U[i].get_z(), mask, tmp);

         U[i].swap_coords(new_x, new_y, new_z);
         }
      }

   m_T.resize(U.size() * 3 * m_p_words);

   word* p = &m_T[0];
   for(size_t i = 0; i != U.size(); ++i)
      {
      U[i].get_x().encode_words(p              , m_p_words);
      U[i].get_y().encode_words(p +   m_p_words, m_p_words);
      U[i].get_z().encode_words(p + 2*m_p_words, m_p_words);
      p += 3*m_p_words;
      }
   }

PointGFp PointGFp_Var_Point_Precompute::mul(const BigInt& k,
                                            RandomNumberGenerator& rng,
                                            const BigInt& group_order,
                                            std::vector<BigInt>& ws) const
   {
   if(k.is_negative())
      throw Invalid_Argument("PointGFp_Var_Point_Precompute scalar must be positive");
   if(ws.size() < PointGFp::WORKSPACE_SIZE)
      ws.resize(PointGFp::WORKSPACE_SIZE);

   // Choose a small mask m and use k' = k + m*order (Coron's 1st countermeasure)
   const BigInt mask(rng, PointGFp_SCALAR_BLINDING_BITS, false);
   const BigInt scalar = k + group_order * mask;

   const size_t elem_size = 3*m_p_words;
   const size_t window_elems = (1ULL << m_window_bits);

   size_t windows = round_up(scalar.bits(), m_window_bits) / m_window_bits;
   PointGFp R(m_curve);
   secure_vector<word> e(elem_size);

   if(windows > 0)
      {
      windows--;

      const uint32_t w = scalar.get_substring(windows*m_window_bits, m_window_bits);

      clear_mem(e.data(), e.size());
      for(size_t i = 1; i != window_elems; ++i)
         {
         const word wmask = CT::is_equal<word>(w, i);

         for(size_t j = 0; j != elem_size; ++j)
            {
            e[j] |= wmask & m_T[i * elem_size + j];
            }
         }

      R.add(&e[0], m_p_words, &e[m_p_words], m_p_words, &e[2*m_p_words], m_p_words, ws);

      /*
      Randomize after adding the first nibble as before the addition R
      is zero, and we cannot effectively randomize the point
      representation of the zero point.
      */
      R.randomize_repr(rng, ws[0].get_word_vector());
      }

   while(windows)
      {
      R.mult2i(m_window_bits, ws);

      const uint32_t w = scalar.get_substring((windows-1)*m_window_bits, m_window_bits);

      clear_mem(e.data(), e.size());
      for(size_t i = 1; i != window_elems; ++i)
         {
         const word wmask = CT::is_equal<word>(w, i);

         for(size_t j = 0; j != elem_size; ++j)
            e[j] |= wmask & m_T[i * elem_size + j];
         }

      R.add(&e[0], m_p_words, &e[m_p_words], m_p_words, &e[2*m_p_words], m_p_words, ws);

      windows--;
      }

   BOTAN_DEBUG_ASSERT(R.on_the_curve());

   return R;
   }


PointGFp_Multi_Point_Precompute::PointGFp_Multi_Point_Precompute(const PointGFp& x,
                                                                 const PointGFp& y)
   {
   std::vector<BigInt> ws(PointGFp::WORKSPACE_SIZE);

   PointGFp x2 = x;
   x2.mult2(ws);

   const PointGFp x3(x2.plus(x, ws));

   PointGFp y2 = y;
   y2.mult2(ws);

   const PointGFp y3(y2.plus(y, ws));

   m_M.reserve(15);

   m_M.push_back(x);
   m_M.push_back(x2);
   m_M.push_back(x3);

   m_M.push_back(y);
   m_M.push_back(y.plus(x, ws));
   m_M.push_back(y.plus(x2, ws));
   m_M.push_back(y.plus(x3, ws));

   m_M.push_back(y2);
   m_M.push_back(y2.plus(x, ws));
   m_M.push_back(y2.plus(x2, ws));
   m_M.push_back(y2.plus(x3, ws));

   m_M.push_back(y3);
   m_M.push_back(y3.plus(x, ws));
   m_M.push_back(y3.plus(x2, ws));
   m_M.push_back(y3.plus(x3, ws));

   PointGFp::force_all_affine(m_M, ws[0].get_word_vector());
   }

PointGFp PointGFp_Multi_Point_Precompute::multi_exp(const BigInt& z1,
                                                    const BigInt& z2) const
   {
   std::vector<BigInt> ws(PointGFp::WORKSPACE_SIZE);

   const size_t z_bits = round_up(std::max(z1.bits(), z2.bits()), 2);

   PointGFp H = m_M[0].zero();

   for(size_t i = 0; i != z_bits; i += 2)
      {
      if(i > 0)
         {
         H.mult2i(2, ws);
         }

      const uint32_t z1_b = z1.get_substring(z_bits - i - 2, 2);
      const uint32_t z2_b = z2.get_substring(z_bits - i - 2, 2);

      const uint32_t z12 = (4*z2_b) + z1_b;

      // This function is not intended to be const time
      if(z12)
         {
         H.add_affine(m_M[z12-1], ws);
         }
      }

   if(z1.is_negative() != z2.is_negative())
      H.negate();

   return H;
   }

}
