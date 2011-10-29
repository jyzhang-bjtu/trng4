// Copyright (C) 2006 Heiko Bauke <heiko.bauke@physik.uni-magdeburg.de>
//  
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License in
// version 2 as published by the Free Software Foundation.
//  
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//  

#if !(defined TRNG_BINOMIAL_DIST_HPP)

#define TRNG_BINOMIAL_DIST_HPP

#include <trng/limits.hpp>
#include <trng/utility.hpp>
#include <trng/math.hpp>
#include <ostream>
#include <istream>
#include <iomanip>
#include <vector>

namespace trng {

  // non-uniform random number generator class
  class binomial_dist {
  public:
    typedef int result_type;
    class param_type;
    
    class param_type {
    private:
      double p_;
      int n_;
      std::vector<double> P_;
      
      void calc_probabilities() {
	P_=std::vector<double>();
	double binom=1.0;
	for (int i=0; i<=n_; ++i) {
	  P_.push_back(binom*
		       math::pow(p_    , static_cast<double>(i   ))*
		       math::pow(1.0-p_, static_cast<double>(n_-i)));
	  binom*=n_-i;
	  binom/=i+1;
	}
	// build list with cumulative density function
	for (std::vector<double>::size_type i(1); i<P_.size(); ++i)
	  P_[i]+=P_[i-1];
	for (std::vector<double>::size_type i(0); i<P_.size(); ++i)
	  P_[i]/=P_.back();
      }
      
    public:
      double p() const { return p_; }
      void p(double p_new) { p_=p_new;  calc_probabilities(); }
      int n() const { return n_; }
      void n(int n_new) { n_=n_new;  calc_probabilities(); }
      explicit param_type(double p, int n) :
	p_(p), n_(n) {
	calc_probabilities();
      }
      friend class binomial_dist;
    };
    
  private:
    param_type P;
    
  public:
    // constructor
    explicit binomial_dist(double p, int n) : P(p, n) {
    }
    explicit binomial_dist(const param_type &P) : P(P) {
    }
    // reset internal state
    void reset() { }
    // random numbers
    template<typename R>
    int operator()(R &r) {
      return utility::discrete(utility::uniformco(r), P.P_.begin(), P.P_.end());
    }
    template<typename R>
    int operator()(R &r, const param_type &p) {
      binomial_dist g(p);
      return g(r);
    }
    // property methods
    int min() const { return 0; }
    int max() const { return P.n(); }
    param_type param() const { return P; }
    void param(const param_type &P_new) { P=P_new; }
    double p() const { return P.p(); }
    void p(double p_new) { P.p(p_new); }
    int n() const { return P.n(); }
    void n(int n_new) { P.n(n_new); }
    // probability density function  
    double pdf(int x) const {
      if (x<0 || x>P.n())
        return 0.0;
      if (x==0)
        return P.P_[0];
      return P.P_[x]-P.P_[x-1];
    }
    // cumulative density function 
    double cdf(int x) const {
      if (x<0)
        return 0.0;
      if (x<=P.n())
        return P.P_[x];
      return 1.0;
    }
  };

  // -------------------------------------------------------------------

  // EqualityComparable concept
  inline bool operator==(const binomial_dist::param_type &p1, 
			 const binomial_dist::param_type &p2) {
    return p1.p()==p2.p() && p1.n()==p2.n();
  }
  inline bool operator!=(const binomial_dist::param_type &p1, 
			 const binomial_dist::param_type &p2) {
    return !(p1==p2);
  }
  
  // Streamable concept
  template<typename char_t, typename traits_t>
  std::basic_ostream<char_t, traits_t> &
  operator<<(std::basic_ostream<char_t, traits_t> &out,
	     const binomial_dist::param_type &P) {
    std::ios_base::fmtflags flags(out.flags());
    out.flags(std::ios_base::dec | std::ios_base::fixed |
	      std::ios_base::left);
    out << '('
	<< std::setprecision(16) << P.p() << ' ' << P.n()
	<< ')';
    out.flags(flags);
    return out;
  }
  
  template<typename char_t, typename traits_t>
  std::basic_istream<char_t, traits_t> &
  operator>>(std::basic_istream<char_t, traits_t> &in,
	     binomial_dist::param_type &P) {
    double p;
    int n;
    std::ios_base::fmtflags flags(in.flags());
    in.flags(std::ios_base::dec | std::ios_base::fixed |
	     std::ios_base::left);
    in >> utility::delim('(')
       >> p >> utility::delim(' ')
       >> n >> utility::delim(')');
    if (in)
      P=binomial_dist::param_type(p, n);
    in.flags(flags);
    return in;
  }
  
  // -------------------------------------------------------------------

  // EqualityComparable concept
  inline bool operator==(const binomial_dist &g1, 
			 const binomial_dist &g2) {
    return g1.param()==g2.param();
  }
  inline bool operator!=(const binomial_dist &g1, 
			 const binomial_dist &g2) {
    return g1.param()!=g2.param();
  }
  
  // Streamable concept
  template<typename char_t, typename traits_t>
  std::basic_ostream<char_t, traits_t> &
  operator<<(std::basic_ostream<char_t, traits_t> &out,
	     const binomial_dist &g) {
    std::ios_base::fmtflags flags(out.flags());
    out.flags(std::ios_base::dec | std::ios_base::fixed |
	      std::ios_base::left);
    out << "[binomial " << g.param() << ']';
    out.flags(flags);
    return out;
  }
  
  template<typename char_t, typename traits_t>
  std::basic_istream<char_t, traits_t> &
  operator>>(std::basic_istream<char_t, traits_t> &in,
	     binomial_dist &g) {
    binomial_dist::param_type p;
    std::ios_base::fmtflags flags(in.flags());
    in.flags(std::ios_base::dec | std::ios_base::fixed |
	     std::ios_base::left);
    in >> utility::ignore_spaces()
       >> utility::delim("[binomial ") >> p >> utility::delim(']');
    if (in)
      g.param(p);
    in.flags(flags);
    return in;
  }
  
}

#endif
