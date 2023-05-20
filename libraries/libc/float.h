/*
    This file is part of duckOS.
    
    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) Byteduck 2016-2020. All rights reserved.
    Copyright (c) Chaziz 2022.
*/

#ifndef DUCKOS_LIBC_FLOAT_H
#define DUCKOS_LIBC_FLOAT_H

#define FLT_RADIX         2
#define DECIMAL_DIG       21
#define FLT_DECIMAL_DIG   9
#define DBL_DECIMAL_DIG   17
#define LDBL_DECIMAL_DIG  21
#define FLT_MIN           1.17549e-38
#define DBL_MIN           2.22507e-308
#define LDBL_MIN          3.3621e-4932
#define FLT_TRUE_MIN      1.4013e-45
#define DBL_TRUE_MIN      4.94066e-324
#define LDBL_TRUE_MIN     3.6452e-4951
#define FLT_MAX           3.40282e+38
#define DBL_MAX           1.79769e+308
#define LDBL_MAX          1.18973e+4932
#define FLT_EPSILON       1.19209e-07
#define DBL_EPSILON       2.22045e-16
#define LDBL_EPSILON      1.0842e-19
#define FLT_DIG           6
#define DBL_DIG           15
#define LDBL_DIG          18
#define FLT_MANT_DIG      24
#define DBL_MANT_DIG      53
#define LDBL_MANT_DIG     64
#define FLT_MIN_EXP       -125
#define DBL_MIN_EXP       -1021
#define LDBL_MIN_EXP      -16381
#define FLT_MIN_10_EXP    -37
#define DBL_MIN_10_EXP    -307
#define LDBL_MIN_10_EXP   -4931
#define FLT_MAX_EXP       128
#define DBL_MAX_EXP       1024
#define LDBL_MAX_EXP      16384
#define FLT_MAX_10_EXP    38
#define DBL_MAX_10_EXP    308
#define LDBL_MAX_10_EXP   4932
//TODO: FLT_ROUNDS
#define FLT_EVAL_METHOD   0
#define FLT_HAS_SUBNORM   1
#define DBL_HAS_SUBNORM   1
#define LDBL_HAS_SUBNORM  1

#endif //DUCKOS_LIBC_FLOAT_H
