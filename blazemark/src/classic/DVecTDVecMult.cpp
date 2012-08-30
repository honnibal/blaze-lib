//=================================================================================================
/*!
//  \file src/classic/DVecTDVecMult.cpp
//  \brief Source file for the classic dense vector/dense vector outer product kernel
//
//  Copyright (C) 2011 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. This library is free software; you can redistribute
//  it and/or modify it under the terms of the GNU General Public License as published by the
//  Free Software Foundation; either version 3, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with a special
//  exception for linking and compiling against the Blaze library, the so-called "runtime
//  exception"; see the file COPYING. If not, see http://www.gnu.org/licenses/.
*/
//=================================================================================================


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <iostream>
#include <blaze/util/Timing.h>
#include <blazemark/classic/DVecTDVecMult.h>
#include <blazemark/classic/init/Matrix.h>
#include <blazemark/classic/init/Vector.h>
#include <blazemark/classic/Matrix.h>
#include <blazemark/classic/Vector.h>
#include <blazemark/system/Config.h>


namespace blazemark {

namespace classic {

//=================================================================================================
//
//  KERNEL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Classic dense vector/dense vector outer product kernel.
//
// \param N The size of the vectors for the outer product.
// \param steps The number of iteration steps to perform.
// \return Minimum runtime of the kernel function.
//
// This kernel function implements the dense vector/dense vector outer product by means of
// classic C++ operator overloading.
*/
double dvectdvecmult( size_t N, size_t steps )
{
   using ::blazemark::element_t;

   ::blaze::setSeed( seed );

   ::blazemark::classic::Vector<element_t> a( N ), b( N );
   ::blazemark::classic::Matrix<element_t> A( N, N );
   ::blaze::timing::WcTimer timer;

   init( a );
   init( b );

   A = outer( a, b );

   for( size_t rep=0UL; rep<reps; ++rep )
   {
      timer.start();
      for( size_t step=0UL; step<steps; ++step ) {
         A = outer( a, b );
      }
      timer.end();

      if( A.rows() != N )
         std::cerr << " Line " << __LINE__ << ": ERROR detected!!!\n";

      if( timer.last() > maxtime )
         break;
   }

   const double minTime( timer.min()     );
   const double avgTime( timer.average() );

   if( minTime * ( 1.0 + deviation*0.01 ) < avgTime )
      std::cerr << " Classic kernel 'dvectdvecmult': Time deviation too large!!!\n";

   return minTime;
}
//*************************************************************************************************

} // namespace classic

} // namespace blazemark
