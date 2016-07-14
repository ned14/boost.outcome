/* value_storage.hpp
The world's most simple C++ monad
(C) 2015 Niall Douglas http://www.nedprod.com/
File Created: July 2015


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "config.hpp"

#ifndef BOOST_OUTCOME_VALUE_STORAGE_H
#define BOOST_OUTCOME_VALUE_STORAGE_H

/*! \file value_storage.hpp
\brief Provides a fixed four state variant
*/

BOOST_OUTCOME_V1_NAMESPACE_BEGIN

//! \brief Type tag for an empty monad \ingroup monad
struct empty_t
{
  constexpr empty_t() {}
};

//! \brief Variable of type empty_t \ingroup monad
constexpr empty_t empty = empty_t();

//! \brief Type tag for a valued monad \ingroup monad
struct value_t
{
  constexpr value_t() {}
};

//! \brief Variable of type value_t \ingroup monad
constexpr value_t value = value_t();

//! \brief Type tag for an errored monad \ingroup monad
struct error_t
{
  constexpr error_t() {}
};

//! \brief Variable of type error_t \ingroup monad
constexpr error_t error = error_t();

//! \brief Type tag for an excepted monad \ingroup monad
struct exception_t
{
  constexpr exception_t() {}
};

//! \brief Variable of type exception_t \ingroup monad
constexpr exception_t exception = exception_t();

//! \brief Specialise to indicate that this type should use the single byte storage layout. You get six bits of storage.
template <class _value_type> struct enable_single_byte_value_storage : std::false_type
{
};
template <> struct enable_single_byte_value_storage<void> : std::true_type
{
};
template <> struct enable_single_byte_value_storage<bool> : std::true_type
{
};

namespace detail
{
#define BOOST_OUTCOME_VALUE_STORAGE_IMPL value_storage_impl_trivial
#define BOOST_OUTCOME_VALUE_STORAGE_NON_TRIVIAL_DESTRUCTOR 0
#include "detail/value_storage.ipp"
#undef BOOST_OUTCOME_VALUE_STORAGE_IMPL
#undef BOOST_OUTCOME_VALUE_STORAGE_NON_TRIVIAL_DESTRUCTOR

#define BOOST_OUTCOME_VALUE_STORAGE_IMPL value_storage_impl_nontrivial
#define BOOST_OUTCOME_VALUE_STORAGE_NON_TRIVIAL_DESTRUCTOR 1
#include "detail/value_storage.ipp"
#undef BOOST_OUTCOME_VALUE_STORAGE_IMPL
#undef BOOST_OUTCOME_VALUE_STORAGE_NON_TRIVIAL_DESTRUCTOR

  template <class _value_type, class _error_type, class _exception_type>
  static constexpr bool can_have_trivial_destructor = (std::is_literal_type<_value_type>::value || std::is_trivially_destructible<_value_type>::value) && (std::is_literal_type<_error_type>::value || std::is_trivially_destructible<_error_type>::value) &&
                                                      (std::is_literal_type<_exception_type>::value || std::is_trivially_destructible<_exception_type>::value);
  template <bool enable, class U, class V> struct move_construct_if_impl
  {
    void operator()(U *v, V &&o) const { new(v) U(std::move(o)); }
  };
  template <class U, class V> struct move_construct_if_impl<false, U, V>
  {
    void operator()(U *, V &&) const {}
  };
  template <bool enable, class U, class V> inline void move_construct_if(U *v, V &&o) { move_construct_if_impl<enable, U, V>()(v, std::move(o)); }

  template <bool enable, class U, class V> struct compare_if_impl
  {
    bool operator()(const U &v, const V &o) const { return v == o; }
  };
  template <class U, class V> struct compare_if_impl<false, U, V>
  {
    bool operator()(const U &, const V &) const { return false; }
  };
  template <bool enable, class U, class V> inline bool compare_if(const U &v, const V &o) { return compare_if_impl<enable, U, V>()(v, o); }
}

