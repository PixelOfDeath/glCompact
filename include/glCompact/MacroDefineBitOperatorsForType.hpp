/*
    glCompact
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
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
