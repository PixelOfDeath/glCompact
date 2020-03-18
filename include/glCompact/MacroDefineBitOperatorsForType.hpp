#pragma once

//Does NOT work for  enums/class enums  that are defined inside another class!
#define DEFINE_BIT_OPERATORS_FOR_TYPE_OUTSIDE_CLASS(ENUMTYPE) \
inline ENUMTYPE  operator& (ENUMTYPE& lhs, ENUMTYPE rhs){return ENUMTYPE(static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) &  static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));}             \
inline ENUMTYPE& operator&=(ENUMTYPE& lhs, ENUMTYPE rhs){lhs =  ENUMTYPE(static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) &  static_cast<std::underlying_type<ENUMTYPE>::type>(rhs)); return lhs;} \
inline ENUMTYPE  operator| (ENUMTYPE& lhs, ENUMTYPE rhs){return ENUMTYPE(static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) |  static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));}             \
inline ENUMTYPE& operator|=(ENUMTYPE& lhs, ENUMTYPE rhs){lhs =  ENUMTYPE(static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) |  static_cast<std::underlying_type<ENUMTYPE>::type>(rhs)); return lhs;} \
inline ENUMTYPE  operator^ (ENUMTYPE& lhs, ENUMTYPE rhs){return ENUMTYPE(static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) ^  static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));}             \
inline ENUMTYPE& operator^=(ENUMTYPE& lhs, ENUMTYPE rhs){lhs =  ENUMTYPE(static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) ^  static_cast<std::underlying_type<ENUMTYPE>::type>(rhs)); return lhs;} \
inline ENUMTYPE  operator~ (               ENUMTYPE rhs){return ENUMTYPE(                                                         ~static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));}