/*! \class value_storage
\tparam _value_type The value type to use
\tparam _error_type The error type to use. Can be void to disable.
\tparam _exception_type The exception type to use. Can be void to disable.
\brief A fixed lightweight variant store for monad.
\ingroup monad

This fixed variant list of empty, a type `value_type`, a lightweight `error_type` or a
heavier `exception_type` typically has a space cost of `max(24, sizeof(R)+8)`. If however
you specialise `enable_single_byte_value_storage<T>` with `true_type`, and both `error_type`
and `exception_type` are disabled (void), a special single byte storage implementation is
enabled. Both `bool` and `void` are already specialised.
*/
template <class _value_type, class _error_type, class _exception_type>
class value_storage : public std::conditional<detail::can_have_trivial_destructor<_value_type, _error_type, _exception_type>, detail::value_storage_impl_trivial<_value_type, _error_type, _exception_type>, detail::value_storage_impl_nontrivial<_value_type, _error_type, _exception_type>>::type
{
  using base = typename std::conditional<detail::can_have_trivial_destructor<_value_type, _error_type, _exception_type>, detail::value_storage_impl_trivial<_value_type, _error_type, _exception_type>, detail::value_storage_impl_nontrivial<_value_type, _error_type, _exception_type>>::type;

public:
  static constexpr bool has_value_type = base::has_value_type;
  static constexpr bool has_error_type = base::has_error_type;
  static constexpr bool has_exception_type = base::has_exception_type;
  using value_type = typename base::value_type;
  using error_type = typename base::error_type;
  using exception_type = typename base::exception_type;
  using storage_type = typename base::storage_type;
  using base::clear;
  static_assert(!std::is_same<value_type, error_type>::value, "R and error_type cannot be the same type");
  static_assert(!std::is_same<value_type, exception_type>::value, "R and exception_type cannot be the same type");
  static_assert(!std::is_same<error_type, exception_type>::value, "error_type and exception_type cannot be the same type");


  static constexpr bool is_nothrow_copy_constructible = std::is_nothrow_copy_constructible<value_type>::value && std::is_nothrow_copy_constructible<exception_type>::value && std::is_nothrow_copy_constructible<error_type>::value;
  static constexpr bool is_nothrow_move_constructible = std::is_nothrow_move_constructible<value_type>::value && std::is_nothrow_move_constructible<exception_type>::value && std::is_nothrow_move_constructible<error_type>::value;
  static constexpr bool is_nothrow_copy_assignable = std::is_nothrow_copy_assignable<value_type>::value && std::is_nothrow_copy_assignable<exception_type>::value && std::is_nothrow_copy_assignable<error_type>::value;
  static constexpr bool is_nothrow_move_assignable = std::is_nothrow_move_assignable<value_type>::value && std::is_nothrow_move_assignable<exception_type>::value && std::is_nothrow_move_assignable<error_type>::value;
  static constexpr bool is_nothrow_destructible = base::is_nothrow_destructible;

  template <class _value_type2> static constexpr bool value_type_is_constructible_from = std::is_same<_value_type, _value_type2>::value || std::is_constructible<_value_type, _value_type2>::value;
  template <class _error_type2> static constexpr bool error_type_is_constructible_from = std::is_void<_error_type2>::value || std::is_same<_error_type, _error_type2>::value || std::is_constructible<_error_type, _error_type2>::value;
  template <class _exception_type2> static constexpr bool exception_type_is_constructible_from = std::is_void<_exception_type2>::value || std::is_same<_exception_type, _exception_type2>::value || std::is_constructible<_exception_type, _exception_type2>::value;
  // True if this storage can be constructed from the specified storage
  template <class _value_type2, class _error_type2, class _exception_type2> static constexpr bool is_constructible_from = value_type_is_constructible_from<_value_type2> &&error_type_is_constructible_from<_error_type2> &&exception_type_is_constructible_from<_exception_type2>;

