/**
 * \file	math_helper.h
 * \author	Thomas Barth 	Hochschule Darmstadt - thomas.barth@h-da.de
 * \date 	22.06.2018
 * \version	1.0
 *
 * \brief Commonly used mathematical functions
 *
 *	Changelog:\n
 *
 * \copyright Copyright ©2018
 * Department of electrical engineering and information technology, Hochschule Darmstadt - University of applied sciences (h_da). All Rights Reserved.
 * Permission to use, copy, modify, and distribute this software and its documentation for educational, and research purposes in the context of non-commercial
 * (unless permitted by h_da) and official h_da projects, is hereby granted for enrolled students of h_da, provided that the above copyright notice,
 * this paragraph and the following paragraph appear in all copies, modifications, and distributions.
 * Contact Prof.Dr.-Ing. Peter Fromm, peter.fromm@h-da.de, Birkenweg 8 64295 Darmstadt - GERMANY for commercial requests.
 *
 * \warning This software is a PROTOTYPE version and is not designed or intended for use in production, especially not for safety-critical applications!
 * The user represents and warrants that it will NOT use or redistribute the Software for such purposes.
 * This prototype is for research purposes only. This software is provided "AS IS," without a warranty of any kind.
 */

#ifndef _MATH_HELPER_H_
#define _MATH_HELPER_H_

#include "global.h"
#include <math.h>

/** \brief Round value */
#define _roundf(X)	((((X) - (sint32_t)(X)) > 0.5) ? (1 + (sint32_t)(X)) : ((sint32_t)(X)))

/** \brief float absolute value */
#define _absf(X)    ( ((X) < 0.0) ? -(X) : (X) )

/** \brief integer absolute value */
#define _abs(X)    ( ((X) < 0) ? -(X) : (X) )

/** \brief  Binary inequality comparison X != Y*/
#define _neqf(X,Y)  ( ((X) > (Y)) ||  ((X) < (Y)) )

/** \brief  Binary lesser than or equal comparison X <= Y*/
#define _leqf(X,Y)  ( !((X) > (Y)) )

/** \brief  get the smaller value out of 2 */
#define _minf(X,Y)  ( ((X) < (Y)) ? (X) : (Y) )

/** \brief  get the smaller value out of 2 */
#define _maxf(X,Y)  ( ((X) > (Y)) ? (X) : (Y) )

/** \brief ceilling division */
#define _ceil_div(x, y) ((x) + (y) - 1) / (y)

/** \brief ceil to pos */
#define _ceil_pos(X) ((X-(int)(X)) > 0 ? (int)(X+1) : (int)(X))

/** \brief definition of PI constant */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/** \brief fraction datatype */
typedef struct{
	uint32_t	numerator;			/**< \brief Numerator*/
	uint32_t	denominator;		/**< \brief denominator*/
}fraction_t;

/** \brief get the GCD(greatest common divisor) of 2 numbers
 *
 *	\param a	First number
 *	\param b	Second number
 *	\return 	GCD
 */
STATIC_INLINE long gcd(long a, long b){

    while(1){
        if (a == 0)
            return b;
        else if (b == 0)
            return a;

        if (a < b)
            b=b%a;
        else{
        	long old_a=a;
        	a=b;
        	b=old_a%b;
        }
    }
}

/** \brief Convert a double number into a fraction
 *
 *	\param input	Double number to be converted
 *	\return 		Fractional representation of the number
 */
STATIC_INLINE fraction_t d2frac(double input){

	fraction_t _ret;

    double integral = floor(input);
    double frac = input - integral;

    const long precision = 1000000000; // This is the accuracy.

    long gcd_ = gcd(round(frac * precision), precision);

    _ret.denominator= precision / gcd_;
    _ret.numerator = round(frac * precision) / gcd_;

    return _ret;
}

#endif /* _MATH_HELPER_H_ */