  template <class _value_type2> static constexpr bool value_type_is_comparable_to = std::is_void<_value_type2>::value || value_type_is_constructible_from<_value_type2>;
  template <class _error_type2> static constexpr bool error_type_is_comparable_to = error_type_is_constructible_from<_error_type2>;
  template <class _exception_type2> static constexpr bool exception_type_is_comparable_to = exception_type_is_constructible_from<_exception_type2>;
  // True if this storage can be compared to the specified storage
  template <class _value_type2, class _error_type2, class _exception_type2> static constexpr bool is_comparable_to = value_type_is_comparable_to<_value_type2> &&error_type_is_comparable_to<_error_type2> &&exception_type_is_comparable_to<_exception_type2>;

  constexpr value_storage() = default;
  constexpr value_storage(empty_t _) noexcept : base(_) {}
  constexpr value_storage(value_t _) noexcept(std::is_nothrow_default_constructible<value_type>::value)
      : base(_)
  {
  }
  constexpr value_storage(error_t _) noexcept(std::is_nothrow_default_constructible<error_type>::value)
      : base(_)
  {
  }
  constexpr value_storage(exception_t _) noexcept(std::is_nothrow_default_constructible<exception_type>::value)
      : base(_)
  {
  }
  constexpr value_storage(const value_type &v) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
      : base(v)
  {
  }
  constexpr value_storage(const error_type &v) noexcept(std::is_nothrow_copy_constructible<error_type>::value)
      : base(v)
  {
  }
  constexpr value_storage(const exception_type &v) noexcept(std::is_nothrow_copy_constructible<exception_type>::value)
      : base(v)
  {
  }
  constexpr value_storage(value_type &&v) noexcept(std::is_nothrow_move_constructible<value_type>::value)
      : base(std::move(v))
  {
  }
  constexpr value_storage(error_type &&v) noexcept(std::is_nothrow_move_constructible<error_type>::value)
      : base(std::move(v))
  {
  }
  constexpr value_storage(exception_type &&v) noexcept(std::is_nothrow_move_constructible<exception_type>::value)
      : base(std::move(v))
  {
  }
  using emplace_t = typename base::emplace_t;
  template <class... Args>
  constexpr explicit value_storage(emplace_t _, Args &&... args)
#if !defined(_MSC_VER) || _MSC_VER > 190022816
  noexcept(std::is_nothrow_constructible<value_type, Args...>::value)
#endif
      : base(_, std::forward<Args>(args)...)
  {
  }
  template <class _value_type2, class _error_type2, class _exception_type2, typename = typename std::enable_if<is_constructible_from<_value_type2, _error_type2, _exception_type2>>::type>
  BOOST_OUTCOME_CXX14_CONSTEXPR explicit value_storage(const value_storage<_value_type2, _error_type2, _exception_type2> &o)
      : value_storage(value_storage<_value_type2, _error_type2, _exception_type2>(o) /* delegate to move constructor */)
  {
  }
  template <class _value_type2, class _error_type2, class _exception_type2, typename = typename std::enable_if<is_constructible_from<_value_type2, _error_type2, _exception_type2>>::type> BOOST_OUTCOME_CXX14_CONSTEXPR explicit value_storage(value_storage<_value_type2, _error_type2, _exception_type2> &&o)
  {
    switch(o.type)
    {
    case storage_type::empty:
      this->type = storage_type::empty;
      break;
    case storage_type::value:
      detail::move_construct_if<has_value_type && value_storage<_value_type2, _error_type2, _exception_type2>::has_value_type>(&this->_value_raw, std::move(o._value_raw));
      this->type = storage_type::value;
      break;
    case storage_type::error:
      detail::move_construct_if<has_error_type && value_storage<_value_type2, _error_type2, _exception_type2>::has_error_type>(&this->error, std::move(o.error));
      this->type = storage_type::error;
      break;
    case storage_type::exception:
      detail::move_construct_if<has_exception_type && value_storage<_value_type2, _error_type2, _exception_type2>::has_exception_type>(&this->exception, std::move(o.exception));
      this->type = storage_type::exception;
      break;
    }
  }
  BOOST_OUTCOME_CXX14_CONSTEXPR value_storage(const value_storage &o) noexcept(is_nothrow_copy_constructible)
      : base()
  {
    switch(o.type)
    {
    case storage_type::empty:
      break;
    case storage_type::value:
      new(&this->_value_raw) value_type(o._value_raw);
      break;
    case storage_type::error:
      new(&this->error) error_type(o.error);
      break;
    case storage_type::exception:
      new(&this->exception) exception_type(o.exception);
      break;
    }
    this->type = o.type;
  }
  BOOST_OUTCOME_CXX14_CONSTEXPR value_storage(value_storage &&o) noexcept(is_nothrow_move_constructible)
      : base()
  {
    switch(o.type)
    {
    case storage_type::empty:
      break;
    case storage_type::value:
      new(&this->_value_raw) value_type(std::move(o._value_raw));
      break;
    case storage_type::error:
      new(&this->error) error_type(std::move(o.error));
      break;
    case storage_type::exception:
      new(&this->exception) exception_type(std::move(o.exception));
      break;
    }
    this->type = o.type;
  }
  BOOST_OUTCOME_CXX14_CONSTEXPR value_storage &operator=(const value_storage &o) noexcept(is_nothrow_destructible &&is_nothrow_copy_constructible)
  {
    clear();
    new(this) value_storage(o);
    return *this;
  }
  BOOST_OUTCOME_CXX14_CONSTEXPR value_storage &operator=(value_storage &&o) noexcept(is_nothrow_destructible &&is_nothrow_move_constructible)
  {
    clear();
    new(this) value_storage(std::move(o));
    return *this;
  }
  BOOST_OUTCOME_CXX14_CONSTEXPR void set_state(value_storage &&o) noexcept(is_nothrow_destructible &&is_nothrow_move_constructible)
  {
    clear();
    new(this) value_storage(std::move(o));
  }

  BOOST_OUTCOME_CXX14_CONSTEXPR void swap(value_storage &o) noexcept(is_nothrow_move_constructible)
  {
    if(this->type == o.type)
    {
      switch(this->type)
      {
      case storage_type::empty:
        break;
      case storage_type::value:
        std::swap(this->value, o.value);
        break;
      case storage_type::error:
        std::swap(this->error, o.error);
        break;
      case storage_type::exception:
        std::swap(this->exception, o.exception);
        break;
      }
    }
    else
    {
      value_storage temp(std::move(o));
      o = std::move(*this);
      *this = std::move(temp);
    }
  }
  template <class U> BOOST_OUTCOME_CXX14_CONSTEXPR void set_value(U &&v)
  {
    assert(this->type == storage_type::empty);
    new(&this->_value_raw) value_type(std::forward<U>(v));
    this->type = storage_type::value;
  }
  template <class... Args> BOOST_OUTCOME_CXX14_CONSTEXPR void emplace_value(Args &&... v)
  {
    assert(this->type == storage_type::empty);
    new(&this->_value_raw) value_type(std::forward<Args>(v)...);
    this->type = storage_type::value;
  }
  void set_exception(exception_type e)
  {
    assert(this->type == storage_type::empty);
    new(&this->exception) exception_type(std::move(e));
    this->type = storage_type::exception;
  }
  // Note to self: this can't be BOOST_OUTCOME_CXX14_CONSTEXPR
  void set_error(error_type e)
  {
    assert(this->type == storage_type::empty);
    new(&this->error) error_type(std::move(e));
    this->type = storage_type::error;
  }
  template <class _value_type2, class _error_type2, class _exception_type2, typename = typename std::enable_if<is_comparable_to<_value_type2, _error_type2, _exception_type2>>::type> BOOST_OUTCOME_CXX14_CONSTEXPR bool operator==(const value_storage<_value_type2, _error_type2, _exception_type2> &o) const
  {
    if(this->type != o.type)
      return false;
    switch(this->type)
    {
    case storage_type::empty:
      return true;
    case storage_type::value:
      return (!has_value_type && !value_storage<_value_type2, _error_type2, _exception_type2>::has_value_type) || detail::compare_if < has_value_type && value_storage<_value_type2, _error_type2, _exception_type2>::has_value_type > (this->value, o.value);
    case storage_type::error:
      return detail::compare_if < has_error_type && value_storage<_value_type2, _error_type2, _exception_type2>::has_error_type > (this->error, o.error);
    case storage_type::exception:
      return detail::compare_if < has_exception_type && value_storage<_value_type2, _error_type2, _exception_type2>::has_exception_type > (this->exception, o.exception);
    }
    return false;
  }
  template <class _value_type2, class _error_type2, class _exception_type2, typename = typename std::enable_if<is_comparable_to<_value_type2, _error_type2, _exception_type2>>::type> BOOST_OUTCOME_CXX14_CONSTEXPR bool operator!=(const value_storage<_value_type2, _error_type2, _exception_type2> &o) const
  {
    return !(*this == o);
  }
};

BOOST_OUTCOME_V1_NAMESPACE_END

namespace std
{
  //! \brief Deserialise a value_storage value_type (only value_type) \ingroup monad
  template <class _value_type, class _error_type, class _exception_type> inline istream &operator>>(istream &s, BOOST_OUTCOME_V1_NAMESPACE::value_storage<_value_type, _error_type, _exception_type> &v)
  {
    using namespace BOOST_OUTCOME_V1_NAMESPACE;
    switch(v.type)
    {
    case value_storage<_value_type, _error_type, _exception_type>::storage_type::value:
      return s >> v.value;
    default:
      throw ios_base::failure("Set the type of lightweight_futures::value_storage to a value_type before deserialising into it");
    }
  }
  //! \brief Serialise a value_storage. Mostly useful for debug printing. \ingroup monad
  template <class _value_type> inline ostream &operator<<(ostream &s, const BOOST_OUTCOME_V1_NAMESPACE::value_storage<_value_type, void, void> &v)
  {
    using namespace BOOST_OUTCOME_V1_NAMESPACE;
    using _error_type = void;
    using _exception_type = void;
    switch(v.type)
    {
    case value_storage<_value_type, _error_type, _exception_type>::storage_type::empty:
      return s << "(empty)";
    case value_storage<_value_type, _error_type, _exception_type>::storage_type::value:
      return s << v.value;
    default:
      return s << "(unknown)";
    }
  }
  template <class _value_type, class _error_type> inline ostream &operator<<(ostream &s, const BOOST_OUTCOME_V1_NAMESPACE::value_storage<_value_type, _error_type, void> &v)
  {
    using namespace BOOST_OUTCOME_V1_NAMESPACE;
    using _exception_type = void;
    switch(v.type)
    {
    case value_storage<_value_type, _error_type, _exception_type>::storage_type::empty:
      return s << "(empty)";
    case value_storage<_value_type, _error_type, _exception_type>::storage_type::value:
      return s << v.value;
    case value_storage<_value_type, _error_type, _exception_type>::storage_type::error:
      return s << v.error;
    default:
      return s << "(unknown)";
    }
  }
  template <class _value_type, class _error_type, class _exception_type> inline ostream &operator<<(ostream &s, const BOOST_OUTCOME_V1_NAMESPACE::value_storage<_value_type, _error_type, _exception_type> &v)
  {
    using namespace BOOST_OUTCOME_V1_NAMESPACE;
    switch(v.type)
    {
    case value_storage<_value_type, _error_type, _exception_type>::storage_type::empty:
      return s << "(empty)";
    case value_storage<_value_type, _error_type, _exception_type>::storage_type::value:
      return s << v.value;
    case value_storage<_value_type, _error_type, _exception_type>::storage_type::error:
      return s << v.error;
    case value_storage<_value_type, _error_type, _exception_type>::storage_type::exception:
      try
      {
        rethrow_exception(v.exception);
      }
      catch(const system_error &e)
      {
        return s << "std::system_error code " << e.code() << ": " << e.what();
      }
      /*catch(const future_error &e)
      {
      return s << "(std::future_error code " << e.code() << ": " << e.what() << ")";
      }*/
      catch(const exception &e)
      {
        return s << "std::exception: " << e.what();
      }
      catch(...)
      {
        return s << "unknown exception";
      }
    default:
      return s << "(unknown)";
    }
  }
}

#endif
